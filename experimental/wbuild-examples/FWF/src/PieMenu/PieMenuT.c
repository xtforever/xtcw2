#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Label.h>
#include <Xfwf/PieMenu.h>

Widget toplevel, label, pie;
static XtAppContext app_context;

String fallbacks[] = {				/* Fallback resources */
    "PieMenuT*background: aquamarine",
    "*label.width: 300",
    "*label.height: 300",
    "*label.frameWidth: 2",
    "*label.frameType: sunken",
    "*label.label: PRESS HERE\\nto pop up a\\nPIE MENU",
    "*label.translations: <BtnDown>: placemenu() XtMenuPopup(pie)",

    /*
     * This pie menu has four entries. Note that the fourth item has
     * two actions. The font and foreground are the same for all of
     * them, except that the foreground for the fouth item is
     * overridden. The background is set differently for each item.
     */
    "*pie.width: 128",
    "*pie.menu: \
	Yellow -> color(Yellow)\\n\
	Green  -> color(Green)\\n\
	Gray   -> color(Gray)\\n\
	Exit   -> color(Red) quit()",
    "*pie.font: *-helvetica-bold-r-*-140-*",
    "*pie.foreground: black",
    "*pie.foreground4: white",
/*
    "*pie.background1: yellow",
    "*pie.background2: green",
    "*pie.background3: gray75",
*/
    "*pie.background4: red",
    NULL,
};


static void placemenu(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    Dimension wd, ht;

    XtVaGetValues(pie,
		  XtNwidth, &wd,
		  XtNheight, &ht,
		  NULL);
    /* Center pie menu on mouse cursor */
    XtVaSetValues(pie,
		  XtNx, event->xbutton.x_root - wd/2,
		  XtNy, event->xbutton.y_root - ht/2,
		  NULL);
}


static void quit(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    exit(0);
}


static void color(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    char s[100];

    sprintf(s, "%s, eh?", params[0]);
    XtVaSetValues(label, XtNlabel, s, NULL);
}


static XtActionsRec actions[] = {
{ "color", color },
{ "quit", quit },
{ "placemenu", placemenu },
};


int main(argc, argv)
    int argc;
    char *argv[];
{
    toplevel = XtAppInitialize(&app_context, "PieMenuT", NULL, 0,
			       &argc, argv, fallbacks, NULL, 0);

#if 0
    (void) XSynchronize(XtDisplay(toplevel), True);
#endif

    XtAppAddActions(app_context, actions, XtNumber(actions));

    label = XtCreateManagedWidget("label", xfwfLabelWidgetClass,
				  toplevel, NULL, 0);

    pie = XtCreatePopupShell("pie", xfwfPieMenuWidgetClass,
			     toplevel, NULL, 0);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
