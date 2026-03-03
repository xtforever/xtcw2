#include <stdio.h>

#include <Xfwf/Array.h>
/* #include <Xfwf/UltraToggle.h>*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/MenuButton.h>

#include <Xfwf/EzMenu.h>
#include <Xfwf/EzME.h>

#include <Xfwf/StrToPmap.h>

double drand48();
long lrand48();

XtAppContext	app_con;

struct thing {
  int	w,h;
};

#define NCOLS	3
#define NROWS	4

struct thing data[NROWS][NCOLS] = {
  { { 100, 40 },{ 100, 40 },{ 100, 40 }, },
  { { 100, 40 },{ 150, 60 },{ 100, 40 }, },
  { { 100, 40 },{ 100, 40 },{ 100, 40 }, },
  { { 100, 40 },{ 100, 40 },{ 100, 40 }, },
};

Widget	array;

String fallback_resources[] = {
  "*initialResourcesPersistent: false",	/* here's a strange resource */
  "*Array.verticalSpacing: 4",
  "*Array.horizontalSpacing: 4",
  0
};

struct value_thing {
  String	name;
/*  Widget	toggle;*/
  Widget	text;
} value_sets[] = {
  { "row" },
  { "column" },
  { "nrows" },
  { "ncolumns" },
  { "hjust" },
  { "vjust" },
};

#define NVALUES	XtNumber(value_sets)

void add_label(w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
{
  XtVarArgsList	list;
  String	values[NVALUES];
  int	i;

  for (i=0; i<NVALUES; i++) {
    XtVaGetValues(value_sets[i].text, XtNstring, &values[i], (String)0);
  }

  list = XtVaCreateArgsList
    ((XtPointer)0,
     XtVaTypedArg, XtNrow, XtRString, (XtArgVal)values[0], sizeof(String),
     XtVaTypedArg, XtNcolumn, XtRString, (XtArgVal)values[1], sizeof(String),
     XtVaTypedArg, XtNrowSpan, XtRString, (XtArgVal)values[2], sizeof(String),
     XtVaTypedArg, XtNcolumnSpan, XtRString, (XtArgVal)values[3], sizeof(String),
     XtVaTypedArg, XtNhorizontalJustify, XtRString, (XtArgVal)values[4], sizeof(String),
     XtVaTypedArg, XtNverticalJustify, XtRString, (XtArgVal)values[5], sizeof(String),
     (String)0);
  XtVaCreateManagedWidget
    ( "thing", labelWidgetClass, array,
     XtVaNestedList, list,
     (String)0);
}

void add_label_c(w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
{
  XtVarArgsList	list;
  String	values[NVALUES];
  int	i;

  for (i=0; i<NVALUES; i++) {
    XtVaGetValues(value_sets[i].text, XtNstring, &values[i], (String)0);
  }

  list = XtVaCreateArgsList
    ((XtPointer)0,
     XtVaTypedArg, XtNrow, XtRString, (XtArgVal)values[0], sizeof(String),
     XtVaTypedArg, XtNcolumn, XtRString, (XtArgVal)values[1], sizeof(String),
     XtVaTypedArg, XtNrowSpan, XtRString, (XtArgVal)values[2], sizeof(String),
     XtVaTypedArg, XtNcolumnSpan, XtRString, (XtArgVal)values[3], sizeof(String),
     XtVaTypedArg, XtNhorizontalJustify, XtRString, (XtArgVal)values[4], sizeof(String),
     XtVaTypedArg, XtNverticalJustify, XtRString, (XtArgVal)values[5], sizeof(String),
     (String)0);

  XfwfArray_VanlAppendWidgetToColumn( "thing", labelWidgetClass, array, list);

  XtFree(list);
}

void add_label_r(w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
{
  XtVarArgsList	list;
  String	values[NVALUES];
  int	i;

  for (i=0; i<NVALUES; i++) {
    XtVaGetValues(value_sets[i].text, XtNstring, &values[i], (String)0);
  }

  list = XtVaCreateArgsList
    ((XtPointer)0,
     XtVaTypedArg, XtNrow, XtRString, (XtArgVal)values[0], sizeof(String),
     XtVaTypedArg, XtNcolumn, XtRString, (XtArgVal)values[1], sizeof(String),
     XtVaTypedArg, XtNrowSpan, XtRString, (XtArgVal)values[2], sizeof(String),
     XtVaTypedArg, XtNcolumnSpan, XtRString, (XtArgVal)values[3], sizeof(String),
     XtVaTypedArg, XtNhorizontalJustify, XtRString, (XtArgVal)values[4], sizeof(String),
     XtVaTypedArg, XtNverticalJustify, XtRString, (XtArgVal)values[5], sizeof(String),
     (String)0);

  XfwfArray_VanlAppendWidgetToRow( "thing", labelWidgetClass, array, list);

  XtFree(list);
}

void add_random(w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
{
  Arg	args[7];
  int	nargs=0;
  int	nrows, ncols;

  XtVaGetValues(array,
		XtNnumRows, &nrows,
		XtNnumColumns, &ncols,
		(String)0);

  XtSetArg(args[nargs], XtNrow, (lrand48()%(nrows+1))); nargs++;
  XtSetArg(args[nargs], XtNcolumn, (lrand48()%(ncols+1))); nargs++;
  XtSetArg(args[nargs], XtNhorizontalJustify, (lrand48()%4)); nargs++;
  XtSetArg(args[nargs], XtNverticalJustify, (lrand48()%4)); nargs++;
  if (drand48()<0.3) {
    XtSetArg(args[nargs], XtNrowSpan, lrand48()%5-1); nargs++;
  }
  if (drand48()<0.3) {
    XtSetArg(args[nargs], XtNcolumnSpan, lrand48()%5-1); nargs++;
  }

  XtCreateManagedWidget("thing", labelWidgetClass, array, args, nargs);
}

void make_createbox(parent)
     Widget	parent;
{
  Widget	createbox,button;
  Arg	args[4];
  Cardinal	nargs;
  int	i;

  nargs=0;
  XtSetArg(args[nargs], XtNhorizontalJustify, XfwfHJleft); nargs++;
  createbox = XfwfArray_AppendWidgetToRow
      ("create", xfwfarrayWidgetClass, parent, args, nargs);

  for (i=0; i<NVALUES; i++) {
#if 0
    value_sets[i].toggle = XtVaCreateManagedWidget
      (value_sets[i].name, xfwfultraToggleWidgetClass, createbox,
       XtNrow, i,
       XtNhorizontalJustify, XfwfHJleft,
       XtNtoggleStyle, i+3,
       (String)0);
#endif
    value_sets[i].text = XtVaCreateManagedWidget
      (value_sets[i].name, asciiTextWidgetClass, createbox,
       XtNrow, i,
       XtNcolumn, 1,
       XtNhorizontalJustify, XfwfHJfill,
       XtNeditType, XawtextEdit,
       (String)0);
  }

  nargs=0;
  XtSetArg(args[nargs], XtNcolumn, 0); nargs++;
  XtSetArg(args[nargs], XtNcolumnSpan, XfwfArrayHorizon); nargs++;
  button = XfwfArray_AppendWidgetToColumn
    ("create", commandWidgetClass, createbox,
     args, nargs);
  XtAddCallback(button, XtNcallback, add_label, (XtPointer)0);

  nargs=0;
  XtSetArg(args[nargs], XtNcolumn, 0); nargs++;
  XtSetArg(args[nargs], XtNcolumnSpan, XfwfArrayHorizon); nargs++;
  button = XfwfArray_AppendWidgetToColumn
    ("appendc", commandWidgetClass, createbox,
     args, nargs);
  XtAddCallback(button, XtNcallback, add_label_c, (XtPointer)0);

  nargs=0;
  XtSetArg(args[nargs], XtNcolumn, 0); nargs++;
  XtSetArg(args[nargs], XtNcolumnSpan, XfwfArrayHorizon); nargs++;
  button = XfwfArray_AppendWidgetToColumn
    ("appendr", commandWidgetClass, createbox,
     args, nargs);
  XtAddCallback(button, XtNcallback, add_label_r, (XtPointer)0);

  nargs=0;
  XtSetArg(args[nargs], XtNcolumn, 0); nargs++;
  XtSetArg(args[nargs], XtNcolumnSpan, XfwfArrayHorizon); nargs++;
  button = XfwfArray_AppendWidgetToColumn
    ("random", commandWidgetClass, createbox,
     args, nargs);
  XtAddCallback(button, XtNcallback, add_random, (XtPointer)0);
}

/**********************************************************************/

void toggle_resource(w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
{
  XtVaSetValues(array, (String)client_data, call_data, (String)0);
}

void set_expandable_col(w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
{
  Widget	text = (Widget)client_data;
  String	s;
  int	row, val;

  XtVaGetValues(text, XtNstring, &s, (String)0);
  sscanf(s, "%d %d", &row, &val);

  XfwfArray_SetExpandableColumn(array, row, val);
}

void set_expandable_row(w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
{
  Widget	text = (Widget)client_data;
  String	s;
  int	row, val;

  XtVaGetValues(text, XtNstring, &s, (String)0);
  sscanf(s, "%d %d", &row, &val);

  XfwfArray_SetExpandableRow(array, row, val);
}

void make_valuesbox(parent)
     Widget	parent;
{
  Widget	valuesbox,button, text;
  Widget	suba;
  Arg	args[4];
  Cardinal	nargs;

#if 0
  nargs=0;
  XtSetArg(args[nargs], XtNhorizontalJustify, XfwfHJleft); nargs++;
  XtSetArg(args[nargs], XtNverticalJustify, XfwfVJtop); nargs++;
  valuesbox = XfwfArray_AppendWidgetToRow
    ("values", xfwfarrayWidgetClass, parent, args, nargs);
#else
  valuesbox = XfwfArray_VaAppendWidgetToRow
    ("values", xfwfarrayWidgetClass, parent,
     XtNhorizontalJustify, XfwfHJleft,
     XtNverticalJustify, XfwfVJtop,
     (String)0);
#endif

  nargs=0;
  button = XfwfArray_AppendWidgetToColumn
    ("samewidth", toggleWidgetClass, valuesbox, args, nargs);
  XtAddCallback(button, XtNcallback,
		toggle_resource, (XtPointer)XtNsameWidth);
  nargs=0;
  button = XfwfArray_AppendWidgetToColumn
    ("sameheight", toggleWidgetClass, valuesbox, args, nargs);
  XtAddCallback(button, XtNcallback,
		toggle_resource, (XtPointer)XtNsameHeight);

  suba = XfwfArray_VaAppendWidgetToColumn
    ("exp-row", xfwfarrayWidgetClass, valuesbox,
     (String)0);

  button = XfwfArray_VaAppendWidgetToRow
    ("activate", commandWidgetClass, suba,
     (String)0);
  text = XfwfArray_VaAppendWidgetToRow
    ("text", asciiTextWidgetClass, suba,
     XtNeditType, XawtextEdit,
     (String)0);
  XtAddCallback(button, XtNcallback,
		set_expandable_row, (XtPointer)text);

  suba = XfwfArray_VaAppendWidgetToColumn
    ("exp-col", xfwfarrayWidgetClass, valuesbox,
     (String)0);

  button = XfwfArray_VaAppendWidgetToRow
    ("activate", commandWidgetClass, suba,
     (String)0);
  text = XfwfArray_VaAppendWidgetToRow
    ("text", asciiTextWidgetClass, suba,
     XtNeditType, XawtextEdit,
     (String)0);
  XtAddCallback(button, XtNcallback,
		set_expandable_col, (XtPointer)text);
}

void destroy_widget(w, client_data, call_data)
      Widget w;
      XtPointer client_data;
      XtPointer call_data;
{
  XtDestroyWidget((Widget)client_data);

}


int main(argc, argv)
     int	argc;
     char	**argv;
{
  Widget	outer, master;
  int	x,y;

/*  malloc_debug(2); */

  outer = XtAppInitialize(&app_con, "ArrayT", (XrmOptionDescList)0, 0,
			  &argc, argv, fallback_resources, (ArgList)0, 0);

  XfwfInstallStringToPixmapConverter();

  master = XtVaCreateManagedWidget
    ("master", xfwfarrayWidgetClass, outer,
     (String)0);

  make_createbox(master);
  make_valuesbox(master);

  {
    Widget	button,menu;

    button = XfwfArray_VaAppendWidgetToRow
      ("testmenu", menuButtonWidgetClass, master,
       (String)0);
    menu = XtVaCreatePopupShell
      ("menu", xfwfEzMenuWidgetClass, button,
       (String)0);
    XtVaCreateManagedWidget
      ("entry1", smeBSBObjectClass, menu,
       (String)0);
    XtVaCreateManagedWidget
      ("entry2", smeBSBObjectClass, menu,
       (String)0);

    button = XtVaCreateManagedWidget
      ("entry3", xfwfEzMEObjectClass, menu,
       (String)0);
    menu =  XtVaCreatePopupShell
      ("menu", xfwfEzMenuWidgetClass, menu,
       (String)0);
    XtVaSetValues(button, XtNmenu, menu, (String)0);
    XtVaCreateManagedWidget
      ("entry1", smeBSBObjectClass, menu,
       (String)0);
    XtVaCreateManagedWidget
      ("entry2", smeBSBObjectClass, menu,
       (String)0);

    button = XtVaCreateManagedWidget
      ("entry3", xfwfEzMEObjectClass, menu,
       (String)0);
    menu =  XtVaCreatePopupShell
      ("menu", xfwfEzMenuWidgetClass, menu,
       (String)0);
    XtVaSetValues(button, XtNmenu, menu, (String)0);
    XtVaCreateManagedWidget
      ("entry1", smeBSBObjectClass, menu,
       (String)0);
    XtVaCreateManagedWidget
      ("entry2", smeBSBObjectClass, menu,
       (String)0);
  }

  {
    Widget	button;
    button = XfwfArray_VaAppendWidgetToRow
      ("destroy", commandWidgetClass, master,
       (String)0);
    XtAddCallback(button, XtNcallback,
		  destroy_widget, (XtPointer)button);
  }

  array = XtVaCreateManagedWidget
    ("array", xfwfarrayWidgetClass, master,
     XtNrow, 1,
#if 1
     XtNverticalSpacing, 2,
     XtNhorizontalSpacing, 2,
#endif
     XtNrowSpan, XfwfArrayInfinity,
     XtNcolumnSpan, XfwfArrayInfinity,
     XtNhorizontalJustify, XfwfHJfill,
     XtNverticalJustify, XfwfVJfill,
     XtNdrawShadows, 1,
     (String)0);

  for (y=0; y<NROWS; y++) {
    for (x=0; x<NCOLS; x++) {
      char	buf[40];
      sprintf(buf, "%dx%d+%d+%d", data[y][x].w, data[y][x].h, x,y);
      XtVaCreateManagedWidget
	(buf, labelWidgetClass, array,
	 XtNrow, y,
	 XtNcolumn, x,
#if 1
	 XtNwidth, data[y][x].w,
	 XtNheight, data[y][x].h,
#endif
	 (String)0);
    }
  }

  XtRealizeWidget(outer);

  XtAppMainLoop(app_con);

  exit(1);
}
