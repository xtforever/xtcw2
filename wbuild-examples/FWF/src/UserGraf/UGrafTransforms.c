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

/* Transform to an Xpoint from a Point. */
void XfwfUGToXPoint(n, u, t)
     XPoint *n;
     Point *u;
     Transform *t;
 {
    float r = (u->x - t->A) * t->Q + t->R;
    if (r>15000) r = 15000; else if (r<-15000) r = -15000;
    n->x = r;
    r = (u->y - t->B) * t->S + t->T;
    if (r>15000) r = 15000; else if (r<-15000) r = -15000;
    n->y = r;
}

/* Transform from a Xpoint to a Point */
void XfwfUGFromXPoint(n, u, t)
     XPoint *n;
     Point *u;
     Transform *t;
 {
    u->x = (n->x - t->R) / t->Q + t->A;
    u->y = (n->y - t->T) / t->S + t->B;
}

/* Find the new window transforms */
static void MakeWindowTransform(pw, W)
     UGrafWidget pw;
     WindowTransform *W;
 {
    XPoint l, h;
    Point t;

    /* Check if the transform is valid */
    if (W->space.x >= W->space.X 
        || W->space.y >= W->space.Y
	|| W->window.x >= W->window.X
	|| W->window.y >= W->window.Y) {
	W->valid = False;
	return;
    }

    t.x = W->space.x; t.y = W->space.y;
    XfwfUGToXPoint(&l,&t,&(UGrafNT(pw)->trans));
    t.x = W->space.X; t.y = W->space.Y;
    XfwfUGToXPoint(&h,&t,&(UGrafNT(pw)->trans));

    W->pixels.x = l.x;
    W->pixels.y = h.y;
    W->pixels.width = h.x - l.x;
    W->pixels.height = l.y - h.y;
	
    W->trans.A = W->window.x;
    W->trans.B = W->window.y;
    W->trans.Q = (h.x - l.x)/(W->window.X - W->window.x);
    W->trans.R = l.x;
    W->trans.S = (h.y - l.y)/(W->window.Y - W->window.y);
    W->trans.T = l.y;
    W->valid = True;
}

/* Remake all of the window transforms */
void XfwfUGMakeWindowTransforms(pw)
     UGrafWidget pw;
 {
    int i;
    for (i=1; i<MaxWindows; ++i) 
	MakeWindowTransform(pw,UGrafWindow(pw,i));
}

/* Find the new normalization transform */
void XfwfUGMakeNormalizationTransform(pw)
     UGrafWidget pw;
 {
    WindowTransform *nt = UGrafNT(pw);
    Screen *scr = XtScreen(pw);
    double xscale, yscale, scale; 

    /* Find the Pixels per MM for the screen */
    double xmm = WidthOfScreen(scr)/WidthMMOfScreen(scr);
    double ymm = HeightOfScreen(scr)/HeightMMOfScreen(scr);

    /* Check if the transform is valid */
    if (nt->space.x > nt->space.X 
        || nt->space.y > nt->space.Y) {
	nt->valid = False;
	nt->space.x = 0.0;
	nt->space.X = 20.0;
	nt->space.y = 0.0;
	nt->space.Y = 20.0;
    }

    /* Find the scale of the window based on the X and Y directions. */
    /* xscale is the width of the window in mm.  yscale is the height */
    /* of the window in mm. */
    xscale = UGrafCore(pw)->width / (nt->space.X - nt->space.x) / xmm;
    yscale = UGrafCore(pw)->height / (nt->space.Y - nt->space.y) / ymm;

    /* Use the smallest dimension in mm as the scale of the screen. */
    scale = (xscale<yscale) ? xscale: yscale;

    /* Find the XRectangle that will fit the requested space. */
    nt->pixels.width = xmm * scale * (nt->space.X - nt->space.x);
    nt->pixels.height = ymm * scale * (nt->space.Y - nt->space.y);
    
    /* Find the offset of the XRectangle to center it in the */
    /* draw space. */
    nt->pixels.x = (UGrafCore(pw)->width - nt->pixels.width)/2;
    nt->pixels.y = (UGrafCore(pw)->height - nt->pixels.height)/2;

    /* Now that the pixel rectangle is found, find the transform */
    /* parameters */
    nt->trans.A = nt->space.x;
    nt->trans.B = nt->space.y;
    nt->trans.Q = 1.0*nt->pixels.width / (nt->space.X - nt->space.x);
    nt->trans.R = 1.0*nt->pixels.x;
    nt->trans.S = -1.0*nt->pixels.height / (nt->space.Y - nt->space.y);
    nt->trans.T = 1.0*(nt->pixels.height + nt->pixels.y);
    nt->valid = True;

    /* Now remake all of the window transformations */
    XfwfUGMakeWindowTransforms(pw);
}

/* User Routine:
   XfwfUGNormal -- 
   Set the drawing space.  This is like setting the page size.  The
   normalization transform lets the drawing windows be specfied
   without knowing the physical device coordinates 
   */
#ifdef NeedFunctionPrototypes
void XfwfUGNormal(UGrafWidget w, float x, float y)
#else
void XfwfUGNormal(w, x, y)
     UGrafWidget w;
     float x;
     float y;
#endif
 {
    if (x<0.0) x = -x;
    if (y<0.0) y = -y;
    UGrafNT(w)->space.x = 0.0;
    UGrafNT(w)->space.y = 0.0;
    UGrafNT(w)->space.X = x;
    UGrafNT(w)->space.Y = y;
    XfwfUGMakeNormalizationTransform(w);
}

/*
  User Routine:
  XfwfUGWindow --
  This sets the transform from the window to the normalization space.
  The plotting routines draw in the window space.
  */
void XfwfUGWindow(pw, w, space, window)
     UGrafWidget pw;
     int w;
     Rectangle *space;
     Rectangle *window;
 {
    if (w<1) return;
    if (w>MaxWindows) return;
    /* Check if the transform is valid */
    if (space->x > space->X 
        || space->y > space->Y
	|| window->x > window->X
	|| window->y > window->Y) {
	XtAppContext ap = XtWidgetToApplicationContext((Widget) pw);
	XtAppWarningMsg(ap,"UGraf","bad_window","UGraf",
			"Illegal Window Parameters",0,0);
	return;
    }
    /* Set the window space */
    UGrafWindow(pw,w)->space.x = space->x;
    UGrafWindow(pw,w)->space.y = space->y;
    UGrafWindow(pw,w)->space.X = space->X;
    UGrafWindow(pw,w)->space.Y = space->Y;
    /* Set the normalization space */
    UGrafWindow(pw,w)->window.x = window->x;
    UGrafWindow(pw,w)->window.y = window->y;
    UGrafWindow(pw,w)->window.X = window->X;
    UGrafWindow(pw,w)->window.Y = window->Y;
    /* Update the transformation */
    MakeWindowTransform(pw,UGrafWindow(pw,w));
    XfwfUGCurrent(pw,w);
}

/* User Routine:
   XfwfUGCurrent --
   Set the current window transformation to use.
   */
int XfwfUGCurrent(pw, transform)
     UGrafWidget pw;
     int transform;
 {
    int old = UGrafTrans(pw)->Current_Transform;
    if (transform<0 || transform >= MaxWindows) transform = 0;
    UGrafTrans(pw)->Current_Transform = transform;
    return old;
}


