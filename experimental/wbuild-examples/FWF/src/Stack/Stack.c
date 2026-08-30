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

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <X11/Xmu/Converters.h>

#include "StackP.h"

#define CORE(a) ((a)->core)
#define COMP(a) ((a)->composite)
#define STACK(a) ((a)->stack)

#define CHILDREN(a) ((a)->composite.children)
#define NUM_CHILDREN(a) ((a)->composite.num_children)

#define BWIDTH(a) ((a)->core.width + 2 * (a)->core.border_width)
#define BHEIGHT(a) ((a)->core.height + 2 * (a)->core.border_width)

#define HEIGHT(a) ((a)->core.height)
#define WIDTH(a) ((a)->core.width)
#define BORDER(a) ((a)->core.border_width)

#define SHOWING(a) ((a)->stack.stackWidget)
#define OK_TO_LAYOUT(a) ((a)->stack.okToLayout)
#define STACK_TYPE(a) ((a)->stack.stackType)
#define SAME_SIZE(a) ((a)->stack.sameSize)
#define FILL_ALL(a) ((a)->stack.fill)

/* 
** Constraint stuff.
*/

#define GRAVITY(a) (((XfwfStackConstraint)((a)->core.constraints))->stack.gravity)
#define FILL(a) (((XfwfStackConstraint)((a)->core.constraints))->stack.fill)

#if MOTIF 
#define SWIDTH(a) ((a)->core.width - (a)->manager.shadow_thickness * 2)
#define SHEIGHT(a) ((a)->core.height - (a)->manager.shadow_thickness * 2)
#define MANAGER(a) ((a)->manager)
#else
#define SWIDTH(a) WIDTH(a)
#define SHEIGHT(a) HEIGHT(a)
#endif /* MOTIF */

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif /* MAX */
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif /* MIN */

#if MOTIF

#if (XmREVISION == 2)

#define RESIZE(a,b,c,d) _XmResizeObject((Widget)(a), (b), (c), (d))
#define MOVE(a,b,c) _XmMoveObject((Widget)(a), (b), (c))
#define CONFIG(a,b,c,d,e,f) _XmConfigureObject((Widget)(a), (b), (c), (d), (e), (f))

#else

#define RESIZE(a,b,c,d) _XmResizeObject((RectObj)(a), (b), (c), (d))
#define MOVE(a,b,c) _XmMoveObject((RectObj)(a), (b), (c))
#define CONFIG(a,b,c,d,e,f) _XmConfigureObject((RectObj)(a), (b), (c), (d), (e), (f))

#endif /* (XmREVISION == 2) */

#else

#define RESIZE(a,b,c,d) XtResizeWidget((a), (b), (c), (d))
#define MOVE(a,b,c) XtMoveWidget((a), (b), (c))
#define CONFIG(a,b,c,d,e,f) XtConfigureWidget((a), (b), (c), (d), (e), (f))

#endif /* MOTIF */

#if NeedFunctionPrototypes

static XtProc classInit();
static XtInitProc initialize(XfwfStackWidget _request, XfwfStackWidget _new, String *_args, Cardinal *_numArgs);
static XtWidgetProc insertChild(Widget _w);
static XtWidgetProc deleteChild(Widget _w);
static XtWidgetProc resize(XfwfStackWidget _sw);
static XtGeometryResult geometryManager(Widget _w, XtWidgetGeometry *_request, XtWidgetGeometry *_reply);
static XtWidgetProc changeManaged(XfwfStackWidget _sw);
static XtGeometryResult queryGeometry(XfwfStackWidget _sw, XtWidgetGeometry *_request, XtWidgetGeometry *_pref);
static Bool setValues(XfwfStackWidget _current, XfwfStackWidget _request,  XfwfStackWidget _new, ArgList _args, Cardinal *_numArgs);
static Bool constraintSetValues(Widget _old, Widget _request, Widget _new, ArgList _args, Cardinal *_numArgs);

static void forceOneManagedWidget(XfwfStackWidget _sw);
static void _nextWidget(XfwfStackWidget _sw);
static void _previousWidget(XfwfStackWidget sw);
static void findLargestGeometry(XfwfStackWidget _sw, Dimension *_width, Dimension *_height, Dimension *_border, Bool _pref);
static void layoutChildren(XfwfStackWidget _sw);
static XtGeometryResult tryLayout(XfwfStackWidget _sw, Mask *_mask);


#else

static XtProc classInit();
static XtInitProc initialize();
static XtWidgetProc insertChild();
static XtWidgetProc deleteChild();
static XtWidgetProc resize();
static XtGeometryResult geometryManager();
static XtWidgetProc changeManaged();
static XtGeometryResult queryGeometry();
static Bool setValues();
static Bool constraintSetValues();

static void forceOneManagedWidget();
static void _nextWidget();
static void _previousWidget();
static void findLargestGeometry();
static void layoutChildren();
static XtGeometryResult tryLayout();

#endif /* NeedFunctionPrototypes */

#define LOCALCONSTRAINTREC XfwfStackConstraintRec

#define TheOffset(field) XtOffset(XfwfStackWidget, stack.field)
static XtResource resources[] =
{
	{XfwfNdata, XfwfCData, XtRPointer, sizeof(XtPointer),
		TheOffset(data), XtRImmediate, (XtPointer)NULL},
	{XfwfNmargin, XfwfCMargin, XtRDimension, sizeof(Dimension),
		TheOffset(margin), XtRImmediate, (XtPointer)2},
	{XfwfNsameSize, XfwfCSameSize, XtRBool, sizeof(Bool),
		TheOffset(sameSize), XtRImmediate, (XtPointer)FALSE},
	{XfwfNfill, XfwfCFill, XtRBool, sizeof(Bool),
		TheOffset(fill), XtRImmediate, (XtPointer)FALSE},
	{XfwfNstackWidget, XfwfCStackWidget, XtRWidget, sizeof(Widget),
		TheOffset(stackWidget), XtRImmediate, (XtPointer)NULL},
	{XfwfNstackType, XfwfCStackType, XfwfRStackType, sizeof(unsigned char),
		TheOffset(stackType), XtRImmediate, (XtPointer)XfwfSTACK_END_TO_END},
};

#undef TheOffset

#define TheOffset(field) XtOffset(XfwfStackConstraint, stack.field)
static XtResource constraintResources[] =
{
	{XfwfNgravity, XfwfCGravity, XtRGravity, sizeof(int),
		TheOffset(gravity), XtRImmediate, (XtPointer)CenterGravity},
	{XfwfNfill, XfwfCFill, XtRBool, sizeof(Bool),
		TheOffset(fill), XtRImmediate, (XtPointer)FALSE},
};

XfwfStackClassRec xfwfStackClassRec = 
{
	{ /* Core Part */
#if MOTIF
	(WidgetClass)&xmManagerClassRec,		/* superclass */
#else
	(WidgetClass)&constraintClassRec,		/* superclass */
#endif /* MOTIF */
	"XfwfStack",						 	/* class_name */
	(Cardinal)sizeof(XfwfStackRec),			/* widget size */
	(XtProc)classInit,						/* class_init */
	(XtWidgetClassProc)NULL,				/* class_part_init */
	(XtEnum)FALSE,							/* class_inited */
	(XtInitProc)initialize,					/* initialize */ 
	(XtArgsProc)NULL,						/* init_hook */
	XtInheritRealize,						/* realize */
	(XtActionList)NULL,						/* actions */
	(Cardinal)0,							/* num_actions */
	(XtResourceList)resources,				/* resources */
	(Cardinal)XtNumber(resources),			/* num_resources */
	NULLQUARK,								/* xrm_class */
	FALSE,									/* compress_motion */
	(XtEnum)FALSE,							/* compress_exposur */
	FALSE,									/* compress enterleave */
	FALSE,									/* visibility_interest */
	(XtWidgetProc)NULL,						/* destroy */
	(XtWidgetProc)resize,					/* resize */
	XtInheritExpose,						/* expose */
	(XtSetValuesFunc)NULL, 					/* set_values */
	(XtArgsFunc)NULL,						/* set_values_hook */ 
	XtInheritSetValuesAlmost,				/* set_values_almost */
	(XtArgsProc)NULL,						/* get_values_hook */
	XtInheritAcceptFocus,					/* accept_focus */
	XtVersion,								/* version */
	(XtPointer)NULL,						/* callback_private */
	XtInheritTranslations,					/* translations */
	(XtGeometryHandler)queryGeometry,		/* query_geometry */
	XtInheritDisplayAccelerator,			/* display_accelerator */
	(XtPointer)NULL,						/* extension */
	},
	{ /* Composite Part */
	(XtGeometryHandler)geometryManager,							 /* geometry_manager */
	(XtWidgetProc)changeManaged,								 /* change_managed */
	(XtWidgetProc)insertChild,						/* inherit_child */		
	(XtWidgetProc)deleteChild,						/* delete_child */
	NULL,										/* extension */
	},
{	/* Constraint Part */
	(XtResourceList)constraintResources,
	XtNumber(constraintResources),
	(Cardinal)sizeof(LOCALCONSTRAINTREC), 
	(XtInitProc)NULL,
	(XtWidgetProc)NULL,
	(XtSetValuesFunc)constraintSetValues,
	(XtPointer)NULL,
},
#if MOTIF
{	/* Manager Part */
	(String)XtInheritTranslations,
	(XmSyntheticResource *)NULL,
	0,
	(XmSyntheticResource *)NULL,
	0,
	(XmParentProcessProc)XmInheritParentProcess,
	(XtPointer)NULL,
},
#endif /* MOTIF */
{	/* Stack Part */
	0,
},
};

WidgetClass xfwfStackWidgetClass = (WidgetClass)&xfwfStackClassRec;

static XtProc classInit()
{
	XtSetTypeConverter(XtRString, XtRGravity, (XtTypeConverter)XmuCvtStringToGravity,
		(XtConvertArgList)NULL, 0, XtCacheAll, (XtDestructor)NULL);

	XtSetTypeConverter(XtRString, XfwfRStackType, (XtTypeConverter)FwfCvtStringToStackType,
		(XtConvertArgList)NULL, 0, XtCacheAll, (XtDestructor)NULL);

	return (XtProc)NULL;
}

static XtInitProc initialize(_request, _new, _args, _numArgs)
XfwfStackWidget _request;
XfwfStackWidget _new;
String *_args;
Cardinal *_numArgs;
{
	Cardinal numParms;
	XtWidgetGeometry pref;

	if (CORE(_new).width <= 0)
		CORE(_new).width = 20;

	if (CORE(_new).height <= 0)
		CORE(_new).height = 20;

	OK_TO_LAYOUT(_new) = TRUE;

	return (XtInitProc)NULL;
}

static XtWidgetProc insertChild(_w)
Widget _w;
{
	XfwfStackWidget sw;

	sw = (XfwfStackWidget)XtParent(_w);

	(*((CompositeWidgetClass)(xfwfStackWidgetClass->core_class.superclass))->composite_class.insert_child)(_w);

	if (SHOWING(sw) == NULL)
		SHOWING(sw) = _w;

#if 0
	forceOneManagedWidget(sw);
#endif

	return (XtWidgetProc)NULL;
}

static XtWidgetProc deleteChild(_w)
Widget _w;
{
	XfwfStackWidget sw;
	Bool needToChange;

	sw = (XfwfStackWidget)XtParent(_w);

	needToChange = (SHOWING(sw) == _w);

	(*((CompositeWidgetClass)(xfwfStackWidgetClass->core_class.superclass))->composite_class.delete_child)(_w);

	SHOWING(sw) = NULL;

	if (needToChange)
		forceOneManagedWidget(sw);

	return (XtWidgetProc)NULL;
}

static XtWidgetProc resize(_sw)
XfwfStackWidget _sw;
{

	layoutChildren(_sw);

	return (XtWidgetProc)NULL;
}

#define MODE(a,b) ((a)->request_mode & (b))

static XtGeometryResult queryGeometry(_sw, _request, _pref)
XfwfStackWidget _sw;
XtWidgetGeometry *_request;
XtWidgetGeometry *_pref;
{
	Dimension width, height, border;
	Cardinal numManagedChildren = 0;

	_pref->border_width = CORE(_sw).border_width;
	_pref->request_mode = (CWWidth | CWHeight);

	if (!XtIsManaged(SHOWING(_sw)))
	{
		_pref->width = 20;
		_pref->height = 20;
		return XtGeometryYes;
	}

	findLargestGeometry(_sw, &width, &height, &border, TRUE);

	_pref->width = width + 2 * border;
	_pref->height = height + 2 * border;

#if MOTIF
	_pref->width += (2 * MANAGER(_sw).shadow_thickness);
	_pref->height += (2 * MANAGER(_sw).shadow_thickness);
#endif /* MOTIF */

	if (MODE(_request, XtCWQueryOnly))
		return XtGeometryYes;

	if (MODE(_request, CWWidth) && MODE(_request, CWHeight))
	{
		if ( (_pref->width == _request->width) && (_pref->height == _request->height) )
		{
			_pref->request_mode = (CWWidth | CWHeight);
			_pref->width = _request->width;
			_pref->height = _request->height;
			return XtGeometryYes;
		}
		else
		if ( (_pref->width > _request->width) && (_pref->height > _request->height) )
			return XtGeometryNo;
		else
			return XtGeometryAlmost;
	}
	else
	if (MODE(_request, CWWidth))
	{
		if (_pref->width == _request->width) 
		{
			_pref->width = _request->width;
			return XtGeometryYes;
		}
		else 
		if (_pref->width < _request->width)
			return XtGeometryAlmost;
		else
			return XtGeometryNo;
	}
	else
	if (MODE(_request, CWHeight))
	{
		if (_pref->height == _request->height)
		{
			_pref->height = _request->height;
			return XtGeometryYes;
		}
		else 
		if (_pref->height < _request->height)
			return XtGeometryAlmost;
		else
			return XtGeometryNo;
	}

	return XtGeometryYes;
}

static XtGeometryResult geometryManager(_w, _request, _reply)
Widget _w;
XtWidgetGeometry *_request;
XtWidgetGeometry *_reply;
{
	XfwfStackWidget sw;
	Mask mask;
	XtGeometryResult result;

	sw = (XfwfStackWidget)XtParent(_w);

	_reply->request_mode = _request->request_mode;

	if ( MODE(_request, CWWidth | CWHeight | CWBorderWidth) )
	{
		Dimension oldWidth, oldHeight, oldBW;
		Dimension replyWidth, replyHeight;
		Dimension width, height;

		oldWidth = WIDTH(_w);
		oldHeight = HEIGHT(_w);
		oldBW = BORDER(_w); 

		if (MODE(_request, CWWidth))
			WIDTH(_w) = _request->width;
		if (MODE(_request, CWHeight))
			HEIGHT(_w) = _request->height;
		if (MODE(_request, CWBorderWidth))
			BORDER(_w) = _request->border_width;

		result = tryLayout(sw, &mask);

		if (result == XtGeometryNo)
		{
			WIDTH(_w) = oldWidth;
			HEIGHT(_w) = oldHeight;
			BORDER(_w) = oldBW;
			return XtGeometryNo;
		}

		if (result == XtGeometryAlmost)
		{
			_reply->request_mode = _request->request_mode;
			if (!(mask & CWWidth))
			{
				_reply->width = WIDTH(_w) = oldWidth;
				_reply->border_width = BORDER(_w) = oldBW;
			}
			if (!(mask & CWHeight))
				_reply->height = HEIGHT(_w) = oldHeight;

			return XtGeometryAlmost;
		}

		if (_request->request_mode != XtCWQueryOnly)
			layoutChildren(sw);

		_reply->width = WIDTH(_w);
		_reply->height = HEIGHT(_w);
		_reply->border_width = BORDER(_w);

		if (_request->request_mode == XtCWQueryOnly)
		{
			WIDTH(_w) = oldWidth;
			HEIGHT(_w) = oldHeight;
			BORDER(_w) = oldBW;
			return XtGeometryYes;
		}

		return XtGeometryYes;
	}

	return XtGeometryYes;
}

static XtGeometryResult tryLayout(_sw, _mask)
XfwfStackWidget _sw;
Mask *_mask;
{
	Dimension width, height, border;
	Dimension trueWidth, trueHeight;

	findLargestGeometry(_sw, &width, &height, &border, FALSE);

	width += (2 * border);
	height += (2 * border);

	if ( (width != SWIDTH(_sw)) || (height != SHEIGHT(_sw)) )
	{
		XtGeometryResult result;
		Dimension replyWidth, replyHeight;

		if (width == SWIDTH(_sw))
			trueWidth = WIDTH(_sw);
		else
		{
			trueWidth = width;
#if MOTIF
			trueWidth += (2 * MANAGER(_sw).shadow_thickness);
#endif
		}

		if (height == SHEIGHT(_sw))
			trueHeight = HEIGHT(_sw);
		else
		{
			trueHeight = height;
#if MOTIF
			trueHeight += (2 * MANAGER(_sw).shadow_thickness);
#endif /* MOTIF */
		}

		while((result = 
			XtMakeResizeRequest((Widget)_sw, trueWidth, trueHeight, &replyWidth, &replyHeight)) == XtGeometryAlmost)
		{
			trueWidth = replyWidth;
			trueHeight = replyHeight;
		}

		*_mask = 0;

		if ((width <= replyWidth) && (height <= replyHeight))
		{
#if MOTIF
			replyWidth -= (2 * MANAGER(_sw).shadow_thickness);
			replyHeight -= (2 * MANAGER(_sw).shadow_thickness);
#endif
			layoutChildren(_sw);
			return XtGeometryYes;
		}
		else
		if (width <= replyWidth)
		{
			*_mask = CWHeight;
			return XtGeometryAlmost;
		}
		else
		if (height <= replyHeight)
		{
			*_mask = CWWidth;
			return XtGeometryAlmost;
		}
		else
			return XtGeometryNo;
	}

	*_mask = CWWidth | CWHeight;
	layoutChildren(_sw);
	return XtGeometryYes;
}

static Bool setValues(_current, _request, _new, _args, _numArgs)
XfwfStackWidget _current;
XfwfStackWidget _request;
XfwfStackWidget _new;
ArgList _args;
Cardinal *_numArgs;
{
	Bool change = FALSE;

	if (SHOWING(_current) != SHOWING(_new))
	{
		forceOneManagedWidget(_new);
		change = TRUE;
	}

	if (SAME_SIZE(_current) != SAME_SIZE(_new))
	{
		resize(_new);
		change = TRUE;
	}

	if (FILL_ALL(_current) != FILL_ALL(_new))
	{
		resize(_new);
		change = TRUE;
	}

	return change;
}

static Bool constraintSetValues(_old, _request, _new, _args, _numArgs)
Widget _old;
Widget _request;
Widget _new;
ArgList _args;
Cardinal *_numArgs;
{
	XfwfStackWidget sw;

	sw = (XfwfStackWidget)XtParent(_new);

	resize(sw);

	return TRUE;
}

static XtWidgetProc changeManaged(_sw)
XfwfStackWidget _sw;
{
	Mask mask;

	if (!OK_TO_LAYOUT(_sw))
		return;

	forceOneManagedWidget(_sw);

	tryLayout(_sw, &mask);

#if MOTIF
	 _XmNavigChangeManaged((Widget)_sw);
#endif /* MOTIF */

	return (XtWidgetProc)NULL;
}

static void layoutChildren(_sw)
XfwfStackWidget _sw;
{
	XtWidgetGeometry childPref;
	Widget child;
	int i;
	Bool anyFilled;
	Dimension border;
	Dimension width, height, sWidth, sHeight;

	if (!OK_TO_LAYOUT(_sw))
		return;

	sWidth = SWIDTH(_sw);
	sHeight = SHEIGHT(_sw);

	anyFilled = FILL_ALL(_sw);

	if (SAME_SIZE(_sw) && !anyFilled)
	{
		for(i=0; i<NUM_CHILDREN(_sw); i++)
		{
			child = *(CHILDREN(_sw)+i);
			if (FILL(child))
			{
				anyFilled = TRUE;
				break;
			}
		}
	}

	if (!anyFilled)
		findLargestGeometry(_sw, &width, &height, &border, FALSE);
	else
	{
		width = SWIDTH(_sw);
		height = SHEIGHT(_sw);
	}

	for(i=0; i<NUM_CHILDREN(_sw); i++)
	{
		child = *(CHILDREN(_sw)+i);
		if (!FILL_ALL(_sw) && !SAME_SIZE(_sw) && !FILL(child))
			XtQueryGeometry(child, NULL, &childPref);
		else
		{
			childPref.width = width;
			childPref.height = height;
			childPref.border_width = BORDER(child);
		}

		switch(GRAVITY(child))
		{
			case NorthWestGravity:
				childPref.x = 0;
				childPref.y = 0;
				break;
			case NorthGravity:
				childPref.x = (sWidth - (childPref.width + 2 * childPref.border_width))/2;
				childPref.y = 0;
				break;
			case WestGravity:
				childPref.x = 0;
				childPref.y = (sHeight - (childPref.height + 2 * childPref.border_width))/2;
				break;
			case SouthWestGravity:
				childPref.x = 0;
				childPref.y = sHeight - (childPref.height + 2 * childPref.border_width);
				break;
			case SouthGravity:
				childPref.x = (sWidth - (childPref.width + 2 * childPref.border_width))/2;
				childPref.y =	sHeight - (childPref.height + 2 * childPref.border_width);
				break;
			case SouthEastGravity:
				childPref.x =  sWidth - (childPref.width + 2 * childPref.border_width);
				childPref.y = sHeight - (childPref.height + 2 * childPref.border_width);
				break;
			case EastGravity:
				childPref.x = sWidth - (childPref.width + 2 * childPref.border_width);
				childPref.y = (sHeight - (childPref.height + 2 * childPref.border_width))/2;
				break;
			case NorthEastGravity:
				childPref.x = (sWidth - (childPref.width + 2 * childPref.border_width))/2;
				childPref.y = (sHeight - (childPref.height + 2 * childPref.border_width))/2;
				break;
			case ForgetGravity: /* Not really a valid choice. */ 
			case CenterGravity:
				childPref.x = (sWidth - (childPref.width + 2 * childPref.border_width))/2;
				childPref.y = (sHeight - (childPref.height + 2 * childPref.border_width))/2;
				break;
		}

		if (childPref.x < 0)
			childPref.x = 0;
		if (childPref.y < 0)
			childPref.y = 0;

		CONFIG(child, childPref.x, childPref.y, childPref.width, childPref.height, childPref.border_width);
	}
}

static void forceOneManagedWidget(_sw)
XfwfStackWidget _sw;
{
	Widget child;
	int i;

	if (NUM_CHILDREN(_sw) == 0)
		return;

	if (!OK_TO_LAYOUT(_sw))
		return;

	OK_TO_LAYOUT(_sw) = FALSE;

	if (SHOWING(_sw) == (Widget)NULL)
		SHOWING(_sw) = *(CHILDREN(_sw));

	for(i=0; i<NUM_CHILDREN(_sw); i++)
	{
		child = *(CHILDREN(_sw)+i);
		if (SHOWING(_sw) != child)
		{
			if (XtIsManaged(child))
				XtUnmanageChild(child);
		}
		else
		{
			if (!XtIsManaged(child))
				XtManageChild(child);
		}
	}

	OK_TO_LAYOUT(_sw) = TRUE;
}

static void _nextWidget(_sw)
XfwfStackWidget _sw;
{
	int i;

	if (NUM_CHILDREN(_sw) == 0)
		return;

	for(i=0;i<NUM_CHILDREN(_sw);i++)
	{
		if (SHOWING(_sw) == *(CHILDREN(_sw)+i))
			break;
	}

	if ((i+1) < NUM_CHILDREN(_sw))
		SHOWING(_sw) = *(CHILDREN(_sw)+i+1);
	else
	if (STACK_TYPE(_sw) == XfwfSTACK_CIRCULAR)
		SHOWING(_sw) = *CHILDREN(_sw);

	forceOneManagedWidget(_sw);
}

static void _previousWidget(_sw)
XfwfStackWidget _sw;
{
	int i;

	if (NUM_CHILDREN(_sw) == 0)
		return;

	for(i=0;i<NUM_CHILDREN(_sw);i++)
	{
		if (SHOWING(_sw) == *(CHILDREN(_sw)+i))
			break;
	}

	if (i > 0)
		SHOWING(_sw) = *(CHILDREN(_sw)+i-1);
	else
	if (STACK_TYPE(_sw) == XfwfSTACK_CIRCULAR)
		SHOWING(_sw) = *(CHILDREN(_sw) + NUM_CHILDREN(_sw) - 1);

	forceOneManagedWidget(_sw);
}

static void findLargestGeometry(_sw, _width, _height, _border, _pref)
XfwfStackWidget _sw;
Dimension *_width;
Dimension *_height;
Dimension *_border;
Bool _pref;
{
	int i;
	Widget child;

	child = NULL;

	*_width = *_height = *_border = 0;

	for(i=0; i<NUM_CHILDREN(_sw); i++)
	{
		child = *(CHILDREN(_sw)+i);
		*_width = MAX(*_width, WIDTH(child));
		*_height = MAX(*_height, HEIGHT(child));
		*_border = MAX(*_border, BORDER(child));
	}
} 


static void toLower(_str1, _str2, _length)
char *_str1;
char *_str2;
int _length;
{
    int i;
    char *ptr;

    for(ptr=_str1,i=0;(ptr!=NULL) && (i<_length);ptr++,i++)
        *(_str2+i) = tolower(*ptr);
}

/*
** This should be moved to some common file. Cwikla
*/

Bool FwfCvtStringToStackType(_display, _args, _numArgs, _from, _to, _data)
Display *_display;
XrmValuePtr _args;
Cardinal *_numArgs;
XrmValuePtr _from;
XrmValuePtr _to;
XtPointer *_data;
{
	char *lower;
	static unsigned char stackType;
	Bool badConversion = FALSE;

	if (*_numArgs != 0)
	{
		XtAppWarningMsg(XtDisplayToApplicationContext(_display), "CvtStringToStackType", "wrongParamaters",
		"ResourceError",
		"FwfCvtStringToStackType needs no arguments.",
		(String *)NULL, (Cardinal *)NULL);
	}

	lower = XtNewString(_from->addr);
	toLower(_from->addr, lower, strlen(_from->addr));

	stackType = XfwfSTACK_END_TO_END;

	if (!strncmp(lower, "end_to_end", 10))
		stackType = XfwfSTACK_END_TO_END;
	else
	if (!strncmp(lower, "circular",	8))
		stackType = XfwfSTACK_CIRCULAR;
	else
		badConversion = TRUE;

	XtFree(lower);

	if (badConversion)
		XtDisplayStringConversionWarning(_display, _from->addr, XfwfRStackType);
	else
	{
		if (_to->addr == NULL)
			_to->addr = (XtPointer)&stackType;
		else
		if (_to->size < sizeof(unsigned char))
			badConversion = TRUE;
		else
			*(unsigned char *)_to->addr = stackType;
			_to->size = sizeof(unsigned char);
	}

	return !badConversion;
}


/*
** PUBLIC
*/

void XfwfStackNextWidget(_w)
Widget _w;
{
	if (!XfwfIsStack(_w))
		return;

	_nextWidget((XfwfStackWidget)_w);
}

void XfwfStackPreviousWidget(_w)
Widget _w;
{
	if (!XfwfIsStack(_w))
		return;

	_previousWidget((XfwfStackWidget)_w);
}
