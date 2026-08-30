#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Label.h>
#include <X11/Xaw/Command.h>

static XtAppContext app_context;
static Widget toplevel, tester, sub1, sub2, sub3, qbtn;

static String fallback_resources[] = {
    "*tester.location: 0 0 520 440",		/* Overall size */

    "LabelT*margin: 5",				/* Global defaults */
    "LabelT*borderWidth: 0",
    "LabelT*frameWidth: 2",
    "LabelT*background: #ffffb6b60000",

    "*tester.outerOffset: 16",			/* Margin around frame */
    "*tester.innerOffset: 6",			/* Margin inside frame */
    "*tester.cursor: spider",
    "*tester.frameWidth: 2",
    "*tester.frameType: sunken",

    /* 
     * The coordinates of the quit button are relative to the top left
     * corner of the tester widget, because quit is not a subclass of
     * XfwfBoard. The rest of the widgets below has coordinates
     * relative to the area *inside* the tester widget's frame.
     */
    "*quit.label: Quit",
    "*quit.x: 24",
    "*quit.y: 24",

    "*sub1.label: \
This program shows multiline labels,\\n\
aligned in various ways,\\n\
inside widgets that grow with the window.",
    "*sub1.alignment: bottom",
    "*sub1.location: 20 45 1.0-40 120",
    "*sub1.foreground: red",
    "*sub1.rvStart: 19",			/* Reverse "multiline" */
    "*sub1.rvLength: 9",
    "*sub1.hlForeground: blue",
    "*sub1.hlStart: 37",			/* Highlight "aligned" */
    "*sub1.hlLength: 7",

    "*sub2.label: at center right",
    "*sub2.alignment: center right",
    "*sub2.location: 5 180 0.5-10 50",
    "LabelT*sub2.background: #5555dbdbaaaa",
    "*sub2.foreground: white",
    "*sub2.cursor: heart",

    "*sub3.frameType: chiseled",
    "*sub3.frameWidth: 4",
    "*sub3.label: at\\ntop\\nleft	tab\\n	tab\\n(insensitive)",
    "*sub3.tablist: 45",
    "*sub3.sensitive: False",
    "*sub3.alignment: top left",
    "*sub3.location: 5 250 150 1.0-255",
    "*sub3.foreground: blue",
    "*sub3.cursor: draped_box",

    "*qbtn.foreground: black",

    NULL,
};


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
    XCC xcc;

    toplevel = XtVaAppInitialize(&app_context, "LabelT", NULL, 0,
				  &argc, argv, fallback_resources, NULL);
    tester = XtVaCreateManagedWidget("tester", xfwfLabelWidgetClass,
				     toplevel, NULL);

    sub1 = XtVaCreateManagedWidget("sub1", xfwfLabelWidgetClass, tester,
                                   NULL);
    sub2 = XtVaCreateManagedWidget("sub2", xfwfLabelWidgetClass, tester,
                                   NULL);
    sub3 = XtVaCreateManagedWidget("sub3", xfwfLabelWidgetClass, tester,
                                   NULL);
    qbtn = XtVaCreateManagedWidget("quit", commandWidgetClass, tester,
                                   NULL);
    XtAddCallback(qbtn, XtNcallback, quit, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
