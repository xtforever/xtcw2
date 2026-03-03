/*  Include files  */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Board.h>
#include <Xfwf/Toggle.h>
#include <Xfwf/Button.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static String fallback_resources[] = {
    "*board.location: 0 0 400 400",		/* Overall size */
    "*board*background: aquamarine",		/* Global resources */
    "*board*foreground: yellow",
    "*board*borderWidth: 0",
    "*board*frameWidth: 2",
    "*board*font: *-helvetica-bold-r-*-140-*",

    "*board.outerOffset: 15",			/* Nice wide border */

    "*1.location: 40 40 1.0-80 40",
    "*1.innerOffset: 3",			/* Move the icon inwards */
    "*1.label: Quit button active",

    "*2.location: 40 120 1.0-80 40",
    "*2.innerOffset: 3",			/* Move the icon inwards */
    "*2.label: Toggle 2",

    "*quit.label: Quit",
    "*quit.location: 40 200 1.0-80 40",
    "*quit.cursor: pirate",

    NULL,
};

static Widget toplevel, board, button1, button2, quitbutton;
static XtAppContext app_context;

/*
 *  Callback
 */
static void on(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    if (w == button1) {
	XtVaSetValues(button2, XtNon, False, NULL);
	XtVaSetValues(quitbutton, XtNsensitive, True, NULL);
    } else {
	XtVaSetValues(button1, XtNon, False, NULL);
	XtVaSetValues(quitbutton, XtNsensitive, False, NULL);
    }
}

static void off(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    if (w == button1)
	XtVaSetValues(quitbutton, XtNsensitive, False, NULL);
}

static void quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
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
    toplevel = XtVaAppInitialize(&app_context, "ToggleT", NULL, 0,
				 &argc, argv, fallback_resources, NULL);
    board = XtVaCreateManagedWidget("board", xfwfBoardWidgetClass, toplevel,
				    NULL);
    button1 = XtVaCreateManagedWidget("1", xfwfToggleWidgetClass, board,
				      NULL);
    button2 = XtVaCreateManagedWidget("2", xfwfToggleWidgetClass, board,
				      XtNon, True,
				      NULL);
    quitbutton = XtVaCreateManagedWidget("quit", xfwfButtonWidgetClass, board,
					 XtNsensitive, False,
					 NULL);
    XtRealizeWidget(toplevel);
    XtAddCallback(button1, XtNonCallback, on, NULL);
    XtAddCallback(button1, XtNoffCallback, off, NULL);
    XtAddCallback(button2, XtNonCallback, on, NULL);
    XtAddCallback(quitbutton, XtNactivate, quit, NULL);
    XtAppMainLoop(app_context);
}
