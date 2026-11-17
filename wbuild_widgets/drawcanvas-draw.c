/*
 * DrawCanvas plug-in: a tiny interactive paint surface.
 *
 * Mouse drag draws strokes, C clears the canvas, Esc quits. The widget owns
 * the timer: this plug-in only marks the canvas dirty, and DrawCanvas repaints
 * once per update interval (see :updateInterval). That keeps a fast mouse from
 * generating one XFlush per motion event.
 */
#include "drawcanvas-draw.h"
#include "wcreg2.h"
#include <X11/Xft/Xft.h>
#include <X11/keysym.h>
#include <string.h>

#define MAX_STROKES 4096

static XSegment strokes[MAX_STROKES];
static int nstrokes;
static int drawing;
static int last_x, last_y;
static GC paint_gc;

static void mark_dirty(Widget w)
{
    XtCallActionProc(w, "mark_dirty", NULL, NULL, 0);
}

static void add_stroke(Widget w, int x0, int y0, int x1, int y1)
{
    if (nstrokes >= MAX_STROKES) return;
    strokes[nstrokes].x1 = (short)x0;
    strokes[nstrokes].y1 = (short)y0;
    strokes[nstrokes].x2 = (short)x1;
    strokes[nstrokes].y2 = (short)y1;
    nstrokes++;
    mark_dirty(w);
}

void drawcanvas_draw_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)user;
    draw_canvas_t *c = (draw_canvas_t*)class;
    if (!c || !c->xdraw) return;

    Drawable d = XftDrawDrawable(c->xdraw);
    Display *dpy = c->dpy;
    unsigned ww = c->win_width  ? c->win_width  : 400;
    unsigned hh = c->win_height ? c->win_height : 300;
    if (ww < 2) ww = 400;
    if (hh < 2) hh = 300;

    /* background */
    XFillRectangle(dpy, d, c->gc[1], 0, 0, ww, hh);

    if (!paint_gc) paint_gc = XCreateGC(dpy, d, 0, NULL);
    XSetForeground(dpy, paint_gc, c->xcol[1].pixel);
    if (nstrokes)
        XDrawSegments(dpy, d, paint_gc, strokes, nstrokes);

    if (c->xfont && c->xdraw) {
        const char *txt = "Drag to draw  -  C clears  -  Esc quits";
        XftDrawStringUtf8(c->xdraw, &c->xcol[1], c->xfont,
                          8, c->xfont->ascent + 6,
                          (const FcChar8*) txt, (int)strlen(txt));
    }
}

void drawcanvas_mouse_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)user;
    draw_canvas_input_t *in = (draw_canvas_input_t*)class;
    if (!in || !in->event) return;

    XEvent *e = in->event;
    if (e->type == ButtonPress) {
        if (e->xbutton.button != Button1) return;
        drawing = 1;
        last_x = e->xbutton.x;
        last_y = e->xbutton.y;
        mark_dirty(w);
    } else if (e->type == MotionNotify) {
        if (!drawing && !(e->xmotion.state & Button1Mask)) return;
        if (!drawing) {
            drawing = 1;
            last_x = e->xmotion.x;
            last_y = e->xmotion.y;
        } else {
            add_stroke(w, last_x, last_y, e->xmotion.x, e->xmotion.y);
            last_x = e->xmotion.x;
            last_y = e->xmotion.y;
        }
    } else if (e->type == ButtonRelease) {
        if (e->xbutton.button == Button1) drawing = 0;
    }
}

void drawcanvas_key_cb(Widget w, XtPointer user, XtPointer class)
{
    (void)user;
    draw_canvas_input_t *in = (draw_canvas_input_t*)class;
    if (!in || !in->event) return;

    KeySym ks = XLookupKeysym((XKeyEvent*)in->event, 0);
    if (ks == XK_c || ks == XK_C) {
        nstrokes = 0;
        mark_dirty(w);
    } else if (ks == XK_Escape && in->canvas && in->canvas->app_context) {
        XtAppSetExitFlag(in->canvas->app_context);
    }
}

void drawcanvas_plugin_init(Widget top)
{
    RCB(top, drawcanvas_draw_cb);
    RCB(top, drawcanvas_key_cb);
    RCB(top, drawcanvas_mouse_cb);
}
