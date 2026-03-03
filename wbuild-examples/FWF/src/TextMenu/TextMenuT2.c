#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/TextMenu.h>
#include <Xfwf/PullDown.h>
#include <Xfwf/MenuBar.h>

static String fallback_resources[] = {
    "TextMenuT.width: 250",
    "TextMenuT.height: 50",

    "*btn1.label: Press here",
#if 0
    "*btn1.menu: \
(item \"first item\" (action \"first_action()\"))\
(item \"second item\" (action \"second_action()\"))\
(item \"third item\" (action \"third_action()\"))\
(item \"fourth item\" (action \"fourth_action()\"))",
#endif
    "*btn2.label: Press here",

    "*menu1.menu: \
_Zeroth item\\n\
_First item\\n\
_Second item\\n\
_Third item\\n\
------------\\n\
E_xit",
    "*menu1.active: 27",			/* `Second' is inactive */

    "*menu2.menu: \
_Zeroth item\\n\
_First item\\n\
_Second item\\n\
_Third item\\n\
------------\\n\
E_xit",
    "*menu2.active: 30",			/* `Zeroth' is inactive */

    NULL,
};

static Widget toplevel, menu1, menu2, btn1, btn2, bar;
static XtAppContext app_context;


/* ARGSUSED */
static void activate(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    printf("\nYou selected %u\n", (Cardinal) call_data);
    if ((Cardinal) call_data == 4) exit(0);
}


/* ARGSUSED */
static void change(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    switch ((Cardinal) call_data) {
    case -1: printf("Outside menu\n"); break;
    case  0: printf("The zeroth item doesn't do anything\n"); break;
    case  1: printf("The first item is unassigned\n"); break;
    case  2: printf("The second item is inactive!\n"); break;
    case  3: printf("The third item does nothing\n"); break;
    case  4: printf("This item stops the program\n"); break;
    }
}


void main(argc, argv)
    int argc;
    String *argv;
{
    toplevel = XtVaAppInitialize
	(&app_context, "TextMenuT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);
    bar = XtVaCreateManagedWidget
	("bar", xfwfMenuBarWidgetClass, toplevel, NULL);
    menu1 = XtCreatePopupShell
	("menu1", xfwfTextMenuWidgetClass, bar, NULL, 0);
    menu2 = XtCreatePopupShell
	("menu1", xfwfTextMenuWidgetClass, toplevel, NULL, 0);
    btn1 = XtVaCreateManagedWidget
	("btn1", xfwfPullDownWidgetClass, bar, XtNpopup, menu1, NULL);
    btn2 = XtVaCreateManagedWidget
	("btn2", xfwfPullDownWidgetClass, bar, XtNpopup, menu2, NULL);
    XtAddCallback(menu1, XtNactivate, activate, NULL);
    XtAddCallback(menu1, XtNchange_selection, change, NULL);
    XtAddCallback(menu2, XtNactivate, activate, NULL);
    XtAddCallback(menu2, XtNchange_selection, change, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
