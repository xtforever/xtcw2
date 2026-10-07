#ifndef DRAWCANVAS_DRAW_H
#define DRAWCANVAS_DRAW_H

#include <X11/Intrinsic.h>
#include <X11/Xft/Xft.h>

/* Drawing context handed to the plug-in callback (call_data).
 * A renamed copy of canvas-draw.h so the DrawCanvas example is self-contained
 * and does not clash with the original Canvas widget's canvas_draw_t. */
typedef struct draw_canvas_st {
    GC gc[2];
    XtAppContext app_context;
    XftFont* xfont;
    XftColor xcol[2];
    XftDraw *xdraw;
    Display *dpy;
    int screen;
    unsigned win_width,win_height,
        sl_posx, sl_posy,
        world_width, world_height;

  void *priv_data;

} draw_canvas_t;

/* Plug-in drawing function (registered as the widget's callback). */
void drawcanvas_draw_cb( Widget w, XtPointer user, XtPointer class );

/* Registers the plug-in callback name with Wcl. Call once with the shell. */
void drawcanvas_plugin_init( Widget top );

#endif
