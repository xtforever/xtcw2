#include "canvas-draw.h"
#include "xtcw/Canvas.h"
#include "mls.h"
#include "font.h"
#include <X11/Xft/Xft.h>
#include <stdint.h>
#include "timer.h"
#include "wcreg2.h"



#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"


#include "ximage2.h"



int  xim2_load_svg( XImage2 *im, const char *filename );
void xim2_render( XImage2 *im, Display *dpy, Visual *vis, Drawable d, float scale, int w0 );
int xim2_put_image(  XImage2 *im, int clip_x, int clip_y,
		     int src_x, int src_y,
		     int dest_x, int dest_y, int dest_w, int dest_h );

XImage *make_bitmap ( Display *dpy, Visual *win_vis, uint32_t *img, int width, int height );
int xim2_rasterize( XImage2 *im, float scale, int w0, int h0 );
int xim2_create_ximage(  XImage2 *im );


void xim2_init(  Widget top )
{
     RCB( top, canvas_draw_cb );
}



int  xim2_load_svg( XImage2 *im, const char *filename )
{

  if(im->nsvg) {
    nsvgDelete(im->nsvg);
    im->nsvg=NULL;
  }
  
  // const char* filename = "nanosvg/example/23.svg";
  // const char* filename = "SVG/warning-sign-9743.svg";
  TRACE(3,"parsing %s\n", filename);
  NSVGimage *image = nsvgParseFromFile(filename, "px", 96.0f);
  if (image == NULL) {
    WARN("Could not open SVG image\n");
    return 1;
  }

  im->orig_w =  (int)image->width;
  im->orig_h = (int)image->height;
  im->nsvg = image;
  return 0;
}

void xim2_render( XImage2 *xim, Display *dpy, Visual *vis, Drawable d, float scale, int w0 )
{  
  if(!xim || ! xim->nsvg ) return;

  xim->dpy=dpy;
  xim->vis=vis;
  xim->d=d;
  
  if( w0 != 0 ) 
     scale = w0 * 1.0 / xim->orig_w;
  
  if(  xim2_rasterize(  xim, scale,  xim->orig_w * scale,  xim->orig_h * scale )) return;
  
  xim2_create_ximage( xim );
}

int xim2_rasterize( XImage2 *im, float scale, int w0, int h0 )
{
  im->rast = nsvgCreateRasterizer();
  if (im->rast == NULL) {
    WARN("Could not init rasterizer.\n");
    return 1;
  }

  free( im->rgba_data ); im->rgba_data=NULL;
  
  uint32_t *img = malloc(w0*h0*4);
  if (img == NULL) {
    ERR("Could not alloc image buffer.\n");
    exit(1);
  }

  im->rgba_data = img;
  printf("rasterizing image %d x %d\n", w0, h0);
  im->width = w0;
  im->height = h0;
  nsvgRasterize( im->rast, im->nsvg, 0,0,scale, (u8*)img, w0, h0, w0*4);
  nsvgDeleteRasterizer(im->rast); im->rast=NULL;
  return 0;
}

int xim2_create_ximage(  XImage2 *im )
{

  Display *dpy=im->dpy;
  Visual *vis=im->vis;
  Drawable d = im->d;
    
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
  im->loaded = 0;
  return 0;
}

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



void draw_svg( Widget w, XtPointer class )
{
  canvas_draw_t *c = canvas_get_priv(w);
  if(!c)return;

  Drawable d = XftDrawDrawable( c->xdraw );
  Display *dpy = c->dpy;
  int screenno = XDefaultScreen(dpy);
  Visual *visual = XDefaultVisual(dpy,screenno);


  if( !c->priv_data ) {
    c->priv_data = calloc( 1, sizeof( XImage2 ));
  }
  XImage2 *xim = c->priv_data;
  if(! xim->loaded ) {
    xim2_load_svg( xim, "nanosvg/example/23.svg" );
    xim2_render( xim, dpy, visual, d, 1.0, 0 );
    xim->loaded=1;
  }

  
  XFillRectangle(  c->dpy, d, c->gc[1], 0,0, c->win_width, c->win_height ); 
  XDrawLine( c->dpy, d, c->gc[0], 0,0, c->win_width, c->win_height );
  TRACE(3,"%u,%u ", c->sl_posx, c->sl_posy );
    
  int slx =  c->sl_posx / 100;
  int sly =  c->sl_posy / 100; 
  // int slx1 = slx + c->win_width;
  // int sly1 = sly + c->win_height;
  

  int logo_x = 800;
  int logo_y = 400;
  int logo_w = xim->width;
  int logo_h = xim->height;
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
  
  int dst_w = xim->width;
  int dst_h = xim->height;
  if( dst_w + dst_x > c->win_width ) dst_w = c->win_width - dst_x;
  if( dst_h + dst_y > c->win_height) dst_h = c->win_height- dst_y;
  
  xim2_put_image( xim, logo_x, logo_y, src_x, src_y,  dst_x, dst_y, dst_w, dst_h );
  printf("draw %u x %u\n",   dst_w, dst_h );
}



#define XTPTR(x) ((XtPointer)(intptr_t)(x)) 

int compress_redraw_list = 0;

struct compress_redraw_st {
  void (*func) (Widget,void *);
  void *class;
  Widget widget;
  int last_call;
  XtIntervalId id;  
};


void compress_redraw_cb(XtPointer client_data, XtIntervalId *)
{
  int p = (intptr_t)  client_data;
  struct compress_redraw_st *n = mls(compress_redraw_list,p);
  n->id = 0;
  n->last_call = timer_get_ms();
  if( n->func ) n->func( n->widget, n->class );
}

/* TODO! der pkey sollte widget+func sein */
void compress_redraw(Widget w, void (*func) (Widget,void *), void *class, int timeout )
{
  XtAppContext  CTX = XtWidgetToApplicationContext(w);

  if(! compress_redraw_list )
    compress_redraw_list = m_create(10,
				    sizeof(struct compress_redraw_st));    
  int p = m_lookup_obj( compress_redraw_list, &func, sizeof(void*) );
  struct compress_redraw_st *n = mls(compress_redraw_list,p);

  /* timer vorhanden, dann expose ignorieren aber
     class aktualisieren damit der letzte expose den richtigen context hat.
     ist bei zoom interessant
  */
  n->class = class;
  if( n->id ) return;

  
  
  int cur = timer_get_ms();
  int diff = cur - n->last_call;
  n->func = func;
  n->widget = w;
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
  compress_redraw(w,draw_svg,class,300);
}

void canvas_zoom( Widget w, canvas_draw_t *c, float scale )
{
  TRACE(3,"zoom to %f", scale );
  XImage2 *xim = c->priv_data;
  Drawable d = XftDrawDrawable( c->xdraw );
  Display *dpy = c->dpy;
  int screenno = XDefaultScreen(dpy);
  Visual *visual = XDefaultVisual(dpy,screenno);
  xim2_render( xim, dpy,visual, d, scale, 0 );
  canvas_draw_cb( w,NULL,c);
}

void mod_zoom(int n);

typedef struct modif_st {
  char *name;
  void *context;
  Widget widget;
  int args;
  void (*func) (int pos);
} modif_t;

int MODIF = 0;

modif_t* modif_get(int n)
{
  if( ! MODIF || n >= m_len(MODIF) ) {
    ERR( "modif num %d does not exists", n);
  }
  return mls(MODIF,n);
}


void modif_destruct(void)
{
    m_free_strings(MODIF, 0);
}

void modif_add( char *name, void (*func)(int) )
{
    int p = m_lookup_str(MODIF, name, 0 );
    struct modif_st *cp = mls(MODIF, p);
    cp->func = func;
}

void modif_init(void)
{
    MODIF = m_create( 10, sizeof(struct modif_st));
    modif_add( "zoom", mod_zoom );
}


void modif_call1( char *name, Widget w, int args )
{
  int p = m_lookup_str(MODIF, name, 1 );
  if( p < 0 ) {
    WARN("function %s not found", name );
  }
      
  struct modif_st *cp = mls(MODIF, p);
  cp->widget = w;
  cp->args = args;
  cp->func(p);  
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


/*
   export_functions

   typedef struct mod_if_st {
   char *name;
   void *context;
   Widget w;
   int args;
   void (*func) (int pos)
   } mod_if;


   callF( "name", Widget w, String *args, Cardinal num_args )
   
   callback load_picture
      fn = xtgetvalue( "*filename", "label" )
      callF( "load_pict", 0, { name:fn, target:"*display_widget"  } )


   load_pict( int p )
   
      char *s = arg_get(args,  "name", 0 );
      if( !s ) return;
      Widget w = arg_get_widget( args, "target", 0 );
      if( !w ) return;
      canvas_draw( w, s, 1.0, 0 );
*/




float arg_get_float(int args, char *name, float def_return )
{
  int p;
  char **key,**value;

  for( p=-1; m_next(args, &p, &key ) && m_next(args, &p, &value );  ) {
    if( strcmp( *key, name ) == 0 ) {
      return strtof(*value,NULL);      
    }
  }
  return def_return;
}



void canvas_xim2_zoom( Widget w, void *class )
{
  float scale = ((intptr_t)class) / 1000.0;
  canvas_draw_t *c = canvas_get_priv(w);
  if(!c || !c->priv_data ) return;
  XImage2 *xim = c->priv_data; 
  Drawable d = XftDrawDrawable( c->xdraw );
  Display *dpy = c->dpy;
  int screenno = XDefaultScreen(dpy);
  Visual *visual = XDefaultVisual(dpy,screenno);
  xim2_render( xim, dpy,visual, d, scale, 0 );
  canvas_draw_cb( w, NULL, c);
}

      
/* 1. create a wrapper for your code */
void mod_zoom(int n)
{
  modif_t *mod = modif_get(n);
  TRACE(1,"MODIF %d", n );

  canvas_draw_t* c = canvas_get_priv( mod->widget );
  if(!c) return;

  float scale = arg_get_float( mod->args, "scale", 1 );
  TRACE(3,"zoom to %f", scale );
  scale *= 1000;
  
  compress_redraw( mod->widget, canvas_xim2_zoom, XTPTR(scale), 300 );
}

