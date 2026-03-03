
/*
 * PcBar.c - progress bar widget.
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <stdio.h>

#include "PcBarP.h"

#define H_DEFAULTWIDTH 		200  
#define H_DEFAULTHEIGHT 	20  
#define V_DEFAULTWIDTH 		30
#define V_DEFAULTHEIGHT 	200  

#define offset(field) XtOffsetOf(XfwfPcBarRec, field)

static XtResource resources[] = {
     {XtNforeground, 	XtCForeground, 	XtRPixel, 		sizeof(Pixel),		offset(pcBar.foreground), 	XtRString, 	  XtDefaultForeground},
	 {XtNpercentage,	XtCPercentage,	XtRInt,			sizeof(int),		offset(pcBar.percentage), XtRInt,      0},
	 {XtNdisplaypc,		XtCDisplaypc,	XtRBoolean,		sizeof(Boolean),	offset(pcBar.displaypc),  XtRBoolean,  False},
	 {XtNshowzero,		XtCShowzero,	XtRBoolean,		sizeof(Boolean),	offset(pcBar.showzero),   XtRBoolean,  False},
	 {XtNvertical,		XtCVertical,	XtRBoolean,		sizeof(Boolean),	offset(pcBar.vertical),   XtRBoolean,  False},
   	 {XtNfont, 			XtCFont, 		XtRFontStruct, 	sizeof (XFontStruct *), offset(pcBar.font),   XtRString,   XtDefaultFont},
};

/* Declaration of methods */

static void Initialize();
static void Redisplay();
static void Destroy();
static void Resize();
static Boolean SetValues();
static XtGeometryResult QueryGeometry();

/* these Core methods not needed:
 *
 * static void ClassInitialize();
 * static void Realize();
 */

XfwfPcBarClassRec xfwfPcBarClassRec = {
    {
    /* core_class fields */
    /* superclass	  	 */ (WidgetClass) &coreClassRec,
    /* class_name	  	 */ "XfwfPcBar",
    /* widget_size	  	 */ sizeof(XfwfPcBarRec),
    /* class_initialize   	 */ NULL,
    /* class_part_initialize	 */ NULL,
    /* class_inited       	 */ FALSE,
    /* initialize	  	 */ Initialize,
    /* initialize_hook		 */ NULL,
    /* realize		  	 */ XtInheritRealize,
    /* actions		  	 */ NULL,
    /* num_actions	  	 */ 0,
    /* resources	  	 */ resources,
    /* num_resources	  	 */ XtNumber(resources),
    /* xrm_class	  	 */ NULLQUARK,
    /* compress_motion	  	 */ TRUE,
    /* compress_exposure  	 */ XtExposeCompressMultiple,
    /* compress_enterleave	 */ TRUE,
    /* visible_interest	  	 */ FALSE,
    /* destroy		  	 */ Destroy,
    /* resize		  	 */ Resize,
    /* expose		  	 */ Redisplay,
    /* set_values	  	 */ SetValues,
    /* set_values_hook		 */ NULL,
    /* set_values_almost	 */ XtInheritSetValuesAlmost,
    /* get_values_hook		 */ NULL,
    /* accept_focus	 	 */ NULL,
    /* version			 */ XtVersion,
    /* callback_private   	 */ NULL,
    /* tm_table		   	 */ XtInheritTranslations,
    /* query_geometry		 */ QueryGeometry,
    /* display_accelerator       */ XtInheritDisplayAccelerator,
    /* extension                 */ NULL
    },
    {
    /* dummy_field               */ 0,
    },
};

WidgetClass xfwfPcBarWidgetClass = (WidgetClass) & xfwfPcBarClassRec;


static void
GetGC(w)
Widget w;
{
	XfwfPcBarWidget cw = (XfwfPcBarWidget) w;
    XGCValues values;
    XtGCMask mask = GCForeground | GCBackground;

    values.foreground = cw->pcBar.foreground;
    values.background = cw->core.background_pixel;

    cw->pcBar.draw_gc = XtGetGC((Widget) cw, mask, &values);

	values.function = GXxor;
	values.foreground = cw->pcBar.foreground ^ cw->core.background_pixel;
	mask = GCFunction | GCForeground | GCBackground;
    cw->pcBar.text_gc = XtGetGC((Widget) cw, mask, &values);
	XSetFont(XtDisplay(cw), cw->pcBar.text_gc, (cw->pcBar.font)->fid); 
}

/* ARGSUSED */
static void
Initialize(treq, tnew, args, num_args)
Widget treq, tnew;
ArgList args;
Cardinal *num_args;
{
	XfwfPcBarWidget new = (XfwfPcBarWidget) tnew;

    /* 
     *  Check instance values set by resources that may be invalid. 
     */

    if ((new->pcBar.percentage < 0) ||
		(new->pcBar.percentage > 100))  {
	XtWarning("PcBar: invalid percentage (using 0)."); 
    	new->pcBar.percentage = 0;
    }

    GetGC(new);
    if (new->core.width == 0) 
		new->core.width = new->pcBar.vertical ? V_DEFAULTWIDTH : H_DEFAULTWIDTH;
    if (new->core.height == 0) 
		new->core.height = new->pcBar.vertical ? V_DEFAULTHEIGHT : H_DEFAULTHEIGHT;

	}

/* ARGSUSED */
static void
Redisplay(w, event)
Widget w;
XExposeEvent *event;
{
	XfwfPcBarWidget cw = (XfwfPcBarWidget) w;

    if (!XtIsRealized((Widget) cw))
		return;

	XfwfPcBarSetPercentage(w, cw->pcBar.percentage);
	}

/* ARGSUSED */
static Boolean
SetValues(current, request, new, args, num_args)
Widget current, request, new;
ArgList args;
Cardinal *num_args;
{
    XfwfPcBarWidget curcw = (XfwfPcBarWidget) current;
    XfwfPcBarWidget newcw = (XfwfPcBarWidget) new;
    Boolean do_redisplay = False;

    if ((curcw->pcBar.foreground != newcw->pcBar.foreground) ||
    	(curcw->core.background_pixel != newcw->core.background_pixel) ||
     	(curcw->pcBar.font != newcw->pcBar.font)) {
		XtReleaseGC((Widget) curcw, curcw->pcBar.draw_gc);
		XtReleaseGC((Widget) curcw, curcw->pcBar.text_gc);
		GetGC(newcw);
		do_redisplay = True;
    }
    if (curcw->pcBar.percentage != newcw->pcBar.percentage) 
		do_redisplay = True;
    if (curcw->pcBar.displaypc != newcw->pcBar.displaypc) 
		do_redisplay = True;
    if ((curcw->pcBar.showzero != newcw->pcBar.showzero) && (newcw->pcBar.percentage == 0)) {
		do_redisplay = True;
		}
    if (curcw->pcBar.vertical != newcw->pcBar.vertical) 
		do_redisplay = True;

    return do_redisplay;
}


static void
Destroy(w)
Widget w;
{
	XfwfPcBarWidget cw = (XfwfPcBarWidget) w;

    if (cw->pcBar.draw_gc)
		XFreeGC(XtDisplay(cw), cw->pcBar.draw_gc);
}


/* ARGSUSED */
static void
Resize(w)
Widget w;
{
	XfwfPcBarWidget cw = (XfwfPcBarWidget) w;

}

static XtGeometryResult QueryGeometry(w, proposed, answer)
Widget w;
XtWidgetGeometry *proposed, *answer;
{
		/* accept any requested change in size */
        return XtGeometryYes;
}


void XfwfPcBarSetPercentage(w, pc)
Widget w;
int pc;
{
	XfwfPcBarWidget cw = (XfwfPcBarWidget) w;
    Display *disp;
    Window win;
	int x,y;
	int tx,ty;
	char buf[10];
	int asc, des, dir;
	XCharStruct overall;

	cw->pcBar.percentage=pc;
    if (!XtIsRealized((Widget) cw))
		return;

	disp=XtDisplay(cw);
	win=XtWindow(cw);

    XSetLineAttributes(disp,cw->pcBar.draw_gc, cw->core.height,
                         LineSolid,CapButt,JoinBevel);
	if (cw->pcBar.vertical) {
	    XSetLineAttributes(disp,cw->pcBar.draw_gc, cw->core.width,
                         LineSolid,CapButt,JoinBevel);
	    y= cw->core.height - (cw->pcBar.percentage * cw->core.height)/100;
	    XDrawLine(disp, win,cw->pcBar.draw_gc,  cw->core.width/2, y, cw->core.width/2, cw->core.height);
    	if (cw->pcBar.percentage <100) 
			XClearArea(disp, win, 0, 0, cw->core.width, y, False);
		}
	else {
	    XSetLineAttributes(disp,cw->pcBar.draw_gc, cw->core.height,
                         LineSolid,CapButt,JoinBevel);
	    x=(cw->pcBar.percentage * cw->core.width)/100;
	    XDrawLine(disp, win,cw->pcBar.draw_gc, 0, cw->core.height/2, x, cw->core.height/2);
    	XClearArea(disp, win, x, 0, cw->core.width, cw->core.height, False);
		}

	if ((cw->pcBar.displaypc) && (cw->pcBar.showzero || cw->pcBar.percentage > 0)) {
		sprintf(buf, "%d%%", cw->pcBar.percentage);
        XTextExtents(cw->pcBar.font, buf, strlen(buf), &dir, &asc, &des, &overall);
		tx = (cw->core.width-overall.width)/2;
		ty = (cw->core.height - (asc+des))/2 + asc;
		XDrawString(disp, win, cw->pcBar.text_gc, tx, ty,  buf, strlen(buf));
		}
	}
