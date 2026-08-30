/*****************************************************************************

	PixEditT.c

	This file serves as a simple example of the use of the PixelEditor
	widget in applications.  It creates a simple button box to hold
	several buttons to access various commands.  Many functions of the
	PixelEditor widget (e.g. brush shapes) are not used by this simple
	shell.  If you do not have my FileSelector widget, then leave the
	HAVE_FILE_SELECTOR label undefined.

	This is gradually evolving from a widget test program into a full
	application, which probably explains any ugliness.

 *****************************************************************************/

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

#include <Xfwf/PixEditT.h>

Globals G;

int main(argc,argv)
int argc;
char **argv;
{
	Arg args[10];
	int arg_count,i;

	XtSetArg(args[0],XtNallowShellResize,True);
	arg_count = 1;
	G.top1 = XtInitialize("Tools","XPixelEditor",NULL,0,
		(Cardinal*)(&argc),argv);
	G.top2 = XtCreateApplicationShell("Brushes",
			applicationShellWidgetClass,args,0);

	G.use_viewport = False;
	for (i = 0; i < argc; i++)
	{
		if (strcmp(argv[i],"-scrollbars") == 0)
			G.use_viewport = True;
	}
	if (G.use_viewport)
	{
		printf("Using Scrollbars\n");
	}
	    else
	{
		printf("Not Using Scrollbars\n");
	}
	
	G.base_name[0] = '\0';
	G.has_hot_spot = False;
	G.have_xpm3_attributes = False;
	strcpy(G.base_name,"blah");

	G.top3 = XtCreateApplicationShell("ImageShell",
			applicationShellWidgetClass,args,arg_count);

	CreateMainMenu(&G);
	CreateBrushMenu(&G);
	CreatePixelEditor(&G);
	CreateFileSelector(&G);
	G.name_dialog = CreateDialog(&G,"Bitmap Base Name?");
	G.size_dialog = CreateDialog(&G,"Image Size: WxH?");
	G.hot_spot_dialog = CreateDialog(&G,"Bitmap Hot Spot: X,Y?");
	XawDialogAddButton(G.hot_spot_dialog,"None",DialogNone,
			   G.hot_spot_dialog);

	XtRealizeWidget(G.top1);
	XtRealizeWidget(G.top2);
	XtRealizeWidget(G.top3);

	XtMainLoop();
} /* End main */

/*---------------------------------------------------------------------------*

                      U T I L I T Y    F U N C T I O N S

 *---------------------------------------------------------------------------*/

Widget NewButton(parent,name,sensitive,callback_func,callback_arg)
Widget parent;
char *name;
Boolean sensitive;
PFV callback_func;
char *callback_arg;
{
	Arg arg;
	Widget w;

	XtSetArg(arg,XtNsensitive,sensitive);
	w = XtCreateManagedWidget(name,commandWidgetClass,parent,&arg,1);
	if (callback_func != NULL)
	{
		XtAddCallback(w,XtNcallback,callback_func,callback_arg);
	}
	return(w);
} /* End NewButton */


void SetValue(widget,name,value)
Widget widget;
char *name;
XtArgVal value;
{
	Arg arg;

	XtSetArg(arg,name,value);
	XtSetValues(widget,&arg,1);
} /* End SetValue */


/*---------------------------------------------------------------------------*

                    C A L L B A C K    F U N C T I O N S

 *---------------------------------------------------------------------------*/

void ChangeTool(w,tool,call_data)
Widget w;
int tool;
caddr_t call_data;
{
	XfwfPixelEditorChangeTool(G.pixel_editor,tool);
} /* End ChangeTool */


void ChooseBrush(w,brush_number,call_data)
Widget w;
int brush_number;
caddr_t call_data;
{
	SetValue(G.pixel_editor,XtNbrush,brush_number);
} /* End ChooseBrush */


void RegionChange(w,client_data,selected_ptr)
Widget w;
caddr_t client_data;
Boolean *selected_ptr;
{
	SetValue(G.buttons[BUTTON_COPY],XtNsensitive,*selected_ptr);
	SetValue(G.buttons[BUTTON_CUT],XtNsensitive,*selected_ptr);
	SetValue(G.buttons[BUTTON_HFLIP],XtNsensitive,*selected_ptr);
	SetValue(G.buttons[BUTTON_VFLIP],XtNsensitive,*selected_ptr);
	SetValue(G.buttons[BUTTON_FILL],XtNsensitive,*selected_ptr);
	SetValue(G.buttons[BUTTON_INVERT],XtNsensitive,*selected_ptr);
} /* End RegionChange */


void RegionCommand(w,command,call_data)
Widget w;
int command;
caddr_t call_data;
{
	switch (command)
	{
	    case COMMAND_COPY:
		XfwfPixelEditorRegionCopy(G.pixel_editor);
		break;
	    case COMMAND_CUT:
		XfwfPixelEditorRegionCut(G.pixel_editor);
		break;
	    case COMMAND_HFLIP:
		XfwfPixelEditorRegionFlipHorizontally(G.pixel_editor);
		break;
	    case COMMAND_VFLIP:
		XfwfPixelEditorRegionFlipVertically(G.pixel_editor);
		break;
	    case COMMAND_FILL:
		XfwfPixelEditorRegionFill(G.pixel_editor,
			XfwfPixelEditorGetColor(G.pixel_editor));
		break;
	    case COMMAND_INVERT:
		XfwfPixelEditorRegionInvert(G.pixel_editor,
			XfwfPixelEditorGetFGColor(G.pixel_editor),
			XfwfPixelEditorGetBGColor(G.pixel_editor));
		break;
	    default:
		printf("Unknown command %d\n",command);
		break;
	}
} /* End RegionCommand */


void ToggleGrid(w,client_data,call_data)
Widget w;
caddr_t client_data,call_data;
{
	int new_grid;
	Arg args[10];

	new_grid = !XfwfPixelEditorGetGrid(G.pixel_editor);
	XtSetArg(args[0],XtNgrid,new_grid);
	XtSetValues((Widget)(G.pixel_editor),args,1);
} /* End ToggleGrid */


void SelectAll(w,client_data,call_data)
Widget w;
caddr_t client_data,call_data;
{
	XfwfPixelEditorSelectAll(G.pixel_editor);
} /* End SelectAll */


void Zoom(w,delta,call_data)
Widget w;
int delta;
caddr_t call_data;
{
	int new_zoom;
	Arg args[10];

	new_zoom = XfwfPixelEditorGetZoom(G.pixel_editor) + delta;
	XtSetArg(args[0],XtNzoom,new_zoom);
	XtSetValues((Widget)(G.pixel_editor),args,1);
} /* End Zoom */


void ChangeFGColor(w,delta,call_data)
Widget w;
int delta;
caddr_t call_data;
{
	Pixel new_color;
	Arg args[10];

	new_color = XfwfPixelEditorGetFGColor(G.pixel_editor) + delta;
	XtSetArg(args[0],XtNdrawColor,new_color);
	XtSetValues((Widget)(G.pixel_editor),args,1);
	printf("Draw Color Set To %d\n",
	       XfwfPixelEditorGetFGColor(G.pixel_editor));
} /* End ChangeFGColor */


void ChangeBGColor(w,delta,call_data)
Widget w;
int delta;
caddr_t call_data;
{
	Pixel new_color;
	Arg args[10];

	new_color = XfwfPixelEditorGetBGColor(G.pixel_editor) + delta;
	XtSetArg(args[0],XtNeraseColor,new_color);
	XtSetValues((Widget)(G.pixel_editor),args,1);
	printf("Erase Color Set To %d\n",
	       XfwfPixelEditorGetBGColor(G.pixel_editor));
} /* End ChangeBGColor */


void Quit(w,client_data,call_data)
Widget w;
caddr_t client_data,call_data;
{
	printf("Quit\n");
	exit(1);
} /* End Quit */


void LoadFile(w,file_type,call_data)
Widget w;
int file_type;
caddr_t call_data;
{
	char *file;
	Arg args[2];
	Boolean status;

	file = SelectFile(&G,True);
	if (file != NULL)
	{
		XtSetArg(args[0],XtNzoom,1);
		XtSetArg(args[1],XtNgrid,False);
		XtSetValues((Widget)(G.pixel_editor),args,2);
		switch (file_type)
		{
		    case FILE_BITMAP:
			status = LoadBitmapFile(file);
			break;
#ifndef NO_XPM
		    case FILE_XPM3:
			status = LoadXPM3File(file);
			break;
#endif
		    default:
			fprintf(stderr,"Unknown file type %d\n",file_type);
			exit(1);
		}
		if (status == False)
		{
			fprintf(stderr,"Can't load file '%s'\n",file);
		}
	}
} /* End LoadFile */


void SaveFile(w,file_type,call_data)
Widget w;
int file_type;
caddr_t call_data;
{
	char *file;
	Boolean status;

	file = SelectFile(&G,False);
	if (file != NULL)
	{
		switch (file_type)
		{
		    case FILE_BITMAP:
			status = SaveBitmapFile(file);
			break;
#ifndef NO_XPM
		    case FILE_XPM3:
			status = SaveXPM3File(file);
			break;
#endif
		    default:
			fprintf(stderr,"Unknown file type %d\n",file_type);
			exit(1);
		}
		if (status == False)
		{
			fprintf(stderr,"Can't save file '%s'\n",file);
		}
	}
} /* End SaveFile */


Boolean LoadBitmapFile(file)
char *file;
{
	if (XfwfPixelEditorLoadBitmapFile(G.pixel_editor,file,
		XfwfPixelEditorGetFGColor(G.pixel_editor),
		XfwfPixelEditorGetBGColor(G.pixel_editor),
		&G.hotx,&G.hoty) == False)
	{
		fprintf(stderr,"Can't open file '%s'\n",file);
		return(False);
	}
	if (G.hotx < 0 || G.hoty < 0)
		G.has_hot_spot = False;
	    else
		G.has_hot_spot = True;
#ifndef NO_XPM
	if (G.have_xpm3_attributes == True)
		XpmFreeAttributes(&(G.xpm3_attributes));
	G.have_xpm3_attributes = False;
#endif
	return(True);
} /* End LoadBitmapFile */


Boolean SaveBitmapFile(file)
char *file;
{
	if (XfwfPixelEditorSaveBitmapFile(G.pixel_editor,"florp",file,
		XfwfPixelEditorGetBGColor(G.pixel_editor),
		G.hotx,G.hoty) == False)
	{
		return(False);
	}
	return(True);
} /* End SaveBitmapFile */


#ifndef NO_XPM
Boolean LoadXPM3File(file)
char *file;
{
	Display *dpy;
	XImage *image;
	Pixel *pixels;
	int x,y,width,height,status;
	XpmAttributes attributes;

	dpy = XtDisplay((Widget)(G.pixel_editor));

	attributes.valuemask = XpmReturnInfos;
	status = XpmReadFileToImage(dpy,file,&image,NULL,&attributes);
	if (status != PixmapSuccess) return(False);

	width = image->width;
	height = image->height;

	pixels = (Pixel *)calloc(width * height,sizeof(Pixel));
	if (pixels == NULL)
	{
		fprintf(stderr,"Out of memory\n");
		image->f.destroy_image(image);
		return(False);
	}

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			pixels[y * width + x] = image->f.get_pixel(image,x,y);

	image->f.destroy_image(image);
	XfwfPixelEditorSetImage(G.pixel_editor,pixels,width,height,False);
	free(pixels);

	if (G.have_xpm3_attributes == True)
		XpmFreeAttributes(&(G.xpm3_attributes));
	G.xpm3_attributes = attributes;
	G.have_xpm3_attributes = True;
	
	return(True);
} /* End LoadXPM3File */


Boolean SaveXPM3File(file)
char *file;
{
        GC gc;
	Pixmap pmap;
	Display *dpy;
	Pixel *pixels;
	XImage *image;
	Screen *screen;
	unsigned long plane_mask;
	XpmAttributes *attributes_ptr;
	int x,y,depth,status;
	unsigned int width,height;

	dpy = XtDisplay((Widget)(G.pixel_editor));
	screen = XtScreen((Widget)(G.pixel_editor));
        gc = XDefaultGCOfScreen(screen);
	depth = XDefaultDepthOfScreen(screen);
	plane_mask = ~0L;

        pixels = XfwfPixelEditorGetImage(G.pixel_editor,&width,&height);
        pmap = XCreatePixmap(dpy,XtWindow((Widget)(G.pixel_editor)),
			     width,height,depth);
        plane_mask = ~0L;
        image = XGetImage(dpy,pmap,0,0,width,height,plane_mask,XYPixmap);

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			image->f.put_pixel(image,x,y,pixels[y * width + x]);

        XPutImage(dpy,pmap,gc,image,0,0,0,0,width,height);
        image->f.destroy_image(image);

	attributes_ptr = NULL;
	if (G.have_xpm3_attributes == True)
	{
		attributes_ptr = &(G.xpm3_attributes);
		attributes_ptr->valuemask = XpmReturnInfos;
	}
	status = XpmWriteFileFromPixmap(dpy,file,pmap,None,attributes_ptr);
        XFreePixmap(dpy,pmap);

	if (status != PixmapSuccess)
		return(False);
	    else
		return(True);
} /* End SaveXPM3File */
#endif


void ChangeName(w,client_data,call_data)
Widget w;
caddr_t client_data,call_data;
{
	char *ret;

	strcpy(G.dialog_state.string,G.base_name);
	ret = DialogGetInput(&G,G.name_dialog);
	if (ret) strcpy(G.base_name,ret);
} /* End ChangeName */


void ChangeSize(w,client_data,call_data)
Widget w;
caddr_t client_data,call_data;
{
	int width,height;
	unsigned int pw,ph;
	char *ret;

	XfwfPixelEditorGetImage(G.pixel_editor,&pw,&ph);
	while (1)
	{
		sprintf(G.dialog_state.string,"%dx%d",pw,ph);
		ret = DialogGetInput(&G,G.size_dialog);
		if (ret == NULL) break;
		if (sscanf(G.dialog_state.string," %dx%d",&width,&height) == 2)
		{
			if ((width <= 0) || (height <= 0))
			{
				fprintf(stderr,"Width & Height Must Be > 0\n");
				continue;
			}
			printf("Resizing to %dx%d\n",width,height);
			XfwfPixelEditorResizeImage(G.pixel_editor,
				width,height);
			break;
		}
	}
} /* End ChangeSize */


void ChangeHotSpot(w,client_data,call_data)
Widget w;
caddr_t client_data,call_data;
{
	int x,y;
	char *ret;

	printf("ChangeHotSpot\n");
	while (1)
	{
		if (G.has_hot_spot == True)
			sprintf(G.dialog_state.string,"%d,%d",G.hotx,G.hoty);
		    else
			strcpy(G.dialog_state.string,"none");
		ret = DialogGetInput(&G,G.hot_spot_dialog);
		if (ret == NULL) break;
		if (sscanf(G.dialog_state.string," %d,%d",&x,&y) == 2)
		{
			G.has_hot_spot = True;
			G.hotx = x;
			G.hoty = y;
			break;
		}
		if ((G.dialog_state.string[0] == '\0') ||
		    (strcmp(G.dialog_state.string,"none") == 0))
		{
			G.has_hot_spot = False;
			G.hotx = -1;
			G.hoty = -1;
			break;
		}
	}
} /* End ChangeHotSpot */


/*---------------------------------------------------------------------------*

             I N I T I A L I Z A T I O N    O F    W I D G E T S

 *---------------------------------------------------------------------------*/

void CreateMainMenu(gp)
Globals *gp;
{
	int i;
	static ButtonData table[] =
	{
	  {BUTTON_PENCIL, "Pencil", True, (PFV)ChangeTool, TOOL_PENCIL},
	  {BUTTON_BRUSH, "Brush", True, (PFV)ChangeTool, TOOL_BRUSH},
	  {BUTTON_ERASER, "Eraser", True, (PFV)ChangeTool, TOOL_ERASER},
	  {BUTTON_REGION, "Region", True, (PFV)ChangeTool, TOOL_REGION},
	  {BUTTON_PASTE, "Paste", True, (PFV)ChangeTool, TOOL_PASTER},
	  {BUTTON_SELECT_ALL, "Select All", True, (PFV)SelectAll, 0},
	  {BUTTON_COPY, "Copy", False, (PFV)RegionCommand, COMMAND_COPY},
	  {BUTTON_CUT, "Cut", False, (PFV)RegionCommand, COMMAND_CUT},
	  {BUTTON_HFLIP, "H Flip", False, (PFV)RegionCommand, COMMAND_HFLIP},
	  {BUTTON_VFLIP, "V Flip", False, (PFV)RegionCommand, COMMAND_VFLIP},
	  {BUTTON_FILL, "Fill", False, (PFV)RegionCommand, COMMAND_FILL},
	  {BUTTON_INVERT, "Invert", False, (PFV)RegionCommand, COMMAND_INVERT},
	  {BUTTON_GRID, "Toggle Grid", True, (PFV)ToggleGrid, 0},
	  {BUTTON_ZOOM_IN, "Zoom In", True, (PFV)Zoom, 2},
	  {BUTTON_ZOOM_OUT, "Zoom Out", True, (PFV)Zoom, -2},
	  {BUTTON_INC_FG_COLOR, "++ Foreground", True, (PFV)ChangeFGColor, 1},
	  {BUTTON_DEC_FG_COLOR, "-- Foreground", True, (PFV)ChangeFGColor, -1},
	  {BUTTON_INC_BG_COLOR, "++ Background", True, (PFV)ChangeBGColor, 1},
	  {BUTTON_DEC_BG_COLOR, "-- Background", True, (PFV)ChangeBGColor, -1},
	  {BUTTON_SET_NAME, "Set Name", True, (PFV)ChangeName, 0},
	  {BUTTON_SET_HOTSPOT, "Set Hotspot", True, (PFV)ChangeHotSpot, 0},
	  {BUTTON_SET_SIZE, "Set Size", True, (PFV)ChangeSize, 0},
	  {BUTTON_LOAD_BITMAP, "Load Bitmap File", True, (PFV)LoadFile, 
		   FILE_BITMAP},
	  {BUTTON_SAVE_BITMAP, "Save Bitmap File", True, (PFV)SaveFile,
		   FILE_BITMAP},
#ifndef NO_XPM
	  {BUTTON_LOAD_XPM3, "Load XPM3 File", True, (PFV)LoadFile, FILE_XPM3},
	  {BUTTON_SAVE_XPM3, "Save XPM3 File", True, (PFV)SaveFile, FILE_XPM3},
#endif
	  {BUTTON_QUIT, "QUIT", True, (PFV)Quit, 0},
	  {END_OF_BUTTONS, NULL, 0, (PFV)NULL, 0}
	};

	gp->main_menu = XtCreateManagedWidget("MainMenu",boxWidgetClass,
					      gp->top1,NULL,0);
	for (i = 0; table[i].number != END_OF_BUTTONS; i++)
	{
		gp->buttons[table[i].number] = NewButton(gp->main_menu,
			table[i].name,table[i].sensitive,table[i].function,
			table[i].arg);
	}
} /* End CreateMainMenu */


void CreateBrushMenu(gp)
Globals *gp;
{
	int i;
	static ButtonData table[] =
	{
	    {0, "S. Box", True, (PFV)ChooseBrush, BRUSH_SMALL_BOX},
	    {0, "M. Box",True, (PFV)ChooseBrush,BRUSH_MEDIUM_BOX},
	    {0, "L. Box", True, (PFV)ChooseBrush, BRUSH_LARGE_BOX},
	    {0, "S. Diam.", True, (PFV)ChooseBrush, BRUSH_SMALL_DIAMOND},
	    {0, "M. Diam.", True, (PFV)ChooseBrush, BRUSH_MEDIUM_DIAMOND},
	    {0, "L. Diam.", True, (PFV)ChooseBrush, BRUSH_LARGE_DIAMOND},
	    {0, "Circle", True, (PFV)ChooseBrush, BRUSH_CIRCLE},
	    {0, "Horiz Line", True, (PFV)ChooseBrush, BRUSH_HOR_LINE},
	    {0, "Vert Line", True, (PFV)ChooseBrush, BRUSH_VERT_LINE},
	    {0, "Pos Diag", True, (PFV)ChooseBrush, BRUSH_POS_DIAG_LINE},
	    {0, "Neg Diag", True, (PFV)ChooseBrush, BRUSH_NEG_DIAG_LINE},
	    {END_OF_BUTTONS, NULL, 0, (PFV)NULL, 0}
	};

	gp->brush_menu = XtCreateManagedWidget("BrushMenu",boxWidgetClass,
					       gp->top2,NULL,0);
	for (i = 0; table[i].number != END_OF_BUTTONS; i++)
	{
		NewButton(gp->brush_menu,table[i].name,table[i].sensitive,
			table[i].function,table[i].arg);
	}
} /* End CreateBrushMenu */


void CreatePixelEditor(gp)
Globals *gp;
{
	Widget parent;
	Arg viewport_args[10];

	if (!gp->use_viewport)
	{
		parent = gp->top3;
	}
	    else
	{
		XtSetArg(viewport_args[0],XtNallowHoriz,True);
		XtSetArg(viewport_args[1],XtNallowVert,True);
		parent = XtCreateManagedWidget("viewport",viewportWidgetClass,
					       gp->top3,viewport_args,2);
	}

	gp->pixel_editor = (XfwfPixelEditorWidget)
		XtCreateManagedWidget("pixelEditor",xfwfPixelEditorWidgetClass,
			parent,NULL,0);
	XtAddCallback((Widget)(G.pixel_editor),XtNregionChangeCallback,
		RegionChange,NULL);
} /* End CreatePixelEditor */


void CreateFileSelector(gp)
Globals *gp;
{
	int x,y,w,h;
	Arg args[10];

#ifndef NO_FILE_SELECTOR
	w = WidthOfScreen(XtScreen(gp->top1)) / 3;
	h = HeightOfScreen(XtScreen(gp->top1)) / 2;
	x = (WidthOfScreen(XtScreen(gp->top1)) - w) / 2;
	y = (HeightOfScreen(XtScreen(gp->top1)) - h) / 2;
	XtSetArg(args[0],XtNx,x);
	XtSetArg(args[1],XtNy,y);
	XtSetArg(args[2],XtNwidth,w);
	XtSetArg(args[3],XtNheight,h);
	gp->fs_pop_up = XtCreatePopupShell("PopupShell",
		transientShellWidgetClass,gp->top1,args,4);
	gp->fs = XtCreateManagedWidget("HFS",xfwfFileSelectorWidgetClass,
		gp->fs_pop_up,NULL,0);
	XtAddCallback(gp->fs,XtNokButtonCallback,
		FileSelectorOk,&(gp->sel_state));
	XtAddCallback(gp->fs,XtNcancelButtonCallback,
		FileSelectorCancel,&(gp->sel_state));
#endif
} /* End CreateFileSelector */


Widget CreateDialog(gp,prompt)
Globals *gp;
char *prompt;
{
	int x,y,w,h;
	Arg args[10];
	Widget pop_up,dialog;

	w = WidthOfScreen(XtScreen(gp->top1)) / 5;
	h = HeightOfScreen(XtScreen(gp->top1)) / 8;
	x = (WidthOfScreen(XtScreen(gp->top1)) - w) / 2;
	y = (HeightOfScreen(XtScreen(gp->top1)) - h) / 2;
	XtSetArg(args[0],XtNx,x);
	XtSetArg(args[1],XtNy,y);
	XtSetArg(args[2],XtNwidth,w);
	XtSetArg(args[3],XtNheight,h);

	pop_up = XtCreatePopupShell("Input",
		transientShellWidgetClass,gp->top1,args,4);
	XtSetArg(args[0],XtNlabel,prompt);
	XtSetArg(args[1],XtNvalue,"");
	dialog = XtCreateManagedWidget("Dialog",dialogWidgetClass,
		pop_up,args,4);
	XawDialogAddButton(dialog,"Cancel",DialogCancel,dialog);
	XawDialogAddButton(dialog,"OK",DialogOk,dialog);
	return(dialog);
} /* End CreateDialog */


/*---------------------------------------------------------------------------*

                F I L E    S E L E C T O R    I N T E R F A C E

 *---------------------------------------------------------------------------*/

#ifndef NO_FILE_SELECTOR

char *SelectFile(gp,read_mode)
Globals *gp;
Boolean read_mode;
{
	Arg args[2];

	XtSetArg(args[0],XtNcheckExistence,read_mode);
	XtSetValues(gp->fs,args,1);
	XtPopup(gp->fs_pop_up,XtGrabExclusive);
	gp->sel_state.done = False;
	while (!gp->sel_state.done)
	{
		XEvent event;

		XtNextEvent(&event);
		XtDispatchEvent(&event);
	}
	XtPopdown(gp->fs_pop_up);
	if (gp->sel_state.cancelled)
		return(NULL);
	    else
		return(gp->sel_state.file_name);
} /* End SelectFile */


void FileSelectorOk(w,sel_state,fs_return)
Widget w;
SelectFileState *sel_state;
XfwfFileSelectorOkButtonReturnStruct *fs_return;
{
	strcpy(sel_state->file_name,fs_return->path);
	strcat(sel_state->file_name,fs_return->file_box_text);
	sel_state->cancelled = False;
	sel_state->done = True;
} /* End FileSelectorOk */


void FileSelectorCancel(w,sel_state,call_data)
Widget w;
SelectFileState *sel_state;
caddr_t call_data;
{
	sel_state->cancelled = True;
	sel_state->done = True;
} /* End FileSelectorCancel */

#else

char *SelectFile(gp,read_mode)
Globals *gp;
Boolean read_mode;
{
	fflush(stdin);
	printf(">>> Enter file name: ");
	fflush(stdout);
	gets(gp->sel_state.file_name);
	return(gp->sel_state.file_name);
} /* End SelectFile */

#endif

/*---------------------------------------------------------------------------*

                       D I A L O G    I N T E R F A C E

 *---------------------------------------------------------------------------*/

char *DialogGetInput(gp,dialog)
Globals *gp;
Widget dialog;
{
	Arg args[4];

	XtSetArg(args[0],XtNvalue,gp->dialog_state.string);
	XtSetValues(dialog,args,1);
	XtPopup(XtParent(dialog),XtGrabExclusive);
	gp->dialog_state.done = False;
	while (!gp->dialog_state.done)
	{
		XEvent event;

		XtNextEvent(&event);
		XtDispatchEvent(&event);
	}
	XtPopdown(XtParent(dialog));
	if (gp->dialog_state.cancelled)
		return(NULL);
	    else
		return(gp->dialog_state.string);
} /* End DialogGetInput */


void DialogOk(w,dialog,call_data)
Widget w;
Widget dialog;
caddr_t call_data;
{
	char *str;

	str = XawDialogGetValueString(dialog);
	strcpy(G.dialog_state.string,str);
	XawAsciiSourceFreeString(str);
	G.dialog_state.cancelled = False;
	G.dialog_state.done = True;
} /* End DialogOk */


void DialogNone(w,dialog,call_data)
Widget w;
Widget dialog;
caddr_t call_data;
{
	strcpy(G.dialog_state.string,"none");
	G.dialog_state.cancelled = False;
	G.dialog_state.done = True;
} /* End DialogNone */


void DialogCancel(w,dialog,call_data)
Widget w;
Widget dialog;
caddr_t call_data;
{
	G.dialog_state.cancelled = True;
	G.dialog_state.done = True;
} /* End DialogCancel */
