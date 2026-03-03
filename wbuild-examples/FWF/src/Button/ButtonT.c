/*  Include files  */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Board.h>
#include <Xfwf/Button.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define LABEL1 "Click here\nto stop"
#define LABEL2 "CLICK HERE\nTO STOP"

static String fallback_resources[] = {
    "*board.location:		0   0        300     350",
    "ButtonT*background:	thistle",
    "*1.location:		40  20       1.0-80  0.25-25",
    "*2.location:		40  0.25+15  1.0-80  0.25-25",
    "*3.location:		40  0.50+10  1.0-80  0.25-25",
    "*4.location:		40  0.75+5   1.0-80  0.25-25",
    "*4.label:			Click here\\nto iconify",
    "*3.cursor:			pirate",
    "*4.cursor:			dotbox",
    "*XfwfButton.borderWidth:	0",
    "*XfwfButton.frameWidth:	2",
    "*XfwfButton.alignment:	bottom right",
    "*XfwfButton.innerOffset:	2",
    NULL,
};

static Widget toplevel, board, button1, button2, button3, button4;
static XtAppContext app_context;

static count1 = 0, count2 = 0;


/* ARGSUSED */
static void iconify(widget, client_data, call_data)
    Widget widget;
    XtPointer client_data, call_data;
{
    XIconifyWindow(XtDisplay(toplevel), XtWindow(toplevel), 
		   XScreenNumberOfScreen(XtScreen(toplevel)));
}


/* ARGSUSED */
static void activate(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    char label[20];

    if (w == button1) {
	(void) sprintf(label, "# clicks: %d", ++count1);
	/* XtVaSetValues(w, XtNlabel, label, NULL); */
    } else {
	(void) sprintf(label, "# clicks: %d", ++count2);
	/* XtVaSetValues(w, XtNlabel, label, NULL); */
    }
    XfwfLabelSetLabel(w, label);
}


/* ARGSUSED */
static void enter_quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    /* XtVaSetValues(w, XtNlabel, LABEL2, NULL); */
    XfwfLabelSetLabel(w, LABEL2);
}


/* ARGSUSED */
static void leave_quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    /* XtVaSetValues(w, XtNlabel, LABEL1, NULL); */
    XfwfLabelSetLabel(w, LABEL1);
}


/* ARGSUSED */
static void quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    exit(0);
}


int main(argc, argv)
    int argc;
    String *argv;
{
    toplevel = XtVaAppInitialize(&app_context, "ButtonT", NULL, 0,
				  &argc, argv, fallback_resources, NULL);
    board = XtVaCreateManagedWidget
	("board", xfwfBoardWidgetClass, toplevel, NULL);
    button1 = XtVaCreateManagedWidget
	("1", xfwfButtonWidgetClass, board, XtNlabel, "# clicks: 0", NULL);
    button2 = XtVaCreateManagedWidget
	("2", xfwfButtonWidgetClass, board, XtNlabel, "# clicks: 0", NULL);
    button3 = XtVaCreateManagedWidget
	("3", xfwfButtonWidgetClass, board, XtNlabel, LABEL1, NULL);
    button4 = XtVaCreateManagedWidget
	("4", xfwfButtonWidgetClass, board, NULL);
    XtRealizeWidget(toplevel);
    XtAddCallback(button1, XtNactivate, activate, NULL);
    XtAddCallback(button2, XtNactivate, activate, NULL);
    XtAddCallback(button3, XtNactivate, quit, NULL);
    XtAddCallback(button3, XtNenter, enter_quit, NULL);
    XtAddCallback(button3, XtNleave, leave_quit, NULL);
    XtAddCallback(button4, XtNactivate, iconify, NULL);
    XtAppMainLoop(app_context);
}
