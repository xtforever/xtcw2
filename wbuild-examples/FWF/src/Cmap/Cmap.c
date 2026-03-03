/*
 * Copyright 1992,1993 John L. Cwikla
 * 
 *
 * Author:
 * 	John L. Cwikla
 *	Wolfram Research, Inc.
 *	cwikla@wri.com
 * 
 * Thanks to:
 *
 *	Pierre-Yves Bonnetain
 *	SILOGIC
 *	78, chemin des Sept Deniers
 *	31200 TOULOUSE
 *	FRANCE.
 * 
 *  who provided the improvement to use the shell's visual.
 */

/*
** Cmap Widget
*/

#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/ShellP.h>

#if MOTIF
#include <Xm/LabelP.h>
#endif

#include <math.h>
#include <varargs.h>

#include "CmapP.h"

/* For actions */
#if NeedFunctionProtoTypes
static void left(CmapWidget _cw, XEvent *_event, String *_argv, int *_argc);
static void right(CmapWidget _cw, XEvent *_event, String *_argv, int *_argc);
static void up(CmapWidget _cw, XEvent *_event, String *_argv, int *_argc);
static void down(CmapWidget _cw, XEvent *_event, String *_argv, int *_argc);
static void wselect(CmapWidget _cw, XEvent *_event, String *_argv, int *_argc);
#else
static void left();
static void right();
static void up();
static void down();
static void wselect();
#endif

/* For widget internals */
#if NeedFunctionProtoTypes
static void initialize(CmapWidget _request, CmapWidget _new);
static void realize(CmapWidget _cw, XtValueMask *_xvm, XSetWindowAttributes *_xswa);
static void resize(CmapWidget _cw);
static void destroy(CmapWidget _cw);
static void redisplay(CmapWidget _cw, XEvent *_event, Region _region);
static Boolean setValues(CmapWidget _current, CmapWidget _request, CmapWidget _new);
#if MOTIF
static void borderHighlight(CmapWidget _cw);
static void borderUnhighlight(CmapWidget _cw);
#endif /* MOTIF */
static void myXtWarning();
#else
static void initialize();
static void realize();
static void resize();
static void destroy();
static void redisplay();
static Boolean setValues();
#if MOTIF
static void borderHighlight();
static void borderUnhighlight();
#endif /* MOTIF */
static void myXtWarning();
#endif

/* For Misc */
#if NeedFunctionProtoTypes
static int xYtoBox(CmapWidget _cw, int _x, int _y);
static void reString(CmapWidget _cw);
static void drawLabel(CmapWidget _cw, GC _gc);
static void drawSelected(CmapWidget _cw, int _boxNumber);
static void drawBox(CmapWidget _cw, int _boxNumber);
static void getFont(CmapWidget _cw);
#else
static int xYtoBox();
static void reString();
static void drawLabel();
static void drawSelected();
static void drawBox();
static void getFont();
#endif

static char cmapTranslations[] =
#if !MOTIF
"<Key>h:				left()\n\
 <Key>l:	right()\n\
 <Key>k:				up()\n\
 <Key>j:				down()\n\
 <Btn1Down>:		select()\n\
 <Btn1Motion>:	select()";
#else
#if (XmREVISION == 2)
"~s <Key>osfLeft: left()\n\
~s <Key>osfRight: right()\n\
~s <Key>osfUp: up() \n\
~s <Key>osfDown: down() \n\
<Btn1Down>: select()\n\
<Btn1Motion>: select()\n\
~s ~m ~a <Key>Return: PrimitiveParentActivate()\n\
<Key>osfActivate: PrimitiveParentActivate()\n\
<Key>osfCancel:PrimitiveParentCancel()\n\
<Enter>:PrimitiveEnter()\n\
<Leave>:PrimitiveLeave()\n\
s ~m ~a <Key>Tab: PrimitivePrevTabGroup()\n\
~m ~a <Key>Tab: PrimitiveNextTabGroup()\n\
Shift<Key>osfUp:PrimitiveTraverseUp()\n\
Shift<Key>osfDown:PrimitiveTraverseDown()\n\
Shift<Key>osfLeft:PrimitiveTraverseLeft()\n\
Shift<Key>osfRight:PrimitiveTraverseRight()\n\
<FocusIn>:PrimitiveFocusIn()\n\
<FocusOut>:PrimitiveFocusOut()\n\
<Unmap>:PrimitiveUnmap()\n\
<Key>osfHelp:PrimitiveHelp()";
#else
"~s <Key>osfLeft: left()\n\
~s <Key>osfRight: right()\n\
~s <Key>osfUp: up() \n\
~s <Key>osfDown: down() \n\
<Btn1Down>: select()\n\
<Btn1Motion>: select()\n\
s ~m ~a <Key>Tab: PrimitivePrevTabGroup()\n\
~m ~a <Key>Tab: PrimitiveNextTabGroup()\n\
Shift<Key>osfUp:PrimitiveTraverseUp()\n\
Shift<Key>osfDown:PrimitiveTraverseDown()\n\
Shift<Key>osfLeft:PrimitiveTraverseLeft()\n\
Shift<Key>osfRight:PrimitiveTraverseRight()\n\
<FocusIn>:PrimitiveFocusIn()\n\
<FocusOut>:PrimitiveFocusOut()\n\
<Unmap>:PrimitiveUnmap()\n\
<Key>osfHelp:PrimitiveHelp()";
#endif /* XmREVISION */
#endif /* MOTIF */

static XtActionsRec cmapActions[] = 
{
	{ "left", left },
	{ "right", right },
	{ "up", up },
	{ "down", down },
	{ "select", wselect}
};

#define RGB_STRING "R:%d G:%d B:%d (%d of %d)"

#define BOXtoX(w, b) (((b) % w->cmap.numX) * w->cmap.realBoxWidth + w->cmap.drawPos.x)
#define BOXtoY(w, b) (((b) / w->cmap.numX) * w->cmap.realBoxHeight + w->cmap.drawPos.y) 
#define ROW(w, a) ((a) / w->cmap.numX)
#define COL(w, a) ((a) % w->cmap.numX)

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define CORE(a) ((a)->core)
#define PRIM(a) ((a)->primitive)
#define CMAP(a) ((a)->cmap)
#define FONT_LIST(a) ((a)->cmap.fontList)
#define THE_FONT(a) ((a)->cmap.font)

#if MOTIF
#define FG(a) ((a)->primitive.foreground)
#define TX(a) ((a)->primitive.highlight_thickness)
#define TY(a) ((a)->primitive.highlight_thickness)
#define TWIDTH(a) ((a)->core.width - 2 * (a)->primitive.highlight_thickness)
#define THEIGHT(a) ((a)->core.height - 2 * (a)->primitive.highlight_thickness)
#define CMAP_MINBOXWIDTH(a) (10 + (a)->primitive.shadow_thickness * 2 + (a)->primitive.highlight_thickness * 2)
#define CMAP_MINBOXHEIGHT(a) (10 + (a)->primitive.shadow_thickness * 2 + (a)->primitive.highlight_thickness * 2)
#define ST(a) ((a)->primitive.shadow_thickness)
#define HT(a) ((a)->primitive.highlight_thickness)
#define HRECT(a) ((a)->cmap.hrects)
#define ISHAD(a) ((a)->cmap.invertedShadows)

#else

#define FG(a) ((a)->cmap.foreground)
#define TX(a) (0)
#define TY(a) (0)
#define TWIDTH(a) ((a)->core.width)
#define THEIGHT(a) ((a)->core.height)
#define CMAP_MINBOXWIDTH(a) (10)
#define CMAP_MINBOXHEIGHT(a) (10)
#define ST(a) (0)
#define HT(a) (0)

#endif /* MOTIF */

#if MOTIF
#if (XmREVISION == 2) && 0 /* disabled, BB, 6 Mar 1995 */
extern XmPrimitiveClassExtRec _XmPrimClassExtRec;
#define LOCALPRIMITIVEEXTREC &_XmPrimClassExtRec
#else
#define LOCALPRIMITIVEEXTREC NULL
#endif
#endif /* MOTIF */

#define TheOffset(field) XtOffset(CmapWidget, cmap.field)

static XtResource cmapResources[] = 
{
#if MOTIF
	{XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
		TheOffset(fontList), XmRImmediate, (XtPointer)NULL},
#else
	{XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
		TheOffset(font), XtRString, (XtPointer)XtDefaultFont},
#endif /* MOTIF */
	{XtNmargin, XtCMargin, XtRDimension, sizeof(Dimension),
		TheOffset(margin), XtRImmediate, (XtPointer)2},
	{XtNchangeCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
		TheOffset(changeCallback), XtRCallback, (XtPointer)NULL},
	{XtNselected, XtCSelected, XtRInt, sizeof(int),
		TheOffset(selected), XtRImmediate, (XtPointer)0},
	{XtNboxWidth, XtCBoxWidth, XtRInt, sizeof(int),
		TheOffset(boxWidth), XtRImmediate, (XtPointer)0},
	{XtNboxHeight, XtCBoxHeight, XtRInt, sizeof(int),
		TheOffset(boxHeight), XtRImmediate, (XtPointer)0},
	{XtNfirstIndex, XtCFirstIndex, XtRInt, sizeof(int),
		TheOffset(firstIndex), XtRImmediate, (XtPointer)0},
	{XtNlastIndex, XtCLastIndex, XtRInt, sizeof(int),
		TheOffset(lastIndex), XtRImmediate, (XtPointer)LastIndexDefault},
	{XtNmappingTable, XtCMappingTable, XtRPointer, sizeof(XtPointer),
		TheOffset(mappingTable), XtRImmediate, (XtPointer)NULL},
#if DEBUGGING
	{XtNstandardColormap, XtCStandardColormap, XtRAtom, sizeof(Atom),
		TheOffset(standardColormap), XtRImmediate, (XtPointer)0},
#endif
#if !MOTIF
	{XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
		TheOffset(foreground), XtRString, (XtPointer)XtDefaultForeground},
#endif /* !MOTIF */
#if MOTIF
	{XtNshowUnselectedShadows, XtCShowShadows, XtRBoolean, sizeof(Boolean),
		TheOffset(showUnselectedShadows), XtRImmediate, (XtPointer)FALSE},
	{XtNinvertedShadows, XtCInvertedShadows, XtRBoolean, sizeof(Boolean),
		TheOffset(invertedShadows), XtRImmediate, (XtPointer)FALSE},
#endif /* MOTIF */
};

#undef TheOffset

CmapClassRec cmapClassRec = 
{
	{				/* CoreClassPart */
#if MOTIF
	(WidgetClass)&xmPrimitiveClassRec, /* superclass */
#else
		(WidgetClass)&widgetClassRec,	/* superclass */
#endif /* MOTIF */
		"Cmap",												/* class_name */
		sizeof(CmapRec),							 /* widget_size */
		NULL,													/* class_initialize */
		NULL,													/* class_part_initialize */
		FALSE,												 /* class_init */
		initialize,										/* initialize */
		NULL,													/* initialize_hook */
		realize,											 /* realize */
		cmapActions,									 /* actions */
		XtNumber(cmapActions),				 /* num_actions */
		cmapResources,								 /* resources */
		XtNumber(cmapResources),			 /* num_resources */
		NULLQUARK,										 /* xrm_class */
		TRUE,													/* compress_motion */
		XtExposeCompressMultiple |
		XtExposeGraphicsExposeMerged,													/* compress_exposure */
		TRUE,													/* compress_enterleave */
		TRUE,													/* visible_intress */
		destroy,											 /* destroy */
		resize,												/* resize */
		redisplay,										 /* expose */
		setValues,										 /* set_values */
		NULL,													/* set_values_hook */
		XtInheritSetValuesAlmost,			/* set_values_almost */
		NULL,													/* get_values_hook */
		NULL,													/* accept_focus */
		XtVersion,										 /* version */
		NULL,													/* callback_private */
		cmapTranslations,							/* tm_translations */
		NULL,
		NULL,
		NULL,
	},
#if MOTIF
{
	borderHighlight,
	borderUnhighlight,
	NULL, /* XtInheritTranslations, */
	(XtActionProc)NULL,
	(XmSyntheticResource *)NULL,
	0,
	(XtPointer)LOCALPRIMITIVEEXTREC,
},
#endif /* MOTIF */
	{ 
		0, /* empty */
	}
};

WidgetClass cmapWidgetClass = (WidgetClass) &cmapClassRec;

static void initialize(_request, _new)
CmapWidget _request;
CmapWidget _new;
{
	Display *display;
	XColor color;
	Widget parent;

	display = XtDisplay(_new);

	getFont(_new);

	if (CMAP(_new).firstIndex < 0)
	{
		myXtWarning("CmapWidget: FirstIndex (%d) cannot be negative. FirstIndex set to 0.", CMAP(_new).firstIndex);
		CMAP(_new).firstIndex = 0;
	}

	parent = XtParent(_new);

	while(!XtIsShell(parent))
		parent = XtParent(_new);

	if (parent == (Widget)NULL)
		CMAP(_new).availColors = XDisplayCells(display, DefaultScreen(display));
	else
	{
		XVisualInfo *vout, vtemplate;
		int numVis;
		ShellWidget shellw;

		shellw = (ShellWidget)parent;

		if (shellw->shell.visual)
		{

			vtemplate.visualid = XVisualIDFromVisual(shellw->shell.visual);
			vout = XGetVisualInfo (display, VisualIDMask, &vtemplate, &numVis);

			if (vout != (XVisualInfo *)NULL)
			{
				CMAP(_new).availColors = vout[0].colormap_size;
				XFree(vout);
			}
			else
				CMAP(_new).availColors = XDisplayCells(display, DefaultScreen(display));
		}
		else
			CMAP(_new).availColors = XDisplayCells(display, DefaultScreen(display));
	}
		
	if (CMAP(_new).lastIndex == LastIndexDefault)
		CMAP(_new).lastIndex = CMAP(_new).availColors-1; 

	if (CMAP(_new).firstIndex > CMAP(_new).availColors-1)
	{
		myXtWarning("CmapWidget: FirstIndex (%d) cannot be more than greatest index (%d).	FirstIndex set to %d.",
			CMAP(_new).firstIndex, (CMAP(_new).availColors-1), (CMAP(_new).availColors-1));
		CMAP(_new).firstIndex = CMAP(_new).availColors-1;
	}

	if (CMAP(_new).lastIndex < CMAP(_new).firstIndex)
	{
		myXtWarning("CmapWidget: LastIndex (%d) cannot be less than FirstIndex (%d).	LastIndex set to %d.", 
			CMAP(_new).lastIndex, CMAP(_new).firstIndex, CMAP(_new).firstIndex+1);
		CMAP(_new).lastIndex = CMAP(_new).firstIndex + 1;
	}

	if ((CMAP(_new).mappingTable == NULL) && (CMAP(_new).lastIndex > CMAP(_new).availColors-1))
	{
		myXtWarning("CmapWidget: LastIndex (%d) cannot be more than greatest index (%d).	LastIndex set to %d.",
			CMAP(_new).lastIndex, CMAP(_new).availColors-1, CMAP(_new).availColors-1);
		CMAP(_new).lastIndex = CMAP(_new).availColors-1;
	}

	if (CMAP(_new).mappingTable == NULL)
		CMAP(_new).usedColors = MIN(CMAP(_new).lastIndex-CMAP(_new).firstIndex+1, CMAP(_new).availColors);
	else
		CMAP(_new).usedColors = CMAP(_new).lastIndex-CMAP(_new).firstIndex+1;

	CMAP(_new).numX = (int)floor(sqrt((double)(CMAP(_new).usedColors)));
	CMAP(_new).numY = CMAP(_new).usedColors/CMAP(_new).numX + (CMAP(_new).usedColors % CMAP(_new).numX ? 1 : 0);

	if (CMAP(_new).selected > CMAP(_new).usedColors)
	{
		myXtWarning("CmapWidget: Selected pixel (%d) is greater than %d. Selected set to %d.",
			CMAP(_new).selected, CMAP(_new).usedColors-1, CMAP(_new).usedColors-1);
		CMAP(_new).selected = CMAP(_new).usedColors-1;
	}

	if (CMAP(_new).selected < 0)
	{
		myXtWarning("CmapWidget: Selected pixel cannot be negative! Selected set to 0.");
		CMAP(_new).selected = 0;
	}


	if (CMAP(_new).mappingTable == (Pixel *)NULL)
		color.pixel = CMAP(_new).selected+CMAP(_new).firstIndex;
	else
		color.pixel = ((Pixel *)CMAP(_new).mappingTable)[CMAP(_new).selected+CMAP(_new).firstIndex];

	XQueryColor(display, CORE(_new).colormap, &color);

	CMAP(_new).selectedColor = color;
	sprintf(CMAP(_new).label, RGB_STRING, color.red, color.green,
		color.blue, CMAP(_new).selected+1, CMAP(_new).usedColors);

	CMAP(_new).labelHeight = CMAP(_new).font->ascent + CMAP(_new).font->descent;
	CMAP(_new).labelWidth = XTextWidth(CMAP(_new).font, CMAP(_new).label, strlen(CMAP(_new).label));
 
	if (_request->core.width == 0)
	{
		Dimension maxLabel;
		int count;

		count = strlen(RGB_STRING) + 19;

		maxLabel = count * THE_FONT(_new)->max_bounds.width; /* Max bounded guess. */

		CORE(_new).width = MAX ((CMAP(_new).boxWidth ? CMAP(_new).boxWidth : CMAP_MINBOXWIDTH(_new)) * CMAP(_new).numX, maxLabel)
			+ 2 * CMAP(_new).margin;

#if MOTIF
		CORE(_new).width += PRIM(_new).highlight_thickness * 2;
#endif /* MOTIF */
	}

	if (_request->core.height == 0)
	{
		CORE(_new).height = (CMAP(_new).boxHeight ? CMAP(_new).boxHeight : CMAP_MINBOXHEIGHT(_new)) * CMAP(_new).numY + CMAP(_new).margin 
			+ CMAP(_new).labelHeight + 2 * CMAP(_new).margin;
#if MOTIF
		CORE(_new).height += PRIM(_new).highlight_thickness * 2;
#endif /* MOTIF */
	}

	CMAP(_new).halfMargin = CMAP(_new).margin/2;

	CMAP(_new).wasHighlighted = FALSE;

#if MOTIF
	if (HT(_new) > 0)
		HRECT(_new) = (XRectangle *)XtMalloc(sizeof(XRectangle) * HT(_new));
#endif /* MOTIF */

	resize(_new);
}

static void realize(_cw, _xvm, _xswa)
CmapWidget _cw; 
XtValueMask *_xvm;
XSetWindowAttributes *_xswa;
{
	XGCValues gcValues;
	XRectangle xrect;

/* Call the Realize procedure (XtInheritRealize) */
	(*cmapWidgetClass->core_class.superclass->core_class.realize)((Widget)_cw, _xvm, _xswa);

/* Now we can create our GC's */

	gcValues.foreground = CMAP(_cw).selected;
	CMAP(_cw).gc = XCreateGC(XtDisplay(_cw), XtWindow(_cw), GCForeground, &gcValues);

/* This one is read only */

	gcValues.foreground = FG(_cw); 
	gcValues.font = CMAP(_cw).font->fid;
	CMAP(_cw).foregroundGC = XtGetGC((Widget)_cw, GCFont | GCForeground, &gcValues);

/* This one is read only */

	gcValues.foreground = _cw->core.background_pixel;
	CMAP(_cw).eraseGC = XtGetGC((Widget)_cw, GCFont | GCForeground, &gcValues);

#if MOTIF
	xrect.x = xrect.y = PRIM(_cw).highlight_thickness;
	xrect.width = CORE(_cw).width - 2 * PRIM(_cw).highlight_thickness;
	xrect.height = CORE(_cw).height - 2 * PRIM(_cw).highlight_thickness;
	XSetClipRectangles(XtDisplay(_cw), CMAP(_cw).gc, 0, 0, &xrect, 1, Unsorted);
	XSetClipRectangles(XtDisplay(_cw), CMAP(_cw).foregroundGC, 0, 0, &xrect, 1, Unsorted);
	XSetClipRectangles(XtDisplay(_cw), CMAP(_cw).eraseGC, 0, 0, &xrect, 1, Unsorted);
#endif /* MOTIF */

}

static void resize(_cw)
CmapWidget _cw;
{
	int marginX, marginY;
	XRectangle xrect;

	marginX = 2 * CMAP(_cw).margin;
	marginY = 2 * CMAP(_cw).margin;

	CMAP(_cw).realBoxWidth = (TWIDTH(_cw) - marginX)/CMAP(_cw).numX;
	CMAP(_cw).realBoxHeight = (THEIGHT(_cw) - marginY - (CMAP(_cw).labelHeight + CMAP(_cw).margin)) /
		CMAP(_cw).numY;

	if (CMAP(_cw).boxWidth)
		if (CMAP(_cw).realBoxWidth > CMAP(_cw).boxWidth)
			 CMAP(_cw).realBoxWidth = CMAP(_cw).boxWidth;

	if (CMAP(_cw).boxHeight)
		if (CMAP(_cw).realBoxHeight > CMAP(_cw).boxHeight)
		 CMAP(_cw).realBoxHeight = CMAP(_cw).boxHeight; 

	marginX += (TWIDTH(_cw) - marginX) - (CMAP(_cw).numX * CMAP(_cw).realBoxWidth);
	marginY += (THEIGHT(_cw) - marginY) - (CMAP(_cw).numY * CMAP(_cw).realBoxHeight + CMAP(_cw).margin + CMAP(_cw).labelHeight);

	CMAP(_cw).drawPos.y = marginY / 2 + TX(_cw);
	CMAP(_cw).drawPos.x = marginX / 2 + TY(_cw);

	CMAP(_cw).labelPos.x = (TWIDTH(_cw)/2) - (CMAP(_cw).labelWidth)/2 + TX(_cw);
	CMAP(_cw).labelPos.y = CMAP(_cw).drawPos.y + (CMAP(_cw).numY * CMAP(_cw).realBoxHeight) + CMAP(_cw).font->ascent 
		+ CMAP(_cw).margin;

#if MOTIF
	if (XtIsRealized(_cw))
	{
		xrect.x = xrect.y = PRIM(_cw).highlight_thickness;
		xrect.width = CORE(_cw).width - 2 * PRIM(_cw).highlight_thickness;
		xrect.height = CORE(_cw).height - 2 * PRIM(_cw).highlight_thickness;
		XSetClipRectangles(XtDisplay(_cw), CMAP(_cw).gc, 0, 0, &xrect, 1, Unsorted);
		XSetClipRectangles(XtDisplay(_cw), CMAP(_cw).foregroundGC, 0, 0, &xrect, 1, Unsorted);
		XSetClipRectangles(XtDisplay(_cw), CMAP(_cw).eraseGC, 0, 0, &xrect, 1, Unsorted);
	}
#endif /* MOTIF */
}

static void destroy(_cw)
CmapWidget _cw;
{
	XFreeGC(XtDisplay(_cw), CMAP(_cw).gc);
	XtReleaseGC((Widget)_cw, CMAP(_cw).foregroundGC);
	XtReleaseGC((Widget)_cw, CMAP(_cw).eraseGC);
	XFreeFont(XtDisplay(_cw),CMAP(_cw).font);
	XtRemoveAllCallbacks((Widget)_cw, XtNchangeCallback);
#if MOTIF
	if (HT(_cw) > 0)
		XtFree((char *)HRECT(_cw));
	XmFontListFree(FONT_LIST(_cw));
#endif /* MOTIF */
}

static void getFont(_cw)
CmapWidget _cw;
{
#if MOTIF
	if (FONT_LIST(_cw) == NULL)
	{
		XmFontList defaultFont;

		defaultFont = _XmGetDefaultFontList((Widget)_cw, 0);
		FONT_LIST(_cw) = XmFontListCopy(defaultFont);
	}
	else
		FONT_LIST(_cw) = XmFontListCopy(FONT_LIST(_cw));

	if (FONT_LIST(_cw) == (XmFontList)NULL)
	{
		THE_FONT(_cw) = XLoadQueryFont(XtDisplay(_cw), "fixed");
		if (THE_FONT(_cw) == (XFontStruct *)NULL)
			THE_FONT(_cw) = XLoadQueryFont(XtDisplay(_cw), "*");
	 }
	 else
		 _XmFontListGetDefaultFont(FONT_LIST(_cw), &THE_FONT(_cw));
#else
	if (THE_FONT(_cw) == (XFontStruct *)NULL)
	{
		if ((THE_FONT(_cw) = XLoadQueryFont(XtDisplay(_cw), "fixed")) == NULL)
			if ((THE_FONT(_cw) = XLoadQueryFont(XtDisplay(_cw), "*")) == NULL)
				myXtWarning("CmapWidget: Fonts %s and %s not found.", "fixed", "*");
	}
#endif /* MOTIF */
}


static void drawLabel(_cw, _gc)
CmapWidget _cw; 
GC _gc;
{
	XDrawString(XtDisplay(_cw), XtWindow(_cw), _gc,
		CMAP(_cw).labelPos.x, CMAP(_cw).labelPos.y,
		CMAP(_cw).label, strlen(CMAP(_cw).label));
}

static void drawSelectedHighlight(_cw, _on)
CmapWidget _cw;
Boolean _on;
{
	int x, y;
	XRectangle rect[2];
	GC gc;
	Dimension hm;
	int numRects;
	
	hm = CMAP(_cw).halfMargin;

	x = BOXtoX(_cw, CMAP(_cw).selected);
	y = BOXtoY(_cw, CMAP(_cw).selected);

#if !MOTIF
	rect[0].x = x + hm;
	rect[0].y = y + hm;
	rect[0].width = (unsigned short)MAX(0, CMAP(_cw).realBoxWidth - (2 + hm - 1));
	rect[0].height = (unsigned short)MAX(0, CMAP(_cw).realBoxHeight - (2 + hm - 1));

	numRects = 1;

	rect[1].x = x + hm + 2;
	rect[1].y = y + hm + 2;
	rect[1].width = (unsigned short)MAX(0, CMAP(_cw).realBoxWidth - (6 + hm - 1));
	rect[1].height = (unsigned short)MAX(0, CMAP(_cw).realBoxHeight - (6 + hm - 1));

	numRects = 2;

	gc = CMAP(_cw).foregroundGC;

	XDrawRectangles(XtDisplay(_cw), XtWindow(_cw), gc,
		rect, numRects);

	XDrawRectangle(XtDisplay(_cw), XtWindow(_cw), CMAP(_cw).eraseGC,
		x + hm + 1, y + hm + 1,
		MAX(0, CMAP(_cw).realBoxWidth - (4 + hm - 1)),
		MAX(0, CMAP(_cw).realBoxHeight - (4 + hm - 1)));
#else

	if (_on)
		gc = PRIM(_cw).highlight_GC;
	else
		gc = CMAP(_cw).eraseGC;

	if (HT(_cw) > 0)
	{
		int i;
		x += hm;
		y += hm;
		for(i=0;i<HT(_cw);i++)
		{
			HRECT(_cw)[i].x = x;
			HRECT(_cw)[i].y = y;
			HRECT(_cw)[i].width = (unsigned short)MAX(0, CMAP(_cw).realBoxWidth - (2 * i + hm + 1));
			HRECT(_cw)[i].height = (unsigned short)MAX(0, CMAP(_cw).realBoxHeight - (2 * i + hm + 1));
			x++;
			y++;
		}

		XDrawRectangles(XtDisplay(_cw), XtWindow(_cw), gc, HRECT(_cw), i);
	}
#endif /* !MOTIF */
}

static void drawSelected(_cw, _boxNumber)
CmapWidget _cw;
int _boxNumber;
{
	int x, y;
	x = BOXtoX(_cw, _boxNumber);
	y = BOXtoY(_cw, _boxNumber);

#if MOTIF
#if (XmREVISION == 2)
	 _XmDrawShadows (XtDisplay(_cw), XtWindow(_cw),
		PRIM(_cw).top_shadow_GC,
		PRIM(_cw).bottom_shadow_GC,
		x + HT(_cw), y + HT(_cw),
		CMAP(_cw).realBoxWidth - (2 * HT(_cw)), CMAP(_cw).realBoxHeight - (2 * HT(_cw)),
		ST(_cw), (CMAP(_cw).invertedShadows ? XmSHADOW_IN : XmSHADOW_OUT));
#else
	_XmDrawShadow (XtDisplay(_cw), XtWindow(_cw),
		(CMAP(_cw).invertedShadows ? PRIM(_cw).bottom_shadow_GC : PRIM(_cw).top_shadow_GC),
		(CMAP(_cw).invertedShadows ? PRIM(_cw).top_shadow_GC : PRIM(_cw).bottom_shadow_GC),
		ST(_cw), 
		x + HT(_cw), y + HT(_cw),
		CMAP(_cw).realBoxWidth - (2 * ST(_cw)), CMAP(_cw).realBoxHeight - (2 * ST(_cw)));
#endif /* XmREVISION */
#endif /* MOTIF */

	drawSelectedHighlight(_cw, CMAP(_cw).wasHighlighted);
}

static void drawBox(_cw, _boxNumber)
CmapWidget _cw;
int _boxNumber;
{
	int x, y;

	if (CMAP(_cw).mappingTable == (Pixel *)NULL)
		XSetForeground(XtDisplay(_cw), CMAP(_cw).gc, _boxNumber+CMAP(_cw).firstIndex);
	else
		XSetForeground(XtDisplay(_cw), CMAP(_cw).gc, ((Pixel *)CMAP(_cw).mappingTable)[_boxNumber+CMAP(_cw).firstIndex]);

	x = BOXtoX(_cw, _boxNumber);
	y = BOXtoY(_cw, _boxNumber);

	XClearArea(XtDisplay(_cw), XtWindow(_cw), x, y, CMAP(_cw).realBoxWidth, CMAP(_cw).realBoxHeight, FALSE);

	XFillRectangle(XtDisplay(_cw), XtWindow(_cw), CMAP(_cw).gc,
		x + CMAP(_cw).halfMargin, y + CMAP(_cw).halfMargin,
		CMAP(_cw).realBoxWidth - CMAP(_cw).halfMargin, CMAP(_cw).realBoxHeight - CMAP(_cw).halfMargin);

#if MOTIF
	if (CMAP(_cw).showUnselectedShadows)
#if (XmREVISION == 2 )
	 	_XmDrawShadows (XtDisplay(_cw), XtWindow(_cw),
				PRIM(_cw).top_shadow_GC,
				PRIM(_cw).bottom_shadow_GC,
				x + CMAP(_cw).halfMargin,
				y + CMAP(_cw).halfMargin,
				CMAP(_cw).realBoxWidth - CMAP(_cw).halfMargin,
				CMAP(_cw).realBoxHeight - CMAP(_cw).halfMargin,
				ST(_cw), (CMAP(_cw).invertedShadows ? XmSHADOW_OUT : XmSHADOW_IN));
#else
		_XmDrawShadow(XtDisplay(_cw), XtWindow(_cw),
			(CMAP(_cw).invertedShadows ? PRIM(_cw).top_shadow_GC : PRIM(_cw).bottom_shadow_GC),
			(CMAP(_cw).invertedShadows ? PRIM(_cw).bottom_shadow_GC: PRIM(_cw).top_shadow_GC),
			ST(_cw),
			x + CMAP(_cw).halfMargin,
			y + CMAP(_cw).halfMargin,
			CMAP(_cw).realBoxWidth - CMAP(_cw).halfMargin,
			CMAP(_cw).realBoxHeight - CMAP(_cw).halfMargin);
#endif /* XmREVISION */
#endif /* MOTIF */
}
 
static void redisplay(_cw, _event, _region)
CmapWidget _cw; 
XEvent *_event; 
Region _region;
{
	int i,j;
	int scol, ecol;
	int srow, erow;
	XRectangle rect;

	XClipBox(_region, &rect);
	if (_cw->core.visible)
	{
		XFillRectangle(XtDisplay(_cw), XtWindow(_cw), CMAP(_cw).eraseGC, rect.x, rect.y,
			rect.width, rect.height);

		rect.x = MAX(rect.x, CMAP(_cw).drawPos.x);
		rect.y = MAX(rect.y, CMAP(_cw).drawPos.y);

		rect.width = MIN(rect.x+rect.width, CMAP(_cw).drawPos.x+CMAP(_cw).numX*CMAP(_cw).realBoxWidth-1);
		rect.height = MIN(rect.y+rect.height, CMAP(_cw).drawPos.y+CMAP(_cw).numY*CMAP(_cw).realBoxHeight-1);

		if(!( (rect.x >= rect.width) || (rect.y >= rect.height)))
		{
			scol = COL(_cw, xYtoBox(_cw, rect.x, rect.y));
			ecol = COL(_cw, xYtoBox(_cw, rect.width, rect.y));
			srow = ROW(_cw, xYtoBox(_cw, rect.x, rect.y));
			erow = ROW(_cw, xYtoBox(_cw, rect.x, rect.height));
			i = scol;
			j = srow;
			for(;i<=ecol; i++)
			{
				for(;(j<=erow) && ((i+CMAP(_cw).numX*j)<CMAP(_cw).usedColors);j++)
					drawBox(_cw, i + j*CMAP(_cw).numX); 
				j = srow;
			}
		}
		drawLabel(_cw, CMAP(_cw).eraseGC);
		drawLabel(_cw, CMAP(_cw).foregroundGC);
		drawSelected(_cw, CMAP(_cw).selected);
	}
	XFlush(XtDisplay(_cw));

	if (cmapWidgetClass->core_class.superclass->core_class.expose != NULL)
		(*cmapWidgetClass->core_class.superclass->core_class.expose)((Widget)_cw, _event, _region);

#if MOTIF
	if (CMAP(_cw).wasHighlighted)
		borderHighlight(_cw);
	else
		borderUnhighlight(_cw);
#endif /* MOTIF */
}

#if MOTIF

static void borderHighlight(_cw)
CmapWidget _cw;
{
	drawSelectedHighlight(_cw, TRUE);
	(*((XmPrimitiveWidgetClass)cmapWidgetClass->core_class.superclass)->primitive_class.border_highlight)((Widget)_cw);

	CMAP(_cw).wasHighlighted = TRUE;
}

static void borderUnhighlight(_cw)
CmapWidget _cw;
{
	drawSelectedHighlight(_cw, FALSE);
	(*((XmPrimitiveWidgetClass)cmapWidgetClass->core_class.superclass)->primitive_class.border_unhighlight)((Widget)_cw);

	CMAP(_cw).wasHighlighted = FALSE;
}

#endif /* MOTIF */

static Boolean setValues(_current, _request, _new)
CmapWidget _current; 
CmapWidget _request; 
CmapWidget _new;
{
	Boolean redisplay = FALSE;
	Boolean newErase = FALSE;
	Boolean newSelected = FALSE;
	Boolean needResize = FALSE;
	Display *display = XtDisplay(_new);
	XGCValues gcValues;
	int gcMask = GCFont;

#if MOTIF 
	if (FONT_LIST(_new) != FONT_LIST(_current))
	{
		XmFontListFree(FONT_LIST(_current));
		if (FONT_LIST(_new) == NULL)
		{
			XmFontList defaultFont;

			defaultFont = _XmGetDefaultFontList((Widget)_new, 0);
			FONT_LIST(_new) = XmFontListCopy(defaultFont);
		}
		else
			FONT_LIST(_new) = XmFontListCopy(FONT_LIST(_new));

		if (FONT_LIST(_new) == (XmFontList)NULL)
		{
			THE_FONT(_new) = XLoadQueryFont(XtDisplay(_new), "fixed");
			if (THE_FONT(_new) == (XFontStruct *)NULL)
				THE_FONT(_new) = XLoadQueryFont(XtDisplay(_new), "*");
		}
		else
			_XmFontListGetDefaultFont(FONT_LIST(_new), &THE_FONT(_new));
	}

#endif /* MOTIF */

	if (THE_FONT(_new) != THE_FONT(_current))
	{
		XFreeFont(XtDisplay(_current), THE_FONT(_current));

		if ((CMAP(_new).font = XLoadQueryFont(display, "fixed")) == NULL)
			if ((CMAP(_new).font = XLoadQueryFont(display, "*")) == NULL)
				myXtWarning("CmapWidget: Fonts %s and %s not found.", "fixed", "*");

        XSetFont(display, CMAP(_new).foregroundGC, CMAP(_new).font->fid);
        newSelected = TRUE;
        newErase = TRUE;
        redisplay = TRUE;
	}

	if (_new->core.background_pixel != _current->core.background_pixel)
	{
		gcValues.foreground = gcValues.background = _new->core.background_pixel;
		gcMask |= (GCForeground | GCBackground);
		newErase = TRUE;
		redisplay = TRUE;
	}

	if (FG(_new) != FG(_current))
		newSelected = TRUE;

	if (CMAP(_new).firstIndex != _current->cmap.firstIndex)
	{
		if (CMAP(_new).firstIndex < 0)
		{
			myXtWarning("CmapWidget: FirstIndex (%d) cannot be negative. FirstIndex set to 0.", 
				CMAP(_new).firstIndex);
			CMAP(_new).firstIndex = 0;
		}
		if (CMAP(_new).firstIndex > CMAP(_new).availColors-1)
		{
			myXtWarning("CmapWidget: FirstIndex (%d) cannot be more than greatest index (%d).	FirstIndex set to %d.",
				CMAP(_new).firstIndex, CMAP(_new).availColors-1, CMAP(_new).availColors-1);
			CMAP(_new).firstIndex = CMAP(_new).availColors-1;
		}
		needResize = TRUE;
	}

	if (CMAP(_new).lastIndex != _current->cmap.lastIndex)
	{
		if (CMAP(_new).lastIndex < CMAP(_new).firstIndex)
		{
			myXtWarning("CmapWidget: LastIndex (%d) cannot be less than FirstIndex (%d).	LastIndex set to %d.",
				CMAP(_new).lastIndex, CMAP(_new).firstIndex, CMAP(_new).firstIndex+1);
			CMAP(_new).lastIndex = CMAP(_new).firstIndex + 1;
		}

		if (CMAP(_new).lastIndex > CMAP(_new).availColors-1)
		{
			myXtWarning("CmapWidget: LastIndex (%d) cannot be more than greatest index (%d).	LastIndex set to %d.",
				CMAP(_new).lastIndex, CMAP(_new).availColors-1, CMAP(_new).availColors-1);
			CMAP(_new).lastIndex = CMAP(_new).availColors-1;
		}
		needResize = TRUE;
	}

	if (needResize)
	{
		if (CMAP(_new).mappingTable == (Pixel *)NULL)
			CMAP(_new).usedColors = MIN(CMAP(_new).lastIndex-CMAP(_new).firstIndex+1, CMAP(_new).availColors);
		else
			CMAP(_new).usedColors = CMAP(_new).lastIndex-CMAP(_new).firstIndex+1;

		CMAP(_new).numX = (int)floor(sqrt((double)(CMAP(_new).usedColors)));
		CMAP(_new).numY = CMAP(_new).usedColors / CMAP(_new).numX + (CMAP(_new).usedColors % CMAP(_new).numX ? 1 : 0);
		redisplay = TRUE;
	}


	if (CMAP(_new).selected != _current->cmap.selected)
	{
		if (CMAP(_new).selected < 0)
		{
			myXtWarning("CmapWidget: Selected pixel cannot be negative! Selected set to 0.");
			CMAP(_new).selected = 0;
		}

		if (CMAP(_new).selected > CMAP(_new).usedColors-1)
		{
			myXtWarning("CmapWidget: Selected pixel (%d) is greater than %d. Selected set to %d.",
				CMAP(_new).selected, CMAP(_new).usedColors-1, CMAP(_new).usedColors-1);
			CMAP(_new).selected = CMAP(_new).lastIndex;
		}

		if (CMAP(_new).selected < 0)
		{
			myXtWarning("CmapWidget: Selected pixel (%d) cannot be negative. Selected set to 0.",
				CMAP(_new).selected);
			CMAP(_new).selected = 0;
		}

		drawLabel(_current, CMAP(_current).eraseGC);

		if (CMAP(_new).mappingTable == (Pixel *)NULL)
			CMAP(_new).selectedColor.pixel = CMAP(_new).selected+CMAP(_new).firstIndex;
		else
			CMAP(_new).selectedColor.pixel = ((Pixel *)CMAP(_new).mappingTable)[CMAP(_new).selected+CMAP(_new).firstIndex];

		XQueryColor(display, _new->core.colormap, &CMAP(_new).selectedColor);

		reString(_new);
		drawLabel(_new, CMAP(_new).foregroundGC);
		drawBox(_new, CMAP(_current).selected);
		drawSelected(_new, CMAP(_new).selected);
		XFlush(XtDisplay(_new));
	}

	if (CMAP(_new).mappingTable != CMAP(_current).mappingTable)
		redisplay = TRUE;

	if (CMAP(_new).margin != CMAP(_current).margin)
	{
		CMAP(_new).halfMargin = CMAP(_new).margin/2;
		redisplay = TRUE;
	}
		
	gcValues.font = THE_FONT(_new)->fid; 

	if (newErase)
	{
		XtReleaseGC((Widget)_new, CMAP(_new).eraseGC);
		_new->cmap.eraseGC = XtGetGC((Widget)_new, gcMask, &gcValues);
	}

	if (newSelected)
	{
		XtReleaseGC((Widget)_new, CMAP(_new).foregroundGC);
		gcValues.foreground = FG(_new);
		_new->cmap.foregroundGC = XtGetGC((Widget)_new, GCFont | GCForeground, &gcValues);
	}

#if MOTIF
	if (HT(_new) != HT(_current))
	{
		if (HT(_current) > 0)
			XtFree((char *)HRECT(_current));

		if (HT(_new) > 0)
			HRECT(_new) = (XRectangle *)XtMalloc(sizeof(XRectangle) * HT(_new));

		redisplay = TRUE;
	}
#endif /* MOTIF */

	if ((CMAP(_new).boxHeight != _current->cmap.boxHeight) ||
			(CMAP(_new).boxWidth != _current->cmap.boxWidth))
		redisplay = TRUE;

	if (redisplay)
		reString(_new);

	return redisplay;
}

static void reString(_cw)
CmapWidget _cw;
{
	sprintf(CMAP(_cw).label, RGB_STRING, CMAP(_cw).selectedColor.red, CMAP(_cw).selectedColor.green,
		CMAP(_cw).selectedColor.blue, CMAP(_cw).selected+1, CMAP(_cw).usedColors);
	CMAP(_cw).labelHeight = CMAP(_cw).font->ascent + CMAP(_cw).font->descent;
	CMAP(_cw).labelWidth = XTextWidth(CMAP(_cw).font, CMAP(_cw).label, strlen(CMAP(_cw).label));
	CMAP(_cw).labelPos.x = (TWIDTH(_cw)/2) - (CMAP(_cw).labelWidth)/2 + TX(_cw);
	CMAP(_cw).labelPos.y = CMAP(_cw).drawPos.y + (CMAP(_cw).numY * CMAP(_cw).realBoxHeight) + CMAP(_cw).font->ascent +
		CMAP(_cw).margin;
}

static int xYtoBox(_cw, _x, _y)
CmapWidget _cw; 
int _x; 
int _y;
{
	int trux, truy;
	int twidth, theight;
	int x, y;

	trux = _x - CMAP(_cw).drawPos.x;
	truy = _y - CMAP(_cw).drawPos.y;
	twidth = CMAP(_cw).numX * CMAP(_cw).realBoxWidth;
	theight = CMAP(_cw).numY * CMAP(_cw).realBoxHeight; 

	if ((trux < 0) || (truy < 0) || (trux >= twidth) || (truy >= theight))
		return -1;

	return trux/CMAP(_cw).realBoxWidth + truy/CMAP(_cw).realBoxHeight * CMAP(_cw).numX;
}

static void left(_cw, _event, _argv, _argc)
CmapWidget _cw; 
XEvent *_event; 
String *_argv; 
int *_argc;
{
	int box;
	Arg warg;
	CmapCallbackStruct ccs;

	if (*_argc != 0)
		myXtWarning("CmapWidget: Left takes no paramaters.");

	box = ((CMAP(_cw).selected - 1) < 0) ? CMAP(_cw).usedColors - 1 : CMAP(_cw).selected - 1;
	XtSetArg(warg, XtNselected, box);
	XtSetValues((Widget)_cw, &warg, 1);
	ccs.reason = CMAP_LEFT;
	ccs.event = _event;
	ccs.color = CMAP(_cw).selectedColor;
	XtCallCallbacks((Widget)_cw, XtNchangeCallback, &ccs);
}

static void right(_cw, _event, _argv, _argc)
CmapWidget _cw; 
XEvent *_event;
String *_argv; 
int *_argc;
{
	int box;
	Arg warg;
	CmapCallbackStruct ccs;

	if (*_argc != 0)
		myXtWarning("CmapWidget: Right takes no paramaters.");

	box = ((CMAP(_cw).selected + 1) == CMAP(_cw).usedColors ) ? 0	: CMAP(_cw).selected + 1;
	XtSetArg(warg, XtNselected, box);
	XtSetValues((Widget)_cw, &warg, 1);
	ccs.reason = CMAP_RIGHT;
	ccs.event = _event;
	ccs.color = CMAP(_cw).selectedColor;
	XtCallCallbacks((Widget)_cw, XtNchangeCallback, &ccs);
}

static void up(_cw, _event, _argv, _argc)
CmapWidget _cw; 
XEvent *_event;
String *_argv;
int *_argc;
{
	int box;
	Arg warg;
	CmapCallbackStruct ccs;

	if (*_argc != 0)
		myXtWarning("CmapWidget: Up takes no paramaters.");

	box = CMAP(_cw).selected - CMAP(_cw).numX;

	if (box < 0)
	{
		box += CMAP(_cw).numX * (CMAP(_cw).numY+1) + 1;
		while (box > CMAP(_cw).usedColors-1)
			box -= CMAP(_cw).numX;
	}

	XtSetArg(warg, XtNselected, box);
	XtSetValues((Widget)_cw, &warg, 1);
	ccs.reason = CMAP_UP;
	ccs.event = _event;
	ccs.color = CMAP(_cw).selectedColor;
	XtCallCallbacks((Widget)_cw, XtNchangeCallback, &ccs);
}

static void down(_cw, _event, _argv, _argc)
CmapWidget _cw;
XEvent *_event;
String *_argv;
int *_argc;
{
	int box;
	Arg warg;
	CmapCallbackStruct ccs;

	if (*_argc != 0)
		myXtWarning("CmapWidget: Down takes no paramaters.");

	box = CMAP(_cw).selected + CMAP(_cw).numX;

	if (box >= CMAP(_cw).usedColors)
	{
		box -= CMAP(_cw).numX * (CMAP(_cw).numY+1) + 1;
		while (box < 0)
			box += CMAP(_cw).numX;
	} 

	XtSetArg(warg, XtNselected, box);
	XtSetValues((Widget)_cw, &warg, 1);
	ccs.reason = CMAP_DOWN;
	ccs.event = _event;
	ccs.color = CMAP(_cw).selectedColor;
	XtCallCallbacks((Widget)_cw, XtNchangeCallback, &ccs);
}

static void wselect(_cw, _event, _argv, _argc)
CmapWidget _cw;
XEvent *_event;
String *_argv;
int *_argc;
{
	int box;
	int x, y;
	Arg warg;
	CmapCallbackStruct ccs;

	switch (_event->type)
	{
		case ButtonPress:
		case ButtonRelease:
			x = ((XButtonEvent *)_event)->x;
			y = ((XButtonEvent *)_event)->y;
			break;
		case KeyPress:
		case KeyRelease:
			x = ((XKeyEvent *)_event)->x;
			y = ((XKeyEvent *)_event)->y;
			break;
		case MotionNotify:
			x = ((XMotionEvent *)_event)->x;
			y = ((XMotionEvent *)_event)->y;
			break;
		default: return; break;
	}

	if ((box = xYtoBox(_cw, x, y)) < 0)
		return;

#if MOTIF
	if (XmProcessTraversal((Widget)_cw, XmTRAVERSE_CURRENT) == FALSE)
		XmProcessTraversal((Widget)_cw, XmTRAVERSE_CURRENT);
#else
	XtSetKeyboardFocus((Widget)_cw, (Widget)_cw);
#endif /* MOTIF */

	if (box == CMAP(_cw).selected)
		return;

	if (box > CMAP(_cw).usedColors-1)
		return;

	XtSetArg(warg, XtNselected, box);
	XtSetValues((Widget)_cw, &warg, 1);
	ccs.reason = CMAP_SELECT;
	ccs.event = _event;
	ccs.color = CMAP(_cw).selectedColor;
	XtCallCallbacks((Widget)_cw, XtNchangeCallback, &ccs);
}

#define MAXSTRING 300

static void myXtWarning(_format, va_alist)
char *_format; 
va_dcl	 /* stupid define already has a ; on it */
{
	va_list parms;
	char dest[MAXSTRING];

	va_start(parms);
	vsprintf(dest, _format, parms);
	va_end(parms);

	XtWarning(dest);
}

/* External routines */

/* If you have
** an application that plays with the colormap, you 
** must verify that the label is correct for that 
** color (ie, the r,g,b values are correct.
** * NOTE * The event is NULL in the callback.
*/

void CmapVerifySelectedColor(_cw, _notify)
Widget _cw;
Boolean _notify;
{
	XColor color;
	CmapWidget cw;

	if (!(XtIsCmap(_cw)))
	{
		myXtWarning("Widget passed to CmapVerifySelectedColor is not a Cmap widget!");
		return;
	}

	cw = (CmapWidget)_cw;
	color.pixel = CMAP(cw).selected;

	if (CMAP(cw).mappingTable == (Pixel *)NULL)
		color.pixel = CMAP(cw).selected+CMAP(cw).firstIndex;
	else
		color.pixel = ((Pixel *)CMAP(cw).mappingTable)[CMAP(cw).selected+CMAP(cw).firstIndex];

	XQueryColor(XtDisplay(cw), cw->core.colormap, &color);
	drawLabel(cw, CMAP(cw).eraseGC);
	CMAP(cw).selectedColor = color;
	reString(cw);
	drawLabel(cw, CMAP(cw).foregroundGC);

	if (_notify)
	{
		CmapCallbackStruct ccs;
		ccs.event = (XEvent *)NULL;
		ccs.color = color;
		ccs.reason = CMAP_VERIFY;
		XtCallCallbacks(_cw, XtNchangeCallback, &ccs);
	}
}

/* 
** Currently selected box in colormap.
** If the widget is not a Cmap widget, the returned value is undefined 
*/

XColor CmapGetSelected(_cw)
Widget _cw;
{
	if (!XtIsCmap(_cw))
	{
		XColor xc;
		myXtWarning("Widget passed to CmapGetSelected is not a Cmap widget!");
		return xc;
	}
	return ((CmapWidget)_cw)->cmap.selectedColor;
}
