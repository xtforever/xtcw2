/*

    UGrafP.h - private header file for the UGraf Widget

    Copyright (C) 1994 Clark McGrew

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

#ifndef _UGrafP_h
#define _UGrafP_h

#include <X11/Xcms.h>
#include <X11/CoreP.h>

#include <Xfwf/UGraf.h>

extern char UGrafTranslations[];
typedef struct {		/* This is where new methods go. */
  char	dummy;	/* some stupid compilers barf on empty structures */
} UGrafClassPart;

typedef struct _UGrafClassRec {
    CoreClassPart	core_class;
    UGrafClassPart	UGraf_class;
} XfwfUGrafClassRec;

extern XfwfUGrafClassRec XfwfugrafClassRec;

/***************************************
  Hershey Fonts and stuff.
****************************************/
typedef struct {
    int font;			/* The font (0-4) */
    int family;			/* The font family (0-7) */
    int justification;		/* 0) none 1) center 2) left */
    float size;			/* The size of the text on the NT. */
    float angle;		/* The angle of the text. */
} TEXT_STUFF;

/***************************************
  
  X Specific stuff used by the UGraf widget.

****************************************/
#ifdef notdef
#define MaxGrays     8
#define MaxColors    8		/* blck white red grn blue cyan magenta crey */
#define MaxHues      18		/* Number of hues */
#define MaxValues    8		/* Number values */
#else
#define MaxGrays     8
#define MaxColors    8		/* blck white red grn blue cyan magenta crey */
#define MaxHues      6		/* Number of hues */
#define MaxValues    6		/* Number values */
#endif
#define MaxShades    MaxHues*MaxValues
#define ColorsInPalette (MaxGrays+MaxColors+MaxShades)
typedef struct {
    int low_hue, low_shade, low_value;
    int high_hue, high_shade, high_value;
    float gamma_int, gamma_red, gamma_green, gamma_blue;
    XColor              palette[ColorsInPalette];
} COLOR;

typedef struct {
    Visual		*visual;
    Drawable            pmap;
    GC                  gc;
    unsigned long       background, foreground;
    COLOR               color;
    XPoint              pointer_location;
    Bool                pointer_valid;
    XtCallbackList      callbacks; 
} X_STUFF;

/*****************************************

  Definition of the types associated with the UGraf Transformations.

******************************************/

/***
  Optimization for the transform

    (i,j) (x,Y) +----------+ (W,j) (X,Y)
                |          |
                | + (n,m)  |
                |   (u,v)  |
  (i,j+H) (x,y) +----------+ (i+W,j+H) (X,y)

  Q = W/(X-x);    R = i;     S = -H/(Y-y);    T = H+j;
  A = x;          B = y;

  n = (u-A)*Q + R;           m = (v-B)*S + T
  u = (n-R)/Q + A;           v = (m-T)/S + B 
***/
typedef struct {
    float Q, R, S, T, A, B;
} Transform;

/* 
   The Normalization Transform --

   This defines the floating point draw space for the UGraf widget.  It
   is mapped to the widget drawable so that the unit length is equal
   in both the X and Y directions and the drawing space is centered in
   the widget.  This means that not all of the drawing space may be
   accessable to the UGraf widget.  When the NormalizationTransform is
   changed then all of the other transforms must be recalculated.

   The Normalization Transform is a special case of the WindowTransform.

   WindowTransform --

   The WindowTransform maps first from the window coordinates to the
   normalization transform coordinates and then to the pixel
   coordinates.  In fact, the transformation is done directly from the
   window coordinates to the pixel coordinates.  There can be up to
   MaxWindows window transforms.  
*/
#define MaxWindows 8
typedef struct {
    Transform trans;		/* The optimized transform. */
    Bool valid;			/* Is this a valid transform. */
    XRectangle pixels;		/* The physical coordinates of the space. */
    Rectangle space;		/* The space on the window to map to. */
    Rectangle window;		/* The coordinates in the space. */
    Point last_point;		/* The last position. */
} WindowTransform;

typedef struct {
    int Current_Transform;
    WindowTransform W[MaxWindows];
} TRANSFORMS;

/************************************************
  The Three Dimensional Transforms
  ***********************************************/
struct _Transform3 {
    Methods3 *methods;
    void *data;			/* Extra transform data. */
    Point3 refp;		/* The view point */
    Point3 vdir;		/* The view direction */
    Point3 udir;		/* The up direction */
    Point3 hdir;		/* The horizontal direction. */
    float aperture;		/* Specify the opening of the view. */
    Bool valid;			/* Does this transform need to be set */
    Point3 last_point;		/* The last point drawn. */
};

typedef struct {
    int Current_Transform;	/* The index of the current transform. */
    Transform3 W[MaxWindows];	/* The transforms. */
    float tolerance;		/* Accuracy tolerance for plotting segments. */
    int min_depth;		/* The minimum number of breaks in a segment */
    int max_depth;		/* The maximum number of breaks in a segment */
    float step_size;		/* The movement size. */
    float step_dir;		/* The movement direction. */
    float step_angle;		/* The angular step. */
} TRANSFORMS3;

typedef struct {		/* This is for widget control variables  */
    X_STUFF x;
    TRANSFORMS t2;
    TRANSFORMS3 t3;
    TEXT_STUFF text;
} UGrafPart;

typedef struct _UGrafRec {
    CorePart core;
    UGrafPart UGraf;
} UGrafRec;

/***********************************
  Define a access macros for the widget.
************************************/
#define UGrafInstance(w) (&((w)->UGraf))
#define UGrafCore(w)     (&((w)->core))

void XfwfUGMakeNormalizationTransform(UGrafWidget pw);
void XfwfUGMakeWindowTransforms(UGrafWidget pw);
void XfwfUGToXPoint(XPoint *n, Point *u, Transform *t);
void XfwfUGFromXPoint(XPoint *n, Point *u, Transform *t);
void XfwfUG3Initialize(Transform3 *t);

/* Access methods for the X stuff */
#define UGrafX(w)         (&(UGrafInstance(w)->x))
#define Palette(w,i)     (&(UGrafX(w)->color.palette[(i)]))
#define UGrafColors(pw,c) (Palette(pw,c))
#define UGrafGrays(pw,g)  (Palette(pw,g+MaxColors))
#define UGrafShades(pw,h,s) (Palette(pw,h+MaxHues*s+MaxColors+MaxGrays))
void DefinePalette(UGrafWidget pw);

/* Access methods for the transforms */
#define UGrafTrans(w)     (&(UGrafInstance((w))->t2))
#define UGrafTransform(w) (UGrafTrans((w))->Current_Transform)
#define UGrafNT(w)        (&(UGrafTrans((w))->W[0]))
#define UGrafWindow(w,n)  (&(UGrafTrans((w))->W[(n)]))
#define UGrafCurrent(w)   (UGrafWindow((w),UGrafTransform((w))))

/* Access methods for the 3d transforms. */
#define UGrafTrans3(w)     (&(UGrafInstance((w))->t3))
#define UGrafTransform3(w) (UGrafTrans3((w))->Current_Transform)
#define UGrafWindow3(w,n)  (&(UGrafTrans3((w))->W[(n)]))
#define UGrafCurrent3(w)   (UGrafWindow3((w),UGrafTransform3((w))))
#define Methods(t)         ((t)->methods)
#define UGrafMethods3(w)   (Methods(UGrafCurrent3((w))))
#endif /* _UGrafP_h */


