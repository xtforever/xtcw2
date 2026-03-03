#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Group.h>
#include <Xfwf/Button.h>
#include <Xfwf/Toggle.h>

static XtAppContext app_context;
static Widget toplevel, board, group, t[6], quit_button;

static String fallback_resources[] = {
    "*board.width: 500",			/* Overall size */
    "*board.height: 400",

    "GroupT*borderWidth: 0",			/* Global */
    "GroupT*background: firebrick",

    /*
     * The next 3 resources keep the group a fixed width, but the
     * height is calculated from the total height of the children.
     */
    "GroupT*group.location: 0.5-130 20 260 280",
    "GroupT*group.columns: 1",
    "GroupT*group.shrinkToFit: True",
    "GroupT*group.label: \\ Choose one:\\ ",
    "GroupT*group.foreground: yellow",
    "GroupT*group.font: *-helvetica-bold-r-*-140-*",
    "GroupT*group.outerOffset: 8",
    "GroupT*group.innerOffset: 8",
    "GroupT*group.frameWidth: 4",

    /*
     * The position of the toggle buttons is set by the group widget,
     * but the width and height are acknowledged. The buttons will
     * have (by default) maximum width and be 40 pixels high.
     */
    "GroupT*XfwfToggle.abs_height: 40",
    "GroupT*XfwfToggle.rel_height: 0.0",
    "GroupT*XfwfToggle.outerOffset: 1",
    "GroupT*XfwfToggle.innerOffset: 3",
    "GroupT*XfwfToggle.alignment: left",

    "GroupT*0.label: orange",
    "GroupT*1.label: apple",
    "GroupT*2.label: melon",
    "GroupT*3.label: grapefruit",
    "GroupT*4.label: strawberry",
    "GroupT*5.label: banana",

    "GroupT*quit.abs_height: 40",
    "GroupT*quit.rel_height: 0.0",
    "GroupT*quit.outerOffset: 1",
    "GroupT*quit.label: Quit",

    NULL,
};


static void quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    long selection;
    String s;

    XtVaGetValues(group, XtNselection, &selection, NULL);
    if (selection == -1L)
	printf("Nothing was selected\n");
    else {
	XtVaGetValues(t[selection], XtNlabel, &s, NULL);
	printf("The final selection was: %s\n", s);
    }
    exit(0);
}


int main(argc, argv)
    int argc;
    char *argv[];
{
    toplevel = XtVaAppInitialize(&app_context, "GroupT", NULL, 0,
				 &argc, argv, fallback_resources, NULL);
    board = XtVaCreateManagedWidget("board", xfwfBoardWidgetClass,
				    toplevel, NULL);
    group = XtVaCreateManagedWidget("group", xfwfGroupWidgetClass,
				    board, NULL);
    t[0] = XtVaCreateManagedWidget("0", xfwfToggleWidgetClass, group, NULL);
    t[1] = XtVaCreateManagedWidget("1", xfwfToggleWidgetClass, group, NULL);
    t[2] = XtVaCreateManagedWidget("2", xfwfToggleWidgetClass, group, NULL);
    t[3] = XtVaCreateManagedWidget("3", xfwfToggleWidgetClass, group, NULL);
    t[4] = XtVaCreateManagedWidget("4", xfwfToggleWidgetClass, group, NULL);
    t[5] = XtVaCreateManagedWidget("5", xfwfToggleWidgetClass, group, NULL);
    quit_button = XtVaCreateManagedWidget("quit", xfwfButtonWidgetClass,
					  group, NULL);
    XtAddCallback(quit_button, XtNactivate, quit, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
