/*
 * Copyright 1992 John L. Cwikla
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of John L. Cwikla or
 * University of Illinois not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.  John L. Cwikla and University of Illinois make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * John L. Cwikla and University of Illinois disclaim all warranties with
 * regard to this software, including all implied warranties of
 * merchantability and fitness, in no event shall John L. Cwikla or
 * University of Illinois be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data or profits, whether in an action of contract, negligence or
 * other tortious action, arising out of or in connection with the use or
 * performance of this software.
 *
 * Author:
 * 	John L. Cwikla
 * 	Materials Research Laboratory Center for Computation
 * 	University Of Illinois at Urbana-Champaign
 *	104 S. Goodwin
 * 	Urbana, IL 61801
 * 
 * 	cwikla@uimrl7.mrl.uiuc.edu
 */

#ifndef SHISTOGRAMP__H
#define SHISTOGRAMP__H

/*
** Simple Histogram Widget
*/

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <Xfwf/Shistogram.h>

#ifdef UBYTE_NEEDED
typedef unsigned char ubyte;
#endif

#define MAXSTRING 25 
#define MAXLABELS 50 
#define MAXCOUNT 256

typedef struct _LabelStruct
{
  Pixel foreground;
  String label;
  GC gc;
  XPoint pos;
  Dimension width;
  Dimension height;
  XFontStruct *font;
} LabelStruct, *LabelPtr;

typedef struct _ShistogramPart
{
  Pixel foreground;          /* for rects */
  Boolean xAxis;             /* Should I display the X axis */
  Boolean yAxis;             /* Should I display the Y axis */
  Boolean showTitle;
  Boolean showStatistics;
  LabelStruct statistics;   
  LabelStruct title;
  LabelStruct axis;
  LabelStruct yAxisLabels[MAXLABELS];
  LabelStruct xAxisLabels[MAXLABELS];
  Dimension maxYLabelWidth;
  Dimension maxXLabelWidth;
  int minimum;             /* minimum value */
  int maximum;             /* maximum value */
  int charCount;          /* How many chars in the "widest" value ? */
  int margin;                /* minimum margin */
  int minCount;              /* minimum number of values for single */
  int maxCount;              /* maximum number of values for single */
  int xTics;                 /* number of x tics */
  int yTics;                 /* number of y tics */
  int number;                /* Total number of values */
  int count[MAXCOUNT];
  int width;                 /* for rectangles */
  int yMax;                  /* of axis */
  int xMax;
  double mean;
  double variance;
  XRectangle drawArea; 
  GC gc;                     /* for rectangles */
  GC eraseGC;                /* We all need one of these */
  XtCallbackList selectCallback;
} ShistogramPart, *ShistogramPartPtr;

typedef struct _ShistogramRec
{
  CorePart core;
  ShistogramPart shistogram;
} ShistogramRec, *ShistogramPtr;

typedef struct _ShistogramClassPart
{
  int empty;
} ShistogramClassPart;

typedef struct _ShistogramClassRec
{
  CoreClassPart core_class;	
  ShistogramClassPart shistogram_class;
} ShistogramClassRec, *ShistogramClassPtr;

extern ShistogramClassRec shistogramClassRec;

#endif /* SHISTOGRAMP__H */
