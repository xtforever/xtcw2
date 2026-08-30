#ifndef XT_TEXT_WRAP_H
#define XT_TEXT_WRAP_H

#include <X11/Intrinsic.h>
#include <X11/Xft/Xft.h>

int t2_write_to_pixmap( Display *dpy, Pixmap p,  XftColor *foreground, XftFont *font, int width, int height, char *txt, int char_width, int x0, int y0 );


#endif
