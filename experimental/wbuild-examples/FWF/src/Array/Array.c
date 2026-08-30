/*
    FileComp.c: implementation of the array widget
    Copyright (C) 1993 Robert Forsman

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */

/*
 * $Log$
 */


#include <X11/IntrinsicP.h>
#include <X11/RectObjP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/CharSet.h>

#include <Xfwf/VarArgs.h>

#include <Xfwf/ArrayP.h>

static int		default_count=0;
static Dimension	default_spacing=0;
static DimensionList	default_dl=0;
static Boolean		default_same=False;

#define Offset(field) XtOffsetOf(XfwfArrayRec, xfwf_array.field)
static XtResource arrayResources[] = {
  { XtNnumRows, XtCNumRows, XtRInt, sizeof(int),
      Offset(n_rows), XtRInt, (XtPointer)&default_count},
  { XtNnumColumns, XtCNumColumns, XtRInt, sizeof(int),
      Offset(n_columns), XtRInt, (XtPointer)&default_count},

  { XtNverticalSpacing, XtCVerticalSpacing, XtRDimension,
      sizeof(Dimension), Offset(vertical_spacing),
      XtRDimension, (XtPointer)&default_spacing },
  { XtNhorizontalSpacing, XtCHorizontalSpacing, XtRDimension,
      sizeof(Dimension), Offset(horizontal_spacing),
      XtRDimension, (XtPointer)&default_spacing },
  { XtNverticalSpacingArray, XtCVerticalSpacingArray, XtRDimensionList,
      sizeof(DimensionList), Offset(vertical_spacing_a),
      XtRDimensionList, (XtPointer)&default_dl },
  { XtNhorizontalSpacingArray, XtCHorizontalSpacingArray, XtRDimensionList,
      sizeof(DimensionList), Offset(horizontal_spacing_a),
      XtRDimensionList, (XtPointer)&default_dl },

  { XtNsameWidth, XtCSameWidth, XtRBoolean, sizeof(Boolean),
      Offset(same_width), XtRBoolean, (XtPointer)&default_same },
  { XtNsameHeight, XtCSameHeight, XtRBoolean, sizeof(Boolean),
      Offset(same_height), XtRBoolean, (XtPointer)&default_same },

  { XtNdrawShadows, XtCDrawShadows, XtRBoolean, sizeof(Boolean),
      Offset(draw_shadows), XtRInt, 0},
  { XtNtopShadowPen, XtCTopShadowPen, XtRXfwfPenPtr, sizeof(XfwfPen*),
      Offset(top_shadow), XtRString, XtDefaultTopShadowPen },
  { XtNbottomShadowPen, XtCBottomShadowPen, XtRXfwfPenPtr, sizeof(XfwfPen*),
      Offset(bottom_shadow), XtRString, XtDefaultBottomShadowPen },
};
#undef Offset

static int	default_pos=0;
static int	default_spread=1;
static XfwfVerticalJustify	default_vjust = XfwfVJcenter;
static XfwfHorizontalJustify	default_hjust = XfwfHJcenter;

#define Offset(field) XtOffsetOf(XfwfArrayConstraintsRec, xfwf_array.field)
static XtResource arrayConstraintResources[] = {
  { XtNrow, XtCRow, XtRInt, sizeof(int),
      Offset(row), XtRInt, (XtPointer)&default_pos},
  { XtNcolumn, XtCColumn, XtRInt, sizeof(int),
      Offset(column), XtRInt, (XtPointer)&default_pos},
  { XtNrowSpan, XtCRowSpan, XtRInt, sizeof(int),
      Offset(nrows), XtRInt, (XtPointer)&default_spread},
  { XtNcolumnSpan, XtCColumnSpan, XtRInt, sizeof(int),
      Offset(ncolumns), XtRInt, (XtPointer)&default_spread},
  { XtNverticalJustify, XtCVerticalJustify, XtRVerticalJustify,
      sizeof(XfwfVerticalJustify), Offset(vjust),
      XtRVerticalJustify, (XtPointer)&default_vjust },
  { XtNhorizontalJustify, XtCHorizontalJustify, XtRHorizontalJustify,
      sizeof(XfwfHorizontalJustify), Offset(hjust),
      XtRHorizontalJustify, (XtPointer)&default_hjust },
};
#undef Offset

#if ! NeedFunctionPrototypes
static void ClassInitialize();
static void ClassPartInitialize();
static void Initialize();
static void ConstraintInitialize();
static void Redisplay();
static void Resize();
static void Destroy();
static void ChangeManaged();
static Boolean SetValues();
static Boolean ConstraintSetValues();
static XtGeometryResult GeometryManager();
static XtGeometryResult PreferredGeometry();
#else /* NeedFunctionPrototypes */
static void ClassInitialize(void);
static void ClassPartInitialize(WidgetClass class);
static void Initialize(Widget request, Widget new, ArgList args, Cardinal *num_args);
static void ConstraintInitialize(Widget request, Widget new, ArgList args, Cardinal *num_args);
static void Redisplay(Widget w, XEvent *event, Region region);
static void Resize(Widget w);
static void Destroy(Widget w);
static void ChangeManaged(Widget w);
static Boolean SetValues(Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args);
static Boolean ConstraintSetValues(Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args);
static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry *request, XtWidgetGeometry *ret);
static XtGeometryResult PreferredGeometry(Widget w, XtWidgetGeometry *req, XtWidgetGeometry *pref);
#endif /* NeedFunctionPrototypes */

CompositeClassExtensionRec composite_extension = {
  0,
  NULLQUARK,
  XtCompositeExtensionVersion,
  sizeof(composite_extension),
  1,
};				/* make use of this struct when we get
				   an insert_child procedure */

XfwfArrayClassRec xfwfarrayClassRec = {
  { /* core_class fields */
    (WidgetClass) &constraintClassRec, /* superclass */
    "Array",
    sizeof(XfwfArrayRec),
    ClassInitialize,
    ClassPartInitialize,
    FALSE,			/* class_inited */
    Initialize,
    0,
    XtInheritRealize,
    0,
    0,				/* num_actions */
    arrayResources,
    XtNumber(arrayResources),
    NULLQUARK,
    TRUE,
    XtExposeCompressMultiple,			/* compress_exposure */
    TRUE,
    FALSE,
    Destroy,
    Resize,
    Redisplay,		/* expose */
    SetValues,
    0,
    XtInheritSetValuesAlmost,
    0,
    0,				/* accept_focus */
    XtVersion,
    0,
    0,
    PreferredGeometry,
    XtInheritDisplayAccelerator,
    0
  }, { /* composite_class fields */
    GeometryManager,
    ChangeManaged,
    XtInheritInsertChild,	/* XXX, gotta replace this and check
				   for non-rectobj children */
    XtInheritDeleteChild,
    0 /*(XtPointer)&composite_extension*/
  }, { /* constraint_class fields */
    arrayConstraintResources,
    XtNumber(arrayConstraintResources),
    sizeof(XfwfArrayConstraintsRec),
    ConstraintInitialize,
    0,
    ConstraintSetValues,
    0
  }, { /* array_class fields */
    0
  }
};

WidgetClass xfwfarrayWidgetClass = (WidgetClass)&xfwfarrayClassRec;


/* type converters ****************************************************/


#define	almost_done(type, value) \
	do {							\
	    if (to->addr != NULL) {				\
		if (to->size < sizeof(type)) {		\
		    to->size = sizeof(type);			\
		    return False;				\
		}						\
		*(type*)(to->addr) = (value);		\
	    }							\
	    else {						\
		static type static_val;				\
		static_val = (value);				\
		to->addr = (XPointer)&static_val;		\
	    }							\
	    to->size = sizeof(type);				\
	} while (0)

#if ! NeedFunctionPrototypes
Boolean Cvt_String_to_XfwfHorizontalJustify(dpy, args, num_args, from, to, converter_data)
     Display *dpy;
     XrmValue *args;
     Cardinal *num_args;
     XrmValue *from;
     XrmValue *to;
     XtPointer *converter_data;
#else /* NeedFunctionPrototypes */
Boolean Cvt_String_to_XfwfHorizontalJustify(Display *dpy, XrmValue *args, Cardinal *num_args, XrmValue *from, XrmValue *to, XtPointer *converter_data)
#endif /* NeedFunctionPrototypes */
{
  if (*num_args != 0)
    XtAppWarningMsg
      (XtDisplayToApplicationContext(dpy), "Wrong Parameters",
       "cvtStringToXfwfHorizontalJustify", "XfwfLibraryError",
       "String to XfwfHorizontalJustify conversion needs no extra arguments",
       (String *)0, (Cardinal *)0);

  if (0==XmuCompareISOLatin1(from->addr, "left"))
    almost_done(XfwfHorizontalJustify, XfwfHJleft);
  else if (0==XmuCompareISOLatin1(from->addr, "right"))
    almost_done(XfwfHorizontalJustify, XfwfHJright);
  else if (0==XmuCompareISOLatin1(from->addr, "center"))
    almost_done(XfwfHorizontalJustify, XfwfHJcenter);
  else if (0==XmuCompareISOLatin1(from->addr, "fill"))
    almost_done(XfwfHorizontalJustify, XfwfHJfill);
  else {
    XtDisplayStringConversionWarning(dpy, from->addr, XtRHorizontalJustify);
    return False;
  }
  return True;
}

#if ! NeedFunctionPrototypes
Boolean Cvt_String_to_XfwfVerticalJustify(dpy, args, num_args, from, to, converter_data)
     Display *dpy;
     XrmValue *args;
     Cardinal *num_args;
     XrmValue *from;
     XrmValue *to;
     XtPointer *converter_data;
#else /* NeedFunctionPrototypes */
Boolean Cvt_String_to_XfwfVerticalJustify(Display *dpy, XrmValue *args, Cardinal *num_args, XrmValue *from, XrmValue *to, XtPointer *converter_data)
#endif /* NeedFunctionPrototypes */
{
  if (*num_args != 0)
    XtAppWarningMsg
      (XtDisplayToApplicationContext(dpy), "Wrong Parameters",
       "cvtStringToXfwfVerticalJustify", "XfwfLibraryError",
       "String to XfwfVerticalJustify conversion needs no extra arguments",
       (String *)0, (Cardinal *)0);

  if (0==XmuCompareISOLatin1(from->addr, "top"))
    almost_done(XfwfVerticalJustify, XfwfVJtop);
  else if (0==XmuCompareISOLatin1(from->addr, "bottom"))
    almost_done(XfwfVerticalJustify, XfwfVJbottom);
  else if (0==XmuCompareISOLatin1(from->addr, "center"))
    almost_done(XfwfVerticalJustify, XfwfVJcenter);
  else if (0==XmuCompareISOLatin1(from->addr, "fill"))
    almost_done(XfwfVerticalJustify, XfwfVJfill);
  else {
    XtDisplayStringConversionWarning(dpy, from->addr, XtRVerticalJustify);
    return False;
  }
  return True;
}

#undef almost_done

/* internal utility proc declarations *********************************/

#if ! NeedFunctionPrototypes
static void CountCells(), EnsureSpace(), ComputeIdeal(), ComputeCompromise();
static void CalcChildrenPosition(), AcceptProposedLayout();
static void LoadCurrentGeometryAsDesired(), AttemptReconfigure();
static void PerformLayout();
Dimension AvailableWidth(), AvailableHeight();
Dimension XfwfArraySizesWidth(), XfwfArraySizesHeight();
void zero_XfwfArraySizesPart(), alloc_XfwfArraySizesPart(),
	free_XfwfArraySizesPart();
#else /* NeedFunctionPrototypes */
static void CountCells(XfwfArrayWidget array), EnsureSpace(XfwfArrayWidget array), ComputeIdeal(XfwfArrayWidget parent, Dimension *width, Dimension *height), ComputeCompromise(XfwfArrayWidget parent, Dimension width, Dimension height);
static void CalcChildrenPosition(XfwfArrayWidget parent), AcceptProposedLayout(XfwfArrayWidget parent);
static void LoadCurrentGeometryAsDesired(XfwfArrayWidget parent), AttemptReconfigure(XfwfArrayWidget parent);
static void PerformLayout(XfwfArrayWidget parent);
Dimension AvailableWidth(RectObj child, XfwfArraySizesPart *part), AvailableHeight(RectObj child, XfwfArraySizesPart *part);
Dimension XfwfArraySizesWidth(XfwfArraySizesPart *sizes), XfwfArraySizesHeight(XfwfArraySizesPart *sizes);
void zero_XfwfArraySizesPart(XfwfArraySizesPart *part), alloc_XfwfArraySizesPart(XfwfArraySizesPart *part, int row, int col),
	free_XfwfArraySizesPart(XfwfArraySizesPart *part);
#endif /* NeedFunctionPrototypes */

/* class methods ******************************************************/

#if ! NeedFunctionPrototypes
static void ClassInitialize()
#else /* NeedFunctionPrototypes */
static void ClassInitialize(void)
#endif /* NeedFunctionPrototypes */
{
  /* add type converters for the justify stuff */
  XtSetTypeConverter(XtRString, XtRHorizontalJustify,
		     Cvt_String_to_XfwfHorizontalJustify,
		     (XtConvertArgList) 0, (Cardinal)0,
		     XtCacheAll, (XtDestructor)0);
  XtSetTypeConverter(XtRString, XtRVerticalJustify,
		     Cvt_String_to_XfwfVerticalJustify,
		     (XtConvertArgList) 0, (Cardinal)0,
		     XtCacheAll, (XtDestructor)0);
  XfwfInstallStringToPenConverter();
}

#if ! NeedFunctionPrototypes
static void ClassPartInitialize(class)
     WidgetClass class;
#else /* NeedFunctionPrototypes */
static void ClassPartInitialize(WidgetClass class)
#endif /* NeedFunctionPrototypes */
{
  /* urgh, handle virtual functions here */
}

/* ARGSUSED */
#if ! NeedFunctionPrototypes
static void Initialize(request, new, args, num_args)
     Widget request;
     Widget new;
     ArgList args;
     Cardinal *num_args;
#else /* NeedFunctionPrototypes */
static void Initialize(Widget request, Widget new, ArgList args, Cardinal *num_args)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array = (XfwfArrayWidget)new;
  XfwfArrayPart		*part = &array->xfwf_array;
  /* check for specified width and height */

  /* currently a non-resource item: */
  part->expandable_rows = 0;
  part->expandable_columns = 0;

  /* blank out these non-resource items */
  zero_XfwfArraySizesPart(&part->actual);
  zero_XfwfArraySizesPart(&part->proposed);

  EnsureSpace(array);

  part->vertical_spacing_a[0] = part->vertical_spacing;
  part->horizontal_spacing_a[0] = part->horizontal_spacing;
}

#if ! NeedFunctionPrototypes
static void Destroy(w)
     Widget w;
#else /* NeedFunctionPrototypes */
static void Destroy(Widget w)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array = (XfwfArrayWidget)w;
  XfwfArrayPart		*part = &array->xfwf_array;

  free_XfwfArraySizesPart(&part->actual);
  free_XfwfArraySizesPart(&part->proposed);
}

#if ! NeedFunctionPrototypes
static Boolean SetValues(current, request, new, args, num_args)
     Widget current;
     Widget request;
     Widget new;
     ArgList args;
     Cardinal *num_args;
#else /* NeedFunctionPrototypes */
static Boolean SetValues(Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayPart	*src = &((XfwfArrayWidget) current)->xfwf_array;
  XfwfArrayPart	*dst = &((XfwfArrayWidget) new)->xfwf_array;
  int	relayout = 0;

  EnsureSpace((XfwfArrayWidget)current);

  /* these are read-only resources */
  CountCells((XfwfArrayWidget)current);
  dst->n_rows = src->n_rows;
  dst->n_columns = src->n_columns;

#define DIFFERENT(field) ( src->field != dst->field )
  if (DIFFERENT(vertical_spacing_a) ||
      DIFFERENT(horizontal_spacing_a) ||
      DIFFERENT(expandable_rows) ||
      DIFFERENT(expandable_columns)) {
    XtAppWarningMsg
      (XtWidgetToApplicationContext(new),
       "protectionViolation",
       "writeToReadOnly", "XfwfLibraryError",
       "XfwfArray: Attempt to write to read-only resource",
       (String*)0, (Cardinal*)0);
    dst->vertical_spacing_a = src->vertical_spacing_a;
    dst->horizontal_spacing_a = src->horizontal_spacing_a;
    dst->expandable_rows = src->expandable_rows;
    dst->expandable_columns = src->expandable_columns;
  }

  if (DIFFERENT(same_height) ||
      DIFFERENT(same_width))
    relayout = 1;

#undef DIFFERENT

  if (relayout)
    AttemptReconfigure((XfwfArrayWidget)new);

  return 0;
}

#if ! NeedFunctionPrototypes
static void Resize(w)
     Widget w;
#else /* NeedFunctionPrototypes */
static void Resize(Widget w)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array = (XfwfArrayWidget)w;

  ComputeCompromise(array, array->core.width, array->core.height);

  AcceptProposedLayout(array);
  CalcChildrenPosition(array);
  PerformLayout(array);
}

#if ! NeedFunctionPrototypes
static void Redisplay(w, event, region)
     Widget w;
     XEvent *event;
     Region region;
#else /* NeedFunctionPrototypes */
static void Redisplay(Widget w, XEvent *event, Region region)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array = (XfwfArrayWidget)w;
  XfwfArrayPart		*part = &array->xfwf_array;
  XfwfArraySizesPart	*sizes = &array->xfwf_array.actual;
  int	i;

  if (part->draw_shadows) {
    int j;
    int	r,c;
    Boolean	*inf_row, *inf_col;

    if (region==0) {
      XClearWindow(XtDisplay(w), XtWindow(w));
      XSetClipMask(XtDisplay(w), part->top_shadow->gc, None);
      XSetClipMask(XtDisplay(w), part->bottom_shadow->gc, None);
    } else {
      XSetRegion(XtDisplay(w), part->top_shadow->gc, region);
      XSetRegion(XtDisplay(w), part->bottom_shadow->gc, region);
    }

    EnsureSpace(array);

    inf_row = XtMalloc(sizeof(*inf_row)*part->n_rows);
    for (j=0; j<part->n_rows; j++) inf_row[j] = 0;
    inf_col = XtMalloc(sizeof(*inf_col)*part->n_columns);
    for (j=0; j<part->n_columns; j++) inf_col[j] = 0;

    for (i=0; i<array->composite.num_children; i++) {
      XfwfArrayConstraintsPart	*cnstr = 
	&((XfwfArrayConstraintsRec*)
	  array->composite.children[i]->core.constraints)
	  ->xfwf_array;

      if (!XtIsManaged(array->composite.children[i]))
	continue;

      if (cnstr->ncolumns==XfwfArrayInfinity)
	for (j=cnstr->row;
	     j<part->n_rows && (cnstr->nrows<1 || j<cnstr->row+cnstr->nrows);
	     j++)
	  inf_row[j] = 1;
      if (cnstr->nrows==XfwfArrayInfinity)
	for (j=cnstr->column;
	     j<part->n_columns && (cnstr->ncolumns<1 ||
				   j<cnstr->column+cnstr->ncolumns);
	     j++)
	  inf_col[j] = 1;
    }

#define LEFT_X(col)	(inf_row[r /*context!*/] && (col)==part->n_columns) \
  ? (XfwfArraySizesWidth(sizes) - part->horizontal_spacing_a[(col)]) \
    : (sizes->column_coord[(col)-1] + sizes->column_widths[(col)-1])
#define MIDDLE_X(col)	( ((col)==0) \
			 ? 0 \
			 : (((col)==part->n_columns) \
			    ? (inf_row[r /*context!*/] \
			       ? XfwfArraySizesWidth(sizes) \
			       : (sizes->column_coord[(col)]) \
			       ) \
			    : (sizes->column_coord[(col)] \
				  - part->horizontal_spacing_a[(col)]/2) \
			    ) \
			 )
#define TOP_Y(row)	(inf_col[c /*context!*/] && (row)==part->n_rows) \
  ? (XfwfArraySizesHeight(sizes) - part->vertical_spacing_a[(row)]) \
    : (sizes->row_coord[(row)-1] + sizes->row_heights[(row)-1])
#define MIDDLE_Y(row)	( ((row)==0) \
			 ? 0 \
			 :( ((row)==part->n_rows) \
			   ? (inf_col[c	/*context!*/] \
			      ? XfwfArraySizesHeight(sizes) \
			      : (sizes->row_coord[(row)]) \
			      ) \
			   : (sizes->row_coord[(row)] \
				 - part->vertical_spacing_a[(row)]/2) \
			   ) \
			 )


    for (r=0; r<part->n_rows; r++) {
      for (c=0; c<part->n_columns; c++) {
	XPoint	border[4];
	/* top */
	border[0].x = MIDDLE_X(c);	border[0].y = MIDDLE_Y(r);
	border[0].x = MIDDLE_X(c);	border[0].y = MIDDLE_Y(r);
	border[1].x = MIDDLE_X(c+1);	border[1].y = MIDDLE_Y(r);
	border[2].x = LEFT_X(c+1);
	border[2].y = sizes->row_coord[r];
	border[3].x = sizes->column_coord[c];
	border[3].y = sizes->row_coord[r];
	XFillPolygon(XtDisplay(w), XtWindow(w), part->top_shadow->gc,
		     border, 4, Convex, CoordModeOrigin);
	/* left */
	border[0].x = MIDDLE_X(c);	border[0].y = MIDDLE_Y(r+1);
	border[1].x = MIDDLE_X(c);	border[1].y = MIDDLE_Y(r);
	border[2].x = sizes->column_coord[c];
	border[2].y = sizes->row_coord[r];
	border[3].x = sizes->column_coord[c];
	border[3].y = TOP_Y(r+1);
	XFillPolygon(XtDisplay(w), XtWindow(w), part->top_shadow->gc,
		     border, 4, Convex, CoordModeOrigin);
	/* bottom */
	border[0].x = MIDDLE_X(c+1);	border[0].y = MIDDLE_Y(r+1);
	border[1].x = MIDDLE_X(c);	border[1].y = MIDDLE_Y(r+1);
	border[2].x = sizes->column_coord[c];
	border[2].y = TOP_Y(r+1);
	border[3].x = LEFT_X(c+1);
	border[3].y = TOP_Y(r+1);
	XFillPolygon(XtDisplay(w), XtWindow(w), part->bottom_shadow->gc,
		     border, 4, Convex, CoordModeOrigin);
	/* right */
	border[0].x = MIDDLE_X(c+1);	border[0].y = MIDDLE_Y(r);
	border[1].x = MIDDLE_X(c+1);	border[1].y = MIDDLE_Y(r+1);
	border[2].x = LEFT_X(c+1);
	border[2].y = TOP_Y(r+1);
	border[3].x = LEFT_X(c+1);
	border[3].y = sizes->row_coord[r];
	XFillPolygon(XtDisplay(w), XtWindow(w), part->bottom_shadow->gc,
		     border, 4, Convex, CoordModeOrigin);
      }
    }
    
    XtFree((char*)inf_row);
    XtFree((char*)inf_col);
#undef LEFT_X
#undef MIDDLE_X
#undef TOP_Y
#undef MIDDLE_Y

    if (region!=0) {
      XSetClipMask(XtDisplay(w), part->top_shadow->gc, None);
      XSetClipMask(XtDisplay(w), part->bottom_shadow->gc, None);
    }
  }
  
  for (i=0; i<array->composite.num_children; i++) {
    RectObj	child = (RectObj)array->composite.children[i];
    RectObjClass	roclass;
    if (!XtIsManaged((Widget)child) || XtIsWidget(child))
      continue;
    if (region)
      switch (XRectInRegion(region, child->rectangle.x, child->rectangle.y,
			    child->rectangle.width, child->rectangle.height)) {
      case RectangleIn:
      case RectanglePart:
	break;
      default:
	continue;
      }
    roclass = (RectObjClass)child->object.widget_class;

    if (roclass->rect_class.expose)
      roclass->rect_class.expose((Widget)child, event, region);
  }
}

/* constraint stuff ***************************************************/

#if ! NeedFunctionPrototypes
static void ConstraintInitialize(request, new, args, num_args)
     Widget request;
     Widget new;
     ArgList args;
     Cardinal *num_args;
#else /* NeedFunctionPrototypes */
static void ConstraintInitialize(Widget request, Widget new, ArgList args, Cardinal *num_args)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayConstraintsPart	*cnstr =
    &((XfwfArrayConstraintsRec*)new->core.constraints)->xfwf_array;

  /* safe, new is a RectObj */
  cnstr->d_width = new->core.width;
  cnstr->d_height = new->core.height;
  cnstr->d_border = new->core.border_width;
  cnstr->CSVhook = 0;
}

#if ! NeedFunctionPrototypes
static void ChangeManaged(w)
     Widget w;
#else /* NeedFunctionPrototypes */
static void ChangeManaged(Widget w)
#endif /* NeedFunctionPrototypes */
{
  LoadCurrentGeometryAsDesired( (XfwfArrayWidget)w );
  
  AttemptReconfigure( (XfwfArrayWidget)w );
}

/* what happens when someone changes our child's constraint resources: */
#if ! NeedFunctionPrototypes
static Boolean ConstraintSetValues(current, request, new, args, num_args)
     Widget current;
     Widget request;
     Widget new;
     ArgList args;
     Cardinal *num_args;
#else /* NeedFunctionPrototypes */
static Boolean ConstraintSetValues(Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args)
#endif /* NeedFunctionPrototypes */
{
  /* keep in mind that these Widgets are really RectObjs */
  XfwfArrayConstraintsPart	*old_part =
    &((XfwfArrayConstraintsRec*)current->core.constraints)->xfwf_array;
  XfwfArrayConstraintsPart	*new_part =
    &((XfwfArrayConstraintsRec*)new->core.constraints)->xfwf_array;

  if (new_part->column<0) {
    XtAppWarningMsg(XtWidgetToApplicationContext(new), "constraintError",
		    "outOfBounds", "XfwfLibraryError",
		    "XfwfArray: column constraint out of bounds",
		    (String*)0, (Cardinal*)0);
    new_part->column = 0;
  }
  if (new_part->row<0) {
    XtAppWarningMsg(XtWidgetToApplicationContext(new), "constraintError",
		    "outOfBounds", "XfwfLibraryError",
		    "XfwfArray: row constraint out of bounds",
		    (String*)0, (Cardinal*)0);
    new_part->row = 0;
  }

#define DIFFERENT(field)  (old_part->field != new_part->field)
  if (DIFFERENT(row) ||
      DIFFERENT(nrows) || 
      DIFFERENT(column) || 
      DIFFERENT(ncolumns) || 
      DIFFERENT(vjust) || 
      DIFFERENT(hjust)) {
#undef DIFFERENT
    AttemptReconfigure((XfwfArrayWidget)new->core.parent);
#define DIFFERENT(field)  (current->core.field != new->core.field)
    /* yick.  We have to tell the Array geometry manager to let us get
       away with it. */
    ((XfwfArrayConstraintsRec*)new->core.constraints)->xfwf_array.CSVhook =
      (DIFFERENT(x) ? CWX : 0) |
	(DIFFERENT(y) ? CWY : 0) |
	  (DIFFERENT(width) ? CWWidth : 0) |
	    (DIFFERENT(height) ? CWHeight : 0)
	      ;
#undef DIFFERENT
  }

  /* no redraw necessary */
  return 0;
}

#if ! NeedFunctionPrototypes
static XtGeometryResult PreferredGeometry(w, req, pref)
     Widget w;
     XtWidgetGeometry *req;
     XtWidgetGeometry *pref;
#else /* NeedFunctionPrototypes */
static XtGeometryResult PreferredGeometry(Widget w, XtWidgetGeometry *req, XtWidgetGeometry *pref)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array = (XfwfArrayWidget)w;
  int	mode = req->request_mode;

  ComputeIdeal(array, &pref->width, &pref->height);
  pref->request_mode = CWWidth|CWHeight;

  if (!mode&CWWidth)
    req->width = array->core.width;
  if (!mode&CWHeight)
    req->height = array->core.height;

  if (   req->width  == pref->width
      && req->height == pref->height)
    return XtGeometryYes;

  if ( pref->request_mode & ~mode)
    return XtGeometryAlmost;

  if (   req->width  == array->core.width
      && req->height == array->core.height)
    return XtGeometryNo;

  return XtGeometryAlmost;
}

/**********************************************************************/

#if ! NeedFunctionPrototypes
void zero_XfwfArraySizesPart(part)
     XfwfArraySizesPart *part;
#else /* NeedFunctionPrototypes */
void zero_XfwfArraySizesPart(XfwfArraySizesPart *part)
#endif /* NeedFunctionPrototypes */
{
  part->column_widths = part->column_coord = 0;
  part->row_heights = part->row_coord = 0;
  part->has_vslack = part->has_hslack = 0;
  part->vertical_slack = part->horizontal_slack = 0;
}

#if ! NeedFunctionPrototypes
void alloc_XfwfArraySizesPart(part, row, col)
     XfwfArraySizesPart *part;
     int row;
     int col;
#else /* NeedFunctionPrototypes */
void alloc_XfwfArraySizesPart(XfwfArraySizesPart *part, int row, int col)
#endif /* NeedFunctionPrototypes */
{
  part->column_widths = (Dimension*)XtMalloc(sizeof(Dimension)*col);
  part->column_coord = (Dimension*)XtMalloc(sizeof(Dimension)*(col+1));
  part->row_heights = (Dimension*)XtMalloc(sizeof(Dimension)*row);
  part->row_coord = (Dimension*)XtMalloc(sizeof(Dimension)*(row+1));
  part->n_rows = row;
  part->n_columns = col;
}

#if ! NeedFunctionPrototypes
void free_XfwfArraySizesPart(part)
     XfwfArraySizesPart *part;
#else /* NeedFunctionPrototypes */
void free_XfwfArraySizesPart(XfwfArraySizesPart *part)
#endif /* NeedFunctionPrototypes */
{
  XtFree((char*)part->column_widths);
  XtFree((char*)part->column_coord);
  XtFree((char*)part->row_heights);
  XtFree((char*)part->row_coord);
}

#if ! NeedFunctionPrototypes
Dimension XfwfArraySizesWidth(sizes)
     XfwfArraySizesPart *sizes;
#else /* NeedFunctionPrototypes */
Dimension XfwfArraySizesWidth(XfwfArraySizesPart *sizes)
#endif /* NeedFunctionPrototypes */
{
  return sizes->column_coord[sizes->n_columns] + sizes->horizontal_slack;
}

#if ! NeedFunctionPrototypes
Dimension XfwfArraySizesHeight(sizes)
     XfwfArraySizesPart *sizes;
#else /* NeedFunctionPrototypes */
Dimension XfwfArraySizesHeight(XfwfArraySizesPart *sizes)
#endif /* NeedFunctionPrototypes */
{
  return sizes->row_coord[sizes->n_rows] + sizes->vertical_slack;
}

/**********************************************************************/

#if ! NeedFunctionPrototypes
static void CountCells(array)
     XfwfArrayWidget array;
#else /* NeedFunctionPrototypes */
static void CountCells(XfwfArrayWidget array)
#endif /* NeedFunctionPrototypes */
{
  int	i;

  array->xfwf_array.n_rows = 0;
  array->xfwf_array.n_columns = 0;
  for (i=0; i<array->composite.num_children; i++) {
    RectObj	child = (RectObj)array->composite.children[i];
    XfwfArrayConstraintsPart	*cnstr =
      &((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;
    int nrows = cnstr->nrows;
    int ncols = cnstr->ncolumns;

    if (!XtIsManaged((Widget)child))
      continue;

    if (nrows<1) nrows=1;
    if (ncols<1) ncols=1;

    if (cnstr->row+nrows > array->xfwf_array.n_rows)
      array->xfwf_array.n_rows = cnstr->row+nrows;
    if (cnstr->column+ncols > array->xfwf_array.n_columns)
      array->xfwf_array.n_columns = cnstr->column+ncols;
  }
}

/* make sure there's enough space in all our arrays,
   except the sizes structs...  urgh */
#if ! NeedFunctionPrototypes
static void EnsureSpace(array)
     XfwfArrayWidget array;
#else /* NeedFunctionPrototypes */
static void EnsureSpace(XfwfArrayWidget array)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayPart	*part = &array->xfwf_array;
  int	nrows, ncols;
  int	i;

  nrows = part->n_rows;
  ncols = part->n_columns;

  CountCells(array);

  if (nrows<part->n_rows || !part->vertical_spacing_a) {
    part->vertical_spacing_a = (Dimension*)XtRealloc
      ((char*) part->vertical_spacing_a,
       sizeof(Dimension)*(part->n_rows+1));

    part->expandable_rows = (Boolean*)XtRealloc
      ((char*) part->expandable_rows,
       sizeof(Boolean)*part->n_rows);

    for (i=nrows; i<part->n_rows; i++)
      part->expandable_rows[i] = 0;
    for (i=nrows+1; i<=part->n_rows; i++) {
      part->vertical_spacing_a[i] = part->vertical_spacing;
      if (i>1) part->vertical_spacing_a[i-1] += part->vertical_spacing;
    }
  }
  if (ncols<part->n_columns || !part->horizontal_spacing_a) {
    part->horizontal_spacing_a = (Dimension*)XtRealloc
      ((char*) part->horizontal_spacing_a,
       sizeof(Dimension)*(part->n_columns+1));

    part->expandable_columns = (Boolean*)XtRealloc
      ((char*) part->expandable_columns,
       sizeof(Boolean)*part->n_columns);

    for (i=ncols; i<part->n_columns; i++)
      part->expandable_columns[i] = 0;
    for (i=ncols+1; i<=part->n_columns; i++) {
      part->horizontal_spacing_a[i] = part->horizontal_spacing;
      if (i>1) part->horizontal_spacing_a[i-1] += part->horizontal_spacing;
    }
  }
}

#if ! NeedFunctionPrototypes
static void ComputeSums(array, sizes)
     XfwfArrayWidget array;
     XfwfArraySizesPart *sizes;
#else /* NeedFunctionPrototypes */
static void ComputeSums(XfwfArrayWidget array, XfwfArraySizesPart *sizes)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayPart	*part = &array->xfwf_array;
  int	i;

  sizes->column_coord[0] = part->horizontal_spacing_a[0];
  for (i=0; i<sizes->n_columns; i++)
    sizes->column_coord[i+1] = sizes->column_coord[i]
      + sizes->column_widths[i] + part->horizontal_spacing_a[i+1];

  sizes->row_coord[0] = part->vertical_spacing_a[0];
  for (i=0; i<sizes->n_rows; i++)
    sizes->row_coord[i+1] = sizes->row_coord[i]
      + sizes->row_heights[i] + part->vertical_spacing_a[i+1];
}

/* Calculate ideal sizes for all the columns and rows */
#if ! NeedFunctionPrototypes
static void CalcIdealComponents(parent)
     XfwfArrayWidget parent;
#else /* NeedFunctionPrototypes */
static void CalcIdealComponents(XfwfArrayWidget parent)
#endif /* NeedFunctionPrototypes */
{
  Dimension	*width_a, *height_a;
  Dimension	*width_s, *height_s;
  XfwfArrayPart	*part = &parent->xfwf_array;
  int	ncolumns;
  int	nrows;
  int	i;

  EnsureSpace(parent);

  ncolumns = parent->xfwf_array.n_columns;
  nrows = parent->xfwf_array.n_rows;

  free_XfwfArraySizesPart(&part->proposed);
  alloc_XfwfArraySizesPart(&part->proposed, nrows, ncolumns);

  height_a = part->proposed.row_heights;
  height_s = part->proposed.row_coord;
  width_a = part->proposed.column_widths;
  width_s = part->proposed.column_coord;

  for (i=0; i<ncolumns; i++)
    width_a[i] = 0;
  for (i=0; i<nrows; i++)
    height_a[i] = 0;

  /* figure out a good size for the rows and columns */
  for (i=0; i<parent->composite.num_children; i++) {
    RectObj	child = (RectObj)parent->composite.children[i];
    XfwfArrayConstraintsPart	*cnstr =
      &((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;

    if (!XtIsManaged((Widget)child))
      continue;

    if (part->same_width) {
      if (cnstr->ncolumns>=0) {
	int	colcount = cnstr->ncolumns ?
	  cnstr->ncolumns : ncolumns - cnstr->column;
	int	i;
	int	w = cnstr->d_width+cnstr->d_border*2;
	for (i=cnstr->column+1; i<cnstr->column+colcount; i++)
	  w -= part->horizontal_spacing_a[i];
	w = (w-1)/colcount +1;
	if (w>0 && w>width_a[0])
	  width_a[0] = w;
      }
    } else if (cnstr->ncolumns==1 &&
	       width_a[cnstr->column] < cnstr->d_width+cnstr->d_border*2)
      width_a[cnstr->column] = cnstr->d_width+cnstr->d_border*2;

    if (part->same_height) {
      if (cnstr->nrows>=0) {
	int	rowcount = cnstr->nrows ?
	  cnstr->nrows : nrows - cnstr->row;
	int	i;
	int	h = cnstr->d_height+cnstr->d_border*2;
	for (i=cnstr->row+1; i<cnstr->row+rowcount; i++)
	  h -= part->vertical_spacing_a[i];
	h = (h-1)/rowcount +1;
	if (h>0 && h>height_a[0])
	  height_a[0] = h;
      }
    } else if (cnstr->nrows==1 &&
	       height_a[cnstr->row] < cnstr->d_height+cnstr->d_border*2)
      height_a[cnstr->row] = cnstr->d_height+cnstr->d_border*2;
  }

  if (part->same_width)
    for (i=1; i<ncolumns; i++)
      width_a[i] = width_a[i-1];
  if (part->same_height)
    for (i=1; i<nrows; i++)
      height_a[i] = height_a[i-1];

  /* an ungreedy/greedy algorithm that figures out which rows and
     columns to expand to fit fat mult-column/row widgets */
  while (1) {
    int	min_wadj;		/* minimum width adjustment any
				   multi-column/row widget needs */
    int min_hadj;		/* height adjustment */
    int idxminw = -1;		/* index of the width widget */
    int idxminh = -1;		/* index of the height widget */
    static int	*width_voters=0;
    static int	*height_voters=0;
    int	j;

    if (!width_voters) {
      width_voters = (int*)XtMalloc(sizeof(int) * part->proposed.n_columns);
      height_voters = (int*)XtMalloc(sizeof(int) * part->proposed.n_rows);
    }

    for (i=0; i<part->proposed.n_columns; i++)
      width_voters[i] = 0;
    for (i=0; i<part->proposed.n_rows; i++)
      height_voters[i] = 0;

    for (i=0; i<parent->composite.num_children; i++) {
      RectObj	child = (RectObj)parent->composite.children[i];
      XfwfArrayConstraintsPart	*cnstr =
	&((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;

      if (!XtIsManaged((Widget)child))
	continue;

      if (!part->same_width &&
	  (cnstr->ncolumns == 0 ||
	   cnstr->ncolumns > 1)) {
	int	adj = cnstr->d_width+cnstr->d_border*2 - AvailableWidth(child, &part->proposed);
	if (adj>0) {
	  if (idxminw<0 || adj < min_wadj) {
	    idxminw = i;
	    min_wadj = adj;
	  }
	  for (j=cnstr->column;
	       (cnstr->ncolumns<1 || j<cnstr->column+cnstr->ncolumns)
	       && j<part->proposed.n_columns;
	       j++)
	    width_voters[j] += 2;
	}
      }
      if (!part->same_height &&
	  (cnstr->nrows == 0 ||
	   cnstr->nrows > 1)) {
	int adj = cnstr->d_height+cnstr->d_border*2 - AvailableHeight(child, &part->proposed);
	if (adj>0) {
	  if (idxminh<0 || adj < min_hadj) {
	    idxminh = i;
	    min_hadj = adj;
	  }
	  for (j=cnstr->row;
	       (cnstr->nrows<1 || j<cnstr->row+cnstr->nrows)
	       && j<part->proposed.n_rows;
	       j++)
	    height_voters[j] += 2;
	}
      }
    }

    /* a row or column with an unsatisfied multi-cell widget will have
       a voters array value of at least 2 */

    /* if there are expandable columns or rows, prefer to satisfy wide
       or tall multi-column widgets: */
    for (i=0; i<part->proposed.n_columns; i++)
      if (part->expandable_columns[i] && width_voters[i]<=2)
	width_voters[i] ++;
    for (i=0; i<part->proposed.n_rows; i++)
      if (part->expandable_rows[i] && !height_voters[i]<=2)
	height_voters[i] ++;

    if (idxminw<0 && idxminh<0) {
      XtFree((char*)width_voters);
      XtFree((char*)height_voters);
      width_voters = height_voters = 0;
      break;			/* only the slackers are left */
    }

    /* we've found the minimum adjustments, now act on them */

    /* do we need to expand some columns? */
    if (idxminw>=0) {
      RectObj	child = (RectObj)parent->composite.children[idxminw];
      XfwfArrayConstraintsPart	*cnstr =
	&((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;
      int	maxvotes=0;
      int	count=0;
      /* find the columns that will benefit the most number of widgets */
      for (j=cnstr->column;
	   (cnstr->ncolumns<1 || j < cnstr->column+cnstr->ncolumns)
	   && j<part->proposed.n_columns;
	   j++) {
	if (width_voters[j]>maxvotes) {
	  maxvotes = width_voters[j];
	  count=1;
	} else if (width_voters[j]==maxvotes) {
	  count++;
	}
      }

      /* expand those columns */
      for (j=cnstr->column;
	   (cnstr->ncolumns<1 || j < cnstr->column+cnstr->ncolumns)
	   && j<part->proposed.n_columns;
	   j++) {
	int	adj;
	if (width_voters[j] < maxvotes)
	  continue;
	adj = (min_wadj+0.5)/count;
	width_a[j] += adj;
	min_wadj -= adj;
	count--;
      }
    }

    /* do we need to expand some rows? */
    if (idxminh >= 0) {
      RectObj	child = (RectObj)parent->composite.children[idxminh];
      XfwfArrayConstraintsPart	*cnstr =
	&((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;
      int	maxvotes=0;
      int	count=0;
      for (j=cnstr->row;
	   (cnstr->nrows<1 || j < cnstr->row+cnstr->nrows)
	   && j<part->proposed.n_rows;
	   j++) {
	if (height_voters[j]>maxvotes) {
	  maxvotes = height_voters[j];
	  count=1;
	} else if (height_voters[j]==maxvotes) {
	  count++;
	}
      }

      /* expand those rows */
      for (j=cnstr->row;
	   (cnstr->nrows<1 || j < cnstr->row+cnstr->nrows)
	   && j<part->proposed.n_rows;
	   j++) {
	int	adj;
	if (height_voters[j] < maxvotes)
	  continue;
	adj = (min_hadj+0.5)/count;
	height_a[j] += adj;
	min_hadj -= adj;
	count--;
      }
    }
  }

  for (i=0; i<parent->composite.num_children; i++) {
    RectObj	child = (RectObj)parent->composite.children[i];
    XfwfArrayConstraintsPart	*cnstr =
      &((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;

    if (!XtIsManaged((Widget)child))
      continue;

    if (cnstr->ncolumns<0) {
      int	adj = cnstr->d_width+cnstr->d_border*2 - AvailableWidth(child, &part->proposed);
      part->proposed.has_hslack = 1;
      if (adj>0)
	part->proposed.horizontal_slack += adj;
    }
    if (cnstr->nrows<0) {
      int	adj = cnstr->d_height+cnstr->d_border*2 - AvailableHeight(child, &part->proposed);
      part->proposed.has_vslack = 1;
      if (adj>0)
	part->proposed.vertical_slack += adj;
    }
     
  }

  ComputeSums(parent, &part->proposed);
}

/* Propose an ideal size for the Array widget based on the desired
   widths and heights of its children.  Compute sizes for the children
   too. */

#if ! NeedFunctionPrototypes
static void ComputeIdeal(parent, width, height)
     XfwfArrayWidget parent;
     Dimension *width;
     Dimension *height;
#else /* NeedFunctionPrototypes */
static void ComputeIdeal(XfwfArrayWidget parent, Dimension *width, Dimension *height)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayPart	*part = &parent->xfwf_array;

  CalcIdealComponents(parent);

  if (width) *width = XfwfArraySizesWidth(&part->proposed);
  if (height) *height = XfwfArraySizesHeight(&part->proposed);
}

/* Propose a compromise layout when the parent can't achieve a perfect
   size.  We assume that all the constraint values are current, as if
   ComputeIdeal had recently been called. */
#if ! NeedFunctionPrototypes
static void ComputeCompromise(parent, width, height)
     XfwfArrayWidget parent;
     Dimension width;
     Dimension height;
#else /* NeedFunctionPrototypes */
static void ComputeCompromise(XfwfArrayWidget parent, Dimension width, Dimension height)
#endif /* NeedFunctionPrototypes */
{
  XfwfArraySizesPart	*part = &parent->xfwf_array.proposed;
  int	i;
  int	ideal, curr;
  int	ncolumns, nrows;

  if (!part->column_widths) {
    /* oops, there is no ideal proposal */
    ComputeIdeal(parent, (Dimension*)0, (Dimension*)0);
  }

  ncolumns=part->n_columns;
  nrows=part->n_rows;

  if (part->has_hslack &&
      width > part->column_coord[ncolumns]) {
    /* if the slack can take some damage, let it */
    part->horizontal_slack = width - part->column_coord[ncolumns];
  } else if (width > XfwfArraySizesWidth(part)
	     && parent->xfwf_array.expandable_columns
	     && !parent->xfwf_array.same_width) {
    /* if we have some expandable columns, let them expand! */
    Boolean	*expandable = parent->xfwf_array.expandable_columns;
    int	slack = width - XfwfArraySizesWidth(part);
    int	total=0;
    for (i=0; i<ncolumns; i++)
      if (expandable[i]) {
	if (part->column_widths[i] <1)
	  part->column_widths[i] = 1;
	total += part->column_widths[i];
      }
    for (i=0; i<ncolumns; i++) {
      int	adj;
      if (!expandable[i])
	continue;
      adj = (slack+0.5) * part->column_widths[i] / total;
      total -= part->column_widths[i];
      slack -= adj;
      part->column_widths[i] += adj;
    }
    ComputeSums(parent, part);
  } else {
    /* distribute change evenly among the children */
    ideal = part->column_coord[ncolumns];
    curr = width;
    for (i=0; i<=ncolumns; i++) {
      int	spacing = parent->xfwf_array.horizontal_spacing_a[i];
      ideal -= spacing;
      curr -= spacing;
    }
    for (i=0; i<ncolumns; i++) {
      int	old = part->column_widths[i];
      int	new;

      new = ideal>0 ? old * curr / ideal : 0;
      curr -= new;
      ideal -= old;
      part->column_widths[i] = new >1 ? new : 1;
    }
    part->horizontal_slack = 0;
  }


  if (part->has_vslack &&
      height > part->row_coord[nrows]) {
    /* if the slack can take some damage, let it */
    part->vertical_slack = height - part->row_coord[nrows];
  } else if (height > XfwfArraySizesHeight(part)
      && parent->xfwf_array.expandable_rows
      && !parent->xfwf_array.same_height) {
  /* if we have some expandable rows, let them expand! */
    Boolean	*expandable = parent->xfwf_array.expandable_rows;
    int	slack = height - XfwfArraySizesHeight(part);
    int	total=0;
    for (i=0; i<nrows; i++)
      if (expandable[i]) {
	if (part->row_heights[i] <1)
	  part->row_heights[i] = 1;
	total += part->row_heights[i];
      }
    for (i=0; i<nrows; i++) {
      int	adj;
      if (!expandable[i])
	continue;
      adj = (slack+0.5) * part->row_heights[i] / total;
      total -= part->row_heights[i];
      slack -= adj;
      part->row_heights[i] += adj;
    }
    ComputeSums(parent, part);
  } else {
    ideal = part->row_coord[nrows];
    curr = height;
    for (i=0; i<=nrows; i++) {
      int spacing = parent->xfwf_array.vertical_spacing_a[i];
      ideal -= spacing;
      curr -= spacing;
    }
    for (i=0; i<nrows; i++) {
      int	old = part->row_heights[i];
      int	new;

      new = ideal>0 ? old * curr / ideal : 0;
      curr -= new;
      ideal -= old;
      part->row_heights[i] = new >1 ? new : 1;
    }
    part->vertical_slack = 0;
  }

  ComputeSums(parent, part);
}

/* calculate a child's position */
#if ! NeedFunctionPrototypes
static void CalcChildPosition(child, sizes)
     RectObj child;
     XfwfArraySizesPart *sizes;
#else /* NeedFunctionPrototypes */
static void CalcChildPosition(RectObj child, XfwfArraySizesPart *sizes)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayConstraintsPart	*cnstr =
    &((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;
  int	x,y;
  int	ww,wh;			/* widget dimensions */
  int	cw, rh;			/* cell dimensions */
  int	bd = cnstr->border = cnstr->d_border;

  x = sizes->column_coord[cnstr->column];
  ww = cnstr->d_width + bd*2;
  cw = AvailableWidth(child, sizes);
  if (ww>cw)			/* no exceeding bounds */
    ww = cw;
  switch(cnstr->hjust) {
  case XfwfHJleft:
    /* everything is already set */
    break;
  case XfwfHJright:
    x += cw - ww;
    break;
  case XfwfHJfill:
    ww = cw;
    break;
  case XfwfHJcenter:
  default:
    x += (cw - ww)/2;
    break;
  }
  cnstr->x = x;
  ww -= bd*2;
  cnstr->width = ww<1 ? 1 : ww;

  y = sizes->row_coord[cnstr->row];
  wh = cnstr->d_height + bd*2;
  rh = AvailableHeight(child, sizes);
  if (wh>rh)			/* no exceeding bounds */
    wh = rh;
  switch (cnstr->vjust) {
  case XfwfVJtop:
    /* everything is already set */
    break;
  case XfwfVJbottom:
    y += rh - wh;
    break;
  case XfwfVJfill:
    wh = rh;
    break;
  case XfwfVJcenter:
  default:
    y += (rh - wh)/2;
    break;
  }
  cnstr->y = y;
  wh -= bd*2;
  cnstr->height = wh<1 ? 1 : wh;
}

#if ! NeedFunctionPrototypes
static void CalcChildrenPosition(parent)
     XfwfArrayWidget parent;
#else /* NeedFunctionPrototypes */
static void CalcChildrenPosition(XfwfArrayWidget parent)
#endif /* NeedFunctionPrototypes */
{
  int	i;

  /* compute positions for all the widgets */
  for (i=0; i<parent->composite.num_children; i++) {

    if (!XtIsManaged(parent->composite.children[i]))
      continue;

    CalcChildPosition((RectObj)parent->composite.children[i],
		      &parent->xfwf_array.actual);
  }
}

#if ! NeedFunctionPrototypes
static void LoadCurrentGeometryAsDesired(parent)
     XfwfArrayWidget parent;
#else /* NeedFunctionPrototypes */
static void LoadCurrentGeometryAsDesired(XfwfArrayWidget parent)
#endif /* NeedFunctionPrototypes */
{
  int	i;

  for (i=0; i<parent->composite.num_children; i++) {
    RectObj	child = (RectObj)parent->composite.children[i];
    XfwfArrayConstraintsPart	*cnstr =
      &((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;

    if (!XtIsManaged((Widget)child))
      continue;

    if (cnstr->d_width<=0)
      cnstr->d_width = child->rectangle.width;
    if (cnstr->d_height<=0)
      cnstr->d_height = child->rectangle.height;
    if (cnstr->d_border<=0)
      cnstr->d_border = child->rectangle.border_width;
  }
}

#if ! NeedFunctionPrototypes
static void AcceptProposedLayout(parent)
     XfwfArrayWidget parent;
#else /* NeedFunctionPrototypes */
static void AcceptProposedLayout(XfwfArrayWidget parent)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayPart	*part = &parent->xfwf_array;
  int	need_redisplay=0;

  if (XtIsRealized((Widget)parent)) {
    if (!part->actual.row_heights)
      need_redisplay = 1;

    if (!need_redisplay) {
      int	i;
      for (i=0; i<part->n_columns; i++) {
	need_redisplay |= (part->actual.column_widths[i] !=
			   part->proposed.column_widths[i]);
      }
      for (i=0; i<part->n_rows; i++) {
	need_redisplay |= (part->actual.row_heights[i] !=
			   part->proposed.row_heights[i]);
      }
    }
  }

  free_XfwfArraySizesPart(&part->actual);
  memcpy(&part->actual, &part->proposed,
	 sizeof(XfwfArraySizesPart));
  zero_XfwfArraySizesPart(&part->proposed);

  if (need_redisplay)
    Redisplay((Widget)parent, (XEvent*)0, (Region)0);
}

#if ! NeedFunctionPrototypes
static void LayoutChild(child)
     RectObj child;
#else /* NeedFunctionPrototypes */
static void LayoutChild(RectObj child)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayConstraintsPart	*cnstr =
    &((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;

  XtConfigureWidget((Widget)child, cnstr->x, cnstr->y,
		    cnstr->width, cnstr->height, cnstr->border);
}

#if ! NeedFunctionPrototypes
static void PerformLayout(parent)
     XfwfArrayWidget parent;
#else /* NeedFunctionPrototypes */
static void PerformLayout(XfwfArrayWidget parent)
#endif /* NeedFunctionPrototypes */
{
  int	i;
  for (i=0; i<parent->composite.num_children; i++) {
    if (!XtIsManaged(parent->composite.children[i]))
      continue;
    LayoutChild((RectObj)parent->composite.children[i]);
  }
}

#if ! NeedFunctionPrototypes
static void AttemptReconfigure(parent)
     XfwfArrayWidget parent;
#else /* NeedFunctionPrototypes */
static void AttemptReconfigure(XfwfArrayWidget parent)
#endif /* NeedFunctionPrototypes */
{
  Dimension	width, height;

  ComputeIdeal(parent, &width, &height);

  if (width!=parent->core.width ||
      height!=parent->core.height) {
    /* urgh, I have to ask my parent for a geometry change. */
    XtGeometryResult	rval;
    Dimension	n_w, n_h;

    rval = XtMakeResizeRequest((Widget)parent, width, height, &n_w, &n_h);

    switch (rval) {
    case XtGeometryAlmost:
      width = n_w;
      height = n_h;
      XtMakeResizeRequest((Widget)parent, width, height, &n_w, &n_h);
      /* don't need to check return value, Xt doc sect. 6.5 */
      break;
    case XtGeometryYes:		/* cool! */
    case XtGeometryDone:
      break;
    case XtGeometryNo:		/* drat */
      width = parent->core.width;
      height = parent->core.height;
      break;
    }
    /* width and height are our current dimensions */
    ComputeCompromise(parent, width, height);

  }
  /* urgh, causes gratuitous redraw */
  AcceptProposedLayout(parent);
  CalcChildrenPosition(parent);
  PerformLayout(parent);
}

#if ! NeedFunctionPrototypes
Dimension AvailableWidth(child, part)
     RectObj child;
     XfwfArraySizesPart *part;
#else /* NeedFunctionPrototypes */
Dimension AvailableWidth(RectObj child, XfwfArraySizesPart *part)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	parent = (XfwfArrayWidget)child->object.parent;
  XfwfArrayConstraintsPart	*cnstr =
    &((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;
  int	i;
  Dimension	rval=0;

  for (i=cnstr->column;
       (cnstr->ncolumns<1 || i<cnstr->column+cnstr->ncolumns)
       && i < part->n_columns;
       i++)
    rval += part->column_widths[i];

  for (i=1+cnstr->column;
       (cnstr->ncolumns<1 || i<cnstr->column+cnstr->ncolumns)
       && i < part->n_columns;
       i++)
    rval += parent->xfwf_array.horizontal_spacing_a[i];

  if (cnstr->ncolumns<0 && part->has_hslack)
    rval += part->horizontal_slack;

  return rval;
}

#if ! NeedFunctionPrototypes
Dimension AvailableHeight(child, part)
     RectObj child;
     XfwfArraySizesPart *part;
#else /* NeedFunctionPrototypes */
Dimension AvailableHeight(RectObj child, XfwfArraySizesPart *part)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	parent = (XfwfArrayWidget)child->object.parent;
  XfwfArrayConstraintsPart	*cnstr =
    &((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;
  int	i;
  Dimension	rval=0;

  for (i=cnstr->row;
       (cnstr->nrows<1 || i<cnstr->row+cnstr->nrows)
       && i<part->n_rows;
       i++)
    rval += part->row_heights[i];

  for (i=1+cnstr->row;
       (cnstr->nrows<1 || i<cnstr->row+cnstr->nrows)
       && i<part->n_rows;
       i++)
    rval += parent->xfwf_array.vertical_spacing_a[i];

  if (cnstr->nrows < 0 && part->has_vslack)
    rval += part->vertical_slack;

  return rval;
}

/**********************************************************************/

#define BETWEEN(a,b,c) (((a)<(c)) ? ( (a)<=(b) && (b)<=(c)) : ((a)>=(b) && (b)>=(c)))

#if ! NeedFunctionPrototypes
static XtGeometryResult GeometryManager(w, request, ret)
     Widget w;
     XtWidgetGeometry *request;
     XtWidgetGeometry *ret;
#else /* NeedFunctionPrototypes */
static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry *request, XtWidgetGeometry *ret)
#endif /* NeedFunctionPrototypes */
{
  RectObj child = (RectObj)w;
  XfwfArrayWidget	parent = (XfwfArrayWidget)child->object.parent;
  XfwfArrayConstraintsPart	*save =
    &((XfwfArrayConstraintsRec*)child->object.constraints)->xfwf_array;
  XtGeometryMask	mode = request->request_mode;
  int	optimistic, pessimistic;

  /* 0 means only the child
     1 means everyone :( */
  int	relayout_required;


  XfwfArrayConstraintsRec	temp;
  XfwfArrayConstraintsPart	*tempp = &temp.xfwf_array;

  if (save->CSVhook) {
    /* ok, a setvalues is triggering a Resize request.  The toolkit
       does not provide us with the proper mechanisms to handle this
       stuff robustly. */
    int mode = save->CSVhook;	/* shadow local */
    save->CSVhook=0;
    if (mode&CWX)
      child->rectangle.x = save->x;
    if (mode&CWY)
      child->rectangle.y = save->y;
    if (mode&CWWidth)
      child->rectangle.width = save->width;
    if (mode&CWHeight)
      child->rectangle.height = save->height;

    request->request_mode &= ~(mode);
    return request->request_mode ? GeometryManager(w, request, ret)
      : XtGeometryYes;
  }

  memcpy(&temp, save, sizeof(temp));
  child->object.constraints = (XtPointer)&temp;

  if (mode&CWWidth)
    tempp->d_width = request->width;
  if (mode&CWHeight)
    tempp->d_height = request->height;
  if (mode&CWBorderWidth)
    tempp->d_border = request->border_width;

  if (   ( !(mode&(CWWidth|CWBorderWidth))
	  || tempp->d_width + 2*tempp->d_border
	  <= AvailableWidth(child, &parent->xfwf_array.actual))
      && ( !(mode&(CWHeight|CWBorderWidth))
	  || tempp->d_height + 2*tempp->d_border
	  <= AvailableHeight(child, &parent->xfwf_array.actual))) {
    relayout_required = 0;
  } else {
    Dimension	ideal_width, ideal_height;
    ComputeIdeal(parent, &ideal_width, &ideal_height);
    if (ideal_width == parent->core.width &&
	ideal_height == parent->core.height) {
      /* cool */
    } else {
      ComputeCompromise(parent, parent->core.width, parent->core.height);
      if (tempp->d_width + 2*tempp->d_border
	  <= AvailableWidth(child, &parent->xfwf_array.proposed)
	  && tempp->d_height + 2*tempp->d_border
	  <= AvailableHeight(child, &parent->xfwf_array.proposed)) {
	/* cool we can fit in a compromise */
      } else {
	XtWidgetGeometry	req, repl;
	XtGeometryResult	rval;

	req.request_mode = CWWidth|CWHeight| (mode&XtCWQueryOnly);
	req.width = ideal_width;
	req.height = ideal_height;
	rval = XtMakeGeometryRequest((Widget)parent, &req, &repl);

	switch (rval) {
	case XtGeometryAlmost:
	  if (BETWEEN(parent->core.width, repl.width, ideal_width) &&
	      BETWEEN(parent->core.height,repl.height,ideal_height)) {
	    XtMakeGeometryRequest((Widget)parent, &req, (XtWidgetGeometry*)0);
	  }
	  break;
	case XtGeometryYes:	/* cool! */
	case XtGeometryDone:
	  break;
	case XtGeometryNo:	/* drat */
	  break;
	}
	ComputeIdeal(parent, &ideal_width, &ideal_height);
	ComputeCompromise(parent, parent->core.width, parent->core.height);
      }
    }
    relayout_required = 1;
  }
  CalcChildPosition(child,
		    relayout_required
		    ? &parent->xfwf_array.proposed
		    : &parent->xfwf_array.actual);

  optimistic = 1;
  pessimistic = 1;
  if (mode & CWX) {
    if (request->x != tempp->x)
      optimistic=0;
    else if (child->rectangle.x != tempp->x)
      pessimistic = 0;
  }
  if (mode & CWY) {
    if (request->y != tempp->y)
      optimistic=0;
    else if (child->rectangle.y != tempp->y)
      pessimistic = 0;
  }
  if (mode & (CWWidth|CWBorderWidth)) {
    if (tempp->width != request->width)
      optimistic = 0;
    else if (tempp->width != child->rectangle.width)
      pessimistic = 0;
  }
  if (mode & (CWHeight|CWBorderWidth)) {
    if (tempp->height != request->height)
      optimistic = 0;
    else if (tempp->height != child->rectangle.height)
      pessimistic = 0;
  }

  if (!(mode & XtCWQueryOnly)) {
    /* the child genuinely desires this */
    memcpy(save, &temp, sizeof(temp));
  }

  child->object.constraints = (XtPointer)save;

  if (optimistic) {
    if (mode & XtCWQueryOnly) {
      return XtGeometryYes;
    } else {
      if (relayout_required) {
	AcceptProposedLayout(parent);
	CalcChildrenPosition(parent);
	PerformLayout(parent);
      } else {
	LayoutChild(child);
      }
      return XtGeometryDone;
    }
  } else if (pessimistic)
    return XtGeometryNo;
  else {
    ret->request_mode = 0;
    if (tempp->x != child->rectangle.x) {
      ret->x = tempp->x;
      ret->request_mode |= CWX;
    }
    if (tempp->y != child->rectangle.y) {
      ret->y = tempp->y;
      ret->request_mode |= CWY;
    }
    if (tempp->width != child->rectangle.width) {
      ret->width = tempp->width;
      ret->request_mode |= CWWidth;
    }
    if (tempp->height != child->rectangle.height) {
      ret->height = tempp->height;
      ret->request_mode |= CWHeight;
    }
    if (tempp->border != child->rectangle.border_width) {
      ret->border_width = tempp->border;
      ret->request_mode |= CWBorderWidth;
    }
    return XtGeometryAlmost;
  }
}

/* convenience procedures *********************************************/

#if ! NeedFunctionPrototypes
void XfwfArray_AppendVerticalSpacing(w, spacing)
     Widget w;
     Dimension spacing;
#else /* NeedFunctionPrototypes */
void XfwfArray_AppendVerticalSpacing(Widget w, Dimension spacing)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array;

  if (!XtIsSubclass(w, xfwfarrayWidgetClass)) {
    XtAppWarningMsg(XtWidgetToApplicationContext(w), "NotSubclass",
		    "NotXfwfArrayWidget", "XfwfLibraryError",
		    "AppendVerticalSpacing: not an Array Widget",
		    (String*)0, (Cardinal*)0);
    return;
  }

  array = (XfwfArrayWidget)w;

  EnsureSpace(array);

  array->xfwf_array.vertical_spacing_a[array->xfwf_array.n_rows] = spacing;

  AttemptReconfigure(array);
}


#if ! NeedFunctionPrototypes
void XfwfArray_AppendHorizontalSpacing(w, spacing)
     Widget w;
     Dimension spacing;
#else /* NeedFunctionPrototypes */
void XfwfArray_AppendHorizontalSpacing(Widget w, Dimension spacing)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array;

  if (!XtIsSubclass(w, xfwfarrayWidgetClass)) {
    XtAppWarningMsg(XtWidgetToApplicationContext(w), "NotSubclass",
		    "NotXfwfArrayWidget", "XfwfLibraryError",
		    "AppendHorizontalSpacing: not an Array Widget",
		    (String*)0, (Cardinal*)0);
    return;
  }

  array = (XfwfArrayWidget)w;

  EnsureSpace(array);

  array->xfwf_array.horizontal_spacing_a[array->xfwf_array.n_columns]
    = spacing;

  AttemptReconfigure(array);
}

/**********************************************************************/

#if ! NeedFunctionPrototypes
static void AppendWidgetToColumn(child)
     Widget child;
#else /* NeedFunctionPrototypes */
static void AppendWidgetToColumn(Widget child)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array = (XfwfArrayWidget)child->core.parent;
  WidgetList	children = array->composite.children;
  int		nchildren = array->composite.num_children;
  XfwfArrayConstraintsPart	*cnstr =
    &((XfwfArrayConstraintsRec*)child->core.constraints)->xfwf_array;
  int	col = cnstr->column;
  int ncols = cnstr->ncolumns;
  int	row=0;
  int	i;

  for (i=0; i<nchildren; i++) {
    Widget	w = children[i];
    XfwfArrayConstraintsPart *cnstr = /* shadow local */
      &((XfwfArrayConstraintsRec*)w->core.constraints)->xfwf_array;
    if (w==child)
      continue;			/* only interested in siblings */
    if ( (cnstr->ncolumns <1 || cnstr->column+cnstr->ncolumns > col)
	&& ( ncols<1 || cnstr->column < col+ncols )
	/* the widgets' columns overlap */
	&& (cnstr->row+cnstr->nrows > row)) {
      row = cnstr->row+cnstr->nrows;
    }
  }

  {
    Arg	arg;
    arg.name = XtNrow;
    arg.value = row;
    XtSetValues(child, &arg, (Cardinal)1);
  }
}

#if NeedFunctionPrototypes
Widget XfwfArray_AppendWidgetToColumn(_Xconst char* name, WidgetClass wclass, Widget parent, ArgList args, Cardinal nargs)
#else
Widget XfwfArray_AppendWidgetToColumn(name, wclass, parent,
				      args, nargs)
     String	name;
     WidgetClass	wclass;
     Widget	parent;
     ArgList	args;
     Cardinal	nargs;
#endif
{
  Widget	child;

  child = XtCreateManagedWidget(name, wclass, parent, args, nargs);

  if (!XtIsSubclass(parent, xfwfarrayWidgetClass)) {
    XtAppWarningMsg(XtWidgetToApplicationContext(parent), "NotSubclass",
		    "NotXfwfArrayWidget", "XfwfLibraryError",
		    "AppendWidgetToColumn: not an Array Widget",
		    (String*)0, (Cardinal*)0);
  } else {
    AppendWidgetToColumn(child);
  }

  return child;
}

#if NeedVarargsPrototypes
Widget XfwfArray_VaAppendWidgetToColumn
(
 _Xconst char* name,
 WidgetClass wclass,
 Widget parent,
 ...)
#else
Widget XfwfArray_VaAppendWidgetToColumn(name, wclass, parent,
					va_alist)
     String	name;
     WidgetClass	wclass;
     Widget	parent;
     va_dcl
#endif
{
#if 1
  va_list	var;
  XtVarArgsList	list;

  Va_start(var, parent);
  list = XfwfCreateArgsList(var);
  va_end(var);

  return XfwfArray_VanlAppendWidgetToColumn(name, wclass, parent, list);
#else
  va_list	var;
  Widget	child;
  int	total_count, typed_count;
  XtTypedArgList	typed_args = 0;
  Cardinal	num_args;

  Va_start(var, parent);
  _XtCountVaList(var, &total_count, &typed_count);
  va_end(var);

  Va_start(var, parent);

  _XtVaToTypedArgList(var, total_count, &typed_args, &num_args);

  child = _XtCreateWidget(name, wclass, parent, (ArgList)0, (Cardinal)0,
			  typed_args, num_args);
  XtManageChild(child);
  va_end(var);

  if (!XtIsSubclass(parent, xfwfarrayWidgetClass)) {
    XtAppWarningMsg(XtWidgetToApplicationContext(parent), "NotSubclass",
		    "NotXfwfArrayWidget", "XfwfLibraryError",
		    "AppendWidgetToColumn: not an Array Widget",
		    (String*)0, (Cardinal*)0);
  } else {
    AppendWidgetToColumn(child);
  }

  return child;
#endif
}

#if ! NeedFunctionPrototypes
Widget XfwfArray_VanlAppendWidgetToColumn(name, wclass, parent, args)
     String name;
     WidgetClass wclass;
     Widget parent;
     XtVarArgsList args;
#else /* NeedFunctionPrototypes */
Widget XfwfArray_VanlAppendWidgetToColumn(_Xconst char *name, WidgetClass wclass, Widget parent, XtVarArgsList args)
#endif /* NeedFunctionPrototypes */
{
  Widget	child;

  child = XtVaCreateManagedWidget(name, wclass, parent,
				  XtVaNestedList, args,
				  (String)0);

  if (!XtIsSubclass(parent, xfwfarrayWidgetClass)) {
    XtAppWarningMsg(XtWidgetToApplicationContext(parent), "NotSubclass",
		    "NotXfwfArrayWidget", "XfwfLibraryError",
		    "AppendWidgetToColumn: not an Array Widget",
		    (String*)0, (Cardinal*)0);
  } else {
    AppendWidgetToColumn(child);
  }

  return child;
}

			 /****/

#if ! NeedFunctionPrototypes
static void AppendWidgetToRow(child)
     Widget child;
#else /* NeedFunctionPrototypes */
static void AppendWidgetToRow(Widget child)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array = (XfwfArrayWidget)child->core.parent;
  WidgetList	children = array->composite.children;
  int		nchildren = array->composite.num_children;
  XfwfArrayConstraintsPart	*cnstr =
    &((XfwfArrayConstraintsRec*)child->core.constraints)->xfwf_array;
  int	row = cnstr->row;
  int nrows = cnstr->nrows;
  int	column=0;
  int	i;

  for (i=0; i<nchildren; i++) {
    Widget	w = children[i];
    XfwfArrayConstraintsPart *cnstr = /* shadow local */
      &((XfwfArrayConstraintsRec*)w->core.constraints)->xfwf_array;
    if (w==child)
      continue;			/* only interested in siblings */
    if ( (cnstr->nrows <1 || cnstr->row+cnstr->nrows > row)
	&& ( nrows<1 || cnstr->row < row+nrows )
	/* the widgets' rows overlap */
	&& (cnstr->column+cnstr->ncolumns > column)) {
      column = cnstr->column+cnstr->ncolumns;
    }
  }

  {
    Arg	arg;
    arg.name = XtNcolumn;
    arg.value = column;
    XtSetValues(child, &arg, (Cardinal)1);
  }
}

#if ! NeedFunctionPrototypes
Widget XfwfArray_AppendWidgetToRow(name, wclass, parent, args, nargs)
     String name;
     WidgetClass wclass;
     Widget parent;
     ArgList args;
     Cardinal nargs;
#else /* NeedFunctionPrototypes */
Widget XfwfArray_AppendWidgetToRow(_Xconst char *name, WidgetClass wclass, Widget parent, ArgList args, Cardinal nargs)
#endif /* NeedFunctionPrototypes */
{
  Widget	child;

  child = XtCreateManagedWidget(name, wclass, parent, args, nargs);

  if (!XtIsSubclass(parent, xfwfarrayWidgetClass)) {
    XtAppWarningMsg(XtWidgetToApplicationContext(parent), "NotSubclass",
		    "NotXfwfArrayWidget", "XfwfLibraryError",
		    "AppendWidgetToRow: not an Array Widget",
		    (String*)0, (Cardinal*)0);
  } else {
    AppendWidgetToRow(child);
  }

  return child;
}

#if ! NeedFunctionPrototypes
Widget XfwfArray_VanlAppendWidgetToRow(name, wclass, parent, args)
     String name;
     WidgetClass wclass;
     Widget parent;
     XtVarArgsList args;
#else /* NeedFunctionPrototypes */
Widget XfwfArray_VanlAppendWidgetToRow(_Xconst char *name, WidgetClass wclass, Widget parent, XtVarArgsList args)
#endif /* NeedFunctionPrototypes */
{
  Widget	child;

  child = XtVaCreateManagedWidget(name, wclass, parent,
				  XtVaNestedList, args,
				  (String)0);

  if (!XtIsSubclass(parent, xfwfarrayWidgetClass)) {
    XtAppWarningMsg(XtWidgetToApplicationContext(parent), "NotSubclass",
		    "NotXfwfArrayWidget", "XfwfLibraryError",
		    "AppendWidgetToRow: not an Array Widget",
		    (String*)0, (Cardinal*)0);
  } else {
    AppendWidgetToRow(child);
  }

  return child;
}

#if NeedVarargsPrototypes
Widget XfwfArray_VaAppendWidgetToRow
(
 _Xconst char* name,
 WidgetClass wclass,
 Widget parent,
 ...)
#else
Widget XfwfArray_VaAppendWidgetToRow(name, wclass, parent,
				     va_alist)
     String	name;
     WidgetClass	wclass;
     Widget	parent;
     va_dcl
#endif
{
  va_list	var;
  XtVarArgsList	list;

  Va_start(var, parent);
  list = XfwfCreateArgsList(var);
  va_end(var);

  return XfwfArray_VanlAppendWidgetToRow(name, wclass, parent, list);
}

/**********************************************************************/

#if ! NeedFunctionPrototypes
void XfwfArray_SetHorizontalSpacing(w, column, spacing)
     Widget w;
     int column;
     int spacing;
#else /* NeedFunctionPrototypes */
void XfwfArray_SetHorizontalSpacing(Widget w, int column, int spacing)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array;

  if (!XtIsSubclass(w, xfwfarrayWidgetClass)) {
    XtAppWarningMsg(XtWidgetToApplicationContext(w), "NotSubclass",
		    "NotXfwfArrayWidget", "XfwfLibraryError",
		    "SetHorizontalSpacing: not an Array Widget",
		    (String*)0, (Cardinal*)0);
    return;
  }

  array = (XfwfArrayWidget)w;

  EnsureSpace(array);
  if (column<0 || column> array->xfwf_array.n_columns)
    return;

  array->xfwf_array.horizontal_spacing_a[column] = spacing;

  AttemptReconfigure(array);
}

#if ! NeedFunctionPrototypes
void XfwfArray_SetVerticalSpacing(w, row, spacing)
     Widget w;
     int row;
     int spacing;
#else /* NeedFunctionPrototypes */
void XfwfArray_SetVerticalSpacing(Widget w, int row, int spacing)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array;

  if (!XtIsSubclass(w, xfwfarrayWidgetClass)) {
    XtAppWarningMsg(XtWidgetToApplicationContext(w), "NotSubclass",
		    "NotXfwfArrayWidget", "XfwfLibraryError",
		    "SetVerticalSpacing: not an Array Widget",
		    (String*)0, (Cardinal*)0);
    return;
  }

  array = (XfwfArrayWidget)w;

  EnsureSpace(array);
  if (row<0 || row> array->xfwf_array.n_rows)
    return;

  array->xfwf_array.vertical_spacing_a[row] = spacing;

  AttemptReconfigure(array);
}

/**********************************************************************/

#if ! NeedFunctionPrototypes
void XfwfArray_SetExpandableColumn(w, column, expandableP)
     Widget w;
     int column;
     int expandableP;
#else /* NeedFunctionPrototypes */
void XfwfArray_SetExpandableColumn(Widget w, int column, int expandableP)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array;

  if (!XtIsSubclass(w, xfwfarrayWidgetClass)) {
    XtAppWarningMsg(XtWidgetToApplicationContext(w), "NotSubclass",
		    "NotXfwfArrayWidget", "XfwfLibraryError",
		    "SetExpandableColumn: not an Array Widget",
		    (String*)0, (Cardinal*)0);
    return;
  }

  array = (XfwfArrayWidget)w;

  EnsureSpace(array);
  if (column<0 || column>= array->xfwf_array.n_columns)
    return;

  array->xfwf_array.expandable_columns[column] = expandableP;

  AttemptReconfigure(array);
}

#if ! NeedFunctionPrototypes
void XfwfArray_SetExpandableRow(w, row, expandableP)
     Widget w;
     int row;
     int expandableP;
#else /* NeedFunctionPrototypes */
void XfwfArray_SetExpandableRow(Widget w, int row, int expandableP)
#endif /* NeedFunctionPrototypes */
{
  XfwfArrayWidget	array;

  if (!XtIsSubclass(w, xfwfarrayWidgetClass)) {
    XtAppWarningMsg(XtWidgetToApplicationContext(w), "NotSubclass",
		    "NotXfwfArrayWidget", "XfwfLibraryError",
		    "SetExpandableRow: not an Array Widget",
		    (String*)0, (Cardinal*)0);
    return;
  }

  array = (XfwfArrayWidget)w;

  EnsureSpace(array);
  if (row<0 || row>= array->xfwf_array.n_rows)
    return;

  array->xfwf_array.expandable_rows[row] = expandableP;

  AttemptReconfigure(array);
}
