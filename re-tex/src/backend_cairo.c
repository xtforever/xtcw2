#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "backend_cairo.h"


#include "svg_factory.h"
#include "utf8.h"

typedef struct {
    Backend base;
    cairo_surface_t *surface;
    cairo_t *cr;
    const char *filename;
    double font_size;
    const char *font_face;
} BackendCairo;

/* Encode a Unicode code point into a NUL-terminated UTF-8 buffer.
 * Falls back to U+FFFD for invalid scalar values. buf must hold 5 bytes. */
static void encode_utf8(char *buf, int c) {
    int n = utf8_encode_cp((uint32_t)c, buf);
    if (n == 0) n = utf8_encode_cp(0xFFFD, buf);
    if (n == 0) { buf[0] = '?'; n = 1; }
    buf[n] = '\0';
}

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
    char str[5];
    cairo_text_extents_t extents;
    encode_utf8(str, c);

    cairo_text_extents(bc->cr, str, &extents);
    return extents.x_advance;
}

static void backend_cairo_get_char_metrics(Backend *self, int c, CharMetrics *metrics) {
    BackendCairo *bc = (BackendCairo*)self;
    char str[5];
    cairo_text_extents_t extents;
    encode_utf8(str, c);

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
    char str[5];
    encode_utf8(str, c);

    cairo_set_font_size(bc->cr, bc->font_size);
    cairo_move_to(bc->cr, x, y);
    cairo_text_path(bc->cr, str);
    cairo_fill(bc->cr);
}

static void cairo_draw_svg(Backend *self, double x, double y, double w, double h, const char *filename) {
    BackendCairo *bc = (BackendCairo*)self;
    if (!bc->cr) return;

    /* This backend draws in pixels (no DPI scaling), so points == pixels.
     * The factory caches the rasterised SVG per (filename, pixel size); the
     * blit below is the whole per-redraw cost. */
    int pixel_w = (int)(w + 0.5);
    int pixel_h = (int)(h + 0.5);
    if (pixel_w <= 0 || pixel_h <= 0) return;

    cairo_surface_t *raster = svg_factory_get_image(filename, pixel_w, pixel_h);
    if (!raster) return;

    cairo_save(bc->cr);
    cairo_identity_matrix(bc->cr);
    cairo_set_source_surface(bc->cr, raster, x, y);
    cairo_paint(bc->cr);
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
