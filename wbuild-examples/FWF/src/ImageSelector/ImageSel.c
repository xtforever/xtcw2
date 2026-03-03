/****************************************************************************

	ImageSel.c

	This file implements the Image Selector widget, a widget allowing
	the display and selection of a two dimensional array of Pixmap
	images with optional text labels.

	October 20, 1990 by Brian Totty and Prakash Ladia.
	Reimplemented April 1992 by Brian Totty

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

#include <stdio.h>
#include <math.h>

#include <Xfwf/ImageSelP.h>
#include <Xfwf/ImageSel.h>

/*===========================================================================*

           D E F I N I T I O N S    A N D    D E C L A R A T I O N S

 *===========================================================================*/

#define	FontAscent(f)	((f)->max_bounds.ascent)
#define	FontDescent(f)	((f)->max_bounds.descent)
#define	FontH(f)	(FontAscent(f) + FontDescent(f))
#define	FontW(f,s)	XTextWidth(f,s,strlen(s))
#define	FontMaxCharW(f)	((f)->max_bounds.rbearing-(f)->min_bounds.lbearing+1)

#define	TypeCalloc(s,t)	(t *)XtCalloc((s),sizeof(t))

#define	StrCopy(s)	(strcpy(TypeCalloc(strlen(s) + 1,char),(s)))

#define max(a,b)	((a) >= (b) ? (a) : (b))
#define min(a,b)	((a) < (b) ? (a) : (b))
#define abs(a)		((a) < 0 ? -(a) : (a))
#define	even(x)		((x) % 2 == 0)
#define	odd(x)		((x) % 2 == 1)
#define	sign(x)		((x) < 0 ? -1 : 1)
#define	square(x)	((x) * (x))
#define	swap(x,y)	{int t; t = x; x = y; y = t;}

/*===========================================================================*

                           L O C A L    D A T A

 *===========================================================================*/

static char pad_stipple_data[32] =
{
	85, 85, 170, 170, 85, 85, 170, 170,
	85, 85, 170, 170, 85, 85, 170, 170,
	85, 85, 170, 170, 85, 85, 170, 170,
	85, 85, 170, 170, 85, 85, 170, 170
};

/*===========================================================================*

   R E S O U R C E    T Y P E S,    F I E L D S,    A N D    D E F A U L T S

 *===========================================================================*/

#define ISFieldOffset(f)	XtOffset(XfwfImageSelectorWidget,	\
					imageSelector.f)
#define	ParamFieldOffset(f)	ISFieldOffset(parameters.f)
#define CoreFieldOffset(f)	XtOffset(XfwfImageSelectorWidget,core.f)
#define	SimpleFieldOffset(f)	XtOffset(XfwfImageSelectorWidget,simple.f)

static XtResource resources[] =
{
	{XtNpreferredNumRows,XtCValue,XtRInt,sizeof(int),
		ParamFieldOffset(preferred_num_rows),XtRString,"0"},
	{XtNpreferredNumCols,XtCValue,XtRInt,sizeof(int),
		ParamFieldOffset(preferred_num_cols),XtRString,"0"},
	{XtNnumSelectable,XtCValue,XtRInt,sizeof(int),
		ParamFieldOffset(num_selectable),XtRString,"-1"},
	{XtNimagePadding,XtCValue,XtRInt,sizeof(int),
		ParamFieldOffset(image_padding),XtRString,"7"},
	{XtNcursor,XtCCursor,XtRCursor,sizeof(Cursor),
		SimpleFieldOffset(cursor),XtRString,"left_ptr"},
	{XtNgridColor,XtCColor,XtRPixel,sizeof(Pixel),
		ParamFieldOffset(grid_color),XtRString,XtDefaultForeground},
	{XtNinfoColor,XtCColor,XtRPixel,sizeof(Pixel),
		ParamFieldOffset(info_color),XtRString,XtDefaultForeground},
	{XtNimagePaddingColor,XtCBackground,XtRPixel,sizeof(Pixel),
		ParamFieldOffset(image_padding_color),XtRString,
		XtDefaultBackground},
	{XtNselectedImagePaddingColor,XtCForeground,XtRPixel,sizeof(Pixel),
		ParamFieldOffset(selected_image_padding_color),XtRString,
		XtDefaultForeground},
	{XtNshowInfo,XtCBoolean,XtRBoolean,sizeof(Boolean),
		ParamFieldOffset(show_info),XtRString,"False"},
	{XtNallowPartialCells,XtCBoolean,XtRBoolean,sizeof(Boolean),
		ParamFieldOffset(allow_partial_cells),XtRString,"True"},
	{XtNcomputePixmapSizes,XtCBoolean,XtRBoolean,sizeof(Boolean),
		ParamFieldOffset(compute_pixmap_sizes),XtRString,"True"},
	{XtNselectionChangeCallback,XtCCallback,XtRCallback,sizeof(caddr_t),
		ParamFieldOffset(selection_change_callback),XtRCallback,NULL},
	{XtNinfoFont,XtCFont,XtRFontStruct,sizeof(XFontStruct *),
		ParamFieldOffset(info_font),XtRString,XtDefaultFont},
	{XtNpadPixmap,XtCPixmap,XtRPixmap,sizeof(Pixmap),
		ParamFieldOffset(pad_pixmap),XtRPixmap,None},
	{XtNimages,XtCValue,XtRPointer,sizeof(XtPointer),
		ParamFieldOffset(images),(XtPointer)NULL},
	{XtNnumImages,XtCValue,XtRInt,sizeof(int),
		ParamFieldOffset(num_images),0},

		/* Read Only Resources */

	{XtNnumRows,XtCReadOnly,XtRInt,sizeof(int),
		ParamFieldOffset(num_rows),XtRString,"0"},
	{XtNnumCols,XtCReadOnly,XtRInt,sizeof(int),
		ParamFieldOffset(num_cols),XtRString,"0"},
	{XtNnumSelected,XtCReadOnly,XtRInt,sizeof(int),
		ParamFieldOffset(num_selected),XtRString,"0"},
};

#undef ISFieldOffset
#undef ParamFieldOffset
#undef CoreFieldOffset
#undef SimpleFieldOffset

/*===========================================================================*

                F U N C T I O N    D E C L A R A T I O N S

 *===========================================================================*/

#if (!NeedFunctionPrototypes)

	/* Widget Methods */

static void		Initialize();
static void		Realize();
static void		Destroy();
static void		Resize();
static Boolean		SetValues();
static XtGeometryResult	QueryGeometry();
static void		Redisplay();

#else

static void		Initialize(XfwfImageSelectorWidget old,
				XfwfImageSelectorWidget new);
static void		Realize(XfwfImageSelectorWidget isw,
				XtValueMask *requested_window_gc_value_mask,
				XSetWindowAttributes *requested_window_attrs);
static void		Destroy(Widget gw);
static void		Resize(XfwfImageSelectorWidget isw);
static Boolean		SetValues(XfwfImageSelectorWidget c_isw,
				XfwfImageSelectorWidget r_isw,
				XfwfImageSelectorWidget n_isw);
static XtGeometryResult	QueryGeometry(XfwfImageSelectorWidget isw,
				XtWidgetGeometry *parent_idea,
				XtWidgetGeometry *our_idea);
static void		Redisplay(XfwfImageSelectorWidget isw, XEvent *event,
				Region region);

#endif

	/* Internal Functions */

#if (!NeedFunctionPrototypes)

static void	ActionStartRegionSelection();
static void	ActionExtendRegionSelection();
static void	ActionEndRegionSelection();
static void	UpdateAllGraphicContexts();
static void	UpdateSelectionStuff();
static void	UpdateDataSizes();
static void	UpdateBounds();
static Boolean	Layout();
static Boolean	CvtCellToIndex();
static Boolean	CvtIndexToCell();
static Boolean	CvtPixelToCell();
static Boolean	CvtCellToPixel();
static void	RedrawRegion();
static void	RedrawCellRegion();
static void	RedrawCell();
static void	RedrawGaps();
static void	SelectRegion();
static void	ToggleRegion();
static void	UnselectAll();
static void	DrawSelectionBox();
static Pixmap	PixmapCreate();
static GC	GCCreate();
static void	GCFree();
static void	NotifySelectionChange();

#else

static void	ActionStartRegionSelection(XfwfImageSelectorWidget isw,
			XButtonEvent *event, char *params,
			Cardinal *num_params);
static void	ActionExtendRegionSelection(XfwfImageSelectorWidget isw,
			XMotionEvent *event, char *params,
			Cardinal *num_params);
static void	ActionEndRegionSelection(XfwfImageSelectorWidget isw,
			XMotionEvent *event, char *params,
			Cardinal *num_params);
static void	UpdateAllGraphicContexts(XfwfImageSelectorWidget isw);
static void	UpdateSelectionStuff(XfwfImageSelectorWidget isw);
static void	UpdateDataSizes(XfwfImageSelectorWidget isw);
static void	UpdateBounds(XfwfImageSelectorWidget isw);
static Boolean	Layout(XfwfImageSelectorWidget isw,
			int w_changeable, int h_changeable,
			Dimension *new_w_ptr, Dimension *new_h_ptr);
static Boolean	CvtCellToIndex(XfwfImageSelectorWidget isw, int x, int y,
			int *index_ptr);
static Boolean	CvtIndexToCell(XfwfImageSelectorWidget isw, int index,
			int *x_ptr, int *y_ptr);
static Boolean	CvtPixelToCell(XfwfImageSelectorWidget isw, int px, int py,
			int *cellx, int *celly);
static Boolean	CvtCellToPixel(XfwfImageSelectorWidget isw,
			int cellx, int celly, int *px, int *py);
static void	RedrawRegion(XfwfImageSelectorWidget isw, Box *box);
static void	RedrawCellRegion(XfwfImageSelectorWidget isw, Box *cell_box);
static void	RedrawCell(XfwfImageSelectorWidget isw, int cx, int cy);
static void	RedrawGaps(XfwfImageSelectorWidget isw);
static void	SelectRegion(XfwfImageSelectorWidget isw, Box *cell_box);
static void	ToggleRegion(XfwfImageSelectorWidget isw, Box *cell_box);
static void	UnselectAll(XfwfImageSelectorWidget isw);
static void	DrawSelectionBox(XfwfImageSelectorWidget isw);
static Pixmap	PixmapCreate(Display *display, Drawable d, char *data,
			unsigned int width, unsigned int height,
			Pixel fg, Pixel bg);
static GC	GCCreate(Display *display, Drawable d, Pixel fg, Pixel bg,
			int line_style, int line_width,
			int fill_type, Pixmap fill_pixmap, Font font);
static void	GCFree(Display *display, GC gc);
static void	NotifySelectionChange(XfwfImageSelectorWidget isw);

#endif

/*===========================================================================*

        A C T I O N    A N D    T R A N S L A T I O N    T A B L E S

 *===========================================================================*/

static char defaultTranslations[] =
"	<Btn1Down>:			StartRegionSelection()\n	\
	Button1 <Btn1Motion>:		ExtendRegionSelection()\n	\
	<Btn1Up>:			EndRegionSelection()\n";

static XtActionsRec actions[] =
{
	{"StartRegionSelection",  (XtActionProc)ActionStartRegionSelection},
	{"ExtendRegionSelection", (XtActionProc)ActionExtendRegionSelection},
	{"EndRegionSelection",    (XtActionProc)ActionEndRegionSelection},
	{NULL,                    (XtActionProc)NULL}
};

/*===========================================================================*

                 S T A T I C    A L L O C A T I O N    A N D

        I N I T I A L I Z A T I O N    O F    C L A S S    R E C O R D

 *===========================================================================*/

#define	SUPERCLASS ((WidgetClass)(&simpleClassRec))

XfwfImageSelectorClassRec xfwfImageSelectorClassRec =
{
	{
		/* superclass		*/	SUPERCLASS,
		/* class_name		*/	"XfwfImageSelector",
		/* widget_size		*/	sizeof(XfwfImageSelectorRec),
		/* class_initialize	*/	NULL,
		/* class_part_initialize*/	NULL,
		/* class_inited		*/	FALSE,
		/* initialize		*/	(XtInitProc)Initialize,
		/* initialize_hook	*/	NULL,
		/* realize		*/	(XtRealizeProc)Realize,
		/* actions		*/	actions,
		/* num_actions		*/	XtNumber(actions),
		/* resources		*/	resources,
		/* resource_count	*/	XtNumber(resources),
		/* xrm_class		*/	NULLQUARK,
		/* compress_motion	*/	TRUE,
		/* compress_exposure	*/	TRUE,
		/* compress_enterleave	*/	TRUE,
		/* visible_interest	*/	FALSE,
		/* destroy		*/	(XtWidgetProc)Destroy,
		/* resize		*/	(XtWidgetProc)Resize,
		/* expose		*/	(XtExposeProc)Redisplay,
		/* set_values		*/	(XtSetValuesFunc)SetValues,
		/* set_values_hook	*/	NULL,
		/* set_values_almost	*/	XtInheritSetValuesAlmost,
		/* get_values_hook	*/	NULL,
		/* accept_focus		*/	NULL,
		/* version		*/	XtVersion,
		/* callback_private	*/	NULL,
		/* tm_table		*/	defaultTranslations,
		/* query_geometry       */	(XtGeometryHandler)
							QueryGeometry,
		/* display_accelerator  */	XtInheritDisplayAccelerator,
		/* extension            */	NULL
	}, /* Core Part */
	{
		/* change_sensitive     */	XtInheritChangeSensitive
	}, /* Simple Part */
	{
		/* dummy variable	*/	0
	}  /* ImageSelector Part */
};

WidgetClass xfwfImageSelectorWidgetClass =
	(WidgetClass)&xfwfImageSelectorClassRec;

/*===========================================================================*

                      T O O L K I T    M E T H O D S

 *===========================================================================*/

/* ARGSUSED */
static void Initialize(old,new)
XfwfImageSelectorWidget old,new;
{
	DisplayData *dd;
	SelectorState *ss;
	Parameters *params;

	dd = ISPartDisplayData(new);
	ss = ISPartSelectorState(new);
	params = ISPartParams(new);

		/* Cache Graphic Values */

	DDDisplay(dd) = XtDisplay(new);
	DDScreen(dd) = XtScreen(new);
	DDScreenNumber(dd) = XScreenNumberOfScreen(DDScreen(dd));
	DDRoot(dd) = XRootWindow(DDDisplay(dd),DDScreenNumber(dd));
	DDVisual(dd) = XDefaultVisual(DDDisplay(dd),DDScreenNumber(dd));
	DDCells(dd) = XDisplayCells(DDDisplay(dd),DDScreenNumber(dd));
	DDWhite(dd) = XWhitePixel(DDDisplay(dd),DDScreenNumber(dd));
	DDBlack(dd) = XBlackPixel(DDDisplay(dd),DDScreenNumber(dd));
	DDCmap(dd) = XDefaultColormap(DDDisplay(dd),DDScreenNumber(dd));

		/* Initialize Internal State */

	SSImagePaddingGC(ss) = None;
	SSSelectedImagePaddingGC(ss) = None;
	SSPixmapGC(ss) = None;
	SSSelectedPixmapGC(ss) = None;
	SSInfoGC(ss) = None;
	SSGridGC(ss) = None;
	SSPadGC(ss) = None;
	SSRegionSelectGC(ss) = None;
	SSEraseGC(ss) = None;

	SSWindowMapped(ss) = False;

	if (ParamsPadPixmap(params) == None)
	{
		ParamsPadPixmap(params) = PixmapCreate(DDDisplay(dd),
			DDRoot(dd),pad_stipple_data,16,16,
			DDBlack(dd),DDWhite(dd));
	}

		/* Compute Derived State */

	if (ParamsComputePixmapSizes(params)) UpdateDataSizes(new);
	UpdateBounds(new);
	Layout(new,(Core(new)->width == 0),(Core(new)->height == 0),
	       (Dimension *)NULL,(Dimension *)NULL);
} /* End Initialize */


/*---------------------------------------------------------------------------*

	static void Realize()

	This function is called to realize a ImageSelector widget.

 *---------------------------------------------------------------------------*/

static void Realize(isw,requested_window_gc_value_mask,requested_window_attrs)
XfwfImageSelectorWidget isw;
XtValueMask *requested_window_gc_value_mask;
XSetWindowAttributes *requested_window_attrs;
{
	Window w;
	Widget widget;
	SelectorState *ss;
	Parameters *params;
	DisplayData *dd;

	widget = (Widget)isw;
	*requested_window_gc_value_mask |= CWEventMask;
	requested_window_attrs->event_mask |= PointerMotionMask;
	XtCreateWindow((Widget)isw,InputOutput,(Visual *)CopyFromParent,
		*requested_window_gc_value_mask,requested_window_attrs);
	w = XtWindow(isw);

	ss = ISPartSelectorState(isw);
	params = ISPartParams(isw);
	dd = ISPartDisplayData(isw);

	UpdateAllGraphicContexts(isw);
} /* End Realize */


/* ARGSUSED */
static void Destroy(gw)
Widget gw;
{
} /* End Destroy */


static void Resize(isw)
XfwfImageSelectorWidget isw;
{
	Layout(isw,False,False,(Dimension *)NULL,(Dimension *)NULL);
} /* End Resize */


/* ARGSUSED */
static Boolean SetValues(c_isw,r_isw,n_isw)
XfwfImageSelectorWidget c_isw,r_isw,n_isw;
{
	Boolean relayout,redraw;
	Parameters *old_p,*new_p;

	relayout = redraw = False;

	old_p = ISPartParams(c_isw);
	new_p = ISPartParams(n_isw);

		/* Don't Allow Changes To Read-Only Resources */

	if (ParamsRows(old_p) != ParamsRows(new_p))
	{
		XtWarning("XfwfImageSelector: XtNrows is read-only");
		ParamsRows(new_p) = ParamsRows(old_p);
	}
	if (ParamsCols(old_p) != ParamsCols(new_p))
	{
		XtWarning("XfwfImageSelector: XtNcols is read-only");
		ParamsCols(new_p) = ParamsCols(old_p);
	}
	if (ParamsNumSelected(old_p) != ParamsNumSelected(new_p))
	{
		XtWarning("XfwfImageSelector: XtNnumSelected is read-only");
		ParamsNumSelected(new_p) = ParamsNumSelected(old_p);
	}

		/* Check For Things That Will Change Layout */
	
	if ((ParamsPreferredRows(old_p) != ParamsPreferredRows(new_p)) ||
	    (ParamsPreferredCols(old_p) != ParamsPreferredCols(new_p)) ||
	    (ParamsImagePadding(old_p) != ParamsImagePadding(new_p)) ||
	    (ParamsShowInfo(old_p) != ParamsShowInfo(new_p)) ||
	    (ParamsImages(old_p) != ParamsImages(new_p)) ||
	    (ParamsNumImages(old_p) != ParamsNumImages(new_p)) ||
	    (ParamsAllowPartialCells(old_p) != ParamsAllowPartialCells(new_p)))
	{
		relayout = redraw = True;
	}

		/* Check For Things That Will Change GC's */

	if ((CoreBGPixel(Core(c_isw)) != CoreBGPixel(Core(n_isw))) ||
	    (ParamsGridColor(old_p) != ParamsGridColor(new_p)) ||
	    (ParamsInfoColor(old_p) != ParamsInfoColor(new_p)) ||
	    (ParamsInfoFont(old_p) != ParamsInfoFont(new_p)) ||
	    (ParamsImagePadColor(old_p) !=
	     ParamsImagePadColor(new_p)) ||
	    (ParamsSelectedImagePadColor(old_p) !=
	     ParamsSelectedImagePadColor(new_p)) ||
	    (ParamsPadPixmap(old_p) != ParamsPadPixmap(new_p)) ||
	    (CoreBGPixel(Core(c_isw)) != CoreBGPixel(Core(n_isw))))
	{
		UpdateAllGraphicContexts(n_isw);
		redraw = True;
	}

		/* Check For Things That Might Change Data Sizes */

	if ((ParamsImages(old_p) != ParamsImages(new_p)) ||
	    (ParamsNumImages(old_p) != ParamsNumImages(new_p)))
	{
		if (ParamsComputePixmapSizes(new_p)) UpdateDataSizes(n_isw);
	}

		/* Check For Things That Might Change Selection Stuff */

	if ((ParamsImages(old_p) != ParamsImages(new_p)) ||
	    (ParamsNumImages(old_p) != ParamsNumImages(new_p)) ||
	    (ParamsNumSelectable(old_p) != ParamsNumSelectable(new_p)))
	{
		UpdateSelectionStuff(n_isw);
	}

	if (relayout)
	{
		UpdateBounds(n_isw);
		Layout(n_isw,True,True,(Dimension *)NULL,(Dimension *)NULL);
	}
	return(redraw);
} /* End SetValues */


static XtGeometryResult QueryGeometry(isw,parent_idea,our_idea)
XfwfImageSelectorWidget isw;
XtWidgetGeometry *parent_idea,*our_idea;
{
	Boolean parent_wants_w,parent_wants_h,we_changed_size;

	parent_wants_w = (parent_idea->request_mode) & CWWidth;
	parent_wants_h = (parent_idea->request_mode) & CWHeight;

	our_idea->request_mode = 0;
	if (!parent_wants_w && !parent_wants_h) return(XtGeometryYes);

	we_changed_size =
		Layout(isw,!parent_wants_w,!parent_wants_h,NULL,NULL);

	our_idea->request_mode |= (CWWidth | CWHeight);
	our_idea->width = Core(isw)->width;
	our_idea->height = Core(isw)->height;
	if ((parent_wants_w && (our_idea->width != parent_idea->width)) ||
	    (parent_wants_h && (our_idea->height != parent_idea->height)))
		return(XtGeometryAlmost);
	    else
		return(XtGeometryYes);
} /* End QueryGeometry */


static void Redisplay(isw,event,region)
XfwfImageSelectorWidget isw;
XEvent *event;
Region region;
{
	Box box;

	if (!XtIsRealized((Widget)isw)) return;
	if (event == NULL)
	{
		BoxX(&box) = 0;
		BoxY(&box) = 0;
		BoxW(&box) = Core(isw)->width;
		BoxH(&box) = Core(isw)->height;
	}
	    else
	{
		BoxX(&box) = event->xexpose.x;
		BoxY(&box) = event->xexpose.y;
		BoxW(&box) = event->xexpose.width + 1;
		BoxH(&box) = event->xexpose.height + 1;
	}
	RedrawRegion(isw,&box);
} /* End Redisplay */

/*===========================================================================*

                      A C T I O N    H A N D L E R S

*===========================================================================*/

/* ARGSUSED */
static void ActionStartRegionSelection(isw,event,params,num_params)
XfwfImageSelectorWidget isw;
XButtonEvent *event;
char *params;
Cardinal *num_params;
{
	if ((event->state & ShiftMask) == 0) UnselectAll(isw);

	SSRegionStartX(ISPartSelectorState(isw)) = event->x;
	SSRegionStartY(ISPartSelectorState(isw)) = event->y;
	SSRegionLastX(ISPartSelectorState(isw)) = event->x;
	SSRegionLastY(ISPartSelectorState(isw)) = event->y;
	SSRegionShifted(ISPartSelectorState(isw)) = 
		((event->state & ShiftMask) != 0);

	DrawSelectionBox(isw);			/* Draw Initial Box */
} /* End ActionStartRegionSelection */


/* ARGSUSED */
static void ActionExtendRegionSelection(isw,event,params,num_params)
XfwfImageSelectorWidget isw;
XMotionEvent *event;
char *params;
Cardinal *num_params;
{
	DrawSelectionBox(isw);			/* Erase Old Box */
	SSRegionLastX(ISPartSelectorState(isw)) = event->x;
	SSRegionLastY(ISPartSelectorState(isw)) = event->y;
	DrawSelectionBox(isw);			/* Draw New Box */
} /* End ActionExtendRegionSelection */


/* ARGSUSED */
static void ActionEndRegionSelection(isw,event,params,num_params)
XfwfImageSelectorWidget isw;
XMotionEvent *event;
char *params;
Cardinal *num_params;
{
	int x1,x2,y1,y2;
	int cx1,cx2,cy1,cy2;
	Box cell_box;

	DrawSelectionBox(isw);			/* Erase Old Box */

	SSRegionLastX(ISPartSelectorState(isw)) = event->x;
	SSRegionLastY(ISPartSelectorState(isw)) = event->y;

	x1 = min(SSRegionStartX(ISPartSelectorState(isw)),
		 SSRegionLastX(ISPartSelectorState(isw)));
	y1 = min(SSRegionStartY(ISPartSelectorState(isw)),
		 SSRegionLastY(ISPartSelectorState(isw)));
	x2 = max(SSRegionStartX(ISPartSelectorState(isw)),
		 SSRegionLastX(ISPartSelectorState(isw)));
	y2 = max(SSRegionStartY(ISPartSelectorState(isw)),
		 SSRegionLastY(ISPartSelectorState(isw)));

	CvtPixelToCell(isw,x1,y1,&cx1,&cy1);
	CvtPixelToCell(isw,x2,y2,&cx2,&cy2);

	BoxX(&cell_box) = cx1;
	BoxY(&cell_box) = cy1;
	BoxW(&cell_box) = cx2 - cx1 + 1;
	BoxH(&cell_box) = cy2 - cy1 + 1;

	if (SSRegionShifted(ISPartSelectorState(isw)) &&
	    (BoxW(&cell_box) == 1) && (BoxH(&cell_box) == 1))
		ToggleRegion(isw,&cell_box);
	    else
		SelectRegion(isw,&cell_box);
} /* End ActionEndRegionSelection */

/*===========================================================================*

                W I D G E T    H E L P E R    R O U T I N E S

 *===========================================================================*/

static void UpdateAllGraphicContexts(isw)
XfwfImageSelectorWidget isw;
{
	Widget widget;
	DisplayData *dd;
	SelectorState *ss;
	Parameters *params;

	widget = (Widget)isw;

	ss = ISPartSelectorState(isw);
	params = ISPartParams(isw);
	dd = ISPartDisplayData(isw);

		/* Free Previous Graphic Contexts */

	if (SSImagePaddingGC(ss) != None)
		GCFree(DDDisplay(dd),SSImagePaddingGC(ss));
	if (SSSelectedImagePaddingGC(ss) != None)
		GCFree(DDDisplay(dd),SSImagePaddingGC(ss));
	if (SSPixmapGC(ss) != None)
		GCFree(DDDisplay(dd),SSPixmapGC(ss));
	if (SSSelectedPixmapGC(ss) != None)
		GCFree(DDDisplay(dd),SSSelectedPixmapGC(ss));
	if (SSInfoGC(ss) != None)
		GCFree(DDDisplay(dd),SSInfoGC(ss));
	if (SSGridGC(ss) != None)
		GCFree(DDDisplay(dd),SSGridGC(ss));
	if (SSPadGC(ss) != None)
		GCFree(DDDisplay(dd),SSPadGC(ss));
	if (SSRegionSelectGC(ss) != None)
		GCFree(DDDisplay(dd),SSRegionSelectGC(ss));
	if (SSEraseGC(ss) != None)
		GCFree(DDDisplay(dd),SSEraseGC(ss));

		/* Allocate New Graphic Contexts */

	SSImagePaddingGC(ss) = GCCreate(DDDisplay(dd),
		DDRoot(dd),ParamsImagePadColor(params),
		Core(widget)->background_pixel,LineSolid,1,FillSolid,
		None,None);
	SSSelectedImagePaddingGC(ss) = GCCreate(DDDisplay(dd),
		DDRoot(dd),ParamsSelectedImagePadColor(params),
		Core(widget)->background_pixel,LineSolid,1,FillSolid,
		None,None);
	SSPixmapGC(ss) = GCCreate(DDDisplay(dd),
		DDRoot(dd),DDBlack(dd),
		Core(widget)->background_pixel,LineSolid,1,FillSolid,
		None,None);
	SSSelectedPixmapGC(ss) = GCCreate(DDDisplay(dd),
		DDRoot(dd),Core(widget)->background_pixel,
		Core(widget)->background_pixel,LineSolid,1,FillSolid,
		None,None);
	SSInfoGC(ss) = GCCreate(DDDisplay(dd),DDRoot(dd),
		ParamsInfoColor(params),Core(widget)->background_pixel,
		LineSolid,1,FillSolid,None,ParamsInfoFont(params)->fid);
	SSGridGC(ss) = GCCreate(DDDisplay(dd),DDRoot(dd),
		ParamsGridColor(params),Core(widget)->background_pixel,
		LineSolid,1,FillSolid,None,None);
	SSPadGC(ss) = GCCreate(DDDisplay(dd),DDRoot(dd),DDBlack(dd),
		Core(widget)->background_pixel,LineSolid,1,FillTiled,
		ParamsPadPixmap(params),None);
	SSRegionSelectGC(ss) = GCCreate(DDDisplay(dd),DDRoot(dd),DDBlack(dd),
		Core(widget)->background_pixel,LineSolid,2,FillSolid,
		None,None);
	XSetFunction(DDDisplay(dd),SSRegionSelectGC(ss),GXxor);
	SSEraseGC(ss) = GCCreate(DDDisplay(dd),DDRoot(dd),
		Core(widget)->background_pixel,Core(widget)->background_pixel,
		LineSolid,1,FillSolid,None,None);
} /* End UpdateAllGraphicContexts */


static void UpdateSelectionStuff(isw)
XfwfImageSelectorWidget isw;
{
	int i;
	XfwfImage *image;
	Parameters *params;
	Boolean selection_violation,selection_count_violation;

	params = ISPartParams(isw);

	selection_violation = False;
	selection_count_violation = False;

	ParamsNumSelected(params) = 0;
	for (i = 0; i < ParamsNumImages(params); i++)
	{
		image = ParamsNthImage(params,i);
		if (ImageSelected(image) && !ImageSelectable(image))
		{
			selection_violation = True;
			ImageSelected(image) = False;
		}
		if (ImageSelected(image) && ParamsNumSelectable(params) != -1)
		{
			if (ParamsNumSelected(params) ==
			    ParamsNumSelectable(params))
			{
				selection_count_violation = True;
				ImageSelected(image) = False;
			}
		}
		if (ImageSelected(image)) ++ ParamsNumSelected(params);
	}

	if (selection_violation)
		XtWarning("XfwfImageSelector: Unselectable images selected");
	if (selection_count_violation)
		XtWarning("XfwfImageSelector: Excess selected images");
} /* End UpdateSelectionStuff */


static void UpdateDataSizes(isw)
XfwfImageSelectorWidget isw;
{
	int dummy;
	Window root;
	XfwfImage *image;
	Parameters *params;
	unsigned int i,border,bit_planes;

	params = ISPartParams(isw);
	for (i = 0; i < ParamsNumImages(params); i++)
	{
		image = ParamsNthImage(params,i);
		if (ImagePixmap(image) == None)
		{
			ImageWidth(image) = 0;
			ImageHeight(image) = 0;
		}
		    else
		{
			XGetGeometry(XtDisplay(isw),ImagePixmap(image),
				&root,&dummy,&dummy,
				&(ImageWidth(image)),&(ImageHeight(image)),
				&border,&bit_planes);
		}
	}
} /* End UpdateDataSizes */	

/*===========================================================================*

                      G E O M E T R Y    L A Y O U T

 *===========================================================================*/

static void UpdateBounds(isw)
XfwfImageSelectorWidget isw;
{
	char *str;
	XfwfImage *image;
	SelectorState *ss;
	Parameters *params;
	int i,text_w,text_h;
	
	params = ISPartParams(isw);
	ss = ISPartSelectorState(isw);

		/* Get Maximum Image BBox */

	SSMaxImageWidth(ss) = 0;
	SSMaxImageHeight(ss) = 0;
	for (i = 0; i < ParamsNumImages(params); i++)
	{
		image = ParamsNthImage(params,i);
		SSMaxImageWidth(ss) =
			max(SSMaxImageWidth(ss),ImageWidth(image));
		SSMaxImageHeight(ss) =
			max(SSMaxImageHeight(ss),ImageHeight(image));
	}
	SSMaxImageWidth(ss) += ParamsImagePadding(params);
	SSMaxImageHeight(ss) += ParamsImagePadding(params);

		/* Get Maximum Info BBox */

	SSMaxInfoWidth(ss) = 0;
	SSMaxInfoHeight(ss) = 0;
	if (ParamsShowInfo(params))
	{
		for (i = 0; i < ParamsNumImages(params); i++)
		{
			image = ParamsNthImage(params,i);
			str = ImageLabel(image);
			if (str == NULL) continue;
			text_w = FontW(ParamsInfoFont(params),str) + 4;
			text_h = FontH(ParamsInfoFont(params)) + 4 + 1;
			SSMaxInfoWidth(ss) = max(SSMaxInfoWidth(ss),text_w);
			SSMaxInfoHeight(ss) = max(SSMaxInfoHeight(ss),text_h);
		}
	}

		/* Calc Cell BBox */

	SSMinCellWidth(ss) = max(SSMaxImageWidth(ss),SSMaxInfoWidth(ss)) + 2;
	SSMinCellHeight(ss) = SSMaxImageHeight(ss) + SSMaxInfoHeight(ss) + 2;

	SSRowGap(ss) = SSColGap(ss) = 0;
} /* End UpdateBounds */


static Boolean Layout(isw,w_changeable,h_changeable,new_w_ptr,new_h_ptr)
XfwfImageSelectorWidget isw;
int w_changeable,h_changeable;
Dimension *new_w_ptr,*new_h_ptr;
{
	Parameters *p;
	SelectorState *ss;
	Dimension old_w,old_h,new_w,new_h;
	char rows_specified,cols_specified;
	int max_cells_across,max_cells_down;
	int rows,cols,min_cell_w,min_cell_h,num_images;

	p = ISPartParams(isw);
	ss = ISPartSelectorState(isw);

	old_w = Core(isw)->width;
	old_h = Core(isw)->height;

	num_images = ParamsNumImages(p);

	min_cell_w = SSMinCellWidth(ss);
	min_cell_h = SSMinCellHeight(ss);

		/* Step 1: Compute Desired Rows & Cols */

	rows = ParamsPreferredRows(p);
	cols = ParamsPreferredCols(p);

	rows_specified = (rows != 0);
	cols_specified = (cols != 0);

	if (!rows_specified)			/* No Preference For Rows */
	{
		if (!cols_specified) cols = (int)ceil(sqrt((float)num_images));
		rows = (cols ? (int)ceil((float)num_images / (float)cols) : 0);
	}
	    else if (!cols_specified)		/* No Preference For Cols */
	{
		cols = (rows ? (int)ceil((float)num_images / (float)rows) : 0);
	}
	rows = max(1,rows);
	cols = max(1,cols);

		/* Step 2: Compute Desired Width & Height */

	new_w = (w_changeable ? cols * min_cell_w + 2 : old_w);
	new_h = (h_changeable ? rows * min_cell_h + 2 : old_h);

		/* Step 3: Recalculate Rows & Cols From Width & Height */

	if (ParamsAllowPartialCells(p))
	{
		max_cells_across =
			(int)ceil(((float)new_w - 1) / (float)min_cell_w);
		max_cells_down =
			(int)ceil(((float)new_h - 1) / (float)min_cell_h);
	}
	    else
	{
		max_cells_across = (new_w - 1) / min_cell_w;
		max_cells_down = (new_h - 1) / min_cell_h;
	}
	max_cells_across = max(1,max_cells_across);
	max_cells_down = max(1,max_cells_down);

	if (cols_specified)			/* We Have Cols Preference */
	{
		cols = max(1,min(max_cells_across,cols));
		rows = (int)ceil((float)num_images / (float)cols);
		rows = max(1,min(rows,max_cells_down));
	}
	    else
	{
		rows = max(1,min(max_cells_down,rows));
		cols = (int)ceil((float)num_images / (float)rows);
		cols = max(1,min(cols,max_cells_across));
	}
	
	ParamsRows(p) = rows;
	ParamsCols(p) = cols;

		/* Step 4: Recalculate Cell Sizes & Update Widget Size */

	SSCellWidth(ss) = max((new_w - 2) / ParamsCols(p),min_cell_w);
	SSCellHeight(ss) = max((new_h - 2) / ParamsRows(p),min_cell_h);

	Core(isw)->width = new_w;
	Core(isw)->height = new_h;

	BoxX(SSCellAreaBox(ss)) = 0;
	BoxY(SSCellAreaBox(ss)) = 0;
	BoxW(SSCellAreaBox(ss)) = cols * SSCellWidth(ss) + 2;
	BoxH(SSCellAreaBox(ss)) = rows * SSCellHeight(ss) + 2;

	BoxX(SSCellBox(ss)) = 0;
	BoxY(SSCellBox(ss)) = 0;
	BoxW(SSCellBox(ss)) = SSCellWidth(ss);
	BoxH(SSCellBox(ss)) = SSCellHeight(ss);

	BoxX(SSInfoBox(ss)) = 1;
	BoxY(SSInfoBox(ss)) = 1;
	BoxW(SSInfoBox(ss)) = SSCellWidth(ss) - 2;
	BoxH(SSInfoBox(ss)) = SSMaxInfoHeight(ss);

	BoxX(SSImageBox(ss)) = 1;
	BoxY(SSImageBox(ss)) = 1 + SSMaxInfoHeight(ss);
	BoxW(SSImageBox(ss)) = BoxW(SSInfoBox(ss));
	BoxH(SSImageBox(ss)) = SSCellHeight(ss) - BoxH(SSInfoBox(ss)) - 2;

	SSColGap(ss) = new_w - BoxW(SSCellAreaBox(ss));
	SSRowGap(ss) = new_h - BoxH(SSCellAreaBox(ss));

	if (new_w_ptr) *new_w_ptr = new_w;
	if (new_h_ptr) *new_h_ptr = new_h;

	if ((old_w != new_w) || (old_h != new_h))
		return(True);
	    else
		return(False);
} /* End Layout */

/*===========================================================================*

           C O O R D I N A T E    S Y S T E M    C O N V E R T E R S

 *===========================================================================*/

static Boolean CvtCellToIndex(isw,x,y,index_ptr)
XfwfImageSelectorWidget isw;
int x,y,*index_ptr;
{
	Parameters *params;
	
	params = ISPartParams(isw);
	*index_ptr = y * ParamsCols(params) + x;
	if (*index_ptr >= ParamsNumImages(params))
		return(False);
	    else
		return(True);
} /* End CvtCellToIndex */


static Boolean CvtIndexToCell(isw,index,x_ptr,y_ptr)
XfwfImageSelectorWidget isw;
int index,*x_ptr,*y_ptr;
{
	Parameters *params;
	
	params = ISPartParams(isw);
	*y_ptr = index / ParamsCols(params);
	*x_ptr = index % ParamsCols(params);
	if (index >= ParamsNumImages(params))
		return(False);
	    else
		return(True);
} /* End CvtIndexToCell */


static Boolean CvtPixelToCell(isw,px,py,cellx,celly)
XfwfImageSelectorWidget isw;
int px,py;
int *cellx,*celly;
{
	SelectorState *ss;
	Parameters *params;

	ss = ISPartSelectorState(isw);
	params = ISPartParams(isw);

	*cellx = max(min(((px - 1) / SSCellWidth(ss)),
			 ParamsCols(params) - 1),0);
	*celly = max(min(((py - 1) / SSCellHeight(ss)),
			 ParamsRows(params) - 1),0);
	return(True);
} /* End CvtPixelToCell */


static Boolean CvtCellToPixel(isw,cellx,celly,px,py)
XfwfImageSelectorWidget isw;
int cellx,celly;
int *px,*py;
{
	SelectorState *ss;
	Parameters *params;
	
	ss = ISPartSelectorState(isw);
	params = ISPartParams(isw);

	*px = cellx * SSCellWidth(ss) + 1;
	*py = celly * SSCellHeight(ss) + 1;

	return(((cellx >= 0) && (cellx < ParamsCols(params))) && 
	       ((celly >= 0) && (celly < ParamsRows(params))));
} /* End CvtCellToPixel */

/*===========================================================================*

                     R E D R A W    R O U T I N E S

 *===========================================================================*/

static void RedrawRegion(isw,box)
XfwfImageSelectorWidget isw;
Box *box;
{
	Box cell_box;
	SelectorState *ss;
	int ul_cell_x,ul_cell_y,lr_cell_x,lr_cell_y;

	ss = ISPartSelectorState(isw);
	CvtPixelToCell(isw,BoxX1(box),BoxY1(box),&ul_cell_x,&ul_cell_y);
	CvtPixelToCell(isw,BoxX2(box),BoxY2(box),&lr_cell_x,&lr_cell_y);
	BoxX(&cell_box) = ul_cell_x;
	BoxY(&cell_box) = ul_cell_y;
	BoxW(&cell_box) = lr_cell_x - ul_cell_x + 1;
	BoxH(&cell_box) = lr_cell_y - ul_cell_y + 1;

	XFillRectangle(XtDisplay(isw),XtWindow(isw),SSEraseGC(ss),
		       BoxX1(box),BoxY1(box),BoxW(box),BoxH(box));
	XDrawRectangle(XtDisplay(isw),XtWindow(isw),SSGridGC(ss),
		BoxX(SSCellAreaBox(ss)),BoxY(SSCellAreaBox(ss)),
		BoxW(SSCellAreaBox(ss)) - 1,BoxH(SSCellAreaBox(ss)) - 1);
	RedrawCellRegion(isw,&cell_box);
	RedrawGaps(isw);
} /* End RedrawRegion */


static void RedrawCellRegion(isw,cell_box)
XfwfImageSelectorWidget isw;
Box *cell_box;
{
	int cx,cy;

	for (cy = BoxY1(cell_box); cy <= BoxY2(cell_box); cy++)
	{
		for (cx = BoxX1(cell_box); cx <= BoxX2(cell_box); cx++)
		{
			RedrawCell(isw,cx,cy);
		}
	}
} /* End RedrawCellRegion */


static void RedrawCell(isw,cx,cy)
XfwfImageSelectorWidget isw;
int cx,cy;
{
	GC gc;
	char *label;
	XfwfImage *image;
	Pixmap pixmap;
	Boolean status;
	SelectorState *ss;
	Parameters *params;
	int index,pixmap_x,pixmap_y,x,y,px,py;
	unsigned int pixmap_w,pixmap_h;

	ss = ISPartSelectorState(isw);
	params = ISPartParams(isw);

	status = CvtCellToPixel(isw,cx,cy,&x,&y);
	if (status == False) return;

		/* Draw Grid Boundary Around Cell */

	XDrawRectangle(XtDisplay(isw),XtWindow(isw),SSGridGC(ss),
		x + BoxX(SSCellBox(ss)),y + BoxY(SSCellBox(ss)),
		BoxW(SSCellBox(ss)) - 1,BoxH(SSCellBox(ss)) - 1);

		/* Draw Padded Cell & No Grid If Cell Out Of Range */

	if (CvtCellToIndex(isw,cx,cy,&index) == False)
	{
		XFillRectangle(XtDisplay(isw),XtWindow(isw),SSPadGC(ss),
			x + BoxX(SSCellBox(ss)) + 1,
			y + BoxY(SSCellBox(ss)) + 1,
			BoxW(SSCellBox(ss)) - 2,BoxH(SSCellBox(ss)) - 2);
		return;
	}

		/* Draw Centered Info Text & Boundary Line If Enabled */

	image = ParamsNthImage(params,index);

	if (ParamsShowInfo(params))
	{
		XDrawLine(XtDisplay(isw),XtWindow(isw),SSGridGC(ss),
			x + BoxX1(SSCellBox(ss)),y + BoxY2(SSInfoBox(ss)),
			x + BoxX2(SSCellBox(ss)),y + BoxY2(SSInfoBox(ss)));
		label = ImageLabel(ParamsNthImage(params,index));
		if (label != NULL)
		{
			px = x + BoxX(SSInfoBox(ss)) + (BoxW(SSInfoBox(ss)) -
				FontW(ParamsInfoFont(params),label)) / 2;
			py = y + BoxY(SSInfoBox(ss)) +
				FontAscent(ParamsInfoFont(params)) +
					(BoxH(SSInfoBox(ss)) -
					 FontH(ParamsInfoFont(params))) / 2;
			XDrawString(XtDisplay(isw),XtWindow(isw),SSInfoGC(ss),
				px + BoxX(SSInfoBox(ss)),
				py + BoxY(SSInfoBox(ss)),
				label,strlen(label));
		}		
	}

		/* Draw Image Padding */

	if (ImageSelected(image))
		gc = SSSelectedImagePaddingGC(ss);
	    else
		gc = SSImagePaddingGC(ss);

	XFillRectangle(XtDisplay(isw),XtWindow(isw),gc,
		x + BoxX(SSImageBox(ss)),y + BoxY(SSImageBox(ss)),
		BoxW(SSImageBox(ss)),BoxH(SSImageBox(ss)));

		/* Draw Centered Image */

	pixmap = ImagePixmap(image);
	if (pixmap != None)
	{
		pixmap_w = ImageWidth(image);
		pixmap_h = ImageHeight(image);
		pixmap_x = x + BoxX(SSImageBox(ss)) +
			(BoxW(SSImageBox(ss)) - pixmap_w) / 2;
		pixmap_y = y + BoxY(SSImageBox(ss)) +
			(BoxH(SSImageBox(ss)) - pixmap_h) / 2;

		if (ImageSelected(image))
			gc = SSSelectedPixmapGC(ss);
		    else
			gc = SSPixmapGC(ss);

		XCopyArea(XtDisplay(isw),pixmap,XtWindow(isw),gc,0,0,
			pixmap_w,pixmap_h,pixmap_x,pixmap_y);
	}

} /* End RedrawCell */	
	

static void RedrawGaps(isw)
XfwfImageSelectorWidget isw;
{
	DisplayData *dd;
	SelectorState *ss;

	dd = ISPartDisplayData(isw);
	ss = ISPartSelectorState(isw);

	if (SSRowGap(ss) > 0)
	{
		XFillRectangle(XtDisplay(isw),XtWindow(isw),SSPadGC(ss),
			       0,Core(isw)->height - SSRowGap(ss),
			       Core(isw)->width,SSRowGap(ss));
	}
	if (SSColGap(ss) > 0)
	{
		XFillRectangle(XtDisplay(isw),XtWindow(isw),SSPadGC(ss),
			       Core(isw)->width - SSColGap(ss),0,
			       SSColGap(ss),Core(isw)->height);
	}
} /* End RedrawGaps */

/*===========================================================================*

              I M A G E    A R R A Y    M A N I P U L A T I O N

 *===========================================================================*/

static void SelectRegion(isw,cell_box)
XfwfImageSelectorWidget isw;
Box *cell_box;
{
	int x,y,index;
	XfwfImage *image;
	Boolean status;
	Box redraw_box;
	Parameters *params;
	int ul_x,ul_y,lr_x,lr_y,cells_changed;
	
	ul_x = BoxX2(cell_box) + 1;
	ul_y = BoxY2(cell_box) + 1;
	lr_x = BoxX1(cell_box) - 1;
	lr_y = BoxY1(cell_box) - 1;
	cells_changed = 0;

	params = ISPartParams(isw);
	for (y = BoxY1(cell_box); y <= BoxY2(cell_box); y++)
	{
		for (x = BoxX1(cell_box); x <= BoxX2(cell_box); x++)
		{
			status = CvtCellToIndex(isw,x,y,&index);
			if (status == False) continue;
			image = ParamsNthImage(params,index);
			if (ImageSelectable(image) &&
			    !ImageSelected(image) &&
			    ((ParamsNumSelectable(params) == -1) ||
			     (ParamsNumSelected(params) <
			      ParamsNumSelectable(params))))
			{
				ul_x = min(ul_x,x);
				ul_y = min(ul_y,y);
				lr_x = max(lr_x,x);
				lr_y = max(lr_y,y);
				++ cells_changed;
				++ ParamsNumSelected(params);
				ImageSelected(image) = True;
			}
		}
	}
	if (cells_changed > 0)
	{
		BoxX(&redraw_box) = ul_x;
		BoxY(&redraw_box) = ul_y;
		BoxW(&redraw_box) = lr_x - ul_x + 1;
		BoxH(&redraw_box) = lr_y - ul_y + 1;
		RedrawCellRegion(isw,&redraw_box);
		NotifySelectionChange(isw);
	}
} /* End SelectRegion */


static void ToggleRegion(isw,cell_box)
XfwfImageSelectorWidget isw;
Box *cell_box;
{
	int x,y,index;
	Boolean status;
	XfwfImage *image;
	SelectorState *ss;
	Parameters *params;
	
	ss = ISPartSelectorState(isw);
	params = ISPartParams(isw);

	for (y = BoxY1(cell_box); y <= BoxY2(cell_box); y++)
	{
		for (x = BoxX1(cell_box); x <= BoxX2(cell_box); x++)
		{
			status = CvtCellToIndex(isw,x,y,&index);
			if (status == False) continue;
			image = ParamsNthImage(params,index);
			if (ImageSelected(image))
			{
				ImageSelected(image) = False;
				-- ParamsNumSelected(params);
			}
			    else if (ImageSelectable(image) &&
				 ((ParamsNumSelectable(params) == -1) ||
				  (ParamsNumSelected(params) <
				   ParamsNumSelectable(params))))
			{
				ImageSelected(image) = True;
				++ ParamsNumSelected(params);
			}
		}
	}
	RedrawCellRegion(isw,cell_box);
	NotifySelectionChange(isw);
} /* End ToggleRegion */


static void UnselectAll(isw)
XfwfImageSelectorWidget isw;
{
	Box redraw_box;
	SelectorState *ss;
	Parameters *params;
	XfwfImage *image;
	int i,ul_x,ul_y,lr_x,lr_y,cell_x,cell_y,num_unselected;

	ss = ISPartSelectorState(isw);
	params = ISPartParams(isw);

	ul_x = ParamsCols(params);
	ul_y = ParamsRows(params);
	lr_x = 0;
	lr_y = 0;
	num_unselected = 0;

	for (i = 0; i < ParamsNumImages(params); i++)
	{
		image = ParamsNthImage(params,i);
		if (ImageSelected(image))
		{
			CvtIndexToCell(isw,i,&cell_x,&cell_y);
			ul_x = min(ul_x,cell_x);
			ul_y = min(ul_y,cell_y);
			lr_x = max(lr_x,cell_x);
			lr_y = max(lr_y,cell_y);
			ImageSelected(image) = False;
			++ num_unselected;
		}
	}
	ParamsNumSelected(params) = 0;
	if (num_unselected > 0)
	{
		BoxX(&redraw_box) = ul_x;
		BoxY(&redraw_box) = ul_y;
		BoxW(&redraw_box) = lr_x - ul_x + 1;
		BoxH(&redraw_box) = lr_y - ul_y + 1;
		RedrawCellRegion(isw,&redraw_box);
	}
	NotifySelectionChange(isw);
} /* End UnselectAll */

/*===========================================================================*

                        S U P P O R T    C O D E

 *===========================================================================*/

static void DrawSelectionBox(isw)
XfwfImageSelectorWidget isw;
{
	SelectorState *ss;
	int x1,y1,x2,y2,w,h;

	ss = ISPartSelectorState(isw);

	x1 = max(BoxX1(SSCellAreaBox(ss)),
		 min(SSRegionStartX(ss),SSRegionLastX(ss)));
	y1 = max(BoxY1(SSCellAreaBox(ss)),
		 min(SSRegionStartY(ss),SSRegionLastY(ss)));
	x2 = min(BoxX2(SSCellAreaBox(ss)),
		 max(SSRegionStartX(ss),SSRegionLastX(ss)));
	y2 = min(BoxY2(SSCellAreaBox(ss)),
		 max(SSRegionStartY(ss),SSRegionLastY(ss)));
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;

	XDrawRectangle(DDDisplay(ISPartDisplayData(isw)),XtWindow(isw),
		SSRegionSelectGC(ss),x1,y1,w - 1,h - 1);
} /* End DrawSelectionBox */


static Pixmap PixmapCreate(display,d,data,width,height,fg,bg)
Display *display;
Drawable d;
char *data;
unsigned int width, height;
Pixel fg,bg;
{
	Pixmap pixmap;
	int depth,screen;

	screen = XDefaultScreen(display);
	depth = XDefaultDepth(display,screen);
	pixmap = XCreatePixmapFromBitmapData(display,d,data,
		width,height,fg,bg,depth);
	return(pixmap);
} /* End PixmapCreate */


static GC GCCreate(display,d,fg,bg,line_style,line_width,fill_type,fill_pixmap,
		   font)
Display *display;
Drawable d;
Pixel fg,bg;
int line_style,line_width,fill_type;
Pixmap fill_pixmap;
Font font;
{
	XGCValues gcvals;
	int gc_mask;
	GC gc;
	gc_mask = (GCForeground | GCBackground | GCLineStyle | GCLineWidth);
	gcvals.background = bg;
	gcvals.foreground = fg;
	gcvals.line_style = line_style;
	gcvals.line_width = line_width;
	if (font != None)
	{
		gc_mask |= GCFont;
		gcvals.font = font;
	}
	if (fill_type == FillTiled)
	{
		gc_mask |= (GCTile | GCFillStyle);
		gcvals.tile = fill_pixmap;
		gcvals.fill_style = FillTiled;
	}
	else if (fill_type == FillStippled)
	{
		gc_mask |= (GCStipple | GCFillStyle);
		gcvals.stipple = fill_pixmap;
		gcvals.fill_style = FillStippled;
	}
	gc = XCreateGC(display,d,gc_mask,&gcvals);
	return(gc);
} /* End GCCreate */


static void GCFree(display,gc)
Display *display;
GC gc;
{
	XFreeGC(display,gc);
} /* End GCFree */


static void NotifySelectionChange(isw)
XfwfImageSelectorWidget isw;
{
	Parameters *params;
	int i,j,pixmapcount;
	XfwfImageSelectorReturnStruct ret;

	params = ISPartParams(isw);

	ret.num_selected = ParamsNumSelected(ISPartParams(isw));
	ret.selected_items = (int *)XtCalloc(ret.num_selected,sizeof(int));

	i = 0;
	pixmapcount = ParamsNumImages(ISPartParams(isw));
	for (j = 0; j < pixmapcount; j++)
	{
		if (ImageSelected(ParamsNthImage(params,j)))
		{
			ret.selected_items[i++] = j;
		}
	}
	
	XtCallCallbacks((Widget)isw,XtNselectionChangeCallback,
		&(ret.selected_items));
	XtFree((char *)(ret.selected_items));
} /* End NotifySelectionChange */

/*===========================================================================

	W I D G E T / A P P L I C A T I O N  I N T E R F A C E 

============================================================================*/

void XfwfImageSelectorGetSelectedData(isw,ret)
XfwfImageSelectorWidget isw;
XfwfImageSelectorReturnStruct *ret;
{
	Parameters *params;
	int i,j,pixmapcount;

	params = ISPartParams(isw);
	ret->num_selected = ParamsNumSelected(ISPartParams(isw));

	ret->selected_items = (int *)XtCalloc(ret->num_selected,sizeof(int));
	i = 0;
	pixmapcount = ParamsNumImages(ISPartParams(isw));
	for (j = 0; j < pixmapcount; j++)
	{
		if (ImageSelected(ParamsNthImage(params,j)))
		{
			(ret->selected_items)[i++] = j;
		}
	}
} /* End XfwfImageSelectorGetSelectedData */
