#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/PushB.h>
#include <Xm/BulletinB.h>
#include <Xm/ScrolledW.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>

#include "RowCol.h"

#ifdef DEBUG
#include "RowColP.h"
#endif

#define MAX 50

static XtAppContext app_context;
static Widget toplevel, rowCol, quit_button, rowcol_button;
Boolean byRow = True;


static Widget _popupMenu = NULL;


static String fallback_resources[] = {
    "RowColT.rowCol.width: 500",
    "RowColT.rowCol.height: 500",

    "RowColT*rowCol.shrinkToFit: True",
    "RowColT*rowCol.storeByRow:  True", 
    "RowColT*rowCol.width: 400",
    "RowColT*rowCol.height: 300",

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
    int i, pos;

    XtVaGetValues(w,
	XmNuserData,	&pos,
	/* XmNposition,	&pos, */
	NULL);

    XtVaSetValues(rowCol, 
        XmNcolumns,          pos, 
        NULL);
}

Widget create_buttons(Widget parent, int position, String name )
{
        Widget bb, command, sub;
        static void popupmenu();


        bb = XtVaCreateManagedWidget("bb", 
                xmBulletinBoardWidgetClass,     parent,
		XmNwidth,                       200,
		XmNheight,			33,
		XmNborderWidth,			2,
		XmNmarginWidth,			0,
		XmNmarginHeight,		0,
		XmNnoResize,			True,
                XmNposition,			position,
		XmNuserData,			position,
                NULL);


	sub = XtVaCreateManagedWidget(name, 
                xmPushButtonWidgetClass,       bb,
                XmNlabelString,                XmStringCreateSimple(name),
                XmNwidth,                      30,
                XmNheight,                     30,
                XmNx,				5,
                XmNy,				3,
		XmNuserData,			position,
                NULL);

	XtAddCallback(sub, XmNactivateCallback, activate, NULL);

        command = XtVaCreateManagedWidget( "command", 
                xmPushButtonWidgetClass,       bb,
                XmNlabelString,                XmStringCreateSimple("command ?"),
                XmNwidth,                      150,
                XmNheight,                     30,
                XmNx,				50,
                XmNy,				3,
                NULL);

	XtAddCallback(command, XmNarmCallback,  popupmenu, NULL);
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

static Widget _target;

static void do_it(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    int what_to_do = (int) client_data;
    int position = -2;

    XtVaGetValues(_target,
        XmNposition,            &position,
        NULL);

    /* printf("target = %s (%d)\n", XtName(_target), position); */

    if(position == -1)
       position = 0;

    switch (what_to_do)
    {
    case 0:
        /* info */
        {
            char name[200];
            Widget child;
            RowColWidget rc;
            int i, pos;
            Position x, y ;

            printf("position = %d,\n", position);

            rc = (RowColWidget) XtParent(_target);
            for(i = 0 ; i < rc->composite.num_children ; i++)
            {
                child = rc->composite.children[i];
                XtVaGetValues( child,
                   XmNposition,      &pos,
                   XmNx,             &x,
                   XmNy,             &y,
                   NULL);
                printf("children[%d] = %4X = (%3d,%3d) = %d\n",  i, child, x, y, pos);
            }
        }
        break;


    case 1:
        /* ajouter */
        {
            char name[200];

            sprintf(name, "N %d", position);

            create_buttons(rowCol, position, name );
        }
        break;

    case 2:
        /* supprimer */
        XtDestroyWidget(_target);
        break;

    case 3:
        /* Monter */
        XtVaSetValues(_target,
	    XmNposition,        position -1,
            NULL);
        break;

    case 4:
        /* Descendre */
        XtVaSetValues(_target,
	    XmNposition,        position +1,
            NULL);
        break;

    case 5:
        /* ajouter 3 */
        create_buttons(rowCol, 3, "X");
        break;

    case 6:
        /* ajouter 0 */
        create_buttons(rowCol, 0, "Y");
        break;

    case 7:
        /* deplacer 5 */

        XtVaSetValues(_target,
	    XmNposition,        5,
            NULL);

        break;

    case 8:
        /* deplacer 0 : fin */
        XtVaSetValues(_target,
	    XmNposition,        0,
            NULL);
        break;

    case 9:
        /* ajouter position -1 */
        create_buttons(rowCol, -1, "-1");
        break;

    case 10:
        /* ajouter position 50 : fin */
        create_buttons(rowCol, 50, "50");
        break;

    case 11:
        /* deplacer -1 : fin */
        XtVaSetValues(_target,
	    XmNposition,        -1,
            NULL);
        break;

    case 12:
        /* deplacer 50 : fin */
        XtVaSetValues(_target,
	    XmNposition,        50,
            NULL);
        break;

    case 13:
        /* deplacer 50 : fin */
        XtVaSetValues(_target,
	    XmNposition,        0,
            NULL);
        break;

    case 14:
        /* deplacer 1 : fin */
        XtVaSetValues(_target,
	    XmNposition,        1,
            NULL);
        break;

    }
}


Widget create_popupMenu(Widget toplevel)
{
    Widget          popupMenu, bp;

    popupMenu = XmCreatePopupMenu(toplevel, "popup", NULL,0);

    bp = XtCreateManagedWidget("info ?",   xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 0);

    bp = XtCreateManagedWidget("ajouter",   xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 1);

    bp = XtCreateManagedWidget("supprimer", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 2);

    bp = XtCreateManagedWidget("Monter",     xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 3);

    bp = XtCreateManagedWidget("Descendre", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 4);

    bp = XtCreateManagedWidget("ajouter 3", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 5);

    bp = XtCreateManagedWidget("ajouter 0", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 6);

    bp = XtCreateManagedWidget("deplacer 5", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 7);

    bp = XtCreateManagedWidget("deplacer 0 (fin)", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 8);

    bp = XtCreateManagedWidget("ajouter -1", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 9);

    bp = XtCreateManagedWidget("ajouter 50", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 10);

    bp = XtCreateManagedWidget("deplacer -1", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 11);

    bp = XtCreateManagedWidget("deplacer 50", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 12);

    bp = XtCreateManagedWidget("deplacer 0", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 13);

    bp = XtCreateManagedWidget("deplacer 1", xmPushButtonWidgetClass, popupMenu, NULL, 0);
    XtAddCallback(bp, XmNactivateCallback, do_it, 14);

    return popupMenu;
}
/*
 * popupmenu -- callback to popup a menu
 *
 * toggle between store by row and store by column
 */
static void popupmenu(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{

    _target = XtParent(w);

    if (_popupMenu == NULL)
        _popupMenu = create_popupMenu(toplevel);

    XmMenuPosition(_popupMenu, ((XmPushButtonCallbackStruct*)call_data)->event);
    XtManageChild(_popupMenu);
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
    XtVaSetValues(rowCol, XmNstoreByRow, byRow, NULL);
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
    Widget bb, sw, command, form;

    toplevel = XtVaAppInitialize(&app_context, "RowColT", NULL, 0,
				  &argc, argv, fallback_resources, NULL);

    form =  XtVaCreateManagedWidget("form", 
                xmFormWidgetClass,                 toplevel,
                NULL);

    sw = XtVaCreateManagedWidget("sw", 
                xmScrolledWindowWidgetClass,       form,
		XmNtopAttachment,                  XmATTACH_FORM,
		XmNbottomAttachment,               XmATTACH_FORM,
		XmNleftAttachment,                 XmATTACH_FORM,
		XmNrightAttachment,                XmATTACH_FORM,
                NULL);

    rowCol = XtVaCreateManagedWidget("rowCol", 
                rowColWidgetClass,       sw,
                /* XmNstoreByRow,           byRow, */
                NULL);

    XtVaSetValues(sw,
		XmNworkWindow,		rowCol,
		NULL);

    XtManageChild(sw);

    /*
     * Create MAX buttons
     */
    for (i = 0; i < MAX; i++) {

	(void) sprintf(t, "%d", i);
	s = XtNewString(t);

        create_buttons(rowCol, (i), s );

    }

#if 0
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

#endif

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
