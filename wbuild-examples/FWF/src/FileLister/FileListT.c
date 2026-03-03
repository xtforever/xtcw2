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
#include <Xfwf/FileList.h>

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
XfwfFileListerOkButtonReturnStruct *ret;
{
	int i;

	printf("OK!\t\t\t");

	printf("Total number of files is %d\n", ret->number_of_files );

    for ( i = 0; i < ret->number_of_files; i++ )
	{
		printf("File #%d: Path = <%s>, \n	File = <%s>\n", i,
			ret->paths[i], ret->files[i]);
	}

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
XfwfFileListerSelectionChangeReturnStruct *ret;
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

int main( argc, argv )
int argc;
char **argv;
{
	Widget top,popup,fs;
	XtAppContext app_context;
	Arg args[3];

	top = XtAppInitialize(&app_context,"FileListerTest",
			      options, XtNumber(options),
			      &argc,argv,
			      NULL,NULL,ZERO);
	popup = XtCreatePopupShell("PopupShell",transientShellWidgetClass,
				   top,NULL,0);
	fs = XtCreateManagedWidget("HFS", xfwfFileListerWidgetClass,
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
