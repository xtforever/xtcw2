#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Board.h>
#include <X11/Xaw/Command.h>

static XtAppContext app_context;
static Widget toplevel, tester, sub1, sub2, sub3, expl;

static String fallback_resources[] = {
    "*tester.location: 0 0 550 300",		/* Overall size */

    "*borderWidth: 0",				/* For all widgets */
    "*frameWidth: 2",

    "*tester.outerOffset: 6",			/* 6 pixel margin around */
    "*tester.cursor: spider",			/* Just an example */
    "*tester.frameType: sunken",

    /* the next 6 are equivalent to: *sub1.location: 20 20 1.0-40 70 */
    "*sub1.abs_x: 20",				/* Fixed x coord.	*/
    "*sub1.abs_y: 20",				/* Fixed y coord.	*/
    "*sub1.abs_height: 70",			/* \ Fixed height	*/
    "*sub1.rel_height: 0.0",			/* /			*/
    "*sub1.rel_width: 1.0",			/* \ 40 pixels narrower	*/
    "*sub1.abs_width: -40",			/* / than parent	*/

    /* The sub2 widget shrinks when the parent grows; "-0.2" */
    "*sub2.location: 0.5-40 100 80 165-0.2",
    "*sub2.background: seagreen",

    /* The sub3 widget grows faster than the parent: "1.2" */
    "*sub3.location: 0.5-40 275-0.2 80 1.2-295",
    "*sub3.cursor: coffee_mug",

    "*explain.label: Resize window to see widgets change; click = stop",
    "*explain.x: 10",
    "*explain.y: 10",

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
    tester = XtVaCreateManagedWidget("tester", xfwfBoardWidgetClass,
				     toplevel, NULL);
    sub1 = XtVaCreateManagedWidget("sub1", xfwfBoardWidgetClass, tester, NULL);
    sub2 = XtVaCreateManagedWidget("sub2", xfwfBoardWidgetClass, tester, NULL);
    sub3 = XtVaCreateManagedWidget("sub3", xfwfBoardWidgetClass, tester, NULL);
    expl = XtVaCreateManagedWidget("explain", commandWidgetClass, sub1, NULL);
    XtAddCallback(expl, XtNcallback, quit, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
