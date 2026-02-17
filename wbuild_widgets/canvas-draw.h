#ifndef CANVAS_DRAW_H
#define CANVAS_DRAW_H

#include <X11/Intrinsic.h>
#include <X11/Xft/Xft.h>

typedef struct canvas_draw_st {
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
  
} canvas_draw_t;

void canvas_draw_cb( Widget w, XtPointer user, XtPointer class );
void canvas_zoom( Widget w, canvas_draw_t *c, float scale );

// shoould be moved into separate files
void xim2_init(  Widget top );


void modif_call1( char *name, Widget w, int args );
void modif_init(void);
void modif_destruct(void);


#endif
