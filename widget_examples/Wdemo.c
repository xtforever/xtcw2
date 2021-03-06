/* Generated by wbuild
 * (generator version 3.3)
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#line 325 "Wdemo.widget"
#include <assert.h>
#line 326 "Wdemo.widget"
#include <stdint.h>
#line 327 "Wdemo.widget"
#include <X11/Intrinsic.h>
#line 328 "Wdemo.widget"
#include <X11/Xmu/Converters.h>
#line 329 "Wdemo.widget"
#include <X11/Xft/Xft.h>
#line 330 "Wdemo.widget"
#include <X11/Xregion.h>
#line 331 "Wdemo.widget"
#include "converters.h"
#line 332 "Wdemo.widget"
#include "xutil.h"
#line 333 "Wdemo.widget"
#include "mls.h"
#include <xtcw/WdemoP.h>
static void _resolve_inheritance(
#if NeedFunctionPrototypes
WidgetClass
#endif
);
#line 35 "Wdemo.widget"
static void initialize(
#if NeedFunctionPrototypes
Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 46 "Wdemo.widget"
static Boolean  set_values(
#if NeedFunctionPrototypes
Widget ,Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 65 "Wdemo.widget"
static void destroy(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 72 "Wdemo.widget"
static void expose(
#if NeedFunctionPrototypes
Widget,XEvent *,Region 
#endif
);
#line 79 "Wdemo.widget"
static void realize(
#if NeedFunctionPrototypes
Widget,XtValueMask *,XSetWindowAttributes *
#endif
);
#line 89 "Wdemo.widget"
static void resize(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 101 "Wdemo.widget"
static void update(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 142 "Wdemo.widget"
static void draw_parts(
#if NeedFunctionPrototypes
Widget,Region 
#endif
);
#line 155 "Wdemo.widget"
static void copy_rectangles(
#if NeedFunctionPrototypes
Widget,Region 
#endif
);
#line 167 "Wdemo.widget"
static void show_region(
#if NeedFunctionPrototypes
Region 
#endif
);
#line 182 "Wdemo.widget"
static void redraw_value1(
#if NeedFunctionPrototypes
Widget,int ,Region 
#endif
);
#line 197 "Wdemo.widget"
static void redraw_value2(
#if NeedFunctionPrototypes
Widget,int ,Region 
#endif
);
#line 227 "Wdemo.widget"
static void reshape_widget(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 237 "Wdemo.widget"
static void paint1(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 257 "Wdemo.widget"
static void calculate_prefered_size(
#if NeedFunctionPrototypes
Widget,int *,int *
#endif
);
#line 265 "Wdemo.widget"
static void calculate_size(
#if NeedFunctionPrototypes
Widget,int *,int *
#endif
);
#line 272 "Wdemo.widget"
static void make_resize_request(
#if NeedFunctionPrototypes
Widget,int ,int 
#endif
);
#line 279 "Wdemo.widget"
static void realloc_colors(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 296 "Wdemo.widget"
static void free_colors(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 309 "Wdemo.widget"
static void alloc_pixmap(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 101 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 101 "Wdemo.widget"
static void update(Widget self)
#else
#line 101 "Wdemo.widget"
static void update(self)Widget self;
#endif
#line 102 "Wdemo.widget"
{
        int bg_clear = 0;
        Region r = NULL;

        if( ((WdemoWidget)self)->wdemo.changed == 0 ) return;

        TRACE(1, "changed: %d", ((WdemoWidget)self)->wdemo.changed );

        if( ((WdemoWidget)self)->wdemo.changed & CHANGE_SIZE ) {
            ((WdemoWidget)self)->wdemo.changed = CHANGE_ALL;
            alloc_pixmap(self);
        } else {
          if( ((WdemoWidget)self)->wdemo.changed & CHANGE_COLOR ) {
            realloc_colors(self);
            ((WdemoWidget)self)->wdemo.changed = CHANGE_ALL;
          }
        }

        if( (((WdemoWidget)self)->wdemo.changed & CHANGE_ALL) == CHANGE_ALL ) {
          /* clear background */
          XFillRectangle(XtDisplay(self),((WdemoWidget)self)->wdemo.pm, ((WdemoWidget)self)->wdemo.gc[BG], 0,0, ((WdemoWidget)self)->core.width,((WdemoWidget)self)->core.height );
          bg_clear = 1;
        }
        else r = XCreateRegion();

        if( ((WdemoWidget)self)->wdemo.changed & CHANGE_VALUE1 ) {
            redraw_value1( self, bg_clear, r );
        }
        if( ((WdemoWidget)self)->wdemo.changed & CHANGE_VALUE2 ) {
            redraw_value2( self, bg_clear, r );
        }

        draw_parts(self,r);
        ((WdemoWidget)self)->wdemo.changed = 0;
}
#line 142 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 142 "Wdemo.widget"
static void draw_parts(Widget self,Region  r)
#else
#line 142 "Wdemo.widget"
static void draw_parts(self,r)Widget self;Region  r;
#endif
#line 143 "Wdemo.widget"
{
  if( ! ((WdemoWidget)self)->core.visible ) return;
  XTFUNC();
  if( r != NULL )
      copy_rectangles(self,r);
   else /* copy full pixmap */
      XCopyArea(XtDisplay(self),((WdemoWidget)self)->wdemo.pm, XtWindow(self), ((WdemoWidget)self)->wdemo.gc[0],
              0,0,((WdemoWidget)self)->core.width,((WdemoWidget)self)->core.height,  /* source pixmap */
              0,0 ); /* target window x,y */
}
#line 155 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 155 "Wdemo.widget"
static void copy_rectangles(Widget self,Region  r)
#else
#line 155 "Wdemo.widget"
static void copy_rectangles(self,r)Widget self;Region  r;
#endif
#line 156 "Wdemo.widget"
{
   if( !r ) return;
   XTFUNC();

   Box *b=&r->extents;
   XCopyArea(XtDisplay(self),((WdemoWidget)self)->wdemo.pm, XtWindow(self), ((WdemoWidget)self)->wdemo.gc[0],
              b->x1, b->y1, b->x2, b->y2, /* source pixmap */
              b->x1, b->y1 ); /* target window x,y */
}
#line 167 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 167 "Wdemo.widget"
static void show_region(Region  tmp)
#else
#line 167 "Wdemo.widget"
static void show_region(tmp)Region  tmp;
#endif
#line 168 "Wdemo.widget"
{
        if( !tmp ) return;
        Box *b = tmp->rects;
        int n = tmp->numRects;
        printf("Num Rects: %d\n", n );
        while( n-- ) {
               printf("%d,%d,%d,%d\n", b->x1, b->y1, b->x2, b->y2 );
               b++;
        }
        printf("Extents: ");
        b=&tmp->extents;
        printf("%d,%d,%d,%d\n", b->x1, b->y1, b->x2, b->y2 );
}
#line 182 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 182 "Wdemo.widget"
static void redraw_value1(Widget self,int  bg_clear,Region  r)
#else
#line 182 "Wdemo.widget"
static void redraw_value1(self,bg_clear,r)Widget self;int  bg_clear;Region  r;
#endif
#line 183 "Wdemo.widget"
{
        XTFUNC();
        if( ! bg_clear )
          XFillRectangle(XtDisplay(self),((WdemoWidget)self)->wdemo.pm, ((WdemoWidget)self)->wdemo.gc[BG], 0,0, ((WdemoWidget)self)->core.width,((WdemoWidget)self)->core.height/2 );
        paint1(self);

        if( r ) {
            XRectangle rec;
            rec.x = 0; rec.y   = 0;
            rec.width = ((WdemoWidget)self)->core.width; rec.height = ((WdemoWidget)self)->core.height;
            XUnionRectWithRegion(&rec,r,r);
        }
}
#line 197 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 197 "Wdemo.widget"
static void redraw_value2(Widget self,int  bg_clear,Region  r)
#else
#line 197 "Wdemo.widget"
static void redraw_value2(self,bg_clear,r)Widget self;int  bg_clear;Region  r;
#endif
#line 198 "Wdemo.widget"
{
        XTFUNC();
        if( ! bg_clear )
          XFillRectangle(XtDisplay(self),((WdemoWidget)self)->wdemo.pm, ((WdemoWidget)self)->wdemo.gc[BG], 0,((WdemoWidget)self)->core.height/2, ((WdemoWidget)self)->core.width,((WdemoWidget)self)->core.height/2 );

        int x,y;
        char *s = ((WdemoWidget)self)->wdemo.value;
        if( is_empty(s) ) goto skip_draw;

        x = ((WdemoWidget)self)->core.width;
        x -= xft_textwidth( XtDisplay(self), ((WdemoWidget)self)->wdemo.xftFont, s );
        x /= 2;
        y = 0.75 * ((WdemoWidget)self)->core.height - ((WdemoWidget)self)->wdemo.xftFont->height / 2;
        XftDrawStringUtf8(((WdemoWidget)self)->wdemo.draw, ((WdemoWidget)self)->wdemo.xcol + FG, ((WdemoWidget)self)->wdemo.xftFont,
    	    x,y+((WdemoWidget)self)->wdemo.xftFont->ascent, (FcChar8*)s, strlen(s) );

        skip_draw:
          if( r ) {
            XRectangle rec;
            rec.x = 0; rec.y   = ((WdemoWidget)self)->core.height/2;
            rec.width = ((WdemoWidget)self)->core.width; rec.height = ((WdemoWidget)self)->core.height/2;
            XUnionRectWithRegion(&rec,r,r);
          }
}
#line 227 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 227 "Wdemo.widget"
static void reshape_widget(Widget self)
#else
#line 227 "Wdemo.widget"
static void reshape_widget(self)Widget self;
#endif
#line 228 "Wdemo.widget"
{
	int w;
	if( ((WdemoWidget)self)->wdemo.cornerRoundPercent >0 && ((WdemoWidget)self)->wdemo.cornerRoundPercent < 100 ) {
	    w = Min(((WdemoWidget)self)->core.height,((WdemoWidget)self)->core.width);
	    w = w * ((WdemoWidget)self)->wdemo.cornerRoundPercent / 100;
	    XmuReshapeWidget( self, XmuShapeRoundedRectangle, w, w );
	}
}
#line 237 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 237 "Wdemo.widget"
static void paint1(Widget self)
#else
#line 237 "Wdemo.widget"
static void paint1(self)Widget self;
#endif
#line 238 "Wdemo.widget"
{
        XTFUNC();
        char *s = ((WdemoWidget)self)->wdemo.label;
        int x,y,w,h;

        /* center text */
        h = ((WdemoWidget)self)->wdemo.xftFont->height;
        w = xft_textwidth(XtDisplay(self), ((WdemoWidget)self)->wdemo.xftFont, s );
        x = (((WdemoWidget)self)->core.width - w) / 2;
        y = ((((WdemoWidget)self)->core.height/2) -h) / 2;

        XftDrawStringUtf8(((WdemoWidget)self)->wdemo.draw, ((WdemoWidget)self)->wdemo.xcol + FG, ((WdemoWidget)self)->wdemo.xftFont,
    			     x,y+((WdemoWidget)self)->wdemo.xftFont->ascent, (FcChar8*)s, strlen(s) );

        y = ((WdemoWidget)self)->core.height / 2 - ((WdemoWidget)self)->wdemo.lineWidth / 2;
        XDrawLine(XtDisplay(self), ((WdemoWidget)self)->wdemo.pm, ((WdemoWidget)self)->wdemo.gc[FG], 0, y, ((WdemoWidget)self)->core.width, y );

}
#line 257 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 257 "Wdemo.widget"
static void calculate_prefered_size(Widget self,int * w,int * h)
#else
#line 257 "Wdemo.widget"
static void calculate_prefered_size(self,w,h)Widget self;int * w;int * h;
#endif
#line 258 "Wdemo.widget"
{
  calculate_size(self,w,h);
  (*w) += 20;
  (*h) += 20;
}
#line 265 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 265 "Wdemo.widget"
static void calculate_size(Widget self,int * w,int * h)
#else
#line 265 "Wdemo.widget"
static void calculate_size(self,w,h)Widget self;int * w;int * h;
#endif
#line 266 "Wdemo.widget"
{
        xft_calc_string_size(self, ((WdemoWidget)self)->wdemo.xftFont, ((WdemoWidget)self)->wdemo.label, w, h );
        (*h) *= 2.5;
}
#line 272 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 272 "Wdemo.widget"
static void make_resize_request(Widget self,int  w,int  h)
#else
#line 272 "Wdemo.widget"
static void make_resize_request(self,w,h)Widget self;int  w;int  h;
#endif
#line 273 "Wdemo.widget"
{
        Dimension w_out, h_out;
        if( XtMakeResizeRequest(self,w,h, &w_out, &h_out ) ==
            XtGeometryAlmost ) XtMakeResizeRequest(self,w_out,h_out,NULL,NULL );
}
#line 279 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 279 "Wdemo.widget"
static void realloc_colors(Widget self)
#else
#line 279 "Wdemo.widget"
static void realloc_colors(self)Widget self;
#endif
#line 280 "Wdemo.widget"
{
  if( ((WdemoWidget)self)->wdemo.gc[0] != 0 ) free_colors(self);

  XGCValues     values = {
  		foreground: ((WdemoWidget)self)->core.background_pixel,
  		graphics_exposures: False,
                line_width: ((WdemoWidget)self)->wdemo.lineWidth };
  uint          mask = GCForeground | GCGraphicsExposures |  GCLineWidth;
  ((WdemoWidget)self)->wdemo.gc[BG] = XCreateGC(XtDisplay(self), ((WdemoWidget)self)->wdemo.pm, mask,&values);
  values.foreground = ((WdemoWidget)self)->wdemo.foreground;
  ((WdemoWidget)self)->wdemo.gc[FG] = XCreateGC(XtDisplay(self), ((WdemoWidget)self)->wdemo.pm, mask,&values);
  ((WdemoWidget)self)->wdemo.gc[CP] = XCreateGC( XtDisplay(self), ((WdemoWidget)self)->wdemo.pm, 0,0 );
  xft_color_alloc( self, ((WdemoWidget)self)->core.background_pixel, ((WdemoWidget)self)->wdemo.xcol+BG );
  xft_color_alloc( self, ((WdemoWidget)self)->wdemo.foreground,       ((WdemoWidget)self)->wdemo.xcol+FG );
}
#line 296 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 296 "Wdemo.widget"
static void free_colors(Widget self)
#else
#line 296 "Wdemo.widget"
static void free_colors(self)Widget self;
#endif
#line 297 "Wdemo.widget"
{
        int i;
        Display *dpy = XtDisplay(self);
        for(i=0;i<2;i++) {
                XFreeGC(XtDisplay(self),((WdemoWidget)self)->wdemo.gc[i]);
                XftColorFree(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
		 None, ((WdemoWidget)self)->wdemo.xcol+i);
        }
        XFreeGC(XtDisplay(self), ((WdemoWidget)self)->wdemo.gc[CP] );
}
#line 309 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 309 "Wdemo.widget"
static void alloc_pixmap(Widget self)
#else
#line 309 "Wdemo.widget"
static void alloc_pixmap(self)Widget self;
#endif
#line 310 "Wdemo.widget"
{
  Display *dpy = XtDisplay(self);
  if( ((WdemoWidget)self)->wdemo.pm ) XFreePixmap(dpy, ((WdemoWidget)self)->wdemo.pm);
  ((WdemoWidget)self)->wdemo.pm = XCreatePixmap(dpy, XtWindow(self), ((WdemoWidget)self)->core.width, ((WdemoWidget)self)->core.height,
                            DefaultDepth(dpy, DefaultScreen(dpy)));
  if( ((WdemoWidget)self)->wdemo.draw )
        XftDrawChange(((WdemoWidget)self)->wdemo.draw, ((WdemoWidget)self)->wdemo.pm );
  else
        ((WdemoWidget)self)->wdemo.draw = XftDrawCreate(dpy, ((WdemoWidget)self)->wdemo.pm,
                       DefaultVisual(dpy, DefaultScreen(dpy)), None);

  realloc_colors(self); /* the drawable has changed, we need new gc's */
}

static XtResource resources[] = {
#line 5 "Wdemo.widget"
{XtNxftFont,XtCXFtFont,XtRXftFont,sizeof(((WdemoRec*)NULL)->wdemo.xftFont),XtOffsetOf(WdemoRec,wdemo.xftFont),XtRString,(XtPointer)"Sans-22"},
#line 6 "Wdemo.widget"
{XtNforeground,XtCForeground,XtRPixel,sizeof(((WdemoRec*)NULL)->wdemo.foreground),XtOffsetOf(WdemoRec,wdemo.foreground),XtRImmediate,(XtPointer)XtDefaultForeground },
#line 7 "Wdemo.widget"
{XtNlineWidth,XtCLineWidth,XtRInt,sizeof(((WdemoRec*)NULL)->wdemo.lineWidth),XtOffsetOf(WdemoRec,wdemo.lineWidth),XtRImmediate,(XtPointer)1 },
#line 8 "Wdemo.widget"
{XtNcornerRoundPercent,XtCCornerRoundPercent,XtRInt,sizeof(((WdemoRec*)NULL)->wdemo.cornerRoundPercent),XtOffsetOf(WdemoRec,wdemo.cornerRoundPercent),XtRImmediate,(XtPointer)20 },
#line 9 "Wdemo.widget"
{XtNcallback,XtCCallback,XtRCallback,sizeof(((WdemoRec*)NULL)->wdemo.callback),XtOffsetOf(WdemoRec,wdemo.callback),XtRImmediate,(XtPointer)NULL },
#line 10 "Wdemo.widget"
{XtNlabel,XtCLabel,XtRString,sizeof(((WdemoRec*)NULL)->wdemo.label),XtOffsetOf(WdemoRec,wdemo.label),XtRImmediate,(XtPointer)NULL },
#line 11 "Wdemo.widget"
{XtNvalue,XtCValue,XtRString,sizeof(((WdemoRec*)NULL)->wdemo.value),XtOffsetOf(WdemoRec,wdemo.value),XtRImmediate,(XtPointer)NULL },
};

WdemoClassRec wdemoClassRec = {
{ /* core_class part */
/* superclass   	*/  (WidgetClass) &coreClassRec,
/* class_name   	*/  "Wdemo",
/* widget_size  	*/  sizeof(WdemoRec),
/* class_initialize 	*/  NULL,
/* class_part_initialize*/  _resolve_inheritance,
/* class_inited 	*/  FALSE,
/* initialize   	*/  initialize,
/* initialize_hook 	*/  NULL,
/* realize      	*/  realize,
/* actions      	*/  NULL,
/* num_actions  	*/  0,
/* resources    	*/  resources,
/* num_resources 	*/  7,
/* xrm_class    	*/  NULLQUARK,
/* compres_motion 	*/  False ,
/* compress_exposure 	*/  XtExposeCompressSeries ,
/* compress_enterleave 	*/  False ,
/* visible_interest 	*/  True ,
/* destroy      	*/  destroy,
/* resize       	*/  resize,
/* expose       	*/  expose,
/* set_values   	*/  set_values,
/* set_values_hook 	*/  NULL,
/* set_values_almost 	*/  XtInheritSetValuesAlmost,
/* get_values+hook 	*/  NULL,
/* accept_focus 	*/  XtInheritAcceptFocus,
/* version      	*/  XtVersion,
/* callback_private 	*/  NULL,
/* tm_table      	*/  NULL,
/* query_geometry 	*/  XtInheritQueryGeometry,
/* display_acceleator 	*/  XtInheritDisplayAccelerator,
/* extension    	*/  NULL 
},
{ /* Wdemo_class part */
 /* dummy */  0
},
};
WidgetClass wdemoWidgetClass = (WidgetClass) &wdemoClassRec;
static void _resolve_inheritance(class)
WidgetClass class;
{
  WdemoWidgetClass c __attribute__((unused)) = (WdemoWidgetClass) class;
  WdemoWidgetClass super __attribute__((unused));
  if (class == wdemoWidgetClass) return;
  super = (WdemoWidgetClass)class->core_class.superclass;
}
#line 35 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 35 "Wdemo.widget"
static void initialize(Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 35 "Wdemo.widget"
static void initialize(request,self,args,num_args)Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 36 "Wdemo.widget"
{
  int w,h;
  ((WdemoWidget)self)->wdemo.draw=0;
  ((WdemoWidget)self)->wdemo.pm=0;
  ((WdemoWidget)self)->wdemo.changed=CHANGE_SIZE;
  ((WdemoWidget)self)->wdemo.gc[0] = 0;
  calculate_prefered_size(self,&w,&h);
  ((WdemoWidget)self)->core.width = w; ((WdemoWidget)self)->core.height = h;
}
#line 46 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 46 "Wdemo.widget"
static Boolean  set_values(Widget  old,Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 46 "Wdemo.widget"
static Boolean  set_values(old,request,self,args,num_args)Widget  old;Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 47 "Wdemo.widget"
{
        ((WdemoWidget)self)->wdemo.changed = 0;
        int i, p = 0;
        for(i=0;i<*num_args;i++) {
           if( strcmp(args[i].name, "label" ) == 0 ) {
               ((WdemoWidget)self)->wdemo.changed |= CHANGE_VALUE1;
               p++;
           } else if( strcmp(args[i].name, "value" ) == 0 ) {
               ((WdemoWidget)self)->wdemo.changed |= CHANGE_VALUE2;
               p++;
           }
        }

        if( p != *num_args ) ((WdemoWidget)self)->wdemo.changed = CHANGE_COLOR;
        update(self);
        return False; /* do NOT call expose() to update widget */
}
#line 65 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 65 "Wdemo.widget"
static void destroy(Widget self)
#else
#line 65 "Wdemo.widget"
static void destroy(self)Widget self;
#endif
#line 66 "Wdemo.widget"
{
        free_colors(self);
        if( ((WdemoWidget)self)->wdemo.draw ) XftDrawDestroy( ((WdemoWidget)self)->wdemo.draw );
        if( ((WdemoWidget)self)->wdemo.pm ) XFreePixmap(XtDisplay(self),((WdemoWidget)self)->wdemo.pm);
}
#line 72 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 72 "Wdemo.widget"
static void expose(Widget self,XEvent * event,Region  region)
#else
#line 72 "Wdemo.widget"
static void expose(self,event,region)Widget self;XEvent * event;Region  region;
#endif
#line 73 "Wdemo.widget"
{
        XTFUNC();
        draw_parts(self,region);
        show_region(region);
}
#line 79 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 79 "Wdemo.widget"
static void realize(Widget self,XtValueMask * mask,XSetWindowAttributes * attributes)
#else
#line 79 "Wdemo.widget"
static void realize(self,mask,attributes)Widget self;XtValueMask * mask;XSetWindowAttributes * attributes;
#endif
#line 80 "Wdemo.widget"
{
        XTFUNC();
	XtCreateWindow(self, (unsigned int) InputOutput,
        		  (Visual *) CopyFromParent, *mask, attributes);
  	reshape_widget(self);
        ((WdemoWidget)self)->wdemo.changed = CHANGE_SIZE;
        update(self);
}
#line 89 "Wdemo.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 89 "Wdemo.widget"
static void resize(Widget self)
#else
#line 89 "Wdemo.widget"
static void resize(self)Widget self;
#endif
#line 90 "Wdemo.widget"
{
    if (XtIsRealized(self)) {
       XTFUNC();
       reshape_widget(self);
       ((WdemoWidget)self)->wdemo.changed = CHANGE_SIZE;
       update(self);
    }
}
