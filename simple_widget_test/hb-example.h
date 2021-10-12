#ifndef HB_EXAMPLE_H
#define HB_EXAMPLE_H

#include <X11/Intrinsic.h>
#include <X11/xpm.h>
#include <X11/Xft/Xft.h>


enum {
    ENGLISH=0, ARABIC, CHINESE
};


int write_to_pixmap( Display *dpy, Pixmap p, Pixel foreground, XftFont *fn, int width, int height );

#endif
