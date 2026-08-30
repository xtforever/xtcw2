/*
 * FChooserT.c : Test program for the FileChooser widget
 *
 * George Ferguson, ferguson@cs.rochester.edu, 21 Jan 1993.
 *
 * This program illustrates use of a FileChooser together with "Ok"
 * and "Cancel" buttons, and with a Text widget for new filenames.
 * Messages are printed to stdout whenever something is selected in
 * in the FileChooser, or prior to exiting after selecting one of the
 * buttons. Typing <Return> in the Text window is equivalent to
 * clicking on "Ok". Note that it is the responsability of the program
 * to keep the FileChooser and Text widget in sync -- the FileChooser
 * knows nothing about buttons or Text widgets, deliberately.
 * I added a menu for changing the sort mode, and another Text item
 * for changing the filter pattern. I guess that makes this a bit overly
 * complicated, but I wanted to show everything.
 */
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/AsciiText.h>
#include <Xfwf/FChooser.h>

static void selectionChangedCallback();
static void okCallback(),cancelCallback();
static void sortModeCallback();
static void okAction();
static void filterAction();

static XtAppContext appContext;
static Widget toplevel;
static Widget chooser;
static Widget filterText,fileText;

static XtActionsRec actionTable[] = {
    { "ok",	okAction },
    { "filter",	filterAction },
};

static char *sortMenuItemNames[] = {
    "None", "Name", "Name, dirs first",
    "Size, ascending", "Size, descending",
    "Access, ascending", "Access, descending",
};

main(argc,argv)
int argc;
char **argv;
{
    Widget form,button,menu,menuItem,label;
    Arg args[4];
    int i;

    toplevel = XtAppInitialize(&appContext,"FileChooserTest",
			       NULL, 0,&argc,argv,NULL,NULL,0);
    XtAppAddActions(appContext,actionTable,XtNumber(actionTable));

    form = XtCreateManagedWidget("form",formWidgetClass,toplevel,NULL,0);

    XtSetArg(args[0],XtNwidth,300);
    chooser = XtCreateManagedWidget("chooser",xfwfFileChooserWidgetClass,
				    form,args,1);
    XtAddCallback(chooser,XtNcallback,selectionChangedCallback,NULL);

    XtSetArg(args[0],XtNlabel,"Ok");
    XtSetArg(args[1],XtNfromHoriz,chooser);
    XtSetArg(args[2],XtNvertDistance,40);
    XtSetArg(args[3],XtNborderWidth,3);
    button = XtCreateManagedWidget("ok",commandWidgetClass,form,args,4);
    XtAddCallback(button,XtNcallback,okCallback,NULL);

    XtSetArg(args[0],XtNlabel,"Cancel");
    XtSetArg(args[1],XtNfromHoriz,chooser);
    XtSetArg(args[2],XtNfromVert,button);
    button = XtCreateManagedWidget("cancel",commandWidgetClass,form,args,3);
    XtAddCallback(button,XtNcallback,cancelCallback,NULL);

    menu = XtCreatePopupShell("sortMenu",simpleMenuWidgetClass,form,NULL,0);
    for (i = 0; i < XtNumber(sortMenuItemNames); i++) {
	XtSetArg(args[0],XtNlabel,sortMenuItemNames[i]);
	menuItem = XtCreateManagedWidget("sortMenuItem",smeBSBObjectClass,
					 menu,args,1);
        XtAddCallback(menuItem,XtNcallback,sortModeCallback,(XtPointer)i);
    }

    XtSetArg(args[0],XtNlabel,"Sort Mode");
    XtSetArg(args[1],XtNfromHoriz,chooser);
    XtSetArg(args[2],XtNfromVert,button);
    XtSetArg(args[3],XtNmenuName,"sortMenu");
    button = XtCreateManagedWidget("sort",menuButtonWidgetClass,form,args,4);

    XtSetArg(args[0],XtNlabel,"Filter:");
    XtSetArg(args[1],XtNborderWidth,0);
    XtSetArg(args[2],XtNfromVert,chooser);
    label = XtCreateManagedWidget("filterLabel",labelWidgetClass,form,args,3);

    XtSetArg(args[0],XtNfromVert,chooser);
    XtSetArg(args[1],XtNfromHoriz,label);
    XtSetArg(args[2],XtNwidth,300);
    XtSetArg(args[3],XtNeditType,XawtextEdit);
    filterText = XtCreateManagedWidget("filterText",asciiTextWidgetClass,
				       form,args,4); 
    XtOverrideTranslations(filterText,
	     XtParseTranslationTable("<Key>Return: filter()"));

    XtSetArg(args[0],XtNlabel,"File:");
    XtSetArg(args[1],XtNborderWidth,0);
    XtSetArg(args[2],XtNfromVert,label);
    label = XtCreateManagedWidget("fileLabel",labelWidgetClass,form,args,3);

    XtSetArg(args[0],XtNfromVert,filterText);
    XtSetArg(args[1],XtNfromHoriz,label);
    XtSetArg(args[2],XtNwidth,300);
    XtSetArg(args[3],XtNeditType,XawtextEdit);
    fileText = XtCreateManagedWidget("fileText",asciiTextWidgetClass,
				     form,args,3);
    XtOverrideTranslations(fileText,
	     XtParseTranslationTable("<Key>Return: ok()"));

    XtRealizeWidget(toplevel);
    XtAppMainLoop(appContext);
    /*NOTREACHED*/
}

/*
 * CALLBACKS:
 */

/*ARGSUSED*/
static void
selectionChangedCallback(w,client_data,call_data)
Widget w;
XtPointer client_data;	/* not used */
XtPointer call_data;	/* return struct */
{
    XfwfFileChooserReturnStruct *ret =
	(XfwfFileChooserReturnStruct *)call_data;
    Arg args[1];

    if (ret->directory == NULL || ret->file == NULL) {
	printf("No selection\n");
	XtSetArg(args[0],XtNstring,"");
    } else {
	printf("Directory: \"%s\"\n",ret->directory);
	printf("File: \"%s\"\n",ret->file);
	XtSetArg(args[0],XtNstring,ret->file);
    }
    XtSetValues(fileText,args,1);
}

/*ARGSUSED*/
static void
okCallback(w,client_data,call_data)
Widget w;
XtPointer client_data;	/* not used */
XtPointer call_data;	/* not used */
{
    Arg args[1];
    char *dir,*file;

    printf("Ok!\n");
    /* Get directory from chooser (or use XfwfFileChooserCurrentDirectory()) */
    XtSetArg(args[0],XtNcurrentDirectory,&dir);
    XtGetValues(chooser,args,1);
    /* But get file from text item (might differ from XtNcurrentFile) */
    XtSetArg(args[0],XtNstring,&file);
    XtGetValues(fileText,args,1);
    /* Sanity check */
    if (dir == NULL || !*dir || file == NULL || !*file) {
	printf("You need to select something before saying Ok!\n");
	return;
    }
    /* Everything ok */
    printf("Directory: \"%s\"\n",dir);
    printf("File: \"%s\"\n",file);
    /* Say goodbye */
    XtDestroyApplicationContext(appContext);
    exit(0);
}

/*ARGSUSED*/
static void
cancelCallback(w,client_data,call_data)
Widget w;
XtPointer client_data;	/* not used */
XtPointer call_data;	/* not used */
{
    printf("Cancel!\n");
    XtDestroyApplicationContext(appContext);
    exit(0);
}

/*ARGSUSED*/
static void
sortModeCallback(w,client_data,call_data)
Widget w;
XtPointer client_data;	/* not used */
XtPointer call_data;	/* item index == sort mode */
{
    int sortMode = (int)client_data;
    Arg args[1];

    printf("Setting sort mode to %s\n",sortMenuItemNames[sortMode]);
    XtSetArg(args[0],XtNsortMode,sortMode);
    XtSetValues(chooser,args,1);
}

/*
 * ACTIONS:
 */

/*ARGSUSED*/
static void
okAction(w,event,params,num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
    okCallback(NULL,NULL,NULL);
}

/*ARGSUSED*/
static void
filterAction(w,event,params,num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
    Arg args[1];
    char *s;

    XtSetArg(args[0],XtNstring,&s);
    XtGetValues(filterText,args,1);
    printf("Setting filter pattern to \"%s\"\n",s);
    XtSetArg(args[0],XtNpattern,s);
    XtSetValues(chooser,args,1);
}
