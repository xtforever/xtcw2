#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/ThWheel2.h>
#include <X11/Xaw/Box.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static String fallback_resources[] = {
    "*box*background: aquamarine",

    /* The step and range are rather arbitrary in this program */
    "*wheel.maxValue: 255",
    "*wheel.step: 4",				/* To allow 1/2 and 1/4 steps */

    NULL,
};

static Widget toplevel, box, wheel;
static XtAppContext app_context;


static void print_it(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    int value;
    XfwfScrollInfo *info = (XfwfScrollInfo *) call_data;
    char s[20];

    XtVaGetValues(w, XtNvalue, &value, NULL);
    /* printf("(vpos = %f) value = %d\n", info->vpos, value); */

    sprintf(s, "#F000%02X", value);
    XtVaSetValues(box, XtVaTypedArg, XtNbackground, XtRString, s, sizeof(s),
		  NULL);
}

/*
 *  Main program
 */
void main(argc, argv)
    int argc;
    String *argv;
{
    toplevel = XtVaAppInitialize
	(&app_context, "ScrollbarT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);

    /* XSynchronize(XtDisplay(toplevel), True); */

    box = XtVaCreateManagedWidget
	("box", boxWidgetClass, toplevel, NULL);
    wheel = XtVaCreateManagedWidget
	("wheel", xfwfThumbWheel2WidgetClass, box, NULL);

    XtAddCallback(wheel, XtNscrollCallback, print_it, NULL);

    XtInstallAllAccelerators(toplevel, toplevel);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
