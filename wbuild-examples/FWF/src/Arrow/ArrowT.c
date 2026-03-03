static char rcsid[] = "$Header: RowColT.c,v 1.1 92/10/12 13:32:05 bert Exp $";

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Arrow.h>

static String fallback_resources[] = {
    "*width: 200",
    "*height: 200",

    "*borderWidth: 0",

    "*tester.foreground: steelblue",

    NULL,
};

static XtAppContext app_context;

static void do_something(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    (void) printf("do something\n");
}


int main(argc, argv)
    int argc;
    char *argv[];
{
    Widget toplevel, tester;

    toplevel = XtVaAppInitialize(&app_context, "ArrowT", NULL, 0,
				  &argc, argv, fallback_resources, NULL);
    tester = XtVaCreateManagedWidget("tester", xfwfArrowWidgetClass,
				     toplevel, NULL);
    XtAddCallback(tester, XtNcallback, do_something, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}

