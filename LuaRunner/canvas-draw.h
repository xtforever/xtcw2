#ifndef CANVAS_DRAW_H
#define CANVAS_DRAW_H

#include <X11/Intrinsic.h>
#include <X11/Xft/Xft.h>

typedef struct canvas_draw_st {
    GC gc[2];
    XftFont* xfont;
    XftColor xcol[2];
    XftDraw *xdraw;
    Display *dpy;
    int screen;
    unsigned win_width,win_height,
        sl_posx, sl_posy,
        world_width, world_height;

} canvas_draw_t;

void canvas_draw_cb( Widget w, XtPointer user, XtPointer class );

#endif
