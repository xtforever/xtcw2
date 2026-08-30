
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


#ifndef _MdialP_h
#define _MdialP_h

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <Xm/XmP.h>
#include <Xm/PrimitiveP.h>

#include "Mdial.h"

typedef struct _MdialPart
{
  Dimension marginWidth;
  Dimension marginHeight;
  Boolean preserveAspect;
  Boolean fullCircle;
  Boolean displayOnly;
  unsigned char labelAlignment;
  unsigned char shadowType;
  int value;
  int maximum;
  int minimum;
  XmFontList fontList;
  Pixel dialBackground;
  Pixel dialForeground;
  unsigned char processingDirection;
  XmStringDirection stringDirection;
  t_func labelFormatter;

  XtCallbackList incrementCallback;
  XtCallbackList decrementCallback;
  XtCallbackList valueChangedCallback;

/*
** Don't look...My private parts...
*/

  GC containerTopShadowGC;
  GC containerBottomShadowGC;
  GC dialForegroundGC;
  GC dialBackgroundGC;

  XmString label;
  XFontStruct *fontStruct;

  Position dialX, dialY;
  Position centerX, centerY;
  Position armX, armY;
  Position labelX, labelY;

  Dimension labelWidth, labelHeight;
  Dimension dialWidth, dialHeight;

  double rangle;

} MdialPart, *MdialPartPtr;

typedef struct _MdialRec
{
  CorePart core;
  XmPrimitivePart primitive;
  MdialPart mdial;
} MdialRec, *MdialPtr;

typedef struct _MdialClassPart
{
  int blammo;
} MdialClassPart;

typedef struct _MdialClassRec
{
  CoreClassPart core_class;	
  XmPrimitiveClassPart primitive_class;
  MdialClassPart mdial_class;
} MdialClassRec, *MdialClassPtr;

extern MdialClassRec mdialClassRec;

#endif /* _MdialP_h */
