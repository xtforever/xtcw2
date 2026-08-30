/*
 * FChooser.c : A widget for choosing a file
 *
 * George Ferguson, ferguson@cs.rochester.edu, 21 Jan 1993.
 *
 * This code is derived from the FileSelector widget by Brian Totty,
 * hence the following copyright applies:
 *
 * Copyright 1990,1991,1992 Brian Totty
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Brian Totty or
 * University of Illinois not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.  Brian Totty and University of Illinois make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * Brian Totty and University of Illinois disclaim all warranties with
 * regard to this software, including all implied warranties of
 * merchantability and fitness, in no event shall Brian Totty or
 * University of Illinois be liable for any special, indirect or
 * consequential damages or any damages whatsoever resulting from loss of
 * use, data or profits, whether in an action of contract, negligence or
 * other tortious action, arising out of or in connection with the use or
 * performance of this software.
 *
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

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <X11/CompositeP.h>
#include <X11/Composite.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/List.h>

#include <Xfwf/DirMgr.h>
#include <Xfwf/FChooserP.h>
#include <Xfwf/FChooser.h>

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
static void		ChildrenUpdate();
static void		DirectoryCallback();
static void		FileCallback();
static void		SelectFileByIndex();
static Boolean		SelectFileByName();
static void		UnselectAll();
static void		Notify();
static void		GotoDeepestLegalDirectory();
static void		UpdateLists();
static void		Chdir();

#else

static void		Initialize(Widget request, Widget new);
static void		Realize(Widget w, XtValueMask *valueMask,
				XSetWindowAttributes *attrs);
static void		Destroy(XfwfFileChooserWidget fcw);
static void		Resize(Widget w);
static Boolean		SetValues(Widget current, Widget request, Widget new);
static XtGeometryResult	GeometryManager(Widget w,
				XtWidgetGeometry *request,
				XtWidgetGeometry *reply);
static void		ChildrenCreate(XfwfFileChooserWidget fcw);
static void		ChildrenRealize(XfwfFileChooserWidget fcw);
static void		ChildrenUpdate(XfwfFileChooserWidget fcw);
static void		DirectoryCallback(Widget w, XtPointer client_data,
				XtPointer call_data);
static void		FileCallback(Widget w, XtPointer client_data,
				XtPointer call_data);
static void		SelectFileByIndex(XfwfFileChooserWidget fcw, int index);
static Boolean		SelectFileByName(XfwfFileChooserWidget fcw, char *name);
static void		UnselectAll(XfwfFileChooserWidget fcw);
static void		Notify(XfwfFileChooserWidget fcw);
static void		GotoDeepestLegalDirectory(XfwfFileChooserWidget fcw);
static void		UpdateLists(XfwfFileChooserWidget fcw);
static void		Chdir(XfwfFileChooserWidget fcw);

#endif

/*---------------------------------------------------------------------------*

              R E S O U R C E    I N I T I A L I Z A T I O N

 *---------------------------------------------------------------------------*/

#define FCFieldOffset(FIELD)	XtOffset(XfwfFileChooserWidget,fileChooser.FIELD)
#define CoreFieldOffset(FIELD)	XtOffset(Widget,core.FIELD)

static XtResource resources[] = {
    { XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
	 CoreFieldOffset(width), XtRString, "500" },
    { XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
	 CoreFieldOffset(height), XtRString, "250" },
    { XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
	 CoreFieldOffset(background_pixel), XtRString, "white" },
    { XtNcurrentDirectory, XtCPathname, XtRString, sizeof(String),
	 FCFieldOffset(current_dir), XtRString, NULL },
    { XtNcurrentFile, XtCFilename, XtRString, sizeof(String),
	 FCFieldOffset(current_file), XtRString, NULL },
    { XtNcallback, XtCCallback, XtRCallback,
	 sizeof(XtCallbackList), FCFieldOffset(callbacks), XtRCallback, NULL},
    {XtNsortMode, XtCValue, XtRInt, sizeof(int),
	 FCFieldOffset(sort_mode), XtRString, "2"},
    {XtNpattern, XtCFile, XtRString, sizeof(String),
	 FCFieldOffset(pattern), XtRString, NULL},
};

#undef FCFieldOffset
#undef CoreFieldOffset

/*---------------------------------------------------------------------------*

                    C L A S S    A L L O C A T I O N

 *---------------------------------------------------------------------------*/

XfwfFileChooserClassRec xfwfFileChooserClassRec = {
    {
	/* superclass		*/	(WidgetClass)&compositeClassRec,
	/* class_name		*/	"XfwfFileChooser",
	/* widget_size		*/	sizeof(XfwfFileChooserRec),
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

WidgetClass xfwfFileChooserWidgetClass =
	(WidgetClass)&xfwfFileChooserClassRec;

/*---------------------------------------------------------------------------*

                    E X P O R T E D    M E T H O D S

 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*

	Initialize()

	This procedure is called by the X toolkit to initialize
	the FileChooser widget instance.  The hook to this routine is in the
	initialize part of the core part of the class.

 *---------------------------------------------------------------------------*/

/* ARGSUSED */
static void
Initialize(request,new)
Widget request,new;
{
    XfwfFileChooserWidget fcw = (XfwfFileChooserWidget)new;
    char *str,*initial_file,path[MAXPATHLEN + 2];
    static char *star = "*";

    FCBusyCursor(fcw) = XCreateFontCursor(XtDisplay(fcw),XC_watch);
    FCDirMenu(fcw) = NULL;
    FCDirMenuButton(fcw) = NULL;
    FCFileList(fcw) = NULL;
    FCDirMgr(fcw) = NULL;
    FCFileNames(fcw) = NULL;
    FCNumFileNames(fcw) = 0;
    FCDirNames(fcw) = NULL;
    FCNumDirNames(fcw) = 0;

    str = (char *)XtCalloc((MAXPATHLEN + 2),sizeof(char));
    if (FCCurrentDirectory(fcw) != NULL) {	/* User Specified Path */
	strcpy(str,FCCurrentDirectory(fcw));
    } else {
	getwd(path);
	strcpy(str,path);
    }
    FCCurrentDirectory(fcw) = str;

    str = (char *)XtCalloc((MAXPATHLEN + 2),sizeof(char));
    initial_file = FCCurrentFile(fcw);
    FCCurrentFile(fcw) = str;

    if (FCCorePart(request)->width <= 0)
	FCCorePart(new)->width = 500;
    if (FCCorePart(request)->height <= 0)
	FCCorePart(new)->height = 200;

    if (FCPattern(fcw) == NULL)
	FCPattern(fcw) = star;
    FCPattern(fcw) = XtNewString(FCPattern(fcw));

    ChildrenCreate(fcw);
    GotoDeepestLegalDirectory(fcw);
    if (initial_file)
	SelectFileByName(fcw,initial_file);
} /* End Initialize */

/*---------------------------------------------------------------------------*

	Realize()

	This function is called to realize a FileChooser widget.

 *---------------------------------------------------------------------------*/

static void Realize(w,valueMask,attrs)
Widget w;
XtValueMask *valueMask;
XSetWindowAttributes *attrs;
{
    XfwfFileChooserWidget fcw = (XfwfFileChooserWidget)w;

    XtCreateWindow(w,InputOutput,(Visual *)CopyFromParent,*valueMask,attrs);
    ChildrenRealize(fcw);
    ChildrenUpdate(fcw);
    Resize(w);
    Notify(fcw);
} /* End Realize */

/*---------------------------------------------------------------------------*

	Destroy()

	This function is called to destroy a FileChooser widget.

 *---------------------------------------------------------------------------*/

static void
Destroy(fcw)
XfwfFileChooserWidget fcw;
{
    int i;

    XtFree(FCCurrentDirectory(fcw));
    XtFree(FCCurrentFile(fcw));
    XtFree(FCPattern(fcw));
    if (FCFileNames(fcw) != NULL) {
	for (i=0; i < FCNumFileNames(fcw); i++)
	    XtFree(FCFileNames(fcw)[i]);
	XtFree((void*)FCFileNames(fcw));
    }
    if (FCDirNames(fcw) != NULL) {
	for (i=0; i < FCNumDirNames(fcw); i++)
	    XtFree((void*)FCDirNames(fcw)[i]);
	XtFree((void*)FCDirNames(fcw));
    }
} /* End Destroy */

/*---------------------------------------------------------------------------*

	Resize()

	This function is called to resize a FileChooser widget.

 *---------------------------------------------------------------------------*/

static void
Resize(w) 
Widget w;
{
    XfwfFileChooserWidget fcw = (XfwfFileChooserWidget)w;

    ChildrenUpdate(fcw);
} /* End Resize */

/*---------------------------------------------------------------------------*

	SetValues()

	This function is the external interface for setting resources.

 *---------------------------------------------------------------------------*/

/* ARGSUSED */
static Boolean
SetValues(current,request,new)
Widget current,request,new;
{
    XfwfFileChooserWidget fcw_current = (XfwfFileChooserWidget)current;
    XfwfFileChooserWidget fcw_new = (XfwfFileChooserWidget)new;
    XfwfFileChooserWidget fcw_request = (XfwfFileChooserWidget)request;

    if (FCCurrentDirectory(fcw_current) != FCCurrentDirectory(fcw_new)){
	strcpy(FCCurrentDirectory(fcw_current),FCCurrentDirectory(fcw_new));
	FCCurrentDirectory(fcw_new) = FCCurrentDirectory(fcw_current);
	Chdir(fcw_new);
    }
    if (FCCurrentFile(fcw_current) != FCCurrentFile(fcw_new)) {
	char *new_name;

	new_name = FCCurrentFile(fcw_new);
	FCCurrentFile(fcw_new) = FCCurrentFile(fcw_current);
	SelectFileByName(fcw_new,new_name);
    }
    if (FCPattern(fcw_current) != FCPattern(fcw_new)) {
	XtFree(FCPattern(fcw_current));
	FCPattern(fcw_new) = XtNewString(FCPattern(fcw_current));
	Chdir(fcw_new);
    }
    if (FCSortMode(fcw_current) != FCSortMode(fcw_new)) {
	Chdir(fcw_new);
    }
    return(False);
} /* End SetValues() */

/*---------------------------------------------------------------------------*

	GeometryManager(w,request,reply)

	This routine acts as the geometry_manager method for the
	FileChooser widget.  It is called when a child wants to
	resize/reposition itself.

	Currently, we allow all requests.

 *---------------------------------------------------------------------------*/

/* ARGSUSED */
static XtGeometryResult
GeometryManager(w,request,reply)
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

	ChildrenCreate(fcw)

	This routine creates the initial child widgets for the
	file selector widget and places them in the widget fcw.
	No placement or resizing is done.  That is done by
	ChildrenUpdate().

 *---------------------------------------------------------------------------*/

static void
ChildrenCreate(fcw)
XfwfFileChooserWidget fcw;
{
    Arg args[4];
    static char *str = NULL;

    XtSetArg(args[0],XtNmenuName,FC_DIR_MENU_NAME);
    FCDirMenuButton(fcw) =
	XtCreateManagedWidget(FC_DIR_MENU_BUTTON_NAME,menuButtonWidgetClass,
			      (Widget)fcw,args,1);
    XtSetArg(args[0],XtNallowVert,True);
    XtSetArg(args[1],XtNforceBars,True);
    FCFileViewport(fcw) =
	XtCreateManagedWidget(FC_FILE_VIEWPORT_NAME,viewportWidgetClass,
			      (Widget)fcw,args,2);
    XtSetArg(args[0],XtNlist,&str);
    XtSetArg(args[1],XtNdefaultColumns,1);
    XtSetArg(args[2],XtNforceColumns,True);
    XtSetArg(args[3],XtNverticalList,True);
    FCFileList(fcw) = XtCreateManagedWidget(FC_FILE_LIST_NAME,listWidgetClass,
					    FCFileViewport(fcw),args,4);
    XtAddCallback(FCFileList(fcw),XtNcallback,
		  (XtCallbackProc)FileCallback,(XtPointer)fcw);
} /* End ChildrenCreate */

/*---------------------------------------------------------------------------*

	ChildrenRealize(fcw)

	This routine realizes the child widgets.  The widgets must
	already have been created and initialized.  Their coordinates
	should already have been set.

 *---------------------------------------------------------------------------*/

static void
ChildrenRealize(fcw)
XfwfFileChooserWidget fcw;
{
    XtRealizeWidget(FCDirMenuButton(fcw));
    XtRealizeWidget(FCFileViewport(fcw));
    XtRealizeWidget(FCFileList(fcw));
} /* End ChildrenRealize */

/*---------------------------------------------------------------------------*

	ChildrenUpdate(fcw)

	This routine takes a FileChooser widget fcw and updates
	the child widgets by recalculating their coordinates based
	on the current size of the FileChooser, and setting the
	appropriate resources.

	We go to some trouble to get something useful displayed in the
	FCDirMenuButton(fcw) if the current directory name is too long
	to fit in the button. It would be nice if the "justify" resource
	of the MenuButton widget did this, but...

 *---------------------------------------------------------------------------*/

static void
ChildrenUpdate(fcw)
XfwfFileChooserWidget fcw;
{
    Dimension w,h;
    int gap;
    Dimension menuButtonW,menuButtonH,fileListW,fileListH;
    Position menuButtonX,menuButtonY,fileListX,fileListY;
    XtWidgetGeometry parent_idea,child_idea;
    Arg args[3];
    XFontStruct *fs;
    char *label;
    Dimension intw;

    if (!XtIsRealized((Widget)fcw)) {
	return;
    }
    w = CoreWidth(fcw);
    h = CoreHeight(fcw);
    gap = 3;
    /* Get The Child Widgets Current Widths And Heights */
    /* (although we don't actually use the existing values... */
    menuButtonW = CoreWidth(FCDirMenuButton(fcw));
    menuButtonH = CoreHeight(FCDirMenuButton(fcw));
    fileListW = CoreWidth(FCFileList(fcw));
    fileListH = CoreHeight(FCFileList(fcw));
    /* Adjust Widths */
    menuButtonW = w;
    fileListW = w;
    /* Adjust menu button label if too small */
    /* It would be nice if the "justify" resource of MenuButton did this... */
    XtSetArg(args[0],XtNinternalWidth,&intw);
    XtSetArg(args[1],XtNfont,&fs);
    XtGetValues(FCDirMenuButton(fcw),args,2);
    label = FCCurrentDirectory(fcw);
    if (XTextWidth(fs,label,strlen(label)) > menuButtonW-intw) {
	char newLabel[MAXPATHLEN];
	while (*label && XTextWidth(fs,label,strlen(label))>menuButtonW-intw) {
	    label += 1;
	}
	if (*label)
	    label += 1;
	strcpy(newLabel,"<");
	strcat(newLabel,label);
	XtSetArg(args[0],XtNlabel,newLabel);
	XtSetValues(FCDirMenuButton(fcw),args,1);
    }
    /* Adjust Heights */
    fileListH = h - menuButtonH - gap;
    /* Listen To Child Height Request For List */
    parent_idea.request_mode = CWWidth | CWHeight;
    parent_idea.width = fileListW;
    parent_idea.height = fileListH;
    XtQueryGeometry(FCFileList(fcw),&parent_idea,&child_idea);
    if ((child_idea.request_mode & CWHeight) &&
	(child_idea.height < parent_idea.height)) {
	fileListH = child_idea.height;
    }
    /* Vertical Positions */
    menuButtonY = 0;
    fileListY = menuButtonH + gap;
    /* Horizontal Positions */
    menuButtonX = 0;
    fileListX = 0;
    /* Move them */
    XtMoveWidget(FCDirMenuButton(fcw),menuButtonX,menuButtonY);
    XtMoveWidget(FCFileViewport(fcw),fileListX,fileListY);
    /* Resize them */
    XtResizeWidget(FCDirMenuButton(fcw),menuButtonW,menuButtonH,
		   CoreBorderWidth(FCDirMenuButton(fcw)));
    XtResizeWidget(FCFileViewport(fcw),fileListW,fileListH,
		   CoreBorderWidth(FCFileViewport(fcw)));
} /* End ChildrenUpdate */

/*---------------------------------------------------------------------------*

                 I N T E R N A L    C A L L B A C K S

 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*

	DirectoryCallback(w,client_data,call_data)

	This is called when the user selects an ancestor directory from
	the menu. The argument "w" is the selected object (hence the two
	calls to XtParent()) and "client_data" is it's index in the menu.

 *---------------------------------------------------------------------------*/

/*ARGSUSED*/
static void
DirectoryCallback(w,client_data,call_data)
Widget w;
XtPointer client_data;	/* index */
XtPointer call_data;	/* not used */
{
    int index = (int)client_data;
    XfwfFileChooserWidget fcw = (XfwfFileChooserWidget)XtParent(XtParent(w));
    int i;

    strcpy(FCCurrentDirectory(fcw),"/");
    for (i = 1; i <= index; i++) {
	strcat(FCCurrentDirectory(fcw),FCDirNames(fcw)[i]);
	strcat(FCCurrentDirectory(fcw),"/");
    }
    Chdir(fcw);
} /* End DirectoryCallback */

/*---------------------------------------------------------------------------*

	FileCallback(w,client_data,call_data)

	This is called when the user selects a file in the fileList.
	The argument "client_data" is the FileChooser widget, "call_data"
	is a pointer to the standard List widget callback information.

 *---------------------------------------------------------------------------*/

/*ARGSUSED*/
static void
FileCallback(w,client_data,call_data)
Widget w;
XtPointer client_data;	/* fcw */
XtPointer call_data;	/* return struct */
{
    XfwfFileChooserWidget fcw = (XfwfFileChooserWidget)client_data;
    XawListReturnStruct *ret = (XawListReturnStruct *)call_data;

    if (ret->list_index == -1) {
	UnselectAll(fcw);		/* Click On Blank Space */
	Notify(fcw);
    } else {
	SelectFileByIndex(fcw,ret->list_index);
    }
} /* End FileCallback */

/*---------------------------------------------------------------------------*

             I N T E R N A L    S U P P O R T    R O U T I N E S

 *---------------------------------------------------------------------------*/

static void
SelectFileByIndex(fcw,index)
XfwfFileChooserWidget fcw;
int index;
{
    DirEntry *dir_entry;

    DirectoryMgrGotoItem(FCDirMgr(fcw),index);
    if ((dir_entry=DirectoryMgrCurrentEntry(FCDirMgr(fcw))) == NULL) {
	fprintf(stderr,"SelectFileByIndex: Entry %d invalid\n",index);
	exit(-1);
    }
    if (DirEntryIsDir(dir_entry) || DirEntryIsDirectoryLink(dir_entry)) {
	strcat(FCCurrentDirectory(fcw),DirEntryFileName(dir_entry));
	Chdir(fcw);
    } else if (!DirEntryIsBrokenLink(dir_entry)) {	/* File */
	strcpy(FCCurrentFile(fcw),DirEntryFileName(dir_entry));
	XawListHighlight(FCFileList(fcw),index);
	Notify(fcw);
    } else {						/* Broken Link */
	XBell(XtDisplay(fcw),0);
	UnselectAll(fcw);
    }
} /* End SelectFileByIndex */

static Boolean
SelectFileByName(fcw,name)
XfwfFileChooserWidget fcw;
char *name;
{
    if (DirectoryMgrGotoNamedItem(FCDirMgr(fcw),name) == FALSE) {
	return(False);
    }
    SelectFileByIndex(fcw,DirectoryMgrCurrentIndex(FCDirMgr(fcw)));
    return(True);
} /* End SelectFileByName */

static void
UnselectAll(fcw)
XfwfFileChooserWidget fcw;
{
    Boolean selected = FCCurrentFile(fcw)[0] != '\0';

    FCCurrentFile(fcw)[0] = '\0';
    XawListUnhighlight(FCFileList(fcw));
    if (selected)
	Notify(fcw);
} /* End UnselectAll */

static void
Notify(fcw)
XfwfFileChooserWidget fcw;
{
    XfwfFileChooserReturnStruct ret;

    if (FCCurrentFile(fcw)[0] != '\0') {
	ret.directory = FCCurrentDirectory(fcw);
	ret.file = FCCurrentFile(fcw);
    } else {
	ret.directory = NULL;
	ret.file = NULL;
    }
    XtCallCallbacks((Widget)fcw,XtNcallback,(XtPointer)&ret);
} /* End Notify */

/*---------------------------------------------------------------------------*

	GotoDeepestLegalDirectory(fcw)

	This function takes a FileChooser widget <fcw> and modifies the
	directory string in FCCurrentDirectory(fcw) to be the deepest
	legal directory above the string.  Partial or incorrect directory
	names are stripped starting at the end.

	It then calls UpdateLists() to reset the information dislayed in
	the FileChooser.

 *---------------------------------------------------------------------------*/

static void
GotoDeepestLegalDirectory(fcw)
XfwfFileChooserWidget fcw;
{
    char *dir,*end;
    char temp[MAXPATHLEN + 2];

    dir = FCCurrentDirectory(fcw);
    for (end=dir; *end != '\0'; ++end)
	/*EMPTY*/;
    while (1) {
	if (DirectoryPathExpand(dir,temp) == NULL) {
	    while (*end != '/' && end != dir) {
		end -= 1;
	    }
	    *end = '\0';
	} else {
	    strcpy(FCCurrentDirectory(fcw),temp);
	    break;
	}
    }
    UnselectAll(fcw);
    UpdateLists(fcw);
} /* End GotoDeepestLegalDirectory */

/*---------------------------------------------------------------------------*

	UpdateLists(fcw)

	This routine resets the information displayed in a FileChooser
	widget by doing the following:
	1. It changes the cursor to the value of the "busyCursor" resource.
	2. The old directory manager (FCDirMgr(fcw)) is closed and a
	   new one opened based on the value of FCCurrentDirectory(fcw).
	3. Any old string arrays are freed, and any existing directory
	   menu is destroyed.
	4. A new array of strings (FCFileNames(fcw)) is allocated and filled
	   with the names of the files in the directory, then this array
	   is used to set what is displayed in the FCFileList(fcw) widget.
	5. A new array of strings (FCDirNames(fcw)) is allocated and filled
	   with the names of the ancestor directories. These are also used
	   to create a new FCDirMenu(fcw), and the label FCDirMenuButton(fcw)
	   is set to the name of the directory.
	6. Finally, the cursor is restored.

 *---------------------------------------------------------------------------*/

static void
UpdateLists(fcw)
XfwfFileChooserWidget fcw;
{
    Arg args[1];
    int i,count;
    char *dir,*start;
    DirEntry *dir_entry;
    char temp[MAXPATHLEN + 2];
    Widget menuItem;

    if (XtIsRealized((Widget)fcw)) {
	/* This is puke-ola. */
	XDefineCursor(XtDisplay(fcw),XtWindow(fcw),FCBusyCursor(fcw));
	XDefineCursor(XtDisplay(fcw),XtWindow(FCFileList(fcw)),
		      FCBusyCursor(fcw));
	XDefineCursor(XtDisplay(fcw),XtWindow(FCDirMenuButton(fcw)),
		      FCBusyCursor(fcw));
	XFlush(XtDisplay(fcw));
    }
    if (FCDirMgr(fcw))
	DirectoryMgrClose(FCDirMgr(fcw));
    FCDirMgr(fcw) = DirectoryMgrSimpleOpen(FCCurrentDirectory(fcw),
					   FCSortMode(fcw),FCPattern(fcw));

    /* Throw away old info */
    if (FCFileNames(fcw) != NULL) {
	for (i = 0; i < FCNumFileNames(fcw); i++)
	    XtFree(FCFileNames(fcw)[i]);
	XtFree((char *)FCFileNames(fcw));
    }
    if (FCDirNames(fcw) != NULL) {
	for (i = 0; i < FCNumDirNames(fcw); i++)
	    XtFree(FCDirNames(fcw)[i]);
	XtFree((char *)FCDirNames(fcw));
    }
    if (FCDirMenu(fcw) != NULL)
	XtDestroyWidget(FCDirMenu(fcw));

    /* Count how many files and dirs we have now */
    FCNumFileNames(fcw) = DirectoryMgrFilteredCount(FCDirMgr(fcw));
    FCNumDirNames(fcw) = 1;
    for (dir=FCCurrentDirectory(fcw)+1; *dir != '\0'; dir++) {
	if (*dir == '/') 
	    FCNumDirNames(fcw) += 1;
    }

    /* Make the array of filenames and set the fileList widget */
    FCFileNames(fcw) = (char **)XtCalloc(FCNumFileNames(fcw)+1,sizeof(char *));
    for (i=0; i < FCNumFileNames(fcw); i++) {
	dir_entry = DirectoryMgrNextEntry(FCDirMgr(fcw));
	if (dir_entry == NULL)
	    XtError("Inconsistent Directory");
	strcpy(temp,DirEntryFileName(dir_entry));
	if (DirEntryIsDir(dir_entry))
	    strcat(temp,"/");
	else if (DirEntryIsBrokenLink(dir_entry))
	    strcat(temp," X");
	else if (DirEntryIsDirectoryLink(dir_entry))
	    strcat(temp,"/");
	else if (DirEntryIsSymLink(dir_entry))
	    strcat(temp," @");
	FCFileNames(fcw)[i] = XtNewString(temp);
    }
    FCFileNames(fcw)[i] = NULL;
    XawListChange(FCFileList(fcw),FCFileNames(fcw),0,0,True);

    /* Make the array of dirnames and build a new dirMenu widget */
    FCDirNames(fcw) = (char **)XtCalloc(FCNumDirNames(fcw)+1,sizeof(char *));
    FCDirNames(fcw)[0] = XtNewString("/");
    FCDirMenu(fcw) = XtCreatePopupShell(FC_DIR_MENU_NAME,simpleMenuWidgetClass,
					(Widget)fcw,NULL,0);
    menuItem = XtCreateManagedWidget("/",smeBSBObjectClass,FCDirMenu(fcw),
				     NULL,0);
    XtAddCallback(menuItem,XtNcallback,DirectoryCallback,(XtPointer)0);
    start = FCCurrentDirectory(fcw);
    for (i = 1; i < FCNumDirNames(fcw); i++) {
	while (*start != '\0' && *start == '/')
	    start += 1;
	count = 0;
	while (*start != '\0' && *start != '/')
	    temp[count++] = *start++;
	temp[count] = '\0';
	FCDirNames(fcw)[i] = XtNewString(temp);
	XtSetArg(args[0],XtNlabel,temp);
	menuItem = XtCreateManagedWidget("dirMenuItem",smeBSBObjectClass,
					 FCDirMenu(fcw),args,1);
        XtAddCallback(menuItem,XtNcallback,DirectoryCallback,(XtPointer)i);
    }
    XtSetArg(args[0],XtNlabel,FCCurrentDirectory(fcw));
    XtSetValues(FCDirMenuButton(fcw),args,1);
    if (XtIsRealized((Widget)fcw)) {
	XUndefineCursor(XtDisplay(fcw),XtWindow(fcw));
	XUndefineCursor(XtDisplay(fcw),XtWindow(FCFileList(fcw)));
	XUndefineCursor(XtDisplay(fcw),XtWindow(FCDirMenuButton(fcw)));
    }
} /* End UpdateLists */


static void Chdir(fcw)
XfwfFileChooserWidget fcw;
{
    GotoDeepestLegalDirectory(fcw);
    ChildrenUpdate(fcw);
} /* End Chdir */

/*---------------------------------------------------------------------------*

                    E X T E R N A L    R O U T I N E S

 *---------------------------------------------------------------------------*/

void
XfwfFileChooserChangeDirectory(fcw,dir)
XfwfFileChooserWidget fcw;
char *dir;
{
    strcpy(FCCurrentDirectory(fcw),dir);
    Chdir(fcw);
} /* End XfwfFileChooserChangeDirectory */

void
XfwfFileChooserRefresh(fcw)
XfwfFileChooserWidget fcw;
{
    XfwfFileChooserChangeDirectory(fcw,".");
} /* End XfwfFileChooserRefresh */

char *
XfwfFileChooserCurrentDirectory(fcw)
XfwfFileChooserWidget fcw;
{
    return(FCCurrentDirectory(fcw));
} /* End XfwfFileChooserCurrentDirectory */

char *
XfwfFileChooserCurrentFile(fcw)
XfwfFileChooserWidget fcw;
{
    return(FCCurrentFile(fcw));
} /* End XfwfFileChooserCurrentFile */

