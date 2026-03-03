/* IconT -- test file for Icon widget */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/RowCol.h>
#include <Xfwf/Icon.h>

static String fallback_resources[] = {
    "IconT.rowcol.width: 200",
    "IconT.rowcol.height: 300",
    /* All predefined icons: */
    "*icon1.image: FATAL",
    "*icon2.image: WARNING",
    "*icon3.image: ERROR",
    "*icon4.image: QUESTION",
    "*icon5.image: INFO",
    "*icon6.image: emptydiamond",
    "*icon7.image: filleddiamond",
    "*icon8.image: emptysquare",
    "*icon9.image: filledsquare",
    NULL,
};

static void quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    exit(0);
}

static Widget toplevel, rowcol, icon1, icon2, icon3, icon4,
    icon5, icon6, icon7, icon8, icon9, icon10;
static XtAppContext app_context;

main(argc, argv)
    int argc;
    String argv[];
{
    printf("If you call this program with the name of a pixmap file\n");
    printf("the program will display that pixmap as the tenth icon.\n");
    printf("Click on the skull to exit.\n");

    toplevel = XtVaAppInitialize
	(&app_context, "IconT", NULL, 0, &argc, argv,
	 fallback_resources, NULL);
    rowcol = XtVaCreateManagedWidget
	("rowcol", xfwfRowColWidgetClass, toplevel, NULL);
    icon1 = XtVaCreateManagedWidget
	("icon1", xfwfIconWidgetClass, rowcol, NULL);
    icon2 = XtVaCreateManagedWidget
	("icon2", xfwfIconWidgetClass, rowcol, NULL);
    icon3 = XtVaCreateManagedWidget
	("icon3", xfwfIconWidgetClass, rowcol, NULL);
    icon4 = XtVaCreateManagedWidget
	("icon4", xfwfIconWidgetClass, rowcol, NULL);
    icon5 = XtVaCreateManagedWidget
	("icon5", xfwfIconWidgetClass, rowcol, NULL);
    icon6 = XtVaCreateManagedWidget
	("icon6", xfwfIconWidgetClass, rowcol, NULL);
    icon7 = XtVaCreateManagedWidget
	("icon7", xfwfIconWidgetClass, rowcol, NULL);
    icon8 = XtVaCreateManagedWidget
	("icon8", xfwfIconWidgetClass, rowcol, NULL);
    icon9 = XtVaCreateManagedWidget
	("icon9", xfwfIconWidgetClass, rowcol, NULL);

    if (argc > 1)
	icon10 = XtVaCreateManagedWidget
	    ("icon10", xfwfIconWidgetClass, rowcol,
	     XtVaTypedArg, XtNimage, XtRString, argv[1], strlen(argv[1]) + 1,
	     NULL);

    XtAddCallback(icon1, XtNactivate, quit, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
