/*
 * Copyright 1993 John L. Cwikla
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of John L. Cwikla or
 * Wolfram Research, Inc not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.  John L. Cwikla and Wolfram Research, Inc make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * John L. Cwikla and Wolfram Research, Inc disclaim all warranties with
 * regard to this software, including all implied warranties of
 * merchantability and fitness, in no event shall John L. Cwikla or
 * Wolfram Research, Inc be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data or profits, whether in an action of contract, negligence or
 * other tortious action, arising out of or in connection with the use or
 * performance of this software.
 *
 * Author:
 *  John L. Cwikla
 *  X Programmer
 *  Wolfram Research Inc.
 *
 *  cwikla@wri.com
*/

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Core.h>

#if MOTIF
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>

#define containerWidgetClass xmRowColumnWidgetClass
#define buttonWidgetClass xmPushButtonWidgetClass
#define labelWidgetClass xmLabelWidgetClass
#define activateCallback XmNactivateCallback

#else
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>

#define containerWidgetClass boxWidgetClass
#define buttonWidgetClass commandWidgetClass
#define labelWidgetClass commandWidgetClass
#define activateCallback XtNcallback
#endif

#include <stdio.h>

#include "Stack.h"

#define CLASS_NAME "XStacker"
#define NAME "XStacker"

int Gravities[] = 
	{	NorthWestGravity, NorthGravity, NorthEastGravity, EastGravity, SouthEastGravity, 
		SouthGravity, SouthWestGravity, WestGravity, CenterGravity };

void NextCB(_w, _data, _call)
Widget _w;
XtPointer _data;
XtPointer _call;
{
	Widget stackWidget;

	stackWidget = (Widget)_data;

	XfwfStackNextWidget(stackWidget);
}

void PrevCB(_w, _data, _call)
Widget _w;
XtPointer _data;
XtPointer _call;
{
	Widget stackWidget;

	stackWidget = (Widget)_data;

	XfwfStackPreviousWidget(stackWidget);
}

int main(argc, argv)
int argc;
char *argv[];
{
	Display *display;
	XtAppContext appContext;
	XEvent event;
	char buffer[10];
	Widget toplevel, stack;
	Widget rc, buttonN, buttonP;
	int i, n;
	Arg warg[2];
	int theGravity;

	XtToolkitInitialize();
	appContext = XtCreateApplicationContext();

	display = XtOpenDisplay(appContext, NULL, NAME, CLASS_NAME,
		NULL, 0, &argc, argv);

	if (!display)
	{
		fprintf(stderr, "%s: can't open display, exiting...\n", CLASS_NAME);
		exit(0);
	}

	toplevel = XtAppCreateShell(NAME, CLASS_NAME, applicationShellWidgetClass, display, NULL, 0);

	rc = XtCreateManagedWidget("rowColumn", containerWidgetClass, toplevel, NULL, 0);

	n = 0;
	buttonN = XtCreateManagedWidget("Next", buttonWidgetClass, rc, warg, n);

	n = 0;
	buttonP = XtCreateManagedWidget("Previous", buttonWidgetClass, rc, warg, n);

	stack = XtCreateManagedWidget("stack", xfwfStackWidgetClass, rc, NULL, 0);

	XtAddCallback(buttonN, activateCallback, NextCB, (XtPointer)stack);
	XtAddCallback(buttonP, activateCallback, PrevCB, (XtPointer)stack);


	theGravity = 0;
	for(i=0;i<20;i++)
	{
		sprintf(buffer, "label %d", i);	

		n = 0;
		XtSetArg(warg[n], XfwfNgravity, Gravities[theGravity]); n++;
		XtCreateManagedWidget(buffer, labelWidgetClass, stack, warg, n);
		theGravity = (theGravity == XtNumber(Gravities)) ? 0 : theGravity + 1;
	}

	XtRealizeWidget(toplevel);

	while(TRUE)
	{
		XEvent event;

		XtAppNextEvent(appContext, &event);
		XtDispatchEvent(&event);
	}
}
