/* Generated by wbuild
 * (generator version 3.3)
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#line 105 "Canvas.widget"
#include <assert.h>
#line 106 "Canvas.widget"
#include <stdint.h>
#line 107 "Canvas.widget"
#include <X11/Intrinsic.h>
#line 108 "Canvas.widget"
#include <X11/Xmu/Converters.h>
#line 109 "Canvas.widget"
#include <X11/Xft/Xft.h>
#line 110 "Canvas.widget"
#include <X11/Xregion.h>
#line 111 "Canvas.widget"
#include "converters.h"
#line 112 "Canvas.widget"
#include "xutil.h"
#line 113 "Canvas.widget"
#include "mls.h"
#line 114 "Canvas.widget"
#include "canvas-draw.h"
#include <xtcw/CanvasP.h>
static void _resolve_inheritance(
#if NeedFunctionPrototypes
WidgetClass
#endif
);
#line 19 "Canvas.widget"
static void expose(
#if NeedFunctionPrototypes
Widget,XEvent *,Region 
#endif
);
#line 26 "Canvas.widget"
static void initialize(
#if NeedFunctionPrototypes
Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 35 "Canvas.widget"
static void destroy(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 43 "Canvas.widget"
static void resize(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 51 "Canvas.widget"
static void realize(
#if NeedFunctionPrototypes
Widget,XtValueMask *,XSetWindowAttributes *
#endif
);
#line 83 "Canvas.widget"
static void paint1(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 89 "Canvas.widget"
static void init_colors(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 83 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 83 "Canvas.widget"
static void paint1(Widget self)
#else
#line 83 "Canvas.widget"
static void paint1(self)Widget self;
#endif
#line 84 "Canvas.widget"
{
    XTFUNC();
    XtCallCallbackList( self, ((CanvasWidget)self)->canvas.callback, (XtPointer) & ((CanvasWidget)self)->canvas.canv );
}
#line 89 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 89 "Canvas.widget"
static void init_colors(Widget self)
#else
#line 89 "Canvas.widget"
static void init_colors(self)Widget self;
#endif
#line 90 "Canvas.widget"
{
	pixel_to_xftcolor( self, ((CanvasWidget)self)->core.background_pixel, BG );
 	pixel_to_xftcolor( self, ((CanvasWidget)self)->canvas.foreground,       FG );
}

static XtResource resources[] = {
#line 5 "Canvas.widget"
{XtNxftFont,XtCXFtFont,XtRXftFont,sizeof(((CanvasRec*)NULL)->canvas.xftFont),XtOffsetOf(CanvasRec,canvas.xftFont),XtRString,(XtPointer)"Serif-22"},
#line 6 "Canvas.widget"
{XtNforeground,XtCForeground,XtRPixel,sizeof(((CanvasRec*)NULL)->canvas.foreground),XtOffsetOf(CanvasRec,canvas.foreground),XtRImmediate,(XtPointer)XtDefaultForeground },
#line 7 "Canvas.widget"
{XtNcallback,XtCCallback,XtRCallback,sizeof(((CanvasRec*)NULL)->canvas.callback),XtOffsetOf(CanvasRec,canvas.callback),XtRImmediate,(XtPointer)NULL },
};

CanvasClassRec canvasClassRec = {
{ /* core_class part */
/* superclass   	*/  (WidgetClass) &coreClassRec,
/* class_name   	*/  "Canvas",
/* widget_size  	*/  sizeof(CanvasRec),
/* class_initialize 	*/  NULL,
/* class_part_initialize*/  _resolve_inheritance,
/* class_inited 	*/  FALSE,
/* initialize   	*/  initialize,
/* initialize_hook 	*/  NULL,
/* realize      	*/  realize,
/* actions      	*/  NULL,
/* num_actions  	*/  0,
/* resources    	*/  resources,
/* num_resources 	*/  3,
/* xrm_class    	*/  NULLQUARK,
/* compres_motion 	*/  False ,
/* compress_exposure 	*/  FALSE ,
/* compress_enterleave 	*/  False ,
/* visible_interest 	*/  False ,
/* destroy      	*/  destroy,
/* resize       	*/  resize,
/* expose       	*/  expose,
/* set_values   	*/  NULL,
/* set_values_hook 	*/  NULL,
/* set_values_almost 	*/  XtInheritSetValuesAlmost,
/* get_values+hook 	*/  NULL,
/* accept_focus 	*/  XtInheritAcceptFocus,
/* version      	*/  XtVersion,
/* callback_private 	*/  NULL,
/* tm_table      	*/  XtInheritTranslations,
/* query_geometry 	*/  XtInheritQueryGeometry,
/* display_acceleator 	*/  XtInheritDisplayAccelerator,
/* extension    	*/  NULL 
},
{ /* Canvas_class part */
 /* dummy */  0
},
};
WidgetClass canvasWidgetClass = (WidgetClass) &canvasClassRec;
static void _resolve_inheritance(class)
WidgetClass class;
{
  CanvasWidgetClass c __attribute__((unused)) = (CanvasWidgetClass) class;
  CanvasWidgetClass super __attribute__((unused));
  if (class == canvasWidgetClass) return;
  super = (CanvasWidgetClass)class->core_class.superclass;
}
#line 19 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 19 "Canvas.widget"
static void expose(Widget self,XEvent * event,Region  region)
#else
#line 19 "Canvas.widget"
static void expose(self,event,region)Widget self;XEvent * event;Region  region;
#endif
#line 20 "Canvas.widget"
{
    XTFUNC();
	XftDrawRect(DRAW,BG,0,0,((CanvasWidget)self)->core.width,((CanvasWidget)self)->core.height);
	paint1(self);
}
#line 26 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 26 "Canvas.widget"
static void initialize(Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 26 "Canvas.widget"
static void initialize(request,self,args,num_args)Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 27 "Canvas.widget"
{
    XTFUNC();
	if( ((CanvasWidget)self)->core.width <=0 ) ((CanvasWidget)self)->core.width = 20;
	if( ((CanvasWidget)self)->core.height <=0 ) ((CanvasWidget)self)->core.height = 20;
	init_colors(self);
	DRAW=0;
}
#line 35 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 35 "Canvas.widget"
static void destroy(Widget self)
#else
#line 35 "Canvas.widget"
static void destroy(self)Widget self;
#endif
#line 36 "Canvas.widget"
{
        XTFUNC();
        if( DRAW ) XftDrawDestroy( DRAW );
        XtReleaseGC( self, ((CanvasWidget)self)->canvas.canv.gc[0] );
        XtReleaseGC( self, ((CanvasWidget)self)->canvas.canv.gc[1] );
}
#line 43 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 43 "Canvas.widget"
static void resize(Widget self)
#else
#line 43 "Canvas.widget"
static void resize(self)Widget self;
#endif
#line 44 "Canvas.widget"
{
	XTFUNC();
    ((CanvasWidget)self)->canvas.canv.win_width  = ((CanvasWidget)self)->core.width;
    ((CanvasWidget)self)->canvas.canv.win_height = ((CanvasWidget)self)->core.height;

}
#line 51 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 51 "Canvas.widget"
static void realize(Widget self,XtValueMask * mask,XSetWindowAttributes * attributes)
#else
#line 51 "Canvas.widget"
static void realize(self,mask,attributes)Widget self;XtValueMask * mask;XSetWindowAttributes * attributes;
#endif
#line 52 "Canvas.widget"
{
    XTFUNC();
	XtCreateWindow(self, (unsigned int) InputOutput,
        		  (Visual *) CopyFromParent,
			  *mask, attributes);
    Display *dpy = XtDisplay(self);
  	DRAW = XftDrawCreate(dpy, XtWindow(self),
                 DefaultVisual(dpy, DefaultScreen(dpy)), None);

    ((CanvasWidget)self)->canvas.canv.xfont    = ((CanvasWidget)self)->canvas.xftFont;
    ((CanvasWidget)self)->canvas.canv.dpy      =  dpy;
    ((CanvasWidget)self)->canvas.canv.screen   = DefaultScreen(dpy);
    
    XGCValues     values;
    values.graphics_exposures = False;
    values.foreground = ((CanvasWidget)self)->canvas.foreground;
    values.background = ((CanvasWidget)self)->core.background_pixel;

    ((CanvasWidget)self)->canvas.canv.gc[0] = XtGetGC( self,
      (unsigned) GCForeground | GCBackground | GCGraphicsExposures,
      &values);

    values.foreground = ((CanvasWidget)self)->core.background_pixel;
    ((CanvasWidget)self)->canvas.canv.gc[1] = XtGetGC( self,
      (unsigned) GCForeground | GCBackground | GCGraphicsExposures,
      &values);
}
#line 96 "Canvas.widget"
#line 97 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 97 "Canvas.widget"
canvas_draw_t * canvas_get_priv(Widget self)
#else
#line 97 "Canvas.widget"
canvas_draw_t * canvas_get_priv(self)Widget self;
#endif
#line 98 "Canvas.widget"
{
    return & ((CanvasWidget)self)->canvas.canv;
}
