/* Slider2T.c -- test program for Slider2 widget
 * Bert Bos <bert@let.rug.nl>
 * $Header: Slider2T.c,v 1.1 92/08/25 11:05:29 bert Exp $
 */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <Xfwf/Board.h>
#include <Xfwf/Label.h>
#include <Xfwf/Slider2.h>
#include <Xfwf/scroll.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static String fallback_resources[] = {
    "Slider2T.board.width: 650",
    "Slider2T.board.height: 350",

    "Slider2T*background: aquamarine",
    "*borderWidth: 0",
    "*frameWidth: 2",
    "*thumbFrameWidth: 2",
    "*minsize: 30",
    "*board.outerOffset: 12",

    "*board.accelerators: <Key>q: quit()",

    "*ONE.sensitive: False",
    "*ONE.location: 10 10 1.0-130 1.0-20",
    "*ONE.label: slider\nONE",
    "*ONE.foreground: white",
    "*ONE.alignment: top left",
    "*ONE.thumbColor: VioletRed",

    "*TWO.location: 1.0-110 1.0-100 100 90",
    "*TWO.label: 2",
    "*TWO.foreground: white",
    "*TWO.location: 1.0-110 1.0-100 100 90",
    "*TWO.thumbColor: Orange",

    "*label.location: 1.0-110 10 100 1.0-120",
    "*label.alignment: top",
    "*label.foreground: yellow",
    "*label.frameWidth: 4",
    "*label.frameType: ledged",

    NULL,
};
static Widget toplevel, board, tester1, tester2, label;
static XtAppContext app_context;
static XtCallbackProc scrollResp1, scrollResp2;

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
    (void) sprintf(s, "x = %.2f\ny = %.2f\nw = %.2f\nh = %.2f", x, y, w, h);
    XtVaSetValues(label, XtNlabel, s, NULL);

    /* Free the old label */
    XtFree(lbl);
    lbl = s;
}

/*
 * cb -- callback: the small slider has been moved
 */
static void cb(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XfwfScrollInfo *info = (XfwfScrollInfo*) call_data;

    if (info->reason != XfwfSDrag)		/* Don't update during move */
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

    toplevel = XtVaAppInitialize(&app_context, "Slider2T", NULL, 0,
				  &argc, argv, fallback_resources, NULL);

    XtAppAddActions(app_context, actions, XtNumber(actions));

    board = XtVaCreateManagedWidget("board", xfwfBoardWidgetClass, toplevel,
				    NULL);
    label = XtVaCreateManagedWidget("label", xfwfLabelWidgetClass, board,
				      NULL);
    tester1 = XtVaCreateManagedWidget("ONE", xfwfSlider2WidgetClass, board,
				      NULL);
    tester2 = XtVaCreateManagedWidget("TWO", xfwfSlider2WidgetClass, board,
				      NULL);

    XfwfConnectScrollingWidgets(tester1, tester2);

    XtVaGetValues(tester1, XtNscrollResponse, &scrollResp1, NULL);
    XtVaGetValues(tester2, XtNscrollResponse, &scrollResp2, NULL);

    info.flags = XFWF_VPOS | XFWF_HPOS | XFWF_VSIZE | XFWF_HSIZE;
    info.vpos = info.hpos = 0.0; info.vsize = info.hsize = 0.25;
    XfwfResizeThumb(tester1, info.hsize, info.vsize);
    XfwfResizeThumb(tester2, info.hsize, info.vsize);
    set_label(&info);
    XtAddCallback(tester2, XtNscrollCallback, cb, NULL);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
