/*
 * Copyright 1992 John L. Cwikla
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of John L. Cwikla or
 * University of Illinois not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.  John L. Cwikla and University of Illinois make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * John L. Cwikla and University of Illinois disclaim all warranties with
 * regard to this software, including all implied warranties of
 * merchantability and fitness, in no event shall John L. Cwikla or
 * University of Illinois be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data or profits, whether in an action of contract, negligence or
 * other tortious action, arising out of or in connection with the use or
 * performance of this software.
 *
 * Author:
 * 	John L. Cwikla
 * 	Materials Research Laboratory Center for Computation
 * 	University Of Illinois at Urbana-Champaign
 *	104 S. Goodwin
 * 	Urbana, IL 61801
 * 
 * 	cwikla@uimrl7.mrl.uiuc.edu
 */

/*
** Simple Histogram Widget
*/

#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <math.h>
#include <varargs.h>

#include <Xfwf/ShistogramP.h>
#include <Xfwf/Shistogram.h>

/* For actions */
#if NeedFunctionProtoTypes
static void Select(ShistogramWidget _w, XEvent *_event, String *_argv, int *_argc);
#else
static void Select();
#endif

/* For widget internals */
#if NeedFunctionProtoTypes
static void Initialize(ShistogramWidget _request, ShistogramWidget _new);
static void Realize(ShistogramWidget _w, XtValueMask *_xvm, XSetWindowAttributes *_xswa);
static void Resize(ShistogramWidget _w);
static void Destroy(ShistogramWidget _w);
static void Redisplay(ShistogramWidget _w);
static Boolean SetValues(ShistogramWidget _current, ShistogramWidget _request, ShistogramWidget _new);
static void MyXtWarning();
#else
static void Initialize();
static void Realize();
static void Resize();
static void Destroy();
static void Redisplay();
static Boolean SetValues();
static void MyXtWarning();
#endif

/* Misc */
#if NeedFunctionProtoTypes
static void LabelExtents(LabelStruct *_ls);
static void PositionLabel(ShistogramWidget _w, LabelStruct *_ls);
static void RedoYAxisLabels(ShistogramWidget _w);
static void RedoXAxisLabels(ShistogramWidget _w);
static void DrawLabel(ShistogramWidget _w, GC _gc, LabelStruct *_ls);
static void DrawYAxisLabels(ShistogramWidget _w);
static void DrawXAxisLabels(ShistogramWidget _w);
static void DrawYAxis(ShistogramWidget _w);
static void DrawXAxis(ShistogramWidget _w);
static void DrawStats(ShistogramWidget _w);
static void DrawBars(ShistogramWidget _w);
static void FindMaxAxisLabelWidth(ShistogramWidget _w);
#else
static void LabelExtents();
static void PositionLabel();
static void RedoYAxisLabels();
static void RedoXAxisLabels();
static void DrawLabel();
static void DrawYAxisLabels();
static void DrawXAxisLabels();
static void DrawYAxis();
static void DrawXAxis();
static void DrawStats();
static void DrawBars();
static void FindMaxAxisLabelWidth();
#endif

static char ShistogramTranslations[] =
"<Btn1Down>:    select()";

static XtActionsRec ShistogramActions[] = 
{
  { "select", Select}
};

#define SHIST_MINWIDTH 300
#define SHIST_MINHEIGHT 300
#define TIC_LENGTH 5
#define TIC_WIDTH TIC_LENGTH 
#define AXIS_DIM (TIC_WIDTH + 1) /* TICK_LENGTH + 1 for actual line */

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a) ((a) > 0 ? (a) : -(a))
#endif

#ifndef SQR
#define SQR(a) ((a)*(a))
#endif

#define YCENTER(y, ls) ((y) - ls.font->descent + ls.height/2)
#define SCALEX(a) ((double)(SHIST.drawArea.width - (SHIST.maxYLabelWidth + AXIS_DIM + SHIST.margin) - \
                  SHIST.maxXLabelWidth)/(double)(SHIST.maximum - SHIST.minimum + 1) * (double)(a))
#define SCALEY(a) ((double)(SHIST.drawArea.height - (SHIST.axis.height + AXIS_DIM + SHIST.margin) - \
                  SHIST.axis.height)/(double)(SHIST.yMax - SHIST.minCount + 1) * (double)(a))

#define TheOffset(field) XtOffset(ShistogramWidget, shistogram.field)

static XtResource ShistogramResources[] = 
{
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
    TheOffset(foreground), XtRString, (caddr_t)XtDefaultForeground},
  {XtNaxisForeground, XtCAxisForeground, XtRPixel, sizeof(Pixel),
    TheOffset(axis.foreground), XtRString, (caddr_t)XtDefaultForeground},
  {XtNstatisticsForeground, XtCStatisticsForeground, XtRPixel, sizeof(Pixel),
    TheOffset(statistics.foreground), XtRString, (caddr_t)XtDefaultForeground},
  {XtNtitleForeground, XtCTitleForeground, XtRPixel, sizeof(Pixel),
    TheOffset(title.foreground), XtRString, (caddr_t)XtDefaultForeground},
  {XtNxAxis, XtCXAxis, XtRBoolean, sizeof(Boolean),
    TheOffset(xAxis), XtRImmediate, (caddr_t)TRUE},
  {XtNyAxis, XtCYAxis, XtRBoolean, sizeof(Boolean),
    TheOffset(yAxis), XtRImmediate, (caddr_t)TRUE}, 
  {XtNmargin, XtCMargin, XtRInt, sizeof(int),
    TheOffset(margin), XtRImmediate, (caddr_t)5},
  {XtNselectCallback, XtCCallback, XtRCallback, sizeof(caddr_t),
    TheOffset(selectCallback), XtRCallback, (caddr_t)NULL},
  {XtNaxisFont, XtCAxisFont, XtRFontStruct, sizeof(XFontStruct *),
    TheOffset(axis.font), XtRString, (caddr_t)XtDefaultFont},
  {XtNtitleFont, XtCTitleFont, XtRFontStruct, sizeof(XFontStruct *),
    TheOffset(title.font), XtRString, (caddr_t)XtDefaultFont},
  {XtNstatisticsFont, XtCStatisticsFont, XtRFontStruct, sizeof(XFontStruct *),
    TheOffset(statistics.font), XtRString, (caddr_t)XtDefaultFont},
  {XtNtitleLabel, XtCTitleLabel, XtRString, sizeof(String),
    TheOffset(title.label), XtRString, (caddr_t)"Shistogram Widget, John L. Cwikla, 1992"},
  {XtNshowTitle, XtCShowTitle, XtRBoolean, sizeof(Boolean),
    TheOffset(showTitle), XtRImmediate, (caddr_t)TRUE},
  {XtNshowStatistics, XtCShowStatistics, XtRBoolean, sizeof(Boolean),
    TheOffset(showStatistics), XtRImmediate, (caddr_t)TRUE}
};

#undef TheOffset

ShistogramClassRec shistogramClassRec = 
{
  {        /* CoreClassPart */
    (WidgetClass)&widgetClassRec,  /* superclass */
    "Shistogram",                  /* class_name */
    sizeof(ShistogramRec),         /* widget_size */
    NULL,                          /* class_initialize */
    NULL,                          /* class_part_initialize */
    FALSE,                         /* class_init */
    Initialize,                    /* initialize */
    NULL,                          /* initialize_hook */
    XtInheritRealize,              /* realize */
    ShistogramActions,             /* actions */
    XtNumber(ShistogramActions),   /* num_actions */
    ShistogramResources,           /* resources */
    XtNumber(ShistogramResources), /* num_resources */
    NULLQUARK,                     /* xrm_class */
    TRUE,                          /* compress_motion */
    TRUE,                          /* compress_exposure */
    TRUE,                          /* compress_enterleave */
    TRUE,                          /* visible_intress */
    Destroy,                       /* destroy */
    Resize,                        /* resize */
    Redisplay,                     /* expose */
    SetValues,                     /* set_values */
    NULL,                          /* set_values_hook */
    XtInheritSetValuesAlmost,      /* set_values_almost */
    NULL,                          /* get_values_hook */
    NULL,                          /* accept_focus */
    XtVersion,                     /* version */
    NULL,                          /* callback_private */
    ShistogramTranslations,        /* tm_translations */
    NULL,
    NULL,
    NULL,
  },
  { 
    0 /* empty */
  }
};

WidgetClass shistogramWidgetClass = (WidgetClass) &shistogramClassRec;

#define SHIST _new->shistogram
#define CORE _new->core

static void Initialize(_request, _new)
ShistogramWidget _request;
ShistogramWidget _new;
{
  Display *display;
  XGCValues gcValues;
  int val;

  display = XtDisplay(_new);

  for(val=0;val<MAXLABELS;val++)
  {
    SHIST.yAxisLabels[val].label = (String)XtMalloc(sizeof(char)*15);
    SHIST.xAxisLabels[val].label = (String)XtMalloc(sizeof(char)*15);
  }

  if (SHIST.axis.font == (XFontStruct *)NULL)
  {
    if ((SHIST.axis.font = XLoadQueryFont(display, "fixed")) == NULL) 
      if ((SHIST.axis.font = XLoadQueryFont(display, "9x15")) == NULL)
        MyXtWarning("ShistogramWidget: Fonts %s and %s not found.", "fixed", "9x15");
  }

  if (SHIST.title.font == (XFontStruct *)NULL)
  {
    if ((SHIST.title.font = XLoadQueryFont(display, "fixed")) == NULL)
      if ((SHIST.title.font = XLoadQueryFont(display, "9x15")) == NULL)
        MyXtWarning("ShistogramWidget: Fonts %s and %s not found.", "fixed", "9x15");
  }

  if (SHIST.statistics.font == (XFontStruct *)NULL)
  {
    if ((SHIST.statistics.font = XLoadQueryFont(display, "fixed")) == NULL)
      if ((SHIST.statistics.font = XLoadQueryFont(display, "9x15")) == NULL)
        MyXtWarning("ShistogramWidget: Fonts %s and %s not found.", "fixed", "9x15");
  }

  if (SHIST.margin < 0)
  {
    MyXtWarning("ShistogramWidget: Margin (%d) cannot be negative.  Margin set to 0.", SHIST.margin);
    SHIST.margin = 0;
  }

  SHIST.minimum = 0;
  SHIST.maximum = 10;
  SHIST.minCount = 0;
  SHIST.maxCount = 10;
  SHIST.mean = 0.0;
  SHIST.variance = 0.0;
  SHIST.number = 0;
  SHIST.yMax = SHIST.maxCount;
  SHIST.xMax = SHIST.maximum;

  FindMaxAxisLabelWidth(_new);

/* 
** All gc's are read only. 
*/
  gcValues.foreground = SHIST.foreground;
  SHIST.gc = XtGetGC((Widget)_new, GCForeground, &gcValues);

  gcValues.foreground = CORE.background_pixel;
  gcValues.background = CORE.background_pixel;
  SHIST.eraseGC = XtGetGC((Widget)_new, GCForeground | GCBackground, &gcValues);

  gcValues.foreground = SHIST.axis.foreground;
  gcValues.font = SHIST.axis.font->fid;
  SHIST.axis.gc = XtGetGC((Widget)_new, GCFont | GCForeground, &gcValues);

  gcValues.foreground = SHIST.statistics.foreground;
  gcValues.font = SHIST.statistics.font->fid;
  SHIST.statistics.gc = XtGetGC((Widget)_new, GCFont | GCForeground, &gcValues);

  gcValues.foreground = SHIST.title.foreground;
  gcValues.font = SHIST.title.font->fid;
  SHIST.title.gc = XtGetGC((Widget)_new, GCFont | GCForeground, &gcValues);

  LabelExtents(&SHIST.title);
  
  SHIST.statistics.label = 0;
  SHIST.axis.label = 0;

  LabelExtents(&SHIST.statistics);
  LabelExtents(&SHIST.axis);

  if (_request->core.width == 0)
    CORE.width = SHIST_MINWIDTH; 
  if (_request->core.height == 0)
    CORE.height = SHIST_MINHEIGHT; 

  Resize(_new);
}
#undef SHIST 
#undef CORE

#define SHIST _w->shistogram

static void FindMaxAxisLabelWidth(_w)
ShistogramWidget _w;
{
  int val;
/*
** See how many characters the "widest" number has.
*/

  SHIST.charCount = 0;
  if (ABS(SHIST.minimum) > ABS(SHIST.maximum))
  {
    if (SHIST.minimum <= 0)
      SHIST.charCount = 1;
    val = SHIST.minimum;
    for(val=ABS(SHIST.minimum);val>0;val/=10)
      SHIST.charCount++;
  }
  else
  {
    if (SHIST.maximum <= 0)
      SHIST.charCount = 1;
    val = SHIST.minimum;
    for(val=ABS(SHIST.maximum);val>0;val/=10)
      SHIST.charCount++;
  }
}

static void PositionLabel(_w, _ls)
ShistogramWidget _w;
LabelStruct *_ls;
{
  _ls->pos.x = SHIST.drawArea.x + (SHIST.drawArea.width/2) - (_ls->width)/2;
  _ls->pos.y = SHIST.drawArea.y + _ls->font->ascent;
  SHIST.drawArea.y += (_ls->height + SHIST.margin);
  SHIST.drawArea.height -= (_ls->height + SHIST.margin);
}

static void RedoYAxisLabels(_w)
ShistogramWidget _w;
{
  int i, height, startX;
  int value;
  double ySpace;
  double labelInc; 
  int values[MAXLABELS];
  int labelHeight;

  height = SHIST.drawArea.height - (SHIST.margin + AXIS_DIM + SHIST.axis.height);
  labelHeight = SHIST.axis.height + SHIST.axis.height/2;
  SHIST.yTics = (height - (labelHeight + SHIST.axis.height))/labelHeight;

  if (SHIST.yTics < 2)
  {
    SHIST.yTics = 0;
    return;
  }

  SHIST.maxYLabelWidth = 0;

  SHIST.yTics = MIN(SHIST.yTics, MAXLABELS);
#if 0
  SHIST.yTics--;
#endif

  labelInc = (double)(SHIST.maxCount - SHIST.minCount + 1.0)/(double)(SHIST.yTics);

  for(i=0;i<SHIST.yTics-1;i++)
  {
    values[i] = SHIST.minCount  + i * labelInc;
    sprintf(SHIST.yAxisLabels[i].label, "%d", values[i]);
    SHIST.yAxisLabels[i].width = XTextWidth(SHIST.axis.font, SHIST.yAxisLabels[i].label, 
      strlen(SHIST.yAxisLabels[i].label));
    SHIST.maxYLabelWidth = MAX(SHIST.maxYLabelWidth, SHIST.yAxisLabels[i].width);
  }

  values[i] = MAX(SHIST.maxCount, SHIST.minCount + i * labelInc);
  sprintf(SHIST.yAxisLabels[i].label, "%d", values[i]);
  SHIST.yAxisLabels[i].width = XTextWidth(SHIST.axis.font, SHIST.yAxisLabels[i].label,
    strlen(SHIST.yAxisLabels[i].label));
  SHIST.maxYLabelWidth = MAX(SHIST.maxYLabelWidth, SHIST.yAxisLabels[i].width);

  SHIST.yMax = values[SHIST.yTics-1];

  startX = SHIST.drawArea.x + SHIST.maxYLabelWidth + SHIST.margin;

  for(i=0;i<SHIST.yTics;i++)
  {
    SHIST.yAxisLabels[i].pos.y = SHIST.drawArea.y + height - 
      (SCALEY(values[i]) - SCALEY(SHIST.minCount));
    SHIST.yAxisLabels[i].pos.x = startX - (SHIST.yAxisLabels[i].width + SHIST.margin);
  }
}

static void RedoXAxisLabels(_w)
ShistogramWidget _w;
{
  int i, width, startX;
  int value;
  double labelInc;

  width = SHIST.drawArea.width - (SHIST.maxYLabelWidth + AXIS_DIM + SHIST.margin) -
    SHIST.maxXLabelWidth/2;
  SHIST.maxXLabelWidth = SHIST.charCount * ABS(SHIST.axis.font->max_bounds.width);
  SHIST.xTics = (width - (SHIST.maxXLabelWidth*3/2))/(SHIST.maxXLabelWidth*3/2);

  if (SHIST.xTics < 2)
  {
    SHIST.xTics = 0;
    return;
  }

  SHIST.xTics = MIN(SHIST.xTics, MAXLABELS);
#if 0
  SHIST.xTics = (SHIST.xTics == 2 ? 2 : SHIST.xTics - 1);
#endif

  labelInc = (double)(SHIST.maximum - SHIST.minimum + 1.0)/(double)(SHIST.xTics);

  startX = SHIST.drawArea.x + SHIST.maxYLabelWidth + AXIS_DIM + SHIST.margin;

  for(i=0;i<SHIST.xTics-1;i++)
  {
    value = SHIST.minimum + i * labelInc;
    sprintf(SHIST.xAxisLabels[i].label, "%d", value);
    SHIST.xAxisLabels[i].width = XTextWidth(SHIST.axis.font, SHIST.xAxisLabels[i].label,
      strlen(SHIST.xAxisLabels[i].label));
    SHIST.xAxisLabels[i].pos.y = SHIST.drawArea.y + SHIST.drawArea.height - 
      (SHIST.margin + AXIS_DIM + SHIST.axis.height) + SHIST.axis.font->ascent;
    SHIST.xAxisLabels[i].pos.x = startX - SCALEX(SHIST.minimum) + SCALEX(value) + ceil((double)(SCALEX(1.0)))/2
    - SHIST.xAxisLabels[i].width/2;
  }

  value = MAX(SHIST.maximum, SHIST.minimum + i * labelInc);
  sprintf(SHIST.xAxisLabels[i].label, "%d", value);
  SHIST.xAxisLabels[i].width = XTextWidth(SHIST.axis.font, SHIST.xAxisLabels[i].label,
    strlen(SHIST.xAxisLabels[i].label));
  SHIST.xAxisLabels[i].pos.y = SHIST.drawArea.y + SHIST.drawArea.height -
    (SHIST.margin + AXIS_DIM + SHIST.axis.height) + SHIST.axis.font->ascent;
  SHIST.xAxisLabels[i].pos.x = startX - SCALEX(SHIST.minimum) + SCALEX(value) + ceil((double)(SCALEX(1.0)))/2
    - SHIST.xAxisLabels[i].width/2;
}
  
static void LabelExtents(_ls)
LabelStruct *_ls;
{  
  _ls->height = _ls->font->ascent + _ls->font->descent;
  if (_ls->label)
    _ls->width = XTextWidth(_ls->font, _ls->label, strlen(_ls->label));
  else 
    _ls->width = 0;
}

static void Resize(_w)
ShistogramWidget _w;
{
  int needDim;
  int xscale, yscale;

  SHIST.drawArea.x = SHIST.margin;
  SHIST.drawArea.y = SHIST.margin;
  SHIST.drawArea.width = _w->core.width - 2 * SHIST.margin;
  SHIST.drawArea.height = _w->core.height -  2 * SHIST.margin;

  PositionLabel(_w, &SHIST.title);
  if (SHIST.showStatistics)
    PositionLabel(_w, &SHIST.statistics);

  RedoYAxisLabels(_w);
  RedoXAxisLabels(_w);
}

static void Destroy(_w)
ShistogramWidget _w;
{
  int i;
  XtReleaseGC((Widget)_w, SHIST.gc);
  XtReleaseGC((Widget)_w, SHIST.axis.gc);
  XtReleaseGC((Widget)_w, SHIST.eraseGC);
  XtReleaseGC((Widget)_w, SHIST.title.gc);
  XtReleaseGC((Widget)_w, SHIST.statistics.gc);
  XFreeFont(XtDisplay(_w), SHIST.axis.font);
  XFreeFont(XtDisplay(_w), SHIST.title.font);
  XFreeFont(XtDisplay(_w), SHIST.statistics.font);
  XtRemoveAllCallbacks((Widget)_w, XtNselectCallback);
  for(i=0;i<MAXLABELS;i++)
  {
    XtFree(SHIST.yAxisLabels[i].label);
    XtFree(SHIST.xAxisLabels[i].label);
  }
}

static void DrawLabel(_w, _gc, _ls)
ShistogramWidget _w;
GC _gc;
LabelStruct *_ls;
{
  XDrawString(XtDisplay(_w), XtWindow(_w), _gc,
    _ls->pos.x, _ls->pos.y,
    _ls->label, strlen(_ls->label));
}

static void DrawYAxisLabels(_w)
ShistogramWidget _w;
{
  int i;
  for(i=0;i<SHIST.yTics;i++)
    DrawLabel(_w, SHIST.axis.gc, &SHIST.yAxisLabels[i]); 
}

static void DrawXAxisLabels(_w)
ShistogramWidget _w;
{
  int i;
  for(i=0;i<SHIST.xTics;i++)
    DrawLabel(_w, SHIST.axis.gc, &SHIST.xAxisLabels[i]);
}

static void DrawYAxis(_w)
ShistogramWidget _w;
{
  XSegment tics[MAXLABELS];
  int i;
  int x;

  x = SHIST.drawArea.x + SHIST.maxYLabelWidth + AXIS_DIM + SHIST.margin;
  XDrawLine(XtDisplay(_w), XtWindow(_w), SHIST.axis.gc,
    x,
    SHIST.yAxisLabels[SHIST.yTics-1].pos.y - SHIST.axis.font->ascent/2,
    x,
    SHIST.yAxisLabels[0].pos.y - SHIST.axis.font->ascent/2);

  for(i=0;i<SHIST.yTics;i++)
  {
    (*(tics+i)).x1 =  x - TIC_WIDTH;
    (*(tics+i)).x2 =  x;
    (*(tics+i)).y1 = (*(tics+i)).y2 = SHIST.yAxisLabels[i].pos.y - SHIST.axis.font->ascent/2;
  }

  XDrawSegments(XtDisplay(_w), XtWindow(_w), SHIST.axis.gc,
    tics, SHIST.yTics);

}

static void DrawXAxis(_w)
ShistogramWidget _w;
{
  XSegment tics[MAXLABELS];
  int i;
  int x;
  int y = SHIST.yAxisLabels[0].pos.y - SHIST.axis.font->ascent/2;

  x = SHIST.drawArea.x + SHIST.maxYLabelWidth + AXIS_DIM + SHIST.margin;
  XDrawLine(XtDisplay(_w), XtWindow(_w), SHIST.axis.gc,
    x,
    y,
    SHIST.drawArea.x + SHIST.drawArea.width,
    y);

  for(i=0;i<SHIST.xTics;i++)
  {
    (*(tics+i)).x1 = (*(tics+i)).x2 = SHIST.xAxisLabels[i].pos.x + SHIST.xAxisLabels[i].width/2;
    (*(tics+i)).y1 = y+1;
    (*(tics+i)).y2 = y + TIC_LENGTH;
  }

  XDrawSegments(XtDisplay(_w), XtWindow(_w), SHIST.axis.gc,
    tics, SHIST.xTics);
}

static char StaticStatLabel[50];

static void DrawStats(_w)
ShistogramWidget _w;
{
  sprintf(StaticStatLabel, "N: %d, Mean: %f, Variance: %f", 
    SHIST.number, SHIST.mean, SHIST.variance);
  SHIST.statistics.label = StaticStatLabel;
  SHIST.statistics.width = XTextWidth(SHIST.statistics.font, StaticStatLabel,
    strlen(StaticStatLabel));
  SHIST.statistics.pos.x = SHIST.drawArea.x + (SHIST.drawArea.width/2) 
    - (SHIST.statistics.width)/2;
  DrawLabel(_w, SHIST.statistics.gc, &SHIST.statistics);
}

static void DrawBars(_w)
ShistogramWidget _w;
{
  int width, height;
  int startX, startY;
  int endX, endY;
  int i;
  int x, y;

  if (SHIST.number == 0)
    return;

  startX = SHIST.drawArea.x + SHIST.maxYLabelWidth + AXIS_DIM + SHIST.margin;
  endX = startX + SHIST.drawArea.height;
  startY = SHIST.yAxisLabels[0].pos.y - SHIST.axis.height/2;
  endY = SHIST.yAxisLabels[SHIST.yTics-1].pos.y - SHIST.axis.height/2;

  width = endX - startX + 1;
  height = startY - endY + 1;

  x = (int)SCALEX(SHIST.minimum);
  for(i=SHIST.minimum;i<=SHIST.maximum;i++)
    if (*(SHIST.count+i))
    {
      y = SCALEY(*(SHIST.count+i)-SHIST.minCount);
      XFillRectangle(XtDisplay(_w), XtWindow(_w), SHIST.gc,
        startX + ((int)SCALEX(i) - x),
        startY - y + 1,
        (int)ceil((double)(SCALEX(1))),
        y);
    }
}

/* 
** This is just my way of showing that the widget is too small to draw.
*/

static void DrawTooSmall(_w)
ShistogramWidget _w;
{
  int x1,x2,y1,y2;
  XDrawLine(XtDisplay(_w), XtWindow(_w), SHIST.axis.gc,
    _w->core.width/2,
    0,
    _w->core.width/2,
    _w->core.height);
  XDrawLine(XtDisplay(_w), XtWindow(_w), SHIST.axis.gc,
    0,
    _w->core.height/2,
    _w->core.width,
    _w->core.height/2); 
}

/* 
** Widget is a pain to draw, so instead of searching regions for
** the affected area, I'll just redraw the whole thing.  Oh, leave
** me alone, if you don't like it you change it.
*/

static void Redisplay(_w)
ShistogramWidget _w;
{
  if (_w->core.visible)
  {
    XClearWindow(XtDisplay(_w), XtWindow(_w));

    if (!SHIST.xTics || !SHIST.yTics)
    {
      DrawTooSmall(_w);
      return;
    }

    if (SHIST.title.label != NULL)
      DrawLabel(_w, SHIST.title.gc, &SHIST.title);

    if (SHIST.showStatistics)
      DrawStats(_w);

    DrawBars(_w);

    if (SHIST.yAxis)
    {
      DrawYAxisLabels(_w);
      DrawYAxis(_w);
    }
    if (SHIST.xAxis)
    {
      DrawXAxisLabels(_w);
      DrawXAxis(_w);
    }  
    XFlush(XtDisplay(_w));
  }
}

#undef SHIST

#define SHIST _new->shistogram
#define CORE _new->core
#define OLDS _current->shistogram
#define OLDC _current->core


static Boolean SetValues(_current, _request, _new)
ShistogramWidget _current;
ShistogramWidget _request;
ShistogramWidget _new;
{
  Boolean redisplay = FALSE;
  Boolean reaxis = FALSE, retitle = FALSE, newtitle = FALSE, restatistics = FALSE;
  Display *display;
  XGCValues gcValues;

  display = XtDisplay(_new);

  if (SHIST.axis.font->fid != OLDS.axis.font->fid)
  {
    if (SHIST.axis.font == (XFontStruct *)NULL)
    {
      if ((SHIST.axis.font = XLoadQueryFont(display, "fixed")) == NULL)
        if ((SHIST.axis.font = XLoadQueryFont(display, "9x15")) == NULL)
          MyXtWarning("ShistogramWidget: Fonts %s and %s not found.", "fixed", "9x15");
    }
    redisplay = TRUE;
    reaxis = TRUE;
  }

  if (SHIST.title.font->fid != OLDS.title.font->fid)
  {
    if (SHIST.title.font == (XFontStruct *)NULL)
      if ((SHIST.title.font = XLoadQueryFont(display, "fixed")) == NULL)
        if ((SHIST.title.font = XLoadQueryFont(display, "9x15")) == NULL)
          MyXtWarning("ShistogramWidget: Fonts %s and %s not found.", "fixed", "9x15");
    retitle = TRUE;
    redisplay = TRUE;
  }

  if (SHIST.statistics.font->fid != OLDS.title.font->fid)
  {
    if (SHIST.statistics.font == (XFontStruct *)NULL)
    {
      if ((SHIST.statistics.font = XLoadQueryFont(display, "fixed")) == NULL)
        if ((SHIST.statistics.font = XLoadQueryFont(display, "9x15")) == NULL)
          MyXtWarning("ShistogramWidget: Fonts %s and %s not found.", "fixed", "9x15");
      LabelExtents(&SHIST.statistics);
    }
    restatistics = TRUE;
    redisplay = TRUE; 
  }

  if (SHIST.margin < 0)
  {
    MyXtWarning("ShistogramWidget: Margin (%d) cannot be negative.  Margin set to 0.", SHIST.margin);
    SHIST.margin = 0;
    redisplay = TRUE;
  }

  if (strcmp(SHIST.title.label, OLDS.title.label))
  {
    XtFree(OLDS.title.label);
    newtitle = TRUE;
  }

  if (SHIST.foreground != OLDS.foreground)
  {
    XtReleaseGC((Widget)_new, SHIST.gc);
    gcValues.foreground = SHIST.foreground;
    SHIST.gc = XtGetGC((Widget)_new, GCForeground, &gcValues);
  }

  if (CORE.background_pixel != OLDC.background_pixel)
  {
    XtReleaseGC((Widget)_new, SHIST.eraseGC);
    gcValues.foreground = CORE.background_pixel;
    gcValues.background = CORE.background_pixel;
    SHIST.eraseGC = XtGetGC((Widget)_new, GCForeground | GCBackground, &gcValues);
  }

  if (SHIST.axis.foreground != OLDS.axis.foreground || reaxis)
  {
    XtReleaseGC((Widget)_new, SHIST.axis.gc);
    gcValues.foreground = SHIST.axis.foreground;
    gcValues.font = SHIST.axis.font->fid;
    SHIST.axis.gc = XtGetGC((Widget)_new, GCFont | GCForeground, &gcValues);
    SHIST.axis.label = 0;
    LabelExtents(&SHIST.axis);
    FindMaxAxisLabelWidth(_new);
  }

  if (SHIST.statistics.foreground != OLDS.statistics.foreground || restatistics)
  {
    XtReleaseGC((Widget)_new, SHIST.statistics.gc);
    gcValues.foreground = SHIST.statistics.foreground;
    gcValues.font = SHIST.statistics.font->fid;
    SHIST.statistics.gc = XtGetGC((Widget)_new, GCFont | GCForeground, &gcValues);
    SHIST.statistics.label = 0;
    LabelExtents(&SHIST.statistics);
  }

  if (SHIST.title.foreground != OLDS.title.foreground || retitle || newtitle)
  {
    XtReleaseGC((Widget)_new, SHIST.title.gc);
    gcValues.foreground = SHIST.title.foreground;
    gcValues.font = SHIST.title.font->fid;
    SHIST.title.gc = XtGetGC((Widget)_new, GCFont | GCForeground, &gcValues);
    LabelExtents(&SHIST.title);
  }

  if (retitle)
    XFreeFont(display, OLDS.title.font);

  if (reaxis)
    XFreeFont(display, OLDS.axis.font);

  if (restatistics)
    XFreeFont(display, OLDS.statistics.font);

  return redisplay;
}

#ifdef MAXSTRING
#undef MAXSTRING
#endif

#define MAXSTRING 300

static void MyXtWarning(_format, va_alist)
char *_format;
va_dcl   /* stupid define already has a ; on it */
{
  va_list parms;
  char dest[MAXSTRING];

  va_start(parms);
  vsprintf(dest, _format, parms);
  va_end(parms);

  XtWarning(dest);
}

/* External routines */

#define CSHIST ((ShistogramWidget)_w)->shistogram

void ShistogramSetData(_w, _data, _count)
Widget _w;
unsigned char *_data;
int _count;
{
  int i;
  int total = 0;
  int total2 = 0;
  int val;

  if (!XtIsShistogram(_w))
  {
    MyXtWarning("ShistogramSetData sent a non-Shistogram widget!");
    return;
  }
 
  CSHIST.number = _count;

  for(i=0;i<MAXCOUNT;i++)
    *(CSHIST.count+i) = 0;

  for(i=0;i<CSHIST.number;i++)
  {
    val = *(_data+i);
    (*(CSHIST.count+val))++;
    total += val;
    total2 += SQR(val);
  }

  CSHIST.minimum = CSHIST.maximum = *_data;

  for(i=1;i<CSHIST.number;i++)
  {
    CSHIST.minimum = MIN(CSHIST.minimum, (int)*(_data+i));
    CSHIST.maximum = MAX(CSHIST.maximum, (int)*(_data+i));
  }

  if (CSHIST.minimum == CSHIST.maximum)
    CSHIST.minimum = 0;

  CSHIST.minCount = CSHIST.maxCount = *(CSHIST.count);
  for(i=CSHIST.minimum;i<=CSHIST.maximum;i++)
  {
    CSHIST.minCount = MIN(CSHIST.minCount, *(CSHIST.count+i)); 
    CSHIST.maxCount = MAX(CSHIST.maxCount, *(CSHIST.count+i));
  } 

  if (CSHIST.minCount == CSHIST.maxCount)
    CSHIST.minCount = 0;

  CSHIST.mean = (double)total/(double)_count; 
  CSHIST.variance = (double)(total2 - SQR(total))/(double)_count/(double)(_count-1); 

  FindMaxAxisLabelWidth((ShistogramWidget)_w);
  Resize((ShistogramWidget)_w);
  Redisplay((ShistogramWidget)_w);
}

#undef CSHIST

static void Select(_w, _event, _argv, _argc)
ShistogramWidget _w;
XEvent *_event;
String *_argv;
int *_argc;
{
  ShistogramCallbackStruct swcs;
  swcs.minX = _w->shistogram.minimum;
  swcs.maxX = _w->shistogram.maximum;
  swcs.mean = _w->shistogram.mean;
  swcs.variance = _w->shistogram.variance;
  swcs.minY = _w->shistogram.minCount;
  swcs.maxY = _w->shistogram.maxCount;
  swcs.event = _event;
  swcs.count = _w->shistogram.count; 
  swcs.number = _w->shistogram.number;
  swcs.reason = SHISTOGRAM_SELECT;
  XtCallCallbacks((Widget)_w, XtNselectCallback, &swcs);
}
