#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <Xfwf/Pager.h>
#include <Xfwf/Board.h>
#include <Xfwf/Button.h>
#include <Xfwf/FChooser.h>
#include <Xfwf/Prompt.h>
#if EDITRES
#include <Xmu/Editres.h>
#endif

static XtAppContext app_context;
static Widget toplevel, pager, board, quit, load, name;
static Widget shell, board2, ok, cancel, fileText, label, chooser;

static String fallback_resources[] = {

    /* --- main window --- */

    "PagerT.board.width: 650",
    "PagerT.board.height: 600",
    "PagerT*background: bisque",
    "PagerT*name.location: 5 5 1.0-10 25",
    "PagerT*name.font: *-helvetica-bold-r-normal-*-180-*-iso8859-1",
    "PagerT*name.foreground: yellow",
    "PagerT*name.background: black",
    "PagerT*pager.location: 5 35 1.0-10 1.0-95",
    "PagerT*pager.background: #FBFBFF",
    "PagerT*pager.foreground: black",
    "PagerT*pager.frameWidth: 2",
    /* "*fontFamily: -*-courier-*-*-*-*-0-0-*-*-*-*-iso8859-1", */
    /* "*fontFamily: -*-helvetica-*-*-*-*-0-0-*-*-*-*-iso8859-1", */
    "PagerT*fontFamily: *-times-*",
    "PagerT*text: `ls -l",
    "PagerT*lines: 25",
    "PagerT*quit.location: 0.5-110 1.0-40 100 30",
    "PagerT*quit.label: Quit",
    "PagerT*open.location: 0.5+10 1.0-40 100 30",
    "PagerT*open.label: Open",

    /* --- pop-up file selector box --- */

    "*box.location: 0 0 330 300",
    "*box*background: pink",
    "*chooser.x: 10",
    "*chooser.y: 10",
    "*chooser.width: 200",
    "*chooser.height: 230",
    "*ok.label: OK",
    "*ok.location: 220 0.5-40 100 30",
    "*cancel.label: Cancel",
    "*cancel.location: 220 0.5+10 100 30",
    "*fileLabel.label: File:",
    "*fileLabel.location: 0 250 0.2-5 30",
    "*fileText.location: 0.2 250 0.8-10 30",

    NULL,
};


static void quit_CB(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    exit(0);
}


static void load_CB(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XtPopup(shell, XtGrabExclusive);
}


static void cancel_CB(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XtPopdown(shell);
}


static void ok_CB(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    char *dir, *file, s[1024];

    /* Get directory from chooser (or use XfwfFileChooserCurrentDirectory()) */
    /* But get file from text item (might differ from XtNcurrentFile) */
    XtVaGetValues(chooser, XtNcurrentDirectory, &dir, NULL);
    XtVaGetValues(fileText, XtNtext, &file, NULL);

    /* Sanity check */
    if (dir != NULL && *dir && file != NULL && *file) {	/* Everything ok */
	XtPopdown(shell);
	strcpy(s, "`expand ");		/* Expand tabs */
	strcat(s, dir);
	strcat(s, file);
	/* fprintf(stderr, "Selected OK: %s\n", s); */
	XtVaSetValues(pager, XtNtext, s, NULL);
	XtVaSetValues(name, XtNlabel, file, NULL);
    }
}


static void selection_CB(w, client_data, call_data)
    Widget w;
    XtPointer client_data;	/* not used */
    XtPointer call_data;	/* return struct */
{
    XfwfFileChooserReturnStruct *ret =
	(XfwfFileChooserReturnStruct *)call_data;

    /* fprintf(stderr, "Selection change: %s\n", ret->file); */
    if (ret->directory == NULL || ret->file == NULL)
	XtVaSetValues(fileText, XtNtext, "", NULL);
    else
	XtVaSetValues(fileText, XtNtext, ret->file, NULL);
}


void main(argc, argv)
    int argc;
    char *argv[];
{
    toplevel = XtVaAppInitialize(&app_context, "PagerT", NULL, 0,
				 &argc, argv, fallback_resources,
				 NULL);
    XSynchronize(XtDisplay(toplevel), True);
#if EDITRES
    XtAddEventHandler(toplevel, (EventMask) 0, True,
		      _XEditResCheckMessages, NULL);
#endif

    if (argc > 2) {
	fprintf(stderr, "Usage: %s [file]\n", argv[0]);
	exit(1);
    }

    printf("\n\
\tThis is a test program for the Pager widget.\n\
\n\
\tIf you start the program without arguments\n\
\tit displays a directory listing, otherwise\n\
\tit shows the contents of the file that is given\n\
\tas argument.\n\
\n");

    /*
     * The main window contains:
     *
     *   XfwfBoard "board"
     *     XfwfPager "pager"
     *     XfwfButton "quit"
     *     XfwfButton "open"
     */
    board = XtVaCreateManagedWidget("board", xfwfBoardWidgetClass, toplevel,0);
    name = XtVaCreateManagedWidget("name", xfwfLabelWidgetClass, board,NULL);
    pager = XtVaCreateManagedWidget("pager", xfwfPagerWidgetClass, board,NULL);
    quit = XtVaCreateManagedWidget("quit", xfwfButtonWidgetClass, board, NULL);
    load = XtVaCreateManagedWidget("open", xfwfButtonWidgetClass, board, NULL);

    XtAddCallback(quit, XtNactivate, quit_CB, NULL);
    XtAddCallback(load, XtNactivate, load_CB, NULL);

    /*
     * A pop-up shell is used for the FileChooser widget:
     *
     *  Board "box"
     *    XfwfFileChooser "chooser"
     *    XfwfButton "ok"
     *    XfwfButton "cancel"
     */
    shell = XtVaCreatePopupShell("Choose a file", transientShellWidgetClass,
				 toplevel, NULL);
    board2 = XtVaCreateManagedWidget("box", xfwfBoardWidgetClass, shell, NULL);
    chooser = XtVaCreateManagedWidget("chooser", xfwfFileChooserWidgetClass,
				    board2, NULL);
    ok = XtVaCreateManagedWidget("ok", xfwfButtonWidgetClass, board2, NULL);
    cancel = XtVaCreateManagedWidget("cancel", xfwfButtonWidgetClass,board2,0);
    label = XtVaCreateManagedWidget("fileLabel", xfwfLabelWidgetClass,
				    board2, NULL);
    fileText = XtVaCreateManagedWidget("fileText", xfwfPromptWidgetClass,
				       board2, NULL);
    /*
     * The ok_CB is bound both to the OK button and to the Return key
     * in the prompt.
     */
    XtAddCallback(ok, XtNactivate, ok_CB, NULL);
    XtAddCallback(fileText, XtNactivate, ok_CB, NULL);
    XtAddCallback(cancel, XtNactivate, cancel_CB, NULL);
    XtAddCallback(chooser, XtNcallback, selection_CB, NULL);

    XtRealizeWidget(toplevel);

    /*
     * If there is a command line argument, interpret it as the file
     * to display, otherwise use the fileChooser widget to get a file
     * name.
     */
    if (argc == 2) {
	char s[1024];
	strcpy(s, "`expand ");
	strcat(s, argv[1]);
	XtVaSetValues(name, XtNlabel, argv[1], NULL);
	XtVaSetValues(pager, XtNtext, s, NULL);
    }

    XtAppMainLoop(app_context);
}
