/*
 * Copyright 1993 John L. Cwikla
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of John L. Cwikla or
 * Wolfram Research, Inc not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.  John L. Cwikla and Wolfram Research, Inc make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * John L. Cwikla and Wolfram Research, Inc disclaim all warranties with
 * regard to this software, including all implied warranties of
 * merchantability and fitness, in no event shall John L. Cwikla or
 * Wolfram Research, Inc be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data or profits, whether in an action of contract, negligence or
 * other tortious action, arising out of or in connection with the use or
 * performance of this software.
 *
 * Author:
 *  John L. Cwikla
 *  X Programmer
 *  Wolfram Research Inc.
 *
 *  cwikla@wri.com
*/

#ifndef _StackP_h
#define _StackP_h

#include "Stack.h"

#if MOTIF
#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#endif /* MOTIF */

#include <X11/ConstrainP.h>

typedef struct _XfwfStackClassPart
{
	int blammo;
} XfwfStackClassPart;

typedef struct _XfwfStackClassRec
{
	CoreClassPart core_class;
	CompositeClassPart composite_class;
	ConstraintClassPart constraint_class;
#if MOTIF 
	XmManagerClassPart manager_class;
#endif /* MOTIF */
	XfwfStackClassPart stack_class;
} XfwfStackClassRec;

extern XfwfStackClassRec xfwfStackClassRec;

typedef struct _XfwfStackPart
{
	XtPointer data;
	Bool sameSize;
	Bool okToLayout;
	Bool fill;
	unsigned char stackType;
	Dimension margin;
	Widget stackWidget;

} XfwfStackPart;

typedef struct _XfwfStackRec
{
	CorePart core;
	CompositePart	 composite;
	ConstraintPart constraint;
#if MOTIF 
	XmManagerPart manager;
#endif /* MOTIF */
	XfwfStackPart stack;
} XfwfStackRec, *XfwfStackPtr;

typedef struct _XfwfStackConstraintPart
{
	int gravity;
	Bool fill;
} XfwfStackConstraintPart;

typedef struct _XfwfStackConstraintRec
{
#if MOTIF
	XmManagerConstraintPart manager;
#endif /* MOTIF */
	XfwfStackConstraintPart stack;
}	XfwfStackConstraintRec, *XfwfStackConstraintPtr;

#endif /* _StackP_h */
