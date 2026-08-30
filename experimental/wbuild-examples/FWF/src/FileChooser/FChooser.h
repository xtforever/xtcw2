/*
 * FChooser.h : Public header file for the FileChooser widget
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

#ifndef _FCHOOSER_H
#define _FCHOOSER_H

#include <Xfwf/DirMgr.h>

extern WidgetClass xfwfFileChooserWidgetClass;

typedef struct _XfwfFileChooserClassRec	*XfwfFileChooserWidgetClass;
typedef struct _XfwfFileChooserRec	*XfwfFileChooserWidget;

#ifndef XtNcurrentDirectory
#define	XtNcurrentDirectory		"currentDirectory"
#endif

#define	XtNcurrentFile			"currentFile"
#define	XtNsortMode			"sortMode"
#define	XtNpattern			"pattern"

#define	XtCPathname			"Pathname"
#define	XtCFilename			"Filename"

typedef struct _XfwfFileChooserReturnStruct
{
	char *directory;
	char *file;
} XfwfFileChooserReturnStruct;

/*---------------------------------------------------------------------------*

                  E X T E R N A L    F U N C T I O N S

 *---------------------------------------------------------------------------*/

#if (!NeedFunctionPrototypes)

void	XfwfFileChooserChangeDirectory();
void	XfwfFileChooserRefresh();
char	*XfwfFileChooserCurrentDirectory();
char	*XfwfFileChooserCurrentFile();

#else

void	XfwfFileChooserChangeDirectory(XfwfFileChooserWidget fcw, char *dir);
void	XfwfFileChooserRefresh(XfwfFileChooserWidget fcw);
char	*XfwfFileChooserCurrentDirectory(XfwfFileChooserWidget fcw);
char	*XfwfFileChooserCurrentFile(XfwfFileChooserWidget fcw);

#endif /* !NeedFunctionPrototypes */
#endif /* !_FCHOOSER_H */
