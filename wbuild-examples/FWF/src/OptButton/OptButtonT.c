/* OptButtonT -- test program for OptionButton widget */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Board.h>
#include <Xfwf/OptButton.h>
#include <Xfwf/TextMenu.h>			/* For XfwfTextMenuData */

static String fallback_resources[] = {
    "*board.width: 400",
    "*board.height: 200",
    "*board*background: lightsteelblue",
    "*board*foreground: yellow",
    "*button.x: 10",
    "*button.y: 10",
    "*button.label: <none>",			/* Initial label */
    "*button.menu:\
first-choice\\n\
second-choice\\n\
third-choice\\n\
Quit",
    NULL,
};

static Widget toplevel, board, button;
static XtAppContext app_context;

static void do_something(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XfwfTextMenuData *data = (XfwfTextMenuData *) call_data;

    if (data->n == 3) exit(0);			/* "Quit" */
    printf("callback: do something with %u (\"%s\")\n", data->n, data->label);
}


int main(argc, argv)
    int argc;
    String *argv;
{
    toplevel = XtVaAppInitialize
	(&app_context, "OptButtonT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);
    board = XtVaCreateManagedWidget
	("board", xfwfBoardWidgetClass, toplevel, NULL);
    button = XtVaCreateManagedWidget
	("button", xfwfOptionButtonWidgetClass, board, NULL);
    XtAddCallback(button, XtNactivate, do_something, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
