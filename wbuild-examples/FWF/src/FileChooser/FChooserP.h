/*
 * FChooserP.h : Private header file for the FileChooser widget
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

#ifndef	_FCHOOSERP_H
#define	_FCHOOSERP_H

#include <Xfwf/DirMgr.h>

#include <X11/CoreP.h>
#include <X11/Core.h>

/*---------------------------------------------------------------------------*

			    C O N S T A N T S

 *---------------------------------------------------------------------------*/

#define FC_DIR_MENU_NAME	"dirMenu"
#define FC_DIR_MENU_BUTTON_NAME	"dirMenuButton"
#define FC_FILE_VIEWPORT_NAME	"fileViewport"
#define FC_FILE_LIST_NAME	"fileList"

/*---------------------------------------------------------------------------*

      S T R U C T U R E   &   W I D G E T    A C C E S S    M A C R O S

 *---------------------------------------------------------------------------*/

#define	FCCorePart(w)		(&((w)->core))
#define	FCCompositePart(w)	(&((w)->composite))
#define	FCFCPart(w)		(&((w)->fileChooser))

#define	FCDirMgr(w)		(FCFCPart(w)->dir_mgr)
#define	FCDirMenu(w)		(FCFCPart(w)->dir_menu)
#define	FCDirMenuButton(w)	(FCFCPart(w)->dir_menu_button)
#define	FCFileViewport(w)	(FCFCPart(w)->file_viewport)
#define	FCFileList(w)		(FCFCPart(w)->file_list)
#define	FCCurrentDirectory(w)	(FCFCPart(w)->current_dir)
#define	FCCurrentFile(w)	(FCFCPart(w)->current_file)
#define	FCDirNames(w)		(FCFCPart(w)->dir_names)
#define	FCNumDirNames(w)	(FCFCPart(w)->num_dir_names)
#define	FCFileNames(w)		(FCFCPart(w)->file_names)
#define	FCNumFileNames(w)	(FCFCPart(w)->num_file_names)
#define	FCBusyCursor(w)		(FCFCPart(w)->busy_cursor)
#define	FCSortMode(w)		(FCFCPart(w)->sort_mode)
#define	FCPattern(w)		(FCFCPart(w)->pattern)

#define CoreWidth(w)            ((w)->core.width)
#define CoreHeight(w)           ((w)->core.height)
#define CoreBorderWidth(w)      ((w)->core.border_width)

/*---------------------------------------------------------------------------*

           W I D G E T    S T R U C T U R E    D E F I N I T I O N

 *---------------------------------------------------------------------------*/

typedef struct
{
	DIRECTORY_MGR	*dir_mgr;
	Widget		dir_menu;
	Widget		dir_menu_button;
	Widget		file_viewport;
	Widget		file_list;
	char		*current_dir;
	char		*current_file;
	char		**dir_names;
	int		num_dir_names;
	char		**file_names;
	int		num_file_names;
	XtCallbackList	callbacks;
	Cursor		busy_cursor;
	int		sort_mode;
	char		*pattern;
} XfwfFileChooserPart;

typedef struct _XfwfFileChooserClassPart
{
	int    empty;
} XfwfFileChooserClassPart;

typedef struct _XfwfFileChooserClassRec
{
	CoreClassPart			core_class;
	CompositeClassPart		composite_class;
	XfwfFileChooserClassPart	fileChooser_class;
} XfwfFileChooserClassRec;

	/* This Is What A Widget Instance Points To */

typedef struct _XfwfFileChooserRec
{
	CorePart		core;
	CompositePart		composite;
	XfwfFileChooserPart	fileChooser;
} XfwfFileChooserRec;

extern XfwfFileChooserClassRec xfwfFileChooserClassRec;

#endif /* !_FCHOOSERP_H */
