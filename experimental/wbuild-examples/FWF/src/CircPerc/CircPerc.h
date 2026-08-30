#ifndef _XfwfCircularPercentage_h
#define _XfwfCircularPercentage_h

/*
#define XtNborderColor "borderColor"
#define XtCBorderColor "BorderColor"
*/

#define XtNinteriorColor "interiorColor"
#define XtCInteriorColor "InteriorColor"

#define XtNcompletedColor "completedColor"
#define XtCCompletedColor "CompletedColor"

#define XtNpercentageCompleted "percentageCompleted"
#define XtCPercentageCompleted "PercentageCompleted"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <X11/Core.h>

extern WidgetClass xfwfCircularPercentageWidgetClass;

typedef struct _XfwfCircularPercentageClassRec *XfwfCircularPercentageWidgetClass;
typedef struct _XfwfCircularPercentageRec      *XfwfCircularPercentageWidget;

#if NeedFunctionPrototypes > 0
extern void XfwfCircularPercentageSetPercentage(Widget,int);
#else
extern void XfwfCircularPercentageSetPercentage();
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif
