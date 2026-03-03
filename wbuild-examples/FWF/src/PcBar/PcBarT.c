#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>	

#include <X11/Xaw/Label.h>	
#include <X11/Xaw/Toggle.h>	
#include <X11/Xaw/Command.h>	
#include <X11/Xaw/Repeater.h>	
#include <X11/Xaw/Form.h>	
#include <X11/Xaw/Cardinals.h>	
#include <X11/StringDefs.h>     /* Standard Name-String definitions */
#include <Xfwf/PcBar.h>

Widget 	toplevel, 
		form, 
		testpc,
		vtestpc, 
		incbutton, 
		decbutton,
		pctoggle, 
		quitbutton;
static int currentpc=0;

/*ARGSUSED*/
void byebye(w, client_data, call_data) 
	Widget w;
	XtPointer client_data;
	XtPointer call_data;
	{
    exit(0); 
    }

/*ARGSUSED*/
void toggle_pc(w, client_data, call_data) 
	Widget w;
	XtPointer client_data;
	XtPointer call_data;
	{
	Boolean a= (Boolean) call_data;

	XtVaSetValues(testpc, XtNdisplaypc, a, NULL);
	XtVaSetValues(vtestpc, XtNdisplaypc, a, NULL);
    }

/*ARGSUSED*/
void incpc(w, client_data, call_data) 
	Widget w;
	XtPointer client_data;
	XtPointer call_data;
	{
	currentpc++;
	if (currentpc>100) currentpc=100;
	XfwfPcBarSetPercentage(testpc, currentpc);
	XfwfPcBarSetPercentage(vtestpc, currentpc);
    }

/*ARGSUSED*/
void decpc(w, client_data, call_data) 
	Widget w;
	XtPointer client_data;
	XtPointer call_data;
	{
	currentpc--;
	if (currentpc<0) currentpc=0;
	XfwfPcBarSetPercentage(testpc, currentpc);
	XfwfPcBarSetPercentage(vtestpc, currentpc);
    }

int main(argc, argv)
	int argc;
	char *argv[]; {
    XtAppContext app_con;

    toplevel = XtVaAppInitialize(
		&app_con,
		"PcBarTest", 
		NULL, 0,
		&argc, argv, 
		NULL, 
		NULL);

    form = XtVaCreateManagedWidget(
        "form",         /* widget name */
        formWidgetClass,        /* widget class */
        toplevel,       /* parent widget*/
		XtNwidth, 200,
        NULL);  


    testpc=XtVaCreateManagedWidget(
		"testpc", 
		xfwfPcBarWidgetClass, 
		form, 
		XtNshowzero, True,
		NULL);
    vtestpc=XtVaCreateManagedWidget(
		"vtestpc", 
		xfwfPcBarWidgetClass, 
		form, 
		XtNshowzero, True,
		XtNvertical, True,
		XtNfromVert, testpc,
		NULL);
    incbutton=XtVaCreateManagedWidget(
		"Increment", 
		repeaterWidgetClass, 
		form, 
		XtNfromVert, vtestpc,
		NULL);
    decbutton=XtVaCreateManagedWidget(
		"Decrement", 
		repeaterWidgetClass, 
		form, 
		XtNfromVert, incbutton,
		NULL);
    pctoggle=XtVaCreateManagedWidget(
		"pctoggle", 
		toggleWidgetClass, 
		form, 
		XtNfromVert, decbutton,
		XtNlabel, "Show % Display",
		NULL);
    quitbutton=XtVaCreateManagedWidget(
		"Quit", 
		commandWidgetClass, 
		form, 
		XtNfromVert, pctoggle,
		NULL);

    XtAddCallback(incbutton, XtNcallback, incpc, 0);                          
    XtAddCallback(decbutton, XtNcallback, decpc, 0);                          
    XtAddCallback(quitbutton, XtNcallback, byebye, 0);                          
    XtAddCallback(pctoggle, XtNcallback, toggle_pc, 0);                          
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_con);
	}

