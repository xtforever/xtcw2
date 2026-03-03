/* Slider4T.c -- test program for Slider4 widget
 * Bert Bos <bert@let.rug.nl>
 */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <Xfwf/Board.h>
#include <Xfwf/Label.h>
#include <Xfwf/Slider4.h>
#include <Xfwf/Slider2.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static String fallback_resources[] = {
    "Slider4T.board.width: 650",
    "Slider4T.board.height: 350",
    "Slider4T*background: aquamarine",
    "*font: *helvetica-bold-r-*-120-*",
    "*borderWidth: 0",
    "*frameWidth: 2",
    "*thumbFrameWidth: 2",
    "*minsize: 30",
    "*background: SeaGreen",

    "*board.accelerators: <Key>q: quit()",
    "*board.outerOffset: 12",

    "*ONE.sensitive: False",
    "*ONE.label: slider\nONE",
    "*ONE.foreground: white",
    "*ONE.alignment: top left",
    "*ONE.location: 10 10 1.0-130 1.0-20",
    "*ONE.thumbColor: BlueViolet",

    "*TWO.label: 2",
    "*TWO.foreground: white",
    "*TWO.location: 1.0-110 1.0-100 100 90",
    "*TWO.thumbColor: Firebrick",
    "*TWO.sashColor: DarkOrange",

    "*label.location: 1.0-110 10 100 1.0-120",
    "*label.alignment: top",
    "*label.frameWidth: 4",
    "*label.frameType: ledged",

    NULL,
};
static Widget toplevel, board, tester1, tester2, label;
static XtAppContext app_context;

/*
 * set_label -- display slider info in the label widget
 */
static void set_label(info)
    XfwfScrollInfo *info;
{
    static String lbl = NULL;
    static float x = 0.0, y = 0.0, w = 0.0, h = 0.0;
    String s;

    /* Update the values */
    if (info->flags & XFWF_VPOS) y = info->vpos;
    if (info->flags & XFWF_HPOS) x = info->hpos;
    if (info->flags & XFWF_VSIZE) h = info->vsize;
    if (info->flags & XFWF_HSIZE) w = info->hsize;

    /* Create a new label */
    s = XtMalloc(60);
    (void) sprintf(s, "x = %.2f\ny = %.2f\nwidth = %.2f\nheight = %.2f",
		   x, y, w, h);
    XtVaSetValues(label, XtNlabel, s, NULL);

    /* Free the old label */
    XtFree(lbl);
    lbl = s;
}

/*
 * cb -- callback: the small slider has been moved or resized
 */
static void cb(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XfwfScrollInfo *info = (XfwfScrollInfo*) call_data;

    if (info->reason != XfwfSDrag		/* Wait until end of move */
	&& info->reason != XfwfSStretch)	/* Wait until end of zoom */
	set_label(info);
}

/*
 * Action function
 */
static void quit(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    exit(0);
}

/*
 *  Main program
 */
int main(argc, argv)
    int argc;
    String *argv;
{
    static XtActionsRec actions[] = { {"quit", quit} };
    XfwfScrollInfo info;

    printf("Press q to quit\n");

    toplevel = XtVaAppInitialize
	(&app_context, "Slider4T", NULL, 0, &argc, argv,
	 fallback_resources, NULL);

    XtAppAddActions(app_context, actions, XtNumber(actions));

    board = XtVaCreateManagedWidget
	("board", xfwfBoardWidgetClass, toplevel, NULL);
    label = XtVaCreateManagedWidget
	("label", xfwfLabelWidgetClass, board, NULL);
    tester1 = XtVaCreateManagedWidget
	("ONE", xfwfSlider2WidgetClass, board, NULL);
    tester2 = XtVaCreateManagedWidget
	("TWO", xfwfSlider4WidgetClass, board, NULL);

    /*
     * Connect the sliders to each other according to the Free Widget
     * Foundation "Scrolling Widgets Interface Policy" (Draft
     * Proposal, Third Revision)
     */
    XfwfConnectScrollingWidgets(tester1, tester2);

    /*
     * Initialize them via convenience routine
     */
    info.reason = XfwfSNotify;
    info.flags = XFWF_VPOS | XFWF_HPOS | XFWF_VSIZE | XFWF_HSIZE;
    info.hpos = info.vpos = 0.0; 
    info.hsize = info.vsize = 0.25;
    XfwfResizeThumb(tester1, info.hsize, info.vsize);
    XfwfResizeThumb(tester2, info.hsize, info.vsize);

    /*
     * Initialize label and add a callback to the smaller slider so
     * that we can update the label when needed.
     */
    set_label(&info);
    XtAddCallback(tester2, XtNscrollCallback, cb, NULL);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
