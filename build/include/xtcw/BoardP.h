/* Generated by wbuild
 * (generator version 3.3)
 */
#ifndef XTCW_BOARDP_H
#define XTCW_BOARDP_H
#include <xtcw/FrameP.h>
#include <xtcw/Board.h>
_XFUNCPROTOBEGIN
#define MAGICNUM 12349 


typedef void (*set_abs_location_Proc)(
#if NeedFunctionPrototypes
Widget,unsigned  int ,int ,int ,int ,int 
#endif
);
#define XtInherit_set_abs_location ((set_abs_location_Proc) _XtInherit)

typedef struct {
/* methods */
set_abs_location_Proc set_abs_location;
/* class variables */
} BoardClassPart;

typedef struct _BoardClassRec {
CoreClassPart core_class;
CompositeClassPart composite_class;
CommonClassPart common_class;
FrameClassPart frame_class;
BoardClassPart board_class;
} BoardClassRec;

typedef struct {
/* resources */
Position  abs_x;
float  rel_x;
Position  abs_y;
float  rel_y;
Position  abs_width;
float  rel_width;
Position  abs_height;
float  rel_height;
float  hunit;
float  vunit;
String  location;
/* private state */
} BoardPart;

typedef struct _BoardRec {
CorePart core;
CompositePart composite;
CommonPart common;
FramePart frame;
BoardPart board;
} BoardRec;

externalref BoardClassRec boardClassRec;

_XFUNCPROTOEND
#endif /* XTCW_BOARDP_H */
