#ifndef HB_EXAMPLE_H
#define HB_EXAMPLE_H

#include <X11/Intrinsic.h>
#include <X11/xpm.h>

int write_to_pixmap( Display *dpy, Pixmap p, Pixel foreground, Pixel background, int width, int height );

#endif
