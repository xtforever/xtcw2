#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/MenuButton.h>
#include <Xfwf/Frame.h>

static XtAppContext app_context;
static Widget toplevel, outer, box, tester1, tester2, tester3,
    tester4, button1, button2, button3, button4;

static String fallback_resources[] = {

    /* Main window and global resources */

    "FrameT.width: 500",			/* Overall size */
    "FrameT.height: 300",
    "*box.width: 500",				/* if above didn't work... */
    "*box.height: 300",

    "*outer.frameType: sunken",
    "*outer.outerOffset: 5",
    "*outer.frameWidth: 2",

    "*outer*borderWidth: 0",			/* For all widgets */

    /*
     * Button 1
     * --------
     * 10 pixels wide chiseled frame, set in 10 pixels from the
     * widget's edge; set shadow scheme to use explicit colors.
     */

    "*tester1.frameType: chiseled",
    "*tester1.outerOffset: 10",
    "*tester1.frameWidth: 10",
    "*tester1.background: SteelBlue1",
    "*tester1.shadowScheme: color",
    "*tester1.topShadowColor: white",
    "*tester1.bottomShadowColor: darkSlateBlue",
    "*button1.background: SteelBlue1",
    "*button1.label: press to quit",

    /*
     * Button 2
     * --------
     * 8 pixels wide ledged frame, set in 13 pixels from the widget's
     * edge; uses automatic shadows (default)
     */

    "*tester2.frameType: ledged",
    "*tester2.outerOffset: 13",
    "*tester2.frameWidth: 8",
    "*tester2.background: MediumOrchid1",
    "*button2.background: MediumOrchid1",
    "*button2.label: press to iconify",

    /*
     * Button 3
     * --------
     * 6 pixels wide raised frame, set in 14 pixels from the widget's
     * edge
     */

    "*tester3.frameType: raised",
    "*tester3.outerOffset: 14",
    "*tester3.frameWidth: 6",
    "*tester3.background: OrangeRed1",
    "*button3.background: OrangeRed1",
    "*button3.label: press to add new widget",

    /*
     * Button 4
     * --------
     * 4 pixels wide sunken frame, set in 13 pixels fromthe widget's
     * edge
     */

    "*tester4.frameType: sunken",
    "*tester4.outerOffset: 10",
    "*tester4.frameWidth: 4",
    "*tester4.background: VioletRed1",
    "*button4.background: VioletRed3",
    "*button4.label: press to quit",

    /*
     * All dynamically added buttons get a frame width of 4 pixels
     */

    "*new.frameWidth: 4",

    NULL,
};


static void add_new(widget, clientData, callData)
    Widget widget;
    XtPointer clientData;
    XtPointer callData;
{
    Widget new_part;

    new_part = XtCreateWidget
	("new", xfwfFrameWidgetClass, (Widget) clientData, NULL, 0);
    XtCreateManagedWidget
	("new button", commandWidgetClass, new_part, NULL, 0);
    XtRealizeWidget(new_part);
    XtManageChild(new_part);
}


static void iconify(widget, clientData, callData)
    Widget widget;
    XtPointer clientData;
    XtPointer callData;
{
    XIconifyWindow(XtDisplay(toplevel), XtWindow(toplevel), 
		   XScreenNumberOfScreen(XtScreen(toplevel)));
}


static void quit(widget, client_data, call_data)
    Widget widget;
    XtPointer client_data, call_data;
{
    exit(0);
}


int main(argc, argv)
    int argc;
    char *argv[];
{
    toplevel = XtVaAppInitialize(&app_context, "FrameT", NULL, 0,
				 &argc, argv, fallback_resources, NULL);
#ifdef DEBUG
    XSynchronize(XtDisplay(toplevel), True);
#endif
    /*
     * Create an XfwfFrame with a Box in it.
     * Then create four Commands inside XfwfFrames and put them in the Box.
     * Add callbacks to each of the four Commands.
     */
    outer = XtCreateManagedWidget
	("outer", xfwfFrameWidgetClass, toplevel, NULL, 0);
    box = XtCreateManagedWidget
	("box", boxWidgetClass, outer, NULL, 0);

    tester1 = XtCreateManagedWidget
	("tester1", xfwfFrameWidgetClass, box, NULL, 0);
    button1 = XtCreateManagedWidget
	("button1", commandWidgetClass, tester1, NULL, 0);

    tester2 = XtCreateManagedWidget
	("tester2", xfwfFrameWidgetClass, box, NULL, 0);
    button2 = XtCreateManagedWidget
	("button2", commandWidgetClass, tester2, NULL, 0);

    tester3 = XtCreateManagedWidget
	("tester3", xfwfFrameWidgetClass, box, NULL, 0);
    button3 = XtCreateManagedWidget
	("button3", commandWidgetClass, tester3, NULL, 0);

    tester4 = XtCreateManagedWidget
	("tester4", xfwfFrameWidgetClass, box, NULL, 0);
    button4 = XtCreateManagedWidget
	("button4", commandWidgetClass, tester4, NULL, 0);

    XtAddCallback(button1, XtNcallback, quit, NULL);
    XtAddCallback(button2, XtNcallback, iconify, NULL);
    XtAddCallback(button3, XtNcallback, add_new, box);
    XtAddCallback(button4, XtNcallback, quit, NULL);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
