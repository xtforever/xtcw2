/* Generated by wbuild
 * (generator version 3.3)
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <xtcw/WoptcP.h>
#line 140 "Woptc.widget"
static void sl_motion(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);
#line 150 "Woptc.widget"
static void t1(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);

static XtActionsRec actionsList[] = {
{"sl_motion", sl_motion},
{"t1", t1},
};

static char defaultTranslations[] = "\
<Btn1Motion>: sl_motion() \n\
<Btn1Up>: sl_motion() \n\
<EnterWindow>: t1() \n\
<LeaveWindow>: t1() \n\
";
static void _resolve_inheritance(
#if NeedFunctionPrototypes
WidgetClass
#endif
);
#line 45 "Woptc.widget"
static void initialize(
#if NeedFunctionPrototypes
Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 61 "Woptc.widget"
static Boolean  set_values(
#if NeedFunctionPrototypes
Widget ,Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 67 "Woptc.widget"
static void destroy(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 74 "Woptc.widget"
static void expose(
#if NeedFunctionPrototypes
Widget,XEvent *,Region 
#endif
);
#line 83 "Woptc.widget"
static void set_slider_pos(
#if NeedFunctionPrototypes
Widget,int ,int ,int 
#endif
);
#line 92 "Woptc.widget"
static void draw_slider(
#if NeedFunctionPrototypes
Widget,int 
#endif
);
#line 83 "Woptc.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 83 "Woptc.widget"
static void set_slider_pos(Widget self,int  vis,int  max,int  first)
#else
#line 83 "Woptc.widget"
static void set_slider_pos(self,vis,max,first)Widget self;int  vis;int  max;int  first;
#endif
#line 84 "Woptc.widget"
{
	((WoptcWidget)self)->woptc.frac = ( vis * 1.0 / max );
	((WoptcWidget)self)->woptc.pos  = first * 1.0 / max;
	int y = ((WoptcWidget)self)->core.height * ((WoptcWidget)self)->woptc.pos;
	draw_slider(self,y);
	
}
#line 92 "Woptc.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 92 "Woptc.widget"
static void draw_slider(Widget self,int  y)
#else
#line 92 "Woptc.widget"
static void draw_slider(self,y)Widget self;int  y;
#endif
#line 93 "Woptc.widget"
{

	if(! XtIsRealized(self) ) {

	     TRACE(9, "Widget is gone" );
	};

	/* compute new slider position (y) and size (h) */ 
	if( y < 0 ) y = 0;
        if( ((WoptcWidget)self)->woptc.frac <= 0 || ((WoptcWidget)self)->woptc.frac > 1 ) ((WoptcWidget)self)->woptc.frac = .1;	
	TRACE(9, "frac:%f y:%d", ((WoptcWidget)self)->woptc.frac,y );

        int h;
        h = ((WoptcWidget)self)->core.height * ((WoptcWidget)self)->woptc.frac;
        if( y+h > ((WoptcWidget)self)->core.height ) y = Max(0,((WoptcWidget)self)->core.height-h);
	TRACE(9, "height:%d sl-height:%d top:%d", ((WoptcWidget)self)->core.height, h, y);
	if( y == ((WoptcWidget)self)->woptc.sly && ((WoptcWidget)self)->woptc.slh == h ) return;


	
	/* undraw old slider */
        if( ((WoptcWidget)self)->woptc.sly >= 0 && ((WoptcWidget)self)->woptc.slh > 0 ) {
             XFillRectangle(XtDisplay(self), XtWindow(self),((WoptcWidget)self)->woptc.gc_background,
                             0,((WoptcWidget)self)->woptc.sly,((WoptcWidget)self)->core.width, ((WoptcWidget)self)->woptc.slh );
        }

	/* draw slider at Pos: (0,$sly) */
        ((WoptcWidget)self)->woptc.sly = y; ((WoptcWidget)self)->woptc.slh = h;
	TRACE(9, "height:%d sl-height:%d top:%d", ((WoptcWidget)self)->core.height, h, ((WoptcWidget)self)->woptc.sly);
        XFillRectangle(XtDisplay(self), XtWindow(self),((WoptcWidget)self)->woptc.gc_fg,
                             0,((WoptcWidget)self)->woptc.sly,((WoptcWidget)self)->core.width, ((WoptcWidget)self)->woptc.slh );


	/* compute relative position 0..1 */		     
	float p = ((WoptcWidget)self)->woptc.sly * 1.0 / ((WoptcWidget)self)->core.height;
        if( p + ((WoptcWidget)self)->woptc.frac > 1.0 ) p = 1.0 - ((WoptcWidget)self)->woptc.frac;
        if( p < 0 ) p = 0;
        ((WoptcWidget)self)->woptc.pos = p;
}

static XtResource resources[] = {
#line 5 "Woptc.widget"
{XtNforeground,XtCForeground,XtRPixel,sizeof(((WoptcRec*)NULL)->woptc.foreground),XtOffsetOf(WoptcRec,woptc.foreground),XtRImmediate,(XtPointer)XtDefaultForeground },
#line 6 "Woptc.widget"
{XtNlineWidth,XtCLineWidth,XtRInt,sizeof(((WoptcRec*)NULL)->woptc.lineWidth),XtOffsetOf(WoptcRec,woptc.lineWidth),XtRImmediate,(XtPointer)1 },
#line 7 "Woptc.widget"
{XtNfilled,XtCFilled,XtRBoolean,sizeof(((WoptcRec*)NULL)->woptc.filled),XtOffsetOf(WoptcRec,woptc.filled),XtRImmediate,(XtPointer)False },
#line 8 "Woptc.widget"
{XtNfrac,XtCFrac,XtRFloat,sizeof(((WoptcRec*)NULL)->woptc.frac),XtOffsetOf(WoptcRec,woptc.frac),XtRString,(XtPointer)"0.1"},
#line 9 "Woptc.widget"
{XtNpos,XtCPos,XtRFloat,sizeof(((WoptcRec*)NULL)->woptc.pos),XtOffsetOf(WoptcRec,woptc.pos),XtRString,(XtPointer)"0.1"},
#line 10 "Woptc.widget"
{XtNcallback,XtCCallback,XtRCallback,sizeof(((WoptcRec*)NULL)->woptc.callback),XtOffsetOf(WoptcRec,woptc.callback),XtRImmediate,(XtPointer)NULL },
#line 11 "Woptc.widget"
{XtNsliderWidth,XtCSliderWidth,XtRDistance,sizeof(((WoptcRec*)NULL)->woptc.sliderWidth),XtOffsetOf(WoptcRec,woptc.sliderWidth),XtRString,(XtPointer)"4mm"},
};

WoptcClassRec woptcClassRec = {
{ /* core_class part */
/* superclass   	*/  (WidgetClass) &coreClassRec,
/* class_name   	*/  "Woptc",
/* widget_size  	*/  sizeof(WoptcRec),
/* class_initialize 	*/  NULL,
/* class_part_initialize*/  _resolve_inheritance,
/* class_inited 	*/  FALSE,
/* initialize   	*/  initialize,
/* initialize_hook 	*/  NULL,
/* realize      	*/  XtInheritRealize,
/* actions      	*/  actionsList,
/* num_actions  	*/  2,
/* resources    	*/  resources,
/* num_resources 	*/  7,
/* xrm_class    	*/  NULLQUARK,
/* compres_motion 	*/  False ,
/* compress_exposure 	*/  FALSE ,
/* compress_enterleave 	*/  False ,
/* visible_interest 	*/  False ,
/* destroy      	*/  destroy,
/* resize       	*/  XtInheritResize,
/* expose       	*/  expose,
/* set_values   	*/  set_values,
/* set_values_hook 	*/  NULL,
/* set_values_almost 	*/  XtInheritSetValuesAlmost,
/* get_values+hook 	*/  NULL,
/* accept_focus 	*/  XtInheritAcceptFocus,
/* version      	*/  XtVersion,
/* callback_private 	*/  NULL,
/* tm_table      	*/  defaultTranslations,
/* query_geometry 	*/  XtInheritQueryGeometry,
/* display_acceleator 	*/  XtInheritDisplayAccelerator,
/* extension    	*/  NULL 
},
{ /* Woptc_class part */
 /* dummy */  0
},
};
WidgetClass woptcWidgetClass = (WidgetClass) &woptcClassRec;
/*ARGSUSED*/
#line 140 "Woptc.widget"
static void sl_motion(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
	static float p;
	int y = event->xbutton.y;
	TRACE(9, "%d", y );
        draw_slider(self,y);
        XtCallCallbackList( self, ((WoptcWidget)self)->woptc.callback, (XtPointer) & (((WoptcWidget)self)->woptc.pos) );	
	TRACE(9, "leave cb" );
}

/*ARGSUSED*/
#line 150 "Woptc.widget"
static void t1(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
	TRACE(9,"Hello");
}

static void _resolve_inheritance(class)
WidgetClass class;
{
  WoptcWidgetClass c __attribute__((unused)) = (WoptcWidgetClass) class;
  WoptcWidgetClass super __attribute__((unused));
  if (class == woptcWidgetClass) return;
  super = (WoptcWidgetClass)class->core_class.superclass;
}
#line 45 "Woptc.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 45 "Woptc.widget"
static void initialize(Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 45 "Woptc.widget"
static void initialize(request,self,args,num_args)Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 46 "Woptc.widget"
{
  XGCValues     values = {
  		foreground: ((WoptcWidget)self)->core.background_pixel,
  		graphics_exposures: False,
                line_width: ((WoptcWidget)self)->woptc.lineWidth };
  uint          mask = GCForeground | GCGraphicsExposures |  GCLineWidth;
  ((WoptcWidget)self)->woptc.gc_background = XtGetGC(self, mask, &values);
  values.foreground = ((WoptcWidget)self)->woptc.foreground; 
  ((WoptcWidget)self)->woptc.gc_fg = XtGetGC(self,mask,&values);
  if( ((WoptcWidget)self)->core.width == 0 ) ((WoptcWidget)self)->core.width=((WoptcWidget)self)->woptc.sliderWidth;
  if( ((WoptcWidget)self)->core.height == 0 ) ((WoptcWidget)self)->core.height=10;
  ((WoptcWidget)self)->woptc.resize = 1;
  ((WoptcWidget)self)->woptc.sly = 0;
}
#line 61 "Woptc.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 61 "Woptc.widget"
static Boolean  set_values(Widget  old,Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 61 "Woptc.widget"
static Boolean  set_values(old,request,self,args,num_args)Widget  old;Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 62 "Woptc.widget"
{
  return True; /* redraw */
}
#line 67 "Woptc.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 67 "Woptc.widget"
static void destroy(Widget self)
#else
#line 67 "Woptc.widget"
static void destroy(self)Widget self;
#endif
#line 68 "Woptc.widget"
{
  if( ((WoptcWidget)self)->woptc.gc_background ) XtReleaseGC(self,((WoptcWidget)self)->woptc.gc_background);
  if( ((WoptcWidget)self)->woptc.gc_fg ) XtReleaseGC(self,((WoptcWidget)self)->woptc.gc_fg);
  TRACE(9,"destroy");
}
#line 74 "Woptc.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 74 "Woptc.widget"
static void expose(Widget self,XEvent * event,Region  region)
#else
#line 74 "Woptc.widget"
static void expose(self,event,region)Widget self;XEvent * event;Region  region;
#endif
#line 75 "Woptc.widget"
{
  XClearWindow( XtDisplay(self), XtWindow(self) );
  ((WoptcWidget)self)->woptc.slh=0; /* do not undraw slider */
  draw_slider(self, ((WoptcWidget)self)->woptc.pos * ((WoptcWidget)self)->core.height );
}
#line 32 "Woptc.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 32 "Woptc.widget"
void  wls_set_slider_pos(Widget  w,int  vis,int  max,int  first)
#else
#line 32 "Woptc.widget"
void  wls_set_slider_pos(w,vis,max,first)Widget  w;int  vis;int  max;int  first;
#endif
#line 33 "Woptc.widget"
{
  WoptcWidgetClass wc = (WoptcWidgetClass)XtClass(w);
  if( XtIsSubclass( w, woptcWidgetClass) )
    {
       set_slider_pos(w,vis,max,first);
    }
}
#line 157 "Woptc.widget"
#line 158 "Woptc.widget"
