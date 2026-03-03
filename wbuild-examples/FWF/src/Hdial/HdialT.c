/*
 * Copyright 1991 John L. Cwikla
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of John L. Cwikla or
 * University of Illinois not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.  John L. Cwikla and University of Illinois make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * John L. Cwikla and University of Illinois disclaim all warranties with
 * regard to this software, including all implied warranties of
 * merchantability and fitness, in no event shall John L. Cwikla or
 * University of Illinois be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data or profits, whether in an action of contract, negligence or
 * other tortious action, arising out of or in connection with the use or
 * performance of this software.
 *
 * Author:
 * 	John L. Cwikla
 * 	Materials Research Laboratory Center for Computation
 * 	University Of Illinois at Urbana-Champaign
 *	104 S. Goodwin
 * 	Urbana, IL 61801
 * 
 * 	cwikla@uimrl7.mrl.uiuc.edu
*/ 

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Core.h>

#include <Xfwf/Hdial.h>

#include <stdio.h>
#include <math.h>

#define APPNAME "HdialTest"
#define APPCLASS "HdialTest"

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

main(argc, argv)
int argc;
char *argv[];
{
  Widget testWidget, toplevel;
  XtAppContext app;
  Display *theDisplay;

  XtToolkitInitialize();
  app = XtCreateApplicationContext();
	
  theDisplay = XtOpenDisplay (app, NULL, APPNAME, APPCLASS, 
    NULL, 0, (Cardinal*)&argc, argv);

  if (!theDisplay)
  {
    printf("%s: can't open display, exiting...", APPNAME);
    exit (0);
  }

  toplevel = XtAppCreateShell (APPNAME, APPCLASS,
		applicationShellWidgetClass, theDisplay, NULL, 0);

  testWidget = XtCreateManagedWidget("HdialWidget", hdialWidgetClass,
    toplevel, NULL, 0);

  XtRealizeWidget(toplevel);

  XtAddEventHandler(testWidget, ButtonPressMask, FALSE,
    (XtEventHandler)QuitIt, NULL);

  printf("Press mouse button 3 to exit.\n");

  XtAppMainLoop(app);
}
