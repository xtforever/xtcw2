/*
    FileComp.h: public header file for an array widget
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

#ifndef _Xfwf_Array_h
#define _Xfwf_Array_h

#include <X11/Intrinsic.h>
#include <X11/Constraint.h>
#include <Xfwf/Pen.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XfwfArrayClassRec * XfwfArrayWidgetClass;
extern WidgetClass xfwfarrayWidgetClass;
typedef struct _XfwfArrayRec *XfwfArrayWidget;

/* array resources */

#ifndef XtNnumRows
#define XtNnumRows	"numRows"
#endif
#ifndef XtCNumRows
#define XtCNumRows	"NumRows"
#endif

#ifndef XtNnumColumns
#define XtNnumColumns	"numColumns"
#endif
#ifndef XtCNumColumns
#define XtCNumColumns	"NumColumns"
#endif

#ifndef XtNsameWidth
#define XtNsameWidth	"sameWidth"
#endif
#ifndef XtCSameWidth
#define XtCSameWidth	"SameWidth"
#endif

#ifndef XtNsameHeight
#define XtNsameHeight	"sameHeight"
#endif
#ifndef XtCSameHeight
#define XtCSameHeight	"SameHeight"
#endif

#ifndef XtNverticalSpacing
#define XtNverticalSpacing	"verticalSpacing"
#endif
#ifndef XtCVerticalSpacing
#define XtCVerticalSpacing	"VerticalSpacing"
#endif

#ifndef XtNhorizontalSpacing
#define XtNhorizontalSpacing	"horizontalSpacing"
#endif
#ifndef XtCHorizontalSpacing
#define XtCHorizontalSpacing	"HorizontalSpacing"
#endif

#ifndef XtRDimensionList
#define XtRDimensionList	"DimensionList"
typedef	Dimension	*DimensionList;
#endif

#ifndef XtNverticalSpacingArray
#define XtNverticalSpacingArray	"verticalSpacingArray"
#endif
#ifndef XtCVerticalSpacingArray
#define XtCVerticalSpacingArray	"VerticalSpacingArray"
#endif

#ifndef XtNhorizontalSpacingArray
#define XtNhorizontalSpacingArray	"horizontalSpacingArray"
#endif
#ifndef XtCHorizontalSpacingArray
#define XtCHorizontalSpacingArray	"HorizontalSpacingArray"
#endif

#ifndef XtNdrawShadows
#define XtNdrawShadows	"drawShadows"
#endif
#ifndef XtcDrawShadows
#define XtCDrawShadows	"DrawShadows"
#endif

/* constraint resources */

#ifndef XtNrow
#define XtNrow	"row"
#endif
#ifndef XtCRow
#define XtCRow	"Row"
#endif

#ifndef XtNrowSpan
#define XtNrowSpan	"rowSpan"
#endif
#ifndef XtCRowSpan
#define XtCRowSpan	"RowSpan"
#endif
/* value meaning that the widget covers all cells to the end of the
   row/column */
#define	XfwfArrayHorizon	((XtArgVal)0)
/* value meaning that the widget covers all cells to the end of the
   row/column and beyond */
#define	XfwfArrayInfinity	((XtArgVal)-1)

#ifndef XtNcolumn
#define XtNcolumn	"column"
#endif
#ifndef XtCcolumn
#define XtCColumn	"Column"
#endif

#ifndef XtNcolumnSpan
#define XtNcolumnSpan	"columnSpan"
#endif
#ifndef XtCColumnSpan
#define XtCColumnSpan	"ColumnSpan"
#endif

typedef enum _XfwfVerticalJustify {
  XfwfVJtop,
  XfwfVJbottom,
  XfwfVJcenter,
  XfwfVJfill,
} XfwfVerticalJustify;
#ifndef XtNverticalJustify
#define XtNverticalJustify	"verticalJustify"
#endif
#ifndef XtCVerticalJustify
#define XtCVerticalJustify	"VerticalJustify"
#endif
#ifndef XtRVerticalJustify
#define XtRVerticalJustify	"VerticalJustify"
#endif

typedef enum _XfwfHorizontalJustify {
  XfwfHJleft,
  XfwfHJright,
  XfwfHJcenter,
  XfwfHJfill,
} XfwfHorizontalJustify;
#ifndef XtNhorizontalJustify
#define XtNhorizontalJustify	"horizontalJustify"
#endif
#ifndef XtCHorizontalJustify
#define XtCHorizontalJustify	"HorizontalJustify"
#endif
#ifndef XtRHorizontalJustify
#define XtRHorizontalJustify	"HorizontalJustify"
#endif

Widget XfwfArray_AppendWidgetToColumn
  (
#if NeedFunctionPrototypes
   _Xconst char*		/* name */,
   WidgetClass			/* wclass */,
   Widget			/* parent */,
   ArgList			/* args */,
   Cardinal			/* nargs */
#endif
   );

Widget XfwfArray_VanlAppendWidgetToColumn
  (
#if NeedFunctionPrototypes
   _Xconst char*		/* name */,
   WidgetClass			/* wclass */,
   Widget			/* parent */,
   XtVarArgsList		/* args */
#endif
   );

Widget XfwfArray_VaAppendWidgetToColumn
  (
#if NeedVarargsPrototypes
   _Xconst char*		/* name */,
   WidgetClass			/* wclass */,
   Widget			/* parent */,
   ...
#endif
   );

Widget XfwfArray_AppendWidgetToRow
  (
#if NeedFunctionPrototypes
   _Xconst char*		/* name */,
   WidgetClass			/* wclass */,
   Widget			/* parent */,
   ArgList			/* args */,
   Cardinal			/* nargs */
#endif
   );

Widget XfwfArray_VanlAppendWidgetToRow
  (
#if NeedFunctionPrototypes
   _Xconst char*		/* name */,
   WidgetClass			/* wclass */,
   Widget			/* parent */,
   XtVarArgsList		/* args */
#endif
   );

Widget XfwfArray_VaAppendWidgetToRow
  (
#if NeedVarargsPrototypes
   _Xconst char*		/* name */,
   WidgetClass			/* wclass */,
   Widget			/* parent */,
   ...
#endif
   );

void XfwfArray_SetHorizontalSpacing
  (
#if NeedFunctionPrototypes
   Widget			/* w */,
   int				/* column */,
   int				/* spacing */
#endif
   );
void XfwfArray_SetVerticalSpacing
  (
#if NeedFunctionPrototypes
   Widget			/* w */,
   int				/* row */,
   int				/* spacing */
#endif
   );
void XfwfArray_AppendHorizontalSpacing
  (
#if NeedFunctionPrototypes
   Widget			/*w*/,
   Dimension			/* spacing */
#endif
   );
void XfwfArray_AppendVerticalSpacing
  (
#if NeedFunctionPrototypes
   Widget			/*w*/,
   Dimension			/* spacing */
#endif
   );

void XfwfArray_SetExpandableColumn
  (
#if NeedFunctionPrototypes
   Widget			/*w*/,
   int				/*column*/,
   int				/*expandableP*/
#endif
   );
void XfwfArray_SetExpandableRow
  (
#if NeedFunctionPrototypes
   Widget			/*w*/,
   int				/*row*/,
   int				/*expandableP*/
#endif
   );

#ifdef __cplusplus
}	/* extern C */
#endif

#endif
