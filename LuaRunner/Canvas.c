/* Generated by wbuild
 * (generator version 3.3)
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#line 156 "Canvas.widget"
#include <assert.h>
#line 157 "Canvas.widget"
#include <stdint.h>
#line 158 "Canvas.widget"
#include <X11/Intrinsic.h>
#line 159 "Canvas.widget"
#include <X11/Xmu/Converters.h>
#line 160 "Canvas.widget"
#include <X11/Xft/Xft.h>
#line 161 "Canvas.widget"
#include <X11/Xregion.h>
#line 162 "Canvas.widget"
#include "converters.h"
#line 163 "Canvas.widget"
#include "xutil.h"
#line 164 "Canvas.widget"
#include "mls.h"
#line 165 "Canvas.widget"
#include "canvas-draw.h"
#include <xtcw/CanvasP.h>
#line 116 "Canvas.widget"
static void zoom(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);

static XtActionsRec actionsList[] = {
{"zoom", zoom},
};
static void _resolve_inheritance(
#if NeedFunctionPrototypes
WidgetClass
#endif
);
#line 26 "Canvas.widget"
static void expose(
#if NeedFunctionPrototypes
Widget,XEvent *,Region 
#endif
);
#line 33 "Canvas.widget"
static void initialize(
#if NeedFunctionPrototypes
Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 45 "Canvas.widget"
static void destroy(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 53 "Canvas.widget"
static void resize(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 61 "Canvas.widget"
static void realize(
#if NeedFunctionPrototypes
Widget,XtValueMask *,XSetWindowAttributes *
#endif
);
#line 94 "Canvas.widget"
static Boolean  set_values(
#if NeedFunctionPrototypes
Widget ,Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 101 "Canvas.widget"
static void paint1(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 107 "Canvas.widget"
static void init_colors(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 101 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 101 "Canvas.widget"
static void paint1(Widget self)
#else
#line 101 "Canvas.widget"
static void paint1(self)Widget self;
#endif
#line 102 "Canvas.widget"
{
    XTFUNC();
    XtCallCallbackList( self, ((CanvasWidget)self)->canvas.callback, (XtPointer) & ((CanvasWidget)self)->canvas.canv );
}
#line 107 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 107 "Canvas.widget"
static void init_colors(Widget self)
#else
#line 107 "Canvas.widget"
static void init_colors(self)Widget self;
#endif
#line 108 "Canvas.widget"
{
	pixel_to_xftcolor( self, ((CanvasWidget)self)->core.background_pixel, BG );
 	pixel_to_xftcolor( self, ((CanvasWidget)self)->canvas.foreground,       FG );
}

static XtResource resources[] = {
#line 10 "Canvas.widget"
{XtNxftFont,XtCXFtFont,XtRXftFont,sizeof(((CanvasRec*)NULL)->canvas.xftFont),XtOffsetOf(CanvasRec,canvas.xftFont),XtRString,(XtPointer)"Serif-22"},
#line 11 "Canvas.widget"
{XtNforeground,XtCForeground,XtRPixel,sizeof(((CanvasRec*)NULL)->canvas.foreground),XtOffsetOf(CanvasRec,canvas.foreground),XtRImmediate,(XtPointer)XtDefaultForeground },
#line 12 "Canvas.widget"
{XtNcallback,XtCCallback,XtRCallback,sizeof(((CanvasRec*)NULL)->canvas.callback),XtOffsetOf(CanvasRec,canvas.callback),XtRImmediate,(XtPointer)NULL },
#line 13 "Canvas.widget"
{XtNtopx,XtCTopx,XtRInt,sizeof(((CanvasRec*)NULL)->canvas.topx),XtOffsetOf(CanvasRec,canvas.topx),XtRImmediate,(XtPointer)0 },
#line 14 "Canvas.widget"
{XtNtopy,XtCTopy,XtRInt,sizeof(((CanvasRec*)NULL)->canvas.topy),XtOffsetOf(CanvasRec,canvas.topy),XtRImmediate,(XtPointer)0 },
#line 15 "Canvas.widget"
{XtNzoom_thousandth,XtCZoom_thousandth,XtRInt,sizeof(((CanvasRec*)NULL)->canvas.zoom_thousandth),XtOffsetOf(CanvasRec,canvas.zoom_thousandth),XtRImmediate,(XtPointer)1000 },
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
/* actions      	*/  actionsList,
/* num_actions  	*/  1,
/* resources    	*/  resources,
/* num_resources 	*/  6,
/* xrm_class    	*/  NULLQUARK,
/* compres_motion 	*/  True ,
/* compress_exposure 	*/  XtExposeCompressMultiple ,
/* compress_enterleave 	*/  True ,
/* visible_interest 	*/  False ,
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
/*ARGSUSED*/
#line 116 "Canvas.widget"
static void zoom(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
	XTFUNC();
	float scale;
	int parse = 0;
	for( int i=0;i<*num_params;i++)
	{
		char *s = params[i];
		TRACE(1,"PARSE: %s", s );
		switch( parse ) {
			//---- key
			case 0: if( strcasecmp(s, "scale") == 0 ) parse=2;
			        break;
			//---- value
			case 2: TRACE(1,"PARSE");
				scale=strtof(s,NULL);
			        canvas_zoom( self, & ((CanvasWidget)self)->canvas.canv, scale );
				parse=0;
				break;
			default: WARN("uknown parameter: %s", s );
		}
	}
}

static void _resolve_inheritance(class)
WidgetClass class;
{
  CanvasWidgetClass c __attribute__((unused)) = (CanvasWidgetClass) class;
  CanvasWidgetClass super __attribute__((unused));
  if (class == canvasWidgetClass) return;
  super = (CanvasWidgetClass)class->core_class.superclass;
}
#line 26 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 26 "Canvas.widget"
static void expose(Widget self,XEvent * event,Region  region)
#else
#line 26 "Canvas.widget"
static void expose(self,event,region)Widget self;XEvent * event;Region  region;
#endif
#line 27 "Canvas.widget"
{
    XTFUNC();
    /* XftDrawRect(DRAW,BG,0,0,$width,$height); */
    paint1(self);
}
#line 33 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 33 "Canvas.widget"
static void initialize(Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 33 "Canvas.widget"
static void initialize(request,self,args,num_args)Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 34 "Canvas.widget"
{

    XTFUNC();
    memset(& ((CanvasWidget)self)->canvas.canv, 0, sizeof (((CanvasWidget)self)->canvas.canv));

	if( ((CanvasWidget)self)->core.width <=0 ) ((CanvasWidget)self)->core.width = 20;
	if( ((CanvasWidget)self)->core.height <=0 ) ((CanvasWidget)self)->core.height = 20;
	init_colors(self);
	DRAW=0;
}
#line 45 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 45 "Canvas.widget"
static void destroy(Widget self)
#else
#line 45 "Canvas.widget"
static void destroy(self)Widget self;
#endif
#line 46 "Canvas.widget"
{
        XTFUNC();
        if( DRAW ) XftDrawDestroy( DRAW );
        XtReleaseGC( self, ((CanvasWidget)self)->canvas.canv.gc[0] );
        XtReleaseGC( self, ((CanvasWidget)self)->canvas.canv.gc[1] );
}
#line 53 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 53 "Canvas.widget"
static void resize(Widget self)
#else
#line 53 "Canvas.widget"
static void resize(self)Widget self;
#endif
#line 54 "Canvas.widget"
{
	XTFUNC();
    ((CanvasWidget)self)->canvas.canv.win_width  = ((CanvasWidget)self)->core.width;
    ((CanvasWidget)self)->canvas.canv.win_height = ((CanvasWidget)self)->core.height;

}
#line 61 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 61 "Canvas.widget"
static void realize(Widget self,XtValueMask * mask,XSetWindowAttributes * attributes)
#else
#line 61 "Canvas.widget"
static void realize(self,mask,attributes)Widget self;XtValueMask * mask;XSetWindowAttributes * attributes;
#endif
#line 62 "Canvas.widget"
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

     ((CanvasWidget)self)->canvas.canv.app_context = XtWidgetToApplicationContext( self );
}
#line 94 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 94 "Canvas.widget"
static Boolean  set_values(Widget  old,Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 94 "Canvas.widget"
static Boolean  set_values(old,request,self,args,num_args)Widget  old;Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 95 "Canvas.widget"
{
	return True; /* yes we want a redraw */	
}
#line 142 "Canvas.widget"
#line 143 "Canvas.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 143 "Canvas.widget"
canvas_draw_t * canvas_get_priv(Widget self)
#else
#line 143 "Canvas.widget"
canvas_draw_t * canvas_get_priv(self)Widget self;
#endif
#line 144 "Canvas.widget"
{
    if( XtIsSubclass( self,canvasWidgetClass ) )
      return & ((CanvasWidget)self)->canvas.canv;
    XtWarning( "Widget is not subclass of canvas" );
    return NULL;
}
