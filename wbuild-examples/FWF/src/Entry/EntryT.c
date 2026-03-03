/* EntryT.c - demo for the Entry widget */
/* 9502 afo@pub.th-zwickau.de */

#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <Xfwf/Button.h>
#include <Xfwf/Entry.h>

#define MAX		3

static String resources[] = {

	"*board.width:				350",
	"*board.height:				200",
	"*foreground:				MediumBlue",
	"*background:				bisque",

	"*XfwfEntry*background:		Gray80",
	"*XfwfButton*foreground:	HotPink",

	"*label1.location:			0.10	0.10	0.25	0.15",
	"*label1.label:				Name",	
	"*label1.alignment:			right",
	"*entry1.location:			0.35	0.10	0.55	0.15",
	"*entry1.max:				20",
	
	"*label2.location:			0.10	0.30	0.25	0.15",
	"*label2.label:				Age",	
	"*label2.alignment:			right",
	"*entry2.location:			0.35	0.30	0.15	0.15",
	"*entry2.valid:				0123456789",
	"*entry2.max:				3",

	"*label3.location:			0.10	0.50	0.25	0.15",
	"*label3.label:				Password",	
	"*label3.alignment:			right",
	"*entry3.location:			0.35	0.50	0.30	0.15",
	"*entry3.max:				8",
	"*entry3.echo:				False",
	
	"*show.location:			0.10	0.80	0.20	0.15",
	"*show.label:				Show",
	"*clear.location:			0.40	0.80	0.20	0.15",
	"*clear.label:				Clear",
	"*quit.location:			0.70	0.80	0.20	0.15",
	"*quit.label:				Quit",
	
	NULL
};

static Widget top,board,label[MAX],entry[MAX],show,clear,quit;

static void ActivateCB(Widget w,XtPointer client_data,XtPointer call_data)
{
	String str=(String) call_data;
	
	fprintf(stderr,"activate(%s) with '%s'\n",XtName(w),str);
}

static void ShowCB(Widget w,XtPointer client_data,XtPointer call_data)
{
	String str;
	int i;
	
	for(i=0; i < MAX;i++)
	{
		XtVaGetValues(entry[i],XtNtext,&str,NULL);
		fprintf(stderr,"%s:\t'%s'\n",XtName(entry[i]),str);
	}
}

static void ClearCB(Widget w,XtPointer client_data,XtPointer call_data)
{	
	int i;
	
	for(i=0;i < MAX;i++)
		XfwfEntryClear(entry[i]);
}

static void QuitCB(Widget w,XtPointer client_data,XtPointer call_data)
{	
	fprintf(stderr,"exiting with the following values:\n");
	ShowCB(w,NULL,NULL);
	exit(0);
}

int main(int ac,char **av)
{
	int i;
	XtAppContext app_context;
	String labels[] = {"label1","label2","label3"};
	String entries[] = {"entry1","entry2","entry3"};
	
	top=XtVaAppInitialize(&app_context,"EntryT",NULL,0,&ac,av,resources,NULL);
	board=XtVaCreateManagedWidget("board",xfwfBoardWidgetClass,top,NULL);
	
	for(i=0;i < MAX;i++)
	{
		label[i]=XtVaCreateWidget(labels[i],xfwfLabelWidgetClass,board,NULL);
		entry[i]=XtVaCreateWidget(entries[i],xfwfEntryWidgetClass,board,NULL);
		XtAddCallback(entry[i],XtNactivate,ActivateCB,NULL);
	}
	XtManageChildren(label,MAX);
	XtManageChildren(entry,MAX);

	show=XtVaCreateManagedWidget("show",xfwfButtonWidgetClass,board,NULL);
	clear=XtVaCreateManagedWidget("clear",xfwfButtonWidgetClass,board,NULL);
	quit=XtVaCreateManagedWidget("quit",xfwfButtonWidgetClass,board,NULL);

	XtAddCallback(show,XtNactivate,ShowCB,NULL);
	XtAddCallback(clear,XtNactivate,ClearCB,NULL);
	XtAddCallback(quit,XtNactivate,QuitCB,NULL);

	XtRealizeWidget(top);
	XtAppMainLoop(app_context);
}
