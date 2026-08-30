
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




#include "MdialP.h"
#include "math.h"

#if DRAG_AND_DROP
#include <Xm/DragC.h>
#include <Xm/DragIconP.h>
#include <Xm/AtomMgr.h>
#endif /* DRAG_AND_DROP */


#ifdef _NO_PROTO
static XtInitProc initialize();
static XtWidgetProc resize();
static Boolean setValues();
static XtExposeProc expose();
static void destroy();

static Boolean validateResources();

static void drawString();
static void drawCircle();
static void drawArm();
static void fillCircle();
static void drawDiamond();

static void copyPrimitiveGC();
static void getBackgroundPixel();
static void getDialFont();
static GC getGC();
static void redoLabel();
static void doExtents();
static void moveArmToCurrentValue();
static void setArmValue();

static void doCallback();


static int rangleToValue();
static double valueToRangle();

static Boolean goodDistance();
static double getRangle();
static void doMoveArm();

static void destroy();
static void set();
static void increment();
static void decrement();
static void drag();

#if DRAG_AND_DROP
static void dragWatch();
static Boolean dragConvertProc();
#endif


#else

static XtInitProc initialize(MdialWidget _request, MdialWidget _new, String *_args, Cardinal *_numArgs);
static XtWidgetProc resize(MdialWidget _mdw);
static Boolean setValues(MdialWidget _current, MdialWidget _request, MdialWidget _new, ArgList _args, Cardinal *_numArgs);
static XtExposeProc expose(MdialWidget _mdw, XEvent *_event, Region _region);
static void destroy(MdialWidget _mdw);

static Boolean validateResources(MdialWidget _current, MdialWidget _request, MdialWidget _new, Boolean _fromInit);

static void drawString(MdialWidget _mdw, GC _gc, Position _x, Position _y, Dimension _width, XmString _xmstring);
static void drawCircle(MdialWidget _mdw, GC _top, GC _bottom, Position _x, Position _y, Dimension _width, Dimension _height, Boolean _full, Boolean _underline);
static void drawArm(MdialWidget _mdw, GC _gc, Boolean _fancy);
static void fillCircle(MdialWidget _mdw, GC _gc, Position _x, Position _y, Dimension _width, Dimension _height, Boolean _full);
static void drawDiamond(MdialWidget _mdw, GC _topGC, GC _bottomGC, GC _fillGC);

static void copyPrimitiveGC(MdialWidget _mdw, GC *_toGC, GC _fromGC);
static void getBackgroundPixel(MdialWidget _mdw, int _offset, XrmValue *_value);
static GC getGC(MdialWidget _mdw, Pixel *_fg, Pixel *_bg);
static void getDialFont(MdialWidget _mdw);
static void doExtents(MdialWidget _mdw);
static void redoLabel(MdialWidget _mdw);
static void moveArmToCurrentValue(MdialWidget _mdw);
static void setArmValue(MdialWidget _mdw, int _x, int _y);
static int rangleToValue(MdialWidget _mdw, double _rangle);
static double valueToRangle(MdialWidget _mdw, int _value);

static void doCallback(MdialWidget _mdw, int _reason, XEvent *_event);

static Boolean goodDistance(MdialWidget _mdw, int _x, int _y);
static double getRangle(MdialWidget _mdw, int _x, int _y);
static void doMoveArm(MdialWidget _mdw, int _value, double _rangle);

static void set(Widget _mdw, XEvent *_event, String *_parms, Cardinal *_numParms);
static void increment(Widget _mdw, XEvent *_event, String *_parms, Cardinal *_numParms);
static void decrement(Widget _mdw, XEvent *_event, String *_parms, Cardinal *_numParms);
static void drag(Widget _mdw, XEvent *_event, String *_parms, Cardinal *_numParms);

#if DRAG_AND_DROP
static void dragWatch(MdialWidget _mdw, XtPointer _data, XEvent *_event, Boolean *_cont);
static Boolean dragConvertProc(Widget _w, Atom *_selection, Atom *_target, Atom *_toType, XtPointer *_toValue, unsigned long *_toLength,
  int *_toFormat, unsigned long *_toMaxLength, XtPointer _data, XtRequestId *_id);
#endif




#endif /* _NO_PROTO */

#define PI 3.14159264
#define RADIANS(a) (PI/180.0 * (double)(a))
#define DEGREES(a) (180.0 / PI * (a))
#define THETA 5.0

#define CORE(a) ((a)->core)
#define PRIM(a) ((a)->primitive)
#define MDIAL(a) ((a)->mdial)

#define WIDTH(a) ((a)->core.width)
#define HEIGHT(a) ((a)->core.height)

#define TOP_GC(a) ((a)->primitive.top_shadow_GC)
#define BOTTOM_GC(a) ((a)->primitive.bottom_shadow_GC)
#define H_THICK(a) ((a)->primitive.highlight_thickness)
#define SH_THICK(a) ((a)->primitive.shadow_thickness)

#define CONT_TOP_GC(a) ((a)->mdial.containerTopShadowGC)
#define CONT_BOTTOM_GC(a) ((a)->mdial.containerBottomShadowGC)
#define FGC(a) ((a)->mdial.dialForegroundGC)
#define BGC(a) ((a)->mdial.dialBackgroundGC)

#define DX(a) ((a)->mdial.dialX)
#define DY(a) ((a)->mdial.dialY)
#define DWIDTH(a) ((a)->mdial.dialWidth)
#define DHEIGHT(a) ((a)->mdial.dialHeight)

#define CX(a) ((a)->mdial.centerX)
#define CY(a) ((a)->mdial.centerY)

#define LX(a) ((a)->mdial.labelX)
#define LY(a) ((a)->mdial.labelY)
#define LWIDTH(a) ((a)->mdial.labelWidth)
#define LHEIGHT(a) ((a)->mdial.labelHeight)
#define LALIGN(a) ((a)->mdial.labelAlignment)

#define AX(a) ((a)->mdial.armX)
#define AY(a) ((a)->mdial.armY)

#define RO(a) ((a)->mdial.displayOnly)
#define LABEL(a) ((a)->mdial.label)
#define MARGIN_WIDTH(a) ((a)->mdial.marginWidth)
#define MARGIN_HEIGHT(a) ((a)->mdial.marginHeight)
#define FONT_LIST(a) ((a)->mdial.fontList)
#define FULL(a) ((a)->mdial.fullCircle)
#define SHAD_TYPE(a) ((a)->mdial.shadowType)
#define VALUE(a) ((a)->mdial.value)
#define VMIN(a) ((a)->mdial.minimum)
#define VMAX(a) ((a)->mdial.maximum)
#define ASPECT(a) ((a)->mdial.preserveAspect)
#define SPOS(a) ((a)->mdial.startPosition)
#define THE_FONT(a) ((a)->mdial.fontStruct)
#define RANGLE(a) ((a)->mdial.rangle)

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a) ((a) > 0 ? (a) : -(a))
#endif


#define MIN_DOT_RADIUS(w) (SH_THICK(w) * 2) 
#define DOT_RADIUS(w) (MIN(DHEIGHT(w), DWIDTH(w))/100 * 2 + MIN_DOT_RADIUS(w)) 

static char mdialTranslations[] =
"~s <Key>osfUp: increment()\n\
~s <Key>osfDown: decrement()\n\
~s <Key>osfLeft: decrement()\n\
~s <Key>osfRight: increment()\n\
Shift<Key>osfUp: decrement(10)\n\
Shift<Key>osfDown: increment(10)\n\
Shift<Key>osfLeft: decrement(10)\n\
Shift<Key>osfRight: increment(10)\n\
<Key>+: increment()\n\
<Key>-: decrement()\n\
<Btn1Down>: set()\n\
<Btn1Motion>: drag()\n\
<Btn1Up>: set() \n\
~s ~m ~a <Key>Return: PrimitiveParentActivate()\n\
<Key>osfActivate: PrimitiveParentActivate()\n\
s ~m ~a <Key>Tab: PrimitivePrevTabGroup()\n\
~m ~a <Key>Tab: PrimitiveNextTabGroup()\n\
<Key>osfCancel:PrimitiveParentCancel()\n\
<Key>osfHelp:PrimitiveHelp()\n\
<FocusIn>:PrimitiveFocusIn()\n\
<FocusOut>:PrimitiveFocusOut()\n\
<Unmap>:PrimitiveUnmap()\n\
<Enter>:PrimitiveEnter()\n\
<Leave>:PrimitiveLeave()";

#define TheOffset(field) XtOffset(MdialWidget, mdial.field)
static XtResource resources[] =
{
  {XmNmarginWidth, XmCMarginWidth, XtRDimension, sizeof(Dimension),
    TheOffset(marginWidth), XmRImmediate, (XtPointer)2},
  {XmNmarginHeight, XmCMarginHeight, XtRDimension, sizeof(Dimension),
    TheOffset(marginHeight), XmRImmediate, (XtPointer)2},
  {XmNdialBackground, XtCBackground, XtRPixel, sizeof(Pixel),
    TheOffset(dialBackground), XtRCallProc, (XtPointer)getBackgroundPixel},
  {XmNdialForeground, XtCForeground, XtRPixel, sizeof(Pixel),
    TheOffset(dialForeground), XmRImmediate, (XtPointer)XtDefaultForeground},
  {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
    TheOffset(fontList), XmRImmediate, (XtPointer) NULL },
  {XmNalignment, XmCAlignment, XmRAlignment, sizeof(unsigned char),
    TheOffset(labelAlignment), XmRImmediate, (XtPointer) XmALIGNMENT_BEGINNING},
  {XmNfullCircle, XmCFullCircle, XtRBoolean, sizeof(Boolean),
    TheOffset(fullCircle), XmRImmediate, (XtPointer)TRUE},
  {XmNshadowType, XmCShadowType, XmRShadowType, sizeof(unsigned char),
    TheOffset(shadowType), XmRImmediate, (XtPointer)XmSHADOW_IN},
  {XmNvalue, XmCValue, XtRInt, sizeof(int),
    TheOffset(value), XmRImmediate, (XtPointer)0},
  {XmNminimum, XmCMinimum, XtRInt, sizeof(int),
    TheOffset(minimum), XmRImmediate, (XtPointer)0},
  {XmNmaximum, XmCMaximum, XtRInt, sizeof(int),
    TheOffset(maximum), XmRImmediate, (XtPointer)100},
  {XmNdisplayOnly, XmCDisplayOnly, XtRBoolean, sizeof(Boolean),
    TheOffset(displayOnly), XmRImmediate, (XtPointer)FALSE},
  {XmNpreserveAspect, XmCPreserveAspect, XtRBoolean, sizeof(Boolean),
    TheOffset(preserveAspect), XmRImmediate, (XtPointer)FALSE},
  {XmNprocessingDirection, XmCProcessingDirection, XmRProcessingDirection, sizeof(unsigned char),
    TheOffset(processingDirection), XmRImmediate, (XtPointer)XmMAX_ON_TOP},
  {XmNlabelFormatter, XmCLabelFormatter, XtRFunction, sizeof(t_func),
    TheOffset(labelFormatter), XmRImmediate, (XtPointer)NULL},
  {XmNstringDirection, XmCStringDirection, XmRStringDirection, sizeof(unsigned char),
    TheOffset(stringDirection), XmRImmediate, (XtPointer)XmSTRING_DIRECTION_DEFAULT},
  {XmNincrementCallback, XmCCallback, XtRCallback, sizeof(XtCallbackList),
    TheOffset(incrementCallback), XmRImmediate, (XtPointer)NULL},
  {XmNdecrementCallback, XmCCallback, XtRCallback, sizeof(XtCallbackList),
    TheOffset(decrementCallback), XmRImmediate, (XtPointer)NULL},
  {XmNvalueChangedCallback, XmCCallback, XtRCallback, sizeof(XtCallbackList),
    TheOffset(valueChangedCallback), XmRImmediate, (XtPointer)NULL},
};

static XtActionsRec mdialActions[] =
{
  {"set", set},
  {"increment", increment},
  {"decrement", decrement},
  {"drag", drag},
};

static XmBaseClassExtRec mdialBaseClassExtRec =
{
  NULL,                                     /* Next extension       */
  NULLQUARK,                                /* record type XmQmotif */
  XmBaseClassExtVersion,                    /* version              */
  sizeof(XmBaseClassExtRec),                /* size                 */
  NULL,                                     /* initialize prehook   */
  XmInheritSetValuesPrehook,                /* set_values prehook   */
  NULL,                                     /* initialize posthook  */
  XmInheritSetValuesPosthook,               /* set_values posthook  */
  XmInheritClass,                           /* secondary class      */
  XmInheritSecObjectCreate,                 /* creation proc        */
  XmInheritGetSecResData,                   /* getSecResData */
  {
    0
  },                                      /* fast subclass        */
  XmInheritGetValuesPrehook,                /* get_values prehook   */
  XmInheritGetValuesPosthook,               /* get_values posthook  */
  (XtWidgetClassProc)NULL,                  /* classPartInitPrehook */
  (XtWidgetClassProc)NULL,                  /* classPartInitPosthook */
  NULL,                                     /* ext_resources        */
  NULL,                                     /* compiled_ext_resources */
  0,                                        /* num_ext_resources    */
  FALSE,                                    /* use_sub_resources    */
  XmInheritWidgetNavigable,                 /* widgetNavigable      */
  XmInheritFocusChange,                     /* focusChange          */
};

XmPrimitiveClassExtRec _MdialPrimitiveClassExtRec = 
{
     NULL,
     NULLQUARK,
     XmPrimitiveClassExtVersion,
     sizeof(XmPrimitiveClassExtRec),
     XmInheritBaselineProc,                  /* widget_baseline */
     XmInheritDisplayRectProc,               /* widget_display_rect */
     (XmWidgetMarginsProc)NULL,              /* widget_margins */
};


MdialClassRec mdialClassRec =
{
  {
    (WidgetClass)&xmPrimitiveClassRec,              /* superclass */
    "Mdial",                                        /* class_name */
    (Cardinal)sizeof(MdialRec),                     /* widget size */
    (XtProc)NULL,                                   /* class_init */
    (XtWidgetClassProc)NULL,                        /* class_part_init */
    (XtEnum)FALSE,                                  /* class_inited */
    (XtInitProc)initialize,                         /* initialize */
    (XtArgsProc)NULL,                               /* init_hook */
    (XtRealizeProc)XtInheritRealize,                         /* realize */
    (XtActionList)mdialActions,                     /* actions */
    (Cardinal)XtNumber(mdialActions),               /* num_actions */
    (XtResourceList)resources,                      /* resources */
    (Cardinal)XtNumber(resources),                  /* num_resources */
    NULLQUARK,                                      /* xrm_class */
    TRUE,                                          /* compress_motion */
    (XtEnum)TRUE,                                  /* compress_exposur */
    TRUE,                                          /* compress enterleave */
    TRUE,                                          /* visibility_interest */
    (XtWidgetProc)destroy,                             /* destroy */
    (XtWidgetProc)resize,                           /* resize */
    (XtExposeProc)expose,                                         /* expose */
    (XtSetValuesFunc)setValues,
    (XtArgsFunc)NULL,                               /* set_values_hook */
    XtInheritSetValuesAlmost,                       /* set_values_almost */
    (XtArgsProc)NULL,                               /* get_values_hook */
    XtInheritAcceptFocus,                           /* accept_focus */
    XtVersion,                                      /* version */
    (XtPointer)NULL,                                /* callback_private */
    mdialTranslations,
    (XtGeometryHandler)XtInheritQueryGeometry,      /* query_geometry */
    XtInheritDisplayAccelerator,                    /* display_accelerator */
    (XtPointer)&mdialBaseClassExtRec,               /* extension */
  },
  {
    XmInheritBorderHighlight,
    XmInheritBorderUnhighlight,
    NULL, /* XtInheritTranslations, */
    NULL,
    NULL,
    0,
    (XtPointer)&_MdialPrimitiveClassExtRec,
   },
  {
    0,
  },
};

WidgetClass mdialWidgetClass = (WidgetClass)&mdialClassRec;

static XtInitProc initialize
#ifdef _NO_PROTO
(_request, _new, _args, _numArgs)
MdialWidget _request;
MdialWidget _new;
String *_args;
Cardinal *_numArgs;
#else
(MdialWidget _request, MdialWidget _new, String *_args, Cardinal *_numArgs)
#endif
{
  if (WIDTH(_request) == 0)
    WIDTH(_new) = 50;
  
  if (HEIGHT(_request) == 0)
    HEIGHT(_new) = 50;

  LABEL(_new) = NULL;
  THE_FONT(_new) = NULL;

/*
** Fake for init
*/

  DX(_new) = DY(_new) = 0;

  DWIDTH(_new) = WIDTH(_new);
  DHEIGHT(_new) = WIDTH(_new);

  validateResources(NULL, _request, _new, TRUE);

  getDialFont(_new);
  FGC(_new) = getGC(_new, &PRIM(_new).foreground, &MDIAL(_new).dialBackground);
  BGC(_new) = getGC(_new, &MDIAL(_new).dialBackground, &MDIAL(_new).dialBackground);
  copyPrimitiveGC(_new, &CONT_TOP_GC(_new), TOP_GC(_new)); 
  copyPrimitiveGC(_new, &CONT_BOTTOM_GC(_new), BOTTOM_GC(_new));

  RANGLE(_new) = valueToRangle(_new, VALUE(_new));

  redoLabel(_new);
  resize(_new);

#if DRAG_AND_DROP
  XtAddEventHandler((Widget)_new, ButtonPressMask, FALSE, (XtEventHandler)dragWatch, (XtPointer)NULL);
#endif
}

static XtRealizeProc realize
#ifdef _NO_PROTO
(_mdw, _xvm, _xswa)
MdialWidget _mdw;
XtValueMask *_xvm;
XSetWindowAttributes *_xswa;
#else
(MdialWidget _mdw, XtValueMask *_xvm, XSetWindowAttributes *_xswa)
#endif /* _NO_PROTO */
{
  (*mdialWidgetClass->core_class.superclass->core_class.realize)((Widget)_mdw, _xvm, _xswa);
}

static XtExposeProc expose
#ifdef _NO_PROTO
(_mdw, _event, _region)
MdialWidget _mdw;
XEvent *_event;
Region _region;
#else
(MdialWidget _mdw, XEvent *_event, Region _region)
#endif
{
/*
** I'm lazy...
*/
  XClearWindow(XtDisplay(_mdw), XtWindow(_mdw));

  fillCircle(_mdw, BGC(_mdw),
    DX(_mdw), DY(_mdw), DWIDTH(_mdw), DHEIGHT(_mdw),
    FULL(_mdw));

  drawDiamond(_mdw, CONT_TOP_GC(_mdw), CONT_BOTTOM_GC(_mdw), BGC(_mdw));

  drawArm(_mdw, FGC(_mdw), TRUE);

  if (SHAD_TYPE(_mdw) == XmSHADOW_IN)
    drawCircle(_mdw, CONT_TOP_GC(_mdw), CONT_BOTTOM_GC(_mdw), 
      DX(_mdw), DY(_mdw), DWIDTH(_mdw), DHEIGHT(_mdw),
      FULL(_mdw), !FULL(_mdw));
  else
    drawCircle(_mdw, CONT_BOTTOM_GC(_mdw), CONT_TOP_GC(_mdw),
      DX(_mdw), DY(_mdw), DWIDTH(_mdw), DHEIGHT(_mdw),
      FULL(_mdw), !FULL(_mdw));


  drawString(_mdw, FGC(_mdw), LX(_mdw), LY(_mdw), LWIDTH(_mdw), LABEL(_mdw));

  if (PRIM(_mdw).shadow_thickness > 0)
    _XmDrawShadows (XtDisplay(_mdw), XtWindow(_mdw),
    TOP_GC(_mdw), BOTTOM_GC(_mdw), H_THICK(_mdw), H_THICK(_mdw), 
    WIDTH(_mdw) - 2 * H_THICK(_mdw),
    HEIGHT(_mdw) - 2 * H_THICK(_mdw),
    SH_THICK(_mdw), XmSHADOW_OUT);
}


static XtWidgetProc resize
#ifdef _NO_PROTO
(_mdw)
MdialWidget _mdw;
#else
(MdialWidget _mdw)
#endif
{
  doExtents(_mdw);
  moveArmToCurrentValue(_mdw);
}

static Boolean setValues
#ifdef _NO_PROTO
(_current, _request, _new, _args, _numArgs)
MdialWidget _current;
MdialWidget _request;
MdialWidget _new;
ArgList _args;
Cardinal *_numArgs;
#else
(MdialWidget _current, MdialWidget _request, MdialWidget _new, ArgList _args, Cardinal *_numArgs)
#endif
{
  return validateResources(_current, _request, _new, FALSE);
}

static Boolean validateResources
#ifdef _NO_PROTO
(_current, _request, _new, _fromInit)
MdialWidget _current;
MdialWidget _request;
MdialWidget _new;
Boolean _fromInit;
#else
(MdialWidget _current, MdialWidget _request, MdialWidget _new, Boolean _fromInit)
#endif
{
  char (parm[255])[3];
  Cardinal numParms;
  Boolean changed, redoFont, redoAllGC, redoFGC, redoBGC; 

  changed = FALSE;
  redoFont = FALSE;
  redoAllGC = FALSE;
  redoFGC = FALSE;
  redoBGC = FALSE;
  
  if (_current == NULL)
    _current = _request;

#if 0
  if (MARGIN_WIDTH(_new) < 0)
  {
    numParms = 0;
    XtAppWarningMsg(XtWidgetToApplicationContext((Widget)_new),
      "Resource", "resource", "ValueOutOfRange",
      "The XmNmarginWidth cannot be less than 0. XmNmarginWidth set to 0.", 
        (String *)parm, &numParms);
    MARGIN_WIDTH(_new) = 0;
    changed = TRUE;
  }

  if (MARGIN_HEIGHT(_new) < 0)
  {
    numParms = 0;
    XtAppWarningMsg(XtWidgetToApplicationContext((Widget)_new),
      "Resource", "resource", "ValueOutOfRange",
      "The XmNmarginHeight cannot be less than 0. XmNmarginHeight set to 0.", 
        (String *)parm, &numParms);
    MARGIN_HEIGHT(_new) = 0;
    changed = TRUE;
  }

  if (MARGIN_WIDTH(_new) < 0)
  {
    numParms = 0;
    XtAppWarningMsg(XtWidgetToApplicationContext((Widget)_new),
      "Resource", "resource", "ValueOutOfRange",
      "The XmNmarginWidth cannot be less than 0. XmNmarginWidth set to 0.", 
        (String *)parm, &numParms);
    MARGIN_WIDTH(_new) = 0;
    changed = TRUE;
  }
#endif

  if (LALIGN(_new) != LALIGN(_current))
    changed = TRUE;

  if (SHAD_TYPE(_new) != SHAD_TYPE(_current))
    changed = TRUE;

  if (VMAX(_new) <= VMIN(_new))
  {
    numParms = 0;
    XtAppWarningMsg(XtWidgetToApplicationContext((Widget)_new),
      "Resource", "resource", "ValueOutOfRange",
      "The XmNmaximum cannot be less or equal to XmNminimum. XmNmaximum set to XmNminimum+1.",
        (String *)parm, &numParms);
    VMAX(_new) = VMIN(_new)+1;
    changed = TRUE;
  }

  if (VALUE(_new) < VMIN(_new))
  {
    numParms = 0;
    sprintf(parm[0], "%d", VMIN(_new));
    XtAppWarningMsg(XtWidgetToApplicationContext((Widget)_new),
      "Resource", "resource", "ValueOutOfRange",
      "The XmNvalue cannot be less than XmNminimum. XmNvalue set to XmNminimum.",
        (String *)parm, &numParms);
    VALUE(_new) = VMIN(_new);
    changed = TRUE;
  }
  else
  if (VALUE(_new) > VMAX(_new))
  {
    numParms = 0;
    sprintf(parm[0], "%d", VMAX(_new));
    XtAppWarningMsg(XtWidgetToApplicationContext((Widget)_new),
      "Resource", "resource", "ValueOutOfRange",
      "The XmNvalue cannot be more than XmNmaximum. XmNvalue set to XmNmaximum.",
        (String *)parm, &numParms);
    VALUE(_new) = VMAX(_new);
    changed = TRUE;
  }

  if (VALUE(_new) != VALUE(_current))
    changed = TRUE;

  if (_fromInit)
    return changed;

  if (FONT_LIST(_new) != FONT_LIST(_current))
  {
    XmFontListFree(FONT_LIST(_current));
    XFreeFont(XtDisplay(_new), THE_FONT(_current));

    getDialFont(_new);
    redoFGC = TRUE;
    redoBGC = TRUE;
  }

  if (MDIAL(_new).processingDirection != MDIAL(_current).processingDirection)
    changed = TRUE;

  if (CORE(_new).sensitive != CORE(_current).sensitive)
  {
    redoFGC = TRUE;
    redoBGC = TRUE;
  }

  if (PRIM(_new).foreground != PRIM(_current).foreground)
    redoFGC = TRUE;

  if (SH_THICK(_new) != SH_THICK(_current))
  {
    redoFGC = TRUE;
    redoBGC = TRUE;
  }

  if (MDIAL(_new).dialBackground != MDIAL(_current).dialBackground)
    redoBGC = TRUE;

  if (TOP_GC(_new) != TOP_GC(_current))
    redoAllGC = TRUE;

  if (BOTTOM_GC(_new) != BOTTOM_GC(_current))
    redoAllGC = TRUE;

  if (redoAllGC || redoFGC)
  {
    XtReleaseGC((Widget)_new, FGC(_new));
    FGC(_new) = getGC(_new, &PRIM(_new).foreground, &MDIAL(_new).dialBackground);
    changed = TRUE;
  }

  if (redoAllGC || redoBGC)
  {
    XtReleaseGC((Widget)_new, BGC(_new));
    BGC(_new) = getGC(_new, &MDIAL(_new).dialBackground, &MDIAL(_new).dialBackground);
    changed = TRUE;
  }

  if (redoAllGC)
  {
    XtReleaseGC((Widget)_new, CONT_TOP_GC(_new));
    copyPrimitiveGC(_new, &CONT_TOP_GC(_new), TOP_GC(_new));

    XtReleaseGC((Widget)_new, CONT_BOTTOM_GC(_new));
    copyPrimitiveGC(_new, &CONT_BOTTOM_GC(_new), BOTTOM_GC(_new));

    changed = TRUE;
  }

  if (changed)
  {
    RANGLE(_new) = valueToRangle(_new, VALUE(_new));
    resize(_new);
  }

  return changed;
}

static void destroy
#ifdef _NO_PROTO
(_mdw)
MdialWidget _mdw;
#else
(MdialWidget _mdw)
#endif
{
  XtReleaseGC((Widget)_mdw, FGC(_mdw));
  XtReleaseGC((Widget)_mdw, BGC(_mdw));
  XtReleaseGC((Widget)_mdw, CONT_TOP_GC(_mdw));
  XtReleaseGC((Widget)_mdw, CONT_BOTTOM_GC(_mdw));
  XtRemoveAllCallbacks((Widget)_mdw, XmNincrementCallback);
  XtRemoveAllCallbacks((Widget)_mdw, XmNdecrementCallback);
  XtRemoveAllCallbacks((Widget)_mdw, XmNvalueChangedCallback);
  XmStringFree(LABEL(_mdw));
  XFreeFont(XtDisplay(_mdw), THE_FONT(_mdw));
  XmFontListFree(FONT_LIST(_mdw));
}

#define USEFUL_GC (GCFont | GCForeground | GCBackground | GCLineStyle | GCCapStyle | GCJoinStyle | GCFillStyle | GCFillRule) 

static void copyPrimitiveGC
#ifdef _NO_PROTO
(_mdw, _toGC, _fromGC)
MdialWidget _mdw;
GC *_toGC;
GC _fromGC;
#else
(MdialWidget _mdw, GC *_toGC, GC _fromGC)
#endif 
{
  XGCValues gcValues;

  XGetGCValues(XtDisplay(_mdw), _fromGC, USEFUL_GC, &gcValues);

  gcValues.line_width = SH_THICK(_mdw);
  gcValues.join_style = JoinMiter;

  *_toGC = XtGetGC((Widget)_mdw,  USEFUL_GC | GCLineWidth, &gcValues);
}

static void getDialFont
#ifdef _NO_PROTO
(_mdw)
MdialWidget _mdw;
#else
(MdialWidget _mdw)
#endif
{ 
  if (FONT_LIST(_mdw) == NULL)
  {
    XmFontList defaultFont;

    defaultFont = _XmGetDefaultFontList((Widget)_mdw, 0);
    FONT_LIST(_mdw) = XmFontListCopy(defaultFont);
  }
  else
    FONT_LIST(_mdw) = XmFontListCopy(FONT_LIST(_mdw));

  if (FONT_LIST(_mdw) == (XmFontList)NULL)
  {
    THE_FONT(_mdw) = XLoadQueryFont(XtDisplay(_mdw), "fixed");
    if (THE_FONT(_mdw) == (XFontStruct *)NULL)
      THE_FONT(_mdw) = XLoadQueryFont(XtDisplay(_mdw), "*");
   }
   else
     _XmFontListGetDefaultFont(FONT_LIST(_mdw), &THE_FONT(_mdw));
}

static GC getGC
#ifdef _NO_PROTO
(_mdw, _fg, _bg)
MdialWidget _mdw;
Pixel *_fg;
Pixel *_bg;
#else
(MdialWidget _mdw, Pixel *_fg, Pixel *_bg)
#endif
{
  XGCValues gcValues;
  XtGCMask gcMask;

  gcMask = GCFont | GCArcMode | GCJoinStyle | GCLineWidth;
 
  if (_fg)
  {
    gcMask |= GCForeground;
    gcValues.foreground = *_fg;
  }
  else
  *_fg = PRIM(_mdw).foreground;

  if (_bg)
  {
    gcMask |= GCBackground;
    gcValues.background = *_bg;
  }
  else
  *_bg = CORE(_mdw).background_pixel;

  gcValues.font = THE_FONT(_mdw)->fid;
  gcValues.arc_mode = ArcPieSlice;
  gcValues.join_style = JoinMiter;
  gcValues.line_width = SH_THICK(_mdw);

  if (!CORE(_mdw).sensitive)
  {
    gcMask |= GCTile;
    gcMask |= GCFillStyle;
    gcValues.tile = XmGetPixmapByDepth(XtScreen(_mdw), "50_foreground", *_fg, *_bg, CORE(_mdw).depth);
    gcValues.fill_style = FillTiled;
  }

  return XtGetGC((Widget)_mdw,  gcMask, &gcValues);
}

static void drawString
#ifdef _NO_PROTO
(_mdw, _gc, _x, _y, _width, _xmstring)
MdialWidget _mdw;
GC _gc;
Position _x;
Position _y;
Dimension _width;
XmString _xmstring;
#else
(MdialWidget _mdw, GC _gc, Position _x, Position _y, Dimension _width, XmString _xmstring)
#endif
{
  XmStringDraw(XtDisplay(_mdw), XtWindow(_mdw), FONT_LIST(_mdw), _xmstring, _gc, 
    _x, _y,  _width, XmALIGNMENT_BEGINNING, MDIAL(_mdw).stringDirection, NULL);

  XFlush(XtDisplay(_mdw));
}


static void drawCircle
#ifdef _NO_PROTO
(_mdw, _top, _bottom, _x, _y, _width, _height, _full, _underline)
MdialWidget _mdw;
GC _top;
GC _bottom;
Position _x;
Position _y;
Dimension _width;
Dimension _height;
Boolean _full;
Boolean _underline;
#else
(MdialWidget _mdw, GC _top, GC _bottom, Position _x, Position _y, Dimension _width, Dimension _height, Boolean _full, Boolean _underline)
#endif
{
  int angle1, angle2;
  int yPos;

  if (_full)
  {
    angle1 = 45*64;
    angle2 = 180*64;
  }
  else
  {
    angle1 = 45*64;
    angle2 = 135*64;
  }

  XDrawArc(XtDisplay(_mdw), XtWindow(_mdw), _bottom,
    _x, _y, _width, _height,
    angle1, angle2);

  if (_full)
  {
    angle1 = 225*64;
    angle2 = 180*64;
  }
  else
  {
    angle1 = 0*64;
    angle2 = 45*64;
  }

  XDrawArc(XtDisplay(_mdw), XtWindow(_mdw), _top,
    _x, _y, _width, _height,
    angle1, angle2);

  if (_underline)
  {
    yPos = _y + _height/2; 

    XDrawLine(XtDisplay(_mdw), XtWindow(_mdw), _top,
      _x,
      yPos,
      _x + _width,
      yPos);
  }
}

static void drawArm
#ifdef _NO_PROTO
(_mdw, _gc, _fancy)
MdialWidget _mdw;
GC _gc;
Boolean _fancy;
#else
(MdialWidget _mdw, GC _gc, Boolean _fancy)
#endif
{
  int i, rad, cx, cy;
  XPoint arm[4];

  rad = MIN(DWIDTH(_mdw), DHEIGHT(_mdw))/2;

  cx = CX(_mdw);  cy = CY(_mdw);

  arm[0].x = AX(_mdw); /* cx + (int) ((double) rad * .80 * cos(RANGLE(_mdw)) ); */
  arm[0].y = AY(_mdw); /* cy + (int) ((double) rad * .80 * sin(RANGLE(_mdw)) ); */

  arm[1].x = cx + (int) ((double) rad * .33 * cos(RANGLE(_mdw)+160) );
  arm[1].y = cy + (int) ((double) rad * .33 * sin(RANGLE(_mdw)+160) );
  arm[2].x = cx + (int) ((double) rad * .33 * cos(RANGLE(_mdw)-160) );
  arm[2].y = cy + (int) ((double) rad * .33 * sin(RANGLE(_mdw)-160) );
  arm[3].x = arm[0].x;
  arm[3].y = arm[0].y;

  if (!_fancy)
    XDrawLines(XtDisplay(_mdw), XtWindow(_mdw), _gc, arm, 4, CoordModeOrigin);
  else
  {
    GC tempGC;

    if ((RANGLE(_mdw) < -PI/2) || (RANGLE(_mdw) > PI*.25))
      tempGC = CONT_TOP_GC(_mdw);
    else
      tempGC = CONT_BOTTOM_GC(_mdw);

    XDrawLine(XtDisplay(_mdw), XtWindow(_mdw), tempGC, arm[0].x, arm[0].y, arm[1].x, arm[1].y);

    if ((RANGLE(_mdw) < PI*.75) && (RANGLE(_mdw) > -PI*.25))
      tempGC = CONT_TOP_GC(_mdw);
    else
      tempGC = CONT_BOTTOM_GC(_mdw);

    XDrawLine(XtDisplay(_mdw), XtWindow(_mdw), tempGC, arm[1].x, arm[1].y, arm[2].x, arm[2].y);

    if ((RANGLE(_mdw) < PI*.25) && (RANGLE(_mdw) > -PI*.75))
      tempGC = CONT_TOP_GC(_mdw);
    else
      tempGC = CONT_BOTTOM_GC(_mdw);

    XDrawLine(XtDisplay(_mdw), XtWindow(_mdw), tempGC, arm[2].x, arm[2].y, arm[3].x, arm[3].y);
  }

  if (DOT_RADIUS(_mdw) > MIN_DOT_RADIUS(_mdw))
    drawCircle(_mdw, CONT_BOTTOM_GC(_mdw), CONT_TOP_GC(_mdw),
      DX(_mdw) + DWIDTH(_mdw)/2 - DOT_RADIUS(_mdw)/2, DY(_mdw) + DHEIGHT(_mdw)/2 - DOT_RADIUS(_mdw)/2, DOT_RADIUS(_mdw), 
      DOT_RADIUS(_mdw), FULL(_mdw), !FULL(_mdw));

  XFlush(XtDisplay(_mdw));
}

static void fillCircle
#ifdef _NO_PROTO
(_mdw, _gc, _x, _y, _width, _height, _full)
MdialWidget _mdw;
GC _gc;
Position _x;
Position _y;
Dimension _width;
Dimension _height;
Boolean _full;
#else
(MdialWidget _mdw, GC _gc, Position _x, Position _y, Dimension _width, Dimension _height, Boolean _full)
#endif
{
  if (_full)
    XFillArc(XtDisplay(_mdw), XtWindow(_mdw), _gc,
      _x, _y, _width, _height,
      0, 360*64);
  else
    XFillArc(XtDisplay(_mdw), XtWindow(_mdw), _gc,
      _x, _y, _width, _height,
     0, 180*64);
}


static void drawDiamond
#ifdef _NO_PROTO
(_mdw, _topGC, _bottomGC, _fillGC)
MdialWidget _mdw;
GC _topGC;
GC _bottomGC;
GC _fillGC;
#else
(MdialWidget _mdw, GC _topGC, GC _bottomGC, GC _fillGC)
#endif 
{
  Position x, y;

 if (DOT_RADIUS(_mdw) == MIN_DOT_RADIUS(_mdw))
   return;

  switch(MDIAL(_mdw).processingDirection)
  {
    case XmMAX_ON_TOP:
      x = CX(_mdw) - SH_THICK(_mdw) + 1;
      y = CY(_mdw) - DHEIGHT(_mdw)/2 + SH_THICK(_mdw);
      break;
    case XmMAX_ON_BOTTOM:
      x = CX(_mdw) - SH_THICK(_mdw) - 1;
      y = CY(_mdw) + DHEIGHT(_mdw)/2 - SH_THICK(_mdw);
      break;
    case XmMAX_ON_LEFT:
      x = CX(_mdw) - DWIDTH(_mdw)/2 + SH_THICK(_mdw);
      y = CY(_mdw) - SH_THICK(_mdw) + 1;
      break;
    case XmMAX_ON_RIGHT:
      x = CX(_mdw) + DWIDTH(_mdw)/2 - SH_THICK(_mdw);
      y = CY(_mdw) - SH_THICK(_mdw) - 1;
      break;
  }

  _XmDrawDiamond(XtDisplay(_mdw), XtWindow (_mdw),
            _topGC, _bottomGC, _fillGC, x, y,
            DOT_RADIUS(_mdw), DOT_RADIUS(_mdw), SH_THICK(_mdw), FALSE);
}

static void getBackgroundPixel
#ifdef _NO_PROTO
(_mdw, _offset, _value)
MdialWidget _mdw;
int _offset;
XrmValue *_value;
#else
(MdialWidget _mdw, int _offset, XrmValue *_value)
#endif
{
  _value->addr = (XtPointer)&CORE(_mdw).background_pixel;
}

static void redoLabel
#ifdef _NO_PROTO
(_mdw)
MdialWidget _mdw;
#else
(MdialWidget _mdw)
#endif
{
  char temp[MAX_DIAL_LABEL+1];

  if (MDIAL(_mdw).labelFormatter == (t_func)NULL)
    sprintf(temp, "%d", VALUE(_mdw));
  else
    (*MDIAL(_mdw).labelFormatter)(_mdw, VALUE(_mdw), temp);

  if (LABEL(_mdw))
    XmStringFree(LABEL(_mdw));

  LABEL(_mdw) = XmStringCreateSimple(temp);

  XmStringExtent(FONT_LIST(_mdw), LABEL(_mdw), &LWIDTH(_mdw), &LHEIGHT(_mdw));

  switch(LALIGN(_mdw))
  {
    case XmALIGNMENT_BEGINNING:
      LX(_mdw) = DX(_mdw);
      break;
    case XmALIGNMENT_CENTER:
      LX(_mdw) = WIDTH(_mdw)/2 - LWIDTH(_mdw)/2;
      break;
    case XmALIGNMENT_END:
      LX(_mdw) = DX(_mdw) + DWIDTH(_mdw) - LWIDTH(_mdw);
      break;
  }
}

static void moveArmToCurrentValue
#ifdef _NO_PROTO
(_mdw)
MdialWidget _mdw;
#else
(MdialWidget _mdw)
#endif
{
  int length;

  length = MIN(DWIDTH(_mdw), DHEIGHT(_mdw));

  AX(_mdw) = CX(_mdw) + (int)(cos(RANGLE(_mdw))*length*3.0/8.0);
  AY(_mdw) = CY(_mdw) + (int)(sin(RANGLE(_mdw))*length*3.0/8.0);
}


static void setArmValue
#ifdef _NO_PROTO
(_mdw, _x, _y)
MdialWidget _mdw;
int _x;
int _y;
#else
(MdialWidget _mdw, int _x, int _y)
#endif
{
  int length;

  RANGLE(_mdw) = getRangle(_mdw, _x, _y);

  VALUE(_mdw) = rangleToValue(_mdw, RANGLE(_mdw));
}

static void armChange
#ifdef _NO_PROTO
(_mdw, _x, _y)
MdialWidget _mdw;
int _x;
int _y;
#else
(MdialWidget _mdw, int _x, int _y)
#endif
{
  int newValue;
  double newRangle;

  newRangle = getRangle(_mdw, _x, _y);
  newValue = rangleToValue(_mdw, newRangle);

  if (newValue == VALUE(_mdw))
    return;

  doMoveArm(_mdw, newValue, newRangle);
}

static void doMoveArm
#ifdef _NO_PROTO
(_mdw, _value, _rangle)
MdialWidget _mdw;
int _value;
double _rangle;
#else
(MdialWidget _mdw, int _value, double _rangle)
#endif
{
  drawArm(_mdw, BGC(_mdw), FALSE);

  XClearArea(XtDisplay(_mdw), XtWindow(_mdw), LX(_mdw), LY(_mdw), LWIDTH(_mdw), LHEIGHT(_mdw), FALSE);

  VALUE(_mdw) = _value;
  RANGLE(_mdw) = _rangle; 

  redoLabel(_mdw);
  moveArmToCurrentValue(_mdw);

  drawArm(_mdw, FGC(_mdw), TRUE);
  drawString(_mdw, FGC(_mdw), LX(_mdw), LY(_mdw), LWIDTH(_mdw), LABEL(_mdw));
}

static void set
#ifdef _NO_PROTO
(_w, _event, _parms, _numParms)
Widget _w;
XEvent *_event;
String *_parms;
Cardinal *_numParms;
#else
(Widget _w, XEvent *_event, String *_parms, Cardinal *_numParms)
#endif
{
  MdialCallbackStruct mbcs;
  MdialWidget mdw;
  int oldValue;

  mdw = (MdialWidget)_w;

  if (RO(mdw))
    return;

  if (_event->type == ButtonPress)
    if (!goodDistance(mdw, _event->xbutton.x, _event->xbutton.y))
      return;

  if (!FULL(mdw) && (CY(mdw) < _event->xbutton.y))
    return;

  oldValue = VALUE(mdw);

  armChange(mdw, _event->xbutton.x, _event->xbutton.y);

  if (VALUE(mdw) == oldValue)
    return;

#ifndef NO_INCDEC_ON_SET
  if ((VALUE(mdw) < oldValue) && XtHasCallbacks((Widget)mdw, XmNdecrementCallback))
  {
    doCallback(mdw, XmCR_DECREMENT, _event);
    return;
  }
  else
  if ((VALUE(mdw) > oldValue) && XtHasCallbacks((Widget)mdw, XmNincrementCallback))
  {
    doCallback(mdw, XmCR_INCREMENT, _event);
    return;
  }
#endif

  doCallback(mdw, XmCR_VALUE_CHANGED, _event);

}

static void increment 
#ifdef _NO_PROTO
(_w, _event, _parms, _numParms)
Widget _w;
XEvent *_event;
String *_parms;
Cardinal *_numParms;
#else
(Widget _w, XEvent *_event, String *_parms, Cardinal *_numParms)
#endif
{
  MdialWidget mdw;
  MdialCallbackStruct mbcs;
  int newValue;
  int inc;

  mdw = (MdialWidget)_w;

  if (RO(mdw))
    return;

  if (*_numParms == 0)
    inc = 1;
  else
    inc = atoi(_parms[0]);

  if (inc <= 0)
    inc = 1;

  newValue = ((VALUE(mdw) + inc) > VMAX(mdw)) ? VMAX(mdw) : VALUE(mdw) + inc;

  if (newValue == VALUE(mdw))
    return;

  doMoveArm(mdw, newValue, valueToRangle(mdw, newValue));

  if (XtHasCallbacks((Widget)mdw, XmNincrementCallback))
    doCallback(mdw, XmCR_INCREMENT, _event);
  else
    doCallback(mdw, XmCR_VALUE_CHANGED, _event);
}

static void decrement 
#ifdef _NO_PROTO
(_w, _event, _parms, _numParms)
Widget _w;
XEvent *_event;
String *_parms;
Cardinal *_numParms;
#else
(Widget _w, XEvent *_event, String *_parms, Cardinal *_numParms)
#endif
{
  MdialWidget mdw;
  MdialCallbackStruct mbcs;
  int newValue;
  int dec;

  mdw = (MdialWidget)_w;

  if (RO(mdw))
    return;

  if (*_numParms == 0)
    dec = 1;
  else
    dec = atoi(_parms[0]);

  if (dec <= 0)
    dec = 1;

  newValue = ((VALUE(mdw) - dec) < VMIN(mdw)) ? VMIN(mdw) : VALUE(mdw) - dec;

  if (newValue == VALUE(mdw)) 
    return;

  doMoveArm(mdw, newValue, valueToRangle(mdw, newValue));

  if (XtHasCallbacks((Widget)mdw, XmNincrementCallback))
    doCallback(mdw, XmCR_DECREMENT, _event);
  else
    doCallback(mdw, XmCR_VALUE_CHANGED, _event);
}

static void drag 
#ifdef _NO_PROTO
(_w, _event, _parms, _numParms)
Widget _w;
XEvent *_event;
String *_parms;
Cardinal *_numParms;
#else
(Widget _w, XEvent *_event, String *_parms, Cardinal *_numParms)
#endif
{
  MdialWidget mdw;

  mdw = (MdialWidget)_w;

  if (RO(mdw))
    return;

  if (_event->type == ButtonPress)
    if (!goodDistance(mdw, _event->xbutton.x, _event->xbutton.y))
      return;

  if (!FULL(mdw) && (CY(mdw) < _event->xbutton.y))
    return;

  armChange(mdw, _event->xbutton.x, _event->xbutton.y);
}



static int rangleToValue
#ifdef _NO_PROTO
(_mdw, _rangle)
MdialWidget _mdw;
double _rangle;
#else
(MdialWidget _mdw, double _rangle)
#endif
{
  switch(MDIAL(_mdw).processingDirection)
  {
    case XmMAX_ON_TOP:
      _rangle += PI/2;
      break;
    case XmMAX_ON_BOTTOM:
      _rangle -= PI/2;
      break;
    case XmMAX_ON_LEFT:
      _rangle += PI;
      break;
  }

  if (_rangle < 0.0)
    _rangle += (2*PI);
  else
  if (_rangle > (2*PI))
    _rangle -= (2*PI);
  
  return (int) (VMIN(_mdw) + (_rangle/(2.0*PI)) * (VMAX(_mdw) - VMIN(_mdw)));
}

static double valueToRangle
#ifdef _NO_PROTO
(_mdw, _value)
MdialWidget _mdw;
int _value;
#else
(MdialWidget _mdw, int _value)
#endif
{
  double rangle;
  rangle = ((double)(VALUE(_mdw) - VMIN(_mdw)))/ ((double)(VMAX(_mdw) - VMIN(_mdw))) * (2.0*PI);

  switch(MDIAL(_mdw).processingDirection)
  {
    case XmMAX_ON_TOP:
      rangle -= PI/2;
      break;
    case XmMAX_ON_BOTTOM:
      rangle += PI/2;
      break;
    case XmMAX_ON_LEFT:
      rangle -= PI;
      break;
  }

  return rangle;
}

static void doExtents
#ifdef _NO_PROTO
(_mdw)
MdialWidget _mdw;
#else
(MdialWidget _mdw)
#endif
{
  int yLeft, usableY, extraMargin;

  DX(_mdw) = 2 * PRIM(_mdw).shadow_thickness + PRIM(_mdw).highlight_thickness + MARGIN_WIDTH(_mdw);
  DY(_mdw) = 2 * PRIM(_mdw).shadow_thickness + PRIM(_mdw).highlight_thickness + MARGIN_HEIGHT(_mdw);

  usableY = HEIGHT(_mdw) - ((4 * PRIM(_mdw).shadow_thickness) + (2 * PRIM(_mdw).highlight_thickness) + (3 * MARGIN_HEIGHT(_mdw)));

  XmStringExtent(FONT_LIST(_mdw), LABEL(_mdw), &LWIDTH(_mdw), &LHEIGHT(_mdw));

  yLeft = usableY - LHEIGHT(_mdw);

  if (yLeft < 10)
    yLeft = 20;

  if (FULL(_mdw))
    DHEIGHT(_mdw) = yLeft;
  else
    DHEIGHT(_mdw) = yLeft/2;

  DWIDTH(_mdw) = WIDTH(_mdw) - ((4 * PRIM(_mdw).shadow_thickness) + (2 * PRIM(_mdw).highlight_thickness) + (2 * MARGIN_WIDTH(_mdw)));

  if (ASPECT(_mdw))
  {
    int diff;
    if (DHEIGHT(_mdw) < DWIDTH(_mdw))
    {
      diff = DWIDTH(_mdw) - DHEIGHT(_mdw);
      DWIDTH(_mdw) = DHEIGHT(_mdw);
      DX(_mdw) += diff/2;
    }
    else
    {
      diff = DHEIGHT(_mdw) - DWIDTH(_mdw);
      DHEIGHT(_mdw) = DWIDTH(_mdw);
      extraMargin += diff/3;
    }
  }

  extraMargin = (usableY - (DHEIGHT(_mdw) + LHEIGHT(_mdw)))/2;

  DY(_mdw) += extraMargin;

  LY(_mdw) = DY(_mdw) + DHEIGHT(_mdw) + MARGIN_HEIGHT(_mdw);

  redoLabel(_mdw);

  if (!FULL(_mdw))
    DHEIGHT(_mdw) = yLeft;

  CX(_mdw) = DX(_mdw) + DWIDTH(_mdw)/2;
  CY(_mdw) = DY(_mdw) + DHEIGHT(_mdw)/2;
}

static Boolean goodDistance
#ifdef _NO_PROTO
(_mdw, _x, _y)
MdialWidget _mdw;
int _x, _y;
#else
(MdialWidget _mdw, int _x, int _y)
#endif
{
  double a, b;
  double dx, dy;
  double c1, c2;

  if (!FULL(_mdw))
    if ((CY(_mdw) - _y) > (DY(_mdw) + DHEIGHT(_mdw)))
      return FALSE;

  a = DWIDTH(_mdw)/2.0;
  b = DHEIGHT(_mdw)/2.0;
  dx = (double)(_x - CX(_mdw));
  dy = (double)(_y - CY(_mdw));

  c1 = (dx*dx) / (a*a);
  c2 = (dy*dy) / (b*b);

  if ((c1 + c2) <= 1.0)
    return TRUE;
  else
    return FALSE;
}

static double getRangle
#ifdef  _NO_PROTO
(_mdw, _x, _y)
MdialWidget _mdw;
int _x, _y;
#else
(MdialWidget _mdw, int _x, int _y)
#endif
{
  double rangle;
  double dx, dy;

  dx = ((double)_x - CX(_mdw));

  if (ABS(dx) < 2)
    return RANGLE(_mdw);
 
  dy = ((double)_y - CY(_mdw));

  rangle = atan2(dy, dx);

  return rangle;
}

#if DRAG_AND_DROP

static void dragWatch
#ifdef _NO_PROTO
(_mdw, _data, _event, _cont)
MdialWidget _mdw;
XtPointer _data;
XEvent *_event;
Boolean *_cont;
#else
(MdialWidget _mdw, XtPointer _data, XEvent *_event, Boolean *_cont)
#endif /* _NO_PROTO */
{
  Widget icon;
  Arg wargs[10];
  int n;
  Position x, y;
  Atom atom;

  if (_event->xbutton.button != Button2)
    return;

  x = _event->xbutton.x;
  y = _event->xbutton.y;

  if ( (x < LX(_mdw)) || (x > (LX(_mdw) + LWIDTH(_mdw))) ||
       (y < LY(_mdw)) || (y > (LY(_mdw) + LHEIGHT(_mdw))) )
    return;

  atom = XmInternAtom(XtDisplay(_mdw), "COMPOUND_TEXT", FALSE);

  icon = _XmGetTextualDragIcon((Widget)_mdw);

  n = 0;
  XtSetArg(wargs[n], XmNcursorBackground, CORE(_mdw).background_pixel); n++;
  XtSetArg(wargs[n], XmNcursorForeground, PRIM(_mdw).foreground); n++;
  XtSetArg(wargs[n], XmNsourceCursorIcon, icon);  n++;
  XtSetArg(wargs[n], XmNexportTargets, &atom); n++;
  XtSetArg(wargs[n], XmNnumExportTargets, 1); n++;
  XtSetArg(wargs[n], XmNdragOperations, XmDROP_COPY); n++;
  XtSetArg(wargs[n], XmNconvertProc, dragConvertProc); n++;
  XtSetArg(wargs[n], XmNclientData, (Widget)_mdw);  n++;
  XmDragStart((Widget)_mdw, _event, wargs, n);
}

static Boolean dragConvertProc
#ifdef _NO_PROTO
(_w, _selection, _target, _toType, _toValue, _toLength, _toFormat, _toMaxLength, _data, _id)
Widget _w;
Atom *_selection;
Atom *_target;
Atom *_toType;
XtPointer *_toValue;
unsigned long *_toLength;
int *_toFormat;
unsigned long *_toMaxLength;
XtPointer _data;
XtRequestId *_id;
#else
(Widget _w, Atom *_selection, Atom *_target, Atom *_toType, XtPointer *_toValue, unsigned long *_toLength,
  int *_toFormat, unsigned long *_toMaxLength, XtPointer _data, XtRequestId *_id)
#endif /* _NO_PROTO */
{
  MdialWidget mdw;
  static char temp[MAX_DIAL_LABEL+1];
  char *list;
  String newString;
  Atom Atom_compoundText, Atom_string, Atom_MotifDrop; 
  Arg warg;
  XtPointer myWidget;
  XTextProperty tp;

  Atom_MotifDrop = XmInternAtom(XtDisplay(_w), "_MOTIF_DROP", FALSE);

  if (*_selection == Atom_MotifDrop)
  {
    XtSetArg (warg, XmNclientData, &myWidget);
    XtGetValues ((Widget)_w, &warg, 1);
    mdw = (MdialWidget)myWidget;
    if (mdw == (MdialWidget)NULL)
      return FALSE;
  } 
  else
    return FALSE;

  Atom_compoundText = XmInternAtom(XtDisplay(_w), "COMPOUND_TEXT", FALSE);
  Atom_string = XmInternAtom(XtDisplay(_w), "STRING", FALSE);

  if (MDIAL(mdw).labelFormatter == (t_func)NULL)
    sprintf(temp, "%d", VALUE(mdw));
  else
    (*MDIAL(mdw).labelFormatter)(mdw, VALUE(mdw), temp);

  if (*_target == Atom_string) 
  {
     *_toType = *_target;
     *_toValue = (XtPointer)temp;
     *_toLength = strlen(temp);
     *_toFormat = 8;
     return TRUE;
  }

  if (*_target != Atom_compoundText) 
    return FALSE;

  list = temp;

  XmbTextListToTextProperty(XtDisplay(_w), &list, 1, XCompoundTextStyle, &tp);
  newString = XtNewString((String)tp.value);
  XtFree((char *)tp.value);

  *_toType = *_target;
  *_toValue = (XtPointer)newString;
  *_toLength = strlen(newString);
  *_toFormat = 8;

  return TRUE;
}
#endif /* DRAG_AND_DROP */

static void doCallback
#ifdef _NO_PROTO
(_mdw, _reason, _event)
MdialWidget _mdw;
int _reason;
XEvent *_event;
#else
(MdialWidget _mdw, int _reason, XEvent *_event)
#endif
{
  MdialCallbackStruct mdcs;

  mdcs.reason = _reason; 
  mdcs.event = _event;
  mdcs.value = VALUE(_mdw);

  switch(_reason)
  {
    case XmCR_INCREMENT:
      XtCallCallbackList((Widget)_mdw, MDIAL(_mdw).incrementCallback, &mdcs);
      break;
    case XmCR_DECREMENT:
      XtCallCallbackList((Widget)_mdw, MDIAL(_mdw).decrementCallback, &mdcs);
      break;
    case XmCR_VALUE_CHANGED:
      XtCallCallbackList((Widget)_mdw, MDIAL(_mdw).valueChangedCallback, &mdcs);
      break;
  }
}

/*
** Public functions
*/

void MdialIncrement
#ifdef _NO_PROTO
(_w, _increment, _callCallbacks)
Widget _w;
int _increment;
Boolean _callCallbacks;
#else
(Widget _w, int _increment, Boolean _callCallbacks)
#endif
{
  MdialCallbackStruct mbcs;
  MdialWidget mdw;
  int newValue;
  double newRangle;

  if (!XtIsMdial(_w))
    return;

  mdw = (MdialWidget)_w;

  if (_increment <= 0)
    _increment = 1;

  newValue = VALUE(mdw)+_increment;

  if (newValue > VMAX(mdw))
    newValue = VMAX(mdw);

  if (VALUE(mdw) == newValue)
    return;

  newRangle = valueToRangle(mdw, newValue);

  doMoveArm(mdw, newValue, newRangle); /* side effect...it updates the values. */

  if (_callCallbacks)
  {
    if (XtHasCallbacks((Widget)mdw, XmNincrementCallback))
      doCallback(mdw, XmCR_INCREMENT, NULL);
    else
      doCallback(mdw, XmCR_VALUE_CHANGED, NULL);
  }
}

void MdialDecrement
#ifdef _NO_PROTO
(_w, _decrement, _callCallbacks)
Widget _w;
int _decrement;
Boolean _callCallbacks;
#else
(Widget _w, int _decrement, Boolean _callCallbacks)
#endif
{
  MdialCallbackStruct mbcs;
  MdialWidget mdw;
  int newValue;
  double newRangle;

  if (!XtIsMdial(_w))
    return;

  mdw = (MdialWidget)_w;

  if (_decrement <= 0)
    _decrement = 1;

  newValue = VALUE(mdw)-_decrement;

  if (newValue < VMIN(mdw))
    newValue = VMIN(mdw);

  if (VALUE(mdw) == newValue)
    return;

  newRangle = valueToRangle(mdw, newValue);

  doMoveArm(mdw, newValue, newRangle); /* side effect...it updates the values. */

  if (_callCallbacks)
  {
    if (XtHasCallbacks((Widget)mdw, XmNdecrementCallback))
      doCallback(mdw, XmCR_DECREMENT, NULL);
    else
      doCallback(mdw, XmCR_VALUE_CHANGED, NULL);
  }
}

void MdialSet
#ifdef _NO_PROTO
(_w, _value, _callCallbacks)
Widget _w;
int _value;
Boolean _callCallbacks;
#else
(Widget _w, int _value, Boolean _callCallbacks)
#endif
{
  MdialCallbackStruct mbcs;
  MdialWidget mdw;
  int newValue;
  double newRangle;

  if (!XtIsMdial(_w))
    return;

  mdw = (MdialWidget)_w;

  newValue = _value;

  if (newValue > VMAX(mdw))
    newValue = VMAX(mdw);
  else
  if (newValue < VMIN(mdw))
    newValue = VMIN(mdw);

  if (VALUE(mdw) == newValue)
    return;

  newRangle = valueToRangle(mdw, newValue);

  doMoveArm(mdw, newValue, newRangle); /* side effect...it updates the values. */

  if (_callCallbacks)
    doCallback(mdw, XmCR_VALUE_CHANGED, NULL);
}

