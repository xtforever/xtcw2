/* MenuBarT -- test program for the MenuBar widget */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Board.h>
#include <Xfwf/Label.h>
#include <Xfwf/MenuBar.h>
#include <Xfwf/PullDown.h>
#include <Xfwf/OptButton.h>
#include <Xfwf/TextMenu.h>
#include <Xfwf/RowCol.h>

static String fallback_resources[] = {
    "MenuBarT.board.width: 450",
    "MenuBarT.board.height: 250",

    "MenuBarT*background: aquamarine",

    /* "*board.frameWidth: 0", */
    /* "*board.frameType: sunken", */
    "*board.highlightThickness: 0",

    "*menubar.location: 0 0 1.0 35",
    "*menubar.frameWidth: 2",
    "*menubar.frameType: raised",
    "*menubar.highlightThickness: 0",
    "*menubar.traversalOn: true",
    "MenuBarT*menubar*background: slateblue",
    "MenuBarT*menubar*foreground: white",
    "MenuBarT*menubar*font: *-helvetica-bold-r-*-140-*",

    "MenuBarT*XfwfTextMenu*background: slateblue",
    "MenuBarT*XfwfTextMenu*foreground: white",
    "MenuBarT*XfwfTextMenu*font: *-helvetica-bold-r-*-140-*",

    "*menu1.label: menu 1",
    "*menu1.hotkey: <Ctrl>1",
#if 1
    "*menu1.menu:\
	_First item  [Ctrl<Key>f]\\n\
	_Second item  [Ctrl<Key>s]\\n\
	-----------\\n\
	E_xit",
#else
        "*menu1.menu:\
	_First item\\n\
	_Second item\\n\
	-----------\\n\
	E_xit",
#endif
    
    "*menu2.label: menu 2",
    "*menu2.hotkey: <Ctrl>2",
    "*menu2.menu:\
_Third item\\n\
_Fourth item",

    "*opt.label: <none>",
    "*opt.menu:\
Sunday\\n\
Monday\\n\
Tuesday\\n\
Wednesday\\n\
Thursday\\n\
Friday\\n\
Saturday",

    "*Menu6.label: menu6",
#if 0
    "*menu6.menu:\
Nothing\\n\
Nothing",
#else
    "*rowcol6.shrinkToFit: true",
    "*button6_1.accelerators: Ctrl<KeyPress>a: activate() MenuPopdown(\"drop6\")",
    "*button6_1.label: aaaa",
    "*button6_1.shrinkToFit: true",
    "*button6_2.accelerators: Ctrl<KeyPress>b: activate() MenuPopdown(\"drop6\")",
    "*button6_2.label: bbbb",
    "*button6_2.shrinkToFit: true",
#endif
    
    "*explain.location: 0 35 1.0 1.0-80",
    "*explain.outerOffset: 5",
    "*explain.alignment: top left",
    "*explain.frameWidth: 2",
    "*explain.frameType: sunken",
    /* "*explain.highlightThickness: 0", */
    "*explain.label: \
This program shows a menubar with three\\n\
pull-down menus. The third menu is actually\\n\
an option button, i.e., it shows the most\\n\
recently chosen item as its label. None of the\\n\
entries does anything, except for `Exit',\\n\
which stops the program.",

    "*status.location: 0 1.0-40 1.0 40",
    "*status.outerOffset: 5",
    "*status.alignment: left",
    "*status.frameWidth: 2",
    "*status.frameType: sunken",

    NULL,
};

static Widget toplevel, board, menubar, menu1, menu2, opt, explain, status;
static Widget menu6, drop6, rowcol6, button6_1, button6_2;
static XtAppContext app_context;



static void activate1(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    char s[100];
    XfwfTextMenuData *data = (XfwfTextMenuData *) call_data;

    sprintf(s, "You selected %u: %s", data->n, data->label);
    XtVaSetValues(status, XtNlabel, s, NULL);
    if (data->n == 2) exit(0);
}



static void activate2(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    char s[100];
    XfwfTextMenuData *data = (XfwfTextMenuData *) call_data;

    sprintf(s, "You selected %u: %s", data->n, data->label);
    XtVaSetValues(status, XtNlabel, s, NULL);
}



static void activate6(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    char s[100], *label;

    XtVaGetValues(w, XtNlabel, &label, NULL);
    sprintf(s, "You selected %s", label);
    XtVaSetValues(status, XtNlabel, s, NULL);
}



static void change1(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XfwfTextMenuData *data = (XfwfTextMenuData *) call_data;
    static String s[] = {
	"",					/* -1 */
	"`First' doesn't do anything",		/* 0 */
	"`Second' is unassigned",		/* 1 */
	"`Exit' stops the program"		/* 2 */
	};

    XtVaSetValues(status, XtNlabel, s[data->n + 1], NULL);
}


static void change2(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XfwfTextMenuData *data = (XfwfTextMenuData *) call_data;
    static String s[] = {
	"",					/* -1 */
	"`Third' is a no-op",			/* 0 */
	"`Fourth' does nothing"			/* 1 */
	};

    XtVaSetValues(status, XtNlabel, s[data->n + 1], NULL);
}


int main(argc, argv)
    int argc;
    String *argv;
{
    toplevel = XtVaAppInitialize
	(&app_context, "MenuBarT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);

    board = XtVaCreateManagedWidget
	("board", xfwfBoardWidgetClass, toplevel, NULL);
    menubar = XtVaCreateManagedWidget
	("menubar", xfwfMenuBarWidgetClass, board, NULL);
    menu1 = XtVaCreateManagedWidget
	("menu1", xfwfPullDownWidgetClass, menubar, NULL);
    menu2 = XtVaCreateManagedWidget
	("menu2", xfwfPullDownWidgetClass, menubar, NULL);
    opt = XtVaCreateManagedWidget
	("opt", xfwfOptionButtonWidgetClass, menubar, NULL);

    /* Create a menu shell for PullDown button 6 */
    drop6 = XtVaCreatePopupShell
	("drop6", overrideShellWidgetClass, toplevel, NULL);
    rowcol6 = XtVaCreateManagedWidget
	("rowcol6", xfwfRowColWidgetClass, drop6, XtNcolumns, 1, NULL);
    button6_1 = XtVaCreateManagedWidget
	("button6_1", xfwfButtonWidgetClass, rowcol6, NULL);
    button6_2 = XtVaCreateManagedWidget
	("button6_2", xfwfButtonWidgetClass, rowcol6, NULL);

    /* Create the PullDown button that pops up the menu */
    menu6 = XtVaCreateManagedWidget
	("menu6", xfwfPullDownWidgetClass, menubar, XtNpopup, drop6, NULL);

    explain = XtVaCreateManagedWidget
	("explain", xfwfLabelWidgetClass, board, NULL);
    status = XtVaCreateManagedWidget
	("status", xfwfLabelWidgetClass, board, NULL);
    XtAddCallback(menu1, XtNactivate, activate1, NULL);
    XtAddCallback(menu2, XtNactivate, activate2, NULL);
    XtAddCallback(menu1, XtNchangeSelection, change1, NULL);
    XtAddCallback(menu2, XtNchangeSelection, change2, NULL);
    XtAddCallback(button6_1, XtNactivate, activate6, NULL);
    XtAddCallback(button6_2, XtNactivate, activate6, NULL);
    XtRealizeWidget(toplevel);
#if 1
    XtInstallAllAccelerators(toplevel, toplevel);
    XtInstallAllAccelerators(menubar, toplevel);
    XtInstallAllAccelerators(menu1, toplevel);
    XtInstallAllAccelerators(menu2, toplevel);
    XtInstallAllAccelerators(opt, toplevel);
    XtInstallAllAccelerators(menu6, toplevel);
#endif
    XtAppMainLoop(app_context);
}
