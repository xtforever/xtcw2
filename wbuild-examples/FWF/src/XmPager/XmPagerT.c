#include <stdio.h>
#include <Xm/Xm.h>
#include <Xfwf/XmPager.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/PushBG.h>
#include <Xm/Label.h>
#include <Xm/FileSB.h>

static XtAppContext app_context;
static Widget toplevel, pager, board, quit, load, name;
static Widget buttonbar, chooser;

static String fallback_resources[] = {

    /* --- main window --- */

    "PagerT.geometry: 650x635",
    /* "PagerT.board*background: #FFDBAA", */
    "PagerT.board.verticalSpacing: 5",
    "PagerT.board.horizontalSpacing: 5",
    "PagerT*name.font: *-helvetica-bold-r-normal-*-180-*-iso8859-1",
    "PagerT*name.foreground: yellow",
    "PagerT.board*name.background: black",
    "PagerT.board.pager.background: #FBFBFF",
    "PagerT*pager.foreground: black",
    /* "*fontFamily: -*-courier-*-*-*-*-0-0-*-*-*-*-iso8859-1", */
    /* "*fontFamily: -*-helvetica-*-*-*-*-0-0-*-*-*-*-iso8859-1", */
    "PagerT*pager.fontFamily: *-times-*",
    "PagerT*pager.text: `ls -l",
    "PagerT*pager.lines: 25",
    "PagerT*quit.labelString: Quit",
    "PagerT*open.labelString: Open",
    "PagerT*buttonbar.orientation: horizontal",

    NULL,
};


/* quit_CB -- handle quit button in main window */
static void quit_CB(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    exit(0);
}


/* load_CB -- handle load button in main window */
static void load_CB(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XtManageChild(chooser);
}


/* cancel_CB -- handle cancel button file selector */
static void cancel_CB(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XtUnmanageChild(chooser);
}


/* ok_CB -- handle OK button in file selector */
static void ok_CB(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XmFileSelectionBoxCallbackStruct *info = 
	(XmFileSelectionBoxCallbackStruct *) call_data;
    char *s, t[1024];

    XtUnmanageChild(chooser);
    if (XmStringGetLtoR(info->value, XmFONTLIST_DEFAULT_TAG, &s)) {
	strcat(strcpy(t, "`expand "), s);
	XtVaSetValues(pager, XtNtext, t, NULL);
	XtVaSetValues(name, XtNlabel, s, NULL);
    }
}



int main(argc, argv)
    int argc;
    char *argv[];
{
    toplevel = XtVaAppInitialize(&app_context, "PagerT", NULL, 0,
				 &argc, argv, fallback_resources,
				 NULL);
#if 0
    XSynchronize(XtDisplay(toplevel), True);
#endif
#if EDITRES
    XtAddEventHandler(toplevel, (EventMask) 0, True,
		      _XEditResCheckMessages, NULL);
#endif

    if (argc > 2) {
	fprintf(stderr, "Usage: %s [file]\n", argv[0]);
	exit(1);
    }

    printf("\n\
\tThis is a test program for the XmPager widget.\n\
\n\
\tIf you start the program without arguments\n\
\tit displays a directory listing, otherwise\n\
\tit shows the contents of the file that is given\n\
\tas argument.\n\
\n");

    /*
     * The main window contains:
     *
     *   XfwfRowColumn "board"
     *     XfwfPager "pager"
     *     XfwfButton "quit"
     *     XfwfButton "open"
     */
    board = XtVaCreateManagedWidget
	("board", xmFormWidgetClass, toplevel, 0);
    name = XtVaCreateManagedWidget
	("name", xmLabelWidgetClass, board,
	 XmNtopAttachment, XmATTACH_FORM,
	 XmNleftAttachment, XmATTACH_FORM,
	 XmNrightAttachment, XmATTACH_FORM,
	 NULL);
    buttonbar = XtVaCreateManagedWidget
	("buttonbar", xmRowColumnWidgetClass, board,
	 XmNleftAttachment, XmATTACH_FORM,
	 XmNrightAttachment, XmATTACH_FORM,
	 XmNbottomAttachment, XmATTACH_FORM,
	 NULL);
    quit = XtVaCreateManagedWidget
	("quit", xmPushButtonGadgetClass, buttonbar, NULL);
    load = XtVaCreateManagedWidget
	("open", xmPushButtonGadgetClass, buttonbar, NULL);
    pager = XtVaCreateManagedWidget
	("pager", xmPagerWidgetClass, board,
	 XmNtopAttachment, XmATTACH_WIDGET, XmNtopWidget, name,
	 XmNtopAttachment, XmATTACH_WIDGET, XmNtopWidget, name,
	 XmNleftAttachment, XmATTACH_FORM,
	 XmNrightAttachment, XmATTACH_FORM,
	 XmNbottomAttachment, XmATTACH_WIDGET, XmNbottomWidget, buttonbar,
	 NULL);

    XtAddCallback(quit, XmNactivateCallback, quit_CB, NULL);
    XtAddCallback(load, XmNactivateCallback, load_CB, NULL);

    /*
     * File selection box
     */
    chooser = XmCreateFileSelectionDialog(toplevel, "chooser", NULL, 0);
    XtUnmanageChild(XmFileSelectionBoxGetChild(chooser, XmDIALOG_HELP_BUTTON));
    XtAddCallback(chooser, XmNokCallback, ok_CB, NULL);
    XtAddCallback(chooser, XmNcancelCallback, cancel_CB, NULL);
	
    /*
     * If there is a command line argument, interpret it as the file
     * to display, otherwise use the fileChooser widget to get a file
     * name.
     */
    if (argc == 2) {
	char t[1024];
	strcat(strcpy(t, "`expand "), argv[1]);
	XtVaSetValues(name, XtNlabel, argv[1], NULL);
	XtVaSetValues(pager, XtNtext, t, NULL);
    }

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
