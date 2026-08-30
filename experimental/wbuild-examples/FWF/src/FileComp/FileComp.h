/*
    FileComp.h: public header file for a file selector widget
    Copyright (C) 1992,93,94 Robert Forsman

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */

/*
 * $Log: FileComp.h,v $
 * Revision 0.17  1993/01/19  12:00:36  thoth
 * important stuff is now prefixed with xfwf (I hope)
 *
 * Revision 0.16  92/07/12  15:04:28  thoth
 * almost every externally visible type and symbol is prefixed with Xfwf now.
 * Also solved wacky unPOSIX problem with Ultrix.  Not really their fault.
 * 
 * Revision 0.15  92/05/20  18:01:53  thoth
 * removed a secret parameter from a function typedef
 * 
 * Revision 0.14  92/05/20  17:12:06  thoth
 * this was released with FWF 3.0
 * 
 * Revision 0.13  92/05/12  13:40:31  thoth
 * added prototype for new XfwfFileCompleteTrimToPath
 * convenience procedure.
 * 
 * Revision 0.12  92/05/07  12:34:02  thoth
 * Added prototypes for new acceptableFilenameProcs that filter
 * out tilde backups.
 * 
 * Revision 0.11  92/05/06  10:54:00  thoth
 * still Beta release
 * 
 * Revision 0.10  92/05/06  10:47:47  thoth
 * still
 * 
 */

#ifndef _FileComplete_h
#define _FileComplete_h

#ifdef __cplusplus
extern "C" {
#endif

struct stat;			/* pacify gcc */

typedef struct _XfwfFileCompleteClassRec *XfwfFileCompleteWidgetClass;
extern WidgetClass xfwffileCompleteWidgetClass;
typedef struct _XfwfFileCompleteRec *XfwfFileCompleteWidget;

#define XtNfilename			"filename"
#define XtNsortProc			"sortProc"

#define XtNstatThreshold		"statThreshold"
#define XtNacceptableFilenameProc	"acceptableFilenameProc"
#define XtNfilenameProcData		"filenameProcData"
#define XtNacceptableStatsProc		"acceptableStatsProc"
#define XtNstatsProcData		"statsProcData"

#define XtNokLabel			"okLabel"
#define XtNshowOkButton			"showOkButton"

#define XtNcancelLabel			"cancelLabel"

#ifndef XtNcancelCallback
#define XtNcancelCallback		"cancelCallback"
#endif
  
#define XtNshowCancelButton		"showCancelButton"

#define XtNfollowLinks			"followLinks"
#define XtNdirnamesOk			"dirnamesOk"
#define XtNstatLinks			"statLinks"

#define XtCFilename	"Filename"
#define XtCSortProc			"SortProc"

#define XtCStatThreshold                "StatThreshold"
#define XtCAcceptableFilenameProc       "AcceptableFilenameProc"
#define XtCFilenameProcData		"FilenameProcData"
#define XtCAcceptableStatsProc          "AcceptableStatsProc"
#define XtCStatsProcData		"StatsProcData"
	                                
#define XtCShowButton                   "ShowButton"
	                                
#define XtCFollowLinks                  "FollowLinks"
#define XtCDirnamesOk                   "DirnamesOk"
#define XtCStatLinks                    "StatLinks"

#define XtRAcceptableFileProc		"AcceptableFileProc"
#define XtRAcceptableStatsProc		"AcceptableStatsProc"
#define XtRFollowLinks                  "FollowLinks"

/* this is really an internal structure */
typedef struct {
  char	*filename;
  char	*repr;
} Xfwffileent;

typedef void (*XfwfFCSortProc)(
#if NeedFunctionPrototypes
Xfwffileent *base,
int nfiles
#endif
);

/**********************************************************************/

typedef Boolean (*XfwfFCAcceptableFilenameProc)(
#if NeedFunctionPrototypes
char *filename,
XtPointer client_data
#endif
);

#define XfwfFCAnyFilename	((XfwfFCAcceptableFilenameProc)0)

/* this is a hack -- the globbing process requires a different way of
   processing the filenames -- to make the user interface consistent,
   the same resource is used to activate the globbing as well as the
   other filename filters. */
#define XfwfFCGlobFiles		((XfwfFCAcceptableFilenameProc)1)

extern Boolean XfwfFCNoDotFiles(
#if NeedFunctionPrototypes
char *filename,
XtPointer ignored
#endif
);

extern Boolean XfwfFCNoTildeBackups(
#if NeedFunctionPrototypes
char *filename,
XtPointer ignored
#endif
);

extern Boolean XfwfFCNoTildeBackupsOrDotFiles(
#if NeedFunctionPrototypes
char *filename,
XtPointer ignored
#endif
);

extern Boolean XfwfFCRegexFiles(
#if NeedFunctionPrototypes
char *filename,
_Xconst XtPointer regex,
XtAppContext super_secret_parameter___death_to_programmers_who_use_this
#endif
);

/**********************************************************************/

typedef Boolean (*XfwfFCAcceptableStatsProc)(
#if NeedFunctionPrototypes
char *filename,
struct stat *filestats,
XtPointer client_data,
XtAppContext super_secret_parameter___death_to_programmers_who_use_this
#endif
);

extern Boolean XfwfFCDirsOrRegexFiles(
#if NeedFunctionPrototypes
char *filename,
struct stat *filestats,
XtPointer regex,
XtAppContext super_secret_parameter___death_to_programmers_who_use_this
#endif
);

extern Boolean XfwfFCDirsOrGlobFiles(
#if NeedFunctionPrototypes
char *filename,
struct stat *filestats,
XtPointer regex,
XtAppContext super_secret_parameter___death_to_programmers_who_use_this
#endif
);

#define XfwfFCAnyStats	((XfwfFCAcceptableStatsProc)0)

/**********************************************************************/

enum XfwfFCFollowLinks {
  XfwfFCNever, XfwfFCWhenShorter, XfwfFCAlways
};

/* procedures to return the widgets that form the subparts of the
   FileComplete widget.  Don't do anything stupid with them. */

Widget XfwfFileCompleteText(
#if NeedFunctionPrototypes
Widget	w
#endif
);
 
Widget XfwfFileCompleteStatus(
#if NeedFunctionPrototypes
Widget	w
#endif
);
 
Widget XfwfFileCompleteFileList(
#if NeedFunctionPrototypes
Widget	w
#endif
);
 
Widget XfwfFileCompleteBox(
#if NeedFunctionPrototypes
Widget	w
#endif
);

/* Convenience procedure to add a command button to the FileComplete
   widget. */

Widget XfwfFileCompleteAddButton(
#if NeedFunctionPrototypes
     Widget	w,
     char	*name,
     XtCallbackProc	func,
     XtPointer	client_data
#endif
);

/* Convenience procedure to eliminate any filename that might have
   been typed.  Good for clearing out a FileComplete widget that has
   been used before.  It preserves any directory elements so that the
   user doesn't have to navigate the filesystem again. */
 
void XfwfFileCompleteTrimToPath(
#if NeedFunctionPrototypes
Widget	w
#endif
);

/* Some functions programmers can use to see what pattern matching
   capabilities the FileComplete widget was compiled with. */

enum xfwfFileCompleteRegexFlavor {
  xfwfFC_NoRegex,
  xfwfFC_ATT,
  xfwfFC_Emacs
};

enum xfwfFileCompleteRegexFlavor
  XfwfFileCompleteRegexFlavor();

int XfwfFileCompleteHasGlob();

/* Force a rescan of the directory.  If you're using a fixed buffer
   for parameters to the Regex and Glob convenience function, you need
   to do this after an XtSetValues. (the pointer hasn't changed, just
   what it points to). */

void XfwfFileCompleteForceRescan(
#if NeedFunctionPrototypes
Widget w
#endif
);

#ifdef __cplusplus
}
#endif

#endif
