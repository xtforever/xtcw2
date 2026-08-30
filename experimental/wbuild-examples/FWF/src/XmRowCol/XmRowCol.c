/*-------------------------------------------------------------------*/
/*                                                                   */
/*                      Widget RowCol                                */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/*   Auteur : Marc Quinton                                           */
/*     Date : Septembre 1995                                         */
/*                                                                   */
/*     Role : Fonctions standard du widget RowColum sous motif +     */
/*            ressource XmNposition qui permet de specifier          */
/*            l'emplacement des elements dans le tableau.            */
/*                                                                   */
/*  Co-auteurs :  Ce widget est initialement issu de la toolkit FWF  */
/*                C'est pourquoi je laisse au veritable auteur tous  */
/*                les honneurs en matiere d'innovation technique.    */
/*                En l'occurence : Bert Bos <bert@let.rug.nl>        */
/*                                                                   */
/*    Mon role : . integration a la toolkit Motif,                   */
/*               . heritage de constraint et ajout de la ressource   */
/*                 XmNposition.                                      */
/*                                                                   */
/*-------------------------------------------------------------------*/

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <Xm/XmP.h>
#include <Xm/ScrolledWP.h>
#include "XmRowColP.h"

/*-------------------------------------------------------------------*/
/* Function declaration                                              */
/*-------------------------------------------------------------------*/

/* core methods */
static void Initialize(Widget, Widget, ArgList, Cardinal*);
static void Resize(Widget);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal* );
static void Destroy(Widget);

/* composite methods */
static XtGeometryResult GeometryManager(Widget, XtWidgetGeometry*, XtWidgetGeometry*);
static void ChangeManaged(Widget);
static XtGeometryResult PreferredGeometry(Widget, XtWidgetGeometry*, XtWidgetGeometry*);

/* this method is invoqued by composite::insert_child */
/* NB : is different from RowColInsertPosition */
static unsigned int RowColInsertPos(Widget);


/* constraint methods */
static void             ConstraintInitialize();
static void             ConstraintDestroy();
static Boolean          ConstraintSetValues();

/* RowCol methods */
static void Layout(Widget, int);

/* usefull internal functions */
static void _align_child(Widget, Position, Position, Dimension, Dimension, Alignment);
static void _compute_inside( Widget, Position*, Position*, Dimension*, Dimension *);

/* functions to maintain position in widget array */
/* we also need to maintain XmNposition ressource to be real value */
static void RowColInsertPosition(Widget);
static void RowColSetPosition(Widget, Widget);
static void RowColDeletePosition(Widget);


/* Event Handler on Resize Window if parent is XmScrolledWindow */
static void ResizeParentWindow(Widget, XtPointer, XEvent*, Boolean *);

/* usefull defines */

#define CoreFieldOffset(f)	XtOffset(Widget,core.f)
#define	CompositeFieldOffset(f)	XtOffset(RowColWidget,composite.f)
#define RowColFieldOffset(f)	XtOffset(RowColWidget,rowCol.f)
#define RowColWidget(w)         ((RowColWidget)(w))

#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

/****************************************************************
 *
 * BoxRowCol Resources
 *
 ****************************************************************/

static XtResource resources[] = {
    { XmNstoreByRow,XmCStoreByRow,XmRBoolean,sizeof(Boolean),
                RowColFieldOffset(storeByRow),
                XmRImmediate,(XtPointer)True },

    {XmNrows,XmCRows,XmRInt,sizeof(int),
                RowColFieldOffset(rows),
		XmRImmediate,(XtPointer)0 },

    {XmNcolumns,XmCColumns,XmRInt,sizeof(int),
                RowColFieldOffset(columns),
                XmRImmediate,(XtPointer)0 },

    {XmNalignment,XmCAlignment,XmRAlignment, sizeof(Alignment),
                RowColFieldOffset(alignment),
                XmRImmediate,(XtPointer)XmTopLeft },

    {XmNshrinkToFit,XmCShrinkToFit,XmRBoolean,sizeof(Boolean),
                RowColFieldOffset(shrinkToFit),
                XmRImmediate,(XtPointer)False },

    {XmNuserData,XmCUserData,XmRPointer,sizeof(XtPointer),
                RowColFieldOffset(userData),
                XmRImmediate,(XtPointer)NULL },

    {XmNspacing,XmCSpacing,XmRDimension,sizeof(Dimension),
                RowColFieldOffset(spacing),
                XmRImmediate,(XtPointer)3 },

#ifdef FRAME
    {XtNframeType,XtCFrameType,XtRFrameType,sizeof(FrameType),
                XtOffsetOf(XfwfRowColRec,xfwfFrame.frameType),
                XtRImmediate,(XtPointer)XfwfSunken },

    {XtNframeWidth,XtCFrameWidth,XtRDimension,sizeof(Dimension),
                XtOffsetOf(XfwfRowColRec,xfwfFrame.frameWidth),
                XtRImmediate,(XtPointer)2},
#endif

};


static XtResource  rowColConstraintResources[] = {
    {XmNposition,XmCPosition,XmRInt,sizeof(int),
                XtOffset(RowColConstraints, rowCol.position),
                XmRImmediate,(XtPointer)0 }
};
/*===========================================================================*

                    C L A S S    A L L O C A T I O N

 *===========================================================================*/

RowColClassRec rowColClassRec =
{
{/* core class fields   */
 /* superclass		*/	(WidgetClass)&constraintClassRec,
 /* class_name		*/	"RowCol",
 /* widget_size		*/	sizeof(RowColRec),
 /* class_initialize	*/	NULL,
 /* class_part_initialize*/	NULL /* _resolve_inheritance */,
 /* class_inited	*/	FALSE,
 /* initialize		*/	Initialize,
 /* initialize_hook	*/	NULL,
 /* realize		*/	XtInheritRealize,
 /* actions		*/	NULL,
 /* num_actions		*/	0,
 /* resources		*/	resources,
 /* resource_count	*/	XtNumber(resources),
 /* xrm_class		*/	NULLQUARK,
 /* compress_motion	*/	TRUE,
 /* compress_exposure	*/	XtExposeCompressMultiple /* FALSE */,
 /* compress_enterleave	*/	True,
 /* visible_interest	*/	FALSE,
 /* destroy		*/	Destroy,
 /* resize		*/	Resize,
 /* expose		*/	NULL,
 /* set_values		*/	SetValues,
 /* set_values_hook	*/	NULL,
 /* set_values_almost	*/	XtInheritSetValuesAlmost,
 /* get_values_hook	*/	NULL,
 /* accept_focus	*/	XtInheritAcceptFocus,
 /* version		*/	XtVersion,
 /* callback_private	*/	NULL,
 /* tm_table		*/	NULL /* defaultTranslations */,
 /* query_geometry      */	PreferredGeometry,
 /* display_accelerator */	XtInheritDisplayAccelerator,
 /* extension           */	NULL,
}, 

{   /* composite_class fields */

    /* geometry_manager   */    GeometryManager,
    /* change_managed     */    ChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension          */	NULL
  },
{   /* constraint_class fields */

    /* subresources       */    rowColConstraintResources,
    /* subresources_count */    XtNumber(rowColConstraintResources),
    /* constraint_size    */    sizeof(RowColConstraintsRec),
    /* initialize         */    ConstraintInitialize,
    /* destroy            */    ConstraintDestroy,
    /* set_values         */    ConstraintSetValues,
    /* extension          */    NULL
},
{   /* RowCol class fields */

 /* layout_Proc		*/	Layout
}
};

WidgetClass rowColWidgetClass = (WidgetClass)&rowColClassRec;



/*===========================================================================*

                    C L A S S    Methods

 *===========================================================================*/


/* =========================================================================*/
/*                                                                          */
/*                     core methods                                         */
/*                                                                          */
/* =========================================================================*/

/*------------------------------------------------------------------------

     Initialize

 *------------------------------------------------------------------------
  The initialize method sets the private variables, in this case only
  max_width and max_height. There is no need to check if the resources
  have sensible values.
 *------------------------------------------------------------------------*/

static void Initialize(Widget request, Widget new, ArgList args, Cardinal *argc)
{
#ifdef DEBUG_0
     printf("RowCol::Initialize()\n");
#endif

    /* MQ : to avoid zero width and/or height */
    if(RowColWidget(new)->core.width == 0)
         RowColWidget(new)->core.width = 10;

    if(RowColWidget(new)->core.height == 0)
         RowColWidget(new)->core.height = 10;

    RowColWidget(new)->rowCol.max_width = RowColWidget(new)->rowCol.max_height = 0;

    /* initialize insert_position function  : RowColInsertPos() */
    RowColWidget(new)->composite.insert_position = RowColInsertPos;

    /* MQ : if parent is an XmScrolledWindow, and resize policy is         */
    /* XmAUTOMATIC add an event handler on resize to be notify and         */
    /* do new layout.                                                      */
    /*---------------------------------------------------------------------*/
    if(XmIsScrolledWindow(XtParent(new)))
    {
         XtInsertEventHandler(XtParent(new), StructureNotifyMask,
            False, ResizeParentWindow, new, XtListTail);

    }
} /* Initialize */


/* Event Handler on Resize Window if parent is XmScrolledWindow */
/* On resize just redo a new layout.                            */
/*--------------------------------------------------------------*/
static void ResizeParentWindow(Widget sw, XtPointer rc, XEvent *event, Boolean *cont)
{
    Layout((Widget)rc, ((RowColWidget)rc)->rowCol.shrinkToFit);
    *cont = True;
}


/*------------------------------------------------------------------------

     Destroy

 *------------------------------------------------------------------------
 *------------------------------------------------------------------------*/

static void Destroy(Widget self)
{
#ifdef DEBUG_0
     printf("RowCol::Destoy()\n");
#endif

    /* MQ : if parent is an XmScrolledWindow, and resize policy is         */
    /* XmAUTOMATIC remove event handler on resize                          */
    /*---------------------------------------------------------------------*/
    if(XmIsScrolledWindow(XtParent(self)))
    {
         XtRemoveEventHandler(XtParent(self), StructureNotifyMask,
            False, ResizeParentWindow, self);

    }
} /* Resize */

/*------------------------------------------------------------------------

     Resize

 *------------------------------------------------------------------------
    The |resize| method is called when the widget is resized. If the
    |rows| and |columns| resources are both zero, the children will have
    to be be re-aligned. In this case, there is no sense in asking the
    parent for a new size, so |layout| is passed a value of |False|.
 *------------------------------------------------------------------------*/

static void Resize(Widget w)
{
#ifdef DEBUG
     printf("RowCol::Resize()\n");
#endif

    if (RowColWidget(w)->rowCol.rows    == 0  && 
        RowColWidget(w)->rowCol.columns == 0)

        Layout(w, False);
} /* Resize */


/*------------------------------------------------------------------------

     SetValues

 *------------------------------------------------------------------------
    The RowCol widget needs to recompute the positions of the children
    when one of the resources changes. When the layout changes, the widget
    also needs to be redrawn, of course.  The private variables are not
    dependent on the resources, so they don't need recomputing.
 *------------------------------------------------------------------------*/

static Boolean SetValues(
    Widget              current,
    Widget              request,
    Widget              new,
    ArgList             args,
    Cardinal            *argc)
{
    Boolean need_layout = False;
    Boolean need_redisplay = False;

#ifdef DEBUG_0
     printf("RowCol::SetValues()\n");
#endif
    if(RowColWidget(current)->rowCol.storeByRow != RowColWidget(new)->rowCol.storeByRow) need_layout = True;
    if (RowColWidget(current)->rowCol.rows != RowColWidget(new)->rowCol.rows) need_layout = True;
    if (RowColWidget(current)->rowCol.columns != RowColWidget(new)->rowCol.columns) need_layout = True;
    if (RowColWidget(current)->rowCol.alignment != RowColWidget(new)->rowCol.alignment) need_layout = True;
    if (RowColWidget(current)->rowCol.shrinkToFit != RowColWidget(new)->rowCol.shrinkToFit) need_layout = True;
    if (need_layout) {
	Layout(new, RowColWidget(new)->rowCol.shrinkToFit);
	need_redisplay = True;
    }
    return need_redisplay;

} /* SetValues */

/* =========================================================================*/
/*                                                                          */
/*                     composite methods                                    */
/*                                                                          */
/* =========================================================================*/

/*------------------------------------------------------------------------

     GeometryManager

 *------------------------------------------------------------------------
    When a child wants to change its size or border width, it calls its
    parent's |geometry_manager| method (through a call to
    |XtMakeGeometryRequest| or |XtMakeResizeRequest|.) The RowCol widget
    always grants size changes to its children. The size change is carried
    out immediately and a new layout is computed. If a child requests a
    change of position, the request is denied. A request for a change in
    stacking order is ignored.
 *------------------------------------------------------------------------*/

static XtGeometryResult GeometryManager(Widget child, XtWidgetGeometry *request, XtWidgetGeometry *reply)
{
    Widget rc = XtParent(child);
    Dimension newwd, newht, newbd;

#ifdef DEBUG_0
     printf("RowCol::GeometryManager(%X,%X)\n", child, request->request_mode);

     printf("RowCol::GeometryManager(%X, request(%d,%d,%d,%d)(%X,%X))\n", child,
		request->x, request->y,
		request->width, request->height,
		request->request_mode, reply->request_mode);
#endif

    if (request->request_mode & XtCWQueryOnly)
         return XtGeometryYes;


    newwd = request->request_mode & CWWidth ? request->width : child->core.width;
    newht = request->request_mode & CWHeight ? request->height : child->core.height;
    newbd = request->request_mode & CWBorderWidth
	? request->border_width : child->core.border_width;


    /* Resize widget as we need */
    XtResizeWidget(child, newwd, newht, newbd);

    /* do new layout, because a child change  */
    /*----------------------------------------*/
    Layout(rc, ((RowColWidget)rc)->rowCol.shrinkToFit);

    /* We have done the work so return always XtGeometryDone */
    /*-------------------------------------------------------*/
    return XtGeometryDone;

} /* GeometryManager */

/*------------------------------------------------------------------------

     ChangeManaged

 *------------------------------------------------------------------------
    If a child becomes managed or unmanaged, the RowCol widget
    recomputes the positions of all managed children. That is done by a
    method |layout|. In the process, the widget may ask its parent for a
    different size if |shrinkToFit = True|
 *------------------------------------------------------------------------*/

static void ChangeManaged(Widget self)
{
#ifdef DEBUG_0
     printf("RowCol::ChangeManaged()\n");
#endif
    Layout(self, ((RowColWidget)self)->rowCol.shrinkToFit);

} /* ChangeManaged */


/*------------------------------------------------------------------------

     PreferredGeometry

 *------------------------------------------------------------------------

 *------------------------------------------------------------------------*/

static XtGeometryResult PreferredGeometry(
    Widget               self, 
    XtWidgetGeometry    *request,
    XtWidgetGeometry    *preferred)
{
#ifdef DEBUG_0
     printf("RowCol::PreferredGeometry(%X)\n", self);
#endif

     /* if no changes are being made to width or height : just agree */
     if( !(request->request_mode & CWWidth ) &&
         !(request->request_mode & CWHeight ))
         return (XtGeometryYes);

    printf("request : %d, %d\n", request->width, request->height);

    preferred->request_mode = (CWWidth | CWHeight);
    preferred->width = request->width;
    preferred->height = request->height;

    return XtGeometryAlmost;

} /* PreferredGeometry */


/*------------------------------------------------------------------------

     RowColInsertPos

 *------------------------------------------------------------------------
     This method is called bye composite::insert_child, to know index
     position of insert child.
 *------------------------------------------------------------------------*/

static unsigned int RowColInsertPos(Widget self)
{
     RowColConstraints constraints = (RowColConstraints)self->core.constraints;
     RowColWidget      rc = (RowColWidget) self->core.parent;

#ifdef DEBUG_0
     printf("RowCol::RowColInsertPos(%d)\n", constraints->rowCol.position);
#endif
     /* test */
     /* return 0; OK : widget a l'envert */
     /* return rc->composite.num_children; : c'est le comportement normal ! */
     /* commence a 0 ... n */

     if(     constraints->rowCol.position <= 0
         ||  constraints->rowCol.position > rc->composite.num_children
       )
         return rc->composite.num_children;
     else
         return MAX(0,(constraints->rowCol.position -1));

}
/* =========================================================================*/
/*                                                                          */
/*                     constraint methods                                   */
/*                                                                          */
/* =========================================================================*/
static void ConstraintInitialize (
    Widget       request,
    Widget       new)
{
     Widget parent = XtParent(new);

#ifdef DEBUG_0
     printf("RowCol::ConstraintInitialize(%X,X%)\n", request, new);
#endif

     RowColInsertPosition(new);
     Layout(parent, ((RowColWidget)parent)->rowCol.shrinkToFit);
}

static Boolean ConstraintSetValues (
    Widget       current,
    Widget       request,
    Widget       new, 
    ArgList      args,
    Cardinal     num_args)
{
     Widget parent = XtParent(new);

#ifdef DEBUG_0
     printf("RowCol::ConstraintSetValues(%X,%X,X%)\n", current, request, new);
#endif

     RowColSetPosition(current, new);
     Layout(parent, ((RowColWidget)parent)->rowCol.shrinkToFit);
}

static void ConstraintDestroy (Widget w)
{
     Widget parent = XtParent(w);

#ifdef DEBUG_0
     printf("RowCol::ConstraintDestroy(%X)\n", w);
#endif

     /* No need to do any thing RowColDeletePosition() */
     /* RowColDeletePosition(w); */
     Layout(parent, ((RowColWidget)parent)->rowCol.shrinkToFit);
}

/* =========================================================================*/
/*                                                                          */
/*                     RowCol methods                                       */
/*                                                                          */
/* =========================================================================*/
/*------------------------------------------------------------------------

     Layout

 *------------------------------------------------------------------------
    The |layout| function is responsible for moving the children to their
    positions in the grid. It is called from |change_managed|,
    |geometry_manager| and |resize|.
    
    The function first computes the maximum width and height of all the
    children, including their borders. Then it computes the number of rows
    and columns. All children are moved to their proper positions with the
    help of a utility function |align_child|, which aligns a child widget
    to the grid.
    
    If |shrink = True|, the RowCol widget also asks its parent for a new
    width and/or height, depending on the resulting layout.
 *------------------------------------------------------------------------*/

static void Layout(Widget self, int shrink)
{

    int nrows, ncols, i, nchild, n;
    Position left, top, x, y;
    Dimension width, height, w, h;
    Widget child;
    RowColWidget rc = (RowColWidget) self;

    RowColConstraints constraints;

#ifdef DEBUG_0
     printf("RowCol::Layout(%X)\n", self);
#endif

    if(rc->composite.num_children == 0)
    {
        return;
    }
    nchild = 0;
    rc->rowCol.max_width = 0;
    rc->rowCol.max_height = 0;
    for (i = 0; i < rc->composite.num_children; i++) {
	child = rc->composite.children[i];
	if (! XtIsManaged(child))
            continue;
	nchild++;
	rc->rowCol.max_width = MAX(rc->rowCol.max_width, child->core.width + 2*child->core.border_width);
	rc->rowCol.max_height = MAX(rc->rowCol.max_height, child->core.height + 2*child->core.border_width);

        /* MQ : update XmNposition ressource for reading width    */
        /*                                                        */
        /*      XtVaGetValues(w,                                  */
        /*         XmNposition,   &pos,                           */
        /*         NULL);                                         */
        /*--------------------------------------------------------*/
        constraints = (RowColConstraints)child->core.constraints;
	constraints->rowCol.position = i+1;
    }

    _compute_inside(self, &left, &top, &width, &height);

    if (rc->rowCol.columns != 0) {
	ncols = rc->rowCol.columns;
	nrows = (nchild + ncols - 1)/ncols;
    } else if (rc->rowCol.rows != 0) {
	nrows = rc->rowCol.rows;
	ncols = (nchild + nrows - 1)/nrows;
    } else {
        /* MQ : */
        if(shrink)
        {
           /* NB : hierarchie is :  ScrolledWin, ScrolledWindowClipWindow, rc(self) */
            Widget parent = XtParent(XtParent(self));
            XmScrolledWindowWidget sw = (XmScrolledWindowWidget) parent;

            if(XmIsScrolledWindow(parent))
            {
                /* get Work Area of ScrolledWindow */
                width  = sw->swindow.AreaWidth - sw->swindow.pad;
                height = sw->swindow.AreaHeight - sw->swindow.pad;
            } else
            {
                /* ncols = nrows = 0, and we want to shrink window */
                width = parent->core.width;
                height = parent->core.height;
            }
        }
        if(rc->rowCol.max_width == 0)
            ncols = 1;
        else
	    ncols = (width - rc->rowCol.spacing)/(rc->rowCol.max_width + (rc->rowCol.spacing));

	if (ncols == 0) ncols = 1;
	nrows = (nchild + ncols - 1)/ncols;
    }

    x = left + rc->rowCol.spacing;
    y = top + rc->rowCol.spacing;
    n = 0;
    if (rc->rowCol.storeByRow) {
	for (i = 0; i < rc->composite.num_children; i++) {
	    child = rc->composite.children[i];
	    if (! XtIsManaged(child))
                continue;
	    _align_child(child, x, y, rc->rowCol.max_width, rc->rowCol.max_height, rc->rowCol.alignment);
	    n++;
	    if (n == ncols) {
		n = 0;
		x = left + rc->rowCol.spacing;
		y += rc->rowCol.max_height + rc->rowCol.spacing;
	    } else
		x += rc->rowCol.max_width + rc->rowCol.spacing;
	}
    } else {
	for (i = 0; i < rc->composite.num_children; i++) {
	    child = rc->composite.children[i];
	    if (! XtIsManaged(child))
                continue;
	    _align_child(child, x, y, rc->rowCol.max_width, rc->rowCol.max_height, rc->rowCol.alignment);
	    n++;
	    if (n == nrows) {
		n = 0;
		y = top + rc->rowCol.spacing;
		x += rc->rowCol.max_width + rc->rowCol.spacing;
	    } else
		y += rc->rowCol.max_height + rc->rowCol.spacing;
	}
    }

    if (shrink) {
	w = 2*left + ncols * rc->rowCol.max_width + ((ncols +1) * rc->rowCol.spacing);
	h = 2*top + nrows * rc->rowCol.max_height + ((nrows +1) * rc->rowCol.spacing);

        if( w <= 0) w = 10;
        if( h <= 0) h = 10;

#if 0
        /* MQ : it is forbiden to resize our self */
	if (rc->rowCol.columns != 0)
	    XtVaSetValues(self, 
                XmNwidth,       w,
                XmNheight,      h, 
                NULL);
	else
	    XtVaSetValues(self, 
                XmNheight,      h,
                NULL);
#else
        {

            /* this is the preconised form for resizing */
            XtGeometryResult result;
            Dimension replyWidth, replyHeight;

            result = XtMakeResizeRequest(self,
               w, h, &replyWidth, &replyHeight);

            if(result == XtGeometryAlmost)
                  XtMakeResizeRequest(self, replyWidth, replyHeight, NULL, NULL);
                
        }
#endif

    }
} /* Layout */

/* =========================================================================*/
/*                                                                          */
/*                     usefull functions                                    */
/*                                                                          */
/* =========================================================================*/
static void _align_child(
    Widget      self,
    Position    cx, 
    Position    cy,
    Dimension   width,
    Dimension   height,
    Alignment   alignment)
{
    Position x, y;

#ifdef DEBUG_0
     printf("RowCol::_align_child(%s)\n", XtName(self));
#endif

    if (alignment & XmLeft) x = cx;
    else if (alignment & XmRight) x = cx + width - self->core.width;
    else x = cx + (width - self->core.width) / 2;
    if (alignment & XmTop) y = cy;
    else if (alignment & XmBottom) y = cy + height - self->core.height;
    else y = cy + (height - self->core.height) / 2;

    if ((self->core.x != x) || (self->core.y != y))
    {
        XtMoveWidget(self, x, y);
    }
}



/*
    _compute_inside()

    A new method |compute_inside| is defined, that returns the area
    inside the highlight border. Subclasses should use this to compute
    their drawable area, in preference to computing it from |$width| and
    |$height|. Subclasses, such as the Frame widget, redefine the method
    if they add more border material.

*/

static void _compute_inside(
    Widget        self,
    Position     *x,
    Position     *y,
    Dimension    *w,
    Dimension    *h)
{
#ifdef DEBUG_0
    printf("RowCol::_compute_inside()\n");
#endif

    *x = 0;  /* could be hightLightBorder */
    *y = 0;
    *w = RowColWidget(self)->core.width;
    *h = RowColWidget(self)->core.height;
   
}


static void RowColInsertPosition(
    Widget w)
{
     RowColConstraints constraints = (RowColConstraints)w->core.constraints;
     RowColWidget      rc = (RowColWidget) w->core.parent;
     int               position = constraints->rowCol.position;

#ifdef DEBUG_0
     printf("RowCol::RowColInsertPosition(%s : %d)\n",XtName(w), position);
#endif


#if 0
    for (i = 0; i < rc->composite.num_children; i++) {
	child = rc->composite.children[i];
	if (! XtIsManaged(child)) continue;
	nchild++;
	rc->rowCol.max_width = MAX(rc->rowCol.max_width, child->core.width + 2*child->core.border_width);
	rc->rowCol.max_height = MAX(rc->rowCol.max_height, child->core.height + 2*child->core.border_width);

    }
#endif
}

/*-----------------------------------------------------------------------------*/
/* This function changes widget position in widget array : composite->children */
/* in order to manage resources positionment widget XmNposition                */
/*-----------------------------------------------------------------------------*/
static void RowColSetPosition(
    Widget   current,
    Widget   new)
{
     RowColConstraints constraints = (RowColConstraints)new->core.constraints;
     RowColConstraints current_constraints = (RowColConstraints)current->core.constraints;
     RowColWidget      rc = (RowColWidget) new->core.parent;
     Widget            *children = rc->composite.children;
     Widget            old;

     int               to = constraints->rowCol.position;
     int               from   = current_constraints->rowCol.position;
     int               i;

#ifdef DEBUG_0
     printf("RowCol::RowColSetPosition(%X,%X)\n",current, new);
#endif

    /* si to vaut zero, cela signifie                               */
    /* qu'on veut le dernier element a la place.                    */
    /*--------------------------------------------------------------*/
    if(    ( to <= 0)
        || ( to > rc->composite.num_children)
      )
        to = rc->composite.num_children;


    if( from == to)
        /* no thing to do ! just return :-) */
        /*----------------------------------*/
        return;

    /* le premier element commence a 1 et dans un tableau en lagage C */
    /* le premeier element possede l'index 0; on ajuste donc ceci     */
    /*----------------------------------------------------------------*/
    from--; to--;


    /* pour le decalage dans le tableau des widgets                   */
    /* composite.childrens[] on dissocie deux cas possibles :         */
    /*----------------------------------------------------------------*/
    if(from > to)
    {
        old = children[from];
        for(i = from ; i > to ; i--)
            children[i] = children[i-1];

        children[to] = old;      
    }
    else
    {
        old = children[from];
        for(i = from ; i < to ; i++)
            children[i] = children[i+1];

        children[to] = old;

    }
}


/* Is never called : this is probably composite class that does the jobs for us */

static void RowColDeletePosition(Widget w)
{
     RowColConstraints constraints = (RowColConstraints)w->core.constraints;
     RowColWidget      rc = (RowColWidget) w->core.parent;
     int               position = constraints->rowCol.position;
     int i;

#ifdef DEBUG_0
     printf("RowCol::RowColDeletePosition(%X : %d)\n",w, position);
#endif

#if 0
    for (i = 0 ; i < rc->composite.num_children; i++)
       printf("children[%d] = %X\n", i+1, rc->composite.children[i]);
#endif
}
