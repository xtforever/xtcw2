/*************************************************************
 *
 * Rheostat.c
 * Rheostat widget implementation
 *
 * Author: Joe English, joe@trystero.art.com
 *
 * $Revision: 1.9 $
 *
 *************************************************************
 *
 * Future enhancements:
 *
 * Add set(value) action
 * SetValues() and Initialize() probably need to do more range checking
 * When number_intervals is is not a divisor of ValueRange, ticks
 *    aren't drawn where the arrow actually goes.
 */

/* HISTORY
 * 25-Jun-1993		James R. R. Service
 *    In calculation_position() the minimum value wasn't being subtracted
 *    from the current value.
 * 15-Jul-1993		Joe English
 *    In Set() action, peg the indicator to one side if the value
 *    is out of range instead of just freezing.
 * 13-Sep-1993		Gregory A. Janee
 *    With tick gravity on, clicking near a tick corresponding to
 *    a *negative* value caused the indicator to jump one tick to the
 *    right of the correct tick.
 * 26-Sep-1994		Daniel V. Klein (dvk@lonewolf.com)
 *    Fixed lots of bugs, made some enhancements:
 *	1) Allow inner_arrow_length and outer_arrow_length to be up to 150%
 *	   of radius (which allows arrows to have their center of rotation
 *         be inside the arrow body)
 *	2) Fixed calculations so dials can have reversed ranges (i.e., with
 *	   the largest value clockwise of the smallest)
 *	3) [ *** Undid this change (anything with a zero thickness was
 *               made invisible) --JE *** ]
 *	4) Tick marks and arc are now CapButt, instead of CapRound (which
 *	   gave problems with thick lines).
 *	5) The arrow is now JoinRound (instead of JoinMiter, which gave
 *	   problems with thick lines).
 *	6) SetValues is a lot smarter (checks more things, doesn't redraw the
 *	   whole widget if it doesn't need to).
 *	7) Intervals work correctly when begin_angle == end_angle.
 *	8) Added a meta-resource called "foreground", which sets all of
 *	   arrow_color, tick_color, and dial_color (if they are not being
 *	   set at the same time).
 *	9) Added a QueryGeometry method
 *
 * 11-Oct-1994		Joe English
 *	In Motif version, traverse to widget on Set action
 *	as per suggestion by Daniel Klein.
 */

#include <stdlib.h>
#include <math.h>

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#ifdef MOTIF
#	include <Xm/XmP.h>
#	include <Xm/PrimitiveP.h>
#endif

#include <Xfwf/RheostatP.h>
#include <Xfwf/Rheostat.h>

/***********************************************************************
 *
 * Convenience macros, defaults, and declarations:
 *
 ***********************************************************************/

#define RADIANS(d)  (M_PI * (d)/ 180.0)
#define DEGREES(r)  ((r) * 180.0 / M_PI)
#define DEG_TO_RAD (180.0 / M_PI)
#define RAD_TO_DEG (M_PI / 180.0)
#define MIN(x,y)	((x) < (y) ? (x) : (y))
#define MAX(x,y)	((x) > (y) ? (x) : (y))

/* Rheostat-specific: */
#define MinAngle(w)     (w->rheostat.minimum_angle)
#define MaxAngle(w)     (w->rheostat.maximum_angle)
#define MinValue(w)     (w->rheostat.minimum_value)
#define MaxValue(w)     (w->rheostat.maximum_value)
#define AngleRange(w)   (w->rheostat.maximum_angle-w->rheostat.minimum_angle)
#define ValueRange(w)   (w->rheostat.maximum_value-w->rheostat.minimum_value)
#define ValueInc(w)   	((w->rheostat.maximum_value-w->rheostat.minimum_value) \
			 / w->rheostat.number_intervals)
#define CenterX(w)      ((int)w->core.width/2)
#define CenterY(w)      ((int)w->core.height/2)

/* MARGIN(w) = space from border to radius. */
#define _MARGIN(w) \
    ( w->rheostat.outer_margin \
    + w->rheostat.tick_length \
    + w->rheostat.dial_thickness \
    + w->rheostat.inner_margin)
#ifdef MOTIF
#define MARGIN(w) (_MARGIN(w)  \
    + w->primitive.highlight_thickness  \
    + w->primitive.shadow_thickness)
#else
#define MARGIN(w) _MARGIN(w)
#endif

/* Cast operator to keep lint/gcc happy: */
#define W (Widget)

/*
 * Default values for various resources:
 */
#define MIN_RADIUS	2
#define DFLT_RADIUS	30
#define DFLT_OUTER	25
#define DFLT_INNER	20
#define DFLT_WIDTH	20
#define DFLT_ARROWTHICKNESS	0
/* Another good set of defaults: RADIUS=INNER=30, OUTER=15, WIDTH=10 */

#define DFLT_TICKLEN	5
#define DFLT_TICKTHICKNESS	0
#define DFLT_DIALTHICKNESS	3
#define DFLT_MINVALUE	0
#define DFLT_MAXVALUE	100
#define DFLT_MINANGLE	45
#define DFLT_MAXANGLE	315

#define DFLT_NUMTICKS	10
#define DFLT_MARGIN	2

/*
 * Method functions:
 */
static void     Initialize();
static void     Redisplay();
static void     Resize();
static void     Destroy();
static Boolean  SetValues();
static XtGeometryResult QueryGeometry();

/*
 * Action functions:
 */
static void     Set();
static void     Notify();
static void     Increment();

/*
 * Private functions:
 */
static void     draw_arrow	(/* XfwfRheostatWidget, GC */);
static void     draw_ticks      (/* XfwfRheostatWidget, GC */);
static void     draw_dial      	(/* XfwfRheostatWidget, GC */);
static void     calculate_position	(/* XfwfRheostatWidget */);
static void     get_GCs     	(/* XfwfRheostatWidget */);
static void     free_GCs     	(/* XfwfRheostatWidget */);
static void	call_callbacks	(/* XfwfRheostatWidget *, String, XEvent * */);


/***********************************************************************
 *
 * Translation, action, and resource tables:
 *
 ***********************************************************************/

#ifdef MOTIF		/* use osfXXX instead of XXX */
static char     default_translations[] =
    "<Btn1Down>:        set() \n\
     <Btn1Motion>:      set() \n\
     <Btn1Up>:      	notify() \n\
     <Key>minus:	increment(-1) notify() \n\
     <Key>plus:		increment(+1) notify() \n\
     <Key>osfPageUp:   	increment(+1i) notify() \n\
     <Key>osfPageDown:  increment(-1i) notify() \n\
     <Key>Return:	notify() \n\
    ";
#else			/* no "virtual keysym" braindamage */
static char     default_translations[] =
    "<Btn1Down>:        set() \n\
     <Btn1Motion>:      set() \n\
     <Btn1Up>:      	notify() \n\
     <Key>minus:	increment(-1) notify() \n\
     <Key>plus:		increment(+1) notify() \n\
     <Key>Prior:   	increment(-1i) notify() \n\
     <Key>Next:  	increment(+1i) notify() \n\
     <Key>Home:		increment(min) notify() \n\
     <Key>End:		increment(max) notify() \n\
     <Key>Return:	notify() \n\
    ";
#endif

static XtActionsRec actions[] = {
    { "set",    (XtActionProc) Set },
    { "notify", (XtActionProc) Notify },
    { "increment", (XtActionProc) Increment }
};

static XtResource resources[] = {
#   define OFFSET(x) (XtOffset(XfwfRheostatWidget, rheostat.x))
    {XtNvalue, XtCValue, XtRInt, sizeof(int),
        OFFSET(value), XtRImmediate, (caddr_t)0},
    {XtNminimumValue, XtCMinimum, XtRInt, sizeof(int),
        OFFSET(minimum_value), XtRImmediate, (caddr_t) DFLT_MINVALUE},
    {XtNmaximumValue, XtCMaximum, XtRInt, sizeof(int),
        OFFSET(maximum_value), XtRImmediate, (caddr_t) DFLT_MAXVALUE},
    {XtNminimumAngle, XtCMinimum, XtRInt, sizeof(int),
        OFFSET(minimum_angle), XtRImmediate, (caddr_t) DFLT_MINANGLE},
    {XtNmaximumAngle, XtCMaximum, XtRInt, sizeof(int),
        OFFSET(maximum_angle), XtRImmediate, (caddr_t) DFLT_MAXANGLE},
    {XtNtickGravity, XtCGravity, XtRBoolean, sizeof(Boolean),
        OFFSET(tick_gravity), XtRImmediate, (caddr_t)True},
    {XtNnumberIntervals, XtCNumberIntervals, XtRInt, sizeof(int),
        OFFSET(number_intervals), XtRImmediate, (caddr_t) DFLT_NUMTICKS},
    {XtNresizeArrow, XtCBoolean, XtRBoolean, sizeof(Boolean),
	OFFSET(resize_arrow), XtRString, "True"},
    {XtNsetCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
        OFFSET(set), XtRCallback, NULL},
    {XtNnotify, XtCCallback, XtRCallback, sizeof(XtPointer),
        OFFSET(notify), XtRCallback, NULL},
    {XtNouterMargin, XtCMargin, XtRDimension, sizeof(Dimension),
	OFFSET(outer_margin), XtRImmediate, (caddr_t) DFLT_MARGIN},
    {XtNtickLength, XtCMargin, XtRDimension, sizeof(Dimension),
        OFFSET(tick_length), XtRImmediate, (caddr_t) DFLT_TICKLEN},
    {XtNdialThickness, XtCThickness, XtRDimension, sizeof(Dimension),
        OFFSET(dial_thickness), XtRImmediate, (caddr_t) DFLT_DIALTHICKNESS},
    {XtNinnerMargin, XtCMargin, XtRDimension, sizeof(Dimension),
	OFFSET(inner_margin), XtRImmediate, (caddr_t) DFLT_MARGIN},
    {XtNradius, XtCLength, XtRDimension, sizeof(Dimension),
        OFFSET(radius), XtRImmediate, (caddr_t) DFLT_RADIUS},
    {XtNouterArrowLength, XtCLength, XtRDimension, sizeof(Dimension),
	OFFSET(outer_arrow_length), XtRImmediate, (caddr_t) DFLT_OUTER},
    {XtNinnerArrowLength, XtCLength, XtRDimension, sizeof(Dimension),
	OFFSET(inner_arrow_length), XtRImmediate, (caddr_t) DFLT_INNER},
    {XtNarrowWidth, XtCWidth, XtRDimension, sizeof(Dimension),
	OFFSET(arrow_width), XtRImmediate, (caddr_t) DFLT_WIDTH},
    {XtNfillArrow, XtCBoolean, XtRBoolean, sizeof(Boolean),
	OFFSET(fill_arrow), XtRString, "False" },
    {XtNarrowThickness, XtCThickness, XtRDimension, sizeof(Dimension),
	OFFSET(arrow_thickness), XtRImmediate, (caddr_t) DFLT_ARROWTHICKNESS},
    {XtNtickThickness, XtCThickness, XtRDimension, sizeof(Dimension),
        OFFSET(tick_thickness), XtRImmediate, (caddr_t) DFLT_TICKTHICKNESS},
#ifdef MOTIF
    {XtNuseShadowColors, XtCBoolean, XtRBoolean, sizeof(Boolean),
	OFFSET(use_shadow_colors), XtRString, "False"},
#endif
    {XtNarrowColor, XtCForeground, XtRPixel, sizeof(Pixel),
        OFFSET(arrow_pixel), XtRString, XtDefaultForeground},
    {XtNdialColor, XtCForeground, XtRPixel, sizeof(Pixel),
        OFFSET(dial_pixel), XtRString, XtDefaultForeground},
    {XtNtickColor, XtCForeground, XtRPixel, sizeof(Pixel),
        OFFSET(tick_pixel), XtRString, XtDefaultForeground},
#   undef OFFSET
};

XfwfRheostatClassRec    xfwfRheostatClassRec =
{
    /* CoreClassPart */
    {
#ifdef MOTIF
	/* superclass            */	(WidgetClass) &xmPrimitiveClassRec,
#else
	/* superclass            */	(WidgetClass) &widgetClassRec,
#endif
	/* class_name            */	"XfwfRheostat",
	/* widget_size           */	sizeof(XfwfRheostatRec),
	/* class_initialize      */	NULL,
	/* class_part_initialize */	NULL,
	/* class_inited          */	FALSE,
	/* initialize            */	Initialize,
	/* initialize_hook       */	NULL,
	/* realize               */	XtInheritRealize,
	/* actions               */	actions,
	/* num_actions           */	XtNumber(actions),
	/* resources             */	resources,
	/* num_resources         */	XtNumber(resources),
	/* xrm_class             */	NULLQUARK,
	/* compress_motion       */	TRUE,
	/* compress_exposure     */	TRUE,
	/* compress_enterleave   */	TRUE,
	/* visible_interest      */	TRUE,
	/* destroy               */	Destroy,
	/* resize                */	Resize,
	/* expose                */	Redisplay,
	/* set_values            */	SetValues,
	/* set_values_hook       */	NULL,
	/* set_values_almost     */	XtInheritSetValuesAlmost,
	/* get_values_hook       */	NULL,
	/* accept_focus          */	NULL,
	/* version               */	XtVersion,
	/* callback private      */	NULL,
	/* tm_table              */	default_translations,
	/* query_geometry        */	QueryGeometry,
	/* display_accelerator   */	XtInheritDisplayAccelerator,
	/* extension             */	NULL
    },
#ifdef MOTIF
    /* Primitive class fields */
    {
	/* border_highlight      */	(XtWidgetProc) _XtInherit,
	/* border_unhighlight    */	(XtWidgetProc) _XtInherit,
	/* translations          */	XtInheritTranslations,
	/* arm_and_activate      */	Notify,
	/* syn_resources         */	NULL,
	/* num_syn_resources     */	0,
	/* extension             */	NULL
    },
#endif
    /* XfwfRheostat class fields */
    {
	/* ignore                */	0
    }
};

WidgetClass     xfwfRheostatWidgetClass = (WidgetClass) &xfwfRheostatClassRec;

/***********************************************************************
 *
 * Method functions:
 *
 **********************************************************************/

/* fake_foreground_color -- added dvk */
static void fake_foreground_color (new, args, num_args)
    XfwfRheostatWidget  new;
    ArgList		args;
    Cardinal		*num_args;
{
    Boolean found_dial_pixel = FALSE;
    Boolean found_tick_pixel = FALSE;
    Boolean found_arrow_pixel = FALSE;
    Boolean found_foreground = FALSE;
    Pixel foreground;
    Cardinal n;

    /*
     * Cheat a little - we don't have an attribute called "foreground", but
     * if the user sets it, transfer those values to each of dial_pixel,
     * tick_pixel, and arrow_pixel (be careful not to override any of those
     * values if they are set at the same time).
     */
    for (n = 0; n < *num_args; n++) {
	if (strcmp (args[n].name, XtNforeground) == 0) {
	    found_foreground = TRUE;
	    foreground = args[n].value;
	    }
	else if (strcmp (args[n].name, XtNdialColor) == 0)
	    found_dial_pixel = TRUE;
	else if (strcmp (args[n].name, XtNtickColor) == 0)
	    found_tick_pixel = TRUE;
	else if (strcmp (args[n].name, XtNarrowColor) == 0)
	    found_arrow_pixel = TRUE;
	}
    if (found_foreground) {
	if (!found_dial_pixel)
	    new->rheostat.dial_pixel = foreground;
	if (!found_tick_pixel)
	    new->rheostat.tick_pixel = foreground;
	if (!found_arrow_pixel)
	    new->rheostat.arrow_pixel = foreground;
	}
}

/*
 * Initialize method:
 */
static void Initialize(request, new, args, num_args)
    XfwfRheostatWidget  request, new;
    ArgList		args;
    Cardinal		*num_args;
{
    int margin = MARGIN(new);
    int user_radius = new->rheostat.radius;	/* request from user */
    int size_radius = 				/* calculated from size */
	MIN(new->core.height,new->core.width)/2 - margin;
    int min_radius = 			/* from arrow dimens */
	(int)(MAX(	new->rheostat.inner_arrow_length,
			new->rheostat.outer_arrow_length) * 0.75);
    int min_dimen;

    /*
     * Check geometry:
     * Set radius from  user value, else widget size, else default.
     * Make sure radius is >= 150% of inner length & outer length
     *  (this allows, e.g., r=10,i=10,o=12) .
     * Make sure width and height are >= 2*(radius + margins)
     */
    if (user_radius != 0)
	new->rheostat.radius = user_radius;
    else if (new->core.width != 0 && new->core.height != 0)
	new->rheostat.radius = size_radius;
    else
        new->rheostat.radius = DFLT_RADIUS;

    /* Make sure radius is large enough: */
    if (new->rheostat.radius < min_radius)
	new->rheostat.radius = min_radius;

    /* Make sure widget is large enough: */
    min_dimen = 2*(new->rheostat.radius+margin);
    if (new->core.width < min_dimen)
	new->core.width = min_dimen;
    if (new->core.height < min_dimen)
	new->core.height = min_dimen;

#ifdef MOTIF
    if (new->rheostat.use_shadow_colors) {
	new->rheostat.arrow_pixel = new->primitive.bottom_shadow_color;
	new->rheostat.dial_pixel = new->primitive.top_shadow_color;
	new->rheostat.tick_pixel = new->primitive.top_shadow_color;
    }
#endif
    /* added, dvk */
    fake_foreground_color (new, args, num_args);
    get_GCs(new);
    calculate_position(new);

    new->rheostat.orig_radius = new->rheostat.radius;
    new->rheostat.orig_outer_length = new->rheostat.outer_arrow_length;
    new->rheostat.orig_inner_length = new->rheostat.inner_arrow_length;
    new->rheostat.orig_width = new->rheostat.arrow_width;
}

/*
 * Destroy method:
 */
static void Destroy(w)
    XfwfRheostatWidget      w;
{
    free_GCs(w);
}

/*
 * Resize method:
 */
static void Resize(w)
    XfwfRheostatWidget      w;
{
    int newr;

    newr =
       (w->core.height < w->core.width
      ? w->core.height : w->core.width) / 2 - MARGIN(w);

    if (newr < 2)
	newr = 2;

    if (w->rheostat.resize_arrow) {
	int oldr = w->rheostat.orig_radius;

	w->rheostat.outer_arrow_length =
		(w->rheostat.orig_outer_length * newr) / oldr;
	w->rheostat.inner_arrow_length =
		(w->rheostat.orig_inner_length * newr) / oldr;
	w->rheostat.arrow_width = (w->rheostat.orig_width * newr) / oldr;
    }
    w->rheostat.radius = newr;

    calculate_position(w);
}

/*
 * Expose method:
 */
static void Redisplay(w, event, region)
    XfwfRheostatWidget      w;
    XEvent         *event;
    Region          region;
{
#	ifdef MOTIF
	int hlt;
#	endif

	if (!XtIsRealized ((W w)))
	    return;

#	ifdef MOTIF
	hlt = w->primitive.highlight_thickness;
	_XmDrawShadow(XtDisplay(w), XtWindow(w),
		w->primitive.top_shadow_GC, w->primitive.bottom_shadow_GC,
		w->primitive.shadow_thickness,
		hlt, hlt, w->core.width - 2*hlt, w->core.height - 2*hlt);
#	endif

	draw_ticks(w, w->rheostat.tick_GC);
	draw_dial(w, w->rheostat.dial_GC);
        draw_arrow(w, w->rheostat.arrow_GC);
}

/*
 * SetValues:
 */
static Boolean SetValues(current, request, new, args, num_args)
    XfwfRheostatWidget	current, request, new;
    ArgList		args;
    Cardinal		*num_args;
{
    Boolean redraw = FALSE;		/* TRUE=>widget needs to be redrawn */
    Boolean recalc = FALSE;		/* TRUE=>arrow position changed */
#   define  CHECK(fld)  (new->fld != current->fld)

    /* added, -dvk */
    fake_foreground_color (new, args, num_args);

    /*
     * Check rheostat parameters:
     */
    if (   CHECK(rheostat.minimum_value) || CHECK(rheostat.maximum_value)
        || CHECK(rheostat.maximum_angle) || CHECK(rheostat.minimum_angle)
    )
    {
        recalc = TRUE;
        redraw = TRUE;
    }

    /*
     * Bounds check:
     */
    if (new->rheostat.value > new->rheostat.maximum_value)
        new->rheostat.value = new->rheostat.maximum_value;
    if (new->rheostat.value < new->rheostat.minimum_value)
        new->rheostat.value = new->rheostat.minimum_value;

    /*
     * Margin, geometry parameters -- may affect radius;
     */
    if (    CHECK(rheostat.outer_margin)
	 || CHECK(rheostat.dial_thickness)
	 || CHECK(rheostat.inner_margin)
#ifdef MOTIF
	 || CHECK(primitive.shadow_thickness)
	 || CHECK(primitive.highlight_thickness)
#endif
    )
    {
	int newr =
	   (new->core.height < new->core.width
	  ? new->core.height : new->core.width) / 2 - MARGIN(new);
	if (newr < MIN_RADIUS)
	    newr = MIN_RADIUS;
	new->rheostat.radius = newr;
	recalc = TRUE;
    }

    /*
     * Radius and thicknesses:
     */
    if (   CHECK(rheostat.radius)
	|| CHECK(rheostat.outer_arrow_length)
	|| CHECK(rheostat.inner_arrow_length)
	|| CHECK(rheostat.arrow_width)
    )
    {
	new->rheostat.orig_radius = new->rheostat.radius;
	new->rheostat.orig_outer_length = new->rheostat.outer_arrow_length;
	new->rheostat.orig_inner_length = new->rheostat.inner_arrow_length;
	new->rheostat.orig_width = new->rheostat.arrow_width;
	recalc = TRUE;
	redraw = TRUE;
    }

    if (   CHECK(rheostat.dial_thickness)
	|| CHECK(rheostat.tick_thickness)
    )
    {
	recalc = TRUE;
	redraw = TRUE;
    }
    /*
     * Check for color or line thickness change:
     */
    if (   CHECK(core.background_pixel)
        || CHECK(rheostat.tick_pixel)
        || CHECK(rheostat.dial_pixel)
        || CHECK(rheostat.arrow_pixel)
	|| CHECK(rheostat.arrow_thickness)
	|| CHECK(rheostat.dial_thickness)
	|| CHECK(rheostat.tick_thickness)
    )
    {
        get_GCs(new);
	free_GCs(current);
        redraw = TRUE;
    }

    /*
     * Other display resources:
     */
    if (   CHECK(rheostat.number_intervals)
	|| CHECK(rheostat.fill_arrow)
	|| CHECK(rheostat.tick_length)
    )
    {
	redraw = TRUE;
    }

    /*
     * Redraw arrow if that's all that's changed
     */
    if (   CHECK(rheostat.outer_arrow_length)
	|| CHECK(rheostat.inner_arrow_length)
	|| CHECK(rheostat.arrow_width)
        || CHECK(rheostat.value)
    )
    {
	if (redraw || !XtIsRealized ((W current))) {
	    recalc = TRUE;
	}
	else {
	    draw_arrow(current,current->rheostat.eraser_GC);
	    calculate_position(new);
	    draw_arrow(new,new->rheostat.arrow_GC);
	    recalc = FALSE;		/* Don't do it twice */
	}
    }

    /*
     * Wrap up:
     */
#   undef CHECK
    if (recalc)
	calculate_position(new);
    return redraw;
}

/*
 * QueryGeometry:
 */
static XtGeometryResult QueryGeometry (w, request, answer)
    XfwfRheostatWidget  w;
    XtWidgetGeometry *request, *answer;
{
    answer->request_mode = CWHeight | CWWidth;
    answer->height = 100;
    answer->width = 100;

    if (((request->request_mode & (CWHeight|CWWidth)) == (CWHeight|CWWidth)) &&
	    request->height == answer->height &&
	    request->width == answer->width)
	return XtGeometryYes;
    else if (answer->height == w->core.height && answer->width == w->core.width)
	return XtGeometryNo;
    else
	return XtGeometryAlmost;
}

/***********************************************************************
 *
 * Action functions:
 *
 ***********************************************************************/

    /*ARGSUSED*/
static void Set(w, event, params, nparams)
    XfwfRheostatWidget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*nparams;
{

#   ifdef MOTIF
    if (event->type == ButtonPress)
    {
	(void)XmProcessTraversal((Widget)w, XmTRAVERSE_CURRENT);
    }
#   endif

    if (event->type == ButtonPress || event->type == MotionNotify)
    {
        int x,y,v;
        double theta,length,radius;

        x = event->xbutton.x - CenterX(w);
        y = event->xbutton.y - CenterY(w);

        if (!x && !y)       /* click at center of widget -- no angle */
            return;
        /* else */

        radius = sqrt((double)(x*x + y*y));

        /*
         * Calculate value in range 0 .. 360
         */
        theta = DEGREES(atan2((double)(-x), (double)(y)));
	if (MinAngle(w) <= MaxAngle(w)) {
	    theta -= MinAngle(w);
	    if (theta < 0.0)
		theta += 360.0;
	    v = MinValue(w)
	      + (int)(theta * (double)ValueRange(w)
			    / (double)AngleRange(w));
	} else {
	    theta -= MaxAngle(w);
	    if (theta < 0.0)
		theta += 360.0;
	    v = MaxValue(w)
	      - (int)(theta * (double)ValueRange(w)
			    / (double)abs(AngleRange(w)));
	}

	/*
	 * If tick_gravity is on, and click is in tick region,
	 * snap to nearest increment:
	 */
        length = (double)(w->rheostat.radius + w->rheostat.inner_margin);
	if (   w->rheostat.tick_gravity
	    && radius >= length
	    && radius <= length
		+ w->rheostat.dial_thickness + w->rheostat.tick_length)
	{
	    double inc = ValueInc(w);
	    v = (v / inc + (v < 0 ? -0.5 : 0.5));
	    v *= inc;
	}

	/*
	 * Bounds-check:
	 * Note: v should never be < minimum_value.
	 */
        if (v > w->rheostat.maximum_value || v < w->rheostat.minimum_value)
	{
	    /*
	     * Value is out of range:
	     * peg the indicator at the minimum or maximum value,
	     * depending on the current value relative to the median value:
	     */
	    double median =
		(w->rheostat.maximum_value + w->rheostat.minimum_value) / 2.0;
	    if (w->rheostat.value <= median)
		v = w->rheostat.minimum_value;
	    else
		v = w->rheostat.maximum_value;
	}

        draw_arrow(w,w->rheostat.eraser_GC);
	w->rheostat.value = v;
	calculate_position(w);
        draw_arrow(w,w->rheostat.arrow_GC);
    }

    call_callbacks(w, XtNsetCallback, event);
}


static void Increment(w, event, params, nparams)
    XfwfRheostatWidget      w;
    XEvent         *event;
    String 	   *params;
    Cardinal	   *nparams;
{
    double inc = ValueInc(w);
    Boolean snap = False;
    double v;

    /*
     * Figure out increment from parameter:
     */
    v = w->rheostat.value;
    if (*nparams != 1) {	/* default: step to nearest interval: */
	v += inc;
	snap = True;
    } else {
	if (!strcmp(params[0],"max")) v = w->rheostat.maximum_value;
	else if (!strcmp(params[0],"min")) v = w->rheostat.minimum_value;
	else if (!strcmp(params[0],"+1i")) { v += inc; snap = True; }
	else if (!strcmp(params[0],"-1i")) { v -= inc; snap = True; }
	else v += atof(params[0]);
    }
    if (snap)
	v = (int)(v / inc + 0.5) * inc;

    /*
     * Bounds-check:
     */
    if (v > w->rheostat.maximum_value)
	v = w->rheostat.maximum_value;
    if (v < w->rheostat.minimum_value)
	v = w->rheostat.minimum_value;

    draw_arrow(w,w->rheostat.eraser_GC);
    w->rheostat.value = v;
    calculate_position(w);
    draw_arrow(w,w->rheostat.arrow_GC);

    call_callbacks(w,XtNsetCallback,event);
}

    /*ARGSUSED*/
static void Notify(w, event, params, nparams)
    XfwfRheostatWidget      w;
    XEvent         *event;
    String 	   *params;
    Cardinal	   *nparams;
{
    call_callbacks(w, XtNnotify, event);
}


/***********************************************************************
 *
 * Utility routines:
 *
 ***********************************************************************/
static void call_callbacks(w, callback_name, event)
    XfwfRheostatWidget w;
    char *callback_name;
    XEvent *event;
{
    XfwfRheostatCallbackStruct cb;

    cb.reason = 0;	/* Rheostat doesn't use this field */
    cb.event = event;
    cb.value = w->rheostat.value;

    XtCallCallbacks(W w, callback_name, (XtPointer)&cb);
}


static void draw_arrow(w, gc)
    XfwfRheostatWidget      w;
    GC              gc;

{
    XfwfDrawArrow(XtDisplay(w), XtWindow(w), gc,
	w->rheostat.tip_x, w->rheostat.tip_y,
	w->rheostat.tip_x - CenterX(w), w->rheostat.tip_y - CenterY(w),
	w->rheostat.outer_arrow_length,
	w->rheostat.inner_arrow_length,
	w->rheostat.arrow_width,
	w->rheostat.fill_arrow);
}

static void draw_dial(w, gc)
    XfwfRheostatWidget      w;
    GC     	         gc;
{
    int radius  = w->rheostat.radius
	+ w->rheostat.inner_margin
	+(w->rheostat.dial_thickness+1) / 2;

    XDrawArc(XtDisplay(w), XtWindow(w), gc,
	CenterX(w) - radius,
	CenterY(w) - radius,
	2*radius, 2*radius,
	64 * ((270 - w->rheostat.minimum_angle + 360) % 360),
	64 * -AngleRange(w)
    );
}

static void draw_ticks(w, gc)
    XfwfRheostatWidget      w;
    GC              gc;
{
    int i,cx,cy;
    double theta,inc;
    double ro,ri;       /* inner & outer radii of ticks */

    if (w->rheostat.number_intervals <= 0 || w->rheostat.tick_length == 0)
	return;

    /*
     * check if minimum_angle == maximum_angle (mod 360),
     * so the last tick doesn't coincide with the first.
     */
    if (w->rheostat.minimum_angle % 360 == w->rheostat.maximum_angle % 360)
	inc = RADIANS ((double)AngleRange (w)) /
	    (double)(w->rheostat.number_intervals + 1);
    else
	inc = RADIANS ((double)AngleRange (w)) /
	    (double)(w->rheostat.number_intervals);
    ri = (double)(w->rheostat.radius + w->rheostat.inner_margin
		+ (w->rheostat.dial_thickness+1)/2);
    ro = ri +  w->rheostat.tick_length + (w->rheostat.dial_thickness+1)/2;
    cx = CenterX(w);
    cy = CenterY(w);

    /*
     * Draw segments:
     */
    theta = RADIANS((double)MinAngle(w));
    i = w->rheostat.number_intervals + 1;
    while (i--) {
        double c = cos(theta);
        double s = sin(theta);

	XDrawLine(XtDisplay(w),XtWindow(w),gc,
	    cx - (int)(ro * s), cy + (int)(ro * c),
	    cx - (int)(ri * s), cy + (int)(ri * c));
        theta += inc;
    }
}

static void calculate_position(w)
    XfwfRheostatWidget      w;
{
    double theta,length;

    length = (double)w->rheostat.radius;
    /*
     * Calculate angle:
     *   theta = (V - minV)*(maxTheta-minTheta) / (maxV - minV) + minTheta
     */
    theta = (double)(w->rheostat.value - MinValue(w))
          * (double)(AngleRange(w))
          / (double)(ValueRange(w))
          + (double)(MinAngle(w));
    theta = RADIANS(theta);

    w->rheostat.tip_x = CenterX(w) - (int)(length * sin(theta));
    w->rheostat.tip_y = CenterY(w) + (int)(length * cos(theta));
}

/*
 * get_GCs
 * allocate foreground & background. GCs.
 */
static void get_GCs(w)
    XfwfRheostatWidget w;
{
    XGCValues       values;
    XtGCMask        mask;

    /*
     * dial:
     */
    mask = GCForeground | GCBackground | GCLineWidth | GCCapStyle;
    values.foreground = w->rheostat.dial_pixel;
    values.background = w->core.background_pixel;
    values.line_width = w->rheostat.dial_thickness;
    values.cap_style = CapButt;
    w->rheostat.dial_GC = XtGetGC(W w, mask, &values);

    /*
     * tick marks:
     */
    mask = GCForeground | GCBackground | GCFunction | GCLineWidth | GCCapStyle;
    values.foreground = w->rheostat.tick_pixel;
    values.background = w->core.background_pixel;
    values.function = GXcopy;
    values.line_width = w->rheostat.tick_thickness;
    values.cap_style = CapButt;
    w->rheostat.tick_GC = XtGetGC(W w, mask, &values);

    /*
     * Arrow:
     */
    mask = GCForeground | GCBackground | GCFunction | GCLineWidth
	 | GCCapStyle | GCJoinStyle | GCFillStyle;
    values.foreground = w->rheostat.arrow_pixel;
    values.background = w->core.background_pixel;
    values.line_width = w->rheostat.arrow_thickness;
    values.cap_style = CapRound;
    values.join_style = JoinRound;
    values.fill_style = FillSolid;
    w->rheostat.arrow_GC = XtGetGC(W w, mask, &values);

    /*
     * Eraser (identical to Arrow except fg & bg pixels are swapped):
     */
    values.foreground = w->core.background_pixel;
    values.background = w->rheostat.arrow_pixel;
    w->rheostat.eraser_GC = XtGetGC(W w, mask, &values);

    return;
}


static void free_GCs(w)
    XfwfRheostatWidget w;
{
    XtReleaseGC(W w,w->rheostat.arrow_GC);
    XtReleaseGC(W w,w->rheostat.eraser_GC);
    XtReleaseGC(W w,w->rheostat.dial_GC);
    XtReleaseGC(W w,w->rheostat.tick_GC);
    return;
}

#if NeedFunctionPrototypes
void XfwfDrawArrow(
    Display *dpy, Drawable d, GC gc,
    Position endx,
    Position endy,			/* position of arrow tip */
    int dx,
    int dy,				/* slope of arrow */
    Dimension outer_length,		/* distance tip->base */
    Dimension inner_length,		/* distance tip->inner */
    Dimension width,			/* distance base->outer points */
    Boolean fill			 /* True=>fill arrow,False=>outline */
)
#else
void XfwfDrawArrow(dpy, d, gc, endx, endy, dx, dy,
		       outer_length, inner_length, width, fill)
    Display *dpy;
    Drawable d;
    GC gc;
    Position endx, endy;		/* position of arrow tip */
    int dx, dy;				/* slope of arrow */
    Dimension outer_length;		/* distance tip->base */
    Dimension inner_length;		/* distance tip->inner */
    Dimension width;			/* distance base->outer points */
    Boolean fill;			/* True=>fill arrow,False=>outline */
#endif

{
    XPoint points[5];
    float scalef = sqrt((double)(dx*dx+dy*dy)); /* normalization factor */
    int al = (int)outer_length,
	bl = (int)inner_length,
	aw = (int)width / 2,
	lx = al *  dx / scalef,	/* distance from tip to base */
	ly = al *  dy / scalef,
	mx = bl *  dx / scalef,	/* distance from tip to middle point */
	my = bl *  dy / scalef,
	wx = aw * -dy / scalef,	/* distance from base to outer points */
	wy = aw *  dx / scalef;

    points[0].x = endx;  	points[0].y = endy;
    points[1].x = endx-lx + wx;	points[1].y = endy-ly + wy;
    points[2].x = endx-mx;	points[2].y = endy-my;
    points[3].x = endx-lx - wx;	points[3].y = endy-ly - wy;
    points[4].x = endx; 	points[4].y = endy;

    if (fill)
	XFillPolygon(dpy,d,gc,
		points,5,
		inner_length <= outer_length ? Nonconvex : Convex,
		CoordModeOrigin);
    else
	XDrawLines(dpy,d,gc,points,5,CoordModeOrigin);
}

/***********************************************************************
 *
 * Public functions:
 *
 ***********************************************************************/

/* XfwfRheostatSetIntCallback()
 * General-purpose callback function for XfwfRheostat widgets;
 * gets the position of the Rheostat, sets *(int *)closure
 *
 * %%% Does anyone actually use this routine?
 */

void XfwfRheostatSetIntCallback(w,closure,call_data)
    Widget w;
    XtPointer closure;
    XtPointer call_data;
{
    *((int *)closure) = ((XfwfRheostatCallbackStruct *)call_data)->value;
}
