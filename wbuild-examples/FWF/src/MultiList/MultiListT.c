/*****************************************************************************

	MultiList_test.c

	This file contains the source for a very simple MultiList widget
	tester program.

	July 1990, Brian Totty

 ****************************************************************************/

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

#include <Xfwf/MultiList.h>

/*---------------------------------------------------------------------------*

	void CallbackHandler(w,client_data,rs)

	This callback handler gets called when the MultiList widget gets a
	highlight or unhighlight action.

 *---------------------------------------------------------------------------*/

void CallbackHandler(w,client_data,rs)
Widget w;
caddr_t client_data;
XfwfMultiListReturnStruct *rs;
{
	int i;

	static char *multiList_action_names[] =
	{
		"MULTILIST_ACTION_NOTHING",
		"MULTILIST_ACTION_HIGHLIGHT",
		"MULTILIST_ACTION_UNHIGHLIGHT",
		"MULTILIST_ACTION_OPEN",
		"MULTILIST_ACTION_STATUS"
	};

	printf("-----------------------------------\n");
	printf("Num Selected:        %d\n",rs->num_selected);
	printf("Selected Items:      ");
	printf("\n");
	printf("Most Recent Action:  %s\n",multiList_action_names[rs->action]);
	printf("Most Recent Item:    %d\n",rs->item);
	printf("Most Recent String:  '%s'\n",
	       (rs->string == NULL ? "NULL" : rs->string));
	for (i = 0; i < rs->num_selected; i++)
	{
		printf("%d ",rs->selected_items[i]);
	}
	if (rs->num_selected == 0) printf("None");
	printf("\n");
} /* End CallbackHandler */


/*---------------------------------------------------------------------------*

	main(argc,argv)

	This is the main entry point of the scrolledList
	widget application program.

 *---------------------------------------------------------------------------*/

int main(argc,argv)
int argc;
char **argv;
{
	Widget top,multiList;
	Arg args[10];

	static char *strings[8] =
	{
		"Happy New Year",
		"To You",
		"Your Family",
		"And Friends",
		"Peace And Joy",
		"Best Wishes For 1993!",
		"(A Token Unsensitive String)",
		NULL
	};

	static Boolean sensitivities[7] =
	{
		True,True,True,True,
		True,True,False
	};

	top = XtInitialize("main","XMultiList",NULL,0,&argc,argv);

	XtSetArg(args[0],XtNlist,strings);
	XtSetArg(args[1],XtNsensitiveArray,sensitivities);
	XtSetArg(args[2],XtNnumberStrings,0);
	XtSetArg(args[3],XtNdefaultColumns,2);
	XtSetArg(args[4],XtNborderWidth,0);
	XtSetArg(args[5],XtNx,1);
	XtSetArg(args[6],XtNy,1);
	XtSetArg(args[7],XtNmaxSelectable,3);
	multiList = XtCreateManagedWidget("multiList",
		  xfwfMultiListWidgetClass,top,args,8);

	XtAddCallback(multiList,XtNcallback,CallbackHandler,NULL);

	XtRealizeWidget(top);
	XtMainLoop();
	return(1);
} /* End main */



