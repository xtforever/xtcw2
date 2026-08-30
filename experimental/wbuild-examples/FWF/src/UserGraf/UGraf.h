/*

    UGraf.h - public header file for the UGraf Widget
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

#ifndef _UGraf_h
#define _UGraf_h

#include <X11/Core.h>

#ifndef XtNvisual
#define XtNvisual	"visual"
#define XtCVisual	"Visual"
#endif

#ifndef XtNlowHue
#define XtNlowHue        "lowHue"
#define XtCLowHue        "LowHue"
#endif

#ifndef XtNhighHue
#define XtNhighHue       "highHue"
#define XtCHighHue       "HighHue"
#endif

#ifndef XtNlowShade
#define XtNlowShade      "lowShade"
#define XtCLowShade      "LowShade"
#endif

#ifndef XtNhighShade
#define XtNhighShade     "highShade"
#define XtCHighShade     "HighShade"
#endif

#ifndef XtNlowValue
#define XtNlowValue      "lowValue"
#define XtCLowValue      "LowValue"
#endif

#ifndef XtNhighValue
#define XtNhighValue     "highValue" 
#define XtCHighValue     "HighValue"
#endif

#ifndef XtNgamma
#define XtNgamma         "gamma"
#define XtCGamma         "Gamma"
#endif

#ifndef XtNredGamma
#define XtNredGamma      "redGamma"
#define XtCRedGamma      "RedGamma"
#endif

#ifndef XtNgreenGamma
#define XtNgreenGamma    "greenGamma"
#define XtCGreenGamma    "GreenGamma"
#endif

#ifndef XtNblueGamma
#define XtNblueGamma     "blueGamma"
#define XtCBlueGamma     "BlueGamma"
#endif

#ifndef XtNtolerance
#define XtNtolerance     "tolerance"
#define XtCTolerance     "Tolerance"
#endif

#ifndef XtNminDepth
#define XtNminDepth     "minDepth"
#define XtCMinDepth     "MinDepth"
#endif

#ifndef XtNmaxDepth
#define XtNmaxDepth     "maxDepth"
#define XtCMaxDepth     "MaxDepth"
#endif

#ifndef XtNstepSize
#define XtNstepSize     "stepSize"
#define XtCStepSize     "StepSize"
#endif

#ifndef XtNstepDir
#define XtNstepDir     "stepDir"
#define XtCStepDir     "StepDir"
#endif

#ifndef XtNstepAngle
#define XtNstepAngle     "stepAngle"
#define XtCStepAngle     "StepAngle"
#endif

#ifndef XtNupdateCallback
#define XtNupdateCallback "updateCallback"
#define XtCUpdateCallback "UpdateCallback"
#endif

#ifndef XtNlocationCallback
#define XtNlocationCallback "locationCallback"
#define XtCLocationCallback "LocationCallback"
#endif

/* Define the basic font types */
#define ROMAN    0
#define ITALIC   1
#define SCRIPT   2
#define GREEK    3

/* Define the font families */
#define SIMPLEX  0
#define DUPLEX   1
#define COMPLEX  2
#define TRIPLEX  3
#define PLAIN    4
#define SMALL    5
#define GOTHIC   6
#define SPECIAL  7

/* Define the font justification */
#define RIGHT_JUSTIFICATION  0
#define CENTER_JUSTIFICATION 1
#define LEFT_JUSTIFICATION   2

extern WidgetClass xfwfUGrafWidgetClass;

typedef struct _UGrafClassRec *UGrafWidgetClass;
typedef struct _UGrafRec      *UGrafWidget;

/*******
  Point3 -- A Point in the 3D Space.
  ******/
typedef struct {
    float x, y, z;
} Point3;

/*******
  Point -- A point in the normalization or window space.
  ******/
typedef struct {		/* A point in NT or W space */
    float x, y;
} Point;

/*******
  Rectangle -- An area in the normalization or window space. 
  ******/
typedef struct {		/* A rectangle in NT or W coordinates. */
    float x,y,X,Y;
} Rectangle;

/*******
  Transform3 -- A 3D to 2D mapping.
  ******/
typedef struct _Transform3 Transform3;

typedef struct _METHODS3 {
    Bool (*To3D)(Point *d2, Point3 *d3, Transform3 *this); 
				/* Transform from 2D to 3D. (unit 3D) */
    Bool (*From3D)(Point *d2, Point3 *d3, Transform3 *this); 
				/* Transform from 3D to 2D. */
    int (*Outline)(Point *d2, int *draw, Transform3 *this); 
				/* return points to outline transform. */
    void (*SetTransform)(Transform3 *this);
				/* Make this transform valid. */
    void (*FreeTransform)(Transform3 *this);
				/* Free any local data. */
} Methods3;

/**********************************************
  The return structure for the location callback.
  *********************************************/
struct _XfwfUGlocationCallbackReturn {
    XPoint xpoint;
    Point point;
    Point3 point3;
    Boolean valid3;
};
typedef struct _XfwfUGlocationCallbackReturn XfwfUGLocationCallbackReturn;

#ifdef PROTO
# undef PROTO
#endif
#ifdef NeedFunctionPrototypes
#define PROTO(X) X
#else
#define PROTO(X) ()
#endif

/* User Routines to set the transform. */
void XfwfUGNormal PROTO((UGrafWidget pw, float x, float y));
void XfwfUGWindow PROTO((UGrafWidget pw, int w, Rectangle *space, Rectangle *window));
int XfwfUGCurrent PROTO((UGrafWidget pw, int transform));

/* User Routines to set the color */
void XfwfUGColor PROTO((UGrafWidget pw, int c));
void XfwfUGGray PROTO((UGrafWidget pw, float g));
void XfwfUGShade PROTO((UGrafWidget pw, float h, float v));

/* User Routines for text */
void XfwfUGFontDefault PROTO((UGrafWidget pw));
void XfwfUGFontFamily PROTO((UGrafWidget pw, int family));
void XfwfUGFontType PROTO((UGrafWidget pw, int type));
void XfwfUGFontSize PROTO((UGrafWidget pw, float size));
void XfwfUGFontAngle PROTO((UGrafWidget pw, float angle));
void XfwfUGFontJustification PROTO((UGrafWidget pw, int justification));

/* Return the points in text.  Users probably don't need to call this */
int XfwfUGTextPoints PROTO((UGrafWidget pw, char *text, Point *p, int *d, int max));
int XfwfUGMarkPoints PROTO((UGrafWidget pw, int mark, Point *p, int *d, int max));

/* User Routines to clear and update windows */
void XfwfUGClear PROTO((UGrafWidget pw));
void XfwfUGClearWindow PROTO((UGrafWidget pw));
void XfwfUGUpdate PROTO((UGrafWidget pw));

/* User Routines to draw. */
void XfwfUGLine PROTO((UGrafWidget pw, float x, float y, int d));
void XfwfUGLines PROTO((UGrafWidget pw, Point *p, int *d, int n));
void XfwfUGFill PROTO((UGrafWidget pw, Point *p, int n));
void XfwfUGMark PROTO((UGrafWidget pw, float x, float y, int mark, float scale));
void XfwfUGText PROTO((UGrafWidget pw, float x, float y, char *text));

/* User Routines to set the 3D transformation */
void XfwfUG3Reference PROTO((UGrafWidget pw, Point3 *refp, Bool set));
void XfwfUG3ViewDirection PROTO((UGrafWidget pw, Point3 *vdir, Bool set));
void XfwfUG3UpDirection PROTO((UGrafWidget pw, Point3 *udir, Bool set));
void XfwfUG3HorizontalDirection PROTO((UGrafWidget pw, Point3 *hdir, Bool set));
float XfwfUG3Aperture PROTO((UGrafWidget pw, float aperture, Bool set));
int XfwfUG3Current PROTO((UGrafWidget pw, int transform));
Methods3 *XfwfUG3SetMethods PROTO((UGrafWidget pw, Methods3 *m));

/* User Routines to draw in 3D. */
void XfwfUG3Line PROTO((UGrafWidget pw, float x, float y, float z, int d));
void XfwfUG3Lines PROTO((UGrafWidget pw, Point3 *p, int *d, int n));
void XfwfUG3Fill PROTO((UGrafWidget pw, Point3 *p, int n));
void XfwfUG3Mark PROTO((UGrafWidget pw, Point3 *p, Point3 *u, Point3 *h,
		 int mark, float scale));
void XfwfUG3Text PROTO((UGrafWidget pw, Point3 *p, Point3 *u, Point3 *h, char *text));

#endif /* _UGraf_h */


