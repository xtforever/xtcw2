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

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <Xfwf/ScrList.h>

extern void exit();


void CallbackHandler(w,client_data,call_data)
Widget w;
caddr_t client_data;
XfwfScrolledListReturnStruct *call_data;
{
	printf("String = '%s'\n",call_data->string);
	printf("Index = %d\n",call_data->index);
}


/*-----------------------------------------------------------*

	main(argc,argv)

	This is the main entry point of the scrolledList
	widget application program.

 *-----------------------------------------------------------*/

#define NUM_STRINGS	16

int main(argc,argv)
int argc;
char **argv;
{
	int i;
	Widget top;
	Arg args[10];
	char buffer[64];
	XfwfScrolledListWidget sw;
	static char *strings[NUM_STRINGS];
	Boolean sensitivities[NUM_STRINGS];

	for (i = 0; i < NUM_STRINGS; i++)
        {
		sprintf(buffer,"String %d",i);
		strings[i] = (char *)malloc(strlen(buffer) + 1);
		strcpy(strings[i],buffer);
		sensitivities[i] = (i % 4 != 3);
	}

	top = XtInitialize("main","XScrolledList",NULL,0,
		&argc,argv);

	XtSetArg(args[0],XtNlist,strings);
	XtSetArg(args[1],XtNnumberStrings,NUM_STRINGS);
	XtSetArg(args[2],XtNsensitiveArray,sensitivities);
        XtSetArg(args[3],XtNwidth,135);
        XtSetArg(args[4],XtNheight,100);

	sw = (XfwfScrolledListWidget)XtCreateManagedWidget("scrolledList",
		xfwfScrolledListWidgetClass,top,args,5);
	XtAddCallback((Widget)sw,XtNcallback,
		(XtCallbackProc)CallbackHandler,NULL);

	XtRealizeWidget(top);
	XtMainLoop();
	return(1);
} /* End main */
