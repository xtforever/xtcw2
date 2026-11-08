#include <cairo.h>
#include <cairo-xlib.h>
#include <X11/Xresource.h>
#include <stdlib.h>
#include <stdio.h>
#include "backend_xpixmap.h"
#include "svg_factory.h"
#include "utf8.h"

typedef struct {
    Backend base;
    cairo_surface_t *surface;
    cairo_t *cr;
    Display *dpy;
    Pixmap pixmap;
    double font_size;
    const char *font_face;
    double dpi;
    double dpi_factor;
} BackendXPixmap;

/* Encode a Unicode code point into a NUL-terminated UTF-8 buffer.
 * Falls back to U+FFFD for invalid scalar values. buf must hold 5 bytes. */
static void encode_utf8(char *buf, int c) {
    int n = utf8_encode_cp((uint32_t)c, buf);
    if (n == 0) n = utf8_encode_cp(0xFFFD, buf);
    if (n == 0) { buf[0] = '?'; n = 1; }
    buf[n] = '\0';
}

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
    char str[5];
    cairo_text_extents_t extents;
    encode_utf8(str, c);

    cairo_text_extents(bp->cr, str, &extents);
    return extents.x_advance;
}

static void xpixmap_get_char_metrics(Backend *self, int c, CharMetrics *metrics) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (!bp->cr) { metrics->width = metrics->height = metrics->depth = 0; return; }
    char str[5];
    cairo_text_extents_t extents;
    encode_utf8(str, c);

    cairo_text_extents(bp->cr, str, &extents);
    /* Use the font's nominal ascent/descent (the TeX line box), not the
     * glyph's ink extents.  Ink extents describe only this glyph's painted
     * bounds, so a line of text would report a height with no leading and
     * clip against the top/bottom of a label.  The advance width still
     * comes from the glyph's text extents. */
    cairo_font_extents_t font_extents;
    cairo_font_extents(bp->cr, &font_extents);
    metrics->width = extents.x_advance;
    metrics->height = font_extents.ascent;
    metrics->depth = font_extents.descent;
}

static void xpixmap_draw_char(Backend *self, double x, double y, int c) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (!bp->cr) return;
    char str[5];
    encode_utf8(str, c);

    cairo_set_font_size(bp->cr, bp->font_size);
    cairo_move_to(bp->cr, x, y);
    cairo_show_text(bp->cr, str);
}

static void xpixmap_draw_svg(Backend *self, double x, double y, double w, double h, const char *filename) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    if (!bp->cr) return;

    /* The cairo context is scaled by dpi_factor, so w/h are in points; the
     * factory works in device pixels. The cached transparent raster is blitted
     * 1:1 in device space, which is the whole per-redraw cost. */
    double k = bp->dpi_factor > 0 ? bp->dpi_factor : 1.0;
    int pixel_w = (int)(w * k + 0.5);
    int pixel_h = (int)(h * k + 0.5);
    if (pixel_w <= 0 || pixel_h <= 0) return;

    cairo_surface_t *raster = svg_factory_get_image(filename, pixel_w, pixel_h);
    if (!raster) return;

    cairo_save(bp->cr);
    cairo_identity_matrix(bp->cr);
    cairo_set_source_surface(bp->cr, raster, x * k, y * k);
    cairo_paint(bp->cr);
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
    free(bp);
}

static double xpixmap_get_dpi(Backend *self) {
    BackendXPixmap *bp = (BackendXPixmap*)self;
    return bp->dpi;
}

Backend* backend_xpixmap_create(Display *dpy, Window parent, int width, int height) {
    BackendXPixmap *bp = malloc(sizeof(BackendXPixmap));
    if (!bp) return NULL;

    bp->dpy = dpy;

    XWindowAttributes attr;
    if (!XGetWindowAttributes(dpy, parent, &attr)) {
        free(bp);
        return NULL;
    }

    if( width < 3 || width > 4000 || height < 3 || height > 4000 ) {
      free(bp);
      return NULL;
    }
      
    bp->pixmap = XCreatePixmap(dpy, parent, width, height, attr.depth);
    if (!bp->pixmap) {
        free(bp);
        return NULL;
    }

    bp->surface = cairo_xlib_surface_create(dpy, bp->pixmap, attr.visual, width, height);
    if (cairo_surface_status(bp->surface) != CAIRO_STATUS_SUCCESS) {
        XFreePixmap(dpy, bp->pixmap);
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
    bp->dpi_factor = bp->dpi / 72.0;
    cairo_scale(bp->cr, bp->dpi_factor, bp->dpi_factor);

    /* Background initialized to transparent black by default, 
       widgets should clear it themselves. */

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
    bp->base.draw_highlight = xpixmap_draw_rect;
    bp->base.get_dpi = xpixmap_get_dpi;

    return (Backend*)&bp->base;
}

Pixmap backend_xpixmap_get_pixmap(Backend *backend) {
    BackendXPixmap *bp = (BackendXPixmap*)backend;
    return bp->pixmap;
}
