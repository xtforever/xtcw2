
/*****************************************************************************

	FileLister.c

	This file contains the C code for the FileLister widget,  which
	is intended to be used as a hierarchical Unix file listing box.

******************************************************************************/

/*
 * Authors:
 *
 * 	Brian Totty 
 * 	Department of Computer Science
 * 	University Of Illinois at Urbana-Champaign
 *	1304 West Springfield Avenue
 * 	Urbana,  IL 61801
 * 
 * 	totty@cs.uiuc.edu
 * 	
 *
 * 	Scott Chilcote
 * 	DSR, Inc.
 * 	12450 Fair Lakes Circle
 * 	Fairfax, VA  22033
 * 
 * 	scotty@cais.com
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
#include <Xfwf/FileListP.h>
#include <Xfwf/FileList.h>

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

#define max(a, b)		((a) > (b) ? (a) : (b))
#define min(a, b)		((a) < (b) ? (a) : (b))
#define XtStrlen(s)		((s) ? strlen(s) : 0)

#define	StrCopy(s)		((char*)strcpy((char *)XtMalloc((strlen(s)+1)*\
					sizeof(char)), ((char *)(s))))

#define FSFieldOffset(field)	XtOffset(XfwfFileListerWidget, 	\
					 FileLister.field)
#define CoreFieldOffset(field)	XtOffset(Widget, core.field)

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
static void		ButtonAdd();
static void		ButtonPath();
static void		ButtonRemove();
static void		ButtonClear();
static char *		GetFileBoxText();
static void		ClickOnPathList();
static void		ClickOnFileList();
static void		ClickOnSelList();
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

static void		Initialize(Widget request,  Widget new);
static void		Realize(Widget gw,  XtValueMask *valueMask, 
				XSetWindowAttributes *attrs);
static void		Destroy(XfwfFileListerWidget flw);
static void		Resize(Widget gw);
static Boolean		SetValues(Widget gcurrent,  Widget grequest, 
				Widget gnew);
static XtGeometryResult	GeometryManager(Widget w, 
				XtWidgetGeometry *request, 
				XtWidgetGeometry *reply);
static void		ChildrenCreate(XfwfFileListerWidget flw);
static void		ChildrenRealize(XfwfFileListerWidget flw);
static void		ChildrenRecalculate(XfwfFileListerWidget flw);
static void		ChildrenUpdate(XfwfFileListerWidget flw);
static void		ButtonUp(Widget w,  XfwfFileListerWidget flw, 
				XtPointer call_data);
static void		ButtonOk(Widget w,  XfwfFileListerWidget flw, 
				XtPointer call_data);
static void		ButtonCancel(Widget w,  XfwfFileListerWidget flw, 
				XtPointer call_data);
static void		ButtonGoto(Widget w,  XfwfFileListerWidget flw, 
				XtPointer call_data);
static void		ButtonPath(Widget w,  XfwfFileListerWidget flw, 
				XtPointer call_data);
static void		ButtonClear(Widget w,  XfwfFileListerWidget flw, 
				XtPointer call_data);
static void		ButtonRemove(Widget w,  XfwfFileListerWidget flw, 
				XtPointer call_data);
static void		ButtonAdd(Widget w,  XfwfFileListerWidget flw, 
				XtPointer call_data);
static char *		GetFileBoxText(XfwfFileListerWidget flw);
static void		ClickOnPathList(Widget w,  XfwfFileListerWidget flw, 
				XtPointer call_data);
static void		ClickOnFileList(Widget w,  XfwfFileListerWidget flw, 
				XtPointer call_data);
static void		ClickOnSelList(Widget w,  XfwfFileListerWidget flw, 
				XtPointer call_data);
static void		SelectFileByIndex(XfwfFileListerWidget flw, 
				int strchr);
static Boolean		SelectFileByName(XfwfFileListerWidget flw, 
				char *name);
static void		UnselectAll(XfwfFileListerWidget flw);
static void		NotifySelectionChange(XfwfFileListerWidget flw);
static void		ParentizeDirectory(char *dir);
static void		GotoDeepestLegalDirectory(XfwfFileListerWidget flw);
static void		UpdateLists(XfwfFileListerWidget flw);
static void		UpdateTextLines(XfwfFileListerWidget flw);
static void		Chdir(XfwfFileListerWidget flw);
static void		DoBusyCursor(Widget w);
static void		UndoBusyCursor(Widget w);
static void		TextWidgetSetText(Widget tw,  char *text);

#endif

/*---------------------------------------------------------------------------*

              R E S O U R C E    I N I T I A L I Z A T I O N

 *---------------------------------------------------------------------------*/

static XtResource resources[] =
{
	{XtNwidth,  XtCWidth,  XtRDimension,  sizeof(Dimension), 
		CoreFieldOffset(width),  XtRString,  "750"}, 
	{XtNheight,  XtCHeight,  XtRDimension,  sizeof(Dimension), 
		CoreFieldOffset(height),  XtRString,  "250"}, 
	{XtNbackground,  XtCBackground,  XtRPixel,  sizeof(Pixel), 
		CoreFieldOffset(background_pixel),  XtRString,  "white"}, 

	{XtNokButtonCallback,  XtCCallback,  XtRCallback, 
		sizeof(XtCallbackList),  FSFieldOffset(ok_button_callbacks), 
		XtRCallback,  NULL}, 
	{XtNcancelButtonCallback,  XtCCallback,  XtRCallback, 
		sizeof(XtCallbackList),  FSFieldOffset(cancel_button_callbacks), 
		XtRCallback,  NULL}, 
	{XtNselectionChangeCallback,  XtCCallback,  XtRCallback, 
		sizeof(XtCallbackList),  FSFieldOffset(sel_change_callbacks), 
		XtRCallback,  NULL}, 

	{XtNshowOkButton,  XtCBoolean,  XtRBoolean,  sizeof(Boolean), 
		FSFieldOffset(show_ok_button),  XtRString,  "True"}, 
	{XtNshowCancelButton,  XtCBoolean,  XtRBoolean,  sizeof(Boolean), 
		FSFieldOffset(show_cancel_button),  XtRString,  "True"}, 
	{XtNflagLinks,  XtCBoolean,  XtRBoolean,  sizeof(Boolean), 
		FSFieldOffset(flag_links),  XtRString,  "False"}, 
	{XtNcheckExistence,  XtCBoolean,  XtRBoolean,  sizeof(Boolean), 
		FSFieldOffset(check_existence),  XtRString,  "True"}, 

	{XtNfileSelected,  XtCBoolean,  XtRBoolean,  sizeof(Boolean), 
		FSFieldOffset(file_selected),  XtRString,  "False"}, 

	{XtNcurrentDirectory,  XtCPathname,  XtRString,  sizeof(String), 
		FSFieldOffset(current_directory),  XtRString,  NULL}, 
	{XtNcurrentFile,  XtCFilename,  XtRString,  sizeof(String), 
		FSFieldOffset(current_file),  XtRString,  NULL}, 
	{XtNtitle,  XtCLabel,  XtRString,  sizeof(String), 
		FSFieldOffset(title),  XtRString,  "File List Builder"}, 
	{XtNsortMode,  XtCValue,  XtRInt,  sizeof(int), 
		FSFieldOffset(sort_mode),  XtRString,  "2"}, 
	{XtNpattern,  XtCFile,  XtRString,  sizeof(String), 
		FSFieldOffset(pattern),  XtRString,  NULL}, 
};

#undef FSFieldOffset
#undef CoreFieldOffset

/*---------------------------------------------------------------------------*

                    C L A S S    A L L O C A T I O N

 *---------------------------------------------------------------------------*/

XfwfFileListerClassRec xfwfFileListerClassRec =
{
	{
		/* superclass		*/	(WidgetClass)SUPERCLASS, 
		/* class_name		*/	"XfwfFileLister", 
		/* widget_size		*/	sizeof(XfwfFileListerRec), 
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
	},  /* Core Part */
	{
		/* geometry_manager     */	GeometryManager, 
		/* change_managed       */	XtInheritChangeManaged, 
		/* insert_child         */	XtInheritInsertChild, 
		/* delete_child         */	XtInheritDeleteChild, 
		/* extension            */	NULL
	},  /* Composite Part */		
	{
		/* no extra class data  */	0
	}  /* FileLister Part */
};

WidgetClass xfwfFileListerWidgetClass =
	(WidgetClass)&xfwfFileListerClassRec;

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
static void Initialize(request, new)
Widget request, new;
{
	int i;
	XfwfFileListerWidget flw;
	char *str, *initial_file, path[MAXPATHLEN + 2];
	static char *star = "*";

	flw = (XfwfFileListerWidget)new;
	FSHandCursor(flw) = XCreateFontCursor(XtDisplay(flw), XC_hand1);
	FSBusyCursor(flw) = XCreateFontCursor(XtDisplay(flw), XC_watch);

	for (i = 0; i < FS_NUM_CHILDREN; i++) FSNthWidget(flw, i) = NULL;

	FSDirMgr(flw) = NULL;
	FSPathList(flw) = NULL;
	FSPathListCount(flw) = 0;
	FSFileList(flw) = NULL;
	FSFileListCount(flw) = 0;
	FSSelFileList(flw)[0] = NULL;
	FSSelPathList(flw)[0] = NULL;
	FSSelListCount(flw) = 0;

	str = (char *)XtCalloc((MAXPATHLEN + 2), sizeof(char));
	if (FSCurrentDirectory(flw) != NULL)	/* User Specified Path */
	{
		strcpy(str, FSCurrentDirectory(flw));
	}
	    else
	{
		getwd(path);
		strcpy(str, path);
	}
	FSCurrentDirectory(flw) = str;

	str = (char *)XtCalloc((MAXPATHLEN + 2), sizeof(char));
	initial_file = FSCurrentFile(flw);
	FSCurrentFile(flw) = str;

	if (FSPattern(flw) == NULL) FSPattern(flw) = star;
	FSPattern(flw) = StrCopy(FSPattern(flw));

	if (FSCorePart(request)->width <= 0) FSCorePart(new)->width = 500;
	if (FSCorePart(request)->height <= 0) FSCorePart(new)->height = 200;

	ChildrenCreate(flw);
	GotoDeepestLegalDirectory(flw);
	if (initial_file) SelectFileByName(flw, initial_file);

	XtSetSensitive(FSNthWidget(flw, FS_I_CLEAR_BUTTON), False);
	XtSetSensitive(FSNthWidget(flw, FS_I_REMOVE_BUTTON), False);
	XtSetSensitive(FSNthWidget(flw, FS_I_PATH_BUTTON), False); 

} /* End Initialize */


/*---------------------------------------------------------------------------*

	Realize()

	This function is called to realize a FileLister widget.

 *---------------------------------------------------------------------------*/

static void Realize(gw, valueMask, attrs)
Widget gw;
XtValueMask *valueMask;
XSetWindowAttributes *attrs;
{
	XfwfFileListerWidget flw;

	flw = (XfwfFileListerWidget)gw;
	XtCreateWindow(gw, InputOutput, (Visual *)CopyFromParent, 
		*valueMask, attrs);

	ChildrenRealize(flw);
	ChildrenUpdate(flw);

	Resize(gw);
	NotifySelectionChange(flw);
} /* End Realize */



/*---------------------------------------------------------------------------*

	Destroy()

	This function is called to destroy a fileLister widget.

 *---------------------------------------------------------------------------*/

static void Destroy(flw)
XfwfFileListerWidget flw;
{
	int i;

	XtFree(FSCurrentDirectory(flw));
	XtFree(FSCurrentFile(flw));
	XtFree(FSPattern(flw));

	if (FSPathList(flw) != NULL)
	{
		for (i = 0; i < FSPathListCount(flw); i++)
			free(FSPathList(flw)[i]);
		free(FSPathList(flw));
	}

	if (FSFileList(flw) != NULL)
	{
		for (i = 0; i < FSFileListCount(flw); i++)
			free(FSFileList(flw)[i]);
		free(FSFileList(flw));
	}

	if (FSSelFileList(flw) != NULL)
	{
		for (i = 0; i < FSSelListCount(flw); i++)
		{
			free(FSSelFileList(flw)[i]);
			free(FSSelPathList(flw)[i]);
		}
	}

} /* End Destroy */



/*---------------------------------------------------------------------------*

	Resize()

	This function is called to resize a fileLister widget.

 *---------------------------------------------------------------------------*/

static void Resize(gw) 
Widget gw;
{
	XfwfFileListerWidget w;

	w = (XfwfFileListerWidget)gw;
	ChildrenUpdate(w);
} /* End Resize */


/*---------------------------------------------------------------------------*

	SetValues(gcurrent, grequest, gnew)

	This function is the external interface for setting resources.

 *---------------------------------------------------------------------------*/

/* ARGSUSED */
static Boolean SetValues(gcurrent, grequest, gnew)
Widget gcurrent, grequest, gnew;
{
	XfwfFileListerWidget current, new;

	current = (XfwfFileListerWidget)gcurrent;
	new = (XfwfFileListerWidget)gnew;

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
			XtSetSensitive(FSNthWidget(new, FS_I_OK_BUTTON), True);
		}
		    else
		{
			Chdir(new);
		}			    
	}
	if (FSShowOkButton(current) != FSShowOkButton(new))
	{
		if (FSShowOkButton(new) == True)
			XtManageChild(FSNthWidget(new, FS_I_OK_BUTTON));
		    else
			XtUnmanageChild(FSNthWidget(new, FS_I_OK_BUTTON));
	}
	if (FSShowCancelButton(current) != FSShowCancelButton(new))
	{
		if (FSShowCancelButton(new) == True)
			XtManageChild(FSNthWidget(new, FS_I_CANCEL_BUTTON));
		    else
			XtUnmanageChild(FSNthWidget(new, FS_I_CANCEL_BUTTON));
	}
	if (FSCurrentDirectory(current) != FSCurrentDirectory(new))
	{
		strcpy(FSCurrentDirectory(current), FSCurrentDirectory(new));
		FSCurrentDirectory(new) = FSCurrentDirectory(current);
		Chdir(new);
	}
	if (FSCurrentFile(current) != FSCurrentFile(new))
	{
		char *new_name;

		new_name = FSCurrentFile(new);
		FSCurrentFile(new) = FSCurrentFile(current);
		SelectFileByName(new, new_name);
	}
	return(False);
} /* End SetValues */



/*---------------------------------------------------------------------------*

	GeometryManager(w, request, reply)

	This routine acts as the geometry_manager method for the
	FileLister widget.  It is called when a child wants to
	resize/reposition itself.

	Currently,  we allow all requests.

 *---------------------------------------------------------------------------*/

/* ARGSUSED */
static XtGeometryResult GeometryManager(w, request, reply)
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

	ChildrenCreate(flw)

	This routine creates the initial child widgets for the
	file listing widget and places them in the widget flw.
	No placement or resizing is done.  That is done by
	ChildrenUpdate().

 *---------------------------------------------------------------------------*/

static void ChildrenCreate(flw)
XfwfFileListerWidget flw;
{
	Arg args[10];
	static char *dummy_string_list[] = { NULL };
	static char *text_box_translations = "#override\n<Key>Return: no-op()";


		/* Title */

	XtSetArg(args[0], XtNlabel, FSTitle(flw));
	XtSetArg(args[1], XtNborderWidth, 0);
	XtSetArg(args[2], XtNresizable, True);
	FSNthWidget(flw, FS_I_TITLE) = XtCreateManagedWidget("title", 
		labelWidgetClass, (Widget)flw, args, 3);

		/* Current Directory Line */

	XtSetArg(args[0], XtNtype, XawAsciiString);
	XtSetArg(args[1], XtNeditType, XawtextEdit);
	XtSetArg(args[2], XtNstring, "");
	XtSetArg(args[3], XtNautoFill, True);
	XtSetArg(args[4], XtNtranslations, text_box_translations);
	FSNthWidget(flw, FS_I_CUR_DIR_TEXT) =
		XtCreateManagedWidget("cur_dir_text", 
				asciiTextWidgetClass, (Widget)flw, args, 4);

		/* Current File Line */

	XtSetArg(args[0], XtNtype, XawAsciiString);
	XtSetArg(args[1], XtNeditType, XawtextEdit);
	XtSetArg(args[2], XtNstring, "");
	XtSetArg(args[3], XtNautoFill, True);
	XtSetArg(args[4], XtNtranslations, text_box_translations);
	FSNthWidget(flw, FS_I_CUR_FILE_TEXT) =
		XtCreateManagedWidget("cur_file_text", 
				asciiTextWidgetClass, (Widget)flw, args, 4);

		/* Path From Root Title */

	XtSetArg(args[0], XtNlabel, "Path From Root");
	XtSetArg(args[1], XtNborderWidth, 0);
	FSNthWidget(flw, FS_I_PATH_LIST_TITLE) =
		XtCreateManagedWidget("path_list_title", 
					labelWidgetClass, (Widget)flw, args, 2);

		/* Directory List */

	XtSetArg(args[0], XtNlist, dummy_string_list);
	XtSetArg(args[1], XtNnumberStrings, 0);
	FSNthWidget(flw, FS_I_PATH_LIST) =
		XtCreateManagedWidget("path_list", 
			xfwfScrolledListWidgetClass, (Widget)flw, args, 2);
	XtAddCallback(FSNthWidget(flw, FS_I_PATH_LIST), 
			XtNcallback, (XtCallbackProc)ClickOnPathList, 
			(XtPointer)flw);

		/* File List Title */

	XtSetArg(args[0], XtNlabel, "Directory Contents");
	XtSetArg(args[1], XtNborderWidth, 0);
	FSNthWidget(flw, FS_I_FILE_LIST_TITLE) =
		XtCreateManagedWidget("file_list_title", 
					labelWidgetClass, (Widget)flw, args, 2);

		/* File List */

	XtSetArg(args[0], XtNlist, dummy_string_list);
	XtSetArg(args[1], XtNnumberStrings, 0);
	FSNthWidget(flw, FS_I_FILE_LIST) =
		XtCreateManagedWidget("file_list", 
			xfwfScrolledListWidgetClass, (Widget)flw, args, 2);
	XtAddCallback(FSNthWidget(flw, FS_I_FILE_LIST), 
		XtNcallback, (XtCallbackProc)ClickOnFileList, 
		(XtPointer)flw);

	/* Selection List Title */

	XtSetArg( args[0],  XtNlabel,  "Selected Files" );
	XtSetArg( args[1],  XtNborderWidth,  0 );
	FSNthWidget( flw,  FS_I_SEL_LIST_TITLE ) =
		XtCreateManagedWidget( "file_list_title",
			labelWidgetClass,
			(Widget) flw,  args,  2 );
 
	/* Selected File List */
 
	XtSetArg( args[0],  XtNlist,  dummy_string_list );
	XtSetArg( args[1],  XtNnumberStrings,  0 );
	FSNthWidget( flw,  FS_I_SEL_LIST) =
		XtCreateManagedWidget( "selection_list", 
			xfwfScrolledListWidgetClass,
			(Widget) flw,  args,  2 );
	XtAddCallback( FSNthWidget( flw,  FS_I_SEL_LIST ),
			XtNcallback,
			( XtCallbackProc ) ClickOnSelList, 
			( XtPointer ) flw );

	/* Path Button */

	XtSetArg(args[0], XtNlabel, "Path");
	FSNthWidget(flw, FS_I_PATH_BUTTON) =
		XtCreateManagedWidget("path_button",
			commandWidgetClass, (Widget)flw, args, 1);
	XtAddCallback(FSNthWidget(flw, FS_I_PATH_BUTTON),
			XtNcallback, (XtCallbackProc)ButtonPath,
			(XtPointer)flw);

	/* Remove Button */

	XtSetArg(args[0], XtNlabel, "Remove");
	FSNthWidget(flw, FS_I_REMOVE_BUTTON) =
		XtCreateManagedWidget("REMOVE_button",
			commandWidgetClass, (Widget)flw, args, 1);
	XtAddCallback(FSNthWidget(flw, FS_I_REMOVE_BUTTON),
			XtNcallback, (XtCallbackProc)ButtonRemove,
			(XtPointer)flw);

	/* Clear Button */

	XtSetArg(args[0], XtNlabel, "Clear");
	FSNthWidget(flw, FS_I_CLEAR_BUTTON) =
		XtCreateManagedWidget("clear_button",
			commandWidgetClass, (Widget)flw, args, 1);
	XtAddCallback(FSNthWidget(flw, FS_I_CLEAR_BUTTON),
			XtNcallback, (XtCallbackProc)ButtonClear,
			(XtPointer)flw);

		/* Goto Button */

	XtSetArg(args[0], XtNlabel, "Goto");
	FSNthWidget(flw, FS_I_GOTO_BUTTON) =
		XtCreateManagedWidget("goto_button", 
				commandWidgetClass, (Widget)flw, args, 1);
	XtAddCallback(FSNthWidget(flw, FS_I_GOTO_BUTTON), 
		XtNcallback, (XtCallbackProc)ButtonGoto, 
		(XtPointer)flw);

		/* Add Button */

	XtSetArg(args[0], XtNlabel, "Add");
	FSNthWidget(flw, FS_I_ADD_BUTTON) =
		XtCreateManagedWidget( "add_button", 
				commandWidgetClass, (Widget)flw, args, 1);
	XtAddCallback(FSNthWidget(flw, FS_I_ADD_BUTTON), 
		XtNcallback, (XtCallbackProc)ButtonAdd, 
		(XtPointer)flw);

		/* Up Button */

	XtSetArg(args[0], XtNlabel, "Up");
	FSNthWidget(flw, FS_I_UP_BUTTON) = XtCreateManagedWidget("up_button", 
		commandWidgetClass, (Widget)flw, args, 1);
	XtAddCallback(FSNthWidget(flw, FS_I_UP_BUTTON), 
		XtNcallback, (XtCallbackProc)ButtonUp, 
		(XtPointer)flw);

		/* OK Button */

	XtSetArg(args[0], XtNlabel, "OK");
	FSNthWidget(flw, FS_I_OK_BUTTON) = XtCreateManagedWidget("ok_button", 
		commandWidgetClass, (Widget)flw, args, 1);
	XtAddCallback(FSNthWidget(flw, FS_I_OK_BUTTON), 
		XtNcallback, (XtCallbackProc)ButtonOk, 
		(XtPointer)flw);

		/* Cancel Button */

	XtSetArg(args[0], XtNlabel, "Cancel");
	FSNthWidget(flw, FS_I_CANCEL_BUTTON) =
		XtCreateManagedWidget("cancel_button", 
				commandWidgetClass, (Widget)flw, args, 1);
	XtAddCallback(FSNthWidget(flw, FS_I_CANCEL_BUTTON), 
		XtNcallback, (XtCallbackProc)ButtonCancel, 
		(XtPointer)flw);
} /* End ChildrenCreate */	


/*---------------------------------------------------------------------------*

	ChildrenRealize(flw)

	This routine realizes the child widgets.  The widgets must
	already have been created and initialized.  Their coordinates
	should already have been set.

 *---------------------------------------------------------------------------*/

static void ChildrenRealize(flw)
XfwfFileListerWidget flw;
{
	int i;
	Widget widget;

	for (i = 0; i < FS_NUM_CHILDREN; i++)
	{
		if (FSNthWidget(flw, i) != NULL)
		{
			widget = FSNthWidget(flw, i);
			XtRealizeWidget(widget);
			if ((i == FS_I_OK_BUTTON &&
				!FSShowOkButton(flw)) ||
				(i == FS_I_CANCEL_BUTTON &&
				!FSShowCancelButton(flw)))
			{
				XtUnmanageChild(widget);
			}
		}
	}
} /* End ChildrenRealize */


/*---------------------------------------------------------------------------*

        ChildrenRecalculate(flw)

	This routine takes a file listing widget and recalculates
	the coordinates and sizes of the constituent components
	based on the current size of the file lister.

	This function does not actually change the child widgets,  it
	just calculates the coordinates and caches the coordinates in
	the FileLister widget.

 *---------------------------------------------------------------------------*/

static void ChildrenRecalculate(flw)
XfwfFileListerWidget flw;
{
	BOX *coords;
	Widget widget;
	int i, w, h, empty_space, gap, orig_path_list_h, orig_file_list_h; 
	int orig_sel_list_h, top;
	XtWidgetGeometry parent_idea, child_idea;

	w = FSCorePart(flw)->width;
	h = FSCorePart(flw)->height;

	/* Get The Child Widgets Current Widths And Heights */

	for (i = 0; i < FS_NUM_CHILDREN; i++)
	{
		if (FSNthWidget(flw, i) != NULL)
		{
			widget = FSNthWidget(flw, i);
			coords = FSNthCoords(flw, i);
			BoxW(coords) = CoreWidth(widget);
			BoxH(coords) = CoreHeight(widget);
		}
	}

		/* Adjust Widths */

	BoxW(FSNthCoords(flw, FS_I_CUR_DIR_TEXT)) = .46 * w;
	BoxW(FSNthCoords(flw, FS_I_CUR_FILE_TEXT)) = .46 * w;
	BoxW(FSNthCoords(flw, FS_I_GOTO_BUTTON)) = .10 * w;
	BoxW(FSNthCoords(flw, FS_I_ADD_BUTTON)) = .10 * w;

	BoxW(FSNthCoords(flw, FS_I_PATH_LIST)) = .30 * w;
	BoxW(FSNthCoords(flw, FS_I_FILE_LIST)) = .30 * w;
	BoxW(FSNthCoords(flw, FS_I_SEL_LIST)) = .30 * w;

	BoxW(FSNthCoords(flw, FS_I_UP_BUTTON)) =
		BoxW(FSNthCoords(flw, FS_I_PATH_LIST)) * .35;
	BoxW(FSNthCoords(flw, FS_I_OK_BUTTON)) =
		BoxW(FSNthCoords(flw, FS_I_FILE_LIST)) * .35;
	BoxW(FSNthCoords(flw, FS_I_CANCEL_BUTTON)) =
		BoxW(FSNthCoords(flw, FS_I_FILE_LIST)) * .35;

	BoxW(FSNthCoords(flw, FS_I_PATH_BUTTON)) = .10 * w;
	BoxW(FSNthCoords(flw, FS_I_REMOVE_BUTTON)) = .10 * w;
	BoxW(FSNthCoords(flw, FS_I_CLEAR_BUTTON)) = .10 * w;


		/* Adjust Heights */

	BoxH(FSNthCoords(flw, FS_I_CUR_DIR_TEXT)) =
		max(BoxH(FSNthCoords(flw, FS_I_CUR_DIR_TEXT)), 
		BoxH(FSNthCoords(flw, FS_I_GOTO_BUTTON)));

	BoxH(FSNthCoords(flw, FS_I_GOTO_BUTTON)) =
		BoxH(FSNthCoords(flw, FS_I_CUR_DIR_TEXT));

	BoxH(FSNthCoords(flw, FS_I_PATH_BUTTON)) =
		BoxH(FSNthCoords(flw, FS_I_CUR_DIR_TEXT));

	BoxH(FSNthCoords(flw, FS_I_REMOVE_BUTTON)) =
		BoxH(FSNthCoords(flw, FS_I_CUR_DIR_TEXT));

	BoxH(FSNthCoords(flw, FS_I_CLEAR_BUTTON)) =
		BoxH(FSNthCoords(flw, FS_I_CUR_DIR_TEXT));


	BoxH(FSNthCoords(flw, FS_I_CUR_FILE_TEXT)) =
		max(BoxH(FSNthCoords(flw, FS_I_CUR_FILE_TEXT)), 
		BoxH(FSNthCoords(flw, FS_I_ADD_BUTTON)));

	BoxH(FSNthCoords(flw, FS_I_ADD_BUTTON)) =
		BoxH(FSNthCoords(flw, FS_I_CUR_FILE_TEXT));

	empty_space = h - (BoxH(FSNthCoords(flw, FS_I_TITLE)) +
			BoxH(FSNthCoords(flw, FS_I_CUR_DIR_TEXT)) +
			BoxH(FSNthCoords(flw, FS_I_CUR_FILE_TEXT)) +
			BoxH(FSNthCoords(flw, FS_I_PATH_LIST_TITLE)) +
			BoxH(FSNthCoords(flw, FS_I_OK_BUTTON)));

	gap = .025 * h;

	BoxH(FSNthCoords(flw, FS_I_PATH_LIST)) = empty_space - 8 * gap;
	BoxH(FSNthCoords(flw, FS_I_FILE_LIST)) = empty_space - 8 * gap;

	orig_path_list_h = BoxH(FSNthCoords(flw, FS_I_PATH_LIST));
	orig_file_list_h = BoxH(FSNthCoords(flw, FS_I_FILE_LIST));

	empty_space = h - (BoxH(FSNthCoords(flw, FS_I_TITLE)) +
		BoxH(FSNthCoords(flw, FS_I_CUR_DIR_TEXT)) +
		BoxH(FSNthCoords(flw, FS_I_PATH_LIST_TITLE)) +
		BoxH(FSNthCoords(flw, FS_I_OK_BUTTON)));
 
    gap = .025 * h;
 
	BoxH(FSNthCoords(flw, FS_I_SEL_LIST))  = empty_space - 7 * gap;

	orig_sel_list_h =  BoxH(FSNthCoords(flw, FS_I_SEL_LIST));


		/* Listen To Child Height Requests For Lists */

    /* FS_I_PATH_LIST */
	parent_idea.request_mode = CWWidth | CWHeight;
	parent_idea.width = BoxW(FSNthCoords(flw, FS_I_PATH_LIST));
	parent_idea.height = BoxH(FSNthCoords(flw, FS_I_PATH_LIST));
	XtQueryGeometry(FSNthWidget(flw, FS_I_PATH_LIST), 
			&parent_idea, &child_idea);

	if ((child_idea.request_mode & CWHeight) &&
		(child_idea.height < parent_idea.height))
	{
		BoxH(FSNthCoords(flw, FS_I_PATH_LIST)) = child_idea.height;
	}

	/* FS_I_FILE_LIST */
	parent_idea.request_mode = CWWidth | CWHeight;
	parent_idea.width = BoxW(FSNthCoords(flw, FS_I_FILE_LIST));
	parent_idea.height = BoxH(FSNthCoords(flw, FS_I_FILE_LIST));
	XtQueryGeometry(FSNthWidget(flw, FS_I_FILE_LIST), 
			&parent_idea, &child_idea);

	if ((child_idea.request_mode & CWHeight) &&
		(child_idea.height < parent_idea.height))
	{
		BoxH(FSNthCoords(flw, FS_I_FILE_LIST)) = child_idea.height;
	}

	/* FS_I_SEL_LIST */
	parent_idea.request_mode = CWWidth | CWHeight;
	parent_idea.width = BoxW(FSNthCoords(flw, FS_I_SEL_LIST));
	parent_idea.height = BoxH(FSNthCoords(flw, FS_I_SEL_LIST));
	XtQueryGeometry(FSNthWidget(flw, FS_I_SEL_LIST), 
		&parent_idea, &child_idea);
	if ((child_idea.request_mode & CWHeight) &&
		(child_idea.height < parent_idea.height))
		{
			BoxH(FSNthCoords(flw, FS_I_SEL_LIST)) = child_idea.height;
		}

		/* Make Sure All Three Lists Have Same Minimum Height */

	BoxH(FSNthCoords(flw, FS_I_PATH_LIST)) =
		min(BoxH(FSNthCoords(flw, FS_I_PATH_LIST)), 
			BoxH(FSNthCoords(flw, FS_I_FILE_LIST)));

	BoxH(FSNthCoords(flw, FS_I_FILE_LIST)) =
		min(BoxH(FSNthCoords(flw, FS_I_PATH_LIST)), 
			BoxH(FSNthCoords(flw, FS_I_FILE_LIST)));


		/* Vertical Positions */
		/* ------------------ */

	BoxY(FSNthCoords(flw, FS_I_TITLE)) = gap;

	BoxY(FSNthCoords(flw, FS_I_CUR_DIR_TEXT)) =
		(BoxY(FSNthCoords(flw, FS_I_TITLE)) +
		 BoxH(FSNthCoords(flw, FS_I_TITLE))) + gap;

	BoxY(FSNthCoords(flw, FS_I_GOTO_BUTTON)) =
		BoxY(FSNthCoords(flw, FS_I_CUR_DIR_TEXT));

	BoxY(FSNthCoords(flw, FS_I_CUR_FILE_TEXT)) =
		(BoxY(FSNthCoords(flw, FS_I_CUR_DIR_TEXT)) +
		 BoxH(FSNthCoords(flw, FS_I_CUR_DIR_TEXT))) + gap;

	BoxY(FSNthCoords(flw, FS_I_ADD_BUTTON)) =
		BoxY(FSNthCoords(flw, FS_I_CUR_FILE_TEXT));

	BoxY(FSNthCoords(flw, FS_I_PATH_BUTTON)) =
		BoxY(FSNthCoords(flw, FS_I_ADD_BUTTON));

	BoxY(FSNthCoords(flw, FS_I_REMOVE_BUTTON)) =
		BoxY(FSNthCoords(flw, FS_I_ADD_BUTTON));

	BoxY(FSNthCoords(flw, FS_I_CLEAR_BUTTON)) =
		BoxY(FSNthCoords(flw, FS_I_ADD_BUTTON));

	BoxY(FSNthCoords(flw, FS_I_PATH_LIST_TITLE)) =
		(BoxY(FSNthCoords(flw, FS_I_CUR_FILE_TEXT)) +
		 BoxH(FSNthCoords(flw, FS_I_CUR_FILE_TEXT))) + gap;

	BoxY(FSNthCoords(flw, FS_I_FILE_LIST_TITLE)) =
		BoxY(FSNthCoords(flw, FS_I_PATH_LIST_TITLE));

	BoxY(FSNthCoords(flw, FS_I_SEL_LIST_TITLE)) =
		BoxY(FSNthCoords(flw, FS_I_CUR_DIR_TEXT));

	BoxY(FSNthCoords(flw, FS_I_PATH_LIST)) =
		(BoxY(FSNthCoords(flw, FS_I_PATH_LIST_TITLE)) +
		 BoxH(FSNthCoords(flw, FS_I_PATH_LIST_TITLE))) + gap +
		(orig_path_list_h - BoxH(FSNthCoords(flw, FS_I_PATH_LIST))) / 2;

	BoxY(FSNthCoords(flw, FS_I_FILE_LIST)) =
		BoxY(FSNthCoords(flw, FS_I_PATH_LIST));

	BoxY(FSNthCoords(flw, FS_I_SEL_LIST)) = h -
		(BoxH(FSNthCoords(flw, FS_I_OK_BUTTON)) + ( 3.23 * gap ) +
		BoxH(FSNthCoords(flw, FS_I_SEL_LIST)));


	top = BoxY(FSNthCoords(flw, FS_I_PATH_LIST)) +
		BoxH(FSNthCoords(flw, FS_I_PATH_LIST));

	empty_space = h - top;

	BoxY(FSNthCoords(flw, FS_I_UP_BUTTON)) = top +
		(h - top - BoxH(FSNthCoords(flw, FS_I_UP_BUTTON))) / 2;

	BoxY(FSNthCoords(flw, FS_I_OK_BUTTON)) = top +
		(h - top - BoxH(FSNthCoords(flw, FS_I_OK_BUTTON))) / 2;

	BoxY(FSNthCoords(flw, FS_I_CANCEL_BUTTON)) = top +
		(h - top - BoxH(FSNthCoords(flw, FS_I_CANCEL_BUTTON))) / 2;

		/* Horizontal Positions */
		/* -------------------- */

	BoxX(FSNthCoords(flw, FS_I_TITLE)) =
		(w - BoxW(FSNthCoords(flw, FS_I_TITLE))) / 2;

	empty_space = w - (BoxW(FSNthCoords(flw, FS_I_CUR_DIR_TEXT)) +
		BoxW(FSNthCoords(flw, FS_I_GOTO_BUTTON)));

	gap = ( empty_space - (.33 * w)) / 3;

	BoxX(FSNthCoords(flw, FS_I_CUR_DIR_TEXT)) = gap;

	BoxX(FSNthCoords(flw, FS_I_GOTO_BUTTON)) =
		(BoxX(FSNthCoords(flw, FS_I_CUR_DIR_TEXT)) +
		 BoxW(FSNthCoords(flw, FS_I_CUR_DIR_TEXT))) + gap;

	empty_space = w - (BoxW(FSNthCoords(flw, FS_I_CUR_FILE_TEXT)) +
		BoxW(FSNthCoords(flw, FS_I_ADD_BUTTON)));

	gap = ( empty_space - (.33 * w)) / 3;
	BoxX(FSNthCoords(flw, FS_I_CUR_FILE_TEXT)) = gap;

	BoxX(FSNthCoords(flw, FS_I_ADD_BUTTON)) =
		(BoxX(FSNthCoords(flw, FS_I_CUR_FILE_TEXT)) +
		 BoxW(FSNthCoords(flw, FS_I_CUR_FILE_TEXT))) + gap;

	empty_space = (0.33 * w) - ( BoxW(FSNthCoords(flw, FS_I_PATH_BUTTON )) +
		BoxW( FSNthCoords(flw, FS_I_REMOVE_BUTTON )) +
		BoxW( FSNthCoords(flw, FS_I_CLEAR_BUTTON )) );

	gap = empty_space / 4;

	BoxX(FSNthCoords(flw, FS_I_PATH_BUTTON)) = (0.66 * w) + gap;

	BoxX(FSNthCoords(flw, FS_I_REMOVE_BUTTON)) = 
		BoxW(FSNthCoords(flw, FS_I_PATH_BUTTON)) +
		BoxX(FSNthCoords(flw, FS_I_PATH_BUTTON)) + gap;

	BoxX(FSNthCoords(flw, FS_I_CLEAR_BUTTON)) = 
		BoxW(FSNthCoords(flw, FS_I_REMOVE_BUTTON)) +
		BoxX(FSNthCoords(flw, FS_I_REMOVE_BUTTON)) + gap;

	empty_space = w - (BoxW(FSNthCoords(flw, FS_I_PATH_LIST)) +
		BoxW(FSNthCoords(flw, FS_I_FILE_LIST)) +
		BoxW(FSNthCoords(flw, FS_I_SEL_LIST)));

	gap = empty_space / 4;
	BoxX(FSNthCoords(flw, FS_I_PATH_LIST)) = gap;

	BoxX(FSNthCoords(flw, FS_I_FILE_LIST)) =
		(BoxX(FSNthCoords(flw, FS_I_PATH_LIST)) +
		 BoxW(FSNthCoords(flw, FS_I_PATH_LIST))) + gap;

	BoxX(FSNthCoords(flw, FS_I_SEL_LIST)) =
		(BoxX(FSNthCoords(flw, FS_I_FILE_LIST)) +
		 BoxW(FSNthCoords(flw, FS_I_FILE_LIST))) + gap;
 
	BoxX(FSNthCoords(flw, FS_I_PATH_LIST_TITLE)) =
		(BoxX(FSNthCoords(flw, FS_I_PATH_LIST)) +
		 (BoxW(FSNthCoords(flw, FS_I_PATH_LIST)) -
		  BoxW(FSNthCoords(flw, FS_I_PATH_LIST_TITLE))) / 2);

	BoxX(FSNthCoords(flw, FS_I_FILE_LIST_TITLE)) =
		(BoxX(FSNthCoords(flw, FS_I_FILE_LIST)) +
		 (BoxW(FSNthCoords(flw, FS_I_FILE_LIST)) -
		  BoxW(FSNthCoords(flw, FS_I_FILE_LIST_TITLE))) / 2);

	BoxX(FSNthCoords(flw, FS_I_SEL_LIST_TITLE)) =
		(BoxX(FSNthCoords(flw, FS_I_SEL_LIST)) +
		 (BoxW(FSNthCoords(flw, FS_I_SEL_LIST)) -
		  BoxW(FSNthCoords(flw, FS_I_SEL_LIST_TITLE))) / 2);

	empty_space = BoxW(FSNthCoords(flw, FS_I_PATH_LIST)) -
		BoxW(FSNthCoords(flw, FS_I_UP_BUTTON));

	gap = empty_space / 2;

	BoxX(FSNthCoords(flw, FS_I_UP_BUTTON)) =
		BoxX(FSNthCoords(flw, FS_I_PATH_LIST)) + gap;

	empty_space = BoxW(FSNthCoords(flw, FS_I_SEL_LIST)) -
		(BoxW(FSNthCoords(flw, FS_I_OK_BUTTON)) +
		 BoxW(FSNthCoords(flw, FS_I_CANCEL_BUTTON)));

	gap = empty_space / 3;

	BoxX(FSNthCoords(flw, FS_I_OK_BUTTON)) =
		BoxX(FSNthCoords(flw, FS_I_SEL_LIST)) + gap;

	BoxX(FSNthCoords(flw, FS_I_CANCEL_BUTTON)) =
		(BoxX(FSNthCoords(flw, FS_I_OK_BUTTON)) +
		 BoxW(FSNthCoords(flw, FS_I_OK_BUTTON))) + gap;
} /* End ChildrenRecalculate */


/*---------------------------------------------------------------------------*

	ChildrenUpdate(flw)

	This routine takes a File Lister widget <flw> and updates
	the child widgets by recalculating their coordinates and
	setting the appropriate resources.   

 *---------------------------------------------------------------------------*/

static void ChildrenUpdate(flw)
XfwfFileListerWidget flw;
{
	int i;
	Widget widget;
	BOX *coords;
	Boolean not_at_root;
	XfwfScrolledListWidget file_list;

	ChildrenRecalculate(flw);

	for (i = 0; i < FS_NUM_CHILDREN; i++)
	{
		if (FSNthWidget(flw, i) != NULL)
		{
			widget = FSNthWidget(flw, i);

			coords = FSNthCoords(flw, i);

			XtMoveWidget(widget, BoxX(coords), BoxY(coords));

			XtResizeWidget(widget, BoxW(coords), BoxH(coords), 
			       CoreBorderWidth(widget));
		}
	}

	file_list = (XfwfScrolledListWidget) FSNthWidget(flw, FS_I_FILE_LIST);

	if (FSPathListCount(flw) != 1)
		not_at_root = True;
	else
		not_at_root = False;

	XtSetSensitive( FSNthWidget(flw, FS_I_UP_BUTTON), not_at_root);

} /* End ChildrenUpdate */


/*---------------------------------------------------------------------------*

                 I N T E R N A L    C A L L B A C K S

 *---------------------------------------------------------------------------*/

static void ButtonUp(w, flw, call_data)
Widget w;
XfwfFileListerWidget flw;
XtPointer call_data;
{
	strcat(FSCurrentDirectory(flw), "..");
	Chdir(flw);
} /* End ButtonUp */


static void ButtonOk(w, flw, call_data)
Widget w;
XfwfFileListerWidget flw;
XtPointer call_data;
{
	XfwfFileListerOkButtonReturnStruct ret;
	int	i;

	ret.number_of_files = FSSelListCount(flw);

	for ( i = 0; i < FSSelListCount(flw); i++ )
	{
		ret.paths[i] = FSSelPathList(flw)[i];
		ret.files[i] = FSSelFileList(flw)[i];
	}

	XtCallCallbacks((Widget)flw, XtNokButtonCallback, (XtPointer)&ret);
} /* End ButtonOk */


static void ButtonCancel(w, flw, call_data)
Widget w;
XfwfFileListerWidget flw;
XtPointer call_data;
{
	XtCallCallbacks((Widget)flw, XtNcancelButtonCallback, NULL);
} /* End ButtonCancel */


static void ButtonGoto(w, flw, call_data)
Widget w;
XfwfFileListerWidget flw;
XtPointer call_data;
{
	Widget cur_dir_text;
	Arg args[10];
	String path;

	XtSetArg(args[0], XtNstring, (XtArgVal)(&path));
	cur_dir_text = FSNthWidget(flw, FS_I_CUR_DIR_TEXT);
	XtGetValues(cur_dir_text, args, 1);
	strcpy(FSCurrentDirectory(flw), path);
	Chdir(flw);
} /* End ButtonGoto */

static void ButtonPath(w, flw, call_data)
Widget w;
XfwfFileListerWidget flw;
XtPointer call_data;
{
	int sel_idx;

	sel_idx = FSSelListIndex(flw);
	strcpy( FSCurrentDirectory(flw), FSSelPathList(flw)[sel_idx] );
	Chdir(flw);
	XtSetSensitive( FSNthWidget( flw, FS_I_OK_BUTTON), True );


} /* End ButtonPath */
 
static void ButtonRemove(w, flw, call_data)
Widget w;
XfwfFileListerWidget flw;
XtPointer call_data;
{
	int	i;
	int sel_idx;

	sel_idx = FSSelListIndex(flw);
 
	if ( sel_idx == -1 )
	{
        	/* Click On Blank Space */
	}
	else
	{
		XtSetSensitive( FSNthWidget( flw, FS_I_PATH_BUTTON), False );
		XtSetSensitive( FSNthWidget( flw, FS_I_REMOVE_BUTTON), False );

		FSSelListCount(flw) -= 1;

		if ( FSSelListCount(flw) < 1 )
		{
			XtSetSensitive( FSNthWidget( flw, FS_I_CLEAR_BUTTON), False );
			FSSelFileList(flw)[0] = NULL;
			FSSelPathList(flw)[0] = NULL;
			free( FSSelFileList(flw)[0] );
			free( FSSelFileList(flw)[0] );
		}
		else
		{
			for ( i = sel_idx; i < FSSelListCount(flw); i++ )
			{
				strcpy( FSSelFileList(flw)[i], FSSelFileList(flw)[i + 1] );
				strcpy( FSSelPathList(flw)[i], FSSelPathList(flw)[i + 1] );
			}

			free( FSSelFileList(flw)[FSSelListCount(flw)] );
			free( FSSelFileList(flw)[FSSelListCount(flw)] );

			FSSelPathList(flw)[FSSelListCount(flw)] = NULL;
			FSSelFileList(flw)[FSSelListCount(flw)] = NULL;
		}

		XfwfScrolledListSetList(FSNthWidget(flw, FS_I_SEL_LIST),
			FSSelFileList(flw), FSSelListCount(flw), True, NULL);
	}

} /* End ButtonRemove */

static void ButtonClear(w, flw, call_data)
Widget w;
XfwfFileListerWidget flw;
XtPointer call_data;
{
	int i;

	XtSetSensitive( FSNthWidget( flw, FS_I_CLEAR_BUTTON), False );

	for ( i = 0; i < FSSelListCount(flw); i++ )
	{
		free( FSSelFileList(flw)[i] );
		free( FSSelPathList(flw)[i] );
	}

	FSSelFileList(flw)[0] = NULL;
	FSSelPathList(flw)[0] = NULL;
	FSSelListCount(flw) = 0;

	XfwfScrolledListSetList(FSNthWidget(flw, FS_I_SEL_LIST),
		FSSelFileList(flw), FSSelListCount(flw), True, NULL);


} /* End ButtonClear */

static void ButtonAdd(w, flw, call_data)
Widget w;
XfwfFileListerWidget flw;
XtPointer call_data;
{
	String file;
	String path;
	Boolean in_list = False;
	int i;
 
	file = GetFileBoxText(flw);
	path = FSCurrentDirectory(flw);

	/* make sure file isn't already in list with same path */
	for ( i = 0; i < FSSelListCount(flw); i++ )
	{
		if (( strcmp( FSSelFileList(flw)[i], file ) == 0 ) &&
			( strcmp( FSSelPathList(flw)[i], path ) == 0 ))
		{
			fprintf( stderr, 
				"FileLister: File %s already in list with same path.\n",
				file );

			in_list = True;
			break;
		}
	}

	if ( in_list == False )
	{
		/* add path and file to selected lists */
		FSSelPathList(flw)[FSSelListCount(flw)] = 
			(char *)malloc( MAXPATHLEN + 2 );
		strcpy( FSSelPathList(flw)[FSSelListCount(flw)], path );
		FSSelFileList(flw)[FSSelListCount(flw)] = 
			(char *)malloc( MAXPATHLEN + 2 );
		strcpy( FSSelFileList(flw)[FSSelListCount(flw)], file );

		FSSelListCount(flw) += 1;

		XfwfScrolledListSetList(FSNthWidget(flw, FS_I_SEL_LIST),
			FSSelFileList(flw), FSSelListCount(flw), True, NULL);

		XtSetSensitive( FSNthWidget( flw, FS_I_CLEAR_BUTTON), True );
	}

} /* End ButtonAdd */

static char *GetFileBoxText(flw)
XfwfFileListerWidget flw;
{
	char *text;
	Arg args[1];
	Widget file_box_widget;

	XtSetArg(args[0], XtNstring, (XtArgVal)(&text));
	file_box_widget = FSNthWidget(flw, FS_I_CUR_FILE_TEXT);
	XtGetValues(file_box_widget, args, 1);
	return(text);
} /* End GetFileBoxText */


static void ClickOnPathList(w, flw, call_data)
Widget w;
XfwfFileListerWidget flw;
XtPointer call_data;
{
	int i;
	XfwfScrolledListReturnStruct *ret;
	
	ret = (XfwfScrolledListReturnStruct *)call_data;

	if (ret->index == -1)
	{
		UnselectAll(flw);		/* Click On Blank Space */
		return;
	}

	strcpy(FSCurrentDirectory(flw), "/");

	for (i = 1; i <= ret->index; i++)
	{
		strcat(FSCurrentDirectory(flw), FSPathList(flw)[i]);
		strcat(FSCurrentDirectory(flw), "/");
	}

	Chdir(flw);

} /* End ClickOnPathList */


static void ClickOnFileList(w, flw, call_data)
Widget w;
XfwfFileListerWidget flw;
XtPointer call_data;
{
	XfwfScrolledListReturnStruct *ret;

	ret = (XfwfScrolledListReturnStruct *)call_data;
	if (ret->index == -1)
		UnselectAll(flw);		/* Click On Blank Space */
		else
		SelectFileByIndex(flw, ret->index);
} /* End ClickOnFileList */


static void ClickOnSelList(w, flw, call_data)
Widget w;
XfwfFileListerWidget flw;
XtPointer call_data;
{
	XfwfScrolledListReturnStruct *ret;
 
	ret = (XfwfScrolledListReturnStruct *)call_data;
	if (ret->index == -1)
	{
		XtSetSensitive( FSNthWidget( flw, FS_I_PATH_BUTTON), False );
		XtSetSensitive( FSNthWidget( flw, FS_I_REMOVE_BUTTON), False );
	}
	else
	{
		XtSetSensitive( FSNthWidget( flw, FS_I_PATH_BUTTON), True );
		XtSetSensitive( FSNthWidget( flw, FS_I_REMOVE_BUTTON), True );
	}

	FSSelListIndex(flw) = ret->index;
} /* End ClickOnSelList */

/*---------------------------------------------------------------------------*

             I N T E R N A L    S U P P O R T    R O U T I N E S

 *---------------------------------------------------------------------------*/

static void SelectFileByIndex(flw, index)
XfwfFileListerWidget flw;
int index;
{
	DirEntry *dir_entry;

	DirectoryMgrGotoItem(FSDirMgr(flw), index);
	dir_entry = DirectoryMgrCurrentEntry(FSDirMgr(flw));
	if (dir_entry == NULL)
	{
		fprintf(stderr, "SelectFileByIndex: Entry %d invalid\n", index);
		exit(-1);
	}
	if (DirEntryIsDir(dir_entry) || DirEntryIsDirectoryLink(dir_entry))
	{
		strcat(FSCurrentDirectory(flw), DirEntryFileName(dir_entry));
		Chdir(flw);
	}
	    else if (!DirEntryIsBrokenLink(dir_entry))	/* File */
	{
		strcpy(FSCurrentFile(flw), DirEntryFileName(dir_entry));
		if (FSCheckExistence(flw))
		{
			XtSetSensitive(FSNthWidget(flw, FS_I_OK_BUTTON), True);
		}
		FSFileSelected(flw) = True;
		TextWidgetSetText(FSNthWidget(flw, FS_I_CUR_FILE_TEXT), 
				  FSCurrentFile(flw));
		XfwfScrolledListHighlightItem(FSNthWidget(flw, FS_I_FILE_LIST), 
					  index);
		NotifySelectionChange(flw);
	}
	    else					/* Broken Link */
	{
		XBell(XtDisplay(flw), 0);
		UnselectAll(flw);
	}
} /* End SelectFileByIndex */


static Boolean SelectFileByName(flw, name)
XfwfFileListerWidget flw;
char *name;
{
	if (DirectoryMgrGotoNamedItem(FSDirMgr(flw), name) == FALSE)
	{
		return(False);
	}
	SelectFileByIndex(flw, DirectoryMgrCurrentIndex(FSDirMgr(flw)));
	return(True);
} /* End SelectFileByName */


static void UnselectAll(flw)
XfwfFileListerWidget flw;
{
	Boolean old_file_selected_flag;

	old_file_selected_flag = FSFileSelected(flw);
	if (FSCheckExistence(flw))
	{
		XtSetSensitive(FSNthWidget(flw, FS_I_OK_BUTTON), False);
	}
	FSCurrentFile(flw)[0] = '\0';
	FSFileSelected(flw) = False;
	TextWidgetSetText(FSNthWidget(flw, FS_I_CUR_FILE_TEXT), 
			  FSCurrentFile(flw));
	XfwfScrolledListUnhighlightAll(FSNthWidget(flw, FS_I_FILE_LIST));
	if (old_file_selected_flag) NotifySelectionChange(flw);
} /* End UnselectAll */


static void NotifySelectionChange(flw)
XfwfFileListerWidget flw;
{
	XfwfFileListerSelectionChangeReturnStruct ret;

	if (FSFileSelected(flw) == True)
	{
		ret.file_selected = True;
		ret.path = FSCurrentDirectory(flw);
		ret.file = FSCurrentFile(flw);
	}
	    else
	{
		ret.file_selected = False;
		ret.path = NULL;
		ret.file = NULL;
	}
	XtCallCallbacks((Widget)flw, XtNselectionChangeCallback, 
			(XtPointer)&ret);
} /* End NotifySelectionChange */


/*---------------------------------------------------------------------------*

	ParentizeDirectory(dir)

	This routine takes a directory pathname <dir> and modifies the
	pathname to strip off the deepest directory,  leaving just the parent
	path,  and the trailing slash.  The parent directory of the root
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

	GotoDeepestLegalDirectory(flw)

	This function takes a FileLister widget <flw> and modifies the
	directory string in FSCurrentDirectory(flw) to be the deepest
	legal directory above the string.  Partial or incorrect directory
	names are stripped starting at the end.

	This routine takes a File Lister widget <flw> and updates
	the child widgets by recalculating their coordinates and
	setting the appropriate resources.  The old directory manager
	is closed and a new one is opened for this directory.  The
	file and path lists are deallocated,  reallocated,  and loaded
	with the path and file data lists.  The lists are then reset
	in the scrolled list widgets.

	This routine requires that all the child widgets have already
	been created.

 *---------------------------------------------------------------------------*/

static void GotoDeepestLegalDirectory(flw)
XfwfFileListerWidget flw;
{
	char dir[MAXPATHLEN + 2];

	strcpy(dir, FSCurrentDirectory(flw));
	while (1)
	{
		if (!DirectoryMgrCanOpen(dir))
		{
			XBell(XtDisplay(flw), 0);
			if (strcmp(dir, "/") == 0)
			{
				fprintf(stderr, "%s: Can't open root\n", 
					"GotoDeepestLegalDirectory");
				exit(1);
			}
			ParentizeDirectory(dir);
		}
		    else
		{
			DirectoryPathExpand(dir, FSCurrentDirectory(flw));
			if (!DirectoryMgrCanOpen(dir))
				strcpy(FSCurrentDirectory(flw), dir);
			break;
		}
	}
	UnselectAll(flw);
	UpdateLists(flw);
} /* End GotoDeepestLegalDirectory */


/*---------------------------------------------------------------------------*

	UpdateLists(flw)

	This routine takes a FileLister widget and builds new path
	and file lists from the current directory,  sort mode,  and pattern
	strings.  The lists and text items are then changed.

 *---------------------------------------------------------------------------*/

static void UpdateLists(flw)
XfwfFileListerWidget flw;
{
	int i, count;
	char *dir, *start;
	DirEntry *dir_entry;
	DirectoryMgr *dir_mgr;
	char temp[MAXPATHLEN + 2];

	if (FSDirMgr(flw)) DirectoryMgrClose(FSDirMgr(flw));
	dir_mgr = DirectoryMgrSimpleOpen(FSCurrentDirectory(flw), 
					 FSSortMode(flw), 
					 FSPattern(flw));
	if (dir_mgr == NULL)
	{
		fprintf(stderr, "UpdateLists: Can't read directory '%s'\n", 
			FSCurrentDirectory(flw));
		exit(1);
	}
		
	FSDirMgr(flw) = dir_mgr;
	if (FSPathList(flw) != NULL)
	{
		for (i = 0; i < FSPathListCount(flw); i++)
			free(FSPathList(flw)[i]);
		free(FSPathList(flw));
	}
	if (FSFileList(flw) != NULL)
	{
		for (i = 0; i < FSFileListCount(flw); i++)
			free(FSFileList(flw)[i]);
		free(FSFileList(flw));
	}

	FSFileListCount(flw) = DirectoryMgrFilteredCount(FSDirMgr(flw));
	FSPathListCount(flw) = 1;
	for (dir = FSCurrentDirectory(flw) + 1; *dir != '\0'; dir++)
	{
		if (*dir == '/') ++ FSPathListCount(flw);
	}

	FSFileList(flw) = (char **)malloc(sizeof(char *) *
					  (FSFileListCount(flw) + 1));
	if (FSFileList(flw) == NULL)
	{
		fprintf(stderr, "UpdateLists: Out of memory\n");
		exit(-1);
	}

	for (i = 0; i < FSFileListCount(flw); i++)
	{
		dir_entry = DirectoryMgrNextEntry(FSDirMgr(flw));
		if (dir_entry == NULL) XtError("Inconsistent Directory");
		strcpy(temp, DirEntryFileName(dir_entry));
		if (DirEntryIsDir(dir_entry))
			strcat(temp, "/");
		    else if (DirEntryIsBrokenLink(dir_entry))
			strcat(temp, " X");
		    else if (DirEntryIsDirectoryLink(dir_entry))
			strcat(temp, "/");
		    else if (DirEntryIsSymLink(dir_entry) && FSFlagLinks(flw))
			strcat(temp, " @");

		FSFileList(flw)[i] = StrCopy(temp);
	}
	FSFileList(flw)[i] = NULL;

	FSPathList(flw) = (char **)malloc(sizeof(char *) *
					  (FSPathListCount(flw) + 1));
	start = FSCurrentDirectory(flw);
	FSPathList(flw)[0] = StrCopy("/");
	for (i = 1; i < FSPathListCount(flw); i++)
	{
		while (*start != '\0' && *start == '/') ++start;
		count = 0;
		while (*start != '\0' && *start != '/')
			temp[count++] = *start++;
		temp[count++] = '\0';
		FSPathList(flw)[i] = StrCopy(temp);
	}
	FSPathList(flw)[i] = NULL;

	XfwfScrolledListSetList(FSNthWidget(flw, FS_I_PATH_LIST), 
			    FSPathList(flw), FSPathListCount(flw), True, NULL);
	XfwfScrolledListSetList(FSNthWidget(flw, FS_I_FILE_LIST), 
			    FSFileList(flw), FSFileListCount(flw), True, NULL);
	UpdateTextLines(flw);
} /* End UpdateLists */


static void UpdateTextLines(flw)
XfwfFileListerWidget flw;
{
	TextWidgetSetText(FSNthWidget(flw, FS_I_CUR_DIR_TEXT), 
			  FSCurrentDirectory(flw));
	TextWidgetSetText(FSNthWidget(flw, FS_I_CUR_FILE_TEXT), 
			  FSCurrentFile(flw));
} /* End UpdateTextLines */


static void Chdir(flw)
XfwfFileListerWidget flw;
{
	DoBusyCursor((Widget)flw);
	GotoDeepestLegalDirectory(flw);
	ChildrenUpdate(flw);
	UndoBusyCursor((Widget)flw);
} /* End Chdir */


static void DoBusyCursor(w)
Widget w;
{
#ifndef NO_BUSY_GRAB
	if (XtIsRealized(w))
	{
		XGrabPointer(XtDisplay(w), XtWindow(w), True, None, 
			GrabModeSync, GrabModeSync, None, FSBusyCursor(w), 
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
		XUngrabPointer(XtDisplay(w), CurrentTime);
	}
#endif
} /* End UndoBusyCursor */


static void TextWidgetSetText(tw, text)
Widget tw;
char *text;
{
	Arg args[3];
	int length, insert_position;
#ifdef X11R3
	static char text_widget_storage[MAXPATHLEN + 2];
#endif

	length = strlen(text);
	insert_position = max(length, 0);
#ifdef X11R3
	/*
	XtTextSetInsertionPoint(tw, insert_position);
	XtSetArg(args[0], XtNstring, text);
	XtSetValues(tw, args, 1);
	{
		XtTextBlock block;

		block.firstPos = 0;
		block.length = length + 1;
		block.ptr = text;
		block.format = FMT8BIT;
		XtTextReplace(flw, 0, block.length - 1, &block);
	}
	*/
#else
	XtSetArg(args[0], XtNstring, text);
	XtSetValues(tw, args, 1);
	XawTextSetInsertionPoint(tw, insert_position);
#endif
} /* End TextWidgetSetText */

/*---------------------------------------------------------------------------*

                    E X T E R N A L    R O U T I N E S

 *---------------------------------------------------------------------------*/

void XfwfFileListerChangeDirectory(flw, dir)
XfwfFileListerWidget flw;
char *dir;
{
	strcpy(FSCurrentDirectory(flw), dir);
	Chdir(flw);
} /* End XfwfFileListerChangeDirectory */


void XfwfFileListerRefresh(flw)
XfwfFileListerWidget flw;
{
	XfwfFileListerChangeDirectory(flw, ".");
} /* End XfwfFileListerRefresh */


void XfwfFileListerGetStatus(flw, ssp)
XfwfFileListerWidget flw;
XfwfFileListerStatusStruct *ssp;
{	
	ssp->file_selected = FSFileSelected(flw);
	ssp->path = FSCurrentDirectory(flw);
	ssp->file = FSCurrentFile(flw);
	ssp->file_box_text = GetFileBoxText(flw);
} /* End XfwfFileListerGetStatus */
