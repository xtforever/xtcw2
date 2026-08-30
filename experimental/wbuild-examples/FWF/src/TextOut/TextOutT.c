#include <stdio.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <Xfwf/ScrollWin.h>
#include <Xfwf/TextOut.h>

static XtAppContext app;

static String fallback_resources[] = {
	"*.background: grey",
	"*.textout.frameWidth: 0",
	"*.font1: -adobe-times-medium-r-normal--12-*-*-*-p-*-iso8859-1",
	"*.font2: -adobe-times-medium-i-normal--12-*-*-*-p-*-iso8859-1",
	"*.font3: -adobe-helvetica-medium-r-normal--12-*-*-*-p-*-iso8859-1",
	"*.font4: -adobe-helvetica-bold-r-normal--12-*-*-*-p-*-iso8859-1",
	"*.color1: black",
	"*.color2: red",
	"*.color3: green",
	"*.color4: blue",
	NULL
};

/*---------------------------------------------------------------------------*

	main(argc,argv)

	This is the main entry point of the scrolledList
	widget application program.

 *---------------------------------------------------------------------------*/

int main(argc,argv)
int argc;
char **argv;
{
	Widget top,textOut,scr;

	top = XtVaAppInitialize(&app,"TextOutT",NULL,0,&argc,argv,fallback_resources, NULL);

	scr = XtVaCreateManagedWidget("scroll", xfwfScrolledWindowWidgetClass,
					top, XtNwidth, 500, XtNheight, 700, NULL);
	textOut = XtVaCreateManagedWidget("textout", xfwfTextOutWidgetClass,
					scr, NULL);

	XtRealizeWidget(top);

	XfwfAddText(textOut, "fred", 1, 1, 0);
	XfwfAddText(textOut, "fred", 2, 2, 0);
	XfwfAddText(textOut, "fred", 3, 3, 0);
	XfwfAddText(textOut, "fred", 4, 4, 1);
	XfwfAddText(textOut, "", 4, 4, 1);
	XfwfAddText(textOut, "Big long text to make sure that line sizin is OK", 4, 4, 1);

	XfwfAddText(textOut, NULL, 0, 0, 0);

	XtAppMainLoop(app);
	return(1);
} /* End main */
