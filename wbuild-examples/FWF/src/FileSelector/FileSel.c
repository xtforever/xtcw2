/*****************************************************************************

	FileSelector.c

	This file contains the C code for the FileSelector widget, which
	is intended to be used as a hierarchical Unix file selector box.

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
 * (Sep 5, 1995: bugfix by Bert Bos <bert@let.rug.nl>, search for [BB])
 */ 

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/CompositeP.h>
#include <X11/Composite.h>
#include <X11/cursorfont.h>
#include <X11/Xaw/SimpleP.h>
#include <X11/Xaw/Simple.h>
#include <X11/Xaw/LabelP.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/CommandP.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiTextP.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/FormP.h>
#include <X11/Xaw/Form.h>

#include <Xfwf/ScrListP.h>
#include <Xfwf/ScrList.h>
#include <Xfwf/FileSelP.h>
#include <Xfwf/FileSel.h>

#ifdef X11R3
#define asciiTextWidgetClass	asciiStringWidgetClass
#define	XawtextEdit		XttextEdit
#define	XtNtype			"type"
#define	XawAsciiString		NULL
#define	XtNautoFill		"autoFill"
typedef	char *			XtPointer;
#endif

#define	NO_BUSY_GRAB

/*---------------------------------------------------------------------------*

                       D E C L A R A T I O N S

 *---------------------------------------------------------------------------*/

#define	SUPERCLASS		(&compositeClassRec)

#ifndef abs
#define abs(a)			((a) < 0 ? -(a) : (a))
#endif

#define max(a,b)		((a) > (b) ? (a) : (b))
#define min(a,b)		((a) < (b) ? (a) : (b))
#define XtStrlen(s)		((s) ? strlen(s) : 0)

#define	StrCopy(s)		((char*)strcpy((char *)XtMalloc((strlen(s)+1)*\
					sizeof(char)),((char *)(s))))

#define FSFieldOffset(field)	XtOffset(XfwfFileSelectorWidget,	\
					 fileSelector.field)
#define CoreFieldOffset(field)	XtOffset(Widget,core.field)

/*---------------------------------------------------------------------------*

                   I N T E R N A L    R O U T I N E S

 *---------------------------------------------------------------------------*/

#if (!NeedFunctionPrototypes)

static void		Initialize();
static void		Realize();
static void		Destroy();
static void		Resize();
static Boolean		SetValues();
static XtGeometryResult	GeometryManager();
static void		ChildrenCreate();
static void		ChildrenRealize();
static void		ChildrenRecalculate();
static void		ChildrenUpdate();
static void		ButtonUp();
static void		ButtonOk();
static void		ButtonCancel();
static void		ButtonGoto();
static void		ButtonSelect();
static char *		GetFileBoxText();
static void		ClickOnPathList();
static void		ClickOnFileList();
static void		SelectFileByIndex();
static Boolean		SelectFileByName();
static void		UnselectAll();
static void		NotifySelectionChange();
static void		ParentizeDirectory();
static void		GotoDeepestLegalDirectory();
static void		UpdateLists();
static void		UpdateTextLines();
static void		Chdir();
static void		DoBusyCursor();
static void		UndoBusyCursor();
static void		TextWidgetSetText();

#else

static void		Initialize(Widget request, Widget new);
static void		Realize(Widget gw, XtValueMask *valueMask,
				XSetWindowAttributes *attrs);
static void		Destroy(XfwfFileSelectorWidget fsw);
static void		Resize(Widget gw);
static Boolean		SetValues(Widget gcurrent, Widget grequest,
				Widget gnew);
static XtGeometryResult	GeometryManager(Widget w,
				XtWidgetGeometry *request,
				XtWidgetGeometry *reply);
static void		ChildrenCreate(XfwfFileSelectorWidget fsw);
static void		ChildrenRealize(XfwfFileSelectorWidget fsw);
static void		ChildrenRecalculate(XfwfFileSelectorWidget fsw);
static void		ChildrenUpdate(XfwfFileSelectorWidget fsw);
static void		ButtonUp(Widget w, XfwfFileSelectorWidget fsw,
				XtPointer call_data);
static void		ButtonOk(Widget w, XfwfFileSelectorWidget fsw,
				XtPointer call_data);
static void		ButtonCancel(Widget w, XfwfFileSelectorWidget fsw,
				XtPointer call_data);
static void		ButtonGoto(Widget w, XfwfFileSelectorWidget fsw,
				XtPointer call_data);
static void		ButtonSelect(Widget w, XfwfFileSelectorWidget fsw,
				XtPointer call_data);
static char *		GetFileBoxText(XfwfFileSelectorWidget fsw);
static void		ClickOnPathList(Widget w, XfwfFileSelectorWidget fsw,
				XtPointer call_data);
static void		ClickOnFileList(Widget w, XfwfFileSelectorWidget fsw,
				XtPointer call_data);
static void		SelectFileByIndex(XfwfFileSelectorWidget fsw,
				int strchr);
static Boolean		SelectFileByName(XfwfFileSelectorWidget fsw,
				char *name);
static void		UnselectAll(XfwfFileSelectorWidget fsw);
static void		NotifySelectionChange(XfwfFileSelectorWidget fsw);
static void		ParentizeDirectory(char *dir);
static void		GotoDeepestLegalDirectory(XfwfFileSelectorWidget fsw);
static void		UpdateLists(XfwfFileSelectorWidget fsw);
static void		UpdateTextLines(XfwfFileSelectorWidget fsw);
static void		Chdir(XfwfFileSelectorWidget fsw);
static void		DoBusyCursor(Widget w);
static void		UndoBusyCursor(Widget w);
static void		TextWidgetSetText(Widget tw, char *text);

#endif

/*---------------------------------------------------------------------------*

              R E S O U R C E    I N I T I A L I Z A T I O N

 *---------------------------------------------------------------------------*/

static XtResource resources[] =
{
	{XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
		CoreFieldOffset(width), XtRString, "500"},
	{XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
		CoreFieldOffset(height), XtRString, "250"},
	{XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
		CoreFieldOffset(background_pixel), XtRString, "white"},

	{XtNokButtonCallback, XtCCallback, XtRCallback,
		sizeof(XtCallbackList), FSFieldOffset(ok_button_callbacks),
		XtRCallback, NULL},
	{XtNcancelButtonCallback, XtCCallback, XtRCallback,
		sizeof(XtCallbackList), FSFieldOffset(cancel_button_callbacks),
		XtRCallback, NULL},
	{XtNselectionChangeCallback, XtCCallback, XtRCallback,
		sizeof(XtCallbackList), FSFieldOffset(sel_change_callbacks),
		XtRCallback, NULL},

	{XtNshowOkButton, XtCBoolean, XtRBoolean, sizeof(Boolean),
		FSFieldOffset(show_ok_button), XtRString, "True"},
	{XtNshowCancelButton, XtCBoolean, XtRBoolean, sizeof(Boolean),
		FSFieldOffset(show_cancel_button), XtRString, "True"},
	{XtNflagLinks, XtCBoolean, XtRBoolean, sizeof(Boolean),
		FSFieldOffset(flag_links), XtRString, "False"},
	{XtNcheckExistence, XtCBoolean, XtRBoolean, sizeof(Boolean),
		FSFieldOffset(check_existence), XtRString, "True"},

	{XtNfileSelected, XtCBoolean, XtRBoolean, sizeof(Boolean),
		FSFieldOffset(file_selected), XtRString, "False"},

	{XtNcurrentDirectory, XtCPathname, XtRString, sizeof(String),
		FSFieldOffset(current_directory), XtRString, NULL},
	{XtNcurrentFile, XtCFilename, XtRString, sizeof(String),
		FSFieldOffset(current_file), XtRString, NULL},
	{XtNtitle, XtCLabel, XtRString, sizeof(String),
		FSFieldOffset(title), XtRString, "File Selector"},
	{XtNsortMode, XtCValue, XtRInt, sizeof(int),
		FSFieldOffset(sort_mode), XtRString, "2"},
	{XtNpattern, XtCFile, XtRString, sizeof(String),
		FSFieldOffset(pattern), XtRString, NULL},
};

#undef FSFieldOffset
#undef CoreFieldOffset

/*---------------------------------------------------------------------------*

                    C L A S S    A L L O C A T I O N

 *---------------------------------------------------------------------------*/

XfwfFileSelectorClassRec xfwfFileSelectorClassRec =
{
	{
		/* superclass		*/	(WidgetClass)SUPERCLASS,
		/* class_name		*/	"XfwfFileSelector",
		/* widget_size		*/	sizeof(XfwfFileSelectorRec),
		/* class_initialize	*/	NULL,
		/* class_part_initialize*/	NULL,
		/* class_inited		*/	FALSE,
		/* initialize		*/	(XtInitProc)Initialize,
		/* initialize_hook	*/	NULL,
		/* realize		*/	(XtRealizeProc)Realize,
		/* actions		*/	NULL,
		/* num_actions		*/	0,
		/* resources		*/	resources,
		/* resource_count	*/	XtNumber(resources),
		/* xrm_class		*/	NULLQUARK,
		/* compress_motion	*/	TRUE,
		/* compress_exposure	*/	TRUE,
		/* compress_enterleave	*/	TRUE,
		/* visible_interest	*/	FALSE,
		/* destroy		*/	(XtWidgetProc)Destroy,
		/* resize		*/	(XtWidgetProc)Resize,
		/* expose		*/	XtInheritExpose,
		/* set_values		*/	(XtSetValuesFunc)SetValues,
		/* set_values_hook	*/	NULL,
		/* set_values_almost	*/	XtInheritSetValuesAlmost,
		/* get_values_hook	*/	NULL,
		/* accept_focus		*/	NULL,
		/* version		*/	XtVersion,
		/* callback_private	*/	NULL,
		/* tm_table		*/	NULL,
		/* query_geometry       */	XtInheritQueryGeometry,
		/* display_accelerator  */	XtInheritDisplayAccelerator,
		/* extension            */	NULL
	}, /* Core Part */
	{
		/* geometry_manager     */	GeometryManager,
		/* change_managed       */	XtInheritChangeManaged,
		/* insert_child         */	XtInheritInsertChild,
		/* delete_child         */	XtInheritDeleteChild,
		/* extension            */	NULL
	}, /* Composite Part */		
	{
		/* no extra class data  */	0
	}  /* FileSelector Part */
};

WidgetClass xfwfFileSelectorWidgetClass =
	(WidgetClass)&xfwfFileSelectorClassRec;

/*---------------------------------------------------------------------------*

                    E X P O R T E D    M E T H O D S

 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*

	Initialize()

	This procedure is called by the X toolkit to initialize
	the widget instance.  The hook to this routine is in the
	initialize part of the core part of the class.

 *---------------------------------------------------------------------------*/

/* ARGSUSED */
static void Initialize(request,new)
Widget request,new;
{
	int i;
	XfwfFileSelectorWidget fsw;
	char *str,*initial_file,path[MAXPATHLEN + 2];
	static char *star = "*";

	fsw = (XfwfFileSelectorWidget)new;
	FSHandCursor(fsw) = XCreateFontCursor(XtDisplay(fsw),XC_hand1);
	FSBusyCursor(fsw) = XCreateFontCursor(XtDisplay(fsw),XC_watch);

	for (i = 0; i < FS_NUM_CHILDREN; i++) FSNthWidget(fsw,i) = NULL;

	FSDirMgr(fsw) = NULL;
	FSPathList(fsw) = NULL;
	FSPathListCount(fsw) = 0;
	FSFileList(fsw) = NULL;
	FSFileListCount(fsw) = 0;

	str = (char *)XtCalloc((MAXPATHLEN + 2),sizeof(char));
	if (FSCurrentDirectory(fsw) != NULL)	/* User Specified Path */
	{
		strcpy(str,FSCurrentDirectory(fsw));
	}
	    else
	{
		getwd(path);
		strcpy(str,path);
	}
	FSCurrentDirectory(fsw) = str;

	str = (char *)XtCalloc((MAXPATHLEN + 2),sizeof(char));
	initial_file = FSCurrentFile(fsw);
	FSCurrentFile(fsw) = str;

	if (FSPattern(fsw) == NULL) FSPattern(fsw) = star;
	FSPattern(fsw) = StrCopy(FSPattern(fsw));

	if (FSCorePart(request)->width <= 0) FSCorePart(new)->width = 500;
	if (FSCorePart(request)->height <= 0) FSCorePart(new)->height = 200;

	ChildrenCreate(fsw);
	GotoDeepestLegalDirectory(fsw);
	if (initial_file) SelectFileByName(fsw,initial_file);
} /* End Initialize */


/*---------------------------------------------------------------------------*

	Realize()

	This function is called to realize a FileSelector widget.

 *---------------------------------------------------------------------------*/

static void Realize(gw,valueMask,attrs)
Widget gw;
XtValueMask *valueMask;
XSetWindowAttributes *attrs;
{
	XfwfFileSelectorWidget fsw;

	fsw = (XfwfFileSelectorWidget)gw;
	XtCreateWindow(gw,InputOutput,(Visual *)CopyFromParent,
		*valueMask,attrs);

	ChildrenRealize(fsw);
	ChildrenUpdate(fsw);

	Resize(gw);
	NotifySelectionChange(fsw);
} /* End Realize */



/*---------------------------------------------------------------------------*

	Destroy()

	This function is called to destroy a fileSelector widget.

 *---------------------------------------------------------------------------*/

static void Destroy(fsw)
XfwfFileSelectorWidget fsw;
{
	int i;

	XtFree(FSCurrentDirectory(fsw));
	XtFree(FSCurrentFile(fsw));
	XtFree(FSPattern(fsw));

	if (FSPathList(fsw) != NULL)
	{
		for (i = 0; i < FSPathListCount(fsw); i++)
			free(FSPathList(fsw)[i]);
		free(FSPathList(fsw));
	}
	if (FSFileList(fsw) != NULL)
	{
		for (i = 0; i < FSFileListCount(fsw); i++)
			free(FSFileList(fsw)[i]);
		free(FSFileList(fsw));
	}
} /* End Destroy */



/*---------------------------------------------------------------------------*

	Resize()

	This function is called to resize a fileSelector widget.

 *---------------------------------------------------------------------------*/

static void Resize(gw) 
Widget gw;
{
	XfwfFileSelectorWidget w;

	w = (XfwfFileSelectorWidget)gw;
	ChildrenUpdate(w);
} /* End Resize */


/*---------------------------------------------------------------------------*

	SetValues(gcurrent,grequest,gnew)

	This function is the external interface for setting resources.

 *---------------------------------------------------------------------------*/

/* ARGSUSED */
static Boolean SetValues(gcurrent,grequest,gnew)
Widget gcurrent,grequest,gnew;
{
	XfwfFileSelectorWidget current,new;

	current = (XfwfFileSelectorWidget)gcurrent;
	new = (XfwfFileSelectorWidget)gnew;

	if (FSPattern(current) != FSPattern(new))
	{
		XtFree(FSPattern(current));
		FSPattern(new) = StrCopy(FSPattern(current));
		Chdir(new);
	}
	if (FSSortMode(current) != FSSortMode(new))
	{
		FSSortMode(new) = FSSortMode(current);
	}
	if (FSTitle(current) != FSTitle(new))
	{
		XtWarning("Title Change Not Supported");
		FSTitle(new) = FSTitle(current);
	}
	if (FSFileSelected(current) != FSFileSelected(new))
	{
		XtWarning("Can't Change XtNfileSelected Resource");
		FSFileSelected(new) = FSFileSelected(current);
	}
	if (FSCheckExistence(current) != FSCheckExistence(new))
	{
		if (!FSCheckExistence(new))
		{
			XtSetSensitive(FSNthWidget(new,FS_I_OK_BUTTON),True);
		}
		    else
		{
			Chdir(new);
		}			    
	}
	if (FSShowOkButton(current) != FSShowOkButton(new))
	{
		if (FSShowOkButton(new) == True)
			XtManageChild(FSNthWidget(new,FS_I_OK_BUTTON));
		    else
			XtUnmanageChild(FSNthWidget(new,FS_I_OK_BUTTON));
	}
	if (FSShowCancelButton(current) != FSShowCancelButton(new))
	{
		if (FSShowCancelButton(new) == True)
			XtManageChild(FSNthWidget(new,FS_I_CANCEL_BUTTON));
		    else
			XtUnmanageChild(FSNthWidget(new,FS_I_CANCEL_BUTTON));
	}
	if (FSCurrentDirectory(current) != FSCurrentDirectory(new))
	{
		strcpy(FSCurrentDirectory(current),FSCurrentDirectory(new));
		FSCurrentDirectory(new) = FSCurrentDirectory(current);
		Chdir(new);
	}
	if (FSCurrentFile(current) != FSCurrentFile(new))
	{
		char *new_name;

		new_name = FSCurrentFile(new);
		FSCurrentFile(new) = FSCurrentFile(current);
		SelectFileByName(new,new_name);
	}
	return(False);
} /* End SetValues */



/*---------------------------------------------------------------------------*

	GeometryManager(w,request,reply)

	This routine acts as the geometry_manager method for the
	FileSelector widget.  It is called when a child wants to
	resize/reposition itself.

	Currently, we allow all requests.

 *---------------------------------------------------------------------------*/

/* ARGSUSED */
static XtGeometryResult GeometryManager(w,request,reply)
Widget w;
XtWidgetGeometry *request;
XtWidgetGeometry *reply;
{
	return(XtGeometryYes);
} /* End GeometryManager */

/*---------------------------------------------------------------------------*

                     L O C A L    R O U T I N E S

 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*

	ChildrenCreate(fsw)

	This routine creates the initial child widgets for the
	file selector widget and places them in the widget fsw.
	No placement or resizing is done.  That is done by
	ChildrenUpdate().

 *---------------------------------------------------------------------------*/

static void ChildrenCreate(fsw)
XfwfFileSelectorWidget fsw;
{
	Arg args[10];
	static char *dummy_string_list[] = { NULL };
	static char *text_box_translations = "#override\n<Key>Return: no-op()";


		/* Title */

	XtSetArg(args[0],XtNlabel,FSTitle(fsw));
	XtSetArg(args[1],XtNborderWidth,0);
	XtSetArg(args[2],XtNresizable,True);
	FSNthWidget(fsw,FS_I_TITLE) = XtCreateManagedWidget("title",
		labelWidgetClass,(Widget)fsw,args,3);

		/* Current Directory Line */

	XtSetArg(args[0],XtNtype,XawAsciiString);
	XtSetArg(args[1],XtNeditType,XawtextEdit);
	XtSetArg(args[2],XtNstring,"");
	XtSetArg(args[3],XtNautoFill,True);
	XtSetArg(args[4],XtNtranslations,text_box_translations);
	FSNthWidget(fsw,FS_I_CUR_DIR_TEXT) =
		XtCreateManagedWidget("cur_dir_text",
				      asciiTextWidgetClass,(Widget)fsw,args,4);

		/* Current File Line */

	XtSetArg(args[0],XtNtype,XawAsciiString);
	XtSetArg(args[1],XtNeditType,XawtextEdit);
	XtSetArg(args[2],XtNstring,"");
	XtSetArg(args[3],XtNautoFill,True);
	XtSetArg(args[4],XtNtranslations,text_box_translations);
	FSNthWidget(fsw,FS_I_CUR_FILE_TEXT) =
		XtCreateManagedWidget("cur_file_text",
				      asciiTextWidgetClass,(Widget)fsw,args,4);

		/* Path From Root Title */

	XtSetArg(args[0],XtNlabel,"Path From Root");
	XtSetArg(args[1],XtNborderWidth,0);
	FSNthWidget(fsw,FS_I_PATH_LIST_TITLE) =
		XtCreateManagedWidget("path_list_title",
				      labelWidgetClass,(Widget)fsw,args,2);

		/* Directory List */

	XtSetArg(args[0],XtNlist,dummy_string_list);
	XtSetArg(args[1],XtNnumberStrings,0);
	FSNthWidget(fsw,FS_I_PATH_LIST) =
		XtCreateManagedWidget("path_list",
			xfwfScrolledListWidgetClass,(Widget)fsw,args,2);
	XtAddCallback(FSNthWidget(fsw,FS_I_PATH_LIST),
		      XtNcallback,(XtCallbackProc)ClickOnPathList,
		      (XtPointer)fsw);

		/* File List Title */

	XtSetArg(args[0],XtNlabel,"Directory Contents");
	XtSetArg(args[1],XtNborderWidth,0);
	FSNthWidget(fsw,FS_I_FILE_LIST_TITLE) =
		XtCreateManagedWidget("file_list_title",
				      labelWidgetClass,(Widget)fsw,args,2);

		/* File List */

	XtSetArg(args[0],XtNlist,dummy_string_list);
	XtSetArg(args[1],XtNnumberStrings,0);
	FSNthWidget(fsw,FS_I_FILE_LIST) =
		XtCreateManagedWidget("file_list",
			xfwfScrolledListWidgetClass,(Widget)fsw,args,2);
	XtAddCallback(FSNthWidget(fsw,FS_I_FILE_LIST),
		      XtNcallback,(XtCallbackProc)ClickOnFileList,
		      (XtPointer)fsw);

		/* Goto Button */

	XtSetArg(args[0],XtNlabel,"Goto");
/*	XtSetArg(args[1],XtNborderWidth,2); */
	FSNthWidget(fsw,FS_I_GOTO_BUTTON) =
		XtCreateManagedWidget("goto_button",
				      commandWidgetClass,(Widget)fsw,args,1);
	XtAddCallback(FSNthWidget(fsw,FS_I_GOTO_BUTTON),
		      XtNcallback,(XtCallbackProc)ButtonGoto,
		      (XtPointer)fsw);

		/* Select Button */

	XtSetArg(args[0],XtNlabel,"Select");
/*	XtSetArg(args[1],XtNborderWidth,2); */
	FSNthWidget(fsw,FS_I_SELECT_BUTTON) =
		XtCreateManagedWidget("select_button",
				      commandWidgetClass,(Widget)fsw,args,1);
	XtAddCallback(FSNthWidget(fsw,FS_I_SELECT_BUTTON),
		      XtNcallback,(XtCallbackProc)ButtonSelect,
		      (XtPointer)fsw);

		/* Up Button */

	XtSetArg(args[0],XtNlabel,"Up");
/*	XtSetArg(args[1],XtNborderWidth,2); */
	FSNthWidget(fsw,FS_I_UP_BUTTON) = XtCreateManagedWidget("up_button",
		commandWidgetClass,(Widget)fsw,args,1);
	XtAddCallback(FSNthWidget(fsw,FS_I_UP_BUTTON),
		      XtNcallback,(XtCallbackProc)ButtonUp,
		      (XtPointer)fsw);

		/* OK Button */

	XtSetArg(args[0],XtNlabel,"OK");
/*	XtSetArg(args[1],XtNborderWidth,2); */
	FSNthWidget(fsw,FS_I_OK_BUTTON) = XtCreateManagedWidget("ok_button",
		commandWidgetClass,(Widget)fsw,args,1);
	XtAddCallback(FSNthWidget(fsw,FS_I_OK_BUTTON),
		      XtNcallback,(XtCallbackProc)ButtonOk,
		      (XtPointer)fsw);

		/* Cancel Button */

	XtSetArg(args[0],XtNlabel,"Cancel");
/*	XtSetArg(args[1],XtNborderWidth,2); */
	FSNthWidget(fsw,FS_I_CANCEL_BUTTON) =
		XtCreateManagedWidget("cancel_button",
				      commandWidgetClass,(Widget)fsw,args,1);
	XtAddCallback(FSNthWidget(fsw,FS_I_CANCEL_BUTTON),
		      XtNcallback,(XtCallbackProc)ButtonCancel,
		      (XtPointer)fsw);
} /* End ChildrenCreate */	


/*---------------------------------------------------------------------------*

	ChildrenRealize(fsw)

	This routine realizes the child widgets.  The widgets must
	already have been created and initialized.  Their coordinates
	should already have been set.

 *---------------------------------------------------------------------------*/

static void ChildrenRealize(fsw)
XfwfFileSelectorWidget fsw;
{
	int i;
	Widget widget;

	for (i = 0; i < FS_NUM_CHILDREN; i++)
	{
		if (FSNthWidget(fsw,i) != NULL)
		{
			widget = FSNthWidget(fsw,i);
			XtRealizeWidget(widget);
			if ((i == FS_I_OK_BUTTON &&
			     !FSShowOkButton(fsw)) ||
			    (i == FS_I_CANCEL_BUTTON &&
			     !FSShowCancelButton(fsw)))
			{
				XtUnmanageChild(widget);
			}
		}
	}
} /* End ChildrenRealize */


/*---------------------------------------------------------------------------*

        ChildrenRecalculate(fsw)

	This routine takes a file selector widget and recalculates
	the coordinates and sizes of the constituent components
	based on the current size of the file selector.

	This function does not actually change the child widgets, it
	just calculates the coordinates and caches the coordinates in
	the FileSelector widget.

 *---------------------------------------------------------------------------*/

static void ChildrenRecalculate(fsw)
XfwfFileSelectorWidget fsw;
{
	BOX *coords;
	Widget widget;
	int i,w,h,empty_space,gap,orig_path_list_h,orig_file_list_h,top;
	XtWidgetGeometry parent_idea,child_idea;

	w = FSCorePart(fsw)->width;
	h = FSCorePart(fsw)->height;

	/* Get The Child Widgets Current Widths And Heights */

	for (i = 0; i < FS_NUM_CHILDREN; i++)
	{
		if (FSNthWidget(fsw,i) != NULL)
		{
			widget = FSNthWidget(fsw,i);
			coords = FSNthCoords(fsw,i);
			BoxW(coords) = CoreWidth(widget);
			BoxH(coords) = CoreHeight(widget);
		}
	}

		/* Adjust Widths */

	BoxW(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT)) = .7 * w;
	BoxW(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT)) = .7 * w;
	BoxW(FSNthCoords(fsw,FS_I_GOTO_BUTTON)) = .15 * w;
	BoxW(FSNthCoords(fsw,FS_I_SELECT_BUTTON)) = .15 * w;

	BoxW(FSNthCoords(fsw,FS_I_PATH_LIST)) = .45 * w;
	BoxW(FSNthCoords(fsw,FS_I_FILE_LIST)) = .45 * w;
	BoxW(FSNthCoords(fsw,FS_I_UP_BUTTON)) =
		BoxW(FSNthCoords(fsw,FS_I_PATH_LIST)) * .35;
	BoxW(FSNthCoords(fsw,FS_I_OK_BUTTON)) =
		BoxW(FSNthCoords(fsw,FS_I_FILE_LIST)) * .35;
	BoxW(FSNthCoords(fsw,FS_I_CANCEL_BUTTON)) =
		BoxW(FSNthCoords(fsw,FS_I_FILE_LIST)) * .35;

		/* Adjust Heights */

	BoxH(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT)) =
		max(BoxH(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT)),
		    BoxH(FSNthCoords(fsw,FS_I_GOTO_BUTTON)));
	BoxH(FSNthCoords(fsw,FS_I_GOTO_BUTTON)) =
		BoxH(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT));

	BoxH(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT)) =
		max(BoxH(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT)),
		    BoxH(FSNthCoords(fsw,FS_I_SELECT_BUTTON)));
	BoxH(FSNthCoords(fsw,FS_I_SELECT_BUTTON)) =
		BoxH(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT));

	empty_space = h - (BoxH(FSNthCoords(fsw,FS_I_TITLE)) +
			   BoxH(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT)) +
			   BoxH(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT)) +
			   BoxH(FSNthCoords(fsw,FS_I_PATH_LIST_TITLE)) +
			   BoxH(FSNthCoords(fsw,FS_I_OK_BUTTON)));
	gap = .025 * h;
	BoxH(FSNthCoords(fsw,FS_I_PATH_LIST)) = empty_space - 8 * gap;
	BoxH(FSNthCoords(fsw,FS_I_FILE_LIST)) = empty_space - 8 * gap;
	orig_path_list_h = BoxH(FSNthCoords(fsw,FS_I_PATH_LIST));
	orig_file_list_h = BoxH(FSNthCoords(fsw,FS_I_FILE_LIST));

		/* Listen To Child Height Requests For Lists */

	parent_idea.request_mode = CWWidth | CWHeight;
	parent_idea.width = BoxW(FSNthCoords(fsw,FS_I_PATH_LIST));
	parent_idea.height = BoxH(FSNthCoords(fsw,FS_I_PATH_LIST));
	XtQueryGeometry(FSNthWidget(fsw,FS_I_PATH_LIST),
			&parent_idea,&child_idea);
	if ((child_idea.request_mode & CWHeight) &&
	    (child_idea.height < parent_idea.height))
	{
		BoxH(FSNthCoords(fsw,FS_I_PATH_LIST)) = child_idea.height;
	}

	parent_idea.request_mode = CWWidth | CWHeight;
	parent_idea.width = BoxW(FSNthCoords(fsw,FS_I_FILE_LIST));
	parent_idea.height = BoxH(FSNthCoords(fsw,FS_I_FILE_LIST));
	XtQueryGeometry(FSNthWidget(fsw,FS_I_FILE_LIST),
			&parent_idea,&child_idea);
	if ((child_idea.request_mode & CWHeight) &&
	    (child_idea.height < parent_idea.height))
	{
		BoxH(FSNthCoords(fsw,FS_I_FILE_LIST)) = child_idea.height;
	}

		/* Make Sure Both Lists Have Same Minimum Height */

	BoxH(FSNthCoords(fsw,FS_I_PATH_LIST)) =
		min(BoxH(FSNthCoords(fsw,FS_I_PATH_LIST)),
		    BoxH(FSNthCoords(fsw,FS_I_FILE_LIST)));
	BoxH(FSNthCoords(fsw,FS_I_FILE_LIST)) =
		min(BoxH(FSNthCoords(fsw,FS_I_PATH_LIST)),
		    BoxH(FSNthCoords(fsw,FS_I_FILE_LIST)));

		/* Vertical Positions */

	BoxY(FSNthCoords(fsw,FS_I_TITLE)) = gap;

	BoxY(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT)) =
		(BoxY(FSNthCoords(fsw,FS_I_TITLE)) +
		 BoxH(FSNthCoords(fsw,FS_I_TITLE))) + gap;
	BoxY(FSNthCoords(fsw,FS_I_GOTO_BUTTON)) =
		BoxY(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT));

	BoxY(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT)) =
		(BoxY(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT)) +
		 BoxH(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT))) + gap;
	BoxY(FSNthCoords(fsw,FS_I_SELECT_BUTTON)) =
		BoxY(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT));

	BoxY(FSNthCoords(fsw,FS_I_PATH_LIST_TITLE)) =
		(BoxY(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT)) +
		 BoxH(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT))) + gap;
	BoxY(FSNthCoords(fsw,FS_I_FILE_LIST_TITLE)) =
		BoxY(FSNthCoords(fsw,FS_I_PATH_LIST_TITLE));

	BoxY(FSNthCoords(fsw,FS_I_PATH_LIST)) =
		(BoxY(FSNthCoords(fsw,FS_I_PATH_LIST_TITLE)) +
		 BoxH(FSNthCoords(fsw,FS_I_PATH_LIST_TITLE))) + gap +
		(orig_path_list_h - BoxH(FSNthCoords(fsw,FS_I_PATH_LIST))) / 2;
	BoxY(FSNthCoords(fsw,FS_I_FILE_LIST)) =
		BoxY(FSNthCoords(fsw,FS_I_PATH_LIST));

	top = BoxY(FSNthCoords(fsw,FS_I_PATH_LIST)) +
		BoxH(FSNthCoords(fsw,FS_I_PATH_LIST));
	empty_space = h - top;
	BoxY(FSNthCoords(fsw,FS_I_UP_BUTTON)) = top +
		(h - top - BoxH(FSNthCoords(fsw,FS_I_UP_BUTTON))) / 2;
	BoxY(FSNthCoords(fsw,FS_I_OK_BUTTON)) = top +
		(h - top - BoxH(FSNthCoords(fsw,FS_I_OK_BUTTON))) / 2;
	BoxY(FSNthCoords(fsw,FS_I_CANCEL_BUTTON)) = top +
		(h - top - BoxH(FSNthCoords(fsw,FS_I_CANCEL_BUTTON))) / 2;

		/* Horizontal Positions */

	BoxX(FSNthCoords(fsw,FS_I_TITLE)) =
		(w - BoxW(FSNthCoords(fsw,FS_I_TITLE))) / 2;

	empty_space = w - (BoxW(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT)) +
			   BoxW(FSNthCoords(fsw,FS_I_GOTO_BUTTON)));
	gap = empty_space / 3;
	BoxX(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT)) = gap;
	BoxX(FSNthCoords(fsw,FS_I_GOTO_BUTTON)) =
		(BoxX(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT)) +
		 BoxW(FSNthCoords(fsw,FS_I_CUR_DIR_TEXT))) + gap;

	empty_space = w - (BoxW(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT)) +
			   BoxW(FSNthCoords(fsw,FS_I_SELECT_BUTTON)));
	gap = empty_space / 3;
	BoxX(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT)) = gap;
	BoxX(FSNthCoords(fsw,FS_I_SELECT_BUTTON)) =
		(BoxX(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT)) +
		 BoxW(FSNthCoords(fsw,FS_I_CUR_FILE_TEXT))) + gap;

	empty_space = w - (BoxW(FSNthCoords(fsw,FS_I_PATH_LIST)) +
			   BoxW(FSNthCoords(fsw,FS_I_FILE_LIST)));
	gap = empty_space / 3;
	BoxX(FSNthCoords(fsw,FS_I_PATH_LIST)) = gap;
	BoxX(FSNthCoords(fsw,FS_I_FILE_LIST)) =
		(BoxX(FSNthCoords(fsw,FS_I_PATH_LIST)) +
		 BoxW(FSNthCoords(fsw,FS_I_PATH_LIST))) + gap;

	BoxX(FSNthCoords(fsw,FS_I_PATH_LIST_TITLE)) =
		(BoxX(FSNthCoords(fsw,FS_I_PATH_LIST)) +
		 (BoxW(FSNthCoords(fsw,FS_I_PATH_LIST)) -
		  BoxW(FSNthCoords(fsw,FS_I_PATH_LIST_TITLE))) / 2);

	BoxX(FSNthCoords(fsw,FS_I_FILE_LIST_TITLE)) =
		(BoxX(FSNthCoords(fsw,FS_I_FILE_LIST)) +
		 (BoxW(FSNthCoords(fsw,FS_I_FILE_LIST)) -
		  BoxW(FSNthCoords(fsw,FS_I_FILE_LIST_TITLE))) / 2);

	empty_space = BoxW(FSNthCoords(fsw,FS_I_PATH_LIST)) -
		BoxW(FSNthCoords(fsw,FS_I_UP_BUTTON));
	gap = empty_space / 2;
	BoxX(FSNthCoords(fsw,FS_I_UP_BUTTON)) =
		BoxX(FSNthCoords(fsw,FS_I_PATH_LIST)) + gap;

	empty_space = BoxW(FSNthCoords(fsw,FS_I_FILE_LIST)) -
		(BoxW(FSNthCoords(fsw,FS_I_OK_BUTTON)) +
		 BoxW(FSNthCoords(fsw,FS_I_CANCEL_BUTTON)));
	gap = empty_space / 3;
	BoxX(FSNthCoords(fsw,FS_I_OK_BUTTON)) =
		BoxX(FSNthCoords(fsw,FS_I_FILE_LIST)) + gap;
	BoxX(FSNthCoords(fsw,FS_I_CANCEL_BUTTON)) =
		(BoxX(FSNthCoords(fsw,FS_I_OK_BUTTON)) +
		 BoxW(FSNthCoords(fsw,FS_I_OK_BUTTON))) + gap;
} /* End ChildrenRecalculate */


/*---------------------------------------------------------------------------*

	ChildrenUpdate(fsw)

	This routine takes a File Selector widget <fsw> and updates
	the child widgets by recalculating their coordinates and
	setting the appropriate resources.   

 *---------------------------------------------------------------------------*/

static void ChildrenUpdate(fsw)
XfwfFileSelectorWidget fsw;
{
	int i;
	Widget widget;
	BOX *coords;
	Boolean not_at_root;
	XfwfScrolledListWidget file_list;

	ChildrenRecalculate(fsw);

	for (i = 0; i < FS_NUM_CHILDREN; i++)
	{
		if (FSNthWidget(fsw,i) != NULL)
		{
			widget = FSNthWidget(fsw,i);
			coords = FSNthCoords(fsw,i);
			XtMoveWidget(widget,BoxX(coords),BoxY(coords));
			XtResizeWidget(widget,BoxW(coords),BoxH(coords),
				       CoreBorderWidth(widget));
		}
	}

	file_list = (XfwfScrolledListWidget)
		FSNthWidget(fsw,FS_I_FILE_LIST);

	if (FSPathListCount(fsw) != 1)
		not_at_root = True;
	    else
		not_at_root = False;

	XtSetSensitive(FSNthWidget(fsw,FS_I_UP_BUTTON),not_at_root);
} /* End ChildrenUpdate */


/*---------------------------------------------------------------------------*

                 I N T E R N A L    C A L L B A C K S

 *---------------------------------------------------------------------------*/

static void ButtonUp(w,fsw,call_data)
Widget w;
XfwfFileSelectorWidget fsw;
XtPointer call_data;
{
	strcat(FSCurrentDirectory(fsw),"..");
	Chdir(fsw);
} /* End ButtonUp */


static void ButtonOk(w,fsw,call_data)
Widget w;
XfwfFileSelectorWidget fsw;
XtPointer call_data;
{
	XfwfFileSelectorOkButtonReturnStruct ret;

	ret.path = FSCurrentDirectory(fsw);
	ret.file = FSCurrentFile(fsw);
	ret.file_box_text = GetFileBoxText(fsw);
	XtCallCallbacks((Widget)fsw,XtNokButtonCallback,(XtPointer)&ret);
} /* End ButtonOk */


static void ButtonCancel(w,fsw,call_data)
Widget w;
XfwfFileSelectorWidget fsw;
XtPointer call_data;
{
	XtCallCallbacks((Widget)fsw,XtNcancelButtonCallback,NULL);
} /* End ButtonCancel */


static void ButtonGoto(w,fsw,call_data)
Widget w;
XfwfFileSelectorWidget fsw;
XtPointer call_data;
{
	Widget cur_dir_text;
	Arg args[10];
	String path;

	XtSetArg(args[0],XtNstring,(XtArgVal)(&path));
	cur_dir_text = FSNthWidget(fsw,FS_I_CUR_DIR_TEXT);
	XtGetValues(cur_dir_text,args,1);
	strcpy(FSCurrentDirectory(fsw),path);
	Chdir(fsw);
} /* End ButtonGoto */


static void ButtonSelect(w,fsw,call_data)
Widget w;
XfwfFileSelectorWidget fsw;
XtPointer call_data;
{
	String file;

	file = GetFileBoxText(fsw);
	if (SelectFileByName(fsw,file) == False) XBell(XtDisplay(fsw),0);
	UpdateTextLines(fsw);
} /* End ButtonSelect */


static char *GetFileBoxText(fsw)
XfwfFileSelectorWidget fsw;
{
	char *text;
	Arg args[1];
	Widget file_box_widget;

	XtSetArg(args[0],XtNstring,(XtArgVal)(&text));
	file_box_widget = FSNthWidget(fsw,FS_I_CUR_FILE_TEXT);
	XtGetValues(file_box_widget,args,1);
	return(text);
} /* End GetFileBoxText */


static void ClickOnPathList(w,fsw,call_data)
Widget w;
XfwfFileSelectorWidget fsw;
XtPointer call_data;
{
	int i;
	XfwfScrolledListReturnStruct *ret;
	
	ret = (XfwfScrolledListReturnStruct *)call_data;
	if (ret->index == -1)
	{
		UnselectAll(fsw);		/* Click On Blank Space */
		return;
	}
	strcpy(FSCurrentDirectory(fsw),"/");
	for (i = 1; i <= ret->index; i++)
	{
		strcat(FSCurrentDirectory(fsw),FSPathList(fsw)[i]);
		strcat(FSCurrentDirectory(fsw),"/");
	}
	Chdir(fsw);
} /* End ClickOnPathList */


static void ClickOnFileList(w,fsw,call_data)
Widget w;
XfwfFileSelectorWidget fsw;
XtPointer call_data;
{
	XfwfScrolledListReturnStruct *ret;

	ret = (XfwfScrolledListReturnStruct *)call_data;
	if (ret->index == -1)
		UnselectAll(fsw);		/* Click On Blank Space */
	    else
		SelectFileByIndex(fsw,ret->index);
} /* End ClickOnFileList */


/*---------------------------------------------------------------------------*

             I N T E R N A L    S U P P O R T    R O U T I N E S

 *---------------------------------------------------------------------------*/

static void SelectFileByIndex(fsw,index)
XfwfFileSelectorWidget fsw;
int index;
{
	DirEntry *dir_entry;

	DirectoryMgrGotoItem(FSDirMgr(fsw),index);
	dir_entry = DirectoryMgrCurrentEntry(FSDirMgr(fsw));
	if (dir_entry == NULL)
	{
		fprintf(stderr,"SelectFileByIndex: Entry %d invalid\n",index);
		exit(-1);
	}
	if (DirEntryIsDir(dir_entry) || DirEntryIsDirectoryLink(dir_entry))
	{
		strcat(FSCurrentDirectory(fsw),DirEntryFileName(dir_entry));
		Chdir(fsw);
	}
	    else if (!DirEntryIsBrokenLink(dir_entry))	/* File */
	{
		strcpy(FSCurrentFile(fsw),DirEntryFileName(dir_entry));
		if (FSCheckExistence(fsw))
		{
			XtSetSensitive(FSNthWidget(fsw,FS_I_OK_BUTTON),True);
		}
		FSFileSelected(fsw) = True;
		TextWidgetSetText(FSNthWidget(fsw,FS_I_CUR_FILE_TEXT),
				  FSCurrentFile(fsw));
		XfwfScrolledListHighlightItem(FSNthWidget(fsw,FS_I_FILE_LIST),
					  index);
		NotifySelectionChange(fsw);
	}
	    else					/* Broken Link */
	{
		XBell(XtDisplay(fsw),0);
		UnselectAll(fsw);
	}
} /* End SelectFileByIndex */


static Boolean SelectFileByName(fsw,name)
XfwfFileSelectorWidget fsw;
char *name;
{
	if (DirectoryMgrGotoNamedItem(FSDirMgr(fsw),name) == FALSE)
	{
		return(False);
	}
	SelectFileByIndex(fsw,DirectoryMgrCurrentIndex(FSDirMgr(fsw)));
	return(True);
} /* End SelectFileByName */


static void UnselectAll(fsw)
XfwfFileSelectorWidget fsw;
{
	Boolean old_file_selected_flag;

	old_file_selected_flag = FSFileSelected(fsw);
	if (FSCheckExistence(fsw))
	{
		XtSetSensitive(FSNthWidget(fsw,FS_I_OK_BUTTON),False);
	}
	FSCurrentFile(fsw)[0] = '\0';
	FSFileSelected(fsw) = False;
	TextWidgetSetText(FSNthWidget(fsw,FS_I_CUR_FILE_TEXT),
			  FSCurrentFile(fsw));
	XfwfScrolledListUnhighlightAll(FSNthWidget(fsw,FS_I_FILE_LIST));
	if (old_file_selected_flag) NotifySelectionChange(fsw);
} /* End UnselectAll */


static void NotifySelectionChange(fsw)
XfwfFileSelectorWidget fsw;
{
	XfwfFileSelectorSelectionChangeReturnStruct ret;

	if (FSFileSelected(fsw) == True)
	{
		ret.file_selected = True;
		ret.path = FSCurrentDirectory(fsw);
		ret.file = FSCurrentFile(fsw);
	}
	    else
	{
		ret.file_selected = False;
		ret.path = NULL;
		ret.file = NULL;
	}
	XtCallCallbacks((Widget)fsw,XtNselectionChangeCallback,
			(XtPointer)&ret);
} /* End NotifySelectionChange */


/*---------------------------------------------------------------------------*

	ParentizeDirectory(dir)

	This routine takes a directory pathname <dir> and modifies the
	pathname to strip off the deepest directory, leaving just the parent
	path, and the trailing slash.  The parent directory of the root
	directory (a single slash) is the root directory.

 *---------------------------------------------------------------------------*/

static void ParentizeDirectory(dir)
char *dir;
{
	char *end_of_dir;

	for (end_of_dir = dir; *end_of_dir != '\0'; ++ end_of_dir);
	if ((*end_of_dir == '/') && (end_of_dir != dir))
		-- end_of_dir;
	while ((*end_of_dir != '/') && (end_of_dir != dir))
	{
		-- end_of_dir;
	}
	++ end_of_dir;
	*end_of_dir = '\0';
} /* End ParentizeDirectory */


/*---------------------------------------------------------------------------*

	GotoDeepestLegalDirectory(fsw)

	This function takes a FileSelector widget <fsw> and modifies the
	directory string in FSCurrentDirectory(fsw) to be the deepest
	legal directory above the string.  Partial or incorrect directory
	names are stripped starting at the end.

	This routine takes a File Selector widget <fsw> and updates
	the child widgets by recalculating their coordinates and
	setting the appropriate resources.  The old directory manager
	is closed and a new one is opened for this directory.  The
	file and path lists are deallocated, reallocated, and loaded
	with the path and file data lists.  The lists are then reset
	in the scrolled list widgets.

	This routine requires that all the child widgets have already
	been created.

 *---------------------------------------------------------------------------*/

static void GotoDeepestLegalDirectory(fsw)
XfwfFileSelectorWidget fsw;
{
	char dir[MAXPATHLEN + 2];

	strcpy(dir,FSCurrentDirectory(fsw));
	while (1)
	{
		if (!DirectoryMgrCanOpen(dir))
		{
			XBell(XtDisplay(fsw),0);
			if (strcmp(dir,"/") == 0)
			{
				fprintf(stderr,"%s: Can't open root\n",
					"GotoDeepestLegalDirectory");
				exit(1);
			}
			/*
			 * BUGFIX: moved the next line from before
			 * the if-statement, after a bug report by
			 * Andrew Robinson. [BB]
			 */
			ParentizeDirectory(dir);
		}
		    else
		{
			DirectoryPathExpand(dir,FSCurrentDirectory(fsw));
			if (!DirectoryMgrCanOpen(dir))
				strcpy(FSCurrentDirectory(fsw),dir);
			break;
		}
	}
	UnselectAll(fsw);
	UpdateLists(fsw);
} /* End GotoDeepestLegalDirectory */


/*---------------------------------------------------------------------------*

	UpdateLists(fsw)

	This routine takes a FileSelector widget and builds new path
	and file lists from the current directory, sort mode, and pattern
	strings.  The lists and text items are then changed.

 *---------------------------------------------------------------------------*/

static void UpdateLists(fsw)
XfwfFileSelectorWidget fsw;
{
	int i,count;
	char *dir,*start;
	DirEntry *dir_entry;
	DirectoryMgr *dir_mgr;
	char temp[MAXPATHLEN + 2];

	if (FSDirMgr(fsw)) DirectoryMgrClose(FSDirMgr(fsw));
	dir_mgr = DirectoryMgrSimpleOpen(FSCurrentDirectory(fsw),
					 FSSortMode(fsw),
					 FSPattern(fsw));
	if (dir_mgr == NULL)
	{
		fprintf(stderr,"UpdateLists: Can't read directory '%s'\n",
			FSCurrentDirectory(fsw));
		exit(1);
	}
		
	FSDirMgr(fsw) = dir_mgr;
	if (FSPathList(fsw) != NULL)
	{
		for (i = 0; i < FSPathListCount(fsw); i++)
			free(FSPathList(fsw)[i]);
		free(FSPathList(fsw));
	}
	if (FSFileList(fsw) != NULL)
	{
		for (i = 0; i < FSFileListCount(fsw); i++)
			free(FSFileList(fsw)[i]);
		free(FSFileList(fsw));
	}

	FSFileListCount(fsw) = DirectoryMgrFilteredCount(FSDirMgr(fsw));
	FSPathListCount(fsw) = 1;
	for (dir = FSCurrentDirectory(fsw) + 1; *dir != '\0'; dir++)
	{
		if (*dir == '/') ++ FSPathListCount(fsw);
	}

	FSFileList(fsw) = (char **)malloc(sizeof(char *) *
					  (FSFileListCount(fsw) + 1));
	if (FSFileList(fsw) == NULL)
	{
		fprintf(stderr,"UpdateLists: Out of memory\n");
		exit(-1);
	}

	for (i = 0; i < FSFileListCount(fsw); i++)
	{
		dir_entry = DirectoryMgrNextEntry(FSDirMgr(fsw));
		if (dir_entry == NULL) XtError("Inconsistent Directory");
		strcpy(temp,DirEntryFileName(dir_entry));
		if (DirEntryIsDir(dir_entry))
			strcat(temp,"/");
		    else if (DirEntryIsBrokenLink(dir_entry))
			strcat(temp," X");
		    else if (DirEntryIsDirectoryLink(dir_entry))
			strcat(temp,"/");
		    else if (DirEntryIsSymLink(dir_entry) && FSFlagLinks(fsw))
			strcat(temp," @");

		FSFileList(fsw)[i] = StrCopy(temp);
	}
	FSFileList(fsw)[i] = NULL;

	FSPathList(fsw) = (char **)malloc(sizeof(char *) *
					  (FSPathListCount(fsw) + 1));
	start = FSCurrentDirectory(fsw);
	FSPathList(fsw)[0] = StrCopy("/");
	for (i = 1; i < FSPathListCount(fsw); i++)
	{
		while (*start != '\0' && *start == '/') ++start;
		count = 0;
		while (*start != '\0' && *start != '/')
			temp[count++] = *start++;
		temp[count++] = '\0';
		FSPathList(fsw)[i] = StrCopy(temp);
	}
	FSPathList(fsw)[i] = NULL;

	XfwfScrolledListSetList(FSNthWidget(fsw,FS_I_PATH_LIST),
			    FSPathList(fsw),FSPathListCount(fsw),True,NULL);
	XfwfScrolledListSetList(FSNthWidget(fsw,FS_I_FILE_LIST),
			    FSFileList(fsw),FSFileListCount(fsw),True,NULL);
	UpdateTextLines(fsw);
} /* End UpdateLists */


static void UpdateTextLines(fsw)
XfwfFileSelectorWidget fsw;
{
	TextWidgetSetText(FSNthWidget(fsw,FS_I_CUR_DIR_TEXT),
			  FSCurrentDirectory(fsw));
	TextWidgetSetText(FSNthWidget(fsw,FS_I_CUR_FILE_TEXT),
			  FSCurrentFile(fsw));
} /* End UpdateTextLines */


static void Chdir(fsw)
XfwfFileSelectorWidget fsw;
{
	DoBusyCursor((Widget)fsw);
	GotoDeepestLegalDirectory(fsw);
	ChildrenUpdate(fsw);
	UndoBusyCursor((Widget)fsw);
} /* End Chdir */


static void DoBusyCursor(w)
Widget w;
{
#ifndef NO_BUSY_GRAB
	if (XtIsRealized(w))
	{
		XGrabPointer(XtDisplay(w),XtWindow(w),True,None,
			     GrabModeSync,GrabModeSync,None,FSBusyCursor(w),
			     CurrentTime);
	}
#endif
} /* End DoBusyCursor */


static void UndoBusyCursor(w)
Widget w;
{
#ifndef NO_BUSY_GRAB
	if (XtIsRealized(w))
	{
		XUngrabPointer(XtDisplay(w),CurrentTime);
	}
#endif
} /* End UndoBusyCursor */


static void TextWidgetSetText(tw,text)
Widget tw;
char *text;
{
	Arg args[3];
	int length,insert_position;
#ifdef X11R3
	static char text_widget_storage[MAXPATHLEN + 2];
#endif

	length = strlen(text);
	insert_position = max(length,0);
#ifdef X11R3
	/*
	XtTextSetInsertionPoint(tw,insert_position);
	XtSetArg(args[0],XtNstring,text);
	XtSetValues(tw,args,1);
	{
		XtTextBlock block;

		block.firstPos = 0;
		block.length = length + 1;
		block.ptr = text;
		block.format = FMT8BIT;
		XtTextReplace(fsw,0,block.length - 1,&block);
	}
	*/
#else
	XtSetArg(args[0],XtNstring,text);
	XtSetValues(tw,args,1);
	XawTextSetInsertionPoint(tw,insert_position);
#endif
} /* End TextWidgetSetText */

/*---------------------------------------------------------------------------*

                    E X T E R N A L    R O U T I N E S

 *---------------------------------------------------------------------------*/

void XfwfFileSelectorChangeDirectory(fsw,dir)
XfwfFileSelectorWidget fsw;
char *dir;
{
	strcpy(FSCurrentDirectory(fsw),dir);
	Chdir(fsw);
} /* End XfwfFileSelectorChangeDirectory */


void XfwfFileSelectorRefresh(fsw)
XfwfFileSelectorWidget fsw;
{
	XfwfFileSelectorChangeDirectory(fsw,".");
} /* End XfwfFileSelectorRefresh */


void XfwfFileSelectorGetStatus(fsw,ssp)
XfwfFileSelectorWidget fsw;
XfwfFileSelectorStatusStruct *ssp;
{	
	ssp->file_selected = FSFileSelected(fsw);
	ssp->path = FSCurrentDirectory(fsw);
	ssp->file = FSCurrentFile(fsw);
	ssp->file_box_text = GetFileBoxText(fsw);
} /* End XfwfFileSelectorGetStatus */
