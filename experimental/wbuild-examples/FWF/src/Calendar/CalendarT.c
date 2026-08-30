#include <stdio.h>
#include <stdlib.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>

#include <Xfwf/Calendar.h>

static XtAppContext app;

Widget top,cal, form, boxit, wrapit, headit, quit;

static String fallback_resources[] = {
	"*.headingColor: blue",
	"*.weekdayColor: steel blue",
	"*.headingFont: -adobe-times-medium-r-*-*-18-*-*-*-*-*-*-*",
	"*.calendarFont: -adobe-times-medium-r-*-*-12-*-*-*-*-*-*-*",
	"*.boxColor: purple",
	"*.color2: red",
	"*.background: bisque",
	"*.innerOffset: 4",
	"*.frameWidth: 3",
	NULL
	};

static int colors[32] = {1};

void changeCB(Widget w, XtPointer client_data, XtPointer call_data) {
	XfwfDate *dt = (XfwfDate*) call_data;
	int i;

	printf("New month : month=%d, year=%d\n", dt->month, dt->year);
	for (i=0; i<32; i++)
		colors[i]=1;
	XfwfSetDayColors(cal, colors);
	}

void calCB(Widget w, XtPointer client_data, XtPointer call_data) {
	XfwfDate *dt = (XfwfDate*) call_data;

	printf("You clicked on : day=%d, month=%d, year=%d\n", dt->day, dt->month, dt->year);
	if (colors[dt->day] & XfwfReverseDate)
		colors[dt->day] = 2;
	else
		colors[dt->day] = 2|XfwfReverseDate;
	XfwfSetDayColors(cal, colors);
	}

void boxitCB(Widget w, XtPointer client_data, XtPointer call_data) {
	Boolean state = (Boolean) ((int) call_data);
	XtVaSetValues(cal, XtNboxed, state, NULL);
	}

void wrapitCB(Widget w, XtPointer client_data, XtPointer call_data) {
	Boolean state = (Boolean) ((int) call_data);
	XtVaSetValues(cal, XtNwrapped, state, NULL);
	}

void headitCB(Widget w, XtPointer client_data, XtPointer call_data) {
	Boolean state = (Boolean) ((int) call_data);
	XtVaSetValues(cal, XtNheading, state, NULL);
	}

void quitit(void) {
	exit(0);
	}

int main(int argc, char *argv[]) {

	top = XtVaAppInitialize(&app,"CalendarT",NULL,0,&argc,argv,fallback_resources, NULL);

	form = XtVaCreateManagedWidget("form", formWidgetClass, top, NULL);
	cal = XtVaCreateManagedWidget("cal", xfwfCalendarWidgetClass, form,	
			XtNborderWidth , 0,
			NULL);
	boxit = XtVaCreateManagedWidget("Boxed", toggleWidgetClass, form, 
			XtNfromVert, cal, NULL);
	wrapit = XtVaCreateManagedWidget("Wrapped", toggleWidgetClass, form, 
			XtNfromHoriz, boxit, 
			XtNfromVert, cal, 
			NULL);
	headit = XtVaCreateManagedWidget("Heading", toggleWidgetClass, form, 
			XtNfromVert, boxit, 
			XtNstate, True,
			NULL);
	quit = XtVaCreateManagedWidget("Quit", commandWidgetClass, form, 
			XtNfromHoriz, headit, 
			XtNfromVert, boxit, 
			NULL);

	XtAddCallback(cal, XtNdateCallback, (XtCallbackProc) calCB, NULL);
	XtAddCallback(cal, XtNchangeCallback, (XtCallbackProc) changeCB, NULL);
	XtAddCallback(boxit, XtNcallback, (XtCallbackProc) boxitCB, NULL);
	XtAddCallback(wrapit, XtNcallback, (XtCallbackProc) wrapitCB, NULL);
	XtAddCallback(headit, XtNcallback, (XtCallbackProc) headitCB, NULL);
	XtAddCallback(quit, XtNcallback, (XtCallbackProc) quitit, NULL);

	XtRealizeWidget(top);
	XtAppMainLoop(app);
	return(1);
} /* End main */
