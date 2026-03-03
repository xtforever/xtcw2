/*****************************************************************************

	ImageSelector.h

	This file is the private include file for the Image Selector
	widget.

	October 20, 1990 by Brian Totty and Prakash Ladia.

******************************************************************************/

/*
 * Author:
 * 	Brian Totty
 * 	Department of Computer Science
 * 	University Of Illinois at Urbana-Champaign
 *	1304 West Springfield Avenue
 * 	Urbana, IL 61801
 * 
 * 	totty@cs.uiuc.edu
 * 	
 */ 

#ifndef	_IMAGE_SELECTOR_H_
#define	_IMAGE_SELECTOR_H_

#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/cursorfont.h>
#include <X11/Xaw/SimpleP.h>
#include <X11/Xaw/Simple.h>

/*---------------------------------------------------------------------------*

             R E S O U R C E    N A M E    D E F I N I T I O N S

 *---------------------------------------------------------------------------*/

#define	XtNpreferredNumRows		"preferredNumRows"
#define	XtNpreferredNumCols		"preferredNumCols"
#define	XtNnumSelectable		"numSelectable"
#define	XtNimagePadding			"imagePadding"
#define	XtNcursor			"cursor"
#define	XtNgridColor			"gridColor"
#define	XtNinfoColor			"infoColor"
#define	XtNimagePaddingColor		"imagePaddingColor"
#define	XtNselectedImagePaddingColor	"selectedImagePaddingColor"
#define	XtNshowInfo			"showInfo"
#define	XtNallowPartialCells		"allowPartialCells"
#define	XtNcomputePixmapSizes		"computePixmapSizes"
#define	XtNselectionChangeCallback	"selectionChangeCallback"
#define	XtNinfoFont			"infoFont"
#define	XtNpadPixmap			"padPixmap"
#define	XtNimages			"images"
#define	XtNnumImages			"numImages"

#define	XtNnumRows			"numRows"
#define	XtNnumCols			"numCols"
#define	XtNnumSelected			"numSelected"

/*---------------------------------------------------------------------------*

              W I D G E T    &    C L A S S    D E F I N I T I O N S

 *---------------------------------------------------------------------------*/

typedef struct _XfwfImageSelectorRec		*XfwfImageSelectorWidget;
typedef struct _XfwfImageSelectorClassRec	*XfwfImageSelectorWidgetClass;

extern WidgetClass				xfwfImageSelectorWidgetClass;

/*---------------------------------------------------------------------------*

             C A L L B A C K    R E T U R N    S T R U C T U R E

 *---------------------------------------------------------------------------*/

typedef struct _XfwfImageSelectorReturnStruct
{
	int num_selected;
	int *selected_items;
} XfwfImageSelectorReturnStruct;

/*---------------------------------------------------------------------------*

                        D A T A    S T R U C T U R E S

 *---------------------------------------------------------------------------*/

typedef struct
{
	char *label;
	Pixmap pixmap;
	Boolean selected;
	Boolean selectable;

	/*
	 * If you do not want to have to compute the pixmap widths &
	 * heights, set the XtNcomputePixmapSizes resource of the
	 * ImageSelector widget.  Computation of pixmap sizes can be
	 * a bit slow, however.
	 */

	unsigned int width;
	unsigned int height;
} XfwfImage;

/*---------------------------------------------------------------------------*

         E X P O R T E D    F U N C T I O N    D E C L A R A T I O N S

 *---------------------------------------------------------------------------*/

#if (!NeedFunctionPrototypes)

void XfwfImageSelectorGetSelectedData();

#else

void XfwfImageSelectorGetSelectedData(XfwfImageSelectorWidget isw,
	XfwfImageSelectorReturnStruct *ret);

#endif

#endif

