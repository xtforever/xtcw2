#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/ThWheel.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Label.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static String fallback_resources[] = {
    "*XfwfThumbWheel.maxValue: 255",		/* Color range 00-FF */
    "*XfwfThumbWheel.step: 4",			/* To allow 1/2 and 1/4 steps */
    NULL,
};

static Widget toplevel, box, wheel_R, wheel_G, wheel_B, label;
static XtAppContext app_context;
static XColor color;
static Display *display;
static Colormap colormap;

static void set_background(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XfwfScrollInfo *info = (XfwfScrollInfo *) call_data;
    int value;
    char s[20];

    XtVaGetValues(w, XtNvalue, &value, NULL);

    if (w == wheel_R) color.red = value << 8;
    else if (w == wheel_G) color.green = value << 8;
    else if (w == wheel_B) color.blue = value << 8;

    XFreeColors(display, colormap, &color.pixel, 1, 0);
    if (XAllocColor(display, colormap, &color)) {
	XtVaSetValues(box, XtNbackground, color.pixel, NULL);
	sprintf(s, "#%04X%04X%04X\n", color.red, color.green, color.blue);
	XtVaSetValues(label, XtNlabel, s, NULL);
    } else {
	fprintf(stderr, "Could not allocate color %s\n", s);
    }
}

/*
 *  Main program
 */
int main(argc, argv)
    int argc;
    String *argv;
{
    char s[20];

    toplevel = XtVaAppInitialize
	(&app_context, "ScrollbarT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);

    XSynchronize(XtDisplay(toplevel), True);

    display = XtDisplay(toplevel);
    colormap = DefaultColormapOfScreen(XtScreen(toplevel));
 
    box = XtVaCreateManagedWidget
	("box", boxWidgetClass, toplevel, NULL);
    label = XtVaCreateManagedWidget
	("label", labelWidgetClass, box, NULL);
    wheel_R = XtVaCreateManagedWidget
	("wheel_R", xfwfThumbWheelWidgetClass, box, NULL);
    wheel_G = XtVaCreateManagedWidget
	("wheel_G", xfwfThumbWheelWidgetClass, box, NULL);
    wheel_B = XtVaCreateManagedWidget
	("wheel_B", xfwfThumbWheelWidgetClass, box, NULL);

    XtAddCallback(wheel_R, XtNscrollCallback, set_background, NULL);
    XtAddCallback(wheel_G, XtNscrollCallback, set_background, NULL);
    XtAddCallback(wheel_B, XtNscrollCallback, set_background, NULL);

    /* Initialize background color */
    color.red = color.green = color.blue = 0;
    XAllocColor(display, colormap, &color);
    XtVaSetValues(box, XtNbackground, color.pixel, NULL);
    sprintf(s, "#%04X%04X%04X\n",color.red, color.green, color.blue);
    XtVaSetValues(label, XtNlabel, s, NULL);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
