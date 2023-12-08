/* Generated by wbuild
 * (generator version 3.3)
 */
#ifndef XTCW_SCROLLEDCANVASP_H
#define XTCW_SCROLLEDCANVASP_H
#include <xtcw/GridboxP.h>
#include <xtcw/ScrolledCanvas.h>
_XFUNCPROTOBEGIN
typedef struct {
/* Constraint resources */
/* Private constraint variables */
int dummy;
} ScrolledCanvasConstraintPart;

typedef struct _ScrolledCanvasConstraintRec {
GridboxConstraintPart gridbox;
ScrolledCanvasConstraintPart scrolledCanvas;
} ScrolledCanvasConstraintRec;


typedef struct {
/* methods */
/* class variables */
int dummy;
} ScrolledCanvasClassPart;

typedef struct _ScrolledCanvasClassRec {
CoreClassPart core_class;
CompositeClassPart composite_class;
ConstraintClassPart constraint_class;
GridboxClassPart gridbox_class;
ScrolledCanvasClassPart scrolledCanvas_class;
} ScrolledCanvasClassRec;

typedef struct {
/* resources */
Distance  slwidth;
uint  world_width;
uint  world_height;
/* private state */
Widget  slv;
Widget  slh;
Widget  can;
canvas_draw_t * priv;
} ScrolledCanvasPart;

typedef struct _ScrolledCanvasRec {
CorePart core;
CompositePart composite;
ConstraintPart constraint;
GridboxPart gridbox;
ScrolledCanvasPart scrolledCanvas;
} ScrolledCanvasRec;

externalref ScrolledCanvasClassRec scrolledCanvasClassRec;

_XFUNCPROTOEND
#endif /* XTCW_SCROLLEDCANVASP_H */
