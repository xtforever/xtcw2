#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "svg_factory.h"
#include "nanosvg.h"
#include "mls.h"
#include "m_tool.h"

/* ------------------------------------------------------------------------- *
 * Caches
 * ------------------------------------------------------------------------- */

typedef struct {
    int        filename_handle;
    NSVGimage *image;
    float      minx, miny, maxx, maxy;  /* solid content bounds (crop box) */
} SvgParseEntry;

typedef struct {
    int              filename_handle;
    int              w, h;
    cairo_surface_t *surface;
} SvgRasterEntry;

static int g_parse_cache  = 0;  /* MLS list of SvgParseEntry  */
static int g_raster_cache = 0;  /* MLS list of SvgRasterEntry */

/* ------------------------------------------------------------------------- *
 * Rendering helpers (shared by the rasteriser)
 * ------------------------------------------------------------------------- */

/* Bounding box of the *solid* shapes: shadow/glow shapes (low opacity) are
 * excluded because nanoSVG ignores SVG filters but would still pollute the
 * bounds, pushing the visible artwork off the text baseline.  Falls back to all
 * visible shapes, then to the viewBox. */
static void solid_bounds(NSVGimage *image, float *bminx, float *bminy,
                         float *bmaxx, float *bmaxy) {
    float minx = 1e30f, miny = 1e30f, maxx = -1e30f, maxy = -1e30f;    /* solid */
    float aminx = 1e30f, aminy = 1e30f, amaxx = -1e30f, amaxy = -1e30f; /* all */
    for (NSVGshape *s = image->shapes; s != NULL; s = s->next) {
        if (!(s->flags & NSVG_FLAGS_VISIBLE)) continue;
        if (s->bounds[0] < aminx) aminx = s->bounds[0];
        if (s->bounds[1] < aminy) aminy = s->bounds[1];
        if (s->bounds[2] > amaxx) amaxx = s->bounds[2];
        if (s->bounds[3] > amaxy) amaxy = s->bounds[3];
        if (s->opacity < 0.9f) continue;   /* skip shadows/glows */
        if (s->bounds[0] < minx) minx = s->bounds[0];
        if (s->bounds[1] < miny) miny = s->bounds[1];
        if (s->bounds[2] > maxx) maxx = s->bounds[2];
        if (s->bounds[3] > maxy) maxy = s->bounds[3];
    }
    if (maxx <= minx || maxy <= miny) { minx = aminx; miny = aminy; maxx = amaxx; maxy = amaxy; }
    if (maxx <= minx || maxy <= miny) { minx = miny = 0.0f; maxx = image->width; maxy = image->height; }

    *bminx = minx; *bminy = miny; *bmaxx = maxx; *bmaxy = maxy;
}

static void set_source_nsvg(cairo_t *cr, NSVGpaint *p, float opacity) {
    if (p->type == NSVG_PAINT_COLOR) {
        uint32_t c = p->color;
        cairo_set_source_rgba(cr, ((c >> 16) & 0xff) / 255.0,
                              ((c >> 8) & 0xff) / 255.0,
                              (c & 0xff) / 255.0,
                              ((c >> 24) & 0xff) / 255.0 * opacity);
    } else if (p->type == NSVG_PAINT_LINEAR_GRADIENT || p->type == NSVG_PAINT_RADIAL_GRADIENT) {
        NSVGgradient *grad = p->gradient;
        cairo_pattern_t *pat;
        float *t = grad->xform;
        cairo_matrix_t matrix;
        cairo_matrix_init(&matrix, t[0], t[1], t[2], t[3], t[4], t[5]);

        if (p->type == NSVG_PAINT_LINEAR_GRADIENT) {
            pat = cairo_pattern_create_linear(0, 0, 0, 1);
        } else {
            pat = cairo_pattern_create_radial(grad->fx + t[4], grad->fy + t[5], 0, 0, 0, 1);
        }
        cairo_pattern_set_matrix(pat, &matrix);

        for (int i = 0; i < grad->nstops; i++) {
            uint32_t c = grad->stops[i].color;
            cairo_pattern_add_color_stop_rgba(pat, grad->stops[i].offset,
                                              ((c >> 16) & 0xff) / 255.0,
                                              ((c >> 8) & 0xff) / 255.0,
                                              (c & 0xff) / 255.0,
                                              ((c >> 24) & 0xff) / 255.0 * opacity);
        }

        if (grad->spread == NSVG_SPREAD_PAD) cairo_pattern_set_extend(pat, CAIRO_EXTEND_PAD);
        else if (grad->spread == NSVG_SPREAD_REFLECT) cairo_pattern_set_extend(pat, CAIRO_EXTEND_REFLECT);
        else if (grad->spread == NSVG_SPREAD_REPEAT) cairo_pattern_set_extend(pat, CAIRO_EXTEND_REPEAT);

        cairo_set_source(cr, pat);
        cairo_pattern_destroy(pat);
    } else {
        cairo_set_source_rgba(cr, 0, 0, 0, opacity);
    }
}

static void render_shapes(cairo_t *cr, NSVGimage *image) {
    for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
        if (!(shape->flags & NSVG_FLAGS_VISIBLE)) continue;

        cairo_new_path(cr);
        for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
            cairo_move_to(cr, path->pts[0], path->pts[1]);
            for (int i = 0; i < path->npts - 1; i += 3) {
                float *p = &path->pts[i * 2];
                cairo_curve_to(cr, p[2], p[3], p[4], p[5], p[6], p[7]);
            }
            if (path->closed) cairo_close_path(cr);
        }

        if (shape->fill.type != NSVG_PAINT_NONE) {
            cairo_set_fill_rule(cr, shape->fillRule == NSVG_FILLRULE_EVENODD
                                    ? CAIRO_FILL_RULE_EVEN_ODD
                                    : CAIRO_FILL_RULE_WINDING);
            set_source_nsvg(cr, &shape->fill, shape->opacity);
            if (shape->stroke.type != NSVG_PAINT_NONE) cairo_fill_preserve(cr);
            else cairo_fill(cr);
        }

        if (shape->stroke.type != NSVG_PAINT_NONE) {
            set_source_nsvg(cr, &shape->stroke, shape->opacity);
            cairo_set_line_width(cr, shape->strokeWidth);
            cairo_set_line_cap(cr, (cairo_line_cap_t)shape->strokeLineCap);
            cairo_set_line_join(cr, (cairo_line_join_t)shape->strokeLineJoin);
            cairo_stroke(cr);
        } else {
            cairo_new_path(cr);
        }
    }
}

/* ------------------------------------------------------------------------- *
 * Public API
 * ------------------------------------------------------------------------- */

cairo_surface_t *svg_factory_get_image(const char *filename, int pixel_w, int pixel_h) {
    if (!filename || pixel_w <= 0 || pixel_h <= 0) return NULL;
    if (g_parse_cache == 0) g_parse_cache = m_create(8, sizeof(SvgParseEntry));
    if (g_raster_cache == 0) g_raster_cache = m_create(8, sizeof(SvgRasterEntry));

    int handle = conststr_lookup_c(filename);

    /* Raster cache hit? */
    {
        int p; SvgRasterEntry *e;
        m_foreach(g_raster_cache, p, e) {
            if (e->filename_handle == handle && e->w == pixel_w && e->h == pixel_h) {
                TRACE(50, "svg_factory: cache hit %s %dx%d", filename, pixel_w, pixel_h);
                return e->surface;
            }
        }
    }

    /* Parsed image + crop bounds (per filename). */
    NSVGimage *image = NULL;
    float minx = 0, miny = 0, maxx = 0, maxy = 0;
    int found = 0;
    {
        int p; SvgParseEntry *e;
        m_foreach(g_parse_cache, p, e) {
            if (e->filename_handle == handle) {
                image = e->image;
                minx = e->minx; miny = e->miny; maxx = e->maxx; maxy = e->maxy;
                found = 1;
                break;
            }
        }
    }
    if (!found) {
        image = nsvgParseFromFile(filename, "px", 96.0f);
        if (!image) {
            fprintf(stderr, "Could not open SVG image: %s\n", filename);
            return NULL;
        }
        solid_bounds(image, &minx, &miny, &maxx, &maxy);
        SvgParseEntry ne = { handle, image, minx, miny, maxx, maxy };
        m_put(g_parse_cache, &ne);
    }

    /* Rasterise once into a transparent ARGB32 surface. */
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, pixel_w, pixel_h);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(surface);
        return NULL;
    }
    cairo_t *cr = cairo_create(surface);
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        return NULL;
    }

    /* Force a fully transparent background (independent of the widget theme,
     * so the raster never has to be regenerated when the background changes). */
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    /* Crop to the content bounds and fill the surface. */
    cairo_scale(cr, (double)pixel_w / (maxx - minx), (double)pixel_h / (maxy - miny));
    cairo_translate(cr, -minx, -miny);
    render_shapes(cr, image);

    cairo_destroy(cr);
    cairo_surface_flush(surface);

    SvgRasterEntry re = { handle, pixel_w, pixel_h, surface };
    m_put(g_raster_cache, &re);
    TRACE(50, "svg_factory: rasterized %s %dx%d", filename, pixel_w, pixel_h);
    return surface;
}

void svg_factory_destroy(void) {
    if (g_raster_cache > 0) {
        int p; SvgRasterEntry *e;
        m_foreach(g_raster_cache, p, e) {
            if (e->surface) cairo_surface_destroy(e->surface);
        }
        m_free(g_raster_cache);
        g_raster_cache = 0;
    }
    if (g_parse_cache > 0) {
        int p; SvgParseEntry *e;
        m_foreach(g_parse_cache, p, e) {
            if (e->image) nsvgDelete(e->image);
        }
        m_free(g_parse_cache);
        g_parse_cache = 0;
    }
}
