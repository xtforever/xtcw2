#include <X11/Intrinsic.h>
#include "mls.h"

static struct XSG_St {
    Widget w;
    Display *dpy;
} XSG;


struct {
  char *fileName;
  Pixmap map,mask;
  XRectangle rect;
  int depth;
} pix_st;



void xsg_init(Widget w)
{
    XSG.w = w;
    XSG.dpy = XtDisplay(w);
}

void xsg_free(void)
{

}

void xsg_update()
{

}

void xsg_click(int x, int y)
{
}


void xsg_render(Drawable d)
{
    XGCValues     values = {
    foreground: 0x29e3f,
    graphics_exposures: False,
    line_width: 1 };
    uint mask = GCForeground | GCGraphicsExposures | GCLineWidth;
    GC gc = XtGetGC(XSG.w, mask, &values);
    
    XFillRectangle(XSG.dpy,d,
		   gc, 0,0, 20,20 );

    XtReleaseGC(XSG.w, gc);
    
}

