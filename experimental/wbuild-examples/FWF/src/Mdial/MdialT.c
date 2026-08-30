
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




#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Core.h>

#include <Xm/Xm.h>
#if 0
#include <Xm/Form.h>
#else
#include <Xm/PanedW.h>
#endif

#include "Mdial.h"

#include <stdio.h>
#include <math.h>

#define APPNAME "MdialTest"
#define APPCLASS "MdialTest"

static void hourFormatter(_w, _value, _toString)
Widget _w;
int _value;
char *_toString;
{
  sprintf(_toString, "%d Hours", _value);
}

static void minuteFormatter(_w, _value, _toString)
Widget _w;
int _value;
char *_toString;
{
  sprintf(_toString, "%d Minutes", _value);
}

static void secondFormatter(_w, _value, _toString)
Widget _w;
int _value;
char *_toString;
{
  sprintf(_toString, "%d Seconds", _value);
}


static void dialCB(_w, _data, _call)
Widget _w;
XtPointer _data;
XtPointer _call;
{
  MdialCallbackStruct *mcs;

  mcs = (MdialCallbackStruct *)_call;

  switch(mcs->reason)
  {
    case XmCR_INCREMENT:
      printf("Increment to: ");
      break;
    case XmCR_DECREMENT:
      printf("Decrement to: ");
      break;
    case XmCR_VALUE_CHANGED:
      printf("Value changed to: ");
      break;
  }

  printf("%d\n", mcs->value);
}

static void valueCheck(_w, _data, _call)
Widget _w;
XtPointer _data;
XtPointer _call;
{
  MdialCallbackStruct *mcs;

  mcs = (MdialCallbackStruct *)_call;

  if (mcs->value == 60)
  {
    MdialIncrement((Widget)_data, 1, TRUE);
    MdialSet(_w, 0, FALSE);
  }
}
  

static void QuitIt(_w, _nil, _event)
Widget _w;
caddr_t _nil;
XEvent *_event;
{
  if (_event->type == ButtonPress)
    if (((XButtonEvent *)_event)->button == 3)
    {
      printf("Have a nice day. --JLC\n");
      exit(1);
    }
}

static void timeOut(_data, _id)
XtPointer _data;
XtIntervalId *_id;
{
  XtAppContext app;
  Widget w;

  w = (Widget)_data;
  app = XtWidgetToApplicationContext(w);

  MdialIncrement((Widget)_data, -1, TRUE);

  XtAppAddTimeOut(app, 1000, timeOut, w);
}


main(argc, argv)
int argc;
char *argv[];
{
  Widget testWidget[3], row, toplevel;
  XtAppContext app;
  Display *theDisplay;
  Arg warg[8];
  int n, i;
  t_func dialFormatter[3];
  long tStamp;

  XtToolkitInitialize();
  app = XtCreateApplicationContext();
	
  theDisplay = XtOpenDisplay (app, NULL, APPNAME, APPCLASS, 
    NULL, 0, &argc, argv);

  if (!theDisplay)
  {
    printf("%s: can't open display, exiting...", APPNAME);
    exit (0);
  }

  toplevel = XtAppCreateShell (APPNAME, APPCLASS,
		applicationShellWidgetClass, theDisplay, NULL, 0);

#if 0
  row = XtCreateManagedWidget("container", xmFormWidgetClass, toplevel, NULL, 0);
#else
  row = XtCreateManagedWidget("container", xmPanedWindowWidgetClass, toplevel, NULL, 0);
#endif
  
  dialFormatter[0] = hourFormatter;
  dialFormatter[1] = minuteFormatter;
  dialFormatter[2] = secondFormatter;

  for(i=0;i<3;i++)
  {
    n = 0;
    XtSetArg(warg[n], XmNminimum, 0); n++;
    XtSetArg(warg[n], XmNmaximum, 60); n++;
    XtSetArg(warg[n], XmNlabelFormatter, dialFormatter[i]); n++;
    testWidget[i] = XtCreateManagedWidget("MdialWidget", mdialWidgetClass, row, warg, n);
  }

  n = 0;
  XtSetArg(warg[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(warg[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(warg[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetValues(testWidget[0], warg, n);

  n = 0;
  XtSetArg(warg[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(warg[n], XmNtopWidget, testWidget[0]); n++;
  XtSetArg(warg[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(warg[n], XmNbottomWidget, testWidget[2]); n++;
  XtSetArg(warg[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(warg[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetValues(testWidget[1], warg, n);

  n = 0;
  XtSetArg(warg[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(warg[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(warg[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetValues(testWidget[2], warg, n);

  XtAddCallback(testWidget[2], XmNincrementCallback, (XtCallbackProc)valueCheck, (XtPointer)testWidget[1]);
  XtAddCallback(testWidget[1], XmNincrementCallback, (XtCallbackProc)valueCheck, (XtPointer)testWidget[0]);
  XtAddCallback(testWidget[0], XmNincrementCallback, (XtCallbackProc)valueCheck, (XtPointer)testWidget[0]);

  XtAppAddTimeOut(app, 1000, timeOut, (XtPointer)testWidget[2]); 

  XtRealizeWidget(toplevel);

  XtAppMainLoop(app);
}
