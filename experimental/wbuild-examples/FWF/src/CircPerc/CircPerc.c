#ifndef _HPUX_SOURCE
#define _HPUX_SOURCE
#endif

#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include "Xfwf/CircPercP.h"

#define DEFAULT_LENGTH 75
#define DEFAULT_WIDTH  75

#define offset(field) XtOffsetOf(XfwfCircularPercentageRec, field)

static XtResource resources[] = {
{
  XtNborderColor, XtCBorderColor,
  XtRPixel, sizeof(Pixel),
  offset(circular_percentage.border_color),
  XtRString,
  (XtPointer) "black"
},
{
  XtNinteriorColor, XtCInteriorColor,
  XtRPixel, sizeof(Pixel),
  offset(circular_percentage.interior_color),
  XtRString,
  (XtPointer) "red"
},
{
  XtNcompletedColor, XtCCompletedColor,
  XtRPixel, sizeof(Pixel),
  offset(circular_percentage.completed_color),
  XtRString,
  (XtPointer) "yellow"
},
{
  XtNpercentageCompleted, XtCPercentageCompleted,
  XtRInt, sizeof(int),
  offset(circular_percentage.iPercentageCompleted),
  XtRImmediate, 
  (XtPointer) 0
}
};

#if NeedFunctionPrototypes > 0
static void Initialize(Widget,Widget,ArgList,Cardinal *);
static void Redisplay(Widget,XExposeEvent *);
static void Destroy(Widget);
static void Resize(Widget);
static Boolean SetValues(Widget,Widget,Widget,ArgList,Cardinal *);
static XtGeometryResult QueryGeometry(Widget,
				      XtWidgetGeometry *,
				      XtWidgetGeometry *);
static void MakeGC(XfwfCircularPercentageWidget);
static void RedrawPart(XfwfCircularPercentageWidget, int, int);
#else
static void Initialize();
static void Redisplay();
static void Destroy();
static void Resize();
static Boolean SetValues();
static XtGeometryResult QueryGeometry();
static void MakeGC();
static void RedrawPart();
#endif


XfwfCircularPercentageClassRec xfwfCircularPercentageClassRec = {
    {
    /* core_class fields     */
    /* superclass            */ (WidgetClass) &widgetClassRec,
    /* class_name            */ "XfwfCircularPercentage",
    /* widget_size           */ sizeof(XfwfCircularPercentageRec),
    /* class_initialize      */ NULL,
    /* class_part_initialize */ NULL,
    /* class_inited          */ FALSE,
    /* initialize            */ (XtInitProc) Initialize,
    /* initialize_hook       */ NULL,
    /* realize               */ XtInheritRealize,
    /* actions               */ NULL,
    /* num_actions           */ 0,
    /* resources             */ resources,
    /* num_resources         */ XtNumber(resources),
    /* xrm_class             */ NULLQUARK,
    /* compress_motion       */ TRUE,
    /* compress_exposure     */ XtExposeCompressMultiple,
    /* compress_enterleave   */ TRUE,
    /* visible_interest      */ FALSE,
    /* destroy               */ (XtWidgetProc) Destroy,
    /* resize                */ (XtWidgetProc) Resize,
    /* expose                */ (XtExposeProc) Redisplay,
    /* set_values            */ (XtSetValuesFunc) SetValues,
    /* set_values_hook       */ NULL,
    /* set_values_almost     */ XtInheritSetValuesAlmost,
    /* get_values_hook       */ NULL,
    /* accept_focus          */ NULL,
    /* version               */ XtVersion,
    /* callback_private      */ NULL,
    /* tm_table              */ NULL,
    /* query_geometry        */ (XtGeometryHandler) QueryGeometry,
    /* display_accelerator   */ XtInheritDisplayAccelerator,
    /* extension             */ NULL
    },
    {
    /* extension          */        0
    }
};

WidgetClass xfwfCircularPercentageWidgetClass = (WidgetClass) & xfwfCircularPercentageClassRec;

#if NeedFunctionPrototypes > 0
static void Initialize(Widget treq_widget, 
		       Widget tnew_widget, 
		       ArgList args, 
		       Cardinal *num_args)
#else
static void Initialize(treq_widget,tnew_widget,args,num_args)
Widget treq_widget;
Widget tnew_widget;
ArgList args;
Cardinal *num_args;
#endif
{
   XfwfCircularPercentageWidget new_widget = (XfwfCircularPercentageWidget) tnew_widget; 
   
     Dimension dimWidth, dimHeight;

     dimWidth = DEFAULT_LENGTH;
     dimHeight = DEFAULT_WIDTH;

#ifdef old					/* Changed, 9 Mar '95 */
     if (dimWidth > new_widget -> core.width)
       new_widget -> core.width = dimWidth;
	
     if (dimHeight > new_widget -> core.height)
       new_widget -> core.height = dimHeight;
#else
     if (new_widget -> core.width == 0)
       new_widget -> core.width = dimWidth;
	
     if (new_widget -> core.height == 0)
       new_widget -> core.height = dimHeight;
#endif

    if (new_widget -> circular_percentage.iPercentageCompleted < 0)
      {
	XtWarning("XfwfCircularPercentage: percentageCompleted resource should have a value between 0 and 10000,\n            (value specified was negative).");
	new_widget -> circular_percentage.iPercentageCompleted = 0;
      }
    else
    if (new_widget -> circular_percentage.iPercentageCompleted > 10000)
      {
	XtWarning("XfwfCircularPercentage: percentageCompleted resource should have a value between 0 and 10000,\n            (value specified was over 10000).");
        new_widget -> circular_percentage.iPercentageCompleted = 10000;
      }

    new_widget -> circular_percentage.gc = None;
}

#if NeedFunctionPrototypes > 0
static void CalculateArcs(XfwfCircularPercentageWidget our_widget, 
			  XArc *arcs, 
			   int iPercentageCompleted)
#else
static void CalculateArcs(our_widget,arcs,iPercentageCompleted)
XfwfCircularPercentageWidget our_widget; 
XArc *arcs; 
int iPercentageCompleted;
#endif
{
  int iXfwfCircularPercentageArc;

  iXfwfCircularPercentageArc = (iPercentageCompleted * 360 * 64) / 10000;

  arcs[0].x = 5 ; 
  arcs[0].y = 5 ;
  arcs[0].width = our_widget -> core.width - arcs[0].x * 2;
  arcs[0].height = our_widget -> core.height - arcs[0].x * 2;
  arcs[0].angle1 = 90 * 64 ;
  arcs[0].angle2 = - iXfwfCircularPercentageArc ;

#ifdef WANT_NOISE

  fprintf (stderr, 
          "(x,y) : (%d,%d) (width,height) (%d,%d) (angle1,angle2) (%d,%d)\n",
	    (int) arcs[0].x,(int)arcs[0].y,
	    (int)arcs[0].width,(int)arcs[0].height,
	    (int)arcs[0].angle1,(int)arcs[0].angle2);
#endif

  arcs[1].x = 5 ; 
  arcs[1].y = 5 ;
  arcs[1].width = our_widget -> core.width - arcs[1].x * 2;
  arcs[1].height = our_widget -> core.height - arcs[1].x * 2;
  arcs[1].angle1 = (90 * 64 - iXfwfCircularPercentageArc) ;
  arcs[1].angle2 = - (360 * 64 - iXfwfCircularPercentageArc);

#ifdef WANT_NOISE

  fprintf (stderr, 
          "(x,y) : (%d,%d) (width,height) (%d,%d) (angle1,angle2) (%d,%d)\n",
	    (int) arcs[1].x,(int)arcs[1].y,
	    (int)arcs[1].width,(int)arcs[1].height,
	    (int)arcs[1].angle1,(int)arcs[1].angle2);

#endif
}

#if NeedFunctionPrototypes > 0
static void Redisplay(Widget w, XExposeEvent *event)
#else
static void Redisplay(w,event)
Widget w;
XExposeEvent *event;
#endif
{
  XfwfCircularPercentageWidget our_widget = (XfwfCircularPercentageWidget) w;
  Display *pDisplay = XtDisplay(our_widget);
  Window window = XtWindow(our_widget);
  XArc arcs[2];
  Dimension dimWidth = our_widget -> core.width;
  Dimension dimHeight = our_widget -> core.height;

  if (!XtIsRealized(w))
    return;
  
  if ((our_widget -> core.width < 25) ||
      (our_widget -> core.height < 25))
    return;

   if (our_widget -> circular_percentage.gc == None)
     MakeGC(our_widget);

   CalculateArcs(our_widget,arcs,our_widget -> circular_percentage.iPercentageCompleted);

   if (arcs[0].angle2)
     {
       XSetForeground(pDisplay, 
		      our_widget -> circular_percentage.gc, 
		      our_widget -> circular_percentage.completed_color);

       XFillArcs(pDisplay,
    	         window,
	         our_widget -> circular_percentage.gc,
	         &arcs[0],
	         1);
     }

   if  (arcs[1].angle2)
     {
       XSetForeground(pDisplay, 
		      our_widget -> circular_percentage.gc, 
		      our_widget -> circular_percentage.interior_color);

       XFillArcs(pDisplay,
    	         window,
	         our_widget -> circular_percentage.gc,
	         &arcs[1],
	         1);
     }

   XSetForeground(pDisplay, 
		  our_widget -> circular_percentage.gc, 
		  our_widget -> circular_percentage.border_color);

   XDrawArc(pDisplay, window, our_widget -> circular_percentage.gc,
	    5,5, dimWidth - 10, dimHeight - 10,
	    0, 360*64);

   XFlush(pDisplay);

}


#define WidgetValuesDiffer(w1,w2,component) (w1 -> circular_percentage.component != \
					     w2 -> circular_percentage.component)

/* ARGSUSED */
#if NeedFunctionPrototypes > 0
static Boolean SetValues(Widget current, 
	                 Widget request, 
	                 Widget new_widget, 
	                 ArgList args, 
	                 Cardinal *num_args)
#else
static Boolean SetValues(current, request, new_widget, args, num_args)
Widget current; 
Widget request; 
Widget new_widget; 
ArgList args; 
Cardinal *num_args;
#endif
{
   Boolean bShouldRedraw = False;

   XfwfCircularPercentageWidget current_widget = (XfwfCircularPercentageWidget) current;
   XfwfCircularPercentageWidget check_widget   = (XfwfCircularPercentageWidget) new_widget;

   if (WidgetValuesDiffer(current_widget,check_widget, border_color) ||
       WidgetValuesDiffer(current_widget,check_widget, interior_color) ||
       WidgetValuesDiffer(current_widget,check_widget, completed_color))
     bShouldRedraw = True;

   if (WidgetValuesDiffer(current_widget,check_widget, iPercentageCompleted))
     {
       if (check_widget -> circular_percentage.iPercentageCompleted < 0 ||
	   check_widget -> circular_percentage.iPercentageCompleted > 10000)
         {
	   XtWarning("XfwfCircularPercentage: new value for percentage out of range [0,10000]).");
	   check_widget -> circular_percentage.iPercentageCompleted =
	         current_widget -> circular_percentage.iPercentageCompleted;
         }
       else
	 bShouldRedraw = True;
     }

   if (bShouldRedraw)
     Redisplay(new_widget,NULL);

   return False;
}

#if NeedFunctionPrototypes > 0
static void Destroy(Widget w)
#else
static void Destroy(w)
Widget w;
#endif
{
  XfwfCircularPercentageWidget our_widget = (XfwfCircularPercentageWidget) w;

  if (our_widget -> circular_percentage.gc != None)
    XFreeGC(XtDisplay(our_widget),our_widget -> circular_percentage.gc);
}

#if NeedFunctionPrototypes > 0
static void Resize(Widget w)
#else
static void Resize(w)
Widget w;
#endif
{
}

#if NeedFunctionPrototypes > 0
static XtGeometryResult QueryGeometry(Widget w, 
				      XtWidgetGeometry *proposed, 
				      XtWidgetGeometry *answer)
#else
static XtGeometryResult QueryGeometry(w, proposed, answer)
Widget w;
XtWidgetGeometry *proposed; 
XtWidgetGeometry *answer;
#endif
{
    XfwfCircularPercentageWidget our_widget = (XfwfCircularPercentageWidget) w;
    XtGeometryResult xtgrOutcome = XtGeometryYes;

#ifdef WANT_NOISE
fprintf(stderr, "Proposed:\n\nreq_mode : %d,\n(x,y):(%d,%d)\n(width,height) : (%d,%d)\nbw: %d, sibling : %x, stack_mode : %i\n\n",
	proposed -> request_mode,
	proposed -> x, proposed -> y,
	proposed -> width, proposed -> height,
	proposed -> border_width,
	proposed -> sibling,
	proposed -> stack_mode);
#endif

    answer -> request_mode = CWWidth | CWHeight;
    answer -> width =  DEFAULT_LENGTH;
    answer -> height = DEFAULT_WIDTH;

    if (proposed -> request_mode & CWWidth)
      if (proposed -> width >= answer -> width)
	answer -> width = proposed -> width;
      else
	 xtgrOutcome = XtGeometryAlmost;

    if (proposed -> request_mode & CWHeight)
      if (proposed -> height >= answer -> height)
	answer -> height = proposed -> height ;
      else
	 xtgrOutcome = XtGeometryAlmost;

    if (answer -> width  == our_widget -> core.width &&
        answer -> height == our_widget -> core.height)
      xtgrOutcome = XtGeometryNo;

#ifdef WANT_NOISE
fprintf(stderr, "Answer:\n\n(width,height) : (%d,%d)\n",
	answer -> width, answer -> height);
#endif

    return xtgrOutcome;
}

#if NeedFunctionPrototypes > 0
static void MakeGC(XfwfCircularPercentageWidget w)
#else
static void MakeGC(w)
XfwfCircularPercentageWidget w;
#endif
{
  Display *pDisplay = XtDisplay(w);

  XGCValues xgcValues;

  xgcValues.function   = GXcopy;
  xgcValues.line_width = 1;
  xgcValues.line_style = LineSolid;
  xgcValues.fill_style = FillSolid;
  xgcValues.fill_rule  = EvenOddRule;
  xgcValues.cap_style  = CapRound;
  xgcValues.join_style = JoinMiter;
  xgcValues.arc_mode   = ArcPieSlice ;

  w -> circular_percentage.gc = XCreateGC(pDisplay,
				 XtWindow(w),
	                         GCFunction| 
			         GCLineWidth| 
			         GCLineStyle| 
			         GCFillStyle| 
			         GCFillRule|
			         GCCapStyle|
			         GCJoinStyle|
			         GCArcMode,
			         &xgcValues);
}

#if NeedFunctionPrototypes > 0
void XfwfCircularPercentageSetPercentage(Widget w, int iNewXfwfCircularPercentage)
#else
void XfwfCircularPercentageSetPercentage(w,iNewXfwfCircularPercentage)
Widget w;
int iNewXfwfCircularPercentage ;
#endif
{
  XfwfCircularPercentageWidget our_widget = (XfwfCircularPercentageWidget) w;

  if (! XtIsSubclass(w,xfwfCircularPercentageWidgetClass))
    return;

  if (iNewXfwfCircularPercentage < 0 || iNewXfwfCircularPercentage > 10000)
    {
     XtWarning("XfwfCircularPercentage: new value for percentage out of range [0,10000]).");
     return;
    }

  RedrawPart(our_widget, 
	     our_widget -> circular_percentage.iPercentageCompleted,
	     iNewXfwfCircularPercentage);

  our_widget -> circular_percentage.iPercentageCompleted = iNewXfwfCircularPercentage;
}

#if NeedFunctionPrototypes > 0
static void RedrawPart(XfwfCircularPercentageWidget our_widget,
		       int iOldXfwfCircularPercentage,
		       int iNewXfwfCircularPercentage)
#else
static void RedrawPart(our_widget,
		       iOldXfwfCircularPercentage,
		       iNewXfwfCircularPercentage)
XfwfCircularPercentageWidget our_widget;
int iOldXfwfCircularPercentage; 
int iNewXfwfCircularPercentage;
#endif
{
   Display *pDisplay = XtDisplay(our_widget);
   Window    window  = XtWindow(our_widget);

   XArc      old_arc[2] ;
   XArc      new_arc[2] ;

   short sDifference;
   Pixel drawing_color;
   short sAngle;

   if (! XtIsRealized((Widget) our_widget))
     return;

   if (our_widget -> circular_percentage.gc == None)
     { 
       Redisplay((Widget)our_widget,NULL);
       return;
     }

   if (iOldXfwfCircularPercentage == iNewXfwfCircularPercentage)
     return;

#ifdef WANT_NOISE
   fprintf(stderr,"*** Redraw Part (%d,%d)\n",iOldXfwfCircularPercentage,iNewXfwfCircularPercentage);
#endif

   CalculateArcs(our_widget, old_arc, iOldXfwfCircularPercentage);
   CalculateArcs(our_widget, new_arc, iNewXfwfCircularPercentage);


   if (iNewXfwfCircularPercentage < iOldXfwfCircularPercentage)
     { 
       sDifference = (old_arc[1].angle1 - new_arc[1].angle1);
       drawing_color = our_widget -> circular_percentage.interior_color;
       sAngle = new_arc[1].angle1;
     }
   else
     { 
       sDifference = (new_arc[1].angle1 - old_arc[1].angle1);
       drawing_color = our_widget -> circular_percentage.completed_color;
       sAngle = old_arc[1].angle1;
     }

   XSetForeground(pDisplay, 
   	          our_widget -> circular_percentage.gc,
		  drawing_color);

   XFillArc(pDisplay, window, our_widget -> circular_percentage.gc,
	    old_arc[1].x, old_arc[1].y, 
	    old_arc[1].width, old_arc[1].height,
	    sAngle, sDifference);

   XSetForeground(pDisplay, 
   	          our_widget -> circular_percentage.gc,
		  our_widget -> circular_percentage.border_color);

   XDrawArc(pDisplay, window, our_widget -> circular_percentage.gc,
	    old_arc[1].x, old_arc[1].y, 
	    old_arc[1].width, old_arc[1].height,
	    sAngle, sDifference);

   XFlush(pDisplay);
}
