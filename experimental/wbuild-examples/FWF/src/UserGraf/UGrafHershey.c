/*

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

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <stdio.h>
#include <math.h>

#include <Xfwf/UGrafP.h>

#include "XfwfHershey.h"
extern HersheyFont cyrilc;
extern HersheyFont gothgbt;
extern HersheyFont gothgrt;
extern HersheyFont gothitt;
extern HersheyFont greekc;
extern HersheyFont greekcs;
extern HersheyFont greekp;
extern HersheyFont greeks;
extern HersheyFont italicc;
extern HersheyFont italiccs;
extern HersheyFont italict;
extern HersheyFont math;
extern HersheyFont misc;
extern HersheyFont romanc;
extern HersheyFont romancs;
extern HersheyFont romand;
extern HersheyFont romanp;
extern HersheyFont romans;
extern HersheyFont romant;
extern HersheyFont scriptc;
extern HersheyFont scripts;

extern HersheyGlyph OccidentalGlyphs[];
extern HersheyGlyph OrientalGlyphs[];

/****
  Define the families of fonts.
  ****/
HersheyFont *simplex[4] = {&romans,   0,         &scripts,  &greeks};
HersheyFont *duplex[4]  = {&romand,   0,         0,         0};
HersheyFont *complex[4] = {&romanc,   &italicc,  &scriptc,  &greekc};
HersheyFont *triplex[4] = {&romant,   &italict,  0,         0};
HersheyFont *plain[4]   = {&romanp,   0,         0,         &greekp};
HersheyFont *small[4]   = {&romancs,  &italiccs, 0,         &greekcs};
HersheyFont *gothic[4]  = {&gothgbt,  &gothitt,  &gothgrt,  &cyrilc};
HersheyFont *special[4] = {&math,     &misc,     0,         0};

HersheyFont **family[8] = {
    simplex, duplex, complex, triplex, plain, small, gothic, special
};

/* 
   Draw a lines from a collection of points using the current normalization 
   transform. 
   */
static void DrawOnNormalization(pw, o, scale, angle, p, d, n)
     UGrafWidget pw;
     Point *o;
     float scale;
     float angle;
     Point *p;
     int *d;
     int n;
 {
    int i;
    Point no;
    XPoint x;			/* The last and current point. */
    XPoint *s;			/* The segments to draw. */
    int ns = 0;			/* The number of segments */
    XPoint *a;			/* The current XPoint to draw. */
    Transform *t = &(UGrafCurrent(pw)->trans);
    Transform *NT = &(UGrafNT(pw)->trans);
    double dangle = M_PI * angle / 180.0; /* Convert angle to radians */
    float sn = sin(dangle);	/* Get the sine of the angle. */
    float cs = cos(dangle);	/* Get the cosine of the angle. */

    s = (XPoint *) malloc(sizeof(XPoint)*n);

    /* Find the point on the current normalization transform to put */
    /* the glyph at. */
    XfwfUGToXPoint(&x,o,t);
    XfwfUGFromXPoint(&x,&no,NT);

    /* Rotate, shift and scale the glyph */
    for (i=0; i<n; ++i) {
	float x = cs * p[i].x - sn * p[i].y;
	float y = sn * p[i].x + cs * p[i].y;
	p[i].x = scale*x + no.x;
	p[i].y = scale*y + no.y;
    }

    /* Loop through the points */
    a = &s[0];
    ns = 0;
    while (n) {
	XfwfUGToXPoint(&x, p, NT);
	if (!(*d)) {		/* Connect this point */
	    if (ns>1) {		/* Draw the current lines */
		XDrawLines(XtDisplay(pw),UGrafX(pw)->pmap,UGrafX(pw)->gc,
			   s,ns,CoordModeOrigin);
	    }
	    a = &s[0];
	    ns = 0;
	};
	a->x = x.x;
	a->y = x.y;
	++a; ++ns; ++p; ++d;	/* Increment the pointers. */
	--n;
    }

    --p;			/* Save the last point */
    XfwfUGToXPoint(&x,p,NT);
    XfwfUGFromXPoint(&x,&(UGrafCurrent(pw)->last_point),t);

    if (ns>1) {			/* Draw the lines */
	XDrawLines(XtDisplay(pw),UGrafX(pw)->pmap,UGrafX(pw)->gc,
		   s,ns,CoordModeOrigin);
    }

    if (s) free(s);
}

int XfwfUGMarkPoints(pw, mark, p, d, max)
     UGrafWidget pw;
     int mark;
     Point *p;
     int *d;
     int max;
 {
    int npts;
    Point no = {0.0, 0.0};

    /* Find the glyph to draw. */
    mark += 32;
    if (mark<32) mark = 32;
    if (mark>=misc.nchar) mark = misc.nchar-1;
    mark = misc.glyph[mark];

    /* Get the points in the glyph. */
    npts = XfwfHershey_Glyph(&(OccidentalGlyphs[mark]), (HersheyPoint *)&no,
			 (HersheyPoint *)p, d, 0);

    return npts;
}

#ifdef NeedFunctionPrototypes
void XfwfUGMark(UGrafWidget pw, float x, float y, int mark, float scale)
#else
void XfwfUGMark(pw, x, y, mark, scale)
     UGrafWidget pw;
     float x;
     float y;
     int mark;
     float scale;
#endif
 {
    Point o;
    Point p[500];
    int d[500], npts;
    Point no = {0.0, 0.0};

    /* Fill the origin */
    o.x = x;
    o.y = y;

    /* Get the points in the glyph. */
    npts = XfwfUGMarkPoints(pw,mark, p, d, 500);

    /* Draw the points on the current normalization transform */
    DrawOnNormalization(pw, &o, scale, 0.0, p, d, npts);
}

void XfwfUGFontFamily(pw, family)
     UGrafWidget pw;
     int family;
 {
    if (family<0) family = 0;
    if (family>7) family = 7;
    UGrafInstance(pw)->text.family = family;
}

void XfwfUGFontType(pw, type)
     UGrafWidget pw;
     int type;
 {
    if (type<0) type = 0;
    if (type>7) type = 7;
    UGrafInstance(pw)->text.font = type;
}

#ifdef NeedFunctionPrototypes
void XfwfUGFontSize(UGrafWidget pw, float size)
#else
void XfwfUGFontSize(pw, size)
     UGrafWidget pw;
     float size;
#endif
 {
    if (size<=0) return;
    UGrafInstance(pw)->text.size = size;
}

#ifdef NeedFunctionPrototypes
void XfwfUGFontAngle(UGrafWidget pw, float angle)
#else
void XfwfUGFontAngle(pw, angle)
     UGrafWidget pw;
     float angle;
#endif
 {
    UGrafInstance(pw)->text.angle = angle;
}

void XfwfUGFontJustification(pw, j)
     UGrafWidget pw;
     int j;
 {
    if (j<0) j = 0;
    if (j>2) j = 0;
    UGrafInstance(pw)->text.justification = j;
}

void XfwfUGFontDefault(pw)
     UGrafWidget pw;
 {
    UGrafInstance(pw)->text.font = 0;
    UGrafInstance(pw)->text.family = 0;
    UGrafInstance(pw)->text.justification = 0;
    UGrafInstance(pw)->text.size = 1.0;
    UGrafInstance(pw)->text.angle = 0.0;
}

int XfwfUGTextPoints(pw, text, p, d, max)
     UGrafWidget pw;
     char *text;
     Point *p;
     int *d;
     int max;
 {
    Point no = {0.0,0.0};
    int npts;
    int fnt, fam;
    int i;

    /* Get the font and family */
    fam = UGrafInstance(pw)->text.family;
    fnt = UGrafInstance(pw)->text.font;
    if (!family[fam][fnt]) {fam = fnt = 0;}

    /* Get the points */
    npts = XfwfHershey_String(family[fam][fnt],OccidentalGlyphs,text,
			  (HersheyPoint *)&no,(HersheyPoint *)p, d);

    /* Deal with justification */
    if (UGrafInstance(pw)->text.justification) {
	if (UGrafInstance(pw)->text.justification == 1) {
	    no.x /= 2.0;
	    no.y /= 2.0;
	}
	for (i=0; i<npts; ++i) {
	    p[i].x = p[i].x - no.x;
	    p[i].y = p[i].y - no.y;
	}
    }

    return npts;
}

#ifdef NeedFunctionPrototypes
void XfwfUGText(UGrafWidget pw, float x, float y, char *text)
#else
void XfwfUGText(pw, x, y, text)
     UGrafWidget pw;
     float x;
     float y;
     char *text;
#endif
 {
    Point o;
    Point p[1000];
    int d[1000], npts;

    /* Fill the origin */
    o.x = x;
    o.y = y;

    npts = XfwfUGTextPoints(pw, text, p, d, 1000);

    /* Draw the points on the current normalization transform */
    DrawOnNormalization(pw, &o, 
			UGrafInstance(pw)->text.size, 
			UGrafInstance(pw)->text.angle, 
			p, d, npts);
}


