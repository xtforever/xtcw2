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

#ifndef _Stack_h
#define _Stack_h

#if MOTIF
#include <Xm/Xm.h>
#endif

#include <X11/Constraint.h>

extern WidgetClass xfwfStackWidgetClass;

typedef struct _XfwfStackRec *XfwfStackWidget;
typedef struct _XfwfStackConstraintRec *XfwfStackConstraint;

#ifndef XfwfIsStack
#define XfwfIsStack(a) (XtIsSubclass(a, xfwfStackWidgetClass))
#endif

#if NeedFunctionPrototypes
extern void XfwfStackNextWidget(Widget _w);
extern void XfwfStackPreviousWidget(Widget _w);
extern Bool FwfCvtStringToStackType(Display *_display, XrmValuePtr _args,
    Cardinal *_numArgs, XrmValuePtr _from, XrmValuePtr _to, XtPointer *_data);
#else
extern void XfwfStackNextWidget();
extern void XfwfStackPreviousWidget();
extern Bool FwfCvtStringToStackType();
#endif /* NeedFunctionPrototypes */

#define XfwfNdata "data"
#define XfwfNmargin "margin"
#define XfwfNsameSize "sameSize"
#define XfwfNstackWidget "stackWidget"
#define XfwfNgravity "gravity"
#define XfwfNfill "fill"
#define XfwfNstackType "stackType"
#define XfwfNpreviousWidgetNotifyCallback "previousWidgetNotifyCallback"
#define XfwfNnextWidgetNotifyCallback "nextWidgetNotifyCallback"
#define XfwfNstackWidgetNotifyCallback "stackWidgetNotifyCallback"

#define XfwfCData "Data"
#define XfwfCMargin "Margin"
#define XfwfCSameSize "SameSize"
#define XfwfCStackWidget "StackWidget"
#define XfwfCGravity "Gravity"
#define XfwfCFill "Fill"
#define XfwfCStackType "StackType"

#define XfwfRStackType "StackType"

enum {XfwfSTACK_END_TO_END, XfwfSTACK_CIRCULAR};


#define XfwfCBR_NEXT_WIDGET
#define XfwfCBR_PREVIOUS_WIDGET
#define XfwfCBR_STACK_WIDGET



#endif /* _Stack_h */
