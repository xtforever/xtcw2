#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Scrollbar.h>
#include <Xfwf/Slider2.h>
#include <Xfwf/Board.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static String fallback_resources[] = {
    "ScrollbarT.board.width: 300",
    "ScrollbarT.board.height: 250",
    "ScrollbarT*borderWidth: 0",
    "ScrollbarT*background: gray80",

    "ScrollbarT*scroll1.location: 1.0-24 4 20 1.0-32",
    "ScrollbarT*scroll1.scrollbarForeground: red",

    "ScrollbarT*scroll2.location: 4 1.0-24 1.0-32 20",

    "ScrollbarT*slider.location: 4 4 1.0-32 1.0-32",
    "ScrollbarT*slider.frameWidth: 2",
    "ScrollbarT*slider.thumbColor: pink",
    NULL,
};

static Widget toplevel, scroll1, scroll2, slider, board;
static XtAppContext app_context;

/*
 *  Main program
 */
int main(argc, argv)
    int argc;
    String *argv;
{
    toplevel = XtVaAppInitialize(&app_context, "ScrollbarT", NULL, 0,
				  &argc, argv, fallback_resources, NULL);
    board = XtVaCreateManagedWidget("board", xfwfBoardWidgetClass, toplevel,
				    NULL);
    slider = XtVaCreateManagedWidget("slider", xfwfSlider2WidgetClass, board,
				     XtNsensitive, False, NULL);
    scroll1 = XtVaCreateManagedWidget("scroll1", xfwfScrollbarWidgetClass,
				      board, XtNvertical, True, NULL);
    scroll2 = XtVaCreateManagedWidget("scroll2", xfwfScrollbarWidgetClass,
				      board, XtNvertical, False, NULL);

    XfwfConnectScrollingWidgets(slider, scroll1);
    XfwfConnectScrollingWidgets(slider, scroll2);

    XfwfResizeThumb(slider, 0.2, 0.2);
    XfwfSetScrollbar(scroll1, 0.0, 0.2);
    XfwfSetScrollbar(scroll2, 0.0, 0.2);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
