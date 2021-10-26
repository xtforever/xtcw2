#ifndef HB_EXAMPLE_H
#define HB_EXAMPLE_H

#include <X11/Intrinsic.h>
#include <X11/xpm.h>
#include <X11/Xft/Xft.h>


enum {
    ENGLISH=0, ARABIC, CHINESE
};

int write_to_pixmap( Display *dpy, Pixmap p,  Pixel foreground, XftFont *font, int width, int height, int id_lang, char *txt , int x0, int y0 );

int hb_measure( Display *dpy, XftFont *font, int id_lang, char *txt, int *w, int *h );
#endif
