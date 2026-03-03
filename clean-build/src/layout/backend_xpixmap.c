#include <cairo.h>
#include <cairo-xlib.h>
#include <X11/Xresource.h>
#include <stdlib.h>
#include <stdio.h>
#include "backend_xpixmap.h"
#include "mls.h"
#include "m_tool.h"
#include "nanosvg.h"

typedef struct {
    int filename_handle;
    NSVGimage *image;
} SVGCacheEntry;

typedef struct {
    Backend base;
    cairo_surface_t *surface;
    cairo_t *cr;
    Display *dpy;
    Pixmap pixmap;
    double font_size;
    const char *font_face;
    double dpi;
    int svg_cache; // List of SVGCacheEntry
} BackendXPixmap;

static void xpixmap_draw_rect(Backend *self, double x, double y, double w, double h, uint32_t color) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (!bp->cr) return;
    
    double a = ((color >> 24) & 0xFF) / 255.0;
    double r = ((color >> 16) & 0xFF) / 255.0;
    double g = ((color >> 8) & 0xFF) / 255.0;
    double b = (color & 0xFF) / 255.0;

    cairo_set_source_rgba(bp->cr, r, g, b, a);
    cairo_rectangle(bp->cr, x, y, w, h);
    cairo_fill(bp->cr);
}

static void xpixmap_set_color(Backend *self, uint32_t color) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (!bp->cr) return;
    
    double a = ((color >> 24) & 0xFF) / 255.0;
    double r = ((color >> 16) & 0xFF) / 255.0;
    double g = ((color >> 8) & 0xFF) / 255.0;
    double b = (color & 0xFF) / 255.0;

    cairo_set_source_rgba(bp->cr, r, g, b, a);
}

static void xpixmap_set_font_size(Backend *self, double size) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (!bp->cr) return;
    bp->font_size = size;
    cairo_set_font_size(bp->cr, size);
}

static void xpixmap_set_font_face(Backend *self, const char *face, int style) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (!bp->cr) return;
    bp->font_face = face;
    cairo_font_slant_t slant = (style & 2) ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL;
    cairo_font_weight_t weight = (style & 1) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;
    cairo_select_font_face(bp->cr, face ? face : "Sans", slant, weight);
}

static double xpixmap_get_char_width(Backend *self, int c) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (!bp->cr) return 0;
    char str[5] = {0};
    if (c < 0 || c > 255) c = '?';
    str[0] = (char)c;
    cairo_text_extents_t extents;
    
    cairo_text_extents(bp->cr, str, &extents);
    return extents.x_advance;
}

static void xpixmap_get_char_metrics(Backend *self, int c, CharMetrics *metrics) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (!bp->cr) { metrics->width = metrics->height = metrics->depth = 0; return; }
    char str[5] = {0};
    if (c < 0 || c > 255) c = '?';
    str[0] = (char)c;
    cairo_text_extents_t extents;
    
    cairo_text_extents(bp->cr, str, &extents);
    metrics->width = extents.x_advance;
    metrics->height = -extents.y_bearing;
    metrics->depth = extents.height + extents.y_bearing;
}

static void xpixmap_draw_char(Backend *self, double x, double y, int c) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (!bp->cr) return;
    char str[5] = {0};
    if (c < 0 || c > 255) c = '?';
    str[0] = (char)c;

    cairo_move_to(bp->cr, x, y);
    cairo_show_text(bp->cr, str);
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

static void xpixmap_draw_svg(Backend *self, double x, double y, double w, double h, const char *filename) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    NSVGimage *image = NULL;
    int filename_handle = conststr_lookup_c(filename);

    int p; SVGCacheEntry *entry;
    m_foreach(bp->svg_cache, p, entry) {
        if (entry->filename_handle == filename_handle) {
            image = entry->image;
            break;
        }
    }

    if (!image) {
        image = nsvgParseFromFile(filename, "px", 96.0f);
        if (image) {
            SVGCacheEntry new_entry = {filename_handle, image};
            m_put(bp->svg_cache, &new_entry);
        }
    }

    if (!image) {
        fprintf(stderr, "Could not open SVG image: %s\n", filename);
        return;
    }

    cairo_save(bp->cr);
    cairo_translate(bp->cr, x, y);
    double scale_x = w / image->width;
    double scale_y = h / image->height;
    cairo_scale(bp->cr, scale_x, scale_y);

    for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
        if (!(shape->flags & NSVG_FLAGS_VISIBLE)) continue;

        cairo_new_path(bp->cr);
        for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
            cairo_move_to(bp->cr, path->pts[0], path->pts[1]);
            for (int i = 0; i < path->npts - 1; i += 3) {
                float *p = &path->pts[i * 2];
                cairo_curve_to(bp->cr, p[2], p[3], p[4], p[5], p[6], p[7]);
            }
            if (path->closed) cairo_close_path(bp->cr);
        }

        if (shape->fill.type != NSVG_PAINT_NONE) {
            cairo_set_fill_rule(bp->cr, shape->fillRule == NSVG_FILLRULE_EVENODD ? CAIRO_FILL_RULE_EVEN_ODD : CAIRO_FILL_RULE_WINDING);
            cairo_set_source_nsvg(bp->cr, &shape->fill, shape->opacity);
            if (shape->stroke.type != NSVG_PAINT_NONE) cairo_fill_preserve(bp->cr);
            else cairo_fill(bp->cr);
        }
        
        if (shape->stroke.type != NSVG_PAINT_NONE) {
            cairo_set_source_nsvg(bp->cr, &shape->stroke, shape->opacity);
            cairo_set_line_width(bp->cr, shape->strokeWidth);
            cairo_set_line_cap(bp->cr, (cairo_line_cap_t)shape->strokeLineCap);
            cairo_set_line_join(bp->cr, (cairo_line_join_t)shape->strokeLineJoin);
            cairo_stroke(bp->cr);
        } else {
            cairo_new_path(bp->cr);
        }
    }

    cairo_restore(bp->cr);
}

static void xpixmap_set_clip(Backend *self, double x, double y, double w, double h, int active) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (active) {
        cairo_reset_clip(bp->cr);
        cairo_rectangle(bp->cr, x, y, w, h);
        cairo_clip(bp->cr);
    } else {
        cairo_reset_clip(bp->cr);
    }
}

static void xpixmap_flush(Backend *self) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (bp->surface) {
        cairo_surface_flush(bp->surface);
    }
}

static void xpixmap_destroy(Backend *self) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (bp->cr) {
        cairo_destroy(bp->cr);
    }
    if (bp->surface) {
        cairo_surface_destroy(bp->surface);
    }
    if (bp->pixmap) {
        XFreePixmap(bp->dpy, bp->pixmap);
    }
    if (bp->svg_cache > 0) {
        int p; SVGCacheEntry *entry;
        m_foreach(bp->svg_cache, p, entry) {
            nsvgDelete(entry->image);
        }
        m_free(bp->svg_cache);
        bp->svg_cache = 0;
    }
    free(bp);
}

Backend* backend_xpixmap_create(Display *dpy, Window parent, int width, int height) {
    BackendXPixmap *bp = malloc(sizeof(BackendXPixmap));
    if (!bp) return NULL;

    bp->dpy = dpy;
    bp->svg_cache = m_create(5, sizeof(SVGCacheEntry));
    
    XWindowAttributes attr;
    if (!XGetWindowAttributes(dpy, parent, &attr)) {
        m_free(bp->svg_cache);
        free(bp);
        return NULL;
    }

    if( width < 3 || width > 4000 || height < 3 || height > 4000 ) {
      m_free(bp->svg_cache);
      free(bp);
      return NULL;
    }
      
    bp->pixmap = XCreatePixmap(dpy, parent, width, height, attr.depth);
    if (!bp->pixmap) {
        m_free(bp->svg_cache);
        free(bp);
        return NULL;
    }

    bp->surface = cairo_xlib_surface_create(dpy, bp->pixmap, attr.visual, width, height);
    if (cairo_surface_status(bp->surface) != CAIRO_STATUS_SUCCESS) {
        XFreePixmap(dpy, bp->pixmap);
        m_free(bp->svg_cache);
        free(bp);
        return NULL;
    }

    bp->cr = cairo_create(bp->surface);
    bp->font_size = 10.0;
    bp->font_face = "Sans";
    
    char *s = XGetDefault(dpy, "Xft", "dpi");
    if (s) {
        bp->dpi = atof(s);
        if (bp->dpi < 1.0 || bp->dpi > 1000.0) bp->dpi = 96.0;
    } else {
        bp->dpi = 96.0;
    }

    cairo_scale(bp->cr, bp->dpi / 72.0, bp->dpi / 72.0);

    cairo_set_source_rgb(bp->cr, 1, 1, 1);
    cairo_paint(bp->cr);

    bp->base.ctx = bp->cr;
    bp->base.draw_rect = xpixmap_draw_rect;
    bp->base.set_color = xpixmap_set_color;
    bp->base.draw_char = xpixmap_draw_char;
    bp->base.draw_svg = xpixmap_draw_svg;
    bp->base.set_font_size = xpixmap_set_font_size;
    bp->base.set_font_face = xpixmap_set_font_face;
    bp->base.get_char_width = xpixmap_get_char_width;
    bp->base.get_char_metrics = xpixmap_get_char_metrics;
    bp->base.flush = xpixmap_flush;
    bp->base.destroy = xpixmap_destroy;
    bp->base.set_clip = xpixmap_set_clip;

    return (Backend*)&bp->base;
}

Pixmap backend_xpixmap_get_pixmap(Backend *backend) {
    BackendXPixmap *bp = (BackendXPixmap*)backend;
    return bp->pixmap;
}
