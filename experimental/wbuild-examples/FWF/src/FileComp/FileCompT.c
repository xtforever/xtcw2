/*
    FileCompT.c : test program for the FileComplete widget
    Copyright (C) 1992,93,94 Robert Forsman

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

static char rcsid[] = "$Id: FileCompT.c,v 0.19 1993/03/11 10:53:00 thoth Exp thoth $";

/*
 * $Log: FileCompT.c,v $
 * Revision 0.19  1993/03/11  10:53:00  thoth
 * Fix some pointer type mismatches.
 *
 * Revision 0.18  93/01/19  12:01:06  thoth
 * important stuff is now prefixed with xfwf (I hope)
 * 
 * Revision 0.17  92/08/24  13:59:29  thoth
 * Totty's initial stab at the Xfwf version of
 * the release.
 * 
 * Revision 0.16  92/07/12  15:08:27  thoth
 * adjusted test program to follow new FWF conventions.
 * 
 * Revision 0.15  92/06/08  22:17:11  thoth
 * portability enhancements using the S_IS* macros.
 * 
 * Revision 0.14  92/05/20  17:13:12  thoth
 * this was released with FWF 3.0
 * 
 * Revision 0.13  92/05/07  12:34:27  thoth
 * Added menu entries to test out the tilde backup filters.
 * 
 * Revision 0.12  92/05/06  11:08:58  thoth
 * adding rcsid[]
 * 
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/AsciiText.h>

#include <Xfwf/FileComp.h>

Widget outer, fc, form, fileproc, statsproc, pattern;
XtAppContext	app_con;

void print_filename(w, client_data, call_data)
     Widget	w;
     XtPointer  client_data, call_data;
{
  printf("the user has selected file \"%s\"\n", call_data);
}

void print_filename_2(w, client_data, call_data)
     Widget	w;
     XtPointer  client_data, call_data;
{
  char	*s;
  XtVaGetValues((Widget)client_data, XtNfilename, &s, (String)0);
  printf("current value is \"%s\"\n", s);
}

void quit()
{
  exit(0);
}

void change_file_proc(w, client_data, call_data)
     Widget	w;
     XtPointer	call_data, client_data;
{
  String	s;
  XtVaGetValues(pattern, XtNstring, &s, (String)0);
  XtVaSetValues(fc,
		XtNacceptableFilenameProc, client_data,
		XtNfilenameProcData, s,
		(String)0);
  XfwfFileCompleteForceRescan(fc);
}

void add_file_proc(parent, label, proc, sensitive)
     Widget	parent;
     char	*label;
     XfwfFCAcceptableFilenameProc	proc;
     int	sensitive;
{
  Widget	temp;
  temp = XtVaCreateManagedWidget
    ("entry", smeBSBObjectClass, parent,
     XtNlabel, label,
     XtNsensitive, sensitive,
     (String)0);
  XtAddCallback(temp, XtNcallback, change_file_proc, proc);
}

Boolean noprograms(filename, filestats)
     char	*filename;
     struct stat	*filestats;
{
  if ( S_ISREG(filestats->st_mode) &&
      (filestats->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0)
    return False;
  else
    return True;
}

void change_stat_proc(w, client_data, call_data)
     Widget	w;
     XtPointer	call_data, client_data;
{
  String	s;
  XtVaGetValues(pattern, XtNstring, &s, (String)0);
  XtVaSetValues(fc,
		XtNacceptableStatsProc, client_data,
		XtNstatsProcData, s,
		(String)0);
  XfwfFileCompleteForceRescan(fc);
}

void add_stat_proc(parent, label, proc, sensitive)
     Widget	parent;
     char	*label;
     XfwfFCAcceptableStatsProc	proc;
     int	sensitive;
{
  Widget	temp;
  temp = XtVaCreateManagedWidget
    ("entry", smeBSBObjectClass, parent,
     XtNlabel, label,
     XtNsensitive, sensitive,
     (String)0);
  XtAddCallback(temp, XtNcallback, change_stat_proc, proc);
}

int main (argc,argv)
     int	argc;
     char	**argv;
{
  Widget	temp;

  outer = XtAppInitialize(&app_con,"FileCompT",NULL,0,
			  &argc,argv,NULL,NULL,0);

  form = XtVaCreateManagedWidget("form", formWidgetClass, outer,
				 (String)0);
  fc = XtVaCreateManagedWidget
    ("selector", xfwffileCompleteWidgetClass, form,
     (String)0);
  XtAddCallback(fc, XtNcallback, print_filename, (XtPointer)0);
  XtAddCallback(fc, XtNcancelCallback, quit, (XtPointer)0);
  XfwfFileCompleteAddButton(fc, "check", print_filename_2, fc);

  /* create the filename filters menu */
  fileproc = XtVaCreateManagedWidget
    ("filename", menuButtonWidgetClass, form,
     XtNfromVert, fc,
     (String)0);
  temp = XtVaCreatePopupShell
    ("menu", simpleMenuWidgetClass, fileproc,
     (String)0);
  add_file_proc(temp, "any", (XtPointer)0, 1);
  add_file_proc(temp, "no .files", XfwfFCNoDotFiles, 1);
  add_file_proc(temp, "no tilde backups", XfwfFCNoTildeBackups, 1);
  add_file_proc(temp, "no tilde backups or .files",
		XfwfFCNoTildeBackupsOrDotFiles, 1);
  add_file_proc(temp, "regex", XfwfFCRegexFiles,
		XfwfFileCompleteRegexFlavor()!=xfwfFC_NoRegex);
  add_file_proc(temp, "glob", XfwfFCGlobFiles,
		XfwfFileCompleteHasGlob());

  /* create the file stats filters menu */
  statsproc = XtVaCreateManagedWidget
    ("stats", menuButtonWidgetClass, form,
     XtNfromVert, fc,
     XtNfromHoriz, fileproc,
     (String)0);
  temp = XtVaCreatePopupShell
    ("menu", simpleMenuWidgetClass, statsproc,
     (String)0);
  add_stat_proc(temp, "any", (XtPointer)0, 1);
  add_stat_proc(temp, "dirs or regex", XfwfFCDirsOrRegexFiles,
		XfwfFileCompleteRegexFlavor()!=xfwfFC_NoRegex);
  add_stat_proc(temp, "dirs or glob", XfwfFCDirsOrGlobFiles,
		XfwfFileCompleteHasGlob());
  add_stat_proc(temp, "no programs", noprograms, 1);

#if 1
  /* create the pattern entry widget for the filename filters menu */
  pattern = XtVaCreateManagedWidget
    ("pattern", asciiTextWidgetClass, form,
     XtNfromVert, fileproc,
     XtNeditType, XawtextEdit,
     (String)0);
#endif

  XtRealizeWidget(outer);

  XtAppMainLoop(app_con);

  return 0;
}
