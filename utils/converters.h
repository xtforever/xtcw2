#ifndef CONVERTERS_H
#define CONVERTERS_H

#include <X11/IntrinsicP.h>

#ifndef  XtRDistance
#define  XtRDistance "Distance"
#endif

#ifndef  XtRStringMArray
#define  XtRStringMArray "StringMArray"
#endif

#ifndef  XtRQVar
#define  XtRQVar "QVar"
#endif

#ifndef  XtRArrayChar
#define  XtRArrayChar "ArrayChar"
#endif

#ifndef  XtRArrayInt
#define  XtRArrayInt "ArrayInt"
#endif

#ifndef XtRAnesthetic
#define XtRAnesthetic "Anesthetic"
#endif

typedef int StringMArray;
typedef String Anesthetic;
#ifndef XTCW_DISTANCE_DEFINED
#define XTCW_DISTANCE_DEFINED
typedef int Distance;
#endif
typedef int ArrayChar;
typedef int ArrayInt;
typedef int QVar;

void converters_init(void);

/* DPI and point/pixel helpers shared by the re-tex widgets.  The DPI is read
 * from the Xft.dpi X resource (default 96), matching the XtRDistance
 * converter below. */
double converters_dpi(Widget w);
double converters_dpi_from_display(Display *dpy);
double converters_pt2px(Widget w, double pt);
double converters_px2pt(Widget w, double px);

/* Convert a Distance specification ("10.5pt", "3mm", "20px", ...) to whole
 * pixels.  Returns 1 on success, 0 for a malformed specification. */
int converters_distance_from_display(Display *dpy, const char *spec, int *out);
int converters_distance_px(Widget w, const char *spec);

#include "long.h"
#include "icon.h"
#include "XCC.h"
#include "choosecol.h"
#include "strarray.h"
#include "StrToPmap.h"
#include "Pen.h"
#include "str2color.h"

#define done_bert(type, value) \
    do {\
	if (to->addr != NULL) {\
	    if (to->size < sizeof(type)) {\
	        to->size = sizeof(type);\
	        return False;\
	    }\
	    *(type*)(to->addr) = (value);\
        } else {\
	    static type static_val;\
	    static_val = (value);\
	    to->addr = (XtPointer)&static_val;\
        }\
        to->size = sizeof(type);\
        return True;\
    } while (0)

#define done_bob(type, value) \
   {                                    \
       if (toVal->addr != NULL) {       \
          if (toVal->size < sizeof(type)) {\
             toVal->size = sizeof(type);\
             return False;              \
          }                             \
          *(type*)(toVal->addr) = (value);\
       }                                \
       else {                           \
          static type static_val;       \
          static_val = (value);         \
          toVal->addr = (XPointer)&static_val;\
       }                                \
       toVal->size = sizeof(type);      \
   }


#endif
