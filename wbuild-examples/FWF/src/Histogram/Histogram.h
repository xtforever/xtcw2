/*****************************************************************************

	Histogram.h

	This file contains is the public .h file for the
	histogram widget.

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

#ifndef _XtHistogram_h
#define _XtHistogram_h

#include <math.h>

/*---------------------------------------------------------------------------*

               R E S O U R C E    N A M E    D E F I N I T I O N S

 *---------------------------------------------------------------------------*/

#define	XtNmaxBinSize		"maxBinSize"
#define	XtNbinWidth		"binWidth"
#define	XtNorigin		"origin"
#define	XtNbinCount		"binCount"
#define	XtNscale		"scale"
#define	XtNscaleFactor		"scale_factor"
#define	XtNshowTitle		"showTitle"
#define	XtNshowBinInfo		"showBinInfo"
#define	XtNshowXAxis		"showXAxis"
#define	XtNshowYAxis		"showYAxis"
#define	XtNshowAxisLabels	"showAxisLabels"
#define	XtNautoscale		"autoscale"
#define	XtNautobins		"autobins"
#define	XtNshowOutliers		"showOutliers"
#define	XtNdiscrete		"discrete"
#define	XtNshowStats		"showStats"
#define	XtNyAxisTickType	"yAxisTickType"
#define	XtNverticalTicks	"verticalTicks"
#define	XtNbar			"bar"
#define	XtNaxis			"axis"
#define	XtNtext			"text"
#define	XtNaxisLabel		"axisLabel"
#define	XtNtextFont		"textFont"
#define	XtNaxisFont		"axisFont"

#ifndef XtNtitle
#define	XtNtitle		"title"
#endif

#define	Y_AXIS_LABEL_BIN_COUNT		0
#define	Y_AXIS_LABEL_BIN_FRACTION	1

/*---------------------------------------------------------------------------*

              W I D G E T    &    C L A S S    D E F I N I T I O N S

 *---------------------------------------------------------------------------*/

typedef struct _XfwfHistogramRec	*XfwfHistogramWidget;
typedef struct _XfwfHistogramClassRec	*XfwfHistogramWidgetClass;

extern WidgetClass			xfwfHistogramWidgetClass;

/*---------------------------------------------------------------------------*

             C A L L B A C K    R E T U R N    S T R U C T U R E

 *---------------------------------------------------------------------------*/

typedef struct _XfwfHistogramReturnStruct
{
	int bin_number;
	int count;
} XfwfHistogramReturnStruct;

/*---------------------------------------------------------------------------*

               U S E R    C A L L A B L E    F U N C T I O N S

 *---------------------------------------------------------------------------*/

#if (!NeedFunctionPrototypes)

void		XfwfHistogramDumpWidgetState();
void		XfwfHistogramAddSample();
void		XfwfHistogramAddMultipleSamples();
int		XfwfHistogramGetBinCount();

#else

void		XfwfHistogramDumpWidgetState(FILE *fp, XfwfHistogramWidget w);
void		XfwfHistogramAddSample(XfwfHistogramWidget w, double sample);
void		XfwfHistogramAddMultipleSamples(XfwfHistogramWidget w,
			int sample_count, float *sample_array);
int		XfwfHistogramGetBinCount(XfwfHistogramWidget w, int bin_num);

#endif

#endif
