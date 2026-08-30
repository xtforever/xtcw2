#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/ThWheel.h>
#include <Xfwf/Slider2.h>
#include <Xfwf/Board.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static String fallback_resources[] = {
    "*board.width: 300",			/* Overall size */
    "*board.height: 300",

    "*board*borderWidth: 0",			/* Global */
    "*board*background: aquamarine",

    "*board.accelerators: <Key>q: quit()",

    "*scroll1.x: 275",
    "*scroll1.y: 20",
    /* The step and range are rather arbitrary in this program,
     * because the slider only looks at the vpos value, which is
     * always between 0 and 1
     */
    "*scroll1.maxValue: 800",			/* Seems to work well */
    "*scroll1.step: 4",				/* To allow 1/2 and 1/4 steps */

    "*scroll2.x: 20",
    "*scroll2.y: 275",
    "*scroll2.maxValue: 800",
    "*scroll2.step: 4",

    "*slider.location: 4 4 265 265",
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
void main(argc, argv)
    int argc;
    String *argv;
{
    static XtActionsRec actions[] = { {"quit", quit} };

    printf("\n\tTest for the thumb wheel.\n");
    printf("\tNote that the slider is also a kind of scrollbar;\n");
    printf("\ttherefore all three widgets are connected by the\n");
    printf("\t\"SWIP\" (Scrolling Widgets Interface Protocol).\n\n");
    printf("\tPress \"q\" to quit.\n\n");

    toplevel = XtVaAppInitialize
	(&app_context, "ScrollbarT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);

    /* XSynchronize(XtDisplay(toplevel), True); */

    XtAppAddActions(app_context, actions, XtNumber(actions));

    board = XtVaCreateManagedWidget
	("board", xfwfBoardWidgetClass, toplevel, NULL);
    slider = XtVaCreateManagedWidget
	("slider", xfwfSlider2WidgetClass, board, NULL);
    scroll1 = XtVaCreateManagedWidget
	("scroll1", xfwfThumbWheelWidgetClass, board, NULL);
    scroll2 = XtVaCreateManagedWidget
	("scroll2", xfwfThumbWheelWidgetClass, board,
	 XtNvertical, False, NULL);

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

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
