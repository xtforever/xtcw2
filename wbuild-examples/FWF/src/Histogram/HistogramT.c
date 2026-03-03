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

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>

#include <Xfwf/Histogram.h>

#if defined(SYSV) || defined(SVR4)
#define random		rand
#endif
 
extern void exit();

	/* Command line options table */

static XrmOptionDescRec options[] =
{
    {"-notitle",        "*histogram.showTitle",      XrmoptionNoArg,"False"},
    {"-showbininfo",    "*histogram.showBinInfo",    XrmoptionNoArg,"True"},
    {"-noxaxis",        "*histogram.showXAxis",      XrmoptionNoArg,"False"},
    {"-noyaxis",        "*histogram.showYAxis",      XrmoptionNoArg,"False"},
    {"-autoscale",      "*histogram.autoscale",      XrmoptionNoArg,"True"},
    {"-noautoscale",    "*histogram.autoscale",      XrmoptionNoArg,"False"},
    {"-autobins",       "*histogram.autobins",       XrmoptionNoArg,"True"},
    {"-showoutliers",   "*histogram.showOutliers",   XrmoptionNoArg,"True"},
    {"-keeppoints",     "*histogram.keepPoints",     XrmoptionNoArg,"True"},
    {"-showstats",      "*histogram.showStats",      XrmoptionNoArg,"True"},

    {"-maxbinsize",     "*histogram.maxBinSize",     XrmoptionSepArg,NULL},
    {"-binwidth",       "*histogram.binWidth",       XrmoptionSepArg,NULL},
    {"-origin",         "*histogram.origin",         XrmoptionSepArg,NULL},
    {"-bincount",       "*histogram.binCount",       XrmoptionSepArg,NULL},
    {"-backingstore",   "*histogram.backingStore",   XrmoptionSepArg,NULL},
    {"-bar",            "*histogram.bar",            XrmoptionSepArg,NULL},
    {"-axis",           "*histogram.axis",           XrmoptionSepArg,NULL},
    {"-text",           "*histogram.text",           XrmoptionSepArg,NULL},
    {"-yaxisticktype",  "*histogram.yAxisTickType",  XrmoptionSepArg,NULL},
    {"-axislabel",      "*histogram.axisLabel",      XrmoptionSepArg,NULL},
    {"-textfont",       "*histogram.textFont",       XrmoptionSepArg,NULL},
    {"-axisfont",       "*histogram.axisFont",       XrmoptionSepArg,NULL},
    {"-title",          "*histogram.title",          XrmoptionSepArg,NULL},
};

/*-----------------------------------------------------------*

	main(argc,argv)

	This is the main entry point of the fileSelector
	widget application program.

 *-----------------------------------------------------------*/

float From0To6()
{
	return(((random() & 255) * 6.0) / 255);
} /* End From0To6 */


void CallbackHandler(w,client_data,call_data)
Widget w;
caddr_t client_data;
XfwfHistogramReturnStruct *call_data;
{
	printf("Bin %d:\tCount %d\n",call_data->bin_number,call_data->count);
}

main(argc,argv)
int argc;
char **argv;
{
	Widget top,hw;
	XEvent event;
	Arg args[4];
	float value;
	union {float f; int i;} float_arg;	/* XtSetArg has float bug */

	top = XtInitialize("main","XHistogram",
		options,XtNumber(options),&argc,argv);

#if 1
	float_arg.f = 0.0;
        XtSetArg(args[0],XtNorigin,float_arg.i);	/* Left Bin = 0 */

	float_arg.f = 0.50;
        XtSetArg(args[1],XtNbinWidth,float_arg.i);	/* Bin Width = .5*/

        XtSetArg(args[2],XtNbinCount,20);		/* 20 Bins ([0..10]) */

	float_arg.f = 10.0;
        XtSetArg(args[3],XtNscaleFactor,float_arg.i);	/* Scale Y by 10s */
	


	hw = XtCreateManagedWidget("histogram",
		xfwfHistogramWidgetClass,top,args,4);

#else
	hw = XtVaCreateManagedWidget
	    ("histogram", xfwfHistogramWidgetClass, top,
	     XtVaTypedArg, XtNorigin, XtRString, "0.0", 4,
	     XtVaTypedArg, XtNbinWidth, XtRString, "0.5", 4,
	     XtNbinCount, 20,
	     XtVaTypedArg, XtNscaleFactor, XtRString, "10.0", 5,
	     NULL);
#endif

	XtAddCallback(hw,XtNcallback,(XtCallbackProc)CallbackHandler,NULL);

	XtRealizeWidget(top);

	while (1)
	{
		while (XtPending())
		{
			XtNextEvent(&event);
			XtDispatchEvent(&event);
		}
		value = ((From0To6() + From0To6() + From0To6()) - 9.0) * 5.0 /
			9.0 + 5.0;
		XfwfHistogramAddSample((XfwfHistogramWidget)hw,value);
	}
} /* End main */
