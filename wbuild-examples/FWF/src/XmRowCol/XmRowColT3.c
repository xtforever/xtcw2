#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>

#include "RowCol.h"

#define MAX 20

static XtAppContext app_context;
static Widget toplevel, toplevel2, tester, button, rowcol_button;


static void delete(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    Dimension width;
#if 0
    XtDestroyWidget(w);
#else

    XtVaGetValues(w,
		XmNwidth,	&width,
		NULL);

    /* printf("RowColT3:(%s,width) = %d\n", XtName(w),width); */

    if( width == 150)
        width = 200;
    else
	width = 150;

    XtVaSetValues(w,
		XmNwidth,	width,
		NULL);

#endif

}


static void activate(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    static int i = 0;
    char buffer[50];

    sprintf(buffer, "%d", i++);
 
    button = XtVaCreateManagedWidget(buffer, 
                xmPushButtonWidgetClass,       tester,
                XmNposition,			0,
		XmNwidth,			150,
		XmNheight,			50,
                NULL);

    XtAddCallback(button, XmNactivateCallback, delete, NULL);
}

/*
 * main -- main routine of RowColT
 *
 * Initialize toolkit, create a RowCol widget with MAX buttons of
 * different sizes and give each a label equal to its sequence number
 * 0 to MAX - 1.
 */
void main(argc, argv)
    int argc;
    char *argv[];
{
    int i, w, h;
    String s;
    char t[20];
    Widget sw;

    toplevel = XtVaAppInitialize(&app_context, "RowColT", NULL, 0,
				  &argc, argv, NULL, NULL);

    sw = XtVaCreateManagedWidget("sw", 
                xmScrolledWindowWidgetClass,       toplevel,
                NULL);

    tester = XtVaCreateManagedWidget("rowCol", 
                rowColWidgetClass,       sw,
                XmNstoreByRow,           True,
                NULL);

#if 1
    toplevel2 = XtCreatePopupShell("command",
		transientShellWidgetClass,	toplevel,
		NULL, 0);
#else
    toplevel2 = XtAppCreateShell( "command", "RowColT",
		applicationShellWidgetClass, XtDisplay(toplevel),
				  &argc, argv);

    toplevel2 = XtVaCreateManagedWidget("tl2", 
                applicationShellWidgetClass,       toplevel,
                NULL);
#endif

    button = XtVaCreateManagedWidget("PB", 
                xmPushButtonWidgetClass,       toplevel2,
		XmNwidth,			150,
		XmNheight,			50,
                NULL);

    XtAddCallback(button, XmNactivateCallback, activate, NULL);

    XtRealizeWidget(toplevel);

    XtRealizeWidget(toplevel2);
    XtPopup(toplevel2, XtGrabNone);

    XtAppMainLoop(app_context);
}

