/* AnimT -- test program for animator widget */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Animator.h>
#include <Xfwf/Board.h>
#include <Xfwf/Label.h>
#include <Xfwf/Button.h>
#include <X11/xpm.h>

#include "on.xpm"
#include "off.xpm"

/*
 * The images for "anim" are given as resources, while the images for
 * "anim2" are #included.
 */
static String fallback_resources[] = {
    "*board.width: 400",			/* Overall size */
    "*board.height: 300",
    "*board*background: aquamarine",		/* Global color */
    "*board.frameWidth: 2",
    "*anim.images: Anim00.xpm Anim05.xpm Anim10.xpm Anim15.xpm\
	Anim20.xpm Anim25.xpm Anim30.xpm Anim35.xpm Anim40.xpm Anim45.xpm\
	Anim50.xpm Anim55.xpm Anim60.xpm Anim65.xpm Anim70.xpm Anim75.xpm",
    "*anim.intervals: 400 400 400 0",
    "*anim.frameWidth: 2",
    "*anim.outerOffset: 4",
    "*label.location: 1.0-150 50 140 70",
    "*label.label: Press button\\nto quit",
    "*label.foreground: yellow",
    "*button.location: 1.0-120 1.0-55 110 45",
    "*anim2.location: 10 0.5-15 24 24",
    "*anim2.frameWidth: 4",
    "*anim2.frameType: ledged",
    "*anim2.traversalOn: False",
    NULL,
};

static Widget toplevel, board, anim, button, anim2, label;
static XtAppContext app_context;
static XImage *images[17], *images2[3];

static void quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    exit(0);
}

int main(argc, argv)
    int argc;
    String *argv;
{
    XImage *dummy;

    printf("\n\tIf you get warnings about images that could not be found,\n");
    printf("\tit means that you didn't start the program from the\n");
    printf("\tright directory. If all goes well, you should see a\n");
    printf("\tplanet in orbit.\n\n");

    toplevel = XtVaAppInitialize
	(&app_context, "AnimT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);
    board = XtVaCreateManagedWidget
	("board", xfwfBoardWidgetClass, toplevel, NULL);
    button = XtVaCreateManagedWidget
	("button", xfwfButtonWidgetClass, board, NULL);
    anim2 = XtVaCreateManagedWidget
	("anim2", xfwfAnimatorWidgetClass, button, NULL);
    anim = XtVaCreateManagedWidget
	("anim", xfwfAnimatorWidgetClass, board, NULL);
    label = XtVaCreateManagedWidget
	("label", xfwfLabelWidgetClass, board, NULL);

    (void) XpmCreateImageFromData(XtDisplay(toplevel), on,
				  &images2[0], &dummy, NULL);
    (void) XpmCreateImageFromData(XtDisplay(toplevel), off,
				  &images2[1], &dummy, NULL);
    images2[2] = NULL;
    XtVaSetValues(anim2, XtNimages, images2, NULL);

    XtAddCallback(button, XtNactivate, quit, NULL);
    XfwfStartAnimation(anim);
    XfwfStartAnimation(anim2);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
