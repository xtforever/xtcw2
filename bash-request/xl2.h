#ifndef XL2
#define XL2

#include <X11/Intrinsic.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include "mls.h"

int xl2_init( Widget w, XftFont *fnt, XftColor *fg, XftColor *bg , char *s);
int xl2_draw_centered(int ctx, XftDraw *draw, XRectangle *r);

#endif
