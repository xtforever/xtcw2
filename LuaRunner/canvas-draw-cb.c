#include "canvas-draw.h"
#include "mls.h"


void canvas_draw_cb( Widget w, XtPointer user, XtPointer class )
{
  canvas_draw_t *c = class;
  Drawable d = XftDrawDrawable( c->xdraw );
  XDrawLine( c->dpy, d, c->gc[0], 0,0, c->win_width, c->win_height );
  TRACE(3,"%u,%u ", c->sl_posx, c->sl_posy);
}
