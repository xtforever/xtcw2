/*
 *     a percent progress bar
 *
 *   Version   Date        Comment
 *   -------   ----        -------
 *   1.00      16-Dec-94   Original release
 *
 *  Author:
 *     Copyright (C) 1994 by Lachlan Wetherall
 *     lwether@cs.newcastle.edu.au            
 *
 *     .-_|\   Department of Computer Science
 *    /     \  University of Newcastle
 *    \.--._*  Callaghan NSW 2308
 *         v   Australia
 */


#ifndef _PcBar_h
#define _PcBar_h

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
  
/*
 * PcBar Widget public include file
 */

/*
 * This include not needed unless the application includes Intrinsic.h
 * after this file.   Anyway, it doesn't hurt.
 */

#include <X11/Core.h>

/* Resources:
 * Name		     Class		RepType		Default Value
 * ----		     -----		-------		-------------
 * ancestorSensitive
 * x		    Position		Int		0
 * y		    Position		Int		0
 * width	Dimension	Dimension	0
 * height	Dimension	Dimension	0
 * borderWidth	BorderWidth	Int
 * sensitive	Sensitive
 * screen		    Screen		Pointer		XtCopyScreen
 * depth		    Depth		Int		XtCopyFromParent
 * colormap	    Colormap	        Pointer	        XtCopyFromParent
 * background	    Background		Pixel		White
 * backgroundPixmap    Pixmap		Pixmap		XtUnspecifiedPixmap
 * borderColor	    BorderColor		Pixel		Black
 * borderPixmap	    BorderPixmap	Pixmap		XtUnspecifiedPixmap
 * mappedWhenManaged   MappedWhenManaged	Boolean		True
 * translations
 * accelerators
 *
 * (from PcBar)
 * foreground    Foreground		Pixel		XtDefaultForeground
 * percentage    Percentage		Int			0
 * displaypc     Displaypc		Boolean		False
 * showzero      Showzero		Boolean		False
 * vertical      Vertical		Boolean		False
 * font     	 Font			XFontStruct*	XtDefaultFont
 */


#define XtNpercentage "percentage"
#define XtCPercentage "Percentage"
#define XtNdisplaypc "displaypc"
#define XtCDisplaypc "Displaypc"
#define XtNshowzero "showzero"
#define XtCShowzero "Showzero"
#define XtNvertical "vertical"
#define XtCVertical "Vertical"

/* Class record constants */

extern WidgetClass xfwfPcBarWidgetClass;

typedef struct _XfwfPcBarClassRec *XfwfPcBarWidgetClass;
typedef struct _XfwfPcBarRec      *XfwfPcBarWidget;

_XFUNCPROTOBEGIN

/* User functions */
extern void XfwfPcBarSetPercentage(
#if NeedFunctionPrototypes
	Widget, int
#endif
);
_XFUNCPROTOEND

#if defined(__cplusplus) || defined(c_plusplus)
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif 
/* DON'T ADD STUFF AFTER THIS #endif */
