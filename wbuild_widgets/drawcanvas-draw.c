/*
 * DrawCanvas plug-in: the drawing function.
 *
 * This is deliberately a plain C file with no wbuild/nanosvg dependency.
 * The only contract is the Xt callback signature; the widget passes the
 * canvas_draw_t context as call_data. Swap this file for another drawing
 * implementation and the widget does not need to change.
 */
#include "drawcanvas-draw.h"
#include "wcreg2.h"
#include <X11/Xft/Xft.h>
#include <string.h>

void drawcanvas_draw_cb( Widget w, XtPointer user, XtPointer class )
{
    draw_canvas_t *c = (draw_canvas_t*) class;
    if( !c ) return;

    Drawable d   = XftDrawDrawable( c->xdraw );
    Display *dpy = c->dpy;
    unsigned ww  = c->win_width  ? c->win_width  : 400;
    unsigned hh  = c->win_height ? c->win_height : 300;
    if( ww < 2 ) ww = 400;
    if( hh < 2 ) hh = 300;

    /* background */
    XFillRectangle( dpy, d, c->gc[1], 0, 0, ww, hh );

    /* cross + rectangle + center marker */
    XDrawLine( dpy, d, c->gc[0], 0, 0, ww, hh );
    XDrawLine( dpy, d, c->gc[0], ww, 0, 0, hh );
    XDrawRectangle( dpy, d, c->gc[0], ww/4, hh/4, ww/2, hh/2 );
    XFillRectangle( dpy, d, c->gc[0], ww/2 - 12, hh/2 - 12, 24, 24 );

    /* label */
    if( c->xfont && c->xdraw ) {
        const char *txt = "DrawCanvas plug-in";
        XftDrawStringUtf8( c->xdraw, &c->xcol[1], c->xfont,
                           8, 20, (const FcChar8*) txt, (int) strlen(txt) );
    }
}

void drawcanvas_plugin_init( Widget top )
{
    RCB( top, drawcanvas_draw_cb );
}
