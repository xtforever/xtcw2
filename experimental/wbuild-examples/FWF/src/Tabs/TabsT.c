#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Board.h>
#include <Xfwf/Label.h>
#include <Xfwf/Tabs.h>

static String fallback_resources[] = {
    "*board.location:		0 0 600 550",
    "*board*background:		#0092aa",
    "*board.background:		#55b6aa",
    /* "*highlightThickness:	0", */
    "*XfwfTabs.font:		*-helvetica-bold-r-*--12-*",
    "*XfwfTabs.frameWidth:	1",
    "*XfwfTabs.tabcolor:	#5592aa",
    "*XfwfTabs.foreground:	white",

    "*label.location:		35 35 1.0-70 1.0-70",
    "*label.borderWidth:	0",

    /*
     * tabs along the top:
     * tab is as wide as label, plus default textmargin
     * corners are 4x2 pixels
     * textmargin looks best if it is as large as cornerwidth
     */
    "*tabs1.location:		35 5 1.0-70 30",
    "*tabs1.tabWidthPercentage:	0",
    "*tabs1.cornerwidth:	4",
    "*tabs1.cornerheight:	2",
    "*tabs1.textmargin:		4",
    /*
     * tabs along the left side:
     * tab is as wide as label, plus default textmargin
     * no diagonal corners
     */
    "*tabs2.location:		5 35 30 1.0-70",
    "*tabs2.tabWidthPercentage:	0",
    "*tabs2.cornerwidth:	0",
    "*tabs2.cornerheight:	0",
    /*
     * tabs along the bottom:
     * each tab is 16% of the total width
     * corners are default (3x3 pixels)
     */
    "*tabs3.location:		35 1.0-35 1.0-70 30",
    "*tabs3.tabWidthPercentage:	14",
    /*
     * tabs along the right side
     * each tab is 60% of the total length
     * corners replaced with a single diagonal line, 2*5=10 pixels skew
     */
    "*tabs4.location:		1.0-35 35 30 1.0-70",
    "*tabs4.tabWidthPercentage:	60",
    "*tabs4.cornerwidth:	5",
    "*tabs4.cornerheight:	9999",		/* A large number */
    "*tabs4.textmargin:		5",
    NULL,
};

static Widget toplevel, board, label, tabs1, tabs2, tabs3, tabs4;
static XtAppContext app_context;

static String texts[] = {
    "This is a test for the XfwfTabs widget.\n\
Click on any of the tabs to display the\n\
corresponding text.\n\
\n\
Top:\n\
tabWidthPercentage=0, cornerwidth=4,\n\
cornerheight=2, textmargin=4\n\
\n\
Left:\n\
tabWidthPercentage=0, cornerwidth=0,\n\
cornerheight=0, [textmargin=3]\n\
\n\
Bottom:\n\
tabWidthPercentage=14, [cornerwidth=3],\n\
[cornerheight=3], [textmargin=3]\n\
\n\
Right:\n\
tabWidthPercentage=60, cornerwidth=5,\n\
cornerheight=9999, textmargin=5\n\
",
"A click on a tab results in a callback\n\
that is used by the test program to\n\
change the text in this label\n\
and to change the number of tabs.",
    "This tab has an empty label",
    "The last tab on the right is the\n\
`Quit' button. The test program will\n\
exit when you click that one.",
    "This is more filler text",
    "There is no space between the tabs\n\
and the label, and the label\n\
has no border.\n\
That's why the tabs and the\n\
label appear to be visually\n\
connected.",
    "This won't get displayed",
};

static String labels[] = {
    "Introduction",
    "Clicks",
    "",						/* An empty label */
    "About the `Quit' tab",
    "More filler",
    "Pixel trick",
    "Quit",
};

static int cur = 0;				/* Current label/text */


/* ARGSUSED */
static void handle_click(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    int tab = (int) call_data;

    if (tab == 0) return;
    cur += tab;

    if (cur == XtNumber(labels) - 1) exit(0);	/* Quit button? */

    /* Change tabs */
    XtVaSetValues(tabs1,
		  XtNlefttabs, cur,
		  XtNrighttabs, XtNumber(labels) - cur - 1,
		  NULL);
    XtVaSetValues(tabs2,
		  XtNlefttabs, cur,
		  XtNrighttabs, XtNumber(labels) - cur - 1,
		  NULL);
    XtVaSetValues(tabs3,
		  XtNlefttabs, cur,
		  XtNrighttabs, XtNumber(labels) - cur - 1,
		  NULL);
    XtVaSetValues(tabs4,
		  XtNlefttabs, cur,
		  XtNrighttabs, XtNumber(labels) - cur - 1,
		  NULL);
    /* Change text */
    XtVaSetValues(label, XtNlabel, texts[cur], NULL);
}


void main(argc, argv)
    int argc;
    String *argv;
{
    toplevel = XtVaAppInitialize(&app_context, argv[0], NULL, 0,
				  &argc, argv, fallback_resources, NULL);
    board = XtVaCreateManagedWidget
	("board", xfwfBoardWidgetClass, toplevel, NULL);
    tabs1 = XtVaCreateManagedWidget
	("tabs1", xfwfTabsWidgetClass, board,
	 XtNlabels, labels,
	 XtNlefttabs, cur,
	 XtNrighttabs, XtNumber(labels) - 1 - cur,
	 XtNorientation, XfwfUpTabs,
	 NULL);
    tabs2 = XtVaCreateManagedWidget
	("tabs2", xfwfTabsWidgetClass, board,
	 XtNlabels, labels,
	 XtNlefttabs, cur,
	 XtNrighttabs, XtNumber(labels) - 1 - cur,
	 XtNorientation, XfwfLeftTabs,
	 NULL);
    tabs3 = XtVaCreateManagedWidget
	("tabs3", xfwfTabsWidgetClass, board,
	 XtNlabels, labels,
	 XtNlefttabs, cur,
	 XtNrighttabs, XtNumber(labels) - 1 - cur,
	 XtNorientation, XfwfDownTabs,
	 NULL);
    tabs4 = XtVaCreateManagedWidget
	("tabs4", xfwfTabsWidgetClass, board,
	 XtNlabels, labels,
	 XtNlefttabs, cur,
	 XtNrighttabs, XtNumber(labels) - 1 - cur,
	 XtNorientation, XfwfRightTabs,
	 NULL);
    label = XtVaCreateManagedWidget
	("label", xfwfLabelWidgetClass, board,
	 XtNlabel, texts[cur],
	 NULL);
    XtRealizeWidget(toplevel);
    XtAddCallback(tabs1, XtNactivate, handle_click, NULL);
    XtAddCallback(tabs2, XtNactivate, handle_click, NULL);
    XtAddCallback(tabs3, XtNactivate, handle_click, NULL);
    XtAddCallback(tabs4, XtNactivate, handle_click, NULL);
    XtAppMainLoop(app_context);
}
