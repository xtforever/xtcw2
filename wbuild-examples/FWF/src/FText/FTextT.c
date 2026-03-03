#include <stdio.h>
#include <Xm/Xm.h>
#include <Xfwf/FText.h>
#include <Xfwf/FTextP.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static String fallback_resources[] = {
    "FTextT.geometry: 200x300",
    "FTextT*background:	#ff92aa",
    NULL,
};

static Widget toplevel, ftext;
static XtAppContext app_context;


/* ARGSUSED */
static void activate(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
}


int main(argc, argv)
    int argc;
    String *argv;
{
    Pixel black;
    int i;
    TextStyle sty = FtNORMALSIZE;
    
    toplevel = XtVaAppInitialize(&app_context, "FTextT", NULL, 0,
				 &argc, argv, fallback_resources, NULL);
    ftext = XtVaCreateManagedWidget
	("ftext", xfwfFormattedTextWidgetClass, toplevel, NULL);
    XtRealizeWidget(toplevel);
    /* XtAddCallback(ftext, XtNactivate, activate, NULL); */

    black = BlackPixelOfScreen(XtScreen(ftext));
    XfwfAddParShape(ftext, /*left*/15, /*right*/15, /*width*/-1,
		    /*mode*/0, /*leading*/1.0);
    for (i = 0; i < 3; i++) {
	XfwfAddWord(ftext, "Dit", 3, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "is", 2, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "een", 3, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "test.", 5, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1500, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
    }
    XfwfAddVSpace(ftext, 50);
    XfwfAddHRule(ftext);
    XfwfAddVSpace(ftext, 50);
    XfwfAddParShape(ftext, 25, 25, -1, FtJUSTIFYCENTER, 1.0);
    XfwfAddWord(ftext, "Dit", 3, black, TRANSPARENT, sty, NULL);
    XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
    XfwfAddWord(ftext, "is", 2, black, TRANSPARENT, sty, NULL);
    XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
    XfwfAddWord(ftext, "nog", 3, black, TRANSPARENT, sty, NULL);
    XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
    XfwfAddWord(ftext, "een", 3, black, TRANSPARENT, sty, NULL);
    XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
    XfwfAddWord(ftext, "test.", 5, black, TRANSPARENT, sty, NULL);
    XfwfAddHSpace(ftext, 1500, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
    XfwfAddVSpace(ftext, 10);
    for (i = 0; i < 8; i++) {
	XfwfAddWord(ftext, "Dit", 3, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "is", 2, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "nog", 3, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "een", 3, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "test,", 5, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1500, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "met", 3, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "een", 3, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "tamelijk", 8, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "lange", 5, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1000, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
	XfwfAddWord(ftext, "lettercombinatie.", 17, black, TRANSPARENT, sty, NULL);
	XfwfAddHSpace(ftext, 1500, TRUE, TRUE, black, TRANSPARENT, sty, NULL);
    }
    XfwfAddWord(ftext, "!END!", 5, black, TRANSPARENT, sty, NULL);
    XfwfAddEOD(ftext);

    XtAppMainLoop(app_context);
}
