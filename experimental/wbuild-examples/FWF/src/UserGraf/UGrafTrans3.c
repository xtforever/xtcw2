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

#include <stdio.h>
#include <math.h>

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <Xfwf/UGrafP.h>
#define InnerProduct(a,b)  ((a).x*(b).x + (a).y*(b).y + (a).z*(b).z)

static Bool DefaultTo3D();
static Bool DefaultFrom3D();
static int DefaultOutline();
static void DefaultSetTransform();
static void DefaultFreeTransform();
Methods3 DefaultTransform3 = {DefaultTo3D, DefaultFrom3D, 
			    DefaultOutline, 
			    DefaultSetTransform, DefaultFreeTransform};

/*
 * XfwfUG3Initialize --
 *    Initialize a transformation to the default state.
 */
void XfwfUG3Initialize(t)
     Transform3 *t;
 {
    Methods(t) = &DefaultTransform3;
    t->refp.x = 0.0;	t->refp.y = 0.0;	t->refp.z = 0.0;
    t->vdir.x = 1.0;	t->vdir.y = 0.0;	t->vdir.z = 0.0;
    t->udir.x = 0.0;	t->udir.y = 0.0;	t->udir.z = 1.0;
    t->hdir.x = 0.0;	t->hdir.y = 1.0;	t->hdir.z = 0.0;
    t->valid = False;
    t->aperture = 1.0;
    t->data = (void *) 0;
}

/*
  XfwfUG3FreeTransform --

     Free any data allocated for a particular transformation.  If a
  transform provides a FreeTransform method then it is called.
  This method should free any memory allocated for the
  transformation and then set the data field to null.  After trying
  to call a FreeTransform method, if the data field is not null
  then the allocated memory is freed.

  If a transform allocates a simple block of memory then it does not
  need to provide a FreeTransform method.  This method is only needed
  if the data block contains pointers to other allocated memory.
  */
void XfwfUG3FreeTransform(t)
     Transform3 *t;
 {
    if (Methods(t))
	if (Methods(t)->FreeTransform) 
	    Methods(t)->FreeTransform(t);

    if (t->data) free(t->data);
}

/*
  XfwfUG3Reference --
     Set the reference point of the current transformation.  
 */
void XfwfUG3Reference(pw, refp, set)
     UGrafWidget pw;
     Point3 *refp;
     int set;
 {
    if (set) {
	UGrafCurrent3(pw)->refp.x = refp->x;
	UGrafCurrent3(pw)->refp.y = refp->y;
	UGrafCurrent3(pw)->refp.z = refp->z;
	UGrafCurrent3(pw)->valid = False;
    }
    else {
	refp->x = UGrafCurrent3(pw)->refp.x;
	refp->y = UGrafCurrent3(pw)->refp.y;
	refp->z = UGrafCurrent3(pw)->refp.z;
    }
}

/*  
 XfwfUG3ViewDirection --
    Set the view direction of the current transformation.
 */
void XfwfUG3ViewDirection(pw, vdir, set)
     UGrafWidget pw;
     Point3 *vdir;
     int set;
 {
    if (set) {
	UGrafCurrent3(pw)->vdir.x = vdir->x;
	UGrafCurrent3(pw)->vdir.y = vdir->y;
	UGrafCurrent3(pw)->vdir.z = vdir->z;
	UGrafCurrent3(pw)->valid = False;
    }
    else {
	vdir->x = UGrafCurrent3(pw)->vdir.x;
	vdir->y = UGrafCurrent3(pw)->vdir.y;
	vdir->z = UGrafCurrent3(pw)->vdir.z;
    }
}

/*
  XfwfUG3UpDirection --
    Set the up direction of the current transformation.
 */
void XfwfUG3UpDirection(pw, udir, set)
     UGrafWidget pw;
     Point3 *udir;
     int set;
 {
    if (set) {
	UGrafCurrent3(pw)->udir.x = udir->x;
	UGrafCurrent3(pw)->udir.y = udir->y;
	UGrafCurrent3(pw)->udir.z = udir->z;
	UGrafCurrent3(pw)->valid = False;
    }
    else {
	udir->x = UGrafCurrent3(pw)->udir.x;
	udir->y = UGrafCurrent3(pw)->udir.y;
	udir->z = UGrafCurrent3(pw)->udir.z;
    }
}

/*
  XfwfUG3HorizontalDirection --
    Set the horizontal direction of the current transformation.
 */
void XfwfUG3HorizontalDirection(pw, hdir, set)
     UGrafWidget pw;
     Point3 *hdir;
     int set;
 {
    if (set) {
	UGrafCurrent3(pw)->hdir.x = hdir->x;
	UGrafCurrent3(pw)->hdir.y = hdir->y;
	UGrafCurrent3(pw)->hdir.z = hdir->z;
	UGrafCurrent3(pw)->valid = False;
    }
    else {
	hdir->x = UGrafCurrent3(pw)->hdir.x;
	hdir->y = UGrafCurrent3(pw)->hdir.y;
	hdir->z = UGrafCurrent3(pw)->hdir.z;
    }
}

/*
  XfwfUG3Aperture --
    Set the aperature of the transform.  This is used differently by
    each type of transform, but it should always be related to the
    opening angle in some way.  It's useful range should be between
    0.0 and 1.0, but it can take any value.
 */  
#ifdef NeedFunctionPrototypes
float XfwfUG3Aperture(UGrafWidget pw, float aperture, int set)
#else
float XfwfUG3Aperture(pw, aperture, set)
     UGrafWidget pw;
     float aperture;
     int set;
#endif
 {
    if (set) {
	UGrafCurrent3(pw)->aperture = aperture;
	UGrafCurrent3(pw)->valid = False;
    }
    return UGrafCurrent3(pw)->aperture;
}

/*
  XfwfUG3FromPoint --
    Transform from a 2D point to a 3D point.
 */
int XfwfUG3FromPoint(d2, d3, t)
     Point *d2;
     Point3 *d3;
     Transform3 *t;
 {
    if (!Methods(t)) return 0;	/* There should be methods installed. */
    if (!Methods(t)->To3D) return 0; /* There must be To3D */
    if (!t->valid)		/* Make the transform valid. */
	if (Methods(t)->SetTransform) 
	    Methods(t)->SetTransform(t);
    t->valid = True;		/* Flag it as valid. */
    d3->x = 0.0;
    d3->y = 0.0;
    d3->z = 0.0;
    return Methods(t)->To3D(d2, d3, t);
}

/*
 XfwfUG3ToPoint --
    Transform from a 3D point to a 2D point.
 */
int XfwfUG3ToPoint(d2, d3, t)
     Point *d2;
     Point3 *d3;
     Transform3 *t;
 {
    if (!Methods(t)) return 0;	/* There should be methods installed. */
    if (!Methods(t)->From3D) return 0; /* There must be From3D */
    if (!t->valid)		/* Make the transform valid. */
	if (Methods(t)->SetTransform) 
	    Methods(t)->SetTransform(t);
    t->valid = True;		/* Flag it as valid. */
    d2->x = 0.0;
    d2->y = 0.0;
    return Methods(t)->From3D(d2, d3, t);
}

/* 
  XfwfUG3Outline -- 
    Provide a list of points to outline the current transform.  This
    is used to provide borders.
 */
int XfwfUG3Outline(pw, p, d)
     UGrafWidget pw;
     Point *p;
     int *d;
 {
    if (!UGrafMethods3(pw)) return 0;
    if (!UGrafCurrent3(pw)->valid)
	if (UGrafMethods3(pw)->SetTransform) 
	    UGrafMethods3(pw)->SetTransform(UGrafCurrent3(pw));
    UGrafCurrent3(pw)->valid = True;
    if (!UGrafMethods3(pw)->Outline) return 0;
    return UGrafMethods3(pw)->Outline(p, d, UGrafCurrent3(pw));
}

/*
 * XfwfUG3SetMethod --
 *   Set the transform method for the current transform.
 */
Methods3 *XfwfUG3SetMethod(pw, m)
     UGrafWidget pw;
     Methods3 *m;
 {
    Methods3 *old = UGrafCurrent3(pw)->methods;
    XfwfUG3FreeTransform(UGrafCurrent3(pw));
    UGrafCurrent3(pw)->methods = m;
    UGrafCurrent3(pw)->valid = False;
    return old;
}

/*
  XfwfUG3Current --
    Set the index of the current three dimensional transformation.
 */
int  XfwfUG3Current(pw, transform)
     UGrafWidget pw;
     int transform;
 {
    int old = UGrafTransform3(pw);
    if (transform<0 || transform>=MaxWindows) transform = 0;
    UGrafTransform3(pw) = transform;
    return old;
}

/* 
   XfwfUG3Normalize --
     Normalize the view, up and horizontal directions 
 */
void XfwfUG3Normalize(t)
     Transform3 *t;
 {
    double v = InnerProduct(t->vdir,t->vdir);
    double u = InnerProduct(t->udir,t->udir);
    double h = InnerProduct(t->hdir,t->hdir);

    /* Check the normalization of the view direction. */
    if (v<0.1) {
	t->vdir.x = 1.0;	t->vdir.y = 0.0;	t->vdir.z =  0.0;
    }
    else {
	v = sqrt(v);
	t->vdir.x /= v; 	t->vdir.y /= v; 	t->vdir.z /= v; 
    }
	
    /* Check the normalization of the up direction */
    if (u<0.1) {
	t->udir.x = 0.0;	t->udir.y = 0.0;	t->udir.z =  1.0;
    }
    else {
	u = sqrt(u);
	t->udir.x /= u; 	t->udir.y /= u; 	t->udir.z /= u; 
    }	

    /* Check the normalization of the horizontal direction */
    if (h<0.1) {
	t->hdir.x = 0.0;	t->hdir.y = 1.0;	t->hdir.z =  0.0;
    }
    else {
	h = sqrt(h);
	t->hdir.x /= h; 	t->hdir.y /= h; 	t->hdir.z /= h; 
    }	
}

/******************
  The default transformation.
  *****************/
static Bool DefaultFrom3D(d2, d3, this)
     Point *d2;
     Point3 *d3;
     Transform3 *this;
 {
    Point3 o;
    double d;
    double x, y;

#ifdef TEST_INVALID
    /* Just to be bizarre, make an invalid band. */
    if (-0.2<d3->x && d3->x<0.2) return False;
    if (-0.2<d3->y && d3->y<0.2) return False;
    if (-0.2<d3->z && d3->z<0.2) return False;
#endif

    /* Shift to the reference point. */
    o.x = d3->x - this->refp.x;
    o.y = d3->y - this->refp.y;
    o.z = d3->z - this->refp.z;

    /* How far is it to the point along the view direction? */
    d = InnerProduct(o,this->vdir);

    if (d<=0.0) return False;	/* Point is in back. */
    
    /* Project onto the window */
    x = InnerProduct(o,this->hdir)/d;
    y = InnerProduct(o,this->udir)/d;

    /* Adjust the aperture */
    d2->x = x/(this->aperture);
    d2->y = y/(this->aperture);
    
    return True;
}

static Bool DefaultTo3D(d2, d3, this)
     Point *d2;
     Point3 *d3;
     Transform3 *this;
 {
    return False;
}

static int DefaultOutline(d2, draw, this)
     Point *d2;
     int *draw;
     Transform3 *this;
 {
    return 0;
}

static void DefaultSetTransform(this)
     Transform3 *this;
 {
    float r1, r2;
    XfwfUG3Normalize(this);
    if (this->aperture<0.001) this->aperture = 0.001;
    /* Make Up perpendicular to View */
    r1 = InnerProduct(this->udir, this->vdir);
    if (r1 > 0.9) {		/* Make sure udir and vdir aren't para. */
	this->udir.x = this->vdir.y;
	this->udir.y = this->vdir.z;
	this->udir.z = this->vdir.x;
	r1 = InnerProduct(this->udir, this->vdir);
    }
    this->udir.x -= r1*this->vdir.x; 
    this->udir.y -= r1*this->vdir.y; 
    this->udir.z -= r1*this->vdir.z; 
    /* Make Horizontal perpendicular to View and Up*/
    this->hdir.x = this->vdir.y*this->udir.z - this->udir.y*this->vdir.z;
    this->hdir.y = this->vdir.z*this->udir.x - this->udir.z*this->vdir.x;
    this->hdir.z = this->vdir.x*this->udir.y - this->udir.x*this->vdir.y;
    XfwfUG3Normalize(this);
}

static void DefaultFreeTransform(this)
     Transform3 *this;
 {
}

struct pnts {
    Point3 d3;			/* 3d point for this vertex. */
    Point d2;			/* 2d point corresponding to d3.  */
    Point a;			/* Average of n->d2 and p->d2 */
    Bool valid;			/* d2 a valid point. */
    Bool nobreaks;		/* segment after this vertex is not broken. */
    float toler;		/* The fractional misplacement of vertex. */
    int depth;			/* The number of divisions to this vertex. */
    struct pnts *n, *p;		/* The next and previous vertices. */
};

#ifdef TEST_PRINT
#define PrintLIST(a,b) _PrintLIST(a,b)
#else
#define PrintLIST(a,b)
#endif
static void _PrintLIST(p, n)
     struct pnts *p;
     int n;
 {
    int i = 0;
    struct pnts *t;
    printf("LIST\n");
    for (t=p; t; t = t->n) {
	if (!(i<n)) printf("****");
	printf("%d %x <%x %x> p(%f,%f,%f) d(%d) b(%d)\n",
	       i,
	       t,
	       t->p, t->n,
	       t->d3.x, t->d3.y, t->d3.z, t->depth, t->nobreaks);
	printf("       a(%f,%f) d(%f,%f) v(%d)\n", 
	       t->a.x, t->a.y, 
	       t->d2.x, t->d2.y,
	       t->valid);
	++i;
    }
}

#ifdef TEST_PRINT
#define PrintPNTS(a,b) _PrintPNTS(a,b)
#else
#define PrintPNTS(a,b)
#endif
static void _PrintPNTS(p, n)
     struct pnts *p;
     int n;
 {
    int i;
    printf ("POINTS\n");
    for (i=0; i<n; ++i) {
	printf("%d %x <%x %x> p(%f,%f,%f) d(%d) b(%d)\n",
	       i,
	       &p[i],
	       p[i].p, p[i].n,
	       p[i].d3.x, p[i].d3.y, p[i].d3.z, p[i].depth, p[i].nobreaks);
	printf("       a(%f,%f) d(%f,%f) v(%d)\n", 
	       p[i].a.x, p[i].a.y, 
	       p[i].d2.x, p[i].d2.y,
	       p[i].valid);
    }
}

/*
 * Check if a segment should be broken in half.  If it should be
 * broken, then  return 1, otherwise return 0. 
 */
static int BreakSegment(pw, h, t)
     UGrafWidget pw;
     struct pnts *h;
     Transform3 *t;
 {
    float sx, sy, Sx, Sy;

    /* Set the depth of this vertex. */
    h->depth = (h->p->depth > h->n->depth)? h->p->depth + 1:
	h->n->depth + 1;

    if (h->depth > UGrafTrans3(pw)->max_depth) return False;

    /* Split the segment in half. */
    h->d3.x = 0.5*(h->p->d3.x + h->n->d3.x);
    h->d3.y = 0.5*(h->p->d3.y + h->n->d3.y);
    h->d3.z = 0.5*(h->p->d3.z + h->n->d3.z);

    /* Find the 2d point at the mid point of the segment.  This */
    /* initializes both h->d2 and h->valid.  */
    h->valid = XfwfUG3ToPoint(&h->d2, &h->d3, t);

    /* The segment that this starts may need to be broke.  */
    h->nobreaks = False;
    
    /* Are the vertices valid. */
    if (!h->valid || !h->p->valid || !h->n->valid) {
	/* At least one is so divide to find the boundary */
	if (h->valid || h->p->valid || h->n->valid) {
	    h->p->n = h;
	    h->n->p = h;
	    h->toler = 1.0;
	    return 1;
	}
	/* None of the vertices are valid so dump this point. */
	else return 0;
    }

    /* Find the average of the previous and next point */
    h->a.x = 0.5*(h->p->d2.x + h->n->d2.x);
    h->a.y = 0.5*(h->p->d2.y + h->n->d2.y);

    /* Make sure the average and mid point are close together. */
    sx = h->a.x - h->d2.x;	/* Find tolerance for the X coordinate. */
    Sx = h->n->d2.x - h->p->d2.x;
    if (Sx>1.0E-10) sx = sx/Sx;
    if (sx<0.0) sx = -sx;

    sy = h->a.y - h->d2.y;	/* Find tolerance for the Y coordinate. */
    Sy = h->n->d2.y - h->p->d2.y;
    if (Sy>1.0E-10) sy = sy/Sy;
    if (sy<0.0) sy = -sy;

    /* Find the worst tolerance. */
    h->toler = (sx>sy) ? sx: sy;
    
    /* The point is to far from the average so insert another vertex */
    /* into the list.  */
    if (h->toler > UGrafTrans3(pw)->tolerance 
	|| h->depth < UGrafTrans3(pw)->min_depth) {
	h->p->n = h;
	h->n->p = h;
	return 1;
    }

    h->p->nobreaks = True;
    return 0;
}

/* 
 * Divide a line into segments so that it follows the geodesic.
 */
static int XfwfUG3BreakLine(pw, a, n, p, d, m)
     UGrafWidget pw;
     Point3 *a;
     int n;
     Point *p;
     int *d;
     int m;
 {
    int npts = 0, pt;
    struct pnts *pts;
    struct pnts *h;
    Transform3 *t = UGrafCurrent3(pw);
    
    pts = (struct pnts *) malloc(sizeof(struct pnts)*m);

    /* Transform the 3D vertex points into 2d. */
    for (npts = 0; npts < n; ++npts) {
	h = &pts[npts];
	h->depth = 0;
	if (npts>0) h->p = &pts[npts-1];
	else h->p = 0;
	if (npts<n-1) h->n = &pts[npts+1];
	else h->n = 0;
	h->d3.x = a[npts].x; h->d3.y = a[npts].y; h->d3.z = a[npts].z;
	h->valid = XfwfUG3ToPoint(&h->d2, &h->d3, t);
	h->nobreaks = False;
	h->a.x = h->d2.x;  h->a.y = h->d2.y;
    }
    npts = n;

    /* Check each segment to see if it needs to be split. */
    for (pt=1; pt<npts && (npts+2)<m ; ++pt) {
	if (pts[pt].p) {
	    h = &pts[npts];  h->p = pts[pt].p;  h->n = &pts[pt];
	    npts += BreakSegment(pw, h, t);
	}
	if (pts[pt].n) {
	    h = &pts[npts];  h->p = &pts[pt];  h->n = pts[pt].n;
	    npts += BreakSegment(pw, h, t);
	}
    }
    PrintLIST(pts,npts);

    /* Connect the nobreak points */
    pt = 0;
    for (h=&pts[0]; h; h = h->n) {
	p[pt].x = h->d2.x;  p[pt].y = h->d2.y; d[pt] = 0;
	if (h->p) {
	    if (h->p->nobreaks) d[pt] = 1;
	}
	if (h->valid) ++pt;
    }

    if (pts) free(pts);
    return pt;
}

#define MaxLineSegments 32
#ifdef NeedFunctionPrototypes
void XfwfUG3Line(UGrafWidget pw, float x, float y, float z, int d)
#else
void XfwfUG3Line(pw, x, y, z, d)
     UGrafWidget pw;
     float x;
     float y;
     float z;
     int d;
#endif
 {
    Point3 p[2];
    Point pts[MaxLineSegments];
    int draw[MaxLineSegments], n=MaxLineSegments;

    if (d) {
	/* Make the points. */
	p[0].x = UGrafCurrent3(pw)->last_point.x;
	p[0].y = UGrafCurrent3(pw)->last_point.y;
	p[0].z = UGrafCurrent3(pw)->last_point.z;

	p[1].x=x; p[1].y=y; p[1].z=z;

	/* Break up the segment between the last point and the next point */
	n = XfwfUG3BreakLine(pw,p,2,pts,draw,n);
	
	/* Draw the lines */
	XfwfUGLines(pw,pts,draw,n);
    }

    /* Save the current point as the last point */
    UGrafCurrent3(pw)->last_point.x = x;
    UGrafCurrent3(pw)->last_point.y = y;
    UGrafCurrent3(pw)->last_point.z = z;
}

#define MaxSegments 32
void XfwfUG3Lines(pw, p, d, n)
     UGrafWidget pw;
     Point3 *p;
     int *d;
     int n;
 {
    int i;
    Point3 s[MaxSegments];	/* The segments to draw. */
    int ns = 0;			/* The number of segments */
    Point3 *a;			/* A pointer into s[] */
    Point pts[MaxLineSegments*MaxSegments]; /* The 2d points */
    int draw[MaxLineSegments*MaxSegments]; /* The 2d drawable array */
    int npts = 0, max = MaxSegments*MaxLineSegments;

    /* Put the last point into the segment list. */
    a = &s[0];
    a->x = UGrafCurrent3(pw)->last_point.x;
    a->y = UGrafCurrent3(pw)->last_point.y;
    a->z = UGrafCurrent3(pw)->last_point.z;
    ++a; ns = 1;

    /* Loop through the points. */
    for (i=0; i<n; ++i) {
	if (!(*d)) {
	    if (ns>1) {
		/* Draw the line segments. */
		npts += XfwfUG3BreakLine(pw,s,ns,pts,draw,max);
		/* Draw the 2D points. */
		XfwfUGLines(pw,pts,draw,npts);
		npts = 0;
	    }
	    a = &s[0];
	    ns = 0;
	} else if (ns>MaxSegments-2) {
	    /* Draw the line segments. */
	    npts += XfwfUG3BreakLine(pw,s,ns,pts,draw,max);
	    /* Draw the 2D points. */
	    XfwfUGLines(pw,pts,draw,npts);
	    /* Save the last point */
	    a = &s[0]; --ns; 
	    a->x = s[ns].x; a->y = s[ns].y; a->z = s[ns].z;
	    /* Reset to the beginning */
	    ns = 0; npts = 0;

	}
	/* Put the next point onto the segments list */
	a->x = p->x;  a->y = p->y;  a->z = p->z;
	++a; ++ns; ++p; ++d;
    }

    if (ns>1) {
	/* Draw the line segments. */
	npts += XfwfUG3BreakLine(pw,s,ns,pts,draw,max);
	/* Draw the 2D points. */
	XfwfUGLines(pw,pts,draw,npts);
    }

}

#ifdef NeedFunctionPrototypes
void XfwfUG3Fill(UGrafWidget pw, Point3 *p, int n)
#else
void XfwfUG3Fill(pw, p, n)
     UGrafWidget pw;
     Point3 *p;
     int n;
#endif
 {/* This space is left unintentionally blank */}

#ifdef NeedFunctionPrototypes
void XfwfUG3Mark(UGrafWidget pw, Point3 *v, Point3 *u, Point3 *h, 
		 int mark, float scale)
#else
void XfwfUG3Mark(pw, v, u, h, mark, scale)
     UGrafWidget pw;
     Point3 *v;
     Point3 *u;
     Point3 *h;
     int mark;
     float scale;
#endif
 {
    int i;
    Point p[1000];
    int d[1000], npts;
    Point3 p3[1000];

    /* Get the 2D points of the text */
    npts = XfwfUGMarkPoints(pw, mark, p,d,1000);

    /* Translate to 3D points */
    for (i=0; i<npts; ++i) {
	p3[i].x = v->x 
	    + scale*(u->x*p[i].y + h->x*p[i].x);
	p3[i].y = v->y 
	    + scale*(u->y*p[i].y + h->y*p[i].x);
	p3[i].z = v->z 
	    + scale*(u->z*p[i].y + h->z*p[i].x);
    }

    /* Draw the lines */
    XfwfUG3Lines(pw, p3, d, npts);
}

void XfwfUG3Text(pw, v, u, h, text)
     UGrafWidget pw;
     Point3 *v;
     Point3 *u;
     Point3 *h;
     char *text;
 {
    int i;
    Point p[1000];
    int d[1000], npts;
    Point3 p3[1000];

    /* Get the 2D points of the text */
    npts = XfwfUGTextPoints(pw, text, p,d,1000);

    /* Translate to 3D points */
    for (i=0; i<npts; ++i) {
	p3[i].x = v->x 
	    + UGrafInstance(pw)->text.size*(u->x*p[i].y + h->x*p[i].x);
	p3[i].y = v->y 
	    + UGrafInstance(pw)->text.size*(u->y*p[i].y + h->y*p[i].x);
	p3[i].z = v->z 
	    + UGrafInstance(pw)->text.size*(u->z*p[i].y + h->z*p[i].x);
    }

    /* Draw the lines */
    XfwfUG3Lines(pw, p3, d, npts);
}

