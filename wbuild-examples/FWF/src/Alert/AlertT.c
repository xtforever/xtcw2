/* AlertT -- test file for Alert widget */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Alert.h>
#include <Xfwf/Button.h>
#ifdef X11R5
#include <X11/Xmu/Editres.h>
#endif

Widget toplevel, button;
XtAppContext app_context;

String fallback_resources[] = {
    /*
     * The main window
     */
    "*button.location: 0 0 200 130",
    "*button.shrinkToFit: True",
    "*button.background: gray75",
    "*button.foreground: black",
    "*button.outerOffset: 10",
    "*button.innerOffset: 10",
    "*button.font: *-times-bold-r-normal-*-240-*",
    "*button.label: \
This test program shows an\\n\
XfwfAlert widget in action.\\n\
If everything is working as it should,\\n\
you should see a blue dialog box\\n\
on top of this window\\n\
after clicking here.",
    /*
     * The popup
     */
    "*tester*background: lightSlateBlue",
    "*tester*foreground: yellow",
    "*tester*font: *-helvetica-bold-r-normal-*-180-*",
    "*tester.outer.innerOffset: 10",
    NULL,
};


static void activate(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    int b;

    b = XfwfDialog
	("tester", toplevel, "INFO",
	 "\nTest\n\nClick on a button\n to choose a number\n",
	 " Exit | One | Two | Three ");
    (void) printf("You chose button %d\n", b);
    if (b == 0) exit(0);
}


main(argc, argv)
    int argc;
    String argv[];
{
    toplevel = XtVaAppInitialize
	(&app_context, "AlertT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);
#ifdef X11R5
    XtAddEventHandler(toplevel, 0, True, _XEditResCheckMessages, NULL);
#endif    
    button = XtVaCreateManagedWidget
	("button", xfwfButtonWidgetClass, toplevel, NULL);
    XtAddCallback(button, XtNactivate, activate, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
