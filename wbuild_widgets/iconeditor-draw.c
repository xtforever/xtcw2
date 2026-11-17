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
static int start_cx, start_cy, last_cx, last_cy, cur_cx, cur_cy;
static Widget grid_w, preview_w, swatch_w;
static GC paint_gc;
static undo_frame undo[MAX_UNDO];
static int undo_count;

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
    /* 0 means transparent; opaque black is 0xFF000000. */
    pix[(size_t)y * (size_t)iw + (size_t)x] = 0xFF000000u | (rgb & 0xFFFFFFu);
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
    uint32_t target;
    if (!pix || x < 0 || y < 0 || x >= iw || y >= ih) return;
    target = pix[(size_t)y * (size_t)iw + (size_t)x];
    rgb = 0xFF000000u | (rgb & 0xFFFFFFu);
    if (target == rgb) return;
    stack = (int *)malloc((size_t)n * 4 * sizeof(int));
    if (!stack) return;
    stack[sp++] = y * iw + x;
    while (sp > 0) {
        int idx = stack[--sp];
        int cx = idx % iw, cy = idx / iw;
        if (pix[idx] != target) continue;
        pix[idx] = rgb;
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

static void repaint_only(void)
{
    mark_dirty(grid_w);
    mark_dirty(preview_w);
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
    repaint_only();
}

/* --- geometry ----------------------------------------------------------- */
static void grid_geometry(draw_canvas_t *c, int *cell, int *ox, int *oy)
{
    int ww = c->win_width ? (int)c->win_width : 1;
    int hh = c->win_height ? (int)c->win_height : 1;
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

static void preview_geometry(draw_canvas_t *c, int *scale, int *ox, int *oy)
{
    int ww = c->win_width ? (int)c->win_width : 1;
    int hh = c->win_height ? (int)c->win_height : 1;
    int margin = 8;
    int cw = (ww - 2 * margin) / (iw > 0 ? iw : 1);
    int ch = (hh - 2 * margin) / (ih > 0 ? ih : 1);
    int s = cw < ch ? cw : ch;
    if (s < 1) s = 1;
    *scale = s;
    *ox = (ww - s * iw) / 2;
    *oy = (hh - s * ih) / 2;
    if (*ox < 0) *ox = 0;
    if (*oy < 0) *oy = 0;
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
                         int cell, int ox, int oy, int w, int h)
{
    Pixel light = color_pixel(c, 0xffffff);
    Pixel dark = color_pixel(c, 0xd0d0d0);
    int x, y;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
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

    if (!c || !c->xdraw) return;
    grid_w = w;
    ensure_pix();
    d = XftDrawDrawable(c->xdraw);
    ww = c->win_width ? (int)c->win_width : 1;
    hh = c->win_height ? (int)c->win_height : 1;
    gc = ensure_gc(c, d);

    XFillRectangle(c->dpy, d, c->gc[1], 0, 0,
                   (unsigned int)ww, (unsigned int)hh);
    grid_geometry(c, &cell, &ox, &oy);
    draw_checker(c, d, gc, cell, ox, oy, iw, ih);

    for (y = 0; y < ih; y++) {
        for (x = 0; x < iw; x++) {
            uint32_t rgb = pix[(size_t)y * (size_t)iw + (size_t)x];
            if (!rgb) continue;
            XSetForeground(c->dpy, gc, color_pixel(c, rgb & 0xFFFFFFu));
            XFillRectangle(c->dpy, d, gc, ox + x * cell, oy + y * cell,
                           (unsigned int)cell, (unsigned int)cell);
        }
    }

    if (cell >= 4) {
        XSetForeground(c->dpy, gc, color_pixel(c, 0x808080));
        for (x = 0; x <= iw; x++)
            XDrawLine(c->dpy, d, gc, ox + x * cell, oy,
                      ox + x * cell, oy + ih * cell);
        for (y = 0; y <= ih; y++)
            XDrawLine(c->dpy, d, gc, ox, oy + y * cell,
                      ox + iw * cell, oy + y * cell);
    }
    XSetForeground(c->dpy, gc, color_pixel(c, 0x000000));
    XDrawRectangle(c->dpy, d, gc, ox - 1, oy - 1,
                   (unsigned int)(iw * cell + 1), (unsigned int)(ih * cell + 1));

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

    if (!c || !c->xdraw) return;
    preview_w = w;
    ensure_pix();
    d = XftDrawDrawable(c->xdraw);
    ww = c->win_width ? (int)c->win_width : 1;
    hh = c->win_height ? (int)c->win_height : 1;
    gc = ensure_gc(c, d);

    XFillRectangle(c->dpy, d, c->gc[1], 0, 0,
                   (unsigned int)ww, (unsigned int)hh);
    preview_geometry(c, &scale, &ox, &oy);
    draw_checker(c, d, gc, scale, ox, oy, iw, ih);

    for (y = 0; y < ih; y++) {
        for (x = 0; x < iw; x++) {
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
        if (e->xbutton.button != Button1) return;
        if (!map_cell(c, e->xbutton.x, e->xbutton.y, &cx, &cy)) return;
        if (tool == TOOL_FILL) {
            push_undo();
            flood_fill(cx, cy, cur_color);
            document_changed();
            return;
        }
        push_undo();
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
        if (!dragging || e->xbutton.button != Button1) return;
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
        dragging = 0;
        document_changed();
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
    if (!path || !*path) return 0;
    ensure_pix();
    for (i = 0; i < iw * ih; i++) {
        uint32_t rgb = pix[i] & 0xFFFFFFu;
        int found = 0, j;
        if (!pix[i]) continue;
        for (j = 0; j < np; j++) if (pal[j] == rgb) { found = 1; break; }
        if (found) continue;
        if (np >= MAX_COLORS || np >= (int)strlen(xpm_chars)) return 0;
        pal[np++] = rgb;
    }
    f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "/* XPM */\nstatic char *icon[] = {\n");
    fprintf(f, "\"%d %d %d 1\",\n", iw, ih, np + 1);
    fprintf(f, "\"  c None\",\n");
    for (i = 0; i < np; i++)
        fprintf(f, "\"%c c #%06X\",\n", xpm_chars[i],
                (unsigned)(pal[i] & 0xffffff));
    for (y = 0; y < ih; y++) {
        fputc('"', f);
        for (x = 0; x < iw; x++) {
            uint32_t rgb = pix[(size_t)y * (size_t)iw + (size_t)x] & 0xFFFFFFu;
            char ch = ' ';
            if (pix[(size_t)y * (size_t)iw + (size_t)x]) {
                for (i = 0; i < np; i++) if (pal[i] == rgb) break;
                if (i < np) ch = xpm_chars[i];
            }
            fputc(ch, f);
        }
        fputs("\",\n", f);
    }
    fputs("};\n", f);
    fclose(f);
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

    if (!path || !*path) return 0;
    f = fopen(path, "r");
    if (!f) return 0;
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
    if (ns < 1 || sscanf(strings[0], "%d %d %d %d", &w, &h, &n, &cpp) != 4)
        goto done;
    if (w < MIN_DIM || h < MIN_DIM || w > MAX_DIM || h > MAX_DIM ||
        n < 1 || n > MAX_COLORS || cpp < 1 || cpp > 4)
        goto done;
    if (ns < 1 + n + h) goto done;

    for (i = 0; i < n; i++) {
        const char *s = strings[1 + i];
        const char *cpos;
        memset(keys[i], 0, sizeof keys[i]);
        if ((int)strlen(s) < cpp) goto done;
        strncpy(keys[i], s, (size_t)cpp);
        cpos = strstr(s + cpp, " c ");
        if (!cpos || strncmp(cpos + 3, "None", 4) == 0)
            cols[i] = 0;
        else
            cols[i] = parse_xpm_color(cpos + 3) | 0xFF000000u;
    }
    newpix = (uint32_t *)calloc((size_t)w * (size_t)h, sizeof(uint32_t));
    if (!newpix) goto done;
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
    if (*n < 1) return;
    if (strcmp(p[0], "line") == 0) tool = TOOL_LINE;
    else if (strcmp(p[0], "rect") == 0) tool = TOOL_RECT;
    else if (strcmp(p[0], "ellipse") == 0 || strcmp(p[0], "circle") == 0)
        tool = TOOL_ELLIPSE;
    else if (strcmp(p[0], "fill") == 0) tool = TOOL_FILL;
    else tool = TOOL_PENCIL;
}

static void iconeditor_set_color(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e;
    if (*n < 1) return;
    cur_color = parse_xpm_color(p[0]);
    mark_dirty(swatch_w);
}

static void iconeditor_resize(Widget w, XEvent *e, String *p, Cardinal *n)
{
    int nw, nh;
    (void)w; (void)e;
    if (*n < 2) return;
    nw = atoi(p[0]);
    nh = atoi(p[1]);
    if (nw < MIN_DIM) nw = MIN_DIM;
    if (nh < MIN_DIM) nh = MIN_DIM;
    if (nw > MAX_DIM) nw = MAX_DIM;
    if (nh > MAX_DIM) nh = MAX_DIM;
    if (nw == iw && nh == ih) return;
    free(pix);
    iw = nw;
    ih = nh;
    pix = (uint32_t *)calloc((size_t)iw * (size_t)ih, sizeof(uint32_t));
    free_undo();
    dragging = 0;
    repaint_only();
}

static void iconeditor_clear(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e; (void)p; (void)n;
    ensure_pix();
    push_undo();
    memset(pix, 0, (size_t)iw * (size_t)ih * sizeof(uint32_t));
    dragging = 0;
    repaint_only();
}

static void iconeditor_undo(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e; (void)p; (void)n;
    if (undo_count <= 0) return;
    undo_count--;
    free(pix);
    pix = undo[undo_count].pix;
    iw = undo[undo_count].w;
    ih = undo[undo_count].h;
    undo[undo_count].pix = NULL;
    dragging = 0;
    repaint_only();
}

static void iconeditor_save(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e;
    if (*n < 1) return;
    save_xpm(p[0]);
}

static void iconeditor_load(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e;
    if (*n < 1) return;
    load_xpm(p[0]);
}

/* Test hook used by the demo smoke test: draw a diagonal in the current
 * colour without synthesising pointer events. */
static void iconeditor_test_stroke(Widget w, XEvent *e, String *p, Cardinal *n)
{
    (void)w; (void)e; (void)p; (void)n;
    ensure_pix();
    push_undo();
    raster_line(0, 0, iw - 1, ih - 1, cur_color);
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
}
