/*
 * Snake for xtcw2's DrawCanvas.
 *
 * This is a port of games/snake.c (Bevel OS) to the DrawCanvas plug-in API.
 * The game state advances from the widget's tick callback; the widget only
 * repaints when the plug-in marks the canvas dirty. The DrawCanvas timer and
 * optional double buffer therefore keep XFlush calls at one per update, not
 * one per key/motion event.
 *
 * Drawing is batched: the checkerboard is a 2x2-cell tile filled with one
 * XFillRectangle, and all snake rectangles are sent with XFillRectangles.
 * A frame costs one tile fill plus a handful of requests instead of 532+
 * per-cell XFillRectangle calls.
 */
#include "drawcanvas-draw.h"
#include "wcreg2.h"
#include <X11/Xft/Xft.h>
#include <X11/keysym.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CELL 12
#define GW   28
#define GH   19
#define TOP  22
#define W    (GW * CELL)
#define H    (TOP + GH * CELL)
#define MAXLEN (GW * GH)

enum {
    P_BG0, P_BG1, P_SNAKE, P_HEAD, P_DARK, P_FOOD, P_PANEL, P_TEXT, P_COUNT
};

static uint8_t sx[MAXLEN], sy[MAXLEN];
static int head, len, dir, next_dir, fx, fy, score, best, alive, paused;
static uint32_t next_move_ms;
static int initialized;
static GC gc;
static Pixmap tile;
static GC tile_gc;
static int tile_ready;
static Pixel pal[P_COUNT];
static XftColor xft_text, xft_warn, xft_pause;
static int colors_ready;

static uint32_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)((uint64_t)ts.tv_sec * 1000u + (uint64_t)ts.tv_nsec / 1000000u);
}

static int on_snake(int x, int y)
{
    for (int i = 0; i < len; i++) {
        int k = (head - i + MAXLEN) % MAXLEN;
        if (sx[k] == x && sy[k] == y) return 1;
    }
    return 0;
}

static void place_food(void)
{
    for (int tries = 0; tries < MAXLEN; tries++) {
        int x = rand() % GW;
        int y = rand() % GH;
        if (!on_snake(x, y)) {
            fx = x;
            fy = y;
            return;
        }
    }
    fx = fy = -1; /* board is full */
}

static void reset(void)
{
    len = 4;
    head = 3;
    dir = next_dir = 0;
    score = 0;
    alive = 1;
    paused = 0;
    for (int i = 0; i < 4; i++) {
        sx[i] = 6 + i;
        sy[i] = GH / 2;
    }
    place_food();
    next_move_ms = now_ms() + 150;
}

static void step(void)
{
    static const int ddx[4] = {1, 0, -1, 0};
    static const int ddy[4] = {0, 1, 0, -1};

    dir = next_dir;
    int nx = sx[head] + ddx[dir];
    int ny = sy[head] + ddy[dir];
    int tail = (head - len + 1 + MAXLEN) % MAXLEN;
    int grow = (nx == fx && ny == fy);

    int hit = nx < 0 || ny < 0 || nx >= GW || ny >= GH;
    if (!hit && on_snake(nx, ny) && !(nx == sx[tail] && ny == sy[tail] && !grow))
        hit = 1;

    if (hit) {
        alive = 0;
        if (score > best) best = score;
        return;
    }

    head = (head + 1) % MAXLEN;
    sx[head] = nx;
    sy[head] = ny;

    if (grow) {
        score += 10;
        if (len < MAXLEN) {
            len++;
            place_food();
        }
    }
}

static void ensure_init(draw_canvas_t *c)
{
    (void)c;
    if (initialized) return;
    initialized = 1;
    srand((unsigned)time(NULL));
    reset();
}

static void ensure_gc(draw_canvas_t *c)
{
    if (!gc)
        gc = XCreateGC(c->dpy, RootWindow(c->dpy, c->screen), 0, NULL);
}

static void ensure_colors(draw_canvas_t *c)
{
    if (colors_ready) return;

    static const struct { unsigned short r, g, b; } rgb[P_COUNT] = {
        {0x0000, 0x1000, 0x0000}, /* BG0 */
        {0x0000, 0x1c00, 0x0000}, /* BG1 */
        {0x2000, 0x9000, 0x2000}, /* SNAKE */
        {0x9000, 0xf000, 0x5000}, /* HEAD */
        {0x0800, 0x3000, 0x0800}, /* DARK */
        {0xe000, 0x2000, 0x2000}, /* FOOD */
        {0x1800, 0x1800, 0x1800}, /* PANEL */
        {0xffff, 0xffff, 0xffff}, /* TEXT */
    };

    Colormap cmap = DefaultColormap(c->dpy, c->screen);
    for (int i = 0; i < P_COUNT; i++) {
        XColor col;
        col.red = rgb[i].r;
        col.green = rgb[i].g;
        col.blue = rgb[i].b;
        col.flags = DoRed | DoGreen | DoBlue;
        if (!XAllocColor(c->dpy, cmap, &col)) col.pixel = BlackPixel(c->dpy, c->screen);
        pal[i] = col.pixel;
    }

    Visual *vis = DefaultVisual(c->dpy, c->screen);
    if (!XftColorAllocName(c->dpy, vis, cmap, "#ffffff", &xft_text))
        xft_text = c->xcol[1];
    if (!XftColorAllocName(c->dpy, vis, cmap, "#ff7070", &xft_warn))
        xft_warn = c->xcol[1];
    if (!XftColorAllocName(c->dpy, vis, cmap, "#ffe070", &xft_pause))
        xft_pause = c->xcol[1];
    colors_ready = 1;
}

static void ensure_tile(draw_canvas_t *c, Drawable d)
{
    if (tile_ready) return;

    int tw = 2 * CELL;
    int th = 2 * CELL;
    tile = XCreatePixmap(c->dpy, d, (unsigned)tw, (unsigned)th,
                         (unsigned)DefaultDepth(c->dpy, c->screen));
    if (!tile) {
        tile_ready = 1;
        return;
    }

    GC g = XCreateGC(c->dpy, tile, 0, NULL);
    /* Cell (x,y) uses ((x + y) & 1) ? BG0 : BG1. */
    XSetForeground(c->dpy, g, pal[P_BG1]);
    XFillRectangle(c->dpy, tile, g, 0, 0, CELL, CELL);
    XSetForeground(c->dpy, g, pal[P_BG0]);
    XFillRectangle(c->dpy, tile, g, CELL, 0, CELL, CELL);
    XFillRectangle(c->dpy, tile, g, 0, CELL, CELL, CELL);
    XSetForeground(c->dpy, g, pal[P_BG1]);
    XFillRectangle(c->dpy, tile, g, CELL, CELL, CELL, CELL);
    XFreeGC(c->dpy, g);

    tile_gc = XCreateGC(c->dpy, d, 0, NULL);
    XSetTile(c->dpy, tile_gc, tile);
    XSetFillStyle(c->dpy, tile_gc, FillTiled);
    tile_ready = 1;
}

static void set_fg(draw_canvas_t *c, int idx)
{
    XSetForeground(c->dpy, gc, pal[idx]);
}

static void draw_text(draw_canvas_t *c, int x, int y, const char *s, XftColor *col)
{
    if (!c->xfont || !c->xdraw || !s) return;
    XftDrawStringUtf8(c->xdraw, col, c->xfont, x, y,
                      (const FcChar8*)s, (int)strlen(s));
}

static int text_width(draw_canvas_t *c, const char *s)
{
    if (!c->xfont || !s) return 0;
    XGlyphInfo ext;
    XftTextExtentsUtf8(c->dpy, c->xfont, (const FcChar8*)s,
                       (int)strlen(s), &ext);
    return ext.xOff;
}

static void draw_board(draw_canvas_t *c, Drawable d, int ox, int oy)
{
    if (!tile) {
        set_fg(c, P_BG0);
        XFillRectangle(c->dpy, d, gc, ox, oy + TOP, W, GH * CELL);
        return;
    }
    XSetTSOrigin(c->dpy, tile_gc, ox, oy + TOP);
    XFillRectangle(c->dpy, d, tile_gc, ox, oy + TOP, W, GH * CELL);
}

static void draw_pieces(draw_canvas_t *c, Drawable d, int ox, int oy)
{
    static XRectangle body[MAXLEN];
    static XRectangle dark[2 * MAXLEN];
    static XRectangle eyes[2];
    static XRectangle food[2];
    static XRectangle food_hi[1];
    static XRectangle stem[1];
    static XRectangle head_rect[1];
    int nb = 0, nd = 0;

    for (int i = 0; i < len; i++) {
        int k = (head - i + MAXLEN) % MAXLEN;
        int px = ox + sx[k] * CELL;
        int py = oy + TOP + sy[k] * CELL;

        if (i == 0) {
            head_rect[0].x = (short)(px + 1);
            head_rect[0].y = (short)(py + 1);
            head_rect[0].width = CELL - 2;
            head_rect[0].height = CELL - 2;
            eyes[0].x = (short)(px + 3);
            eyes[0].y = (short)(py + 3);
            eyes[0].width = 2;
            eyes[0].height = 2;
            eyes[1].x = (short)(px + 7);
            eyes[1].y = (short)(py + 3);
            eyes[1].width = 2;
            eyes[1].height = 2;
        } else {
            body[nb].x = (short)(px + 1);
            body[nb].y = (short)(py + 1);
            body[nb].width = CELL - 2;
            body[nb].height = CELL - 2;
            nb++;
        }

        dark[nd].x = (short)(px + 1);
        dark[nd].y = (short)(py + CELL - 3);
        dark[nd].width = CELL - 2;
        dark[nd].height = 2;
        nd++;
        dark[nd].x = (short)(px + CELL - 3);
        dark[nd].y = (short)(py + 1);
        dark[nd].width = 2;
        dark[nd].height = CELL - 2;
        nd++;
    }

    if (nb) {
        set_fg(c, P_SNAKE);
        XFillRectangles(c->dpy, d, gc, body, nb);
    }
    if (len > 0) {
        set_fg(c, P_HEAD);
        XFillRectangles(c->dpy, d, gc, head_rect, 1);
    }
    if (nd) {
        set_fg(c, P_DARK);
        XFillRectangles(c->dpy, d, gc, dark, nd);
    }
    if (len > 0) {
        set_fg(c, P_BG0);
        XFillRectangles(c->dpy, d, gc, eyes, 2);
    }

    if (fx >= 0 && fy >= 0) {
        int px = ox + fx * CELL;
        int py = oy + TOP + fy * CELL;
        food[0].x = (short)(px + 3);
        food[0].y = (short)(py + 2);
        food[0].width = 6;
        food[0].height = 8;
        food[1].x = (short)(px + 2);
        food[1].y = (short)(py + 3);
        food[1].width = 8;
        food[1].height = 6;
        food_hi[0].x = (short)(px + 4);
        food_hi[0].y = (short)(py + 3);
        food_hi[0].width = 2;
        food_hi[0].height = 2;
        stem[0].x = (short)(px + 6);
        stem[0].y = (short)py;
        stem[0].width = 1;
        stem[0].height = 3;

        set_fg(c, P_FOOD);
        XFillRectangles(c->dpy, d, gc, food, 2);
        set_fg(c, P_BG1);
        XFillRectangles(c->dpy, d, gc, food_hi, 1);
        set_fg(c, P_DARK);
        XFillRectangles(c->dpy, d, gc, stem, 1);
    }
}

static void panel(draw_canvas_t *c, Drawable d, int ox, int oy)
{
    char buf[64];

    set_fg(c, P_PANEL);
    XFillRectangle(c->dpy, d, gc, ox, oy, W, TOP);
    set_fg(c, P_DARK);
    XDrawLine(c->dpy, d, gc, ox, oy + TOP - 1, ox + W, oy + TOP - 1);
    XDrawLine(c->dpy, d, gc, ox, oy, ox + W, oy);

    int baseline = oy + 5 + (c->xfont ? c->xfont->ascent : 11);
    snprintf(buf, sizeof buf, "Score %d", score);
    draw_text(c, ox + 8, baseline, buf, &xft_text);
    snprintf(buf, sizeof buf, "Best %d", best);
    draw_text(c, ox + 100, baseline, buf, &xft_text);

    const char *msg = !alive ? "Game over - Enter restarts"
                    : paused ? "Paused - P resumes"
                             : "Arrows/WASD steer";
    XftColor *col = !alive ? &xft_warn : paused ? &xft_pause : &xft_text;
    int tw = text_width(c, msg);
    int tx = ox + W - 8 - tw;
    if (tx < ox + 180) tx = ox + 180;
    draw_text(c, tx, baseline, msg, col);
}

void snake_draw_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)w;
    (void)user;
    draw_canvas_t *c = (draw_canvas_t*)class;
    if (!c || !c->xdraw) return;

    ensure_init(c);
    ensure_gc(c);
    ensure_colors(c);

    Drawable d = XftDrawDrawable(c->xdraw);
    ensure_tile(c, d);
    unsigned ww = c->win_width  ? c->win_width  : W;
    unsigned hh = c->win_height ? c->win_height : H;
    if (ww < 2) ww = W;
    if (hh < 2) hh = H;

    int ox = (ww > W) ? (int)((ww - W) / 2) : 0;
    int oy = (hh > H) ? (int)((hh - H) / 2) : 0;

    set_fg(c, P_BG0);
    XFillRectangle(c->dpy, d, gc, 0, 0, ww, hh);

    draw_board(c, d, ox, oy);
    draw_pieces(c, d, ox, oy);
    panel(c, d, ox, oy);
}

void snake_tick_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)user;
    draw_canvas_t *c = (draw_canvas_t*)class;
    if (!c) return;

    ensure_init(c);
    uint32_t now = now_ms();
    if (!alive || paused || now < next_move_ms) return;

    step();
    int period = 150 - score / 4;
    if (period < 60) period = 60;
    next_move_ms = now + (uint32_t)period;
    XtCallActionProc(w, "mark_dirty", NULL, NULL, 0);
}

void snake_key_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)user;
    draw_canvas_input_t *in = (draw_canvas_input_t*)class;
    if (!in || !in->event || !in->canvas) return;

    draw_canvas_t *c = in->canvas;
    ensure_init(c);

    KeySym ks = XLookupKeysym((XKeyEvent*)in->event, 0);
    int want = -1;
    switch (ks) {
    case XK_Right: case XK_d: case XK_D: want = 0; break;
    case XK_Down:  case XK_s: case XK_S: want = 1; break;
    case XK_Left:  case XK_a: case XK_A: want = 2; break;
    case XK_Up:    case XK_w: case XK_W: want = 3; break;
    }

    if (want >= 0) {
        if (alive && !paused && ((want + 2) & 3) != dir)
            next_dir = want;
        return;
    }

    switch (ks) {
    case XK_p: case XK_P:
        if (alive) {
            paused = !paused;
            XtCallActionProc(w, "mark_dirty", NULL, NULL, 0);
        }
        break;
    case XK_Return: case XK_space: case XK_n: case XK_N:
        if (!alive) {
            reset();
            XtCallActionProc(w, "mark_dirty", NULL, NULL, 0);
        }
        break;
    case XK_Escape:
        if (c->app_context) XtAppSetExitFlag(c->app_context);
        break;
    }
}

void snake_mouse_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)user;
    draw_canvas_input_t *in = (draw_canvas_input_t*)class;
    if (!in || !in->event || !in->canvas) return;
    if (in->event->type != ButtonPress || in->event->xbutton.button != Button1) return;

    ensure_init(in->canvas);
    if (!alive) reset();
    else paused = !paused;
    XtCallActionProc(w, "mark_dirty", NULL, NULL, 0);
}

void snake_plugin_init(Widget top)
{
    RCB(top, snake_draw_cb);
    RCB(top, snake_key_cb);
    RCB(top, snake_mouse_cb);
    RCB(top, snake_tick_cb);
}
