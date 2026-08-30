#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Board.h>
#include <Xfwf/Enforcer.h>
#include <X11/Xaw/Command.h>

static XtAppContext app_context;
static Widget toplevel, tester, expl, enforcer;

static String fallback_resources[] = {
    "*tester.location: 0 0 550 300",		/* Overall size */

    "*borderWidth: 0",				/* For all widgets */
    "*frameWidth: 2",

    "*tester.outerOffset: 6",			/* 6 pixel margin around */
    "*tester.cursor: spider",			/* Just an example */
    "*tester.frameType: sunken",

    "*enforcer.location: 0.1 0.1 0.8 0.4",
    "*enforcer.innerOffset: 4",

    "*explain.label: Resize window to see widgets change; click = stop",

    NULL,
};

/* ARGSUSED */
void quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    exit(0);
}

int main(argc, argv)
    int argc;
    char *argv[];
{
    toplevel = XtVaAppInitialize(&app_context, "BoardT", NULL, 0,
				  &argc, argv, fallback_resources, NULL);
    tester = XtVaCreateManagedWidget
	("tester", xfwfBoardWidgetClass, toplevel, NULL);
    enforcer = XtVaCreateManagedWidget
	("enforcer", xfwfEnforcerWidgetClass, tester, NULL);
    expl = XtVaCreateManagedWidget
	("explain", commandWidgetClass, enforcer, NULL);
    XtAddCallback(expl, XtNcallback, quit, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
