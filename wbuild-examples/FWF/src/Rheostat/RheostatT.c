
/* Use -DEDITRES to enable EDITRES protocol;
 * you probably need X11R5 for this to work.
 * Under IRIX 4.0, which has an almost-R5-but-not-quite version 
 * of the toolkit, you need this too:
 */
 
#ifdef sgi 
#define _XEditResCheckMessages _EditResCheckMessages
#endif

/* Use -DMOTIF to use the Motif version.  This works under IRIX 4.0
 * and probably nowhere else.
 */

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#ifdef MOTIF
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#else
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#endif

#include <Xfwf/Rheostat.h>

#define APPCLASS "RheostatTest"

#ifdef EDITRES
extern void _XEditResCheckMessages();
#endif

static XrmOptionDescRec options[] = {
 { "-minvalue", "*minimumValue", XrmoptionSepArg, NULL },
 { "-maxvalue", "*maximumValue", XrmoptionSepArg, NULL },
 { "-minangle", "*minimumAngle", XrmoptionSepArg, NULL },
 { "-maxangle", "*maximumAngle", XrmoptionSepArg, NULL },
 { "-intervals", "*numberIntervals", XrmoptionSepArg, NULL },

 { "-outer", "*outerArrowLength", XrmoptionSepArg, NULL },
 { "-inner", "*innerArrowLength", XrmoptionSepArg, NULL },
 { "-width", "*arrowWidth", XrmoptionSepArg, NULL },

 { "-fill", "*fillArrow",   XrmoptionNoArg, "True"   },
 { "+fill", "*fillArrow",   XrmoptionNoArg, "False"   },
 { "-gravity", "*tickGravity",   XrmoptionNoArg, "True"   },
 { "+gravity", "*tickGravity",   XrmoptionNoArg, "False"   }
};

	/*ARGSUSED*/
static void GetRheostatValue(w, closure, call_data)
Widget w;
XtPointer closure;
XtPointer call_data;
{
    XfwfRheostatCallbackStruct *cd = (XfwfRheostatCallbackStruct *)call_data;

    printf("Rheostat value: %d\n",cd->value);
    return;
}

static void CopyRheostatValue(w, closure, call_data)
Widget w;
XtPointer closure;
XtPointer call_data;
{
    Widget otherWidget = (Widget)closure;
    XfwfRheostatCallbackStruct *cd = (XfwfRheostatCallbackStruct *)call_data;

    XtVaSetValues(otherWidget, XtNvalue, cd->value, NULL);
    return;
}

	/*ARGSUSED*/
static void QuitCallback(w, closure, call_data)
Widget w;
XtPointer closure;
XtPointer call_data;
{
	exit(0);
}

int main(argc, argv)
int argc;
char *argv[];
{
    Display		*display;
    Widget		toplevel;
    Widget		form,dial1,dial2,quit;
    char		*progname;
    XtAppContext	app_context;

    /*
     * Initialize toolkit and open display.
     */
    progname = argv[0];
    {
	char *cp;
	while (cp=strchr(progname,'/'))
	    progname = cp+1;
    }
    XtToolkitInitialize();
    app_context = XtCreateApplicationContext();
    display = XtOpenDisplay(
	app_context, NULL, progname, APPCLASS,
	options, XtNumber(options),
	&argc, argv);

    if (!display) {
	char *s;
	s = getenv("DISPLAY");
	fprintf(stderr,"%s: Can't open display %s\n", progname,
		s ? s : "-- $DISPLAY not set");
	exit(1);
    }

    /*
     * Command-line processing:
     */
    if (argc > 1) {
	fprintf(stderr,"%s: Bad option %s\n", argv[0], argv[1]);
	exit(1);
    }

    toplevel = XtVaAppCreateShell(
	progname, APPCLASS, applicationShellWidgetClass, display,
	XtNargv, argv,
	XtNargc, argc,
	NULL);

    /*
    XtGetApplicationResources(
	toplevel, (XtPointer) &app_defaults,
	resources, XtNumber(resources),
	(ArgList)NULL,0);
    */

#ifdef EDITRES
    XtAddEventHandler(toplevel, 0, True, _XEditResCheckMessages, NULL);
#endif

    /*
     * Create widget hierarchy:
     */
#ifdef MOTIF
    form = XtCreateManagedWidget("form",xmFormWidgetClass,toplevel,NULL,0);
#else
    form = XtCreateManagedWidget("form",formWidgetClass,toplevel,NULL,0);
#endif
    dial1 = XtCreateManagedWidget("dial",xfwfRheostatWidgetClass,form,NULL,0);
    dial2 = XtCreateManagedWidget("dial",xfwfRheostatWidgetClass,form,NULL,0);
#ifdef MOTIF
    quit = XtCreateManagedWidget("quit",xmPushButtonWidgetClass,form,NULL,0);
    XtVaSetValues(quit, XtVaTypedArg, XmNlabelString,
		  XtRString, "Quit", 5, NULL);
#else
    quit = XtCreateManagedWidget("quit",commandWidgetClass,form,NULL,0);
    XtVaSetValues(quit, XtNlabel, "Quit", NULL);
#endif

    /*
     * Geometry:
     */
#ifdef MOTIF
    XtVaSetValues(dial2, XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, dial1, NULL);
    XtVaSetValues(quit,  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, dial2, NULL);
#else
    XtVaSetValues(dial2, XtNfromVert, dial1, NULL);
    XtVaSetValues(quit, XtNfromVert, dial2, NULL);
#endif
    
    /*
     * Callbacks:
     */
    XtAddCallback(dial1, XtNnotify, GetRheostatValue, NULL);
    /* Link the two rheostat values: */
    XtAddCallback(dial1, XtNsetCallback, CopyRheostatValue, (XtPointer)dial2);
    XtAddCallback(dial2, XtNsetCallback, CopyRheostatValue, (XtPointer)dial1);

#ifdef MOTIF
    XtAddCallback(quit,XmNactivateCallback,QuitCallback,0);
#else
    XtAddCallback(quit,XtNcallback,QuitCallback,0);
#endif
    
    /*
     * Go for it:
     */
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}

