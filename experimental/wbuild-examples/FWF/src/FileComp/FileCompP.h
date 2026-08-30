/*
    FileCompP.h: public header file for a file selector widget
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
 * $Log: FileCompP.h,v $
 * Revision 0.14  1993/01/19  12:00:42  thoth
 * important stuff is now prefixed with xfwf (I hope)
 *
 * Revision 0.13  92/08/24  13:59:21  thoth
 * Totty's initial stab at the Xfwf version of
 * the release.
 * 
 * Revision 0.12  92/07/12  15:05:52  thoth
 * almost every externally visible type and symbol is prefixed with Xfwf now.
 * 
 * Revision 0.11  92/05/06  10:54:03  thoth
 * still Beta release
 * 
 * Revision 0.10  92/05/06  10:47:48  thoth
 * still
 * 
 */

#ifndef _FileCompleteP_h
#define _FileCompleteP_h

#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#ifndef _POSIX_PATH_MAX
#include <sys/param.h>
#define _POSIX_PATH_MAX	 	MAXPATHLEN
#endif
#include <Xfwf/FileComp.h>
#include <X11/Xaw/FormP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XfwfFileCompleteClassPart {
  char	foo; /* I can't think of what to put here, help me out guys */
} XfwfFileCompleteClassPart;

typedef struct _FileCompleteClassRec {
  CoreClassPart		core_class;
  CompositeClassPart	composite_class;
  ConstraintClassPart	constraint_class;
  FormClassPart		form_class;
  XfwfFileCompleteClassPart	xfwf_filecomp_class;
} XfwfFileCompleteClassRec;

extern XfwfFileCompleteClassRec xfwffileCompleteClassRec;

typedef char	pathT[_POSIX_PATH_MAX+1];

typedef struct _XfwfFileCompletePart {
  /* public stuff */
  char	*filename_;
  XfwfFCSortProc	sortProc;

  int	statThreshold;
  XfwfFCAcceptableFilenameProc		acceptFilenameProc;
  XtPointer				filenameProcData;
  XfwfFCAcceptableStatsProc			acceptStatsProc;
  XtPointer				statsProcData;

  String	okLabel;
  XtCallbackList	callback;
  Boolean	showOkButton;

  String	cancelLabel;
  XtCallbackList	cancelCallback;
  Boolean	showCancelButton;

  Boolean		dirnamesOk;
  Boolean		statLinks;
  enum XfwfFCFollowLinks	followLinks;

  /* internal state info	******************************* */

  Widget	text_input;
  Widget	status_label;
  XtIntervalId	msg_timeout;
  Widget	scrolled_list;
  Widget	button_box;
  Widget	ok_button;
  Widget	cancel_button;

  pathT		_filename;

  pathT		currdir;
  pathT		currfn;
  struct stat	cdstats;

  /* list of files in the current directory */
  Xfwffileent	*files;
  int		fileslen;
  int		filessize;

  /* list of entries displayed in the scrolledList */
  String	*candidates;
  int		*candidx;

  /* for background statting of long directories */
  int		last_statted_file;
  XtWorkProcId	bgstat;

  /* to implement double click on the scrolledList */
  String	last_selected_filename;
  int		last_selected_index;
  struct timeval time_last_selected;

} XfwfFileCompletePart;

typedef struct _XfwfFileCompleteRec {
  CorePart		core;
  CompositePart		composite;
  ConstraintPart	constraint;
  FormPart		form;
  XfwfFileCompletePart	xfwf_filecomp;
} XfwfFileCompleteRec;


#ifdef __cplusplus
}
#endif

#endif
