#include "canvas-draw.h"
#include "mls.h"
#include "font.h"
#include <X11/Xft/Xft.h>
#include <stdint.h>
#include "timer.h"



#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"


/*
XSetFunction(dpy, gc, GXandInverted);
XSetBackground(dpy, gc, 0UL);
XSetForeground(dpy, gc, ~0UL);
XCopyPlane(dpy, mask, win, gc, ..., 1UL);
XSetFunction(dpy, gc, GXor);
XCopyArea(dpy, image, win, gc, ...);
*/


typedef unsigned char u8;
typedef uint32_t img_pixel_t;

XImage *make_bitmap ( Display *dpy, Visual *win_vis, uint32_t *img, int width, int height );

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


int  xim2_load_svg( XImage2 *im, const char *filename )
{
  NSVGimage *image = NULL;
  // const char* filename = "nanosvg/example/23.svg";
  // const char* filename = "SVG/warning-sign-9743.svg";
  TRACE(3,"parsing %s\n", filename);
  image = nsvgParseFromFile(filename, "px", 96.0f);
  if (image == NULL) {
    WARN("Could not open SVG image\n");
    return 1;
  }
  im->orig_w =  (int)image->width;
  im->orig_h = (int)image->height;
  im->nsvg = image;
  return 0;
}

int xim2_rasterize( XImage2 *im, float scale, int w0, int h0 )
{
  im->rast = nsvgCreateRasterizer();
  if (im->rast == NULL) {
    WARN("Could not init rasterizer.\n");
    return 1;
  }
  
  uint32_t *img = malloc(w0*h0*4);
  if (img == NULL) {
    printf("Could not alloc image buffer.\n");
    return 1;
  }
  im->rgba_data = img;
  printf("rasterizing image %d x %d\n", w0, h0);
  im->width = w0;
  im->height = h0;
  nsvgRasterize( im->rast, im->nsvg, 0,0,scale, (u8*)img, w0, h0, w0*4);
  nsvgDeleteRasterizer(im->rast); im->rast=NULL;
  nsvgDelete(im->nsvg); im->nsvg=NULL;
  return 0;
}

int xim2_create_ximage(  XImage2 *im, Display *dpy, Visual *vis, Drawable d )
{

  im->dpy=dpy;
  im->vis=vis;
  im->d=d;
  
  XImage *mask_image = make_bitmap( dpy, vis, im->rgba_data, im->width, im->height );
  XImage *image = XCreateImage(dpy, vis, DefaultDepth(dpy, DefaultScreen(dpy)),
			       ZPixmap, 0, (char *)im->rgba_data, im->width, im->height, 32, 0);
  im->rgba_data = NULL; 
  // Create the mask pixmap and copy the silhouette onto it.
  Pixmap mask = XCreatePixmap (dpy, d, im->width, im->height, 1);
  GC mask_gc = XCreateGC (dpy, mask, 0, 0);
  XSetBackground (dpy, mask_gc, 0);
  XSetForeground (dpy, mask_gc, 1);
  XPutImage (dpy, mask, mask_gc, mask_image, 0, 0, 0, 0, im->width, im->height);
  XFreeGC (dpy, mask_gc);
  XDestroyImage (mask_image);
  im->mask = mask;
  im->image = image;
  return 0;
}

int xim2_put_image(  XImage2 *im, int clip_x, int clip_y,
		     int src_x, int src_y,
		     int dest_x, int dest_y, int dest_w, int dest_h )
{
  
  GC gc = XDefaultGC(im->dpy,XDefaultScreen(im->dpy));
  XSetClipMask (im->dpy, gc, im->mask);
  XSetClipOrigin (im->dpy, gc, clip_x, clip_y );  
  XPutImage(im->dpy, im->d, gc, im->image, src_x,src_y,   dest_x, dest_y, dest_w, dest_h );
  XSetClipMask (im->dpy, gc, None);
  return 0;
}

int xim2_delete( XImage2 *im )
{
  if( im->image ) {
    XDestroyImage ( im->image );
    im->image=NULL;
  }
  
  if(im->rast) {
    nsvgDeleteRasterizer(im->rast); im->rast=NULL;
  }
  
  if(im->nsvg) {
    nsvgDelete(im->nsvg); im->nsvg=NULL;
  }
  

  if( im->mask ) {
    XFreePixmap( im->dpy, im->mask );
    im->mask = 0;
  }

  free( im->rgba_data );
  im->rgba_data = NULL;
  return 0;
}

  /* color-coding: siehe NSVG_RGB macro in nanosvg.h */

  /* uint32_t *g = (uint32_t *)img; */
  /* for(int i=0;i<w0*h0;i++) { */
  /*   char *x=(char *)(g+i); */
  /*   if( g[i] == 0 ) x[3] = 0xff; */
  /* } */
  /* FILE *fp = fopen("dump.bmp", "wb" ); */
  /* fwrite( img, 1, w0*h0*4, fp ); */
  /* fclose(fp); */



/* int load2( canvas_draw_t *c, XImage2 *ret_image ) */
/* { */

/*   if(ret_image->loaded ) return 0; */
    
/*   Display *dpy = c->dpy; */
/*   int screenno = XDefaultScreen(dpy); */
/*   Visual *visual = XDefaultVisual(dpy,screenno); */
/*   GC gc = XDefaultGC(dpy,screenno); */
/*   Drawable d = XftDrawDrawable( c->xdraw ); */
  
/*   int width,height; */
/*   char *rgba_data = load1( &width, &height ); */
/*   if( ! rgba_data ) return NULL; */

  
/*   XImage *mask_image = make_bitmap( dpy, visual, rgba_data, width, height ); */
  
/*   XImage *image = XCreateImage(dpy, visual, DefaultDepth(dpy, DefaultScreen(dpy)), */
/*                                  ZPixmap, 0, (char *)rgba_data, width, height, 32, 0); */

/*   if (image == NULL) { */
/*     fprintf(stderr, "Failed to create XImage.\n"); */
/*     exit(EXIT_FAILURE); */
/*   } */


/*   // Create the mask pixmap and copy the silhouette onto it. */
/*   Pixmap mask = XCreatePixmap (dpy, d, width, height, 1); */
/*   GC mask_gc = XCreateGC (dpy, mask, 0, 0); */
/*   XSetBackground (dpy, mask_gc, 0); */
/*   XSetForeground (dpy, mask_gc, 1); */
/*   XPutImage (dpy, mask, mask_gc, mask_image, 0, 0, 0, 0, width, height); */
/*   XFreeGC (dpy, mask_gc); */
/*   XDestroyImage (mask_image); */
  
/*   ret_image-> image = image; */
/*   ret_image-> mask = mask; */
/*   ret_image-> width = width; */
/*   ret_image-> height = height; */
/*   ret_image->loaded = 1; */
/*   return 0; */
/* } */

/* from: yadex
 *	Sticker_priv::make_bitmap - create 1-bpp XImage from Img
 *
 *	Return pointer on XImage on success, null pointer on
 *	failure.
 */
XImage *make_bitmap ( Display *dpy, Visual *win_vis, uint32_t *img, int width, int height )
{

  // int bitmap_bit_order = BitmapBitOrder (dpy);
  const size_t bitmap_pad     = BitmapPad (dpy);
  const size_t bytes_per_line = (width + bitmap_pad - 1) / bitmap_pad
				* bitmap_pad / 8;
  u8 *buf = (u8 *) calloc ( 1, (unsigned long) bytes_per_line * height);
  if (! buf)
  {
    ERR ("Not enough memory to display %dx%d image", width, height);
    return 0;
  }

  
  uint32_t *src = img;
  uint32_t *src_end = img +  width * height;
  u8 *bmp = buf;
  u8 *buf_end = buf + bytes_per_line * height;

  for( size_t y = 0 ; y < height; y++ ) {
    size_t byte_index = 0;
    register u8 bit = 1;
    u8 d = 0;
    for( size_t x = 0; x < width ; x++ ) {
      if( src >= src_end ) { ERR("internal error src > src_end"); return NULL; }


      if( *src++ != 0 ) d |= bit;
      if( bit == 128 ) {
	bmp[byte_index++] = d;
	d=0;
	bit=1;
      } else {
	bit <<=1;
      } 
    }
    
    if( bit != 1 ) bmp[byte_index] = d;
    
    bmp +=  bytes_per_line;
        
  }
  
  if( bmp != buf_end ) {
    ERR("internal error bmp != buf_end" );
    return NULL;
  }

  /* FILE *fp = fopen("dump1.bmp", "wb" ); */
  /* fwrite( buf, 1, bytes_per_line * height, fp ); */
  /* fclose(fp); */

 
  XImage *ximage = XCreateImage (dpy, win_vis, 1, XYBitmap, 0,
    (char *) buf, width, height, bitmap_pad, bytes_per_line);
  if (ximage == 0)
  {
    ERR ("XCreateImage() returned NULL");
    free (buf);
    return 0;
  }
  return ximage;
}



void draw_svg( XtPointer class )
{
  canvas_draw_t *c = class;
  Drawable d = XftDrawDrawable( c->xdraw );
  Display *dpy = c->dpy;
  int screenno = XDefaultScreen(dpy);
  Visual *visual = XDefaultVisual(dpy,screenno);
  static XImage2 img2 = {0};
    

  if(! img2.loaded ) {
    char *s = "nanosvg/example/23.svg";
    // char *s2 = "SVG/warning-sign-9743.svg"; 
    if( xim2_load_svg( &img2, s) ) return;
    int w0=1800;
    float scale = w0 * 1.0 / img2.orig_w;
    
    if(  xim2_rasterize(  &img2, scale,  img2.orig_w * scale,  img2.orig_h * scale )) return;
    if( xim2_create_ximage( &img2, dpy, visual, d ) ) return; 
      
    img2.last_x = -1;
    img2.loaded=1;
  }

  /* delayed draw:
     if we receive a call in less than 300ms we wait for another 300ms before drawing
     if we receive another call, while we are waiting, we reset the counter
  */

  
  XFillRectangle(  c->dpy, d, c->gc[1], 0,0, c->win_width, c->win_height ); 
  XDrawLine( c->dpy, d, c->gc[0], 0,0, c->win_width, c->win_height );
  TRACE(3,"%u,%u ", c->sl_posx, c->sl_posy );
  
  
  int slx =  c->sl_posx / 100;
  int sly =  c->sl_posy / 100; 
  // int slx1 = slx + c->win_width;
  // int sly1 = sly + c->win_height;
  

  int logo_x = 800;
  int logo_y = 400;
  int logo_w = img2.width;
  int logo_h = img2.height;
  int logo_x1 = logo_x + logo_w;
  int logo_y1 = logo_y + logo_h;

  
  logo_x -= slx;
  logo_x1 -= slx;
  logo_y -= sly;
  logo_y1 -= sly;
  
  printf("%d x %d, %d x %d \n", logo_x, logo_y,  logo_x1,logo_y1 );
  printf("%u x %u\n",  c->win_width, c->win_height );
  
  if( logo_x >=  (int)c->win_width ||
      logo_x1 <= 0 ||
      logo_y >= (int)c->win_height ||
      logo_y1 <= 0 ) return;

  printf("draw:\n");
  int src_x = 0;
  int src_y = 0;
  int dst_x = logo_x;
  int dst_y = logo_y;

  if( logo_x < 0 ) {
    src_x = -logo_x;
    dst_x = 0;
  }
  if( logo_y < 0 ) {
    src_y = -logo_y;
    dst_y = 0;
  }
  
  int dst_w = img2.width;
  int dst_h = img2.height;
  if( dst_w + dst_x > c->win_width ) dst_w = c->win_width - dst_x;
  if( dst_h + dst_y > c->win_height) dst_h = c->win_height- dst_y;
  
   
    
  xim2_put_image( &img2, logo_x, logo_y, src_x, src_y,  dst_x, dst_y, dst_w, dst_h );
  printf("draw %u x %u\n",   dst_w, dst_h );
}



#define XTPTR(x) ((XtPointer)(intptr_t)(x)) 

int compress_redraw_list = 0;

struct compress_redraw_st {
  void (*func) (void *);
  void *class;
  int last_call;
  XtIntervalId id;  
};


void compress_redraw_cb(XtPointer client_data, XtIntervalId *)
{
  int p = (intptr_t)  client_data;
  struct compress_redraw_st *n = mls(compress_redraw_list,p);
  n->id = 0;
  n->last_call = timer_get_ms();
  if( n->func ) n->func( n->class );
}

void compress_redraw(XtAppContext CTX, void (*func) (void *), void *class, int timeout )
{
  if(! compress_redraw_list )
    compress_redraw_list = m_create(10,
				    sizeof(struct compress_redraw_st));    
  int p = m_lookup_obj( compress_redraw_list, &func, sizeof(void*) );
  struct compress_redraw_st *n = mls(compress_redraw_list,p);

  /* timer vorhanden, dann expose ignorieren */
  if( n->id ) return;
  
  int cur = timer_get_ms();
  int diff = cur - n->last_call;
  n->func = func;
  n->class = class;

  /* kein timer aktiv, also entweder
     in die warteschlange oder sofort ausfuehren */
  if( diff < timeout ) {
    n->id=XtAppAddTimeOut(CTX, timeout, compress_redraw_cb, XTPTR(p) );
    return;
  }
  
  compress_redraw_cb(  XTPTR(p), NULL );
}

void canvas_draw_cb( Widget w, XtPointer user, XtPointer class )
{
  compress_redraw(
		  XtWidgetToApplicationContext(w),
		  draw_svg,
		  class,
		  300 );
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
