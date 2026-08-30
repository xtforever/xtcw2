/*
    ArrayP.h: public header file for an array widget
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

#ifndef _Xfwf_ArrayP_h
#define _Xfwf_ArrayP_h

#include <X11/ConstrainP.h>
#include <Xfwf/Array.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _XfwfArrayClassPart {
  char	foo;			/* lame compilers... */
} XfwfArrayClassPart;

typedef struct _XfwfArrayClassRec {
  CoreClassPart		core_class;
  CompositeClassPart	composite_class;
  ConstraintClassPart	constraint_class;
  XfwfArrayClassPart	xfwf_array_class;
} XfwfArrayClassRec;

extern XfwfArrayClassRec xfwfarrayClassRec;

typedef struct _XfwfArraySizesPart {
  Dimension	*column_widths, *column_coord;
  Dimension	*row_heights, *row_coord;

  Boolean	has_hslack, has_vslack; /* how subgenii are you? */
  Dimension	vertical_slack;	/* ick, the space beyond the cells */
  Dimension	horizontal_slack; /* that is necessary for extended cells */

  int	n_columns, n_rows;
} XfwfArraySizesPart;

typedef struct _XfwfArrayPart {
  int	n_rows;
  int	n_columns;

  Boolean	*expandable_rows; /* rows that can take up slack */
  Boolean	*expandable_columns; /* columns " */

  Boolean	same_width;	/* are all columns the same width? */
  Boolean	same_height;	/* are all rows the same height? */

  Dimension	vertical_spacing; /* space between rows */
  Dimension	horizontal_spacing; /* space between columns */
  /* the arrays are generated from the scalars unless the programmer
     provides them */
  Dimension	*vertical_spacing_a; /* space between rows */
  Dimension	*horizontal_spacing_a; /* space between columns */

  Boolean	draw_shadows;
  XfwfPenPtr	top_shadow, bottom_shadow;

  /* private goop */

  /* if these are set, the widget will be adjusted.  I hope */
  XfwfArraySizesPart	actual;
  XfwfArraySizesPart	proposed;

} XfwfArrayPart;

typedef struct _XfwfArrayRec {
  CorePart		core;
  CompositePart	composite;
  ConstraintPart	constraint;
  XfwfArrayPart		xfwf_array;
} XfwfArrayRec;

typedef struct _XfwfArrayConstraintsPart {
  int	row, column;
  int	nrows, ncolumns;
  XfwfVerticalJustify	vjust;
  XfwfHorizontalJustify	hjust;

  /* fields used during layout calculation */
  Position	x,y;
  Dimension	width,height;	/* assigned by fill or squish */
  Dimension	border;
  /* desired dimensions */
  Dimension	d_width,d_height; /* calculate above stuff from these */
  Dimension	d_border;

  Boolean	CSVhook;	/* don't ask */
} XfwfArrayConstraintsPart;

typedef struct _XfwfArrayConstraintsRec {
  XfwfArrayConstraintsPart	xfwf_array;
} XfwfArrayConstraintsRec;

#ifdef __cplusplus
}
#endif

#endif /*_Xfwf_ArrayP_h*/
