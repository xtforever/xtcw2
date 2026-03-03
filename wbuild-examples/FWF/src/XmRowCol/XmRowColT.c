#if 0

========================================================================
RowCol Widget : 

October 1995/Marc Quinton/STNA/DGAC/France.
email : quinton@stna7.stna.dgac.fr
Web   : http://www.stna7.stna.dgac.fr/~quinton/motif/7su/RowCol/
        http://www.stna7.stna.dgac.fr/~quinton/

Require : X11R4/R5, Motif1.1 or Motif 1.2
Tested  : on SunOs5.2/X11R5/Motif1.2
          and HPUX9.0/X11R5/Motif1.2


Thank to Bert Bos <bert@let.rug.nl> for his RowCol widget which
this one comes from. He has done main of the work. This widget
is translated from Rowcol.w code and adapted to Motif includes
and libraries.


The RowCol widget aligns all its children in rows and columns.
They are aligned on a grid that is coarse enough to
hold the largest of the children.
he position of each children can be manage with a new ressource
named XmNPosition. This ressource is a constraint ressource and is
inherited from parent (RowCol) to child.
 
Here is a simple example to see how it works :


        #include <Xm.h>
        #include "RowCol.h"

        ...

        /* Create RowCol Widget container */
        rc = XtVaCreateManagedWidget( "rowcol"
             rowColWidgetClass,       parent,
             NULL);


        /* insert one child : the first one */
        child1 = XtVaCreateManagedWidget( "child1"
             someWidgetClass,       rc,
             NULL);

        /* insert to the end of the list */
        child2 = XtVaCreateManagedWidget( "child2"
             someWidgetClass,       rc,
             XmNposition,           0,    /* Last position */
             NULL);


        /* insert to position 2 */
        child3 = XtVaCreateManagedWidget( "child3"
             someWidgetClass,       rc,
             XmNposition,           2,    /* position 2*/
             NULL);

        ...


You can get position of a child width this code :


        Position pos;

        XtVaGetValues(child3
             XmNposition,           &pos,    /* get position */
             NULL);

        /* pos = 2 */


You can set position of a child width this code :

        Position pos;

        XtVaSetValues(child3
             XmNposition,           &pos,    /* get position */
             NULL);


Send problemes to quinton@stna7.stna.dgac.fr
General GNU licencing can be applied to this code :

 . no garanty, if your computer explode because of this widget, this is not my
   probleme.
 . you can not make money with this code,
 . if you make significant changes just tell me,
 . you are free to redistribute or to change this program 
   Just leave this notice copyright.
 
========================================================================

#endif

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>

#include "XmRowCol.h"

#define MAX 20

static XtAppContext app_context;
static Widget toplevel, tester, sub[MAX], quit_button, rowcol_button;
Boolean byRow = True;

static String fallback_resources[] = {
    "RowColT.tester.width: 500",
    "RowColT.tester.height: 500",

    "RowColT*borderWidth: 0",
    "RowColT*background: gray50",
    "RowColT*foreground: white",

    "RowColT*tester.shrinkToFit: True",
    "RowColT*tester.width: 400",
    "RowColT*tester.height: 300",

    "RowColT*tester.XfwfButton.frameWidth: 2",

    NULL,
};

/*
 * activate -- callback for all buttons
 *
 * The buttons are numbered 0 to MAX, the number of the activated
 * button becomes the value of the `columns' resource of the RowCol
 * widget.
 */
static void activate(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    int i;

    i = 0;
    while (w != sub[i]) i++;
    XtVaSetValues(tester, XmNcolumns, i, NULL);
}


/*
 * quit -- callback for quit button
 */
static void quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    exit(0);
}


/*
 * rowcol -- callback for rowcol button
 *
 * toggle between store by row and store by column
 */
static void rowcol(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    byRow = ! byRow;
    XtVaSetValues(tester, XmNstoreByRow, byRow, NULL);
}


/*
 * main -- main routine of RowColT
 *
 * Initialize toolkit, create a RowCol widget with MAX buttons of
 * different sizes and give each a label equal to its sequence number
 * 0 to MAX - 1.
 */
int main(argc, argv)
    int argc;
    char *argv[];
{
    int i, w, h;
    String s;
    char t[20];

    toplevel = XtVaAppInitialize(&app_context, "RowColT", NULL, 0,
				  &argc, argv, fallback_resources, NULL);

    tester = XtVaCreateManagedWidget("tester", 
                rowColWidgetClass,       toplevel,
                XmNstoreByRow,           byRow,
                NULL);

    /*
     * Create MAX buttons
     */
    for (i = 0, w = 80 + MAX, h = 23; i < MAX; i++, w -= 1, h += 1) {
	(void) sprintf(t, "%d", i);
	s = XtNewString(t);
	sub[i] = XtVaCreateManagedWidget(s, 
                xmPushButtonWidgetClass, tester,
                XmNlabelString,          XmStringCreateSimple(s),
                XmNwidth,                w,
                XmNheight,               h,
                NULL);

	XtAddCallback(sub[i], XmNactivateCallback, activate, NULL);
    }
    /*
     * Create a button to toggle between `by row' and `by column'
     */
    rowcol_button = XtVaCreateManagedWidget("rowcol", 
                xmPushButtonWidgetClass,    tester,
                XmNlabelString,             XmStringCreateSimple("row/col"),
                XmNwidth,                   w, 
                XmNheight,                  h,
                NULL);

    XtAddCallback(rowcol_button, XmNactivateCallback, rowcol, NULL);

    /*
     * Create a quit button
     */
    w -= 1; h += 1;
    quit_button = XtVaCreateManagedWidget("quit", 
                xmPushButtonWidgetClass,    tester,
                XmNlabelString,             XmStringCreateSimple("quit"),
                XmNwidth,                   w, 
                XmNheight,                  h,
                NULL);

    XtAddCallback(quit_button, XmNactivateCallback, quit, NULL);
    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
