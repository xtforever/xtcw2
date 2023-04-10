#include "canvas-draw.h"
#include "mls.h"
#include "font.h"
#include <X11/Xft/Xft.h>



void canvas_draw_cb( Widget w, XtPointer user, XtPointer class )
{
  canvas_draw_t *c = class;
  Drawable d = XftDrawDrawable( c->xdraw );
  XDrawLine( c->dpy, d, c->gc[0], 0,0, c->win_width, c->win_height );
  TRACE(3,"%u,%u ", c->sl_posx, c->sl_posy);
}

typedef struct draw2_st {
    int init;
    XftFont *sym, *big, *small;
} draw2_t;

static draw2_t draw2 = {0};

void draw_row(canvas_draw_t *c) {
    if(! draw2.init ) {
	draw2.sym = font_load( c->dpy, c->screen, "FontAwesome-12" );
    }
        
}
