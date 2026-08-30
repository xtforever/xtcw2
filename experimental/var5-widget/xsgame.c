#include <X11/Intrinsic.h>
#include "mls.h"
#include "xutil.h"
#include <math.h>
#define foreground_pixel 0x1E00FE
#define background_pixel 0x221101

static struct XSG_St {
    Widget w;
    Display *dpy;
    GC gc[2];

    int width,height;
} XSG;


struct pix_st {
  char *fileName;
  Pixmap map,mask;
  XRectangle rect;
  int depth;
};


struct ship_one {
    int x, y, w, h;
    struct pix_st pix;
    GC gc;
    float px,py,dx,dy;
    void (*ship_move)(struct ship_one *s);
};

int SHIPS=0;


static void create_gc(void)
{
  XGCValues     values = {
  		foreground: background_pixel,
  		graphics_exposures: False,
                line_width: 1 };
  uint mask = GCForeground | GCGraphicsExposures | GCLineWidth;
  XSG.gc[0] = XtGetGC(XSG.w, mask, &values);
  values.foreground = foreground_pixel;
  XSG.gc[1] = XtGetGC(XSG.w,mask,&values);
}





static void load_ship1( struct ship_one *s, char *name )
{
    int e = load_pixmap_from_file(
				  XSG.w,
				  name,
				  & s->pix.map,
				  & s->pix.mask
				  );
    if(e) {
	ERR("%s not found", name );
    }
    int x, y;
    unsigned int width, height, bw, depth;
    Window w;

    e=XGetGeometry(XSG.dpy,
		   s->pix.map,
		   &w, &x, &y, &width, &height, &bw, &depth);

    s->x=100;
    s->y=80;
    s->w = width;
    s->h = height;
    s->px=0;
    s->py=0;
    s->dy=3;
    s->dx=5;
}


static void x_ship1_move( struct ship_one *s )
{    
    s->px += s->dx;
    if( s->px >= 6.28 ) s->px = 0;
    s->x = 100 + 50 * cos(s->px);
    s->py += s->dy;
    if( s->py >= 6.28 ) s->py = 0;
    s->y = 100 + 50 * sin(s->py);

    if( s->px >= XSG.width
	|| s->px < 0
	|| s->py < 0 
	|| s->py > XSG.height )
	{
	    s->dx = -s->dx;
	    s->dy = -s->dy;
	}
}

unsigned xrand(unsigned max)
{
    unsigned p = random() % max;
    if( p >= max ) ERR("p>max");
    return p;
}

static void x_ship2_move( struct ship_one *s )
{    

    s->x += s->dx;
    s->y += s->dy;

    if( s->x + s->w >= XSG.width ||
	s->x <= 0 ) 
	s->dx = -s->dx;
    
    if( s->y + s->h >= XSG.height ||
	s->y <= 0 )
	{
	    s->dy = -s->dy;
	}


    if( --s->py < 0 ) {
	s->py = xrand( XSG.height );
	float z = xrand(6);	
	s->dy += (3.0 - z);
	printf("%f %f %f\n", s->py, s->dy, z );
    }

    if( --s->px < 0 ) {
	s->px = xrand( XSG.width );
	float z = xrand(8);
	s->dx += (4.0 - z);
	printf("%f %f %f\n", s->px, s->dx, z );
    }
    
}




void xsg_init(Widget w, int dx, int dy)
{
    if( XSG.w != w ) {
	XSG.w = w;
	XSG.dpy = XtDisplay(w);
	create_gc();

	SHIPS =m_create(3,sizeof(struct ship_one));

	struct ship_one *s = m_add(SHIPS);
	load_ship1( s, "pngwing.com.xpm");
	s->ship_move = x_ship1_move;
	s->dx = .1;
	s->dy = .07;
	
	for(int i=0;i<10;i++) {
	
	    s = m_add(SHIPS);
	    load_ship1( s, "ship.xpm");
	    s->ship_move = x_ship2_move;
	    s->x = xrand(dx - s->w);
	    s->y = xrand(dy - s->h);
	}
	
    }
    
    XSG.width  = dx;
    XSG.height = dy;
    
}

void xsg_free(void)
{
    m_free(SHIPS);
}

void xsg_update()
{
    struct ship_one *d;
    int p;
    m_foreach(SHIPS,p,d) {
	d->ship_move(d);
    }
}

void xsg_click(int x, int y)
{
    struct ship_one *d = mls(SHIPS,1);
    d->x = x; d->y = y;
}


void xsg_render(Drawable d)
{
    xsg_update();

    
    XGCValues     values = {
    foreground: 0x29e3f,
    graphics_exposures: False,
    line_width: 1 };
    uint mask = GCForeground | GCGraphicsExposures | GCLineWidth;
    GC gc = XtGetGC(XSG.w, mask, &values);
    
    XFillRectangle(XSG.dpy,d,
		   gc, 0,0, 20,20 );

    // SHIP1.gc = XCreateGC(XSG.dpy,d,0,NULL );
    struct ship_one *s;
    int p;
    m_foreach(SHIPS,p,s) {    
	XSetClipMask(XSG.dpy, gc,  s->pix.mask );
	XSetClipOrigin(XSG.dpy, gc, s->x, s->y );    
	XCopyArea(XSG.dpy, s->pix.map, d, gc,
	      0,0, s->w, s->h, /* source pixmap */
              s->x,s->y ); /* target window x,y */
    }
    XtReleaseGC(XSG.w, gc);    
}

