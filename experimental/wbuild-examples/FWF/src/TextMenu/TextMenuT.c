#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/TextMenu.h>
#include <Xfwf/Button.h>
#include <Xfwf/Board.h>

static String fallback_resources[] = {
    "TextMenuT.board.width: 300",
    "TextMenuT.board.height: 50",

    "*button1.label: Press",
    "*button1.location: 10 10 60 30",

    "*button2.label: Press",
    "*button2.location: 70 10 60 30",

    "*button3.label: Press",
    "*button3.location: 130 10 60 30",

    "*button4.label: Press",
    "*button4.location: 190 10 60 30",

    "*button5.label: Press",
    "*button5.location: 250 10 60 30",

    "*menu1.menu: \
_Zeroth item\\n\
Firs_t item\\n\
_Second item\\n\
_Third item\\n\
-----------\\n\
E_xit",
    "*menu1.active: 27",			/* 11011 = `Second' inactive */

    "*menu2.menu: \
_Zeroth item\\n\
_First item\\n\
Se_cond item\\n\
_Third item\\n\
-----------\\n\
E_xit",
    "*menu2.active: 30",			/* 11110 = `Zeroth' inactive */

#if 0
    "*menu3.menu: \
_Zeroth item\\n\
_First item\\n\
Se_cond item\\n\
_Third item\\n\
-----------\\n\
E_xit",
    "*menu3.active: 30",			/* 11110 = `Zeroth' inactive */

    "*menu4.menu: \
_Zeroth item\\n\
_First item\\n\
Se_cond item\\n\
_Third item\\n\
-----------\\n\
E_xit",
    "*menu4.active: 30",			/* 11110 = `Zeroth' inactive */
#endif

    "*menu5.menu: \
_Zeroth item\\n\
_First item\\n\
Se_cond item\\n\
_Third item\\n\
-----------\\n\
E_xit",
    "*menu5.active: 30",			/* 11110 = `Zeroth' inactive */

    NULL,
};

static Widget toplevel, menu1, menu2, button1, button2, board;
static Widget button3, button4, button5, menu3, menu4, menu5;
static XtAppContext app_context;


/* * activate is attached the activate callback of both menus */

/* ARGSUSED */
static void activate(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XfwfTextMenuData *data = (XfwfTextMenuData*) call_data;

    printf("You selected %u (\"%s\")\n", data->n, data->label);
    if (data->n == 4) exit(0);
}


/*
 * change is attached to the changeSelection callback of both menus
 */

/* ARGSUSED */
static void change(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    switch (((XfwfTextMenuData*) call_data)->n) {
    case -1: printf("  Outside menu\n"); break;
    case  0: printf("  The zeroth item doesn't do anything\n"); break;
    case  1: printf("  The first item is unassigned\n"); break;
    case  2: printf("  The second item is inactive\n"); break;
    case  3: printf("  The third item does nothing\n"); break;
    case  4: printf("  This item stops the program\n"); break;
    }
}


/* ARGSUSED */
static void place_menu(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    XButtonEvent *b = (XButtonEvent *) event;
    Widget menu;

    /* Open the menu with the mouse 10 pixels inside */
    switch (params[0][4]) {
    case '1': menu = menu1; break;
    case '2': menu = menu2; break;
    case '3': menu = menu3; break;
    case '4': menu = menu4; break;
    case '5': menu = menu5; break;
    }
    XtVaSetValues(menu, XtNx, b->x_root - 10, XtNy, b->y_root - 10, NULL);
}


static char trans1[] = "<BtnDown>: place_menu(menu1) XtMenuPopup(menu1)";
static char trans2[] = "<BtnDown>: place_menu(menu2) XtMenuPopup(menu2)";
static char trans3[] = "<BtnDown>: place_menu(menu3) XtMenuPopup(menu3)";
static char trans4[] = "<BtnDown>: place_menu(menu4) XtMenuPopup(menu4)";
static char trans5[] = "<BtnDown>: place_menu(menu5) XtMenuPopup(menu5)";

static XtActionsRec actions[] = {{"place_menu", place_menu}};


int main(argc, argv)
    int argc;
    String *argv;
{
    printf("Keyboard traversal is not working.\n");
    printf("Use pulldown buttons instead of normal buttons for that.\n");

    toplevel = XtVaAppInitialize
	(&app_context, "TextMenuT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);

    XtAppAddActions(app_context, actions, XtNumber(actions));

    board = XtVaCreateManagedWidget
	("board", xfwfBoardWidgetClass, toplevel, NULL);
    menu1 = XtCreatePopupShell
	("menu1", xfwfTextMenuWidgetClass, toplevel, NULL, 0);
    menu2 = XtCreatePopupShell
	("menu2", xfwfTextMenuWidgetClass, toplevel, NULL, 0);
    menu3 = XtCreatePopupShell
	("menu3", xfwfTextMenuWidgetClass, toplevel, NULL, 0);
    menu4 = XtCreatePopupShell
	("menu4", xfwfTextMenuWidgetClass, toplevel, NULL, 0);
    menu5 = XtCreatePopupShell
	("menu5", xfwfTextMenuWidgetClass, toplevel, NULL, 0);
    button1 = XtVaCreateManagedWidget
	("button1", xfwfButtonWidgetClass, board, 
	 XtVaTypedArg, XtNtranslations, XtRString, trans1, sizeof(trans1),
	 NULL);
    button2 = XtVaCreateManagedWidget
	("button2", xfwfButtonWidgetClass, board,
	 XtVaTypedArg, XtNtranslations, XtRString, trans2, sizeof(trans2),
	 NULL);
    button3 = XtVaCreateManagedWidget
	("button3", xfwfButtonWidgetClass, board,
	 XtVaTypedArg, XtNtranslations, XtRString, trans3, sizeof(trans2),
	 NULL);
    button4 = XtVaCreateManagedWidget
	("button4", xfwfButtonWidgetClass, board,
	 XtVaTypedArg, XtNtranslations, XtRString, trans4, sizeof(trans2),
	 NULL);
    button5 = XtVaCreateManagedWidget
	("button5", xfwfButtonWidgetClass, board,
	 XtVaTypedArg, XtNtranslations, XtRString, trans5, sizeof(trans2),
	 NULL);

    XtAddCallback(menu1, XtNactivate, activate, NULL);
    XtAddCallback(menu1, XtNchangeSelection, change, NULL);
    XtAddCallback(menu2, XtNactivate, activate, NULL);
    XtAddCallback(menu2, XtNchangeSelection, change, NULL);
    XtAddCallback(menu3, XtNactivate, activate, NULL);
    XtAddCallback(menu3, XtNchangeSelection, change, NULL);
    XtAddCallback(menu4, XtNactivate, activate, NULL);
    XtAddCallback(menu4, XtNchangeSelection, change, NULL);
    XtAddCallback(menu5, XtNactivate, activate, NULL);
    XtAddCallback(menu5, XtNchangeSelection, change, NULL);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
