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



#ifndef _Mdial_h
#define _Mdial_h

extern WidgetClass mdialWidgetClass;
typedef struct _MdialClassRec *MdialWidgetClass;
typedef struct _MdialRec *MdialWidget;

#ifndef XtIsMdial
#define XtIsMdial(w) XtIsSubclass((w), mdialWidgetClass)
#endif

#define XmNdialBackground "dialBackground"
#define XmNdialForeground "dialForeground"
#define XmNpreserveAspect "preserveAspect"
#define XmNfullCircle "fullCircle"
#define XmNstartPosition "startPosition"
#define XmNlabelFormatter "labelFormatter"
#define XmNdisplayOnly "displayOnly"
#define XmNlabelAlignment "labelAlignment"

#define XmCPreserveAspect "PreserveAspect"
#define XmCFullCircle "FullCircle"
#define XmCStartPosition "StartPosition"
#define XmCLabelFormatter "LabelFormatter"
#define XmCDisplayOnly "DisplayOnly"

#define MAX_DIAL_LABEL 50

/* Reasons */
#define MHDIAL_INCREMENT 1
#define MHDIAL_DECREMENT 2
#define MHDIAL_SET 3

typedef void (*t_func)();

typedef struct _MdialCallbackStruct
{
  int reason;
  XEvent *event;
  int value;
} MdialCallbackStruct, *MdialCallbackPtr;

#ifdef _NO_PROTO
void MdialIncrement();
void MdialDecrement();
void MdialSet();

#else
void MdialIncrement(Widget _w, int _increment, Boolean _callCallbacks);
void MdialDecrement(Widget _w, int _decrement, Boolean _callCallbacks);
void MdialSet(Widget _w, int _value, Boolean _callCallbacks);

#endif


#endif /* _Mdial_h */
