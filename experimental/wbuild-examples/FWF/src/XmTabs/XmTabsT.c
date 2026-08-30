#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xfwf/XmTabs.h>

#define TABHT 30
#define NR_OF_LABELS 7

static String fallback_resources[] = {
    "XmTabsT.geometry:		600x600",
    "*board*background:		#0092aa",
    "*board.background:		#55b6aa",
    "*board.shadowThickness:	0",
    "*board.borderWidth:	0",
    "*XmTabs.shadowThickness:	2",
    "*XmTabs.font:		*-helvetica-medium-r-*--14-*",
    "*XmTabs.tabcolor:		#5592aa",
    "*XmTabs.labels:\
|Introduction|Clicks||About the `Quit' tab|More filler|Pixel trick|Quit",
    /*
     * tabs along the top:
     * tab is as wide as label, plus default textmargin
     * corners are 4x2 pixels
     * textmargin looks best if it is as large as cornerwidth
     */
    "*tabs1.tabWidthPercentage:	0",
    "*tabs1.cornerwidth:	4",
    "*tabs1.cornerheight:	2",
    "*tabs1.textmargin:		4",
    /*
     * tabs along the left side:
     * tab is as wide as label, plus default textmargin
     * no diagonal corners
     */
    "*tabs2.tabWidthPercentage:	0",
    "*tabs2.cornerwidth:	0",
    /*
     * tabs along the botom:
     * each tab is 16% of the total width
     * corners are default (3x3 pixels)
     */
    "*tabs3.tabWidthPercentage:	14",
    /*
     * tabs along the right side
     * each tab is 60% of the total length
     * corners replaced with a single diagonal line, 2*5=10 pixels skew
     */
    "*tabs4.tabWidthPercentage:	60",
    "*tabs4.cornerwidth:	5",
    "*tabs4.cornerheight:	9999",		/* A large number */
    "*tabs4.textmargin:		5",
    NULL,
};

static Widget toplevel, board, label, tabs1, tabs2, tabs3, tabs4;
static XtAppContext app_context;

static String texts[] = {
    "This is a test for the XmTabs widget.\n\
Click on any of the tabs to display the\n\
corresponding text.",
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

static int cur = 0;				/* Current label/text */


/* ARGSUSED */
static void handle_click(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    int tab = (int) call_data;

    if (tab == 0) return;
    cur += tab;

    if (cur == NR_OF_LABELS - 1) exit(0);	/* Quit button? */

    /* Change tabs */
    XtVaSetValues(tabs1,
		  XtNlefttabs, cur,
		  XtNrighttabs, NR_OF_LABELS - cur - 1,
		  NULL);
    XtVaSetValues(tabs2,
		  XtNlefttabs, cur,
		  XtNrighttabs, NR_OF_LABELS - cur - 1,
		  NULL);
    XtVaSetValues(tabs3,
		  XtNlefttabs, cur,
		  XtNrighttabs, NR_OF_LABELS - cur - 1,
		  NULL);
    XtVaSetValues(tabs4,
		  XtNlefttabs, cur,
		  XtNrighttabs, NR_OF_LABELS - cur - 1,
		  NULL);
    /* Change text */
    XtVaSetValues(label, XtVaTypedArg, XmNlabelString,
		  XtRString, texts[cur], strlen(texts[cur]) + 1, NULL);
}


int main(argc, argv)
    int argc;
    String *argv;
{
    toplevel = XtVaAppInitialize(&app_context, argv[0], NULL, 0,
				  &argc, argv, fallback_resources, NULL);
    board = XtVaCreateManagedWidget
	("board", xmFormWidgetClass, toplevel, NULL);
    tabs1 = XtVaCreateManagedWidget
	("tabs1", xmTabsWidgetClass, board,
	 XtNlefttabs, cur,
	 XtNrighttabs, NR_OF_LABELS - 1 - cur,
	 XtNorientation, XfwfUpTabs,
	 XmNtopAttachment, XmATTACH_FORM,
	 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset, TABHT,
	 XmNrightAttachment, XmATTACH_FORM, XmNrightOffset, TABHT,
	 XmNbottomAttachment, XmATTACH_OPPOSITE_FORM, XmNbottomOffset, -TABHT,
	 NULL);

    (void) XtVaCreateManagedWidget
	("test", xmFormWidgetClass, tabs1, NULL);
    
    tabs2 = XtVaCreateManagedWidget
	("tabs2", xmTabsWidgetClass, board,
	 XtNlefttabs, cur,
	 XtNrighttabs, NR_OF_LABELS - 1 - cur,
	 XtNorientation, XfwfLeftTabs,
	 XmNleftAttachment, XmATTACH_FORM,
	 XmNtopAttachment, XmATTACH_FORM, XmNtopOffset, TABHT,
	 XmNbottomAttachment, XmATTACH_FORM, XmNbottomOffset, TABHT,
	 XmNrightAttachment, XmATTACH_OPPOSITE_FORM, XmNrightOffset, -TABHT,
	 NULL);
    tabs3 = XtVaCreateManagedWidget
	("tabs3", xmTabsWidgetClass, board,
	 XtNlefttabs, cur,
	 XtNrighttabs, NR_OF_LABELS - 1 - cur,
	 XtNorientation, XfwfDownTabs,
	 XmNbottomAttachment, XmATTACH_FORM,
	 XmNleftAttachment, XmATTACH_FORM, XmNleftOffset, TABHT,
	 XmNrightAttachment, XmATTACH_FORM, XmNrightOffset, TABHT,
	 XmNtopAttachment, XmATTACH_OPPOSITE_FORM, XmNtopOffset, -TABHT,
	 NULL);
    tabs4 = XtVaCreateManagedWidget
	("tabs4", xmTabsWidgetClass, board,
	 XtNlefttabs, cur,
	 XtNrighttabs, NR_OF_LABELS - 1 - cur,
	 XtNorientation, XfwfRightTabs,
	 XmNrightAttachment, XmATTACH_FORM,
	 XmNtopAttachment, XmATTACH_FORM, XmNtopOffset, TABHT,
	 XmNbottomAttachment, XmATTACH_FORM, XmNbottomOffset, TABHT,
	 XmNleftAttachment, XmATTACH_OPPOSITE_FORM, XmNleftOffset, -TABHT,
	 NULL);
    label = XtVaCreateManagedWidget
	("label", xmLabelWidgetClass, board,
	 XtVaTypedArg, XmNlabelString, XmRString,
	 texts[cur], strlen(texts[cur]) + 1,
	 XmNtopAttachment, XmATTACH_WIDGET, XmNtopWidget, tabs1,
	 XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget, tabs2,
	 XmNbottomAttachment, XmATTACH_WIDGET, XmNbottomWidget, tabs3,
	 XmNrightAttachment, XmATTACH_WIDGET, XmNrightWidget, tabs4,
	 NULL);
    XtRealizeWidget(toplevel);
    XtAddCallback(tabs1, XtNactivateCallback, handle_click, NULL);
    XtAddCallback(tabs2, XtNactivateCallback, handle_click, NULL);
    XtAddCallback(tabs3, XtNactivateCallback, handle_click, NULL);
    XtAddCallback(tabs4, XtNactivateCallback, handle_click, NULL);
    XtAppMainLoop(app_context);
}
