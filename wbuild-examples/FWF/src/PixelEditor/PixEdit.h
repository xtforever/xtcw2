/*****************************************************************************

	PixEdit.h

	This file contains is the public .h file for the Pixel Editor
	widget.

	October 14, 1990 by Brian Totty

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

#ifndef _PIXEL_EDITOR_H_
#define _PIXEL_EDITOR_H_

#define	TOOL_PENCIL		0
#define	TOOL_BRUSH		1
#define	TOOL_ERASER		2
#define	TOOL_REGION		3
#define	TOOL_PASTER		4

#define	BRUSH_SMALL_BOX		0
#define	BRUSH_MEDIUM_BOX	1
#define	BRUSH_LARGE_BOX		2
#define	BRUSH_SMALL_DIAMOND	3
#define	BRUSH_MEDIUM_DIAMOND	4
#define	BRUSH_LARGE_DIAMOND	5
#define	BRUSH_VERT_LINE		6
#define	BRUSH_HOR_LINE		7
#define	BRUSH_NEG_DIAG_LINE	8
#define	BRUSH_POS_DIAG_LINE	9
#define	BRUSH_CIRCLE		10
#define	BRUSH_USER_DEFINED	-1

/*---------------------------------------------------------------------------*

                 C O M P A T I B I L I T Y    D E F I N E S

 *---------------------------------------------------------------------------*/

#define	XtNcolor			XtNdrawColor
#define	XfwfPixelEditorGetColor		XfwfPixelEditorGetFGColor

/*---------------------------------------------------------------------------*

             R E S O U R C E    N A M E    D E F I N I T I O N S

 *---------------------------------------------------------------------------*/

#define	XtNzoom				"zoom"
#define	XtNdrawColor			"drawColor"
#define	XtNeraseColor			"eraseColor"
#define	XtNleftClipper			"leftClipper"
#define	XtNrightClipper			"rightClipper"
#define	XtNtopClipper			"topClipper"
#define	XtNbottomClipper		"bottomClipper"
#define	XtNclipperWidth			"clipperWidth"
#define	XtNclipperHeight		"clipperHeight"
#define	XtNgrid				"grid"
#define	XtNclippers			"clippers"
#define	XtNautoScale			"autoScale"
#define	XtNopaquePaste			"opaquePaste"
#define	XtNtool				"tool"
#define	XtNbrush			"brush"
#define	XtNdataChangeCallback		"dataChangeCallback"
#define	XtNregionChangeCallback		"regionChangeCallback"
#define	XtNpasteBufferChangeCallback	"pasteBufferChangeCallback"

#define	XtCTool				"Tool"
#define	XtCBrush			"Brush"
#define	XtCPixelArray			"PixelArray"

#define	XtRTool				"Tool"
#define	XtRBrush			"Brush"

#define	XtNtoolPencil			"Pencil"
#define	XtNtoolBrush			"Brush"
#define	XtNtoolEraser			"Eraser"
#define	XtNtoolRegion			"Region"
#define	XtNtoolPaster			"Paster"

#define	XtNbrushSmallBox		"SmallBox"
#define	XtNbrushMediumBox		"MediumBox"
#define	XtNbrushLargeBox		"LargeBox"
#define	XtNbrushSmallDiamond		"SmallDiamond"
#define	XtNbrushMediumDiamond		"MediumDiamond"
#define	XtNbrushLargeDiamond		"LargeDiamond"
#define	XtNbrushVertLine		"VertLine"
#define	XtNbrushHorizLine		"HorizLine"
#define	XtNbrushNegDiagLine		"NegDiagLine"
#define	XtNbrushPosDiagLine		"PosDiagLine"
#define	XtNbrushCircle			"Circle"
#define	XtNbrushUserDefined		"UserDefined"

/*---------------------------------------------------------------------------*

              W I D G E T    &    C L A S S    D E F I N I T I O N S

 *---------------------------------------------------------------------------*/

typedef struct _XfwfPixelEditorRec	*XfwfPixelEditorWidget;
typedef struct _XfwfPixelEditorClassRec	*XfwfPixelEditorWidgetClass;

extern WidgetClass			xfwfPixelEditorWidgetClass;

/*---------------------------------------------------------------------------*

         E X P O R T E D    F U N C T I O N    D E C L A R A T I O N S

 *---------------------------------------------------------------------------*/

#if (!NeedFunctionPrototypes)

Boolean		XfwfPixelEditorGetGrid();
int		XfwfPixelEditorGetZoom();
Pixel		XfwfPixelEditorGetFGColor();
Pixel		XfwfPixelEditorGetBGColor();
Boolean		XfwfPixelEditorSelectRegion();
void		XfwfPixelEditorSelectAll();
void		XfwfPixelEditorRegionFlipHorizontally();
void		XfwfPixelEditorRegionFlipVertically();
void		XfwfPixelEditorRegionFill();
void		XfwfPixelEditorRegionInvert();
Boolean		XfwfPixelEditorRegionCopy();
void		XfwfPixelEditorRegionCut();
Boolean		XfwfPixelEditorChangeTool();
Pixel *		XfwfPixelEditorGetImage();
void		XfwfPixelEditorSetImage();
void		XfwfPixelEditorResizeImage();
Pixel *		XfwfPixelEditorBitmapDataToPixels();
unsigned char *	XfwfPixelEditorPixelsToBitmapData();
char *		XfwfPixelEditorGetBitmapData();
void		XfwfPixelEditorSetBitmapData();
Boolean		XfwfPixelEditorLoadBitmapFile();
Boolean		XfwfPixelEditorSaveBitmapFile();

#else

Boolean		XfwfPixelEditorGetGrid(XfwfPixelEditorWidget pew);
int		XfwfPixelEditorGetZoom(XfwfPixelEditorWidget pew);
Pixel		XfwfPixelEditorGetFGColor(XfwfPixelEditorWidget pew);
Pixel		XfwfPixelEditorGetBGColor(XfwfPixelEditorWidget pew);
Boolean		XfwfPixelEditorSelectRegion(XfwfPixelEditorWidget pew,
			int x1, int y1, int x2, int y2);
void		XfwfPixelEditorSelectAll(XfwfPixelEditorWidget pew);
void		XfwfPixelEditorRegionFlipHorizontally(
			XfwfPixelEditorWidget pew);
void		XfwfPixelEditorRegionFlipVertically(XfwfPixelEditorWidget pew);
void		XfwfPixelEditorRegionFill(XfwfPixelEditorWidget pew,
			Pixel color);
void		XfwfPixelEditorRegionInvert(XfwfPixelEditorWidget pew,
			Pixel bg, Pixel fg);
Boolean		XfwfPixelEditorRegionCopy(XfwfPixelEditorWidget pew);
void		XfwfPixelEditorRegionCut(XfwfPixelEditorWidget pew);
Boolean		XfwfPixelEditorChangeTool(XfwfPixelEditorWidget pew, int tool);
Pixel *		XfwfPixelEditorGetImage(XfwfPixelEditorWidget pew,
			unsigned int *w_ptr, unsigned int *h_ptr);
void		XfwfPixelEditorSetImage(XfwfPixelEditorWidget pew,
			Pixel *pixels, unsigned int w, unsigned int h,
			int shared);
void		XfwfPixelEditorResizeImage(XfwfPixelEditorWidget pew,
			unsigned int w, unsigned int h);
Pixel *		XfwfPixelEditorBitmapDataToPixels(XfwfPixelEditorWidget pew,
			unsigned char *bits, unsigned int w, unsigned int h,
			Pixel fg_color, Pixel bg_color);
unsigned char *	XfwfPixelEditorPixelsToBitmapData(XfwfPixelEditorWidget pew,
			Pixel *pixels, int w, int h, Pixel bg_color);
char *		XfwfPixelEditorGetBitmapData(XfwfPixelEditorWidget pew,
			unsigned int *w_ptr, unsigned int *h_ptr, Pixel bg);
void		XfwfPixelEditorSetBitmapData(XfwfPixelEditorWidget pew,
			unsigned char *bits, unsigned int w, unsigned int h,
			Pixel fg, Pixel bg, int shared);
Boolean		XfwfPixelEditorLoadBitmapFile(XfwfPixelEditorWidget pew,
			char *filename, Pixel fg_color, Pixel bg_color,
			int *hotxp, int *hotyp);
Boolean		XfwfPixelEditorSaveBitmapFile(XfwfPixelEditorWidget pew,
			char *base_name, char *filename, Pixel bg_color,
			int hotx, int hoty);

#endif

/*---------------------------------------------------------------------------*

             C A L L B A C K    R E T U R N    S T R U C T U R E

 *---------------------------------------------------------------------------*/

typedef struct _XfwfPixelEditorReturnStruct
{
	int dummy;
} XfwfPixelEditorReturnStruct;

#endif

