/* Generated by wbuild
 * (generator version 3.3)
 */
#ifndef XTCW_WBUTTONP_H
#define XTCW_WBUTTONP_H
#include <xtcw/WlabelP.h>
#include <xtcw/Wbutton.h>
_XFUNCPROTOBEGIN

typedef struct {
/* methods */
/* class variables */
int dummy;
} WbuttonClassPart;

typedef struct _WbuttonClassRec {
CoreClassPart core_class;
WheelClassPart wheel_class;
WlabelClassPart wlabel_class;
WbuttonClassPart wbutton_class;
} WbuttonClassRec;

typedef struct {
/* resources */
/* private state */
int dummy;
} WbuttonPart;

typedef struct _WbuttonRec {
CorePart core;
WheelPart wheel;
WlabelPart wlabel;
WbuttonPart wbutton;
} WbuttonRec;

externalref WbuttonClassRec wbuttonClassRec;

_XFUNCPROTOEND
#endif /* XTCW_WBUTTONP_H */
