/*****************************************************************************

	HistogramP.h

	This file contains is the private .h file for the
	histogram widget.

 *****************************************************************************/

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

#ifndef _XtHistogramP_h
#define _XtHistogramP_h

#include <X11/CoreP.h>
#include <Xfwf/Histogram.h>

/* New fields for the histogram widget instance record */

/*---------------------------------------------------------------------------*

      L O C A L    D A T A    S T R U C T U R E    D E F I N I T I O N S

 *---------------------------------------------------------------------------*/

#define	DATA_BLOCK_SIZE 32768

typedef struct data_block
{
	struct data_block *next_block;	/* Next Data Block In List */
	float data[DATA_BLOCK_SIZE];	/* Data Point Array */
	unsigned int bytes_used;	/* Number Of Bytes Used In Block */
} DATA_BLOCK;

typedef struct
{
	int count;			/* Number Of Data Blocks In List */
	DATA_BLOCK *block_list;		/* First In List Of Data Blocks */
	DATA_BLOCK *last_block;		/* Last Data Block */
} DATA_POINTS;

typedef struct
{
	float bin_width;		/* Width Of Each Bin */
	float origin;			/* Start Of First Bin */
	int bin_count;			/* Number Of Histogram Bins */
	int scale;			/* Vertical Scale */
	float scale_factor;		/* Auto Scaling Factor */

	int total_bin_count;		/* Count Of Bins Plus Outlier Bins */
	int bars_across;		/* Total Number Of Bars Across */
	int first_bin;			/* First Bin Number To Draw */
	int last_bin;			/* Last Bin Number To Draw */
	double plus_inf;		/* Largest Possible Number */
	double minus_inf;		/* Smallest Possible Number */
	float smallest;			/* Smallest Data Value */
	float largest;			/* Largest Data Value */
	int *bins;			/* Array Of Histogram Bins */
	int size_of_fullest_bin;	/* Size Of The Most Full Visible Bin */

	int N;				/* Total Number Of Samples So Far */
	float sum_of_samples;		/* Sum Of Samples So Far */
	float sum_of_squared_samples;	/* Sum Of Squared Samples So Far */
	float mean;			/* Current Mean Of Samples */
	float variance;			/* Current Variance Of Samples */
	DATA_POINTS *data;		/* Data Points */
} HISTOGRAM;

typedef struct
{
	Boolean show_bin_info;		/* Show Bin Info Line? */
	Boolean show_x_axis;		/* Show The X Axis & Ticks? */
	Boolean show_y_axis;		/* Show The Y Axis & Ticks? */
	Boolean show_axis_labels;	/* Show The Axis Labels? */
	Boolean autoscale;		/* Automatically Scale? */
	Boolean autobins;		/* Dynamically Add Bins? */
	Boolean show_outlier_bins;	/* Show Out Of Range Bins? */
	Boolean discrete;		/* Discrete Or Continuous? */
	Boolean keep_points;		/* Keep Data Points? */
	Boolean	show_stats;		/* Show Statistics? */
	int y_axis_tick_type;		/* How To Label Vertical Ticks */
	int vertical_ticks;		/* Number Of Vertical Ticks Wanted */
} OPTIONS;

typedef struct
{
	Pixel bar_color;		/* Color For Histogram Bar */
	Pixel axis_color;		/* Color For Axes */
	Pixel text_color;		/* Color For Text */
	Pixel axis_label_color;		/* Color For Axis Labels */
	XFontStruct *text_font;		/* Text Font */
	XFontStruct *axis_font;		/* Axis Label Font */

	Pixmap gray_pixmap;		/* Gray Stipple Pattern */
	Pixel erase_color;		/* Background Color */
	GC erase_gc;			/* Erasing GC */
	GC bar_gc;			/* Histogram Bar GC */
	GC axis_gc;			/* Axis Line GC */
	GC text_gc;			/* Text GC */
	GC axis_label_gc;		/* Axis Label GC */
	GC gray_gc;			/* Gray Stipple GC */
	Boolean too_small;		/* Widget Too Small To Draw? */
	XtCallbackList callbacks;	/* Callbacks For Bar Clicks */
} X_STUFF;

typedef struct
{
	int cx;				/* Center X Coordinate */
	int cy;				/* Center Y Coordinate */
	int title_y;			/* Y Position Of Title Line */
	int title_h;			/* Height Of Title Line */
	int bin_info_y;			/* Y Position Of Bin Info Line */
	int bin_info_h;			/* Height Of Bin Info Line */
	int stats_y;			/* Y Position Of Stats Line */
	int stats_h;			/* Height Of Stats Line */
	int bar_top_y;			/* Y Position Of Top Of Bars */
	int bar_bottom_y;		/* Y Position Of Bottom Of Bars */
	int bar_h;			/* Resulting Maximum Bar Height */
	int y_axis_x;			/* Y Axis X Coordinate */
	int x_axis_y;			/* X Axis Y Coordinate */
	int bar_left_x;			/* X Position Of Left Of Bars */
	int bar_right_x;		/* X Position Of Right Of Bars */
	int y_label_x;			/* Y Axis Labels X Coordinate */
	int y_label_w;			/* Y Axis Label Width */
	int y_tick_x;			/* Y Axis Tick Left X Coordinate */
	int x_label_y;			/* X Axis Labels Y Coordinate */
	int x_tick_y;			/* X Axis Tick Bottom Y Coordinate */
	int bar_w;			/* Width Of A Histogram Bar */
} COORDINATES;

typedef struct
{
	char *title;			/* Title */
} ATTRIBUTES;

/*---------------------------------------------------------------------------*

     W I D G E T    D A T A    S T R U C T U R E    D E F I N I T I O N S

 *---------------------------------------------------------------------------*/

	/* The Histogram Specific Part Of The Instance Record */

typedef struct
{
	HISTOGRAM histogram_data;	/* Actual Histogram Data */
	OPTIONS options;		/* Flags Indicating Options */
	X_STUFF x_stuff;		/* X Related Information */
	ATTRIBUTES attribs;		/* Widget Attributes, Etc. */
	COORDINATES coords;		/* Component Coordinates */
} XfwfHistogramPart;

	/* The Full Instance Record For A Histogram Widget */

typedef struct _XfwfHistogramRec
{
	CorePart core;
	XfwfHistogramPart histogram;
} XfwfHistogramRec;

	/* No Additional Class Data For A Histogram Widget */

typedef struct
{
	int dummy;
} XfwfHistogramClassPart;

	/* The Full Class Record For A Histogram Widget */

typedef struct _XfwfHistogramClassRec
{
	CoreClassPart core_class;
	XfwfHistogramClassPart histogram_class;
} XfwfHistogramClassRec;

	/* Declare The Pointer To The Class Record So Others Can Use It */

extern XfwfHistogramClassRec xfwfHistogramClassRec;

/*---------------------------------------------------------------------------*

          D A T A    S T R U C T U R E    A C C E S S    M A C R O S

 *---------------------------------------------------------------------------*/

#define	InstanceHist(w)		(&((w)->histogram))
#define	InstanceCore(w)		(&((w)->core))

#define	HistCore(w)		(InstanceCore(w))
#define	HistData(w)		(&(InstanceHist(w)->histogram_data))
#define	HistOptions(w)		(&(InstanceHist(w)->options))
#define	HistX(w)		(&(InstanceHist(w)->x_stuff))
#define	HistAttribs(w)		(&(InstanceHist(w)->attribs))
#define	HistCoords(w)		(&(InstanceHist(w)->coords))

/*---------------------------------------------------------------------------*

        O T H E R    M A C R O S    F O R    I N T E R N A L    U S E

 *---------------------------------------------------------------------------*/

#define	BinLowOutlier(w)	(0)
#define	BinHighOutlier(w)	(HistData(w)->bin_count + 1)
#define	BinIsOutlier(w,bin)	(((bin) <= BinLowOutlier(w)) ||		\
				 ((bin) >= BinHighOutlier(w)))

#define	RoundOut(f)		((f) < 0 ? - ceil(-(f)) : ceil(f))
#define	RoundIn(f)		(aint(f))
#define	RoundDown(f)		(floor(f))
#define	RoundUp(f)		(ceil(f))

#define	DataBlockNext(db)	((db)->next_block)
#define	DataBlockData(db)	((db)->data)
#define	DataBlockBytesUsed(db)	((db)->bytes_used)

#define	DataBlockNew()		((DATA_BLOCK *)XtMalloc(sizeof(DATA_BLOCK)))
#define	DataBlockFree(db)	XtFree((char *)(db))

#define	DataPointsCount(dp)	((dp)->count)
#define	DataPointsBlockList(dp)	((dp)->block_list)
#define	DataPointsLast(dp)	((dp)->last_block)

#define	DataPointsAddPoint(dp,p)					\
{									\
	DATA_BLOCK *block;						\
									\
	if (DataBlockBytesUsed(DataPointsLast(dp)) == DATA_BLOCK_SIZE)	\
	{								\
		DataBlockNext(DataPointsLast(dp)) = DataBlockNew();	\
		DataPointsLast(dp) = DataBlockNext(DataPointsLast(dp));	\
	}								\
	block = DataPointsLast(dp);					\
	DataBlockData(block)[DataBlockBytesUsed(block)++] = (p);	\
} /* End DataPointsAddPoint */
		
#endif

