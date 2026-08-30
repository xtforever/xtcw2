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

#include <Xfwf/UGrafP.h>

/* User Routine:
   XfwfUGClear --
   Clear the entire widget.
   */
void XfwfUGClear(pw)
     UGrafWidget pw;
 {
    XSetForeground(XtDisplay(pw), UGrafX(pw)->gc, UGrafX(pw)->background);
    XFillRectangle(XtDisplay(pw),UGrafX(pw)->pmap,UGrafX(pw)->gc,
		   0, 0, UGrafCore(pw)->width, UGrafCore(pw)->height);
    XSetForeground(XtDisplay(pw), UGrafX(pw)->gc, UGrafX(pw)->foreground);
}

/* User Routine:
   XfwfUGClearWindow --
   Clear just the current transformation window.
   */
void XfwfUGClearWindow(pw)
     UGrafWidget pw;
 {
    XSetForeground(XtDisplay(pw), UGrafX(pw)->gc, UGrafX(pw)->background);
    XFillRectangle(XtDisplay(pw),UGrafX(pw)->pmap,UGrafX(pw)->gc,
		   UGrafCurrent(pw)->pixels.x, UGrafCurrent(pw)->pixels.y,
		   UGrafCurrent(pw)->pixels.width, 
		   UGrafCurrent(pw)->pixels.height);
    XSetForeground(XtDisplay(pw), UGrafX(pw)->gc, UGrafX(pw)->foreground);
}

/* User Routine:
   XfwfUGLine --
   Draw a line from the last point.
   */
#ifdef NeedFunctionPrototypes
void XfwfUGLine(UGrafWidget pw, float x, float y, int d)
#else
void XfwfUGLine(pw, x, y, d)
     UGrafWidget pw;
     float x;
     float y;
     int d;
#endif
 {
    if (d) {			/* If connected to last point. */
	XPoint x1, x2;
	Point p;
	p.x = x;
	p.y = y;
	XfwfUGToXPoint(&x1, &(UGrafCurrent(pw)->last_point),
		       &(UGrafCurrent(pw)->trans));
	XfwfUGToXPoint(&x2, &p, &(UGrafCurrent(pw)->trans));
	XDrawLine(XtDisplay(pw), UGrafX(pw)->pmap, UGrafX(pw)->gc,
		  x1.x,x1.y,x2.x,x2.y);
    }
    /* Save the last point. */
    UGrafCurrent(pw)->last_point.x = x;
    UGrafCurrent(pw)->last_point.y = y;
}

/* User Routine:
   XfwfUGLines --
   Draw a lines from a collection of points.
   */
void XfwfUGLines(pw, p, d, n)
     UGrafWidget pw;
     Point *p;
     int *d;
     int n;
 {
    XPoint x;		/* The last and current point. */
    int i;
    XPoint *s;		/* The segments to draw. */
    int ns = 0;			/* The number of segments */
    XPoint *a;
    Transform *t = &(UGrafCurrent(pw)->trans);

    s = (XPoint *) malloc (sizeof(XPoint)*n);

    /* Convert the last point to an XPoint. */
    XfwfUGToXPoint(s,&(UGrafCurrent(pw)->last_point),t);
    ns = 1;

    /* Loop through the points */
    for (i=0; i<n; ++i) {
	XfwfUGToXPoint(&x, p, t);
	if (!(*d)) {		/* Connect this point */
	    if (ns>1) {		/* Draw the current lines */
		XDrawLines(XtDisplay(pw),UGrafX(pw)->pmap,UGrafX(pw)->gc,
			   s,ns,CoordModeOrigin);
	    }
	    ns = 0;
	}
	s[ns].x = x.x;
	s[ns].y = x.y;
	++ns; ++p; ++d;		/* Increment the pointers. */
    }

    if (ns>1) {			/* Draw the lines */
	XDrawLines(XtDisplay(pw),UGrafX(pw)->pmap,UGrafX(pw)->gc,
		   s,ns,CoordModeOrigin);
    }

    /* Save the last point */
    --p;			/* We've gone one to far. */
    UGrafCurrent(pw)->last_point.x = p->x;
    UGrafCurrent(pw)->last_point.y = p->y;

    if (s) free(s);
}

/* User Routine:
   XfwfUGFill -- Fill a polygon.
   */
void XfwfUGFill(pw, p, n)
     UGrafWidget pw;
     Point *p;
     int n;
 {
    XPoint *xp;
    int i;

    xp = (XPoint *) malloc(sizeof(XPoint)*n);

    for (i=0; i<n; ++i) {
	XfwfUGToXPoint(&xp[i], p, &(UGrafCurrent(pw)->trans));
	++p;
    }
    XFillPolygon(XtDisplay(pw),UGrafX(pw)->pmap,UGrafX(pw)->gc,xp,n,
		 Complex,CoordModeOrigin);

    if (xp) free(xp);
}
