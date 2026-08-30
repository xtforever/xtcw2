/*****************************************************************************

	FileLister.h

	This file contains the user includes for the FileLister widget.

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

/* The following constant specifies the maximum number of files to be
   selected using the file lister widget.               */
#define MAX_POSSIBLE_FILES  120

#ifndef _FILELISTER_H_
#define _FILELISTER_H_

#include <Xfwf/ScrList.h>
#include <Xfwf/DirMgr.h>

extern WidgetClass	xfwfFileListerWidgetClass;

typedef struct _XfwfFileListerClassRec *XfwfFileListerWidgetClass;
typedef struct _XfwfFileListerRec      *XfwfFileListerWidget;

#define	XtNpathname XtNcurrentDirectory		/* For Compatibility */

#ifndef XtNtitle
#define	XtNtitle			"title"
#endif

#ifndef XtNcurrentDirectory
#define	XtNcurrentDirectory		"currentDirectory"
#endif

#define	XtNcurrentFile			"currentFile"
#define	XtNsortMode			"sortMode"
#define	XtNpattern			"pattern"
#define	XtNokButtonCallback		"okButtonCallback"
#define	XtNcancelButtonCallback		"cancelButtonCallback"
#define	XtNselectionChangeCallback	"selectionChangeCallback"
#define	XtNshowOkButton			"showOkButton"
#define	XtNshowCancelButton		"showCancelButton"
#define	XtNfileSelected			"fileSelected"
#define	XtNflagLinks			"flagLinks"
#define	XtNcheckExistence		"checkExistence"

#define	XtCPathname			"Pathname"
#define	XtCFilename			"Filename"

typedef struct _XfwfFileListerOkButtonReturnStruct
{
	char *paths[MAX_POSSIBLE_FILES];
	char *files[MAX_POSSIBLE_FILES];
	int  number_of_files;
} XfwfFileListerOkButtonReturnStruct;

typedef struct _XfwfFileListerSelectionChangeReturnStruct
{
	Boolean file_selected;
	char *path;
	char *file;
} XfwfFileListerSelectionChangeReturnStruct;

typedef struct _XfwfFileListerStatusStruct
{
	Boolean file_selected;
	char *path;
	char *file;
	char *file_box_text;
} XfwfFileListerStatusStruct;

/*---------------------------------------------------------------------------*

                  E X T E R N A L    F U N C T I O N S

 *---------------------------------------------------------------------------*/

#if (!NeedFunctionPrototypes)

void	XfwfFileListerChangeDirectory();
void	XfwfFileListerRefresh();
void	XfwfFileListerGetStatus();

#else

void	XfwfFileListerChangeDirectory(XfwfFileListerWidget fsw, char *dir);
void	XfwfFileListerRefresh(XfwfFileListerWidget fsw);
void	XfwfFileListerGetStatus(XfwfFileListerWidget fsw,
				  XfwfFileListerStatusStruct *ssp);

#endif
#endif
