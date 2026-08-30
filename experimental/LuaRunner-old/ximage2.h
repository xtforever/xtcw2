#ifndef XIMAGE2_H
#define XIMAGE2_H

#include <stdint.h>
#include "mls.h"
#include "font.h"
#include <X11/Xft/Xft.h>

#include "nanosvg.h"
#include "nanosvgrast.h"

typedef unsigned char u8;
typedef uint32_t img_pixel_t;
typedef struct _Ximage2 {
  XImage* image;
  Pixmap mask;
  size_t width,height, orig_w, orig_h;
  NSVGimage *nsvg;
  NSVGrasterizer *rast;
  uint32_t *rgba_data;
  Display *dpy; Visual *vis; Drawable d; 
  int loaded;
  int last_x,last_y;
} XImage2;


int  xim2_load_svg( XImage2 *im, const char *filename );
void xim2_render( XImage2 *im, Display *dpy, Visual *vis, Drawable d, float scale, int w0 );

int xim2_put_image(  XImage2 *im, int clip_x, int clip_y,
		     int src_x, int src_y,
		     int dest_x, int dest_y, int dest_w, int dest_h );

int xim2_delete( XImage2 *im );

#endif

