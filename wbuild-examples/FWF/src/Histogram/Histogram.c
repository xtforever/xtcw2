/*****************************************************************************

	Histogram.c

	This file contains the C code "guts" of the histogram widget.

	Brian Totty

******************************************************************************/

/*
 * Author:
 * 	Brian Totty
 * 	Department of Computer Science
 * 	University Of Illinois at Urbana-Champaign
 *	1304 West Springfield Avenue
 * 	Urbana, IL 61801
 * 
 * 	totty@cs.uiuc.edu
 * 	
 */ 

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>

#include <Xfwf/HistogramP.h>
#include <Xfwf/Histogram.h>

/*===========================================================================*

           D E F I N I T I O N S    A N D    D E C L A R A T I O N S

 *===========================================================================*/

#define FontPad		1			/* Text Padding */
#define	FontAscent(f)	((f)->max_bounds.ascent)
#define	FontDescent(f)	((f)->max_bounds.descent)
#define	FontH(f)	(FontPad + FontAscent(f) + FontDescent(f) + FontPad)
#define	FontW(f,s)	XTextWidth(f,s,strlen(s))
#define	FontMaxCharW(f)	((f)->max_bounds.rbearing-(f)->min_bounds.lbearing+1)

#define	JUST_LEFT	0			/* Text Justification */
#define	JUST_RIGHT	1
#define	JUST_CENTER	2

#define	TITLE_PADDING_HEIGHT	3

#define max(a,b)	((a) > (b) ? (a) : (b))
#define min(a,b)	((a) < (b) ? (a) : (b))
#define abs(a)		((a) < 0 ? -(a) : (a))
#define	square(x)	((x) * (x))
#define XtStrlen(s)	((s) ? strlen(s) : 0)

#ifndef HUGE_VAL				/* Largest Float */
#define HUGE_VAL HUGE
#endif

/*===========================================================================*

    D E C L A R A T I O N S    O F    H I S T O G R A M    R O U T I N E S

 *===========================================================================*/

	/* Toolkit Methods */

#if (!NeedFunctionPrototypes)

static void		ClassInitialize();
static void		Initialize();
static void		Realize();
static void		Destroy();
static void		Resize();
static Boolean		SetValues();

#else

static void		ClassInitialize();
static void		Initialize(XfwfHistogramWidget old,
				XfwfHistogramWidget new);
static void		Realize(Widget gw,
				XtValueMask *requested_window_gc_value_mask,
				XSetWindowAttributes *requested_window_attrs);
static void		Destroy(Widget gw);
static void		Resize(Widget gw);
static Boolean		SetValues(Widget gcurrent, Widget grequest,
				Widget gnew);

#endif

	/* Internal Implementation Methods */

#if (!NeedFunctionPrototypes)

static void		InitializeBins();
static void		RecalcComponentCoordinates();
static void		Redisplay();
static void		CvtStringToFloat();
static void		RedrawAll();
static void		RedrawTitle();
static void		RedrawBinInfo();
static void		RedrawStats();
static void		RedrawXAxis();
static void		RedrawYAxis();
static void		RedrawData();
static int		BinCoords();
static void		RedrawSingleBar();
static void		RedrawEntireWidget();
static void		DrawJustifiedText();
static char *		ShortenFloat();
static int		SampleToBinNumber();
static int		CoordsToBinNumber();
static int		AddSample();
static void		Click();
static DATA_POINTS *	DataPointsNew();
static void		DataPointsFree();

#else

static void		InitializeBins(XfwfHistogramWidget w);
static void		RecalcComponentCoordinates(XfwfHistogramWidget w);
static void		Redisplay(XfwfHistogramWidget w, XEvent *event,
				Region region);
static void		CvtStringToFloat(XrmValuePtr args, int *arg_count,
				XrmValuePtr source, XrmValuePtr dest);
static void		RedrawAll(XfwfHistogramWidget w,
				int rx, int ry, int rw, int rh);
static void		RedrawTitle(XfwfHistogramWidget w,
				int rx, int ry, int rw, int rh);
static void		RedrawBinInfo(XfwfHistogramWidget w,
				int rx, int ry, int rw, int rh);
static void		RedrawStats(XfwfHistogramWidget w,
				int rx, int ry, int rw, int rh);
static void		RedrawXAxis(XfwfHistogramWidget w,
				int rx, int ry, int rw, int rh);
static void		RedrawYAxis(XfwfHistogramWidget w,
				int rx, int ry, int rw, int rh);
static void		RedrawData(XfwfHistogramWidget w,
				int rx, int ry, int rw, int rh);
static int		BinCoords(XfwfHistogramWidget w, int bin_number,
				int *left_x_ptr, int *top_y_ptr,
				int *w_ptr, int *h_ptr);
static void		RedrawSingleBar(XfwfHistogramWidget w, int bin_number);
static void		RedrawEntireWidget(XfwfHistogramWidget w);
static void		DrawJustifiedText(XfwfHistogramWidget w,
				int justification, int x, int y,
				int field_w, GC gc, XFontStruct *font,
				char *str);
static char *		ShortenFloat(register char *from, register char *to);
static int		SampleToBinNumber(XfwfHistogramWidget w,
				double sample, int *full_redraw_return);
static int		CoordsToBinNumber(XfwfHistogramWidget w,
				int x, int y, int check_vertically);
static int		AddSample(XfwfHistogramWidget w, double sample,
				int update_flag, int *bin_ptr);
static void		Click(Widget w, XEvent *event, char *params,
				Cardinal *num_params);
static DATA_POINTS *	DataPointsNew();
static void		DataPointsFree(DATA_POINTS *dp);

#endif

/*===========================================================================*

                           L O C A L    D A T A

 *===========================================================================*/

static unsigned char gray_stipple_data[32] =
{
	85, 85, 170, 170, 85, 85, 170, 170,
	85, 85, 170, 170, 85, 85, 170, 170,
	85, 85, 170, 170, 85, 85, 170, 170,
	85, 85, 170, 170, 85, 85, 170, 170
};

/*===========================================================================*

                   A C T I O N    T A B L E    D A T A

 *===========================================================================*/

static char action_translation_table[] = "<Btn1Down>: Click()";

static XtActionsRec actions[] =
{
	{"Click",	(XtActionProc)Click},
	{NULL,		(XtActionProc)NULL}
};

/*===========================================================================*

   R E S O U R C E    T Y P E S,    F I E L D S,    A N D    D E F A U L T S

 *===========================================================================*/

#define HistFieldOffset(field)	XtOffset(XfwfHistogramWidget,histogram.field)
#define CoreFieldOffset(field)	XtOffset(Widget,core.field)

static XtResource resources[] =
{
	{XtNwidth,XtCWidth,XtRDimension,sizeof(Dimension),
		CoreFieldOffset(width),XtRString,"600"},
	{XtNheight,XtCHeight,XtRDimension,sizeof(Dimension),
		CoreFieldOffset(height),XtRString,"250"},
	{XtNbackground,XtCBackground,XtRPixel,sizeof(Pixel),
		CoreFieldOffset(background_pixel),XtRString,"white"},

	{XtNbinWidth,XtCValue,XtRFloat,sizeof(float),
		HistFieldOffset(histogram_data.bin_width),XtRString,".5"},
	{XtNorigin,XtCValue,XtRFloat,sizeof(float),
		HistFieldOffset(histogram_data.origin),XtRString,"0"},
	{XtNbinCount,XtCValue,XtRInt,sizeof(int),
		HistFieldOffset(histogram_data.bin_count),XtRString,"20"},
	{XtNscale,XtCValue,XtRInt,sizeof(int),
		HistFieldOffset(histogram_data.scale),XtRString,"10"},
	{XtNscaleFactor,XtCValue,XtRFloat,sizeof(float),
		HistFieldOffset(histogram_data.scale_factor),XtRString,"2.0"},

	{XtNshowBinInfo,XtCBoolean,XtRBoolean,sizeof(Boolean),
		HistFieldOffset(options.show_bin_info),XtRString,"False"},
	{XtNshowXAxis,XtCBoolean,XtRBoolean,sizeof(Boolean),
		HistFieldOffset(options.show_x_axis),XtRString,"True"},
	{XtNshowYAxis,XtCBoolean,XtRBoolean,sizeof(Boolean),
		HistFieldOffset(options.show_y_axis),XtRString,"True"},
	{XtNshowAxisLabels,XtCBoolean,XtRBoolean,sizeof(Boolean),
		HistFieldOffset(options.show_axis_labels),XtRString,"True"},
	{XtNautoscale,XtCBoolean,XtRBoolean,sizeof(Boolean),
		HistFieldOffset(options.autoscale),XtRString,"True"},
	{XtNautobins,XtCBoolean,XtRBoolean,sizeof(Boolean),
		HistFieldOffset(options.autobins),XtRString,"False"},
	{XtNshowOutliers,XtCBoolean,XtRBoolean,sizeof(Boolean),
		HistFieldOffset(options.show_outlier_bins),XtRString,"False"},
	{XtNdiscrete,XtCBoolean,XtRBoolean,sizeof(Boolean),
		HistFieldOffset(options.discrete),XtRString,"False"},
	{XtNshowStats,XtCBoolean,XtRBoolean,sizeof(Boolean),
		HistFieldOffset(options.show_stats),XtRString,"False"},
	{XtNyAxisTickType,XtCValue,XtRInt,sizeof(int),
		HistFieldOffset(options.y_axis_tick_type),XtRString,"0"},
	{XtNverticalTicks,XtCValue,XtRInt,sizeof(int),
		HistFieldOffset(options.vertical_ticks),XtRString,"10"},

	{XtNbar,XtCForeground,XtRPixel,sizeof(Pixel),
	    	HistFieldOffset(x_stuff.bar_color),XtRString,"Blue"},
	{XtNaxis,XtCForeground,XtRPixel,sizeof(Pixel),
	    	HistFieldOffset(x_stuff.axis_color),XtRString,"Black"},
	{XtNtext,XtCForeground,XtRPixel,sizeof(Pixel),
	    	HistFieldOffset(x_stuff.text_color),XtRString,"Brown"},
	{XtNaxisLabel,XtCForeground,XtRPixel,sizeof(Pixel),
	    	HistFieldOffset(x_stuff.axis_label_color),XtRString,"Black"},
	{XtNtextFont,XtCFont,XtRFontStruct,sizeof(XFontStruct *),
		HistFieldOffset(x_stuff.text_font),XtRString,"variable"},
	{XtNaxisFont,XtCFont,XtRFontStruct,sizeof(XFontStruct *),
		HistFieldOffset(x_stuff.axis_font),XtRString,"6x13"},
	{XtNcallback,XtCCallback,XtRCallback,sizeof(caddr_t),
		HistFieldOffset(x_stuff.callbacks),XtRCallback,NULL},

	{XtNtitle,XtCLabel,XtRString,sizeof(String),
	        HistFieldOffset(attribs.title),XtRString,NULL}
};

#undef HistFieldOffset
#undef CoreFieldOffset

/*===========================================================================*

                 S T A T I C    A L L O C A T I O N    A N D

        I N I T I A L I Z A T I O N    O F    C L A S S    R E C O R D

 *===========================================================================*/

#define	SUPERCLASS (&widgetClassRec)

XfwfHistogramClassRec xfwfHistogramClassRec =
{
	{
		/* superclass		*/	SUPERCLASS,
		/* class_name		*/	"XfwfHistogram",
		/* widget_size		*/	sizeof(XfwfHistogramRec),
		/* class_initialize	*/	ClassInitialize,
		/* class_part_initialize*/	NULL,
		/* class_inited		*/	FALSE,
		/* initialize		*/	(XtInitProc)Initialize,
		/* initialize_hook	*/	NULL,
		/* realize		*/	(XtRealizeProc)Realize,
		/* actions		*/	actions,
		/* num_actions		*/	XtNumber(actions),
		/* resources		*/	resources,
		/* resource_count	*/	XtNumber(resources),
		/* xrm_class		*/	NULLQUARK,
		/* compress_motion	*/	TRUE,
		/* compress_exposure	*/	TRUE,
		/* compress_enterleave	*/	TRUE,
		/* visible_interest	*/	FALSE,
		/* destroy		*/	(XtWidgetProc)Destroy,
		/* resize		*/	(XtWidgetProc)Resize,
		/* expose		*/	(XtExposeProc)Redisplay,
		/* set_values		*/	(XtSetValuesFunc)SetValues,
		/* set_values_hook	*/	NULL,
		/* set_values_almost	*/	XtInheritSetValuesAlmost,
		/* get_values_hook	*/	NULL,
		/* accept_focus		*/	NULL,
		/* version		*/	XtVersion,
		/* callback_private	*/	NULL,
		/* tm_table		*/	action_translation_table,
		/* query_geometry       */	XtInheritQueryGeometry,
		/* display_accelerator  */	XtInheritDisplayAccelerator,
		/* extension            */	NULL
	}, /* Core Part */
	{
		/* dummy variable	*/	0
	}  /* XfwfHistogram Part */
};

WidgetClass xfwfHistogramWidgetClass = (WidgetClass)&xfwfHistogramClassRec;


/*===========================================================================*

                      T O O L K I T    M E T H O D S

 *===========================================================================*/

/*---------------------------------------------------------------------------*

	static void ClassInitialize()

	This procedure is called by the X toolkit to initialize the class.
	The class is only initialized once, and is shared among all
	instances of the class.  The hook to this routine is in the
	class_initialize part of the core part of the class.  We use this
	as a place to add resource type converters for the histogram class.

 *---------------------------------------------------------------------------*/

static void ClassInitialize()
{
	XtAddConverter(XtRString,XtRFloat,
		(XtConverter)CvtStringToFloat,NULL,0);
} /* End ClassInitialize */


/*---------------------------------------------------------------------------*

	static void Initialize(old,new)

	This procedure is called by the X toolkit to initialize the widget
	instance.  The widget will not be realized yet (no window will have
	been created), so only non-graphic values will be initialized.  The
	hook to this routine is in the initialize part of the core part of
	the class.

	The parameters <old> and <new> are widgets where <old> represents
	the widget before the superclass modified any of the parameters
	and <new> is the current state of the widget.  <old> can be used
	to determine the original requested data before the superclass
	changed anything.  The widget <new> will become the new widget.

 *---------------------------------------------------------------------------*/

static void Initialize(old,new)
XfwfHistogramWidget old,new;
{
	Widget w;
	int min_height,min_width;
	char *title;

	if (HistAttribs(new)->title != NULL)
	{
		title = XtMalloc(XtStrlen(HistAttribs(new)->title) + 1);
		strcpy(title,HistAttribs(new)->title);
		HistAttribs(new)->title = title;
	}

		/* Initialize Data Section */

	HistData(new)->data = NULL;
	if (HistOptions(new)->autobins)
		HistOptions(new)->keep_points = TRUE;
	if (HistOptions(new)->keep_points)
		HistData(new)->data = DataPointsNew();

	HistData(new)->plus_inf = HUGE_VAL;
	HistData(new)->minus_inf = - HUGE_VAL;
	HistData(new)->smallest = HistData(new)->plus_inf;
	HistData(new)->largest = HistData(new)->minus_inf;
	HistData(new)->N = 0;
	HistData(new)->sum_of_samples = 0;

		/* Initialize Bins */

	HistData(new)->bins = NULL;
	InitializeBins(new);

	HistX(new)->too_small = False;
} /* End Initialize */


/*---------------------------------------------------------------------------*

	static void Realize()

	This function is called to realize a histogram widget.

 *---------------------------------------------------------------------------*/

static void Realize(gw,requested_window_gc_value_mask,requested_window_attrs)
Widget gw;
XtValueMask *requested_window_gc_value_mask;
XSetWindowAttributes *requested_window_attrs;
{
	XtGCMask gc_value_mask;
	XGCValues gc_values;
	XfwfHistogramWidget w;
	Pixel fg,bg;

	w = (XfwfHistogramWidget)gw;
	XtCreateWindow(gw,InputOutput,(Visual *)CopyFromParent,
		*requested_window_gc_value_mask,requested_window_attrs);

	HistX(w)->erase_color = InstanceCore(w)->background_pixel;
	gc_value_mask = GCForeground | GCBackground | GCLineWidth;
	gc_values.background = HistX(w)->erase_color;
	gc_values.line_width = 0;

		/* Make A Read-Only GC For Erasing */

	gc_values.foreground = HistX(w)->erase_color;
	HistX(w)->erase_gc = XtGetGC((Widget)w,gc_value_mask,&gc_values);

		/* Make A Read-Only GC For Histogram Bars */

	gc_values.foreground = HistX(w)->bar_color;
	HistX(w)->bar_gc = XtGetGC((Widget)w,gc_value_mask,&gc_values);

		/* Make A Read-Only GC For Histogram Axes */

	gc_values.foreground = HistX(w)->axis_color;
	HistX(w)->axis_gc = XtGetGC((Widget)w,gc_value_mask,&gc_values);

		/* Make A Read-Only GC For Histogram Text */

	gc_values.foreground = HistX(w)->text_color;
	if (HistX(w)->text_font != NULL)
	{
	        gc_values.font = HistX(w)->text_font->fid;
		gc_value_mask |= GCFont;
	}
	HistX(w)->text_gc = XtGetGC((Widget)w,gc_value_mask,&gc_values);
	gc_value_mask &= ~GCFont;

		/* Make A Read-Only GC For Histogram Axis Labels */

	gc_values.foreground = HistX(w)->axis_label_color;
	if (HistX(w)->axis_font != NULL)
	{
	        gc_values.font = HistX(w)->axis_font->fid;
		gc_value_mask |= GCFont;
	}
	HistX(w)->axis_label_gc = XtGetGC((Widget)w,gc_value_mask,&gc_values);
	gc_value_mask &= ~GCFont;

		/* Make Stipple Pattern & Associated Graphic Context */

	HistX(w)->gray_pixmap = XCreateBitmapFromData(XtDisplay(w),
		XtWindow(w),(char *)gray_stipple_data,16,16);
	gc_value_mask |= (GCStipple | GCFillStyle);
	gc_values.foreground = HistX(w)->bar_color;
	gc_values.stipple = HistX(w)->gray_pixmap;
	gc_values.fill_style = FillStippled;
	HistX(w)->gray_gc = XtGetGC((Widget)w,gc_value_mask,&gc_values);
	gc_value_mask &= ~(GCStipple | GCFillStyle);

	Resize(gw);
} /* End Realize */


/*---------------------------------------------------------------------------*

	Destroy()

	This function is called to destroy a histogram widget.

 *---------------------------------------------------------------------------*/

static void Destroy(gw)
Widget gw;
{
	XfwfHistogramWidget w;

	w = (XfwfHistogramWidget)gw;

	XtDestroyGC(HistX(w)->erase_gc);
	XtDestroyGC(HistX(w)->bar_gc);
	XtDestroyGC(HistX(w)->axis_gc);
	XtDestroyGC(HistX(w)->text_gc);
	XtDestroyGC(HistX(w)->axis_label_gc);

	if (HistData(w)->data) DataPointsFree(HistData(w)->data);
	if (HistData(w)->bins) XtFree((char *)(HistData(w)->bins));
	if (HistAttribs(w)->title) XtFree(HistAttribs(w)->title);
} /* End Destroy */


/*---------------------------------------------------------------------------*

	Resize()

	This function is called to resize a histogram widget.

 *---------------------------------------------------------------------------*/

static void Resize(gw) 
Widget gw;
{
	XfwfHistogramWidget w;
	int *array,size,i;

	w = (XfwfHistogramWidget)gw;
	RecalcComponentCoordinates(w);
} /* End Resize */


/*---------------------------------------------------------------------------*

	SetValues()

 *---------------------------------------------------------------------------*/

/* ARGSUSED */
static Boolean SetValues(gcurrent,grequest,gnew)
Widget gcurrent,grequest,gnew;
{
	fprintf(stderr,"SetValues Unimplemented!!!!!\n");
	return(True);
} /* End SetValues */

/*===========================================================================*

                    I N T E R N A L    R O U T I N E S

 *===========================================================================*/

/*---------------------------------------------------------------------------*

	InitializeBins(w)

	This routine allocates the bins and initializes them, freeing
	any previous existing bins if they are not NULL.  This is used in
	initialization and when changing the number of bins.  The number
	of bins (not counting the two outlier bins which are always created
	even if outliers are disabled) must be set in the widget field
	bin_count before calling this routine.

 *---------------------------------------------------------------------------*/

static void InitializeBins(w)
XfwfHistogramWidget w;
{
	float sample;
	int bin,dummy;
	register int i;
	register DATA_BLOCK *db;
	register DATA_POINTS *dp;
	static int recursion_depth = 0;

	if (recursion_depth > 1)
	{
		fprintf(stderr,"InitializeBins: Recursion Depth Too High!\n");
		exit(-1);
	}
	if (HistData(w)->bins) XtFree((char *)(HistData(w)->bins));
	HistData(w)->size_of_fullest_bin = 0;
	HistData(w)->total_bin_count = HistData(w)->bin_count + 2;
	HistData(w)->bins = (int *)XtCalloc(HistData(w)->total_bin_count,
		sizeof(int));

		/* Find First & Last Visible Bins */

	if (HistOptions(w)->show_outlier_bins == False)
	{
		HistData(w)->first_bin = 1;
		HistData(w)->last_bin = HistData(w)->bin_count;
	}
	    else
	{
		HistData(w)->first_bin = 0;
		HistData(w)->last_bin = HistData(w)->bin_count + 1;
	}

		/* Reload Data If There Is Any */

	if (HistData(w)->data)
	{
		dp = HistData(w)->data;
		for (db = DataPointsBlockList(dp); db != NULL;
		     db = DataBlockNext(db))
		{
			for (i = 0; i < DataBlockBytesUsed(db); i++)
			{
				sample = DataBlockData(db)[i];
				++ recursion_depth;
				bin = SampleToBinNumber(w,sample,&dummy);
				-- recursion_depth;
				++ (HistData(w)->bins)[bin];
			}
		}
	}	
} /* End InitializeBins */


/*---------------------------------------------------------------------------*

	RecalcComponentCoordinates(w)

	This routine takes a widget <w> and recalculates coordinate
	data that describes positions of the components of the
	histogram widget.  This routine is usually called after a
	resize event.

 *---------------------------------------------------------------------------*/

static void RecalcComponentCoordinates(w)
XfwfHistogramWidget w;
{
	int info_h,title_h,wid_h,wid_w;
	char buffer[32];

	HistCoords(w)->cx = HistCore(w)->width / 2;
	HistCoords(w)->cy = HistCore(w)->height / 2;

	HistCoords(w)->title_y = TITLE_PADDING_HEIGHT;
	HistCoords(w)->title_h = (HistAttribs(w)->title != NULL ?
		FontH(HistX(w)->text_font) + 1 : 0);
	HistCoords(w)->bin_info_y = HistCoords(w)->title_y +
		HistCoords(w)->title_h;
	HistCoords(w)->bin_info_h = (HistOptions(w)->show_bin_info ?
		FontH(HistX(w)->text_font) + 1 : 0);
	HistCoords(w)->stats_y = HistCoords(w)->bin_info_y +
		HistCoords(w)->bin_info_h;
	HistCoords(w)->stats_h = (HistOptions(w)->show_stats ?
		FontH(HistX(w)->text_font) + 1 : 0);
	HistCoords(w)->bar_top_y = HistCoords(w)->stats_y +
		HistCoords(w)->stats_h +
		(HistOptions(w)->show_y_axis &&
		 HistOptions(w)->show_axis_labels ?
			FontH(HistX(w)->axis_font) : 0);
	if (HistOptions(w)->show_x_axis)
	{
		HistCoords(w)->x_label_y = HistCore(w)->height - 1 -
			FontH(HistX(w)->axis_font);
		HistCoords(w)->x_tick_y = HistCoords(w)->x_label_y - 2;
		HistCoords(w)->x_axis_y = HistCoords(w)->x_tick_y - 5;
		HistCoords(w)->bar_bottom_y = HistCoords(w)->x_axis_y - 1;
	}
	    else
	{
		HistCoords(w)->x_label_y = HistCore(w)->height - 1;
		HistCoords(w)->x_tick_y = HistCore(w)->height - 1;
		HistCoords(w)->x_axis_y = HistCore(w)->height - 1;
		HistCoords(w)->bar_bottom_y = HistCore(w)->height - 1;
	}

	if (HistOptions(w)->show_y_axis)
	{
		switch(HistOptions(w)->y_axis_tick_type)
		{
		    case Y_AXIS_LABEL_BIN_FRACTION:
			strcpy(buffer,"100.0%");
			break;
		    default:
			sprintf(buffer,"%ld",HistData(w)->scale);
			break;
		}
		HistCoords(w)->y_label_w = strlen(buffer) *
			FontMaxCharW(HistX(w)->axis_font) + 5;
		HistCoords(w)->y_label_x = 1;
		HistCoords(w)->y_tick_x = HistCoords(w)->y_label_x +
			HistCoords(w)->y_label_w - 1;
		HistCoords(w)->y_axis_x = HistCoords(w)->y_tick_x + 5;
		HistCoords(w)->bar_left_x = HistCoords(w)->y_axis_x + 1;
	}
	    else
	{
		HistCoords(w)->y_label_x = 0;
		HistCoords(w)->y_label_w = 0;
		HistCoords(w)->y_tick_x = 0;
		HistCoords(w)->y_axis_x = 0;
		HistCoords(w)->bar_left_x = 0;
	}

	HistData(w)->bars_across = HistData(w)->bin_count +
		(HistOptions(w)->show_outlier_bins ? 2 : 0);

	HistCoords(w)->bar_w =
		(HistCore(w)->width - 1 - HistCoords(w)->bar_left_x) /
		HistData(w)->bars_across;
	HistCoords(w)->bar_right_x = HistCoords(w)->bar_left_x +
		HistCoords(w)->bar_w * HistData(w)->bars_across - 1;

	HistX(w)->too_small = False;
	HistCoords(w)->bar_h = HistCoords(w)->bar_bottom_y -
		HistCoords(w)->bar_top_y + 1;
	if (HistCoords(w)->bar_h < 5) HistX(w)->too_small = True;
	if (HistCoords(w)->bar_right_x - HistCoords(w)->bar_left_x < 5)
		HistX(w)->too_small = True;
} /* End RecalcComponentCoordinates() */


/*---------------------------------------------------------------------------*

	Redisplay(w,event,region)

	This routine is called to redraw the region <region>
	of the widget <gw>.  The event that caused the redraw
	is pointed to by <event>.

 *---------------------------------------------------------------------------*/

/* ARGSUSED */
static void Redisplay(w,event,region)
XfwfHistogramWidget w;
XEvent *event;
Region region;
{
	int rx,ry,rw,rh;

	rx = event->xexpose.x;
	ry = event->xexpose.y;
	rw = event->xexpose.width;
	rh = event->xexpose.height;

	if (HistX(w)->too_small)
	{
		XFillRectangle(XtDisplay(w),XtWindow(w),HistX(w)->text_gc,
			rx,ry,rw,rh);
		return;
	}
	RedrawAll(w,rx,ry,rw,rh);
} /* End Redisplay */


/*---------------------------------------------------------------------------*

	static void CvtStringToFloat(args,arg_count,source,dest)

	This routine is an X type converter.  It takes a String type input
	and generates a Float type output.

 *---------------------------------------------------------------------------*/

static void CvtStringToFloat(args,arg_count,source,dest)
XrmValuePtr args;
int *arg_count;
XrmValuePtr source,dest;
{
	char message[80];
	static float result;

	if (sscanf(source->addr,"%f",&result) != 1)
	{
		sprintf(message,"String '%s' can't be converted int a float\n",
			source->addr);
		XtWarning(message);
	}
	    else
	{
		dest->addr = (char *)&result;
		dest->size = sizeof(float);
	}
} /* End CvtStringToFloat */


/*---------------------------------------------------------------------------*

	RedrawAll(w,rx,ry,rw,rh)

	This function redraws all the components of the histogram widget
	<w>.  The region needing redrawing is the rectangle <rx,ry,rw,rh>.

 *---------------------------------------------------------------------------*/

static void RedrawAll(w,rx,ry,rw,rh)
XfwfHistogramWidget w;
int rx,ry,rw,rh;
{
	XFillRectangle(XtDisplay(w),XtWindow(w),
		HistX(w)->erase_gc,rx,ry,rw,rh);
	if (HistAttribs(w)->title) RedrawTitle(w,rx,ry,rw,rh);
	if (HistOptions(w)->show_bin_info) RedrawBinInfo(w,rx,ry,rw,rh);
	if (HistOptions(w)->show_stats) RedrawStats(w,rx,ry,rw,rh);
	if (HistOptions(w)->show_x_axis) RedrawXAxis(w,rx,ry,rw,rh);
	if (HistOptions(w)->show_y_axis) RedrawYAxis(w,rx,ry,rw,rh);
	RedrawData(w,rx,ry,rw,rh);
} /* End RedrawAll */


/*---------------------------------------------------------------------------*

	RedrawTitle(w,rx,ry,rw,rh)

	This function redraws the title line for the widget <w>.  The
	region needing redrawing is the rectangle <rx,ry,rw,rh>.

 *---------------------------------------------------------------------------*/

static void RedrawTitle(w,rx,ry,rw,rh)
XfwfHistogramWidget w;
int rx,ry,rw,rh;
{
	if (HistAttribs(w)->title)
	{
		XFillRectangle(XtDisplay(w),XtWindow(w),HistX(w)->erase_gc,0,
			HistCoords(w)->title_y,HistCore(w)->width,
			HistCoords(w)->title_h);
		DrawJustifiedText(w,JUST_CENTER,0,HistCoords(w)->title_y,
			HistCore(w)->width,HistX(w)->text_gc,
			HistX(w)->text_font,HistAttribs(w)->title);
	}
} /* End RedrawTitle */



/*---------------------------------------------------------------------------*

	RedrawBinInfo(w,rx,ry,rw,rh)

	This function redraws the bin info line for the widget <w>.  The
	region needing redrawing is the rectangle <rx,ry,rw,rh>.

 *---------------------------------------------------------------------------*/

static void RedrawBinInfo(w,rx,ry,rw,rh)
XfwfHistogramWidget w;
int rx,ry,rw,rh;
{
	char buffer[256];

	if (HistOptions(w)->show_bin_info)
	{
		sprintf(buffer,"Bins: %d, Width: %.2f, Max Count: %ld",
			HistData(w)->bin_count,HistData(w)->bin_width,
			HistData(w)->size_of_fullest_bin);
		XFillRectangle(XtDisplay(w),XtWindow(w),HistX(w)->erase_gc,0,
			HistCoords(w)->bin_info_y,HistCore(w)->width,
			HistCoords(w)->bin_info_h);
		DrawJustifiedText(w,JUST_CENTER,0,HistCoords(w)->bin_info_y,
			HistCore(w)->width,HistX(w)->text_gc,
			HistX(w)->text_font,buffer);
	}
} /* End RedrawBinInfo */


/*---------------------------------------------------------------------------*

	RedrawStats(w,rx,ry,rw,rh)

	This function redraws the stats line for the widget <w>.  The
	region needing redrawing is the rectangle <rx,ry,rw,rh>.

 *---------------------------------------------------------------------------*/

static void RedrawStats(w,rx,ry,rw,rh)
XfwfHistogramWidget w;
int rx,ry,rw,rh;
{
	char buffer[256],minv[256],maxv[256],mean[256],variance[256];

	if (HistOptions(w)->show_stats)
	{
		sprintf(buffer,"%f",HistData(w)->mean);
		ShortenFloat(buffer,mean);
		sprintf(buffer,"%f",HistData(w)->variance);
		ShortenFloat(buffer,variance);
		sprintf(buffer,"%f",HistData(w)->smallest);
		ShortenFloat(buffer,minv);
		sprintf(buffer,"%f",HistData(w)->largest);
		ShortenFloat(buffer,maxv);
		sprintf(buffer,
			"N: %d, Min: %s, Max: %s, Mean: %s, Variance: %s",
			HistData(w)->N,minv,maxv,mean,variance);
		XFillRectangle(XtDisplay(w),XtWindow(w),HistX(w)->erase_gc,0,
			HistCoords(w)->stats_y,HistCore(w)->width,
			HistCoords(w)->stats_h);
		DrawJustifiedText(w,JUST_CENTER,0,HistCoords(w)->stats_y,
			HistCore(w)->width,HistX(w)->text_gc,
			HistX(w)->text_font,buffer);
	}
} /* End RedrawStats */


/*---------------------------------------------------------------------------*

	RedrawXAxis(w,rx,ry,rw,rh)

	This function redraws the X axis, tick marks and labels for the
	widget <w>.  The region needing redrawing is the rectangle
	<rx,ry,rw,rh>.

 *---------------------------------------------------------------------------*/

static void RedrawXAxis(w,rx,ry,rw,rh)
XfwfHistogramWidget w;
int rx,ry,rw,rh;
{
	int i,tick_x,top,bottom,label_width,left,right,prev_right,label_len;
	float bar_start;
	char label[32],buffer[32];

	XDrawLine(XtDisplay(w),XtWindow(w),HistX(w)->axis_gc,
		HistCoords(w)->y_axis_x,HistCoords(w)->x_axis_y,
		HistCoords(w)->bar_right_x,HistCoords(w)->x_axis_y);

	prev_right = -32768;
	for (i = HistData(w)->first_bin; i <= HistData(w)->last_bin; i++)
	{
		tick_x = HistCoords(w)->y_axis_x +
			(i - HistData(w)->first_bin) * HistCoords(w)->bar_w;
		top = HistCoords(w)->x_axis_y;
		bottom = HistCoords(w)->x_tick_y;
		XDrawLine(XtDisplay(w),XtWindow(w),HistX(w)->axis_gc,
			tick_x,top,tick_x,bottom);
		if (HistOptions(w)->show_axis_labels)
		{
			if (i == 0)
				strcpy(label,"<");
			    else if (i == HistData(w)->bin_count + 1)
				strcpy(label,">");
			    else
			{
				bar_start = (i - 1) * HistData(w)->bin_width +
					HistData(w)->origin;
				if (bar_start == 0.0)
				{
					label[0] = '0';
					label[1] = '\0';
				}
				    else
				{
					sprintf(buffer,"%f",bar_start);
					ShortenFloat(buffer,label);
				}
			}
			label_width = FontW(HistX(w)->axis_font,label);
			left = tick_x - label_width / 2;
			right = tick_x + label_width - 1;
			if ((left >= prev_right) || (i == 1))
			{
				XDrawLine(XtDisplay(w),XtWindow(w),
					  HistX(w)->axis_gc,tick_x,
					  HistCoords(w)->x_label_y,
					  tick_x,top);
				DrawJustifiedText(w,JUST_LEFT,
					left,HistCoords(w)->x_label_y,
					label_width,HistX(w)->axis_label_gc,
					HistX(w)->axis_font,label);
				prev_right = right;
			}
		}
	}
} /* End RedrawXAxis */


/*---------------------------------------------------------------------------*

	RedrawYAxis(w,rx,ry,rw,rh)

	This function redraws the Y axis, tick marks and labels for the
	widget <w>.  The region needing redrawing is the rectangle
	<rx,ry,rw,rh>.

 *---------------------------------------------------------------------------*/

static void RedrawYAxis(w,rx,ry,rw,rh)
XfwfHistogramWidget w;
int rx,ry,rw,rh;
{
	int i,vertical_ticks,count_per_tick,tick_h,tick_y,left,right,count;
	int prev_top,label_width,label_height,top,bottom;
	char label[32],buffer[32];
	float fraction_of_scale,fraction_of_total;

	XDrawLine(XtDisplay(w),XtWindow(w),HistX(w)->axis_gc,
		HistCoords(w)->y_axis_x,HistCoords(w)->x_axis_y,
		HistCoords(w)->y_axis_x,HistCoords(w)->bar_top_y);
	if (HistOptions(w)->vertical_ticks > HistData(w)->scale)
		vertical_ticks = HistData(w)->scale;
	    else
		vertical_ticks = HistOptions(w)->vertical_ticks;
	vertical_ticks = max(vertical_ticks,1);
	count = 0;
	prev_top = 32768;
	label_height = FontH(HistX(w)->axis_font);
	for (i = 0; i <= vertical_ticks; i++)
	{
		switch(HistOptions(w)->y_axis_tick_type)
		{
		    case Y_AXIS_LABEL_BIN_COUNT:
			fraction_of_scale = (float)count /
				(float)HistData(w)->scale;
			sprintf(label,"%d",count);
			break;
		    case Y_AXIS_LABEL_BIN_FRACTION:
			fraction_of_scale = (float)count /
				(float)HistData(w)->scale;
			sprintf(label,"%.1f%%",fraction_of_scale * 100.0);
			break;
		    default:
			fprintf(stderr,"Unknown yAxisTickType %d\n",
				HistOptions(w)->y_axis_tick_type);
			exit(1);
		}
		tick_h = (fraction_of_scale * (float)HistCoords(w)->bar_h);
		tick_y = HistCoords(w)->x_axis_y - tick_h;
		right = HistCoords(w)->y_axis_x;
		left = HistCoords(w)->y_tick_x;
		XDrawLine(XtDisplay(w),XtWindow(w),HistX(w)->axis_gc,
			left,tick_y,right,tick_y);
		if (HistOptions(w)->show_axis_labels)
		{
			label_width = FontW(HistX(w)->axis_font,label);
			bottom = tick_y + label_height / 2;
			top = bottom - label_height + 1;
			if (bottom <= prev_top)
			{
				DrawJustifiedText(w,JUST_CENTER,
					HistCoords(w)->y_label_x,top,
					HistCoords(w)->y_label_w,
					HistX(w)->axis_label_gc,
					HistX(w)->axis_font,label);
				prev_top = top;
			}
		}
		switch(HistOptions(w)->y_axis_tick_type)
		{
		    case Y_AXIS_LABEL_BIN_COUNT:
		    case Y_AXIS_LABEL_BIN_FRACTION:
			count_per_tick = HistData(w)->scale /
				max(vertical_ticks,1);
			break;
		}
		count += count_per_tick;
	}
} /* End RedrawYAxis */


/*---------------------------------------------------------------------------*

	RedrawData(w,rx,ry,rw,rh)

	This function redraws the actual histogram data bars for the
	widget <w>.  The region needing redrawing is the rectangle
	<rx,ry,rw,rh>.

 *---------------------------------------------------------------------------*/

static void RedrawData(w,rx,ry,rw,rh)
XfwfHistogramWidget w;
int rx,ry,rw,rh;
{
	int i;

	for (i = HistData(w)->first_bin; i <= HistData(w)->last_bin; i++)
	{
		RedrawSingleBar(w,i);
	}
} /* End RedrawData */


/*---------------------------------------------------------------------------*

       static int BinCoords(w,bin_number,left_x_ptr,top_y_ptr,w_ptr,h_ptr)

       This routine returns the coordinates of the bin numbered <bin_number>
       returning TRUE if the numbered bin is visible and FALSE otherwise.
       If the bin is not visible, the returned coordinates will be garbage.

 *---------------------------------------------------------------------------*/

static int BinCoords(w,bin_number,left_x_ptr,top_y_ptr,w_ptr,h_ptr)
XfwfHistogramWidget w;
int bin_number,*left_x_ptr,*top_y_ptr,*w_ptr,*h_ptr;
{
	int count;
	float fraction_of_scale;

	if (bin_number < 0 || bin_number >= HistData(w)->total_bin_count)
		return(FALSE);
	count = HistData(w)->bins[bin_number];
	fraction_of_scale = (float)count / (float)HistData(w)->scale;
	*w_ptr = HistCoords(w)->bar_w;
	*h_ptr = (int)(fraction_of_scale * (float)HistCoords(w)->bar_h);
	*left_x_ptr = HistCoords(w)->bar_left_x +
		(bin_number - HistData(w)->first_bin) * HistCoords(w)->bar_w;
	*top_y_ptr = HistCoords(w)->bar_bottom_y - *h_ptr + 1;
	return(TRUE);
} /* End BinCoords */


/*---------------------------------------------------------------------------*

	RedrawSingleBar(w,bin_number)

	This routine takes the histogram widget <w> and redraws the
	bar corresponding to bin <bin_number>, where the bin number
	varies from 0 to bin_count-1.

 *---------------------------------------------------------------------------*/

static void RedrawSingleBar(w,bin_number)
XfwfHistogramWidget w;
int bin_number;
{
	int count;
	int left,top,width,height;
	float fraction_of_scale;
	GC gc;

	BinCoords(w,bin_number,&left,&top,&width,&height);
	if (bin_number < 1 || bin_number > HistData(w)->bin_count)
		gc = HistX(w)->gray_gc;
	    else
		gc = HistX(w)->bar_gc;
	XFillRectangle(XtDisplay(w),XtWindow(w),gc,left,top,width,height);
} /* End RedrawSingleBar */


/*---------------------------------------------------------------------------*

	RedrawEntireWidget(w)

	This routine does a full redraw of the entire area of the
	widget <w>.

 *---------------------------------------------------------------------------*/

static void RedrawEntireWidget(w)
XfwfHistogramWidget w;
{
	RedrawAll(w,0,0,HistCore(w)->width,HistCore(w)->height);
} /* End RedrawEntireWidget */


/*---------------------------------------------------------------------------*

	DrawJustifiedText(w,justification,x,y,field_w,gc,font,str)

	This routine draws the string <str> with the graphic context <gc>
	on the widget <w>.  The <justification> parameter indicates what
	type of justification to do, JUST_LEFT, JUST_CENTER, or JUST_RIGHT.

	Left justification draws the text proceeding to the right, with
	the upper left corner of the text placed at <x>,<y>. Right
	justification draws the text to the left, with the upper right
	corner at <x>,<y>.  Center justification draws the text centered
	between <x> and <x> + <field_width) - 1, with the top at <y>.
	Field width is only used for center justification.  The <font>
	parameter should match the font in the <gc> and is used to quickly
	obtained text extent information.  If the text is too big, it will
	be clipped.

 *---------------------------------------------------------------------------*/

static void DrawJustifiedText(w,justification,x,y,field_w,gc,font,str)
XfwfHistogramWidget w;
int justification,x,y,field_w;
GC gc;
XFontStruct *font;
char *str;
{
	int txt_width,base_x,base_y;

	if (str == NULL) return;
	txt_width = FontW(font,str);
	base_y = y + FontPad + FontAscent(font);
	switch (justification)
	{
	    case JUST_LEFT:
		base_x = x;
		break;
	    case JUST_RIGHT:
		base_x = x - txt_width;
		break;
	    case JUST_CENTER:
		base_x = x + (field_w - txt_width) / 2;
		break;
	    default:
		fprintf(stderr,"DrawJustifiedText: Bad Justification\n");
		return;
	}
	XDrawString(XtDisplay(w),XtWindow(w),gc,base_x,base_y,str,strlen(str));
} /* End DrawJustifiedText */


/*---------------------------------------------------------------------------*

	static char *ShortenFloat(from,to)

	This routine takes a floating point number string in <from>
	and writes out a new version of the string, with unneeded zeroes
	removed, into <to>.

 *---------------------------------------------------------------------------*/

static char *ShortenFloat(from,to)
register char *from,*to;
{
	register char *end,*p,*new_from;

	new_from = from;
	while (*new_from == '0') ++new_from;

	end = new_from + strlen(new_from) - 1;
	while ((end >= new_from) && (*end == '0')) --end;

		/* end now points to last non-zero character */

	p = to;
	while (new_from <= end) *(p++) = *(new_from++);

	if (*to == '\0')
	{
		*p++ = '0';
	}
	    else if (*end == '.' && end > new_from)
	{
		-- p;			/* Skip Final Decimal Point */
	}
	    else if (*end == '.')	/* If Just '.', Make it 0 */
	{
		*p++ = '0';
	}
	*p = '\0';

	return(to);
} /* End ShortenFloat */


/*---------------------------------------------------------------------------*

	static int SampleToBinNumber(w,sample,full_redraw_return)

	This routine takes a histogram widget <w> and a floating point
	sample <sample>, and returns the bin number corresponding to the
	sample.  If autobins is on and the sample is an outlier, the
	histogram bins are increased to fit the sample.  If some change
	to the histogram is done, full_redraw_return is set to true indicating
	that the histogram needs to be fully redrawn.  Otherwise, only the
	changing bar needs to be redrawn, so false is stored through
	full_redraw_return.

 *---------------------------------------------------------------------------*/

static int SampleToBinNumber(w,sample,full_redraw_return)
XfwfHistogramWidget w;
double sample;
int *full_redraw_return;
{
	int bin,first_bin,last_bin;

	bin = (sample - HistData(w)->origin) / HistData(w)->bin_width + 1;
	*full_redraw_return = FALSE;
	if (BinIsOutlier(w,bin))
	{
		if (HistOptions(w)->autobins)
		{
			first_bin = RoundDown(HistData(w)->smallest /
				HistData(w)->bin_width);
			last_bin = RoundDown(HistData(w)->largest /
				HistData(w)->bin_width);
			HistData(w)->origin = (float)first_bin *
				HistData(w)->bin_width;
			HistData(w)->bin_count = last_bin - first_bin + 1;
			bin = ((sample - HistData(w)->origin) /
				HistData(w)->bin_width) + 1;
			InitializeBins(w);
			*full_redraw_return = TRUE;
		}
		    else
		{
			if (bin <= BinLowOutlier(w))
				bin = BinLowOutlier(w);
			    else
				bin = BinHighOutlier(w);
		}
	}
	return(bin);
} /* End SampleToBinNumber */


/*---------------------------------------------------------------------------*

	static int CoordsToBinNumber(w,x,y,check_vertically)

	This routine takes a histogram widget <w> and <x> and <y> coordinates
	and determines which bin the click landed in.  If <check_vertically>
	is FALSE, only the <x> value is checked, so a click vertically
	outside the bin will still register the bar number.  -1 is returned
	if no bin is clicked on.

 *---------------------------------------------------------------------------*/

static int CoordsToBinNumber(w,x,y,check_vertically)
XfwfHistogramWidget w;
int x,y,check_vertically;
{
	int i,bx,by,bw,bh;

	for (i = HistData(w)->first_bin; i <= HistData(w)->last_bin; i++)
	{
		BinCoords(w,i,&bx,&by,&bw,&bh);
		if (x >= bx && x < bx + bw)
		{
			if (check_vertically)
			{
				if (y >= by && y < by + bh)
					return(i);
				    else
					return(-1);
			}
			    else
			{
				return(i);
			}
		}
	}
	return(-1);
} /* End CoordsToBinNumber */


/*---------------------------------------------------------------------------*

	static int AddSample(w,sample,update_flag,bin_ptr)

	This routine is used as the low level utility routine for adding
	samples to the histogram.  The actual redrawing of the histogram
	must be done by the caller, either using RedrawEntireWidget()
	or RedrawSingleBar().  The routine will update the stats and info
	lines if <update_flag> set to TRUE.  The bin number that the sample
	fell in is returned through <bin_ptr> and the function returns TRUE
	if the entire widget needs to be redrawn because a scaling took place.

	The normal way a user adds a sample to a widget is through the
	HistogramAddSample() and HistogramAddMultipleSamples() calls.

 *---------------------------------------------------------------------------*/

static int AddSample(w,sample,update_flag,bin_ptr)
XfwfHistogramWidget w;
double sample;
int update_flag;
int *bin_ptr;
{
	int full_redraw_required;
	DATA_POINTS *dp;

	full_redraw_required = FALSE;
	HistData(w)->smallest = min(HistData(w)->smallest,sample);
	HistData(w)->largest = max(HistData(w)->largest,sample);
	*bin_ptr = SampleToBinNumber(w,sample,&full_redraw_required);
	++ HistData(w)->bins[*bin_ptr];
	++ HistData(w)->N;

		/* Add Sample To Data Points, If Keeping Them Around */

	if (HistOptions(w)->keep_points)
	{
		dp = HistData(w)->data;
		DataPointsAddPoint(dp,sample);
	}

		/* Update Statistics */

	HistData(w)->sum_of_samples += sample;
	HistData(w)->sum_of_squared_samples += (sample * sample);
	if (HistOptions(w)->show_stats)
	{
		HistData(w)->variance =
			(HistData(w)->sum_of_squared_samples -
				(square(HistData(w)->sum_of_samples) /
					HistData(w)->N)) /
			(HistData(w)->N - 1);
		HistData(w)->mean =
			HistData(w)->sum_of_samples / HistData(w)->N;
		if (update_flag == TRUE)
		{
			RedrawStats(w,0,0,HistCore(w)->width,
				    HistCore(w)->height);
		}
	}

	if ((HistOptions(w)->show_outlier_bins == True) ||
	    (*bin_ptr >= HistData(w)->first_bin &&
	     *bin_ptr <= HistData(w)->last_bin))
	{
		if (HistData(w)->bins[*bin_ptr] >
		    HistData(w)->size_of_fullest_bin)
		{
			HistData(w)->size_of_fullest_bin =
				HistData(w)->bins[*bin_ptr];
			if (update_flag == TRUE)
			{
				RedrawBinInfo(w,0,0,HistCore(w)->width,
					      HistCore(w)->height);
			}
		}				
	}

		/* Code To Handle Rescaling */

	if ((HistData(w)->size_of_fullest_bin != 0) &&
	    (HistData(w)->size_of_fullest_bin >= HistData(w)->scale))
	{
		HistData(w)->scale = HistData(w)->scale *
			HistData(w)->scale_factor;
		full_redraw_required = TRUE;
	}

	return(full_redraw_required);
} /* End AddSample */


static void Click(w,event,params,num_params)
Widget w;
XEvent *event;
char *params;
Cardinal *num_params;
{
	int bin,count,x,y;
	XfwfHistogramReturnStruct callback_return_value;

	x = event->xbutton.x;
	y = event->xbutton.y;
	bin = CoordsToBinNumber((XfwfHistogramWidget)w,x,y,FALSE);
	if (bin == -1) return;
	count = XfwfHistogramGetBinCount((XfwfHistogramWidget)w,bin);

	callback_return_value.bin_number = bin;
	callback_return_value.count = count;

	XtCallCallbacks(w,XtNcallback,(caddr_t)&callback_return_value);
} /* End Click */

/*===========================================================================*

                       D A T A    S T R U C T U R E S

 *===========================================================================*/

/*---------------------------------------------------------------------------*

	static DATA_POINTS *DataPointsNew()

	This routine returns a pointer to a newly created DATA_POINTS
	data structure, used to keep track of the samples seen.

 *---------------------------------------------------------------------------*/

static DATA_POINTS *DataPointsNew()
{
	DATA_BLOCK *block;
	DATA_POINTS *points;

	points = (DATA_POINTS *)XtMalloc(sizeof(DATA_POINTS));
	block = DataBlockNew();
	DataPointsCount(points) = 0;
	DataPointsBlockList(points) = block;
	DataPointsLast(points) = block;
	return(points);
} /* End DataPointsNew */


/*---------------------------------------------------------------------------*

	static void DataPointsFree(dp)

	This routine frees the DATA_POINTS structure pointed to by <dp>
	including any DATA_BLOCK structures that are components of the
	DATA_POINTS structure.

 *---------------------------------------------------------------------------*/

static void DataPointsFree(dp)
DATA_POINTS *dp;
{
	DATA_BLOCK *db,*next;

	for (db = DataPointsBlockList(dp); db != NULL; db = next)
	{
		next = DataBlockNext(db);
		DataBlockFree(db);
	}
	free((char *)(dp));
} /* End DataPointsFree */


/*===========================================================================*

                      E X T E R N A L    M E T H O D S

 *===========================================================================*/

/*---------------------------------------------------------------------------*

	void XfwfHistogramDumpWidgetState(fp,w)

	This routine takes a histogram widget <w> and prints out parts of
	the widget state to the file pointer <fp>.

 *---------------------------------------------------------------------------*/

void XfwfHistogramDumpWidgetState(fp,w)
FILE *fp;
XfwfHistogramWidget w;
{
	fprintf(fp,"width = %d\n",HistCore(w)->width);
	fprintf(fp,"height = %d\n",HistCore(w)->height);
	fprintf(fp,"title = %d",HistAttribs(w)->title);
	if (HistAttribs(w)->title)
		fprintf(fp," (%s)\n",HistAttribs(w)->title);
	    else
		fprintf(fp,"\n");

	fprintf(fp,"bin_width = %f\n",HistData(w)->bin_width);
	fprintf(fp,"origin = %f\n",HistData(w)->origin);
	fprintf(fp,"bin_count = %d\n",HistData(w)->bin_count);
	fprintf(fp,"scale = %d\n",HistData(w)->scale);
	fprintf(fp,"total_bin_count = %d\n",HistData(w)->total_bin_count);
	fprintf(fp,"bars_across = %d\n",HistData(w)->bars_across);
	fprintf(fp,"first_bin = %d\n",HistData(w)->first_bin);
	fprintf(fp,"last_bin = %d\n",HistData(w)->last_bin);
	fprintf(fp,"plus_inf = %f\n",HistData(w)->plus_inf);
	fprintf(fp,"minus_inf = %f\n",HistData(w)->minus_inf);
	fprintf(fp,"smallest = %f\n",HistData(w)->smallest);
	fprintf(fp,"largest = %f\n",HistData(w)->largest);
	fprintf(fp,"bins = %d\n",HistData(w)->bins);
	fprintf(fp,"size_of_fullest_bin = %d\n",
		HistData(w)->size_of_fullest_bin);
	fprintf(fp,"N = %d\n",HistData(w)->N);
	fprintf(fp,"mean = %f\n",HistData(w)->mean);
	fprintf(fp,"variance = %f\n",HistData(w)->variance);

	fprintf(fp,"show_bin_info = %d\n",HistOptions(w)->show_bin_info);
	fprintf(fp,"show_x_axis = %d\n",HistOptions(w)->show_x_axis);
	fprintf(fp,"show_y_axis = %d\n",HistOptions(w)->show_y_axis);
	fprintf(fp,"show_axis_labels = %d\n",HistOptions(w)->show_axis_labels);
	fprintf(fp,"autoscale = %d\n",HistOptions(w)->autoscale);
	fprintf(fp,"autobins = %d\n",HistOptions(w)->autobins);
	fprintf(fp,"show_outlier_bins = %d\n",
		HistOptions(w)->show_outlier_bins);
	fprintf(fp,"discrete = %d\n",HistOptions(w)->discrete);
	fprintf(fp,"keep_points = %d\n",HistOptions(w)->keep_points);
	fprintf(fp,"show_stats = %d\n",HistOptions(w)->show_stats);
	fprintf(fp,"vertical_ticks = %d\n",HistOptions(w)->vertical_ticks);

	fprintf(fp,"bar_color = %d\n",HistX(w)->bar_color);
	fprintf(fp,"axis_color = %d\n",HistX(w)->axis_color);
	fprintf(fp,"text_color = %d\n",HistX(w)->text_color);
	fprintf(fp,"axis_label_color = %d\n",HistX(w)->axis_label_color);
	fprintf(fp,"text_font = %d\n",HistX(w)->text_font);
	fprintf(fp,"axis_font = %d\n",HistX(w)->axis_font);
	fprintf(fp,"gray_pixmap = %d\n",HistX(w)->gray_pixmap);
	fprintf(fp,"erase_color = %d\n",HistX(w)->erase_color);
	fprintf(fp,"erase_gc = %d\n",HistX(w)->erase_gc);
	fprintf(fp,"bar_gc = %d\n",HistX(w)->bar_gc);
	fprintf(fp,"axis_gc = %d\n",HistX(w)->axis_gc);
	fprintf(fp,"text_gc = %d\n",HistX(w)->text_gc);
	fprintf(fp,"axis_label_gc = %d\n",HistX(w)->axis_label_gc);
	fprintf(fp,"gray_gc = %d\n",HistX(w)->gray_gc);
	fprintf(fp,"too_small = %d\n",HistX(w)->too_small);
} /* End XfwfHistogramDumpWidgetState */


/*---------------------------------------------------------------------------*

	void XfwfHistogramAddSample(w,sample)

	This routine takes a histogram widget <w> and adds the sample
	point <sample> to the widget, redrawing it as necessary.

 *---------------------------------------------------------------------------*/

void XfwfHistogramAddSample(w,sample)
XfwfHistogramWidget w;
double sample;
{
	int full_redraw_required,bin;

	full_redraw_required = AddSample(w,sample,TRUE,&bin);
	if (full_redraw_required)
	{
		RecalcComponentCoordinates(w);
		RedrawEntireWidget(w);
	}
	    else
	{
		RedrawSingleBar(w,bin);
	}
} /* End XfwfHistogramAddSample */


/*---------------------------------------------------------------------------*

	void XfwfHistogramAddMultipleSamples(w,sample_count,sample_array)

	This routine takes a histogram widget <w> and adds <sample_count>
	samples from the array <sample_array> to the histogram widget <w>
	and then redraws the entire widget.  This is intended to be more
	efficient than redrawing the widget after adding each sample.

 *---------------------------------------------------------------------------*/

void XfwfHistogramAddMultipleSamples(w,sample_count,sample_array)
XfwfHistogramWidget w;
int sample_count;
float *sample_array;
{
	int i,dummy;

	for (i = 0; i < sample_count; i++)
		AddSample(w,sample_array[i],FALSE,&dummy);
	RecalcComponentCoordinates(w);
	RedrawEntireWidget(w);
} /* End XfwfHistogramAddMultipleSamples */


/*---------------------------------------------------------------------------*

	int XfwfHistogramGetBinCount(w,bin_num)

	This routine returns the count of samples withing the bin
	numbered <bin_num> of the histogram widget <w>.  -1 is returned
	on error.

 *---------------------------------------------------------------------------*/

int XfwfHistogramGetBinCount(w,bin_num)
XfwfHistogramWidget w;
int bin_num;
{
	if (bin_num < 0 || bin_num >= HistData(w)->total_bin_count) return(-1);
	return(HistData(w)->bins[bin_num]);
} /* End XfwfHistogramGetBinCount */


/*---------------------------------------------------------------------------*

	XfwfZeroBins(w)

	This routine reset Bins of the widget, then call InitializeBins
	specifying the default number of Bins.

	Added by Daniele Alberto Galliano (galliano@athena.polito.it)

 *---------------------------------------------------------------------------*/

void XfwfZeroBins(w)
XfwfHistogramWidget w;
{
	if (HistData(w)->data) DataPointsFree(HistData(w)->data);
	HistData(w)->data = NULL;

	XtFree((char *)(HistData(w)->bins));
	HistData(w)->bins = NULL;
	InitializeBins(w);
	RedrawEntireWidget(w);
} /* End XfwfZeroBins */
