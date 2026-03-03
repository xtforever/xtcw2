/****************************************************************************

	ImageSelectorP.h

	This file is the private include file for the Image Selector
	widget.

	October 20, 1990 by Brian Totty and Prakash Ladia.

 ****************************************************************************/

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

#ifndef	_IMAGE_SELECTORP_H_
#define	_IMAGE_SELECTORP_H_

#include <Xfwf/ImageSel.h>

/*===========================================================================

                   T Y P E    D E F I N I T I O N S

  ===========================================================================*/

typedef struct
{
	int preferred_num_rows;				/* RW */
	int preferred_num_cols;				/* RW */
	int num_selectable;				/* RW */
	int image_padding;				/* RW */
	Cursor cursor;					/* RW */
	Pixel grid_color;				/* RW */
	Pixel info_color;				/* RW */
	Pixel image_padding_color;			/* RW */
	Pixel selected_image_padding_color;		/* RW */
	Boolean show_info;				/* RW */
	Boolean allow_partial_cells;			/* RW */
	Boolean compute_pixmap_sizes;			/* RW */
	XtCallbackList selection_change_callback;	/* RW */
	XFontStruct *info_font;				/* RW */
	Pixmap pad_pixmap;				/* RW */
	XfwfImage *images;				/* RW */
	int num_images;					/* RW */

	int num_rows;					/* R  */
	int num_cols;					/* R  */
	int num_selected;				/* R  */
} Parameters;

typedef	struct
{
	String name;
	int value;
} Binding;

typedef struct
{
	int x;
	int y;
	int w;
	int h;
} Box;

typedef struct
{
	int x;
	int y;
} Point;

typedef struct
{
	Display *display;
	Window root;
	Screen *screen;
	int screen_number;
	Visual *visual;
	int cells;
	Pixel white,black;
	int dynamic_color_flag;
	Colormap std_cmap;
} DisplayData;

typedef struct
{
	GC image_padding_gc;
	GC selected_image_padding_gc;
	GC pixmap_gc;
	GC selected_pixmap_gc;
	GC info_gc;
	GC grid_gc;
	GC pad_gc;
	GC region_select_gc;
	GC erase_gc;
	Boolean window_mapped;
	int max_image_width;
	int max_image_height;
	int max_info_width;
	int max_info_height;
	int min_cell_width;
	int min_cell_height;
	int cell_width;
	int cell_height;
	int row_gap;
	int col_gap;
	int region_start_x;
	int region_start_y;
	int region_last_x;
	int region_last_y;
	Boolean region_shifted;
	Box cell_area_box;
	Box cell_box;
	Box info_box;
	Box image_box;
} SelectorState;

/*===========================================================================

                             M A C R O S

  ===========================================================================*/

#define	BoxX(b)				((b)->x)
#define	BoxY(b)				((b)->y)
#define	BoxX1(b)			BoxX(b)
#define	BoxY1(b)			BoxY(b)
#define	BoxW(b)				((b)->w)
#define	BoxH(b)				((b)->h)
#define	BoxX2(b)			(BoxX1(b) + BoxW(b) - 1)
#define	BoxY2(b)			(BoxY1(b) + BoxH(b) - 1)

#define	PointX(p)			((p)->x)
#define	PointY(p)			((p)->y)

#define	ParamsPreferredRows(p)		((p)->preferred_num_rows)
#define	ParamsPreferredCols(p)		((p)->preferred_num_cols)
#define	ParamsNumSelectable(p)		((p)->num_selectable)
#define	ParamsImagePadding(p)		((p)->image_padding)
#define	ParamsCursor(p)			((p)->cursor)
#define	ParamsGridColor(p)		((p)->grid_color)
#define	ParamsInfoColor(p)		((p)->info_color)
#define	ParamsImagePadColor(p)		((p)->image_padding_color)
#define	ParamsSelectedImagePadColor(p)	((p)->selected_image_padding_color)
#define	ParamsShowInfo(p)		((p)->show_info)
#define	ParamsAllowPartialCells(p)	((p)->allow_partial_cells)
#define	ParamsAllowPartialCells(p)	((p)->allow_partial_cells)
#define	ParamsComputePixmapSizes(p)	((p)->compute_pixmap_sizes)
#define	ParamsInfoFont(p)		((p)->info_font)
#define	ParamsPadPixmap(p)		((p)->pad_pixmap)
#define	ParamsImages(p)			((p)->images)
#define	ParamsNumImages(p)		((p)->num_images)
#define	ParamsRows(p)			((p)->num_rows)
#define	ParamsCols(p)			((p)->num_cols)
#define	ParamsNumSelected(p)		((p)->num_selected)

#define	ParamsNthImage(p,n)		(&(ParamsImages(p)[n]))

#define	DDDisplay(dd)			((dd)->display)
#define	DDRoot(dd)			((dd)->root)
#define	DDScreen(dd)			((dd)->screen)
#define	DDScreenNumber(dd)		((dd)->screen_number)
#define	DDVisual(dd)			((dd)->visual)
#define	DDCells(dd)			((dd)->cells)
#define	DDWhite(dd)			((dd)->white)
#define	DDBlack(dd)			((dd)->black)
#define	DDDynamicColorFlag(dd)		((dd)->dynamic_color_flag)
#define	DDCmap(dd)			((dd)->std_cmap)

#define	ImageLabel(i)			((i)->label)
#define	ImagePixmap(i)			((i)->pixmap)
#define	ImageSelected(i)		((i)->selected)
#define	ImageSelectable(i)		((i)->selectable)
#define	ImageWidth(i)			((i)->width)
#define	ImageHeight(i)			((i)->height)

#define	SSImagePaddingGC(ss)		((ss)->image_padding_gc)
#define	SSSelectedImagePaddingGC(ss)	((ss)->selected_image_padding_gc)
#define	SSPixmapGC(ss)			((ss)->pixmap_gc)
#define	SSSelectedPixmapGC(ss)		((ss)->selected_pixmap_gc)
#define	SSInfoGC(ss)			((ss)->info_gc)
#define	SSGridGC(ss)			((ss)->grid_gc)
#define	SSPadGC(ss)			((ss)->pad_gc)
#define	SSRegionSelectGC(ss)		((ss)->region_select_gc)
#define	SSEraseGC(ss)			((ss)->erase_gc)

#define	SSWindowMapped(ss)		((ss)->window_mapped)
#define	SSMaxImageWidth(ss)		((ss)->max_image_width)
#define	SSMaxImageHeight(ss)		((ss)->max_image_height)
#define	SSMaxInfoWidth(ss)		((ss)->max_info_width)
#define	SSMaxInfoHeight(ss)		((ss)->max_info_height)
#define	SSMinCellWidth(ss)		((ss)->min_cell_width)
#define	SSMinCellHeight(ss)		((ss)->min_cell_height)
#define	SSCellWidth(ss)			((ss)->cell_width)
#define	SSCellHeight(ss)		((ss)->cell_height)
#define	SSRowGap(ss)			((ss)->row_gap)
#define	SSColGap(ss)			((ss)->col_gap)
#define	SSRegionStartX(ss)		((ss)->region_start_x)
#define	SSRegionStartY(ss)		((ss)->region_start_y)
#define	SSRegionLastX(ss)		((ss)->region_last_x)
#define	SSRegionLastY(ss)		((ss)->region_last_y)
#define	SSRegionShifted(ss)		((ss)->region_shifted)
#define	SSCellAreaBox(ss)		(&((ss)->cell_area_box))
#define	SSCellBox(ss)			(&((ss)->cell_box))
#define	SSInfoBox(ss)			(&((ss)->info_box))
#define	SSImageBox(ss)			(&((ss)->image_box))

#define	CoreBGPixel(c)			((c)->background_pixel)

#define	Core(w)				(&((w)->core))
#define	Simple(w)			(&((w)->simple))
#define	ISPart(w)			(&((w)->imageSelector))

#define	ISTitle(is)			((is)->title)
#define	ISDisplayData(is)		(&((is)->display_data))
#define	ISParams(is)			(&((is)->parameters))
#define	ISSelectorState(is)		(&((is)->selector_state))

#define	ISPartTitle(w)			(ISTitle(ISPart(w)))
#define	ISPartDisplayData(w)		(ISDisplayData(ISPart(w)))
#define	ISPartParams(w)			(ISParams(ISPart(w)))
#define	ISPartSelectorState(w)		(ISSelectorState(ISPart(w)))

/*===========================================================================*

          W I D G E T    S T R U C T U R E    D E F I N I T I O N

 *===========================================================================*/

typedef struct _XfwfImageSelectorPart
{
	String			title;
	DisplayData		display_data;
	Parameters		parameters;
	SelectorState		selector_state;
} XfwfImageSelectorPart;

typedef struct _XfwfImageSelectorClassPart
{
	int    empty;
} XfwfImageSelectorClassPart;

typedef struct _XfwfImageSelectorClassRec
{
	CoreClassPart			core_class;
	SimpleClassPart			simple_class;
	XfwfImageSelectorClassPart	imageSelector_class;
} XfwfImageSelectorClassRec;

	/* This Is What A Widget Instance Points To */

typedef struct _XfwfImageSelectorRec
{
	CorePart		core;
	SimplePart		simple;
	XfwfImageSelectorPart	imageSelector;
} XfwfImageSelectorRec;

extern XfwfImageSelectorClassRec xfwfImageSelectorClassRec;

#endif
