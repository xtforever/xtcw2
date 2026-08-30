#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/RowCol.h>
#include <Xfwf/Button.h>

#define MAX 20

static XtAppContext app_context;
static Widget toplevel, tester, sub[MAX], quit_button, rowcol_button;
Boolean byRow = True;

static String fallback_resources[] = {
    "RowColT.tester.width: 500",
    "RowColT.tester.height: 500",

    "RowColT*borderWidth: 0",
    "RowColT*background: gray50",
    "RowColT*foreground: white",

    "RowColT*tester.shrinkToFit: True",
    "RowColT*tester.outerOffset: 6",
    "RowColT*tester.width: 400",
    "RowColT*tester.height: 300",
    "RowColT*tester.cursor: spider",
    "RowColT*tester.frameWidth: 2",
    "RowColT*tester.frameType: sunken",

    "RowColT*tester.XfwfButton.frameWidth: 2",

    NULL,
};

/*
 * activate -- callback for all buttons
 *
 * The buttons are numbered 0 to MAX, the number of the activated
 * button becomes the value of the `columns' resource of the RowCol
 * widget.
 */
static void activate(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    int i;

    i = 0;
    while (w != sub[i]) i++;
    XtVaSetValues(tester, XtNcolumns, i, NULL);
}


/*
 * quit -- callback for quit button
 */
static void quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    exit(0);
}


/*
 * rowcol -- callback for rowcol button
 *
 * toggle between store by row and store by column
 */
static void rowcol(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    byRow = ! byRow;
    XtVaSetValues(tester, XtNstoreByRow, byRow, NULL);
}


/*
 * main -- main routine of RowColT
 *
 * Initialize toolkit, create a RowCol widget with MAX buttons of
 * different sizes and give each a label equal to its sequence number
 * 0 to MAX - 1.
 */
int main(argc, argv)
    int argc;
    char *argv[];
{
    int i, w, h;
    String s;
    char t[20];

    toplevel = XtVaAppInitialize(&app_context, "RowColT", NULL, 0,
				  &argc, argv, fallback_resources, NULL);
    tester = XtVaCreateManagedWidget("tester", xfwfRowColWidgetClass,
				     toplevel,
				     XtNstoreByRow, byRow,
				     NULL);

    /*
     * Create MAX buttons
     */
    for (i = 0, w = 80 + MAX, h = 23; i < MAX; i++, w -= 1, h += 1) {
	(void) sprintf(t, "%d", i);
	s = XtNewString(t);
	sub[i] = XtVaCreateManagedWidget(s, xfwfButtonWidgetClass, tester,
					 XtNlabel, s, XtNwidth, w,
					 XtNheight, h, NULL);
	XtAddCallback(sub[i], XtNactivate, activate, NULL);
    }
    /*
     * Create a button to toggle between `by row' and `by column'
     */
    rowcol_button = XtVaCreateManagedWidget("rowcol", xfwfButtonWidgetClass,
					    tester, XtNlabel, "row/col",
					    XtNwidth, w, XtNheight, h, NULL);
    XtAddCallback(rowcol_button, XtNactivate, rowcol, NULL);
    /*
     * Create a quit button
     */
    w -= 1; h += 1;
    quit_button = XtVaCreateManagedWidget("quit", xfwfButtonWidgetClass,
					  tester, XtNlabel, "quit",
					  XtNwidth, w, XtNheight, h, NULL);
    XtAddCallback(quit_button, XtNactivate, quit, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
