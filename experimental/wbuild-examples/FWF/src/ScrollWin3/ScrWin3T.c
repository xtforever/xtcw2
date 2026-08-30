#include <stdlib.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/ScrWin3.h>
#include <Xfwf/MultiList.h>
#include <Xfwf/Label.h>

	static char *rowstrings[] =
        {
		"root",
		"rick",
		"bert",
		"brian",
		"fwf",
		"gene",
		"steve",
		"gone",
		"erik",
		"joe",
		"herman",
                NULL
        };

	static char *strings[] =
        {
		"0	0	System Administrator	/bin/sh",
		"400	1	Rick Richardson	/bin/ksh",
		"367	4	Bert Bos	/bin/ksh",
		"368	5	Brian Totty	/bin/ksh",
		"369	6	Free Widget Foundation	/bin/ksh",
		"364	2	Gene Olson	/bin/bash",
		"365	3	Steve Wahl	/bin/ksh",
		"370	7	Deleted User	/bin/csh",
		"371	7	Deleted User	/bin/sh",
		"372	7	Deleted User	/bin/rsh",
		"373	7	Herman Hermans	/bin/ksh",
		NULL
        };

        static Boolean sensitivities[] =
        {
                True,True,True,True,
                True,True,True,False,
		False,False,True
        };

static String fallback_resources[] = {
    "ScrWin3T.swindow.width: 300",
    "ScrWin3T.swindow.height: 210",
    "ScrWin3T*background: steelblue",
    "ScrWin3T*foreground: white",
    "ScrWin3T*list.borderWidth: 0",
    "ScrWin3T*list.tablist: 50 100 300",
    "ScrWin3T*list.border: green",
    "ScrWin3T*list.foreground: blue",
    "ScrWin3T*list.background: light yellow",
    "ScrWin3T*list.highlightForeground: blue",
    "ScrWin3T*list.highlightBackground: light blue",

    "ScrWin3T*swindow.colHdrHeight: 22",
    "ScrWin3T*swindow.rowHdrWidth: 50",
    "ScrWin3T*swindow.corner.label: login",
    "ScrWin3T*swindow.corner.alignment: top left",

    "ScrWin3T*swindow.colhdr.label: uid	gid	name	shell",
    "ScrWin3T*swindow.colhdr.background: blue",
    "ScrWin3T*swindow.colhdr.alignment: top left",
    "ScrWin3T*swindow.colhdr.tablist: 50 100 300",

    "ScrWin3T*swindow.rowhdr.shadeSurplus: False",
    "ScrWin3T*swindow.rowhdr.background: blue",
    NULL,
};

static Widget toplevel, swindow, corner, rowhdr, colhdr, list;
static XtAppContext app_context;

selected(w, client_data, cb)
Widget				w;
XfwfMultiListReturnStruct	*cb;
{
	Widget	ow;
	int	i;

	if (w == rowhdr) ow = list;
	else if (w == list) ow = rowhdr;
	else return;
	
	printf("selected:");
	XfwfMultiListUnhighlightAll( (XfwfMultiListWidget) ow);
	for (i = 0; i < cb->num_selected; ++i) {
		printf(" %d",cb->selected_items[i]);
		XfwfMultiListHighlightItem( (XfwfMultiListWidget) ow,
			cb->selected_items[i]);
	    }
	printf("\n");
}

int main(argc, argv)
    int argc;
    String *argv;
{
    int i;

    toplevel = XtVaAppInitialize
	(&app_context, "ScrWin3T", NULL, 0, &argc, argv,
	 fallback_resources, (String) NULL);

    swindow = XtVaCreateManagedWidget
	("swindow", xfwfScrolledWindow3WidgetClass, toplevel, NULL);

    corner = XtVaCreateManagedWidget("corner", xfwfLabelWidgetClass, swindow,
	NULL);

    colhdr = XtVaCreateManagedWidget("colhdr", xfwfLabelWidgetClass, swindow,
	NULL);

    rowhdr = XtVaCreateManagedWidget("rowhdr", xfwfMultiListWidgetClass,
	swindow,
	XtNlist,rowstrings,
	XtNsensitiveArray,sensitivities,
        XtNnumberStrings,0,
        XtNdefaultColumns,1,
        XtNborderWidth,0,
        XtNx,1,
        XtNy,1,
        XtNmaxSelectable,3,
	NULL);

    list = XtVaCreateManagedWidget("list", xfwfMultiListWidgetClass, swindow,
	XtNlist,strings,
	XtNsensitiveArray,sensitivities,
        XtNnumberStrings,0,
        XtNdefaultColumns,1,
        XtNborderWidth,0,
        XtNx,1,
        XtNy,1,
        XtNmaxSelectable,3,
	NULL);

    XtAddCallback(list, XtNcallback, (XtCallbackProc) selected, 0);
    XtAddCallback(rowhdr, XtNcallback, (XtCallbackProc) selected, 0);

    XtRealizeWidget(toplevel);

    /* Start with the windows scrolled halfway */
    XfwfScrollTo(swindow, 0.5, 0.5);

    XtAppMainLoop(app_context);
}
