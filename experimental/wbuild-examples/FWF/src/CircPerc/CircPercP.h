#ifndef _XmCircularPercentageWidgetP_h
#define _XmCircularPercentageWidgetP_h

#include <X11/CoreP.h>
#include "CircPerc.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* class part definition */

typedef 
  struct
    {
      int iMakeCompilerHappy ; /* dummy field */
    } XfwfCircularPercentageClassPart;

/* full class record declaration */

typedef 
  struct _XfwfCircularPercentageClassRec
    {
      CoreClassPart        core_class;
      XfwfCircularPercentageClassPart  circular_percentage_class;
    } XfwfCircularPercentageClassRec;

/* new fields for this widget */

typedef
  struct
    {

      /* resources */
      Pixel   border_color;
      Pixel   interior_color;
      Pixel   completed_color;
      int     iPercentageCompleted;

      /* private data */
      GC       gc;
    } XfwfCircularPercentagePart;

/* full instance record declaration */

typedef
  struct _XfwfCircularPercentageRec
    {
      CorePart        core;
      XfwfCircularPercentagePart  circular_percentage;
    } XfwfCircularPercentageRec;

extern XfwfCircularPercentageClassRec xfwfCircularPercentageClassRec;

#if defined(__cplusplus) || defined(c_plusplus)
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif

