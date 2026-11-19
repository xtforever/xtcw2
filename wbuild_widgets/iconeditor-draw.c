/*
 * IconEditor plug-in for DrawCanvas.
 *
 * The editor keeps a small RGB pixel grid (0 = transparent).  The main
 * DrawCanvas renders the grid scaled to its current window; a second
 * DrawCanvas renders a preview.  Actions let the Lua UI change the tool,
 * colour and icon size, undo/clear and save/load XPM files.
 */
#include "drawcanvas-draw.h"
#include "iconeditor-draw.h"
#include "wcreg2.h"
#include "xt_error.h"
#include <X11/Xft/Xft.h>
#include <X11/keysym.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DIM 64
#define MIN_DIM 4
#define MAX_UNDO 32
#define MAX_COLORS 64
#define MAX_XPM_STRINGS 8192

enum {
    TOOL_PENCIL = 0,
    TOOL_LINE,
    TOOL_RECT,
    TOOL_ELLIPSE,
    TOOL_FILL
};

typedef struct undo_frame {
    int w, h;
    uint32_t *pix;
} undo_frame;

static int iw = 32, ih = 32;
static uint32_t *pix;
static uint32_t cur_color = 0x000000;
static int tool = TOOL_PENCIL;
static int dragging;
static int drag_button;          /* Button1 draws, Button3 erases */
static int erasing;              /* write transparent pixels instead of cur_color */
static int start_cx, start_cy, last_cx, last_cy, cur_cx, cur_cy;
static Widget grid_w, preview_w, swatch_w;
static GC paint_gc;
static undo_frame undo[MAX_UNDO];
static int undo_count;

/* Bounding box of icon cells touched since the last repaint.  Only the cells
 * inside it are redrawn, so setting a pixel does not repaint the whole
 * canvas (which caused visible flicker on non-buffered/remote displays). */
static int dmg_x0, dmg_y0, dmg_x1, dmg_y1;
static int dmg_valid;

/* Canvas geometry cached by the last draw callback, used to translate the
 * icon damage box into widget coordinates. */
static int grid_ww, grid_hh;
static int preview_ww, preview_hh;

static void damage_add(int x, int y);
static void canvas_geometry(int ww, int hh, int *cell, int *ox, int *oy);

static const char xpm_chars[] =
    ".+*#@%&$abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/* --- colour cache ------------------------------------------------------- */
typedef struct color_cache {
    uint32_t rgb;
    Pixel pixel;
} color_cache;

static color_cache colors[256];
static int color_count;

static Pixel color_pixel(draw_canvas_t *c, uint32_t rgb)
{
    int i;
    XColor xc;
    for (i = 0; i < color_count; i++)
        if (colors[i].rgb == rgb) return colors[i].pixel;
    xc.red   = (unsigned short)(((rgb >> 16) & 0xff) * 257);
    xc.green = (unsigned short)(((rgb >> 8) & 0xff) * 257);
    xc.blue  = (unsigned short)((rgb & 0xff) * 257);
    xc.flags = DoRed | DoGreen | DoBlue;
    if (!XAllocColor(c->dpy, DefaultColormap(c->dpy, c->screen), &xc))
        return BlackPixel(c->dpy, c->screen);
    if (color_count < (int)(sizeof(colors) / sizeof(colors[0]))) {
        colors[color_count].rgb = rgb;
        colors[color_count].pixel = xc.pixel;
        color_count++;
    }
    return xc.pixel;
}

/* --- model -------------------------------------------------------------- */
static void ensure_pix(void)
{
    if (!pix) {
        pix = (uint32_t *)calloc((size_t)iw * (size_t)ih, sizeof(uint32_t));
        if (!pix) {
            iw = 1;
            ih = 1;
            pix = (uint32_t *)calloc(1, sizeof(uint32_t));
        }
    }
}

static void free_undo(void)
{
    int i;
    for (i = 0; i < undo_count; i++) {
        free(undo[i].pix);
        undo[i].pix = NULL;
    }
    undo_count = 0;
}

static void push_undo(void)
{
    ensure_pix();
    if (undo_count >= MAX_UNDO) {
        free(undo[0].pix);
        memmove(&undo[0], &undo[1], sizeof(undo[0]) * (MAX_UNDO - 1));
        undo_count = MAX_UNDO - 1;
    }
    undo[undo_count].w = iw;
    undo[undo_count].h = ih;
    undo[undo_count].pix = (uint32_t *)malloc((size_t)iw * (size_t)ih *
                                              sizeof(uint32_t));
    if (!undo[undo_count].pix) return;
    memcpy(undo[undo_count].pix, pix, (size_t)iw * (size_t)ih *
           sizeof(uint32_t));
    undo_count++;
}

static void set_pixel(int x, int y, uint32_t rgb)
{
    if (!pix || x < 0 || y < 0 || x >= iw || y >= ih) return;
    /* 0 means transparent; opaque black is 0xFF000000.  The right mouse
     * button erases by writing the transparent sentinel. */
    if (erasing)
        pix[(size_t)y * (size_t)iw + (size_t)x] = 0;
    else
        pix[(size_t)y * (size_t)iw + (size_t)x] =
            0xFF000000u | (rgb & 0xFFFFFFu);
    damage_add(x, y);
}

static void raster_line(int x0, int y0, int x1, int y1, uint32_t rgb)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        set_pixel(x0, y0, rgb);
        if (x0 == x1 && y0 == y1) break;
        {
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
}

static void raster_rect(int x0, int y0, int x1, int y1, uint32_t rgb)
{
    int x, y;
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
    for (x = x0; x <= x1; x++) {
        set_pixel(x, y0, rgb);
        set_pixel(x, y1, rgb);
    }
    for (y = y0; y <= y1; y++) {
        set_pixel(x0, y, rgb);
        set_pixel(x1, y, rgb);
    }
}

static void raster_ellipse(int x0, int y0, int x1, int y1, uint32_t rgb)
{
    int a, b, b1;
    long dx, dy, err, e2;
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
    a = x1 - x0;
    b = y1 - y0;
    if (a < 1 || b < 1) {
        raster_line(x0, y0, x1, y1, rgb);
        return;
    }
    b1 = b & 1;
    dx = 4L * (1 - a) * b * b;
    dy = 4L * (b1 + 1) * a * a;
    err = dx + dy + b1 * a * a;
    if (x0 > x1) { x0 = x1; x1 += a; }
    if (y0 > y1) y0 = y1;
    y0 += (b + 1) / 2;
    y1 = y0 - b1;
    a *= 8 * a;
    b1 = 8 * b * b;
    do {
        set_pixel(x1, y0, rgb);
        set_pixel(x0, y0, rgb);
        set_pixel(x0, y1, rgb);
        set_pixel(x1, y1, rgb);
        e2 = 2 * err;
        if (e2 <= dy) { y0++; y1--; err += dy += a; }
        if (e2 >= dx || 2 * err > dy) { x0++; x1--; err += dx += b1; }
    } while (x0 <= x1);
    while (y0 - y1 < b) {
        set_pixel(x0 - 1, y0, rgb);
        set_pixel(x1 + 1, y0++, rgb);
        set_pixel(x0 - 1, y1, rgb);
        set_pixel(x1 + 1, y1--, rgb);
    }
}

static void flood_fill(int x, int y, uint32_t rgb)
{
    int n = iw * ih;
    int *stack;
    int sp = 0;
    uint32_t target, fill;
    if (!pix || x < 0 || y < 0 || x >= iw || y >= ih) return;
    target = pix[(size_t)y * (size_t)iw + (size_t)x];
    fill = erasing ? 0 : (0xFF000000u | (rgb & 0xFFFFFFu));
    if (target == fill) return;
    stack = (int *)malloc((size_t)n * 4 * sizeof(int));
    if (!stack) return;
    stack[sp++] = y * iw + x;
    while (sp > 0) {
        int idx = stack[--sp];
        int cx = idx % iw, cy = idx / iw;
        if (pix[idx] != target) continue;
        pix[idx] = fill;
        damage_add(cx, cy);
        if (cx > 0) stack[sp++] = idx - 1;
        if (cx + 1 < iw) stack[sp++] = idx + 1;
        if (cy > 0) stack[sp++] = idx - iw;
        if (cy + 1 < ih) stack[sp++] = idx + iw;
    }
    free(stack);
}

/* --- notifications ------------------------------------------------------ */
static void mark_dirty(Widget w)
{
    if (w) XtCallActionProc(w, "mark_dirty", NULL, NULL, 0);
}

/* Ask a DrawCanvas to repaint only the given widget rectangle. */
static void mark_area(Widget w, int x, int y, int ww, int hh)
{
    String params[4];
    char a[16], b[16], c[16], d[16];
    if (!w || ww <= 0 || hh <= 0) return;
    snprintf(a, sizeof a, "%d", x);
    snprintf(b, sizeof b, "%d", y);
    snprintf(c, sizeof c, "%d", ww);
    snprintf(d, sizeof d, "%d", hh);
    params[0] = a;
    params[1] = b;
    params[2] = c;
    params[3] = d;
    XtCallActionProc(w, "mark_dirty_area", NULL, params, 4);
}

static void damage_reset(void)
{
    dmg_valid = 0;
}

static void damage_add(int x, int y)
{
    if (x < 0 || y < 0 || x >= iw || y >= ih) return;
    if (!dmg_valid) {
        dmg_x0 = dmg_x1 = x;
        dmg_y0 = dmg_y1 = y;
        dmg_valid = 1;
        return;
    }
    if (x < dmg_x0) dmg_x0 = x;
    if (x > dmg_x1) dmg_x1 = x;
    if (y < dmg_y0) dmg_y0 = y;
    if (y > dmg_y1) dmg_y1 = y;
}

/* Map the icon-cell damage box to widget coordinates and mark it.  A small
 * margin covers the grid lines and the one-pixel outline. */
static void damage_widget(Widget w, int cell, int ox, int oy,
                          int ww, int hh)
{
    int x, y, x2, y2;
    if (!w || ww <= 0 || hh <= 0) {
        mark_dirty(w);
        return;
    }
    x = ox + dmg_x0 * cell - 2;
    y = oy + dmg_y0 * cell - 2;
    x2 = ox + (dmg_x1 + 1) * cell + 2;
    y2 = oy + (dmg_y1 + 1) * cell + 2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x2 > ww) x2 = ww;
    if (y2 > hh) y2 = hh;
    if (x2 <= x || y2 <= y) {
        mark_dirty(w);
        return;
    }
    mark_area(w, x, y, x2 - x, y2 - y);
}

static void repaint_only(void)
{
    mark_dirty(grid_w);
    mark_dirty(preview_w);
    damage_reset();
}

/* Pointer edits bump the DrawCanvas edit counter; the Lua UI polls it so the
 * plug-in never calls back into Lua from a mouse callback. */
static void document_changed(void)
{
    if (grid_w) {
        int n = 0;
        XtVaGetValues(grid_w, "changeCount", &n, NULL);
        XtVaSetValues(grid_w, "changeCount", n + 1, NULL);
    }
    if (dragging && tool != TOOL_PENCIL && tool != TOOL_FILL) {
        /* The rubber-band overlay can span cells outside the committed
         * pixels, so it needs a full repaint. */
        mark_dirty(grid_w);
        mark_dirty(preview_w);
    } else if (dmg_valid) {
        int cell, ox, oy;
        canvas_geometry(grid_ww, grid_hh, &cell, &ox, &oy);
        damage_widget(grid_w, cell, ox, oy, grid_ww, grid_hh);
        canvas_geometry(preview_ww, preview_hh, &cell, &ox, &oy);
        damage_widget(preview_w, cell, ox, oy, preview_ww, preview_hh);
    } else {
        repaint_only();
    }
    damage_reset();
}

/* --- geometry ----------------------------------------------------------- */
static void canvas_geometry(int ww, int hh, int *cell, int *ox, int *oy)
{
    int margin = 8;
    int cw = (ww - 2 * margin) / (iw > 0 ? iw : 1);
    int ch = (hh - 2 * margin) / (ih > 0 ? ih : 1);
    int s = cw < ch ? cw : ch;
    if (s < 1) s = 1;
    *cell = s;
    *ox = (ww - s * iw) / 2;
    *oy = (hh - s * ih) / 2;
    if (*ox < 0) *ox = 0;
    if (*oy < 0) *oy = 0;
}

static void grid_geometry(draw_canvas_t *c, int *cell, int *ox, int *oy)
{
    int ww = c->win_width ? (int)c->win_width : 1;
    int hh = c->win_height ? (int)c->win_height : 1;
    canvas_geometry(ww, hh, cell, ox, oy);
}

static void preview_geometry(draw_canvas_t *c, int *scale, int *ox, int *oy)
{
    int ww = c->win_width ? (int)c->win_width : 1;
    int hh = c->win_height ? (int)c->win_height : 1;
    canvas_geometry(ww, hh, scale, ox, oy);
}

static int map_cell(draw_canvas_t *c, int x, int y, int *cx, int *cy)
{
    int cell, ox, oy, ix, iy;
    grid_geometry(c, &cell, &ox, &oy);
    if (x < ox || y < oy) return 0;
    ix = (x - ox) / cell;
    iy = (y - oy) / cell;
    if (ix < 0 || iy < 0 || ix >= iw || iy >= ih) return 0;
    *cx = ix;
    *cy = iy;
    return 1;
}

/* --- drawing ------------------------------------------------------------ */
static GC ensure_gc(draw_canvas_t *c, Drawable d)
{
    if (!paint_gc) paint_gc = XCreateGC(c->dpy, d, 0, NULL);
    return paint_gc;
}

static void draw_checker(draw_canvas_t *c, Drawable d, GC gc,
                         int cell, int ox, int oy,
                         int x0, int y0, int x1, int y1)
{
    Pixel light = color_pixel(c, 0xffffff);
    Pixel dark = color_pixel(c, 0xd0d0d0);
    int x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++) {
            XSetForeground(c->dpy, gc, ((x + y) & 1) ? dark : light);
            XFillRectangle(c->dpy, d, gc, ox + x * cell, oy + y * cell,
                           (unsigned int)cell, (unsigned int)cell);
        }
    }
}

void iconeditor_draw_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)user;
    draw_canvas_t *c = (draw_canvas_t *)class;
    Drawable d;
    GC gc;
    int ww, hh, cell, ox, oy, x, y;
    int cx0, cy0, cx1, cy1, dx, dy, dw, dh;
    Boolean full;

    if (!c || !c->xdraw) return;
    grid_w = w;
    ensure_pix();
    d = XftDrawDrawable(c->xdraw);
    ww = c->win_width ? (int)c->win_width : 1;
    hh = c->win_height ? (int)c->win_height : 1;
    grid_ww = ww;
    grid_hh = hh;
    gc = ensure_gc(c, d);
    grid_geometry(c, &cell, &ox, &oy);

    full = c->damage_full || c->damage_w <= 0 || c->damage_h <= 0;
    dx = full ? 0 : c->damage_x;
    dy = full ? 0 : c->damage_y;
    dw = full ? ww : c->damage_w;
    dh = full ? hh : c->damage_h;
    if (dx < 0) { dw += dx; dx = 0; }
    if (dy < 0) { dh += dy; dy = 0; }
    if (dx + dw > ww) dw = ww - dx;
    if (dy + dh > hh) dh = hh - dy;
    if (dw <= 0 || dh <= 0) {
        full = True;
        dx = dy = 0;
        dw = ww;
        dh = hh;
    }

    if (full) {
        XFillRectangle(c->dpy, d, c->gc[1], 0, 0,
                       (unsigned int)ww, (unsigned int)hh);
        cx0 = 0; cy0 = 0;
        cx1 = iw - 1; cy1 = ih - 1;
        draw_checker(c, d, gc, cell, ox, oy, cx0, cy0, cx1, cy1);
    } else {
        XRectangle clip;
        clip.x = (short)dx;
        clip.y = (short)dy;
        clip.width = (unsigned short)dw;
        clip.height = (unsigned short)dh;
        XSetClipRectangles(c->dpy, gc, 0, 0, &clip, 1, Unsorted);
        XFillRectangle(c->dpy, d, c->gc[1], dx, dy,
                       (unsigned int)dw, (unsigned int)dh);
        cx0 = (dx - ox) / cell;
        cy0 = (dy - oy) / cell;
        cx1 = (dx + dw - 1 - ox) / cell;
        cy1 = (dy + dh - 1 - oy) / cell;
        if (cx0 < 0) cx0 = 0;
        if (cy0 < 0) cy0 = 0;
        if (cx1 >= iw) cx1 = iw - 1;
        if (cy1 >= ih) cy1 = ih - 1;
        if (cx0 > cx1 || cy0 > cy1) {
            /* damage lies outside the icon grid */
            cx0 = cy0 = 0;
            cx1 = cy1 = -1;
        }
        if (cx0 <= cx1 && cy0 <= cy1)
            draw_checker(c, d, gc, cell, ox, oy, cx0, cy0, cx1, cy1);
    }

    for (y = cy0; y <= cy1; y++) {
        for (x = cx0; x <= cx1; x++) {
            uint32_t rgb = pix[(size_t)y * (size_t)iw + (size_t)x];
            if (!rgb) continue;
            XSetForeground(c->dpy, gc, color_pixel(c, rgb & 0xFFFFFFu));
            XFillRectangle(c->dpy, d, gc, ox + x * cell, oy + y * cell,
                           (unsigned int)cell, (unsigned int)cell);
        }
    }

    if (cell >= 4) {
        XSetForeground(c->dpy, gc, color_pixel(c, 0x808080));
        for (x = cx0; x <= cx1 + 1; x++) {
            if (x < 0 || x > iw) continue;
            XDrawLine(c->dpy, d, gc, ox + x * cell, oy,
                      ox + x * cell, oy + ih * cell);
        }
        for (y = cy0; y <= cy1 + 1; y++) {
            if (y < 0 || y > ih) continue;
            XDrawLine(c->dpy, d, gc, ox, oy + y * cell,
                      ox + iw * cell, oy + y * cell);
        }
    }
    XSetForeground(c->dpy, gc, color_pixel(c, 0x000000));
    XDrawRectangle(c->dpy, d, gc, ox - 1, oy - 1,
                   (unsigned int)(iw * cell + 1), (unsigned int)(ih * cell + 1));
    if (!full)
        XSetClipMask(c->dpy, gc, None);

    if (dragging && tool != TOOL_PENCIL && tool != TOOL_FILL) {
        int x0 = ox + start_cx * cell, y0 = oy + start_cy * cell;
        int x1 = ox + cur_cx * cell, y1 = oy + cur_cy * cell;
        int xa = x0 < x1 ? x0 : x1, ya = y0 < y1 ? y0 : y1;
        int xb = x0 > x1 ? x0 : x1, yb = y0 > y1 ? y0 : y1;
        XSetForeground(c->dpy, gc, color_pixel(c, cur_color));
        if (tool == TOOL_LINE) {
            XDrawLine(c->dpy, d, gc, x0 + cell / 2, y0 + cell / 2,
                      x1 + cell / 2, y1 + cell / 2);
        } else if (tool == TOOL_RECT) {
            XDrawRectangle(c->dpy, d, gc, xa, ya,
                           (unsigned int)(xb - xa + cell),
                           (unsigned int)(yb - ya + cell));
        } else if (tool == TOOL_ELLIPSE) {
            XDrawArc(c->dpy, d, gc, xa, ya,
                     (unsigned int)(xb - xa + cell),
                     (unsigned int)(yb - ya + cell), 0, 360 * 64);
        }
    }
}

void iconeditor_swatch_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)user;
    draw_canvas_t *c = (draw_canvas_t *)class;
    Drawable d;
    GC gc;
    int ww, hh;
    if (!c || !c->xdraw) return;
    swatch_w = w;
    d = XftDrawDrawable(c->xdraw);
    ww = c->win_width ? (int)c->win_width : 1;
    hh = c->win_height ? (int)c->win_height : 1;
    gc = ensure_gc(c, d);
    XFillRectangle(c->dpy, d, c->gc[1], 0, 0,
                   (unsigned int)ww, (unsigned int)hh);
    XSetForeground(c->dpy, gc, color_pixel(c, cur_color));
    XFillRectangle(c->dpy, d, gc, 2, 2,
                   (unsigned int)(ww > 4 ? ww - 4 : 1),
                   (unsigned int)(hh > 4 ? hh - 4 : 1));
    XSetForeground(c->dpy, gc, color_pixel(c, 0xffffff));
    XDrawRectangle(c->dpy, d, gc, 1, 1,
                   (unsigned int)(ww > 2 ? ww - 3 : 1),
                   (unsigned int)(hh > 2 ? hh - 3 : 1));
}

void iconeditor_preview_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)user;
    draw_canvas_t *c = (draw_canvas_t *)class;
    Drawable d;
    GC gc;
    int ww, hh, scale, ox, oy, x, y;
    int cx0, cy0, cx1, cy1, dx, dy, dw, dh;
    Boolean full;

    if (!c || !c->xdraw) return;
    preview_w = w;
    ensure_pix();
    d = XftDrawDrawable(c->xdraw);
    ww = c->win_width ? (int)c->win_width : 1;
    hh = c->win_height ? (int)c->win_height : 1;
    preview_ww = ww;
    preview_hh = hh;
    gc = ensure_gc(c, d);
    preview_geometry(c, &scale, &ox, &oy);

    full = c->damage_full || c->damage_w <= 0 || c->damage_h <= 0;
    dx = full ? 0 : c->damage_x;
    dy = full ? 0 : c->damage_y;
    dw = full ? ww : c->damage_w;
    dh = full ? hh : c->damage_h;
    if (dx < 0) { dw += dx; dx = 0; }
    if (dy < 0) { dh += dy; dy = 0; }
    if (dx + dw > ww) dw = ww - dx;
    if (dy + dh > hh) dh = hh - dy;
    if (dw <= 0 || dh <= 0) {
        full = True;
        dx = dy = 0;
        dw = ww;
        dh = hh;
    }

    if (full) {
        XFillRectangle(c->dpy, d, c->gc[1], 0, 0,
                       (unsigned int)ww, (unsigned int)hh);
        cx0 = 0; cy0 = 0;
        cx1 = iw - 1; cy1 = ih - 1;
        draw_checker(c, d, gc, scale, ox, oy, cx0, cy0, cx1, cy1);
    } else {
        XRectangle clip;
        clip.x = (short)dx;
        clip.y = (short)dy;
        clip.width = (unsigned short)dw;
        clip.height = (unsigned short)dh;
        XSetClipRectangles(c->dpy, gc, 0, 0, &clip, 1, Unsorted);
        XFillRectangle(c->dpy, d, c->gc[1], dx, dy,
                       (unsigned int)dw, (unsigned int)dh);
        cx0 = (dx - ox) / scale;
        cy0 = (dy - oy) / scale;
        cx1 = (dx + dw - 1 - ox) / scale;
        cy1 = (dy + dh - 1 - oy) / scale;
        if (cx0 < 0) cx0 = 0;
        if (cy0 < 0) cy0 = 0;
        if (cx1 >= iw) cx1 = iw - 1;
        if (cy1 >= ih) cy1 = ih - 1;
        if (cx0 > cx1 || cy0 > cy1) {
            /* damage lies outside the icon grid */
            cx0 = cy0 = 0;
            cx1 = cy1 = -1;
        }
        if (cx0 <= cx1 && cy0 <= cy1)
            draw_checker(c, d, gc, scale, ox, oy, cx0, cy0, cx1, cy1);
    }

    for (y = cy0; y <= cy1; y++) {
        for (x = cx0; x <= cx1; x++) {
            uint32_t rgb = pix[(size_t)y * (size_t)iw + (size_t)x];
            if (!rgb) continue;
            XSetForeground(c->dpy, gc, color_pixel(c, rgb & 0xFFFFFFu));
            XFillRectangle(c->dpy, d, gc, ox + x * scale, oy + y * scale,
                           (unsigned int)scale, (unsigned int)scale);
        }
    }
    XSetForeground(c->dpy, gc, color_pixel(c, 0x000000));
    XDrawRectangle(c->dpy, d, gc, ox - 1, oy - 1,
                   (unsigned int)(iw * scale + 1), (unsigned int)(ih * scale + 1));
    if (!full)
        XSetClipMask(c->dpy, gc, None);
}

/* --- mouse / keyboard --------------------------------------------------- */
void iconeditor_mouse_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)user;
    draw_canvas_input_t *in = (draw_canvas_input_t *)class;
    draw_canvas_t *c;
    XEvent *e;
    int cx, cy;

    if (!in || !in->event || !in->canvas) return;
    c = in->canvas;
    e = in->event;

    if (e->type == ButtonPress) {
        int button = e->xbutton.button;
        if (button != Button1 && button != Button3) return;
        if (!map_cell(c, e->xbutton.x, e->xbutton.y, &cx, &cy)) return;
        erasing = (button == Button3);
        drag_button = button;
        push_undo();
        if (tool == TOOL_FILL) {
            flood_fill(cx, cy, cur_color);
            document_changed();
            erasing = 0;
            return;
        }
        dragging = 1;
        start_cx = last_cx = cur_cx = cx;
        start_cy = last_cy = cur_cy = cy;
        if (tool == TOOL_PENCIL) set_pixel(cx, cy, cur_color);
        document_changed();
    } else if (e->type == MotionNotify) {
        if (!dragging) return;
        if (!map_cell(c, e->xmotion.x, e->xmotion.y, &cx, &cy)) return;
        cur_cx = cx;
        cur_cy = cy;
        if (tool == TOOL_PENCIL) {
            raster_line(last_cx, last_cy, cx, cy, cur_color);
            last_cx = cx;
            last_cy = cy;
        }
        document_changed();
    } else if (e->type == ButtonRelease) {
        if (!dragging || e->xbutton.button != drag_button) return;
        if (map_cell(c, e->xbutton.x, e->xbutton.y, &cx, &cy)) {
            cur_cx = cx;
            cur_cy = cy;
        }
        if (tool == TOOL_LINE)
            raster_line(start_cx, start_cy, cur_cx, cur_cy, cur_color);
        else if (tool == TOOL_RECT)
            raster_rect(start_cx, start_cy, cur_cx, cur_cy, cur_color);
        else if (tool == TOOL_ELLIPSE)
            raster_ellipse(start_cx, start_cy, cur_cx, cur_cy, cur_color);
        /* Keep dragging set for document_changed() so the rubber band of a
         * shape tool is erased by a full repaint. */
        document_changed();
        dragging = 0;
        erasing = 0;
    }
    (void)w;
}

void iconeditor_key_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)user;
    draw_canvas_input_t *in = (draw_canvas_input_t *)class;
    KeySym ks;
    if (!in || !in->event) return;
    ks = XLookupKeysym((XKeyEvent *)in->event, 0);
    if ((ks == XK_z || ks == XK_Z) &&
        (((XKeyEvent *)in->event)->state & ControlMask)) {
        XtCallActionProc(w, "iconeditor_undo", NULL, NULL, 0);
        document_changed();
    } else if (ks == XK_Delete || ks == XK_BackSpace) {
        XtCallActionProc(w, "iconeditor_clear", NULL, NULL, 0);
        document_changed();
    }
}

/* --- XPM I/O ------------------------------------------------------------ */
static int hex_val(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int valid_hex_color(const char *s)
{
    int i, n;
    if (!s) return 0;
    if (*s == '#') s++;
    n = (int)strlen(s);
    if (n != 3 && n != 6) return 0;
    for (i = 0; i < n; i++)
        if (hex_val((unsigned char)s[i]) < 0) return 0;
    return 1;
}

static uint32_t parse_xpm_color(const char *s)
{
    int i, v;
    uint32_t rgb = 0;
    if (!s) return 0;
    if (strncmp(s, "None", 4) == 0) return 0;
    if (*s == '#') s++;
    for (i = 0; i < 6 && s[i]; i++) {
        v = hex_val((unsigned char)s[i]);
        if (v < 0) return 0;
        rgb = (rgb << 4) | (uint32_t)v;
    }
    if (i == 3) {
        uint32_t r = (rgb >> 8) & 0xf, g = (rgb >> 4) & 0xf, b = rgb & 0xf;
        rgb = (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
    } else if (i != 6) {
        return 0;
    }
    return rgb;
}

static int save_xpm(const char *path)
{
    FILE *f;
    uint32_t pal[MAX_COLORS];
    int np = 0, x, y, i;
    int failed = 0;

    if (!path || !*path) {
        xt_error_set(XT_ERR_PATH, "iconeditor_save: empty path");
        return 0;
    }
    ensure_pix();
    for (i = 0; i < iw * ih; i++) {
        uint32_t rgb = pix[i] & 0xFFFFFFu;
        int found = 0, j;
        if (!pix[i]) continue;
        for (j = 0; j < np; j++) if (pal[j] == rgb) { found = 1; break; }
        if (found) continue;
        if (np >= MAX_COLORS || np >= (int)strlen(xpm_chars)) {
            xt_error_set(XT_ERR_COLORS,
                         "iconeditor_save: too many colours for XPM");
            return 0;
        }
        pal[np++] = rgb;
    }
    f = fopen(path, "w");
    if (!f) {
        xt_error_set(XT_ERR_OPEN, "iconeditor_save: cannot open '%s'", path);
        return 0;
    }
    if (fprintf(f, "/* XPM */\nstatic char *icon[] = {\n") < 0) failed = 1;
    if (!failed && fprintf(f, "\"%d %d %d 1\",\n", iw, ih, np + 1) < 0)
        failed = 1;
    if (!failed && fprintf(f, "\"  c None\",\n") < 0) failed = 1;
    for (i = 0; i < np && !failed; i++)
        if (fprintf(f, "\"%c c #%06X\",\n", xpm_chars[i],
                    (unsigned)(pal[i] & 0xffffff)) < 0) failed = 1;
    for (y = 0; y < ih && !failed; y++) {
        if (fputc('"', f) == EOF) failed = 1;
        for (x = 0; x < iw && !failed; x++) {
            uint32_t rgb = pix[(size_t)y * (size_t)iw + (size_t)x] & 0xFFFFFFu;
            char ch = ' ';
            if (pix[(size_t)y * (size_t)iw + (size_t)x]) {
                for (i = 0; i < np; i++) if (pal[i] == rgb) break;
                if (i < np) ch = xpm_chars[i];
            }
            if (fputc(ch, f) == EOF) failed = 1;
        }
        if (!failed && fputs("\",\n", f) == EOF) failed = 1;
    }
    if (!failed && fputs("};\n", f) == EOF) failed = 1;
    if (fclose(f) != 0) failed = 1;
    if (failed) {
        xt_error_set(XT_ERR_WRITE,
                     "iconeditor_save: write failed for '%s'", path);
        return 0;
    }
    return 1;
}

static int load_xpm(const char *path)
{
    FILE *f;
    char line[4096];
    char **strings = NULL;
    int ns = 0, cap = 0;
    int w = 0, h = 0, n = 0, cpp = 0;
    char keys[MAX_COLORS][5];
    uint32_t cols[MAX_COLORS];
    uint32_t *newpix = NULL;
    int i, x, y, ok = 0;

    if (!path || !*path) {
        xt_error_set(XT_ERR_PATH, "iconeditor_load: empty path");
        return 0;
    }
    f = fopen(path, "r");
    if (!f) {
        xt_error_set(XT_ERR_OPEN, "iconeditor_load: cannot open '%s'", path);
        return 0;
    }
    while (fgets(line, sizeof line, f)) {
        char *p = line;
        while ((p = strchr(p, '"')) != NULL) {
            char *e;
            p++;
            e = strchr(p, '"');
            if (!e) break;
            if (ns >= MAX_XPM_STRINGS) break;
            if (ns >= cap) {
                int nc = cap ? cap * 2 : 64;
                char **tmp = (char **)realloc(strings, (size_t)nc * sizeof(char *));
                if (!tmp) break;
                strings = tmp;
                cap = nc;
            }
            strings[ns] = (char *)malloc((size_t)(e - p) + 1);
            if (!strings[ns]) break;
            memcpy(strings[ns], p, (size_t)(e - p));
            strings[ns][e - p] = 0;
            ns++;
            p = e + 1;
        }
    }
    fclose(f);
    if (ns < 1 || sscanf(strings[0], "%d %d %d %d", &w, &h, &n, &cpp) != 4) {
        xt_error_set(XT_ERR_FORMAT, "iconeditor_load: invalid XPM header");
        goto done;
    }
    if (w < MIN_DIM || h < MIN_DIM || w > MAX_DIM || h > MAX_DIM ||
        n < 1 || n > MAX_COLORS || cpp < 1 || cpp > 4) {
        xt_error_set(XT_ERR_FORMAT,
                     "iconeditor_load: unsupported XPM dimensions/palette");
        goto done;
    }
    if (ns < 1 + n + h) {
        xt_error_set(XT_ERR_FORMAT, "iconeditor_load: truncated XPM");
        goto done;
    }

    for (i = 0; i < n; i++) {
        const char *s = strings[1 + i];
        const char *cpos;
        memset(keys[i], 0, sizeof keys[i]);
        if ((int)strlen(s) < cpp) {
            xt_error_set(XT_ERR_FORMAT, "iconeditor_load: bad colour key");
            goto done;
        }
        strncpy(keys[i], s, (size_t)cpp);
        cpos = strstr(s + cpp, " c ");
        if (!cpos || strncmp(cpos + 3, "None", 4) == 0)
            cols[i] = 0;
        else
            cols[i] = parse_xpm_color(cpos + 3) | 0xFF000000u;
    }
    newpix = (uint32_t *)calloc((size_t)w * (size_t)h, sizeof(uint32_t));
    if (!newpix) {
        xt_error_set(XT_ERR_MEMORY, "iconeditor_load: out of memory");
        goto done;
    }
    for (y = 0; y < h; y++) {
        const char *row = strings[1 + n + y];
        if ((int)strlen(row) < w * cpp) continue;
        for (x = 0; x < w; x++) {
            char key[5];
            int k;
            memset(key, 0, sizeof key);
            strncpy(key, row + (size_t)x * (size_t)cpp, (size_t)cpp);
            for (k = 0; k < n; k++) {
                if (strcmp(key, keys[k]) == 0) {
                    newpix[(size_t)y * (size_t)w + (size_t)x] = cols[k];
                    break;
                }
            }
        }
    }

    free(pix);
    pix = newpix;
    newpix = NULL;
    iw = w;
    ih = h;
    free_undo();
    ok = 1;

done:
    if (newpix) free(newpix);
    for (i = 0; i < ns; i++) free(strings[i]);
    free(strings);
    if (ok) repaint_only();
    return ok;
}

/* --- actions ------------------------------------------------------------ */
static void iconeditor_set_tool(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e;
    if (*n < 1) {
        xt_error_set(XT_ERR_ARG, "iconeditor_set_tool: missing tool");
        return;
    }
    if (strcmp(p[0], "pencil") == 0) tool = TOOL_PENCIL;
    else if (strcmp(p[0], "line") == 0) tool = TOOL_LINE;
    else if (strcmp(p[0], "rect") == 0) tool = TOOL_RECT;
    else if (strcmp(p[0], "ellipse") == 0 || strcmp(p[0], "circle") == 0)
        tool = TOOL_ELLIPSE;
    else if (strcmp(p[0], "fill") == 0) tool = TOOL_FILL;
    else
        xt_error_set(XT_ERR_ARG, "iconeditor_set_tool: unknown tool '%s'",
                     p[0]);
}

static void iconeditor_set_color(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e;
    if (*n < 1) {
        xt_error_set(XT_ERR_ARG, "iconeditor_set_color: missing colour");
        return;
    }
    if (!valid_hex_color(p[0])) {
        xt_error_set(XT_ERR_ARG,
                     "iconeditor_set_color: invalid colour '%s'", p[0]);
        return;
    }
    cur_color = parse_xpm_color(p[0]);
    mark_dirty(swatch_w);
}

static void iconeditor_resize(Widget w, XEvent *e, String *p, Cardinal *n)
{
    char *end;
    long nw, nh;
    (void)w; (void)e;
    if (*n < 2) {
        xt_error_set(XT_ERR_ARG,
                     "iconeditor_resize: width and height required");
        return;
    }
    nw = strtol(p[0], &end, 10);
    if (end == p[0] || *end) {
        xt_error_set(XT_ERR_ARG, "iconeditor_resize: invalid width '%s'",
                     p[0]);
        return;
    }
    nh = strtol(p[1], &end, 10);
    if (end == p[1] || *end) {
        xt_error_set(XT_ERR_ARG, "iconeditor_resize: invalid height '%s'",
                     p[1]);
        return;
    }
    if (nw < MIN_DIM || nw > MAX_DIM || nh < MIN_DIM || nh > MAX_DIM) {
        xt_error_set(XT_ERR_RANGE,
                     "iconeditor_resize: size must be %d..%d",
                     MIN_DIM, MAX_DIM);
        return;
    }
    if (nw == iw && nh == ih) return;
    free(pix);
    iw = (int)nw;
    ih = (int)nh;
    pix = (uint32_t *)calloc((size_t)iw * (size_t)ih, sizeof(uint32_t));
    free_undo();
    dragging = 0;
    erasing = 0;
    repaint_only();
}

static void iconeditor_clear(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e; (void)p; (void)n;
    ensure_pix();
    push_undo();
    memset(pix, 0, (size_t)iw * (size_t)ih * sizeof(uint32_t));
    dragging = 0;
    erasing = 0;
    repaint_only();
}

static void iconeditor_undo(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e; (void)p; (void)n;
    if (undo_count <= 0) {
        xt_error_set(XT_ERR_STATE, "iconeditor_undo: nothing to undo");
        return;
    }
    undo_count--;
    free(pix);
    pix = undo[undo_count].pix;
    iw = undo[undo_count].w;
    ih = undo[undo_count].h;
    undo[undo_count].pix = NULL;
    dragging = 0;
    erasing = 0;
    repaint_only();
}

static void iconeditor_save(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e;
    if (*n < 1) {
        xt_error_set(XT_ERR_ARG, "iconeditor_save: missing path");
        return;
    }
    save_xpm(p[0]);
}

static void iconeditor_load(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e;
    if (*n < 1) {
        xt_error_set(XT_ERR_ARG, "iconeditor_load: missing path");
        return;
    }
    load_xpm(p[0]);
}

/* Test hook used by the demo smoke test: draw a diagonal in the current
 * colour without synthesising pointer events. */
static void iconeditor_test_stroke(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e; (void)p; (void)n;
    ensure_pix();
    push_undo();
    erasing = 0;
    raster_line(0, 0, iw - 1, ih - 1, cur_color);
    repaint_only();
}

/* Test hook used by the demo smoke test: erase the top-left cell without
 * synthesising a Button3 event. */
static void iconeditor_test_erase(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e; (void)p; (void)n;
    ensure_pix();
    push_undo();
    erasing = 1;
    set_pixel(0, 0, cur_color);
    erasing = 0;
    repaint_only();
}

void iconeditor_plugin_init(Widget top)
{
    RCB(top, iconeditor_draw_cb);
    RCB(top, iconeditor_preview_cb);
    RCB(top, iconeditor_swatch_cb);
    RCB(top, iconeditor_mouse_cb);
    RCB(top, iconeditor_key_cb);
    RAC(top, iconeditor_set_tool);
    RAC(top, iconeditor_set_color);
    RAC(top, iconeditor_resize);
    RAC(top, iconeditor_clear);
    RAC(top, iconeditor_undo);
    RAC(top, iconeditor_save);
    RAC(top, iconeditor_load);
    RAC(top, iconeditor_test_stroke);
    RAC(top, iconeditor_test_erase);
}
