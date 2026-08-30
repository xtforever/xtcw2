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

#ifndef SHISTOGRAM__H
#define SHISTOGRAM__H

/*
** Simple Histogram Widget
*/

/* Default Translations:
** <Btn1Down>: select()
*/

/* Resources:                Type:         Default:              Comment:

*  XtNmargin               : integer     : 5                   : minimum margin
*  XtNselectCallback       : callback    : NULL                : callback for select action
*  XtNxAxis                : boolean     : TRUE                : show x Axis?
*  XtNyAxis                : boolean     : TRUE                : show y Axis?
*  XtNtitleFont            : XFontStruct      : XtDefaultFont       :
*  XtNstatisticsFont       : XFontStruct      : XtDefaultFont       :
*  XtNaxisFont             : XFontStruct      : XtDefaultFont       :
*  XtNtitleForeground      : pixel       : XtDefaultForeground :
*  XtNstatisticsForeground : pixel       : XtDefaultForeground :
*  XtNaxisForeground       : pixel       : XtDefaultForeground :
*  XtNforeground           : pixel       : XtDefaultForeground : for boxes
*  XtNshowTitle            : boolean     : TRUE                :
*  XtNshowStatistics       : boolean     : TRUE                :

*/

extern WidgetClass shistogramWidgetClass;
typedef struct _ShistogramClassRec *ShistogramWidgetClass;
typedef struct _ShistogramRec *ShistogramWidget;

#ifndef XtIsShistogram /* Gee...Like someone else would have already defined this! */
#define XtIsShistogram(w) XtIsSubclass((w), shistogramWidgetClass)
#endif

#define XtNselectCallback "selectCallback"
#define XtNmargin "margin"
#define XtNxAxis "xAxis"
#define XtNyAxis "yAxis"
#define XtNtitleFont "titleFont"
#define XtNaxisFont "axisFont"
#define XtNstatisticsFont "statisticsFont"
#define XtNaxisForeground "axisForeground"
#define XtNtitleForeground "titleForeground"
#define XtNstatisticsForeground "statisticsForeground"
#define XtNtitleLabel "titleLabel"
#define XtNselectCallback "selectCallback"
#define XtNshowTitle "showTitle"
#define XtNshowStatistics "showStatistics"

#ifndef XtCMargin
#define XtCMargin "Margin"
#endif
#define XtCXAxis "XAxis"
#define XtCYAxis "YAxis"
#define XtCAxisFont "AxisFont"
#define XtCTitleFont "TitleFont"
#define XtCAxisForeground "AxisForeground"
#define XtCStatisticsFont "StatisticsFont"
#define XtCTitleForeground "TitleForeground"
#define XtCStatisticsForeground "StatisticsForeground"
#define XtCTitleLabel "TitleLabel"
#define XtCShowTitle "ShowTitle"
#define XtCShowStatistics "ShowStatistics"

/* Reasons */
#define SHISTOGRAM_SELECT 1

typedef struct _ShistogramCallbackStruct
{
  int reason;
  XEvent *event;
  int minX;
  int maxX;
  int minY;
  int maxY;
  double mean;
  double variance;
  int *count;
  int number; 
} ShistogramCallbackStruct, *ShistogramCallbackPtr;

/* External Routine */

#if NeedFunctionProtoTypes
extern void ShistogramSetData(Widget _w, unsigned char *_data, int _count);
#else
extern void ShistogramSetData();
#endif

#endif /* SHISTOGRAM__H */
