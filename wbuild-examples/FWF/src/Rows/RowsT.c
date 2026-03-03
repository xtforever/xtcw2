#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Rows.h>
#include <Xfwf/Button.h>

#define MAX 20

static XtAppContext app_context;
static Widget toplevel, tester, sub[MAX], quit_button, rowcol_button;
Boolean byRow = True;

static String fallback[] = {
    "RowsT.tester.width: 500",
    "RowsT.tester.height: 500",

    "RowsT*borderWidth: 0",
    "RowsT*background: gray50",
    "RowsT*foreground: white",

    "RowsT*tester.outerOffset: 6",
    "RowsT*tester.cursor: spider",
    "RowsT*tester.frameWidth: 2",
    "RowsT*tester.frameType: sunken",
    "RowsT*tester.alignTop: False",

    "RowsT*XfwfButton.frameWidth: 2",

    NULL,
};

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
 * main -- main routine of RowsT
 *
 * Initialize toolkit, create a Rows widget with MAX buttons of
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

    toplevel = XtVaAppInitialize
	(&app_context, "RowsT", NULL, 0, &argc, argv, fallback, NULL);
    tester = XtVaCreateManagedWidget
	("tester", xfwfRowsWidgetClass, toplevel, NULL);

    /*
     * Create MAX buttons
     */
    for (i = 0, w = 80 + MAX, h = 23; i < MAX; i++, w -= 1, h += 1) {
	(void) sprintf(t, "%d", i);
	s = XtNewString(t);
	sub[i] = XtVaCreateManagedWidget
	    (s, xfwfButtonWidgetClass, tester,
	     XtNlabel, s, XtNwidth, w, XtNheight, h, NULL);
    }
    /*
     * Create a quit button
     */
    w -= 1; h += 1;
    quit_button = XtVaCreateManagedWidget
	("quit", xfwfButtonWidgetClass, tester,
	 XtNlabel, "quit", XtNwidth, w, XtNheight, h, NULL);
    XtAddCallback(quit_button, XtNactivate, quit, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
