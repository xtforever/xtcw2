#define TEST2

#include <stdlib.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/ScrollWin.h>
#ifdef TEST1
#include <Xfwf/Histogram.h>
#else
#ifdef TEST2
#include <Xfwf/Label.h>
#else
#include <Xfwf/Shistogram.h>
#endif
#endif

static String fallback_resources[] = {
    "ScrollWinT.swindow.width: 500",
    "ScrollWinT.swindow.height: 500",
    "ScrollWinT*background: #5592aa",
    /* "ScrollWinT*background: cadetblue", */
    "ScrollWinT*hist.width: 900",
    "ScrollWinT*hist.height: 600",
    "ScrollWinT*hist.borderWidth: 0",
    "ScrollWinT*hist.bar: yellow",
    "ScrollWinT*hist.text: yellow",
    "ScrollWinT*hist.axisLabel: white",
    "ScrollWinT*hist.title: Random graph",
    "ScrollWinT*hist.autobins: True",
    "ScrollWinT*hist.showBinInfo: True",
    "ScrollWinT*hist.showStats: True",
    "ScrollWinT*hist.binWidth: 1.0",
    NULL,
};

static Widget toplevel, swindow, hist;
static XtAppContext app_context;

int main(argc, argv)
    int argc;
    String *argv;
{
    int i;

    toplevel = XtVaAppInitialize
	(&app_context, "ScrollWinT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);
    swindow = XtVaCreateManagedWidget
	("swindow", xfwfScrolledWindowWidgetClass, toplevel, NULL);
#ifdef TEST1
    hist = XtVaCreateManagedWidget
	("hist", xfwfHistogramWidgetClass, swindow, NULL);
#else
#ifdef TEST2
    hist = XtVaCreateManagedWidget
	("hist", xfwfLabelWidgetClass, swindow, XtNlabel,
	 "\
label label label\n\
label label\n\
label label label\n\
label label\n\
label label label\n\
label label\n\
label label label\n\
label label\n\
label label label\n\
label label",
	 NULL);
#else
    hist = XtVaCreateManagedWidget
	("hist", shistogramWidgetClass, swindow, NULL);
#endif
#endif
    XtRealizeWidget(toplevel);
#ifdef TEST1
    for (i = 0; i < 100; i++)
	XfwfHistogramAddSample((XfwfHistogramWidget) hist, rand() % 100);
#else
#ifdef TEST2
#else
    ShistogramSetData(hist, "\
hjdfhjhjd ui347845378r 7re yreyughhfhfuiruig fgug ur uyy25675906067iommn fdfdh\
HKDFHREUIY7845IYRT  KDBHSHJDSJKDG;GH;PTYIPOI\6700597345672  7836146`1986 GJHKK\
",
		      156);
#endif
#endif
    XtAppMainLoop(app_context);
}
