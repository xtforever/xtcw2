/*
 * Author:
 * 	Brian Totty
 * 	Department of Computer Science
 * 	University Of Illinois at Urbana-Champaign
 *	1304 West Springfield Avenue
 * 	Urbana, IL 61801
 * 
 * 	totty@cs.uiuc.edu
 * 	
 */ 

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/ShellP.h>
#include <X11/Shell.h>
#include <Xfwf/FileSel.h>

static XrmOptionDescRec options[] =
{
    {"-title",     "*HFS.title",             XrmoptionSepArg,NULL},
    {"-sortmode",  "*HFS.sortMode",          XrmoptionSepArg,NULL},
    {"-pattern",   "*HFS.pattern",           XrmoptionSepArg,NULL},
    {"-file",      "*HFS.currentFile",       XrmoptionSepArg,NULL},
    {"-dir",       "*HFS.currentDirectory",  XrmoptionSepArg,NULL},
    {"-nook",      "*HFS.showOkButton",      XrmoptionNoArg,"False"},
    {"-nocancel",  "*HFS.showCancelButton",  XrmoptionNoArg,"False"},
    {"-flaglinks", "*HFS.flagLinks",         XrmoptionNoArg,"True"}
};

void OkCallbackHandler(w,client_data,ret)
Widget w;
caddr_t client_data;
XfwfFileSelectorOkButtonReturnStruct *ret;
{
	printf("OK!\t\t\t");
	printf("Path = <%s>, File = <%s>, File Box = <%s>\n",
	       ret->path,ret->file,ret->file_box_text);
	XtDestroyApplicationContext(XtWidgetToApplicationContext(w));
	exit(0);
} /* End OkCallbackHandler */


void CancelCallbackHandler(w,client_data,call_data)
Widget w;
caddr_t client_data;
caddr_t call_data;
{
	printf("Cancel!\n");
	XtDestroyApplicationContext(XtWidgetToApplicationContext(w));
	exit(0);
} /* End CancelCallbackHandler */


void SelectionChangeCallbackHandler(w,client_data,ret)
Widget w;
caddr_t client_data;
XfwfFileSelectorSelectionChangeReturnStruct *ret;
{
	printf("Selection Change!\t");
	if (ret->file_selected == False)
		printf("No Items Selected!\n");
	    else
		printf("Path = <%s>, File = <%s>\n",ret->path,ret->file);
} /* End SelectionChangeCallbackHandler */



/*-----------------------------------------------------------*

	main(argc,argv)

	This is the main entry point of the fileSelector
	widget application program.

 *-----------------------------------------------------------*/

int main(argc,argv)
int argc;
char **argv;
{
	Widget top,popup,fs;
	XtAppContext app_context;
	Arg args[3];

#ifdef X11R3
	top = XtInitialize("main","FileSelectorTest",
			   options,XtNumber(options),&argc,argv);
#else
	top = XtAppInitialize(&app_context,"FileSelectorTest",
			      options,XtNumber(options),
			      &argc,argv,
			      NULL,NULL,ZERO);
	popup = XtCreatePopupShell("PopupShell",transientShellWidgetClass,
				   top,NULL,0);
#endif
	fs = XtCreateManagedWidget("HFS",xfwfFileSelectorWidgetClass,
				   top,args,0);

	XtAddCallback(fs,XtNokButtonCallback,
		(XtCallbackProc)OkCallbackHandler,NULL);
	XtAddCallback(fs,XtNcancelButtonCallback,
		(XtCallbackProc)CancelCallbackHandler,NULL);
	XtAddCallback(fs,XtNselectionChangeCallback,
		(XtCallbackProc)SelectionChangeCallbackHandler,NULL);

	XtRealizeWidget(top);
/*	XtPopup(popup,XtGrabExclusive);	*/
#ifdef X11R3
	XtMainLoop();
#else
	XtAppMainLoop(app_context);
#endif
	return(1);
} /* End main */
