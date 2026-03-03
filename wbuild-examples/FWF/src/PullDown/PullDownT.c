/* PullDownT -- test program for PullDown widget */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Board.h>
#include <Xfwf/RowCol.h>
#include <Xfwf/PullDown.h>
#include <Xfwf/TextMenu.h>

static String fallback_resources[] = {
    "PullDownT.board.width: 400",
    "PullDownT.board.height: 200",

    "PullDownT*background: gray80",

    "*board.frameWidth: 2",
    "*board.frameType: sunken",

    "*menubar.location: 0 0 1.0 40",
    "*menubar.frameWidth: 2",
    "*menubar.frameType: raised",

    "*XfwfPullDown.location: 0 0 80 30",	/* For both buttons */

    "*menu1.label: menu 1",
    "*menu1.hotkey: Alt<Key>1",
    "*menu1.menu: \
	_quit",

    "*menu2.label: menu 2",
    "*menu2.hotkey: Alt<Key>2",
    "*menu2.menu: \
	_first item\\n\
	s_econd item\\n\
	th_ird item\\n\
	fou_rth item",

    NULL,
};


/* ARGSUSED */
static void activate1(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XfwfTextMenuData *data = (XfwfTextMenuData *) call_data;

    printf("You selected %u: %s\n", data->n, data->label);
    exit(0);
}


/* ARGSUSED */
static void activate2(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XfwfTextMenuData *data = (XfwfTextMenuData *) call_data;

    printf("You selected %u: %s\n", data->n, data->label);
}


static Widget toplevel, board, menubar, menu1, menu2;
static XtAppContext app_context;


int main(argc, argv)
    int argc;
    String *argv;
{
    toplevel = XtVaAppInitialize
	(&app_context, "PullDownT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);
    /* (void) XSynchronize(display, True); */
    board = XtVaCreateManagedWidget
	("board", xfwfBoardWidgetClass, toplevel, NULL);
    menubar = XtVaCreateManagedWidget
	("menubar", xfwfRowColWidgetClass, board, NULL);
    menu1 = XtVaCreateManagedWidget
	("menu1", xfwfPullDownWidgetClass, menubar, NULL);
    menu2 = XtVaCreateManagedWidget
	("menu2", xfwfPullDownWidgetClass, menubar, NULL);
    /* XtInstallAllAccelerators(toplevel, toplevel); */
    XtAddCallback(menu1, XtNactivate, activate1, NULL);
    XtAddCallback(menu2, XtNactivate, activate2, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
