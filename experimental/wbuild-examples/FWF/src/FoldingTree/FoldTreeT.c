/*  Include files  */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/FoldTree.h>
#include <Xfwf/Button.h>
#include <X11/Xmu/Editres.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static String fallback_resources[] = {
    "*tree1.expanded:		True",
    "*tree2.expanded:		True",
    "*tree3.expanded:		True",
    "*shrinkToFit:		True",
    "*alignment:		top left",
    "*XfwfLabel.frameWidth:	2",
    "*XfwfButton.borderWidth:	0",
    "*XfwfButton.frameWidth:	2",
    "*XfwfButton.innerOffset:	2",
    "*l1.label:			Tree label",
    "*l2.label:			Subtree label",
    "*l3.label:			Leafnode label",
    "*1.label:			Dummy button",
    "*2.label:			Dummy button",
    "*3.label:			Click here\\nto stop",
    "*4.label:			Click here\\nto iconify",
    "*3.cursor:			pirate",
    "*4.cursor:			dotbox",
    NULL,
};

static Widget toplevel, tree1, tree2, tree3;
static Widget label1, label2, label3;
static Widget button1, button2, button3, button4;
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
	XtVaSetValues(w, XtNlabel, label, NULL);
    } else {
	(void) sprintf(label, "# clicks: %d", ++count2);
	XtVaSetValues(w, XtNlabel, label, NULL);
    }
}


/* ARGSUSED */
static void print_stat(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    Bool expand = (Bool) call_data;

    if (expand)
        printf("Expanding %s...\n", XtName(w));
    else
        printf("Collapsing %s...\n", XtName(w));
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
    toplevel = XtVaAppInitialize(&app_context, "FoldTreeT", NULL, 0,
				  &argc, argv, fallback_resources, NULL);

    XtAddEventHandler(toplevel, (EventMask) 0, True,
		      _XEditResCheckMessages, NULL);

    tree1 = XtVaCreateManagedWidget
	("tree1", xfwfFoldingTreeWidgetClass, toplevel, NULL);
    label1 = XtVaCreateManagedWidget
	("l1", xfwfLabelWidgetClass, tree1, NULL);
    tree2 = XtVaCreateManagedWidget
	("tree2", xfwfFoldingTreeWidgetClass, tree1, NULL);
    tree3 = XtVaCreateManagedWidget
	("tree3", xfwfFoldingTreeWidgetClass, tree1, NULL);
    button3 = XtVaCreateManagedWidget
	("3", xfwfButtonWidgetClass, tree1, NULL);
    button4 = XtVaCreateManagedWidget
	("4", xfwfButtonWidgetClass, tree1, NULL);
    label2 = XtVaCreateManagedWidget
	("l2", xfwfLabelWidgetClass, tree2, NULL);
    button1 = XtVaCreateManagedWidget
	("1", xfwfButtonWidgetClass, tree2, XtNlabel, "# clicks: 0", NULL);
    button2 = XtVaCreateManagedWidget
	("2", xfwfButtonWidgetClass, tree2, XtNlabel, "# clicks: 0", NULL);
    label3 = XtVaCreateManagedWidget
	("l3", xfwfLabelWidgetClass, tree3, NULL);
    XtRealizeWidget(toplevel);
    XtAddCallback(tree1, XtNactivate, print_stat, NULL);
    XtAddCallback(tree2, XtNactivate, print_stat, NULL);
    XtAddCallback(tree3, XtNactivate, print_stat, NULL);
    XtAddCallback(button1, XtNactivate, activate, NULL);
    XtAddCallback(button2, XtNactivate, activate, NULL);
    XtAddCallback(button3, XtNactivate, quit, NULL);
    XtAddCallback(button4, XtNactivate, iconify, NULL);
    XtAppMainLoop(app_context);
}
