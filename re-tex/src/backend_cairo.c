#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "backend_cairo.h"


#include "nanosvg.h"
#include "mls.h"
#include "m_tool.h"

typedef struct {
    int filename_handle;
    NSVGimage *image;
} SVGCacheEntry;

typedef struct {
    Backend base;
    cairo_surface_t *surface;
    cairo_t *cr;
    const char *filename;
    double font_size;
    const char *font_face;
    int svg_cache; // List of SVGCacheEntry
} BackendCairo;

static void cairo_draw_rect(Backend *self, double x, double y, double w, double h, uint32_t color) {
    BackendCairo *bc = (BackendCairo*)self;
    
    double a = ((color >> 24) & 0xFF) / 255.0;
    double r = ((color >> 16) & 0xFF) / 255.0;
    double g = ((color >> 8) & 0xFF) / 255.0;
    double b = (color & 0xFF) / 255.0;

    cairo_set_source_rgba(bc->cr, r, g, b, a);
    cairo_rectangle(bc->cr, x, y, w, h);
    cairo_fill(bc->cr);
}

static void cairo_set_color(Backend *self, uint32_t color) {
    BackendCairo *bc = (BackendCairo*)self;
    
    double a = ((color >> 24) & 0xFF) / 255.0;
    double r = ((color >> 16) & 0xFF) / 255.0;
    double g = ((color >> 8) & 0xFF) / 255.0;
    double b = (color & 0xFF) / 255.0;

    cairo_set_source_rgba(bc->cr, r, g, b, a);
}

static void backend_cairo_set_font_size(Backend *self, double size) {
    BackendCairo *bc = (BackendCairo*)self;
    bc->font_size = size;
    cairo_set_font_size(bc->cr, size);
}

static void backend_cairo_set_font_face(Backend *self, const char *face, int style) {
    BackendCairo *bc = (BackendCairo*)self;
    bc->font_face = face;
    cairo_font_slant_t slant = (style & 2) ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL;
    cairo_font_weight_t weight = (style & 1) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;
    cairo_select_font_face(bc->cr, face ? face : "Sans", slant, weight);
}

static double backend_cairo_get_char_width(Backend *self, int c) {
    BackendCairo *bc = (BackendCairo*)self;
    char str[5] = {0};
    str[0] = (char)c;
    cairo_text_extents_t extents;
    
    cairo_text_extents(bc->cr, str, &extents);
    return extents.x_advance;
}

static void backend_cairo_get_char_metrics(Backend *self, int c, CharMetrics *metrics) {
    BackendCairo *bc = (BackendCairo*)self;
    char str[5] = {0};
    str[0] = (char)c;
    cairo_text_extents_t extents;
    
    cairo_text_extents(bc->cr, str, &extents);
    /* Use the font's nominal ascent/descent (the TeX line box), not the
     * glyph's ink extents.  See backend_xpixmap.c for rationale.  The
     * advance width still comes from the glyph's text extents. */
    cairo_font_extents_t font_extents;
    cairo_font_extents(bc->cr, &font_extents);
    metrics->width = extents.x_advance;
    metrics->height = font_extents.ascent;
    metrics->depth = font_extents.descent;
}

static void cairo_draw_char(Backend *self, double x, double y, int c) {
    BackendCairo *bc = (BackendCairo*)self;
    char str[5] = {0};
    str[0] = (char)c;

    cairo_set_font_size(bc->cr, bc->font_size);
    cairo_move_to(bc->cr, x, y);
    cairo_text_path(bc->cr, str);
    cairo_fill(bc->cr);
}

static void cairo_set_source_nsvg(cairo_t *cr, NSVGpaint *p, float opacity) {
    if (p->type == NSVG_PAINT_COLOR) {
        uint32_t c = p->color;
        cairo_set_source_rgba(cr, ((c>>16) & 0xff)/255.0, ((c>>8) & 0xff)/255.0, (c & 0xff)/255.0, ((c>>24) & 0xff)/255.0 * opacity);
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
            cairo_pattern_add_color_stop_rgba(pat, grad->stops[i].offset, ((c>>16) & 0xff)/255.0, ((c>>8) & 0xff)/255.0, (c & 0xff)/255.0, ((c>>24) & 0xff)/255.0 * opacity);
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

static void cairo_draw_svg(Backend *self, double x, double y, double w, double h, const char *filename) {
    BackendCairo *bc = (BackendCairo*)self;
    NSVGimage *image = NULL;
    int filename_handle = conststr_lookup_c(filename);

    int p; SVGCacheEntry *entry;
    m_foreach(bc->svg_cache, p, entry) {
        if (entry->filename_handle == filename_handle) {
            image = entry->image;
            break;
        }
    }

    if (!image) {
        image = nsvgParseFromFile(filename, "px", 96.0f);
        if (image) {
            SVGCacheEntry new_entry = {filename_handle, image};
            m_put(bc->svg_cache, &new_entry);
        }
    }

    if (!image) {
        fprintf(stderr, "Could not open SVG image: %s\n", filename);
        return;
    }

    cairo_save(bc->cr);
    cairo_translate(bc->cr, x, y);
    double scale_x = w / image->width;
    double scale_y = h / image->height;
    cairo_scale(bc->cr, scale_x, scale_y);

    for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
        if (!(shape->flags & NSVG_FLAGS_VISIBLE)) continue;

        cairo_new_path(bc->cr);
        for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
            cairo_move_to(bc->cr, path->pts[0], path->pts[1]);
            for (int i = 0; i < path->npts - 1; i += 3) {
                float *p = &path->pts[i * 2];
                cairo_curve_to(bc->cr, p[2], p[3], p[4], p[5], p[6], p[7]);
            }
            if (path->closed) cairo_close_path(bc->cr);
        }

        if (shape->fill.type != NSVG_PAINT_NONE) {
            cairo_set_fill_rule(bc->cr, shape->fillRule == NSVG_FILLRULE_EVENODD ? CAIRO_FILL_RULE_EVEN_ODD : CAIRO_FILL_RULE_WINDING);
            cairo_set_source_nsvg(bc->cr, &shape->fill, shape->opacity);
            if (shape->stroke.type != NSVG_PAINT_NONE) cairo_fill_preserve(bc->cr);
            else cairo_fill(bc->cr);
        }
        
        if (shape->stroke.type != NSVG_PAINT_NONE) {
            cairo_set_source_nsvg(bc->cr, &shape->stroke, shape->opacity);
            cairo_set_line_width(bc->cr, shape->strokeWidth);
            cairo_set_line_cap(bc->cr, (cairo_line_cap_t)shape->strokeLineCap);
            cairo_set_line_join(bc->cr, (cairo_line_join_t)shape->strokeLineJoin);
            cairo_stroke(bc->cr);
        } else {
            cairo_new_path(bc->cr);
        }
    }

    cairo_restore(bc->cr);
}

static void cairo_set_clip(Backend *self, double x, double y, double w, double h, int active) {
    BackendCairo *bc = (BackendCairo*)self;
    if (active) {
        cairo_reset_clip(bc->cr);
        cairo_rectangle(bc->cr, x, y, w, h);
        cairo_clip(bc->cr);
    } else {
        cairo_reset_clip(bc->cr);
    }
}

static void backend_cairo_flush(Backend *self) {
    BackendCairo *bc = (BackendCairo*)self;
    if (bc->surface) {
        cairo_surface_flush(bc->surface);
    }
}

static void backend_cairo_destroy(Backend *self) {
    BackendCairo *bc = (BackendCairo*)self;
    if (bc->cr) {
        cairo_destroy(bc->cr);
    }
    if (bc->surface) {
        cairo_surface_write_to_png(bc->surface, bc->filename);
        cairo_surface_destroy(bc->surface);
    }
    if (bc->svg_cache > 0) {
        int p; SVGCacheEntry *entry;
        m_foreach(bc->svg_cache, p, entry) {
            nsvgDelete(entry->image);
        }
        m_free(bc->svg_cache);
        bc->svg_cache = 0;
    }
    free(bc);
}

Backend* backend_cairo_create(const char* filename, int width, int height) {
    BackendCairo *bc = malloc(sizeof(BackendCairo));
    if (!bc) return NULL;

    bc->filename = filename;
    bc->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status(bc->surface) != CAIRO_STATUS_SUCCESS) {
        free(bc);
        return NULL;
    }

    bc->cr = cairo_create(bc->surface);
    bc->font_size = 10.0;
    bc->font_face = "Sans";
    bc->svg_cache = m_create(5, sizeof(SVGCacheEntry));

    // Fill background with white
    cairo_set_source_rgb(bc->cr, 1, 1, 1);
    cairo_paint(bc->cr);

    bc->base.ctx = bc->cr;
    bc->base.draw_rect = cairo_draw_rect;
    bc->base.set_color = cairo_set_color;
    bc->base.draw_char = cairo_draw_char;
    bc->base.draw_svg = cairo_draw_svg;
    bc->base.set_font_size = backend_cairo_set_font_size;
    bc->base.set_font_face = backend_cairo_set_font_face;
    bc->base.get_char_width = backend_cairo_get_char_width;
    bc->base.get_char_metrics = backend_cairo_get_char_metrics;
    bc->base.flush = backend_cairo_flush;
    bc->base.destroy = backend_cairo_destroy;
    bc->base.set_clip = cairo_set_clip;
    bc->base.draw_highlight = cairo_draw_rect;

    return (Backend*)&bc->base;
}
