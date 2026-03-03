
/* 
 * PcBarP.h - Private definitions for PcBar widget
 */

#ifndef _XfwfPcBarP_h
#define _XfwfPcBarP_h

/*
 * This include not needed unless the .c file includes IntrinsicP.h
 * after this file.   Anyway, it doesn't hurt.
 */
#include <X11/CoreP.h>

#include "PcBar.h"

/* New fields for the PcBar widget class record */

typedef struct {
	int make_compiler_happy;	/* keep compiler happy */
} XfwfPcBarClassPart;

/* Full class record declaration */
typedef struct _XfwfPcBarClassRec {
    CoreClassPart	core_class;
    XfwfPcBarClassPart	pcBar_class;
} XfwfPcBarClassRec;

extern XfwfPcBarClassRec xfwfPcBarClassRec;

/* New fields for the Pcbar widget record */
typedef struct {
    /* resources */
    Pixel	foreground;
    int percentage;
	Boolean displaypc;
	Boolean showzero;
	Boolean vertical;
    XFontStruct *font;          /* Font to draw in. */

    /* private state */
    GC		draw_gc;	
	GC		text_gc;
} XfwfPcBarPart;

/*
 * Full instance record declaration
 */
typedef struct _XfwfPcBarRec {
    CorePart		core;
    XfwfPcBarPart	pcBar;
} XfwfPcBarRec;

#endif
