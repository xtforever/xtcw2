#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/VScrollb.h>
#include <Xfwf/HScrollb.h>
#include <Xfwf/Slider2.h>
#include <Xfwf/Board.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static String fallback_resources[] = {
    "*board.width: 300",			/* Overall size */
    "*board.height: 250",

    "*board*borderWidth: 0",			/* Global */
    "*board*background: aquamarine",

    "*board.accelerators: <Key>q: quit()",

    "*scroll1.location: 1.0-24 4 20 1.0-32",	/* At right edge */
    "*scroll1.scrollbarForeground: violetred",
    "*scroll1._slider.cursor: sb_v_double_arrow",

    "*scroll2.location: 4 1.0-24 1.0-32 20",	/* Along bottom */
    "*scroll2.scrollbarForeground: orange",
    "*scroll2._slider.cursor: sb_h_double_arrow",

    "*slider.location: 4 4 1.0-32 1.0-32",
    "*slider.frameWidth: 2",
    "*slider.thumbColor: pink",

    NULL,
};

static Widget toplevel, scroll1, scroll2, slider, board;
static XtAppContext app_context;


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

    printf("\n\tTest for the horizontal & vertical scrollbars.\n");
    printf("\tNote that the slider is also a kind of scrollbar;\n");
    printf("\ttherefore all three widgets are connected by the\n");
    printf("\t\"SWIP\" (Scrolling Widgets Interface Protocol).\n\n");
    printf("\tPress \"q\" to quit.\n\n");

    toplevel = XtVaAppInitialize
	(&app_context, "ScrollbarT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);

    XtAppAddActions(app_context, actions, XtNumber(actions));

    board = XtVaCreateManagedWidget
	("board", xfwfBoardWidgetClass, toplevel, NULL);
    slider = XtVaCreateManagedWidget
	("slider", xfwfSlider2WidgetClass, board, NULL);
    scroll1 = XtVaCreateManagedWidget
	("scroll1", xfwfVScrollbarWidgetClass, board, NULL);
    scroll2 = XtVaCreateManagedWidget
	("scroll2", xfwfHScrollbarWidgetClass, board, NULL);

    /*
     * All descendants of XfwfBoard automatically install
     * accelerators, so the following is usually not needed:
     *
     * XtInstallAllAccelerators(toplevel, toplevel);
     */

    /* Hook them up to SWIP */
    XfwfConnectScrollingWidgets(slider, scroll1);
    XfwfConnectScrollingWidgets(slider, scroll2);

    /* Initial sizes */
    XfwfResizeThumb(slider, 0.2, 0.2);
    XfwfSetScrollbar(scroll1, 0.0, 0.2);
    XfwfSetScrollbar(scroll2, 0.0, 0.2);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
