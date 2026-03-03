/*
    FileComp.c: source code for a file selector widget
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

static char copyright[] = "FileComplete: file selector widget\nCopyright (C) 1992,93,94 Robert Forsman\nGNU Library General Public License\n";

static char rcsid[] = "$Id: FileComp.c,v 0.27 1993/04/20 10:49:56 thoth Exp thoth $";

/*
 * $Log: FileComp.c,v $
 * Revision 0.27  1993/04/20  10:49:56  thoth
 * We never want this widget to be a file widget.
 *
 * Revision 0.26  93/03/11  10:52:17  thoth
 * Fix some pointer type mismatches.  There are certainly some left.
 * 
 * Revision 0.25  93/01/19  12:00:17  thoth
 * important stuff is now prefixed with xfwf (I hope)
 * 
 * Revision 0.24  92/08/24  15:25:42  thoth
 * A vain attempt to placate lint
 * 
 * Revision 0.23  92/08/24  14:03:41  thoth
 * lint-like patches big thanks to
 * Bert Bos <bert@let.rug.nl>
 * 
 * Revision 0.22  92/08/24  13:58:16  thoth
 * Totty's initial stab at the Xfwf version of
 * the release.
 * 
 * Revision 0.21  92/07/12  15:02:46  thoth
 * almost every externally visible type and symbol is prefixed with
 * Xfwf now.
 * Also solved wacky unPOSIX problem with Ultrix.  Not really their fault.
 * 
 * Revision 0.20  92/06/30  10:58:00  thoth
 * used the toolkit's XtNewString instead of strdup.
 * fixed prototype error for regex stuff.
 * 
 * Revision 0.19  92/06/08  22:15:12  thoth
 * portability enhancements using the S_IS* macros.
 * use the d_ino member of the struct dirent instead of
 * Sun's d_fileno.
 * 
 * Revision 0.18  92/05/20  18:01:07  thoth
 * adjusted to handle change in ScrolledList widget.
 * provided my own LowerCase because it doesn't exist
 * under R5.
 * 
 * Revision 0.17  92/05/20  17:11:50  thoth
 * this was released with FWF 3.0
 * 
 * Revision 0.16  92/05/12  13:34:35  thoth
 * commented some procedures,
 * minor fix to geometry management,
 * added XfwfFielCompleteTrimToPath
 * convenience procedure, used in the
 * modified xedit.  This procedure will
 * probably be used in many applications.
 * 
 * Revision 0.15  92/05/08  16:27:22  thoth
 * uncovered and fixed a SetValues bug.
 * uncovered and fixed some HEINOUS
 * child layout bugs.
 * 
 * Revision 0.14  92/05/07  12:33:27  thoth
 * Added new convenience procedures for acceptableFilenameProcs
 * that filter out tilde backups.
 * 
 * Revision 0.13  92/05/06  12:50:10  thoth
 * fixed a copyright message
 * 
 * Revision 0.12  92/05/06  12:06:36  thoth
 * I'm still getting used to RCS, don'tcha know.
 * 
 * Revision 0.11.2.1  92/05/06  10:58:56  thoth
 * still fixing rcsid[]
 * 
 * Revision 0.11.1.1  92/05/06  10:56:20  thoth
 * still fixing rcsid[]
 * 
 * Revision 0.11  92/05/06  10:53:58  thoth
 * still Beta release
 * 
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#include <X11/Xfuncproto.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/SimpleP.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Box.h>
#include <Xfwf/ScrList.h>

#include <Xfwf/FileCompP.h>
#ifdef USEONELINE
#include "OneLineText.h"
#endif
#if defined(USE_REGEX) && defined(GNU_REGEX)
#include <regex.h>
#endif
#if defined(USE_GLOB)
#include <glob.h>
#endif

#ifdef SVR4                                     /* BB, 27 Mar '96 */
#define bcopy(b1, b2, length) memcpy(b2, b1, length)
#define bzero(b, length) memset(b, 0, length)
#endif

#define offset(field) XtOffset(XfwfFileCompleteWidget, xfwf_filecomp.field)
static XtResource resources[] = {
  {XtNfilename, XtCFilename, XtRString, sizeof(String),
     offset(filename_), XtRString, (XtPointer) "./"},
  {XtNsortProc, XtCSortProc, XtRFunction, sizeof(XfwfFCSortProc),
     offset(sortProc), XtRImmediate, (XtPointer)0},

  {XtNstatThreshold, XtCStatThreshold, XtRInt, sizeof(int),
     offset(statThreshold), XtRImmediate, (XtPointer)32},
  {XtNacceptableFilenameProc, XtCAcceptableFilenameProc, XtRAcceptableFileProc,
     sizeof(XfwfFCAcceptableFilenameProc), offset(acceptFilenameProc),
     XtRImmediate, (XtPointer)0},
  {XtNfilenameProcData, XtCFilenameProcData, XtRString, sizeof(String),
     offset(filenameProcData), XtRImmediate, (XtPointer)0},
  {XtNacceptableStatsProc, XtCAcceptableStatsProc, XtRAcceptableStatsProc,
     sizeof(XfwfFCAcceptableStatsProc), offset(acceptStatsProc),
     XtRImmediate, (XtPointer)0},
  {XtNstatsProcData, XtCStatsProcData, XtRString, sizeof(String),
     offset(statsProcData), XtRImmediate, (XtPointer)0},
				                                   
  {XtNokLabel, XtCLabel, XtRString, sizeof(String),
     offset(okLabel), XtRString, (XtPointer)"Ok"},
  {XtNcallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
     offset(callback), XtRImmediate, (XtPointer)0},
  {XtNshowOkButton, XtCShowButton, XtRBoolean, sizeof(Boolean),
     offset(showOkButton), XtRImmediate, (XtPointer)TRUE},
  /* XtRBoolean for the default value crashes in
     lib/Xt/Resources.c:GetResources line 776 at the XtBCopy.
     wierd */

  {XtNcancelLabel, XtCLabel, XtRString, sizeof(String),
     offset(cancelLabel), XtRString, (XtPointer)"Cancel"},
  {XtNcancelCallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
     offset(cancelCallback), XtRImmediate, (XtPointer)0},
  {XtNshowCancelButton, XtCShowButton, XtRBoolean, sizeof(Boolean),
     offset(showCancelButton), XtRImmediate, (XtPointer)TRUE},

  {XtNdirnamesOk, XtCDirnamesOk, XtRBoolean, sizeof(Boolean),
     offset(dirnamesOk), XtRImmediate, (XtPointer)FALSE},
  {XtNstatLinks, XtCStatLinks, XtRBoolean, sizeof(Boolean),
     offset(statLinks), XtRImmediate, (XtPointer)TRUE}, 
  {XtNfollowLinks, XtCFollowLinks, XtRFollowLinks,
     sizeof(enum XfwfFCFollowLinks), offset(followLinks),
     XtRImmediate, (XtPointer)XfwfFCNever}, 
};

#undef offset

static void ClassInitialize(), Initialize(), Destroy(), GetValuesHook();
static Boolean SetValues(), Layout();
static void ItemSelected();
static void AttemptFileCompletion(), Notify(), AuxNotify(), Cancel();

static XtActionsRec	actions[] = {
  "complete", AttemptFileCompletion,
  "notify", Notify
};

XfwfFileCompleteClassRec xfwffileCompleteClassRec = {
{ /* core_class fields */
    /* superclass         */    (WidgetClass) &formClassRec,
    /* class_name         */    "FileComplete",
    /* widget_size        */    sizeof(XfwfFileCompleteRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_init    */    NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */    NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    actions,
    /* num_actions        */    XtNumber(actions),
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    SetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */    GetValuesHook,
    /* accept_focus       */    NULL,
    /* version            */    XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry     */	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension          */	NULL
  },
  { /* composite_class fields */
    /* geometry_manager   */   XtInheritGeometryManager,
    /* change_managed     */   XtInheritChangeManaged,
    /* insert_child       */   XtInheritInsertChild,
    /* delete_child       */   XtInheritDeleteChild,
    /* extension          */   NULL
  },
  { /* constraint_class fields */
    /* subresourses       */   NULL,
    /* subresource_count  */   0,
    /* constraint_size    */   sizeof(FormConstraintsRec),
    /* initialize         */   NULL,
    /* destroy            */   NULL,
    /* set_values         */   NULL,
    /* extension          */   NULL
  },
  { /* form_class fields */
    /* layout             */   Layout
  },
  { /* filecomp_class fields */
    /* foo		  */   0
  },
};

WidgetClass xfwffileCompleteWidgetClass =
  (WidgetClass)&xfwffileCompleteClassRec;

String olxlations = "\
#override\n\
<Key>space: complete(optlist)\n\
<Key>Tab: complete()\n\
<Key>Return: notify()\n\
Ctrl<Key>m: notify()\n\
Ctrl<Key>j: notify()\n\
";

#define L_READY		""
#define L_SCANNING	"scanning directory"
#define L_STATTING	"statting files"
#define L_NOCOMPLETE	"no completions"
#define L_UNIQUE	"sole completion"
#define L_NONUNIQUE	"complete but not unique"
#define L_ERRORDIR	"error opening directory"

Boolean CvtStringToAcceptableFileProc(), CvtStringToAcceptableStatsProc(),
  CvtStringToFollowLinks();



static void
ClassInitialize()
     /* load up some type converters from String to
	+ AcceptableFileProc
	+ AcceptableStatsProc
	+ FollowLinks
	*/
{
  XtSetTypeConverter(XtRString, XtRAcceptableFileProc,
		     CvtStringToAcceptableFileProc, NULL, 0,
		     XtCacheAll, NULL);
  XtSetTypeConverter(XtRString, XtRAcceptableStatsProc,
		     CvtStringToAcceptableStatsProc, NULL, 0,
		     XtCacheAll, NULL);
  XtSetTypeConverter(XtRString, XtRFollowLinks,
		     CvtStringToFollowLinks, NULL, 0,
		     XtCacheAll, NULL);
}



static void ChoosePrefix();
static void UpdateFilesList();

#if defined(USE_REGEX) && defined(GNU_REGEX)
static struct re_pattern_buffer compbuf;
#endif

static void
Initialize(request, new)
     Widget	request, new;
     /* clear out all the private fields.
	create the subwidgets and configure their callbacks.
	load up the initial directory listing. */
{
  XfwfFileCompleteWidget	fcw = (XfwfFileCompleteWidget)new;
  XfwfFileCompletePart *fcwp = &fcw->xfwf_filecomp;

  /* clean out the private parts */
  fcwp->msg_timeout = 0;
  *fcwp->currdir = '\0';
  strcpy(fcwp->currfn, "/");
  fcwp->cdstats.st_ino = 0;
  fcwp->files = NULL;
  fcwp->fileslen = fcwp->filessize = 0;
  fcwp->candidates = NULL;
  fcwp->candidx = NULL;
  fcwp->last_statted_file = 0;
  fcwp->bgstat = 0;
  fcwp->last_selected_filename = NULL;
  fcwp->last_selected_index = -1;
  fcwp->time_last_selected.tv_sec = 0;
  strcpy(fcwp->_filename, fcwp->filename_);
  fcwp->filename_ = fcwp->_filename;

  fcwp->text_input = XtVaCreateManagedWidget
    ("text",
#ifdef USEONELINE
     oneLineTextWidgetClass,
#else
     asciiTextWidgetClass,
#endif
     (Widget) fcw,
     XtNeditType, XawtextEdit,
     XtNtop, XtChainTop,
     XtNbottom, XtChainTop,
     XtNleft, XtChainLeft,
     XtNright, XtChainRight,
     XtNstring, fcwp->_filename,
     XtNtype, XawAsciiString,
     XtNinsertPosition, strlen(fcwp->_filename),
     (String)0);

  /* if you override the defaults, be sure to replicate the functionality */
  if (!fcw->core.accelerators) {
    fcw->core.accelerators = XtParseAcceleratorTable(olxlations);
    XtInstallAccelerators(fcwp->text_input, (Widget) fcw);
  }

  fcwp->status_label = XtVaCreateManagedWidget
    ("status", labelWidgetClass, (Widget) fcw,
     XtNlabel, "initing",
     XtNfromVert, fcwp->text_input,
     XtNtop, XtChainTop,
     XtNbottom, XtChainTop,
     XtNleft, XtChainLeft,
     XtNright, XtChainRight,
     XtNborderWidth, 0,
     XtNjustify, XtJustifyLeft,
     (String)0);

  fcwp->scrolled_list = XtVaCreateManagedWidget
    ("files", xfwfScrolledListWidgetClass, (Widget) fcw,
     XtNfromVert, fcwp->status_label,
     XtNtop, XtChainTop,
     XtNbottom, XtChainBottom,
     XtNleft, XtChainLeft,
     XtNright, XtChainRight,
     (String)0);
  XtAddCallback(fcwp->scrolled_list, XtNcallback, ItemSelected, new);

  fcwp->button_box = XtVaCreateManagedWidget
    ("buttons", boxWidgetClass, (Widget) fcw,
     XtNfromVert, fcwp->status_label,
     XtNfromHoriz, fcwp->scrolled_list,
     XtNtop, XtChainTop,
     XtNbottom, XtChainBottom,
     XtNleft, XtChainRight,
     XtNright, XtChainRight,
     XtNresizable, TRUE,
     (String)0);

  if (fcwp->showOkButton) {
    fcwp->ok_button = XtVaCreateManagedWidget
      ("ok", commandWidgetClass, fcwp->button_box,
       XtNlabel, fcwp->okLabel,
       (String)0);
    XtAddCallback(fcwp->ok_button, XtNcallback, AuxNotify, fcw);
  } else
    fcwp->ok_button = NULL;
    
  if (fcwp->showCancelButton) {
    fcwp->cancel_button = XtVaCreateManagedWidget
      ("cancel", commandWidgetClass, fcwp->button_box,
       XtNlabel, fcwp->cancelLabel,
       (String)0);
    XtAddCallback(fcwp->cancel_button, XtNcallback, Cancel, fcw);
  } else
    fcwp->cancel_button = NULL;

#if defined(USE_REGEX) && defined(GNU_REGEX)
  /* this code initializes the compile buffer for the regex routines */
  compbuf.buffer = XtMalloc(256);
  compbuf.allocated = 256;
  compbuf.fastmap = compbuf.translate = NULL;
#endif

  UpdateFilesList(new);
  ChoosePrefix(new);

  XtSetKeyboardFocus((Widget) fcw, fcwp->text_input);
}



static void Destroy(w)
     XfwfFileCompleteWidget	w;
     /* The FileComplete can have quite a few resources that need
	to be cleaned up.  It can have the label clearing timeout,
	it can have the statting workproc, it has quite a bit of
	dynamic memory...
	*/
{
  XfwfFileCompletePart	*fcwp = &w->xfwf_filecomp;

  if (fcwp->msg_timeout)
    XtRemoveTimeOut(fcwp->msg_timeout);
  /* failure to remove the timeout could cause a real mess */

  if (fcwp->bgstat)
    XtRemoveWorkProc(fcwp->bgstat);
  /* as could failure to remove the workproc */

  if (fcwp->files!=NULL) {
    int	i;
    for (i=0; i<fcwp->fileslen; i++) {
      if (fcwp->files[i].filename != fcwp->files[i].repr)
	free(fcwp->files[i].repr);
      free(fcwp->files[i].filename);
    }
  }

  if (fcwp->candidates)
    free(fcwp->candidates);
  if (fcwp->candidx)
    free(fcwp->candidx);
#if defined(USE_REGEX) && defined(GNU_REGEX)
  if (compbuf.buffer) {	  /* free the compile buffer from the regex routines */
    XtFree(compbuf.buffer);
    compbuf.buffer = NULL;
  }
#endif
}

/* this is used to detect a double-click.
   The 0.8sec should be configurable. */
static int within_time(tp)
     struct timeval	*tp;
{
  struct timeval	curr;
  long	diff;
  gettimeofday(&curr, NULL);
  diff = curr.tv_usec - tp->tv_usec;
  diff += (curr.tv_sec - tp->tv_sec)*1000000L;

  return diff < 800000L;
}


/* this is used by the qsort(3) routine to sort the file list */
static int
cmp_fileent(a, b)
     Xfwffileent	*a, *b;
{
  return strcmp(a->filename, b->filename);
}


/* this finds the last slash in a filename.
   It should be the same as strrchr(s,'/'),
   don't ask me why I don't use it instead. */
static char *
last_slash(s)
     char	*s;
{
  char	*p;
  for (p=s+strlen(s);
       *p!='/' && p>= s;
       p--)
    ;
  return (p>=s) ? p : NULL;
}


static void
set_filename(w, newspec)
     XfwfFileCompleteWidget	w;
     char	*newspec;
     /* this procedure sets the selected filename to newspec or, if
	newspec==NULL, it merely updates the text widget to reflect
	a new value. */
{
  if (newspec)
    strcpy(w->xfwf_filecomp._filename, newspec);
  else
    newspec = w->xfwf_filecomp._filename;

  XtVaSetValues(w->xfwf_filecomp.text_input, XtNstring, newspec, (String)0);
  XtVaSetValues(w->xfwf_filecomp.text_input,
		XtNinsertPosition, strlen(newspec),
		(String)0);
}

static void set_status(
#if NeedFunctionPrototypes
     XfwfFileCompleteWidget	w,
     char	*str,
     int	timeout
#endif
);

/* this is a timeout to change the status label of the widget */
static void reset_status(client_data, timer)
     XtPointer client_data;
     XtIntervalId	*timer;
{
  XfwfFileCompleteWidget	fcw = (XfwfFileCompleteWidget)client_data;
  fcw->xfwf_filecomp.msg_timeout = 0;
  set_status(fcw, L_READY, 0);
}

/* this procedure changes the status label of the widget and
   optionally adds a timeout to make the message disappear after a while. */
static void set_status(w, str, timeout)
     XfwfFileCompleteWidget	w;
     char	*str;
     int	timeout;
{
  XtVaSetValues(w->xfwf_filecomp.status_label, XtNlabel, str, (String)0);
  if (w->xfwf_filecomp.msg_timeout) {
    XtRemoveTimeOut(w->xfwf_filecomp.msg_timeout);
  }
  if (timeout) {
    w->xfwf_filecomp.msg_timeout =
      XtAppAddTimeOut (XtWidgetToApplicationContext((Widget) w),
		       5000, reset_status, (Widget) w);
  } else
    w->xfwf_filecomp.msg_timeout = 0;
}

/* POSIX does not require the S_ISSOCK and S_ISFIFO macros.  Most UNIX
   versions do have these, however.  If these predicates are not defined,
   we will try to build them from other macros that may exist.  If we
   can't build them, then we assume the OS doesn't support such things.
   */

#ifndef S_ISSOCK
#ifdef S_IFMT
#ifdef S_IFSOCK
#define	S_ISSOCK(mode)	( ((mode) & S_IFMT) == S_IFSOCK )
#endif
#endif
#endif

#ifndef S_ISLNK
#ifdef S_IFMT
#ifdef S_IFLNK
#define	S_ISLNK(mode)	( ((mode) & S_IFMT) == S_IFLNK )
#endif
#endif
#endif

/* this procedure figures out what the type suffix of a filename
   should be.  directories get /, UNIX domain sockets get =, symlinks get
   @, busted links get a !.  entries that fail the predicate get a -1.
   */
static int special_of(filename, stat_link, pred, pred_data, spooge)
     char	*filename;
     Boolean	stat_link;
     XfwfFCAcceptableStatsProc	pred;
     XtPointer	pred_data;
     XtAppContext	spooge;
{
  extern int lstat(), stat();
  struct stat buf;

  if ((stat_link?lstat:stat)(filename, &buf)<0)
    return '!';
  if (pred && !pred(filename, &buf, pred_data, spooge))
    return -1;
  else if (S_ISDIR(buf.st_mode))
    return '/';
#ifdef S_ISSOCK
  else if (S_ISSOCK(buf.st_mode))
    return '=';
#endif
#ifdef S_ISLNK
  else if (S_ISLNK(buf.st_mode))
    return '@';
#endif
  else
    return 0;
}

/* figures out whether a filespec points to a directory or not.
   if the filespec does not name a real file, the results are undefined */
static int
isdirectory(filespec)
     char	*filespec;
{
  struct stat	check;
  stat(filespec, &check);
  return S_ISDIR(check.st_mode);
}


/* this workproc will stat one more file in the widget's list.  When it
   runs out of files to stat, it reloads the ScrolledList with the new
   representations */
static Boolean
stat_files(client_data)
     XtPointer client_data;
{
  XfwfFileCompleteWidget	fcw = (XfwfFileCompleteWidget) client_data;
  pathT	temp;
  char	special;
  Xfwffileent	*lastfile;

  lastfile = & fcw->xfwf_filecomp.files[fcw->xfwf_filecomp.last_statted_file];

  strcpy(temp, fcw->xfwf_filecomp.currdir);
  strcat(temp, lastfile->filename);
  special = special_of(temp,fcw->xfwf_filecomp.statLinks, NULL, NULL,
		       XtWidgetToApplicationContext((Widget) fcw));
  if (!special) {
    lastfile->repr = lastfile->filename;
  } else {
    int len = strlen(lastfile->filename);
    lastfile->repr = (char*)malloc(len + 2);
    strcpy(lastfile->repr, lastfile->filename);
    lastfile->repr[len] = special;
    lastfile->repr[len+1] = 0;
  }
/*  printf("."); fflush(stdout);*/
  fcw->xfwf_filecomp.last_statted_file++;

  if (fcw->xfwf_filecomp.last_statted_file<fcw->xfwf_filecomp.fileslen)
    return 0;
  else {
    strcpy(temp, fcw->xfwf_filecomp.currfn);
    strcpy(fcw->xfwf_filecomp.currfn, "/");
    ChoosePrefix(fcw);
    set_status(fcw, L_READY, 0);
    fcw->xfwf_filecomp.bgstat = 0;
    return 1;
  }
}


/* this internal procedure will update the ScrolledList to contain
   only those filenames whose prefix matches the trailing component 
   of the current filespec.  It short-circuits out if the prefix is
   identical to the previous one. */
static void ChoosePrefix(w)
     XtPointer w;
{
  XfwfFileCompleteWidget	fcw = (XfwfFileCompleteWidget)w;
  String	*newlist;
  int	*newidx;
  int	llen, lsize;
  int	i, preflen;
  char	*pref;

  {
    char	*s;
    s = last_slash(fcw->xfwf_filecomp._filename);
    pref = s ? s+1 : fcw->xfwf_filecomp._filename;
  }

  if (0==strcmp(pref, fcw->xfwf_filecomp.currfn))
    return;
  strcpy(fcw->xfwf_filecomp.currfn, pref);

  newlist = (String*)malloc(sizeof(*newlist) * (lsize=8));
  newidx = (int*)malloc(sizeof(*newidx) * lsize);
  llen=0;

  preflen = strlen(pref);
  for (i=0; i<fcw->xfwf_filecomp.fileslen; i++) {
    if (0!=strncmp(pref, fcw->xfwf_filecomp.files[i].filename, preflen))
      continue;
    newidx[llen] = i;
    newlist[llen] = fcw->xfwf_filecomp.files[i].repr;
    llen++;
    if (llen>=lsize) {
      newlist = (String*)realloc(newlist, sizeof(*newlist) * (lsize*=2));
      newidx = (int*)realloc(newidx, sizeof(*newidx) * lsize);
    }
  }
  newlist[llen]=NULL;
  XfwfScrolledListSetList(fcw->xfwf_filecomp.scrolled_list, newlist, llen,
			  1, NULL);
  XfwfScrolledListUnhighlightAll(fcw->xfwf_filecomp.scrolled_list);

  if (fcw->xfwf_filecomp.candidates) {
    free(fcw->xfwf_filecomp.candidates);
    free(fcw->xfwf_filecomp.candidx);
  }
  fcw->xfwf_filecomp.candidates = newlist;
  fcw->xfwf_filecomp.candidx = newidx;
}


/* this procedure unconditionally rescans the directory
   to load a new list of files */
static void _UpdateFilesList(w)
     XfwfFileCompleteWidget	w;
{
  XfwfFileCompletePart	*fcwp = &w->xfwf_filecomp;
  DIR	*dp;
  struct dirent	*dep;
  pathT	temp;
  int	offset, idx;

  set_status(w, L_SCANNING, 0);

  strcpy(temp, fcwp->currdir);
  offset = strlen(temp);

  if (fcwp->last_statted_file<fcwp->fileslen) {
    XtRemoveWorkProc(fcwp->bgstat);
    fcwp->bgstat = 0;
  }

  if (fcwp->files==NULL) {
    fcwp->files = (Xfwffileent*)malloc(sizeof(*fcwp->files)
				       * (fcwp->filessize=8));
  } else {
    int	i;
    for (i=0; i<fcwp->fileslen; i++) {
      if (fcwp->files[i].filename != fcwp->files[i].repr)
	free(fcwp->files[i].repr);
      free(fcwp->files[i].filename);
    }
  }
  fcwp->fileslen=0;

  strcpy(fcwp->currfn, "/");

  dp = opendir(fcwp->currdir);
  if (dp==NULL) {
    set_status(w, L_ERRORDIR, 0);
    /* give the user a way out of this dir */
    fcwp->files[0].filename = XtNewString(".");
    fcwp->files[0].repr = XtNewString("./");
    fcwp->files[1].filename = XtNewString("..");
    fcwp->files[1].repr = XtNewString("../");
    fcwp->fileslen = 2;
    ChoosePrefix(w);
    fcwp->last_statted_file = 2;	/* to avoid segfaults! */
    return;
  }

  /* this is done here since the glob() functions work on whole
     directories, not on individual files like the rest of the 
     'AcceptableFilenameProc's do it */
  if (fcwp->acceptFilenameProc == XfwfFCGlobFiles) {
#if defined(USE_GLOB)
    glob_t	glob_buffer;
    char	glob_path_save[MAXPATHLEN+1];
    int 	i, glob_ret;

    closedir(dp);	/* glob does its own dir access */
      
    /* glob doesn't return "." and ".." */    
    fcwp->files[0].filename = XtNewString(".");
    fcwp->files[0].repr = XtNewString("./");
    fcwp->files[1].filename = XtNewString("..");
    fcwp->files[1].repr = XtNewString("../");
    fcwp->fileslen = 2;
    fcwp->last_statted_file = 2;
      
    getcwd(glob_path_save, MAXPATHLEN);
    chdir(fcwp->currdir);
      
    glob_ret = glob(fcwp->filenameProcData, 0, NULL, &glob_buffer);
    chdir(glob_path_save);
      
    /* if nothing matched, we're done! */
    if(glob_ret == 3 || (glob_buffer.gl_flags & GLOB_NOMATCH)) {
      globfree(&glob_buffer);
      return;
    }

    /* copy glob's return list to ours */
    for (i = 0; i < glob_buffer.gl_pathc; i++) {
      if (fcwp->fileslen>=fcwp->filessize) {
        fcwp->files = (Xfwffileent*)realloc
          (fcwp->files, sizeof(*fcwp->files) * (fcwp->filessize*=2));
      }

      fcwp->files[fcwp->fileslen].filename = XtNewString(glob_buffer.gl_pathv[i]);
      fcwp->fileslen++;
    }

    globfree(&glob_buffer);
  } else {
#else
    Cardinal	num_subs = 0;
    String	subs[1];
    /* urgh, globbing was not available at the time :( */
    XtAppWarningMsg(XtWidgetToApplicationContext((Widget)w),
		    "packageUnavailable", "xfwfFileCompGlob",
		    "XfwfLibraryError",
		    "attempt to use globbing in a FileComplete widget.  The FWF library was not compiled with that option.",
		    subs, &num_subs);
    fcwp->acceptFilenameProc = 0;
  }
  {
#endif
   
      while (NULL != (dep = readdir(dp))) {
	if (fcwp->fileslen>=fcwp->filessize) {
	  fcwp->files = (Xfwffileent*)realloc
	    (fcwp->files, sizeof(*fcwp->files) * (fcwp->filessize*=2));
	}

	if (fcwp->fileslen>=2 && /* . and .. MUST be included, sorry */
	    fcwp->acceptFilenameProc &&
	    !(fcwp->acceptFilenameProc)(dep->d_name, fcwp->filenameProcData))
	  continue;

	fcwp->files[fcwp->fileslen].filename = XtNewString(dep->d_name);
	fcwp->fileslen++;
      }
      closedir(dp);
      fcwp->last_statted_file=0;

      if (fcwp->sortProc) {
	fcwp->sortProc(fcwp->files, fcwp->fileslen);
      } else {
	/* we don't need to sort . and .. */
	qsort(fcwp->files+2, fcwp->fileslen-2,
	      sizeof(*fcwp->files), cmp_fileent);
      }
    }


  set_status(w, L_STATTING, 0);

  if (fcwp->fileslen <= fcwp->statThreshold ||
      fcwp->statThreshold==-1 || fcwp->acceptStatsProc) {
    for (idx=0; idx<fcwp->fileslen; idx++) {
      char	special;
      Xfwffileent	*currfile;

      currfile = fcwp->files+idx;
      strcpy(temp+offset, currfile->filename);
      special = special_of
	(temp, fcwp->statLinks, fcwp->acceptStatsProc, fcwp->statsProcData,
	 XtWidgetToApplicationContext((Widget) w));
      if (special==-1) {
	free(currfile->filename);
	bcopy(currfile+1, currfile,
	      sizeof(*currfile) * (--fcwp->fileslen - idx));
	idx--;
	continue;
      }
      if (!special) {
	currfile->repr = currfile->filename;
      } else {
	int len = strlen(currfile->filename);
	currfile->repr = (char*)malloc(len + 2);
	strcpy(currfile->repr, currfile->filename);
	currfile->repr[len] = special;
	currfile->repr[len+1] = 0;
      }
    }
    fcwp->last_statted_file = fcwp->fileslen;
    ChoosePrefix(w);
    set_status(w, L_READY, 0);
  } else {
    for (idx=0; idx<fcwp->fileslen; idx++)
      fcwp->files[idx].repr = fcwp->files[idx].filename;
    fcwp->bgstat = XtAppAddWorkProc(XtWidgetToApplicationContext((Widget) w),
stat_files, (Widget) w);
  }
}

/* this procedure checks to see if the filespec in the widget
   represents a new directory (or represents the old directory
   modified).  if so, it loads up the new list of files */
static void UpdateFilesList(w)
     XfwfFileCompleteWidget	w;
{
  char	*filename;
  pathT	newdir;
  struct stat	newstats;
  
  filename = last_slash(w->xfwf_filecomp._filename);
  
  if (filename==NULL)
    strcpy(newdir, "./");
  else {
    char	*s, *p;
    for (s=w->xfwf_filecomp._filename, p=newdir; s<=filename; s++,p++)
      *p = *s;
    *p = 0;
  }
  
  strcpy(w->xfwf_filecomp.currdir, newdir);
  bzero(&newstats, sizeof(newstats));
  stat(w->xfwf_filecomp.currdir, &newstats);
  newstats.st_atime = w->xfwf_filecomp.cdstats.st_atime;
  if (newstats.st_dev != w->xfwf_filecomp.cdstats.st_dev ||
      newstats.st_ino != w->xfwf_filecomp.cdstats.st_ino ||
      newstats.st_mode != w->xfwf_filecomp.cdstats.st_mode ||
      newstats.st_nlink != w->xfwf_filecomp.cdstats.st_nlink ||
      newstats.st_mtime != w->xfwf_filecomp.cdstats.st_mtime ||
      newstats.st_ctime != w->xfwf_filecomp.cdstats.st_ctime ) {
    bcopy(&newstats, &w->xfwf_filecomp.cdstats, sizeof(newstats));
    _UpdateFilesList(w);
  }
}

/* this is the callback for when a user clicks on the
   ScrolledList.  It either replaces the trailing filename
   component with the selected one, or causes the item to
   be selected by the FileComplete widget. */
static void ItemSelected(w, client_data, call_data)
     Widget	w;
     XtPointer	call_data, client_data;
{
  XfwfFileCompleteWidget	fcw = (XfwfFileCompleteWidget)client_data;
  XfwfFileCompletePart *fcwp =
    &(fcw)->xfwf_filecomp;
  XfwfScrolledListReturnStruct	*lrs =
    (XfwfScrolledListReturnStruct*)call_data;

  if (lrs->string==NULL || lrs->index<0 )
    return;

  /*printf("%s(%d)\n", lrs->string, lrs->index);*/
  if (fcwp->last_selected_filename && lrs->string &&
      0==strcmp(lrs->string, fcwp->last_selected_filename) &&
      lrs->index == fcwp->last_selected_index &&
      within_time(&fcwp->time_last_selected)) {
    pathT	temp;
    Xfwffileent	*selfile;

    fcwp->time_last_selected.tv_sec = 0;
    fcwp->time_last_selected.tv_usec = 0;

    strcpy(temp, fcwp->currdir);
    selfile = fcwp->files + fcwp->candidx[lrs->index];
    strcat(temp, selfile->filename);
    if ((selfile->repr[strlen(selfile->repr)-1]=='/' || isdirectory(temp)) &&
	!fcwp->dirnamesOk) {
      strcat(temp, "/");
      squish_filespec(temp, fcwp->followLinks);
      set_filename(fcw, temp);
      UpdateFilesList(fcw);
    } else {
      set_filename(fcw, temp);
      Notify(fcw, NULL, NULL, NULL);
    }
  } else {
    pathT	temp;
    Xfwffileent	*selfile;

    gettimeofday(&fcwp->time_last_selected, NULL);
    if (fcwp->last_selected_filename)
      free (fcwp->last_selected_filename);
    fcwp->last_selected_filename =
      (char*) (lrs->string ? XtNewString(lrs->string) : NULL);
    fcwp->last_selected_index = lrs->index;
    strcpy(temp, fcwp->currdir);
    selfile = fcwp->files + fcwp->candidx[lrs->index];
    strcat(temp, selfile->filename);
    set_filename(fcw, temp);
  }
}

/* when the user selects a file, this action procedure is called.
   It can be directly invoked by pressing <Return> or it can be
   indirectly invoked through the Ok button Callback.  It calls
   all the user's callbacks after normalizing the filespec. */
static void Notify(w, evt, argv, argc)
     Widget	w;
     XEvent	*evt;
     String	*argv;
     Cardinal	*argc;
{
  XfwfFileCompleteWidget	fcw = (XfwfFileCompleteWidget)w;
  String	filespec;
  pathT		temp;

  XtVaGetValues(fcw->xfwf_filecomp.text_input, XtNstring, &filespec,
		(String)0);
  strcpy(temp, filespec);
  squish_filespec(temp, fcw->xfwf_filecomp.followLinks);
  if (0!=strcmp(temp,filespec))
    set_filename(fcw, temp);
  else 
    strcpy(fcw->xfwf_filecomp._filename, temp);

  XtCallCallbackList((Widget) fcw, fcw->xfwf_filecomp.callback, temp);
}

/* this is the OK button callback, it calls the standard Notify procedure */
static void AuxNotify(w, client_data, call_data)
     Widget	w;
     XtPointer  client_data, call_data;
{
  Notify((XfwfFileCompleteWidget)client_data, NULL, NULL, NULL);
}

/* this is the Cancel button callback.  It calls all the user's
   cancelCallbacks */
static void Cancel(w, client_data, call_data)
     Widget	w;
     XtPointer  client_data, call_data;
{
  XfwfFileCompleteWidget	fcw = (XfwfFileCompleteWidget)client_data;
  XtCallCallbackList((Widget) fcw, fcw->xfwf_filecomp.cancelCallback, NULL);
}


/* this procedure takes the filespec from the widget and attempts
   to complete it as far as it unambiguously can.  It places the
   extended filename in the buffer pointed to by new. */
static void complete(fcw, new)
     XfwfFileCompleteWidget	fcw;
     char	*new;
{
  char	*s;
  int	idx, origlen, firstlen;
  int	othermatch;

  strcpy(new, fcw->xfwf_filecomp._filename);
  s = last_slash(new);
  if (s==NULL)
    s = new;
  else 
    s++;
  /* s now points to the beginning of the filename part */

  origlen = strlen(s);
  for (idx=0; idx<fcw->xfwf_filecomp.fileslen; idx++)
    if (0==strncmp(s, fcw->xfwf_filecomp.files[idx].filename, origlen))
      break;

  if (idx>=fcw->xfwf_filecomp.fileslen) {
    set_status(fcw, L_NOCOMPLETE, 1);
    return;
  }

  /* we have a completion */
  strcpy(s, fcw->xfwf_filecomp.files[idx].filename);
  firstlen = strlen(fcw->xfwf_filecomp.files[idx].filename);
  /* let's see how much of it matches */

  othermatch=0;
  for (idx++; idx<fcw->xfwf_filecomp.fileslen; idx++) {
    char	*p, *r;
    if (0!=strncmp(s, fcw->xfwf_filecomp.files[idx].filename, origlen)) {
      /* since the files array is sorted, if the first
	 origlen characters don't match, no following items
	 in the array will match */
      break;
    }
    othermatch=1;
    for (p=s, r=fcw->xfwf_filecomp.files[idx].filename;
	 (*p) && (*r) && (*p==*r);
	 p++, r++)
      ;
    *p=0;
    if (p == s+origlen) {
      if (firstlen == p-s)
	set_status(fcw, L_NONUNIQUE, 1);
      /* short circuit */
      XBell(XtDisplay((Widget)fcw), 100);
      return;
    }
  }

  if (!othermatch)
    set_status(fcw, L_UNIQUE, 1);

  if (!othermatch && isdirectory(new))
    strcat(new, "/");
}

/* this is an action proc that attempts to perform filename completion
   on the current filespec of the widget.  its exact behavior is
   documented in the manual page */
static void AttemptFileCompletion(w, evt, argv, argc)
     Widget	w;
     XEvent	*evt;
     String	*argv;
     Cardinal	*argc;
{
  XfwfFileCompleteWidget fcw = (XfwfFileCompleteWidget)w;
  pathT		newspec;
  int	optlist=0, newlen;

  switch (*argc) {
  case 1:
    if (0==strcmp(*argv, "optlist"))
      optlist=1;
  }

  set_status(fcw, L_READY, 0);

  {
    String	filespec;
    XtVaGetValues(fcw->xfwf_filecomp.text_input, XtNstring, &filespec,
		  (String)0);
    strcpy(fcw->xfwf_filecomp._filename, filespec);
    squish_filespec(fcw->xfwf_filecomp._filename,
		    fcw->xfwf_filecomp.followLinks);
  }

  UpdateFilesList(fcw);
  complete(fcw, newspec);

  newlen = strlen(newspec);

  if (newlen==strlen(fcw->xfwf_filecomp._filename) && optlist) {
    set_filename(fcw, newspec);
    UpdateFilesList(fcw);
    ChoosePrefix(fcw);
  } else if (newspec[newlen-1]=='/') {
    squish_filespec(newspec, fcw->xfwf_filecomp.followLinks);
    set_filename(fcw, newspec);
    UpdateFilesList(fcw);
    ChoosePrefix(fcw);
  } else
    set_filename(fcw, newspec);
}


/* this procedure checks to see if the user has adjusted any widget
   values that may cause recomputation of various parts of the widget. */
static Boolean 
SetValues(current, request, new, args, num_args)
     Widget current, request, new;
     ArgList args;
     Cardinal *num_args;
#define DIFFER(field) (newfcw->xfwf_filecomp.field != \
			oldfcw->xfwf_filecomp.field)
{
  Arg	targs[20];
  int	nargs;
  XfwfFileCompleteWidget	oldfcw = (XfwfFileCompleteWidget) current;
  XfwfFileCompleteWidget	newfcw = (XfwfFileCompleteWidget) new;
  if (!newfcw->xfwf_filecomp.filename_) {
    newfcw->xfwf_filecomp.filename_ = "./";
  }
  if (!oldfcw->xfwf_filecomp.filename_ ||
      0!= strcmp(newfcw->xfwf_filecomp._filename,
		 newfcw->xfwf_filecomp.filename_)) {
    set_filename(newfcw, newfcw->xfwf_filecomp.filename_);
  }
  newfcw->xfwf_filecomp.filename_ = newfcw->xfwf_filecomp._filename;

  if (DIFFER(sortProc) ||
      DIFFER(acceptFilenameProc) || DIFFER(filenameProcData) ||
      DIFFER(acceptStatsProc) || DIFFER(statsProcData) ||
      DIFFER(statLinks) ) {
    _UpdateFilesList(newfcw);
    strcpy(newfcw->xfwf_filecomp.currfn, "/");
    ChoosePrefix(newfcw);
  }

  if (DIFFER(showOkButton)) {
    if (newfcw->xfwf_filecomp.showOkButton) {
      newfcw->xfwf_filecomp.ok_button = XtVaCreateManagedWidget
	("ok", commandWidgetClass, newfcw->xfwf_filecomp.button_box,
	 XtNlabel, newfcw->xfwf_filecomp.okLabel,
	 XtNcallback, newfcw->xfwf_filecomp.callback,
	 (String)0);
    } else {
      XtDestroyWidget(newfcw->xfwf_filecomp.ok_button);
      newfcw->xfwf_filecomp.ok_button = NULL;
    }
  } else {
    nargs=0;
    if (DIFFER(okLabel)) {
      XtSetArg(targs[nargs], XtNlabel, newfcw->xfwf_filecomp.okLabel);
      nargs++;
    }
    if (DIFFER(callback)) {
      XtSetArg(targs[nargs], XtNcallback, newfcw->xfwf_filecomp.callback);
      nargs++;
    }
    if (nargs>0 && newfcw->xfwf_filecomp.ok_button)
      XtSetValues(newfcw->xfwf_filecomp.ok_button, targs, nargs);
  }

  if (DIFFER(showCancelButton)) {
    if (newfcw->xfwf_filecomp.showCancelButton) {
      newfcw->xfwf_filecomp.cancel_button = XtVaCreateManagedWidget
	("cancel", commandWidgetClass, newfcw->xfwf_filecomp.button_box,
	 XtNlabel, newfcw->xfwf_filecomp.cancelLabel,
	 XtNcallback, newfcw->xfwf_filecomp.cancelCallback,
	 (String)0);
    } else {
      XtDestroyWidget(newfcw->xfwf_filecomp.cancel_button);
      newfcw->xfwf_filecomp.cancel_button = NULL;
    }
  } else {
    nargs=0;
    if (DIFFER(cancelLabel)) {
      XtSetArg(targs[nargs], XtNlabel, newfcw->xfwf_filecomp.cancelLabel);
      nargs++;
    }
    if (DIFFER(cancelCallback)) {
      XtSetArg(targs[nargs], XtNcallback,newfcw->xfwf_filecomp.cancelCallback);
      nargs++;
    }
    if (nargs>0 && newfcw->xfwf_filecomp.cancel_button)
      XtSetValues(newfcw->xfwf_filecomp.cancel_button, targs, nargs);
  }
  /* I don't think we need to redisplay.. (?) */
  return 0;
}

#undef DIFFER

/* this procedure intercepts the getting of the filename and makes
   sure that the value returned reflects the normalized contents of
   the text buffer */
static void
GetValuesHook(w, args, num_args)
Widget w;
ArgList args;
Cardinal * num_args;
{
  XfwfFileCompletePart	*fcwp = &((XfwfFileCompleteWidget)w)->xfwf_filecomp;
  int	i;

  for (i=0; i<*num_args; i++) {
    if (0==strcmp(args[i].name, XtNfilename)) {
      char	*filespec;
      static pathT	temp;

      XtVaGetValues(fcwp->text_input, XtNstring, &filespec, (String)0);
      strcpy(temp, filespec);
      squish_filespec(temp, fcwp->followLinks);
      if (0!=strcmp(temp,filespec))
	set_filename(w, temp);
      else 
	strcpy(fcwp->_filename, temp);
      *(String*)args[i].value = temp;
    }
  }
}


/* really twisted form layout goop */
static void DoWierdStuff(fw, width, height)
     FormWidget	fw;
     int	width, height;
{
    XtGeometryResult	rval;
    XtWidgetGeometry	xtwg;
    WidgetList	children = fw->composite.children;
    FormConstraints	fc0, fc1, fc2, fc3;

    fc0 = ( (FormConstraints)children[0]->core.constraints);
    xtwg.request_mode = 0;
    rval = XtQueryGeometry(children[0], &xtwg, &xtwg);
    fc0->form.virtual_height = xtwg.height;

    fc1 = ( (FormConstraints)children[1]->core.constraints);
    xtwg.request_mode = 0;
    rval = XtQueryGeometry(children[1], &xtwg, &xtwg);
    fc1->form.virtual_height = xtwg.height;

    fc2 = ( (FormConstraints)children[2]->core.constraints);
    xtwg.request_mode = 0;
    rval = XtQueryGeometry(children[2], &xtwg, &xtwg);
    fc2->form.virtual_width = xtwg.width;
    fc2->form.virtual_height = xtwg.height;

    fc3 = ( (FormConstraints)children[3]->core.constraints);
    xtwg.request_mode = 0;
    rval = XtQueryGeometry(children[3], &xtwg, &xtwg);
    fc3->form.virtual_width = xtwg.width;
    fc3->form.virtual_height = xtwg.height;

    if (fw->core.width==0 && fw->core.height==0) {
      int	h2, h3;
      fc0->form.virtual_width = fc1->form.virtual_width =
	fc2->form.virtual_width + fc2->form.dx
	  + fc3->form.virtual_width + 2*fc3->form.dx +
	  2*(children[2]->core.border_width + children[3]->core.border_width);
      /* adjust below */

      h2 = fc2->form.virtual_height
	+ 2*(fc2->form.dy + children[2]->core.border_width);
      h3 = fc3->form.virtual_height
	+ 2*(fc3->form.dy + children[3]->core.border_width);
      if (h3 > h2)
	fc2->form.virtual_height =
	  h3 - 2*(fc2->form.dy + children[2]->core.border_width);
      else
	fc3->form.virtual_height =
	  h2 - 2*(fc3->form.dy + children[3]->core.border_width);;
    } else {
      /* all mentioned artifacts are because of strangeness in the
	 Form widget.  Nothing we can easily do about it */
      fc0->form.virtual_width = fc1->form.virtual_width =
	fw->core.width /* this is an artifact -> - 1 */;
      /* adjust below */

      fc2->form.virtual_width = fw->core.width -
	(fc3->form.virtual_width /* this is an artifact -> + 1 */
	 + 2 * (children[3]->core.border_width + fc3->form.dx) )
	  - fc2->form.dx - 2*children[2]->core.border_width;

      fc2->form.virtual_height = fc3->form.virtual_height =
	fw->core.height /* this is an artifact -> */ - 1
	  - ( fc0->form.virtual_height + fc0->form.dy
	     + fc1->form.virtual_height + fc1->form.dy
	     + 2*(children[0]->core.border_width
		  + children[1]->core.border_width)
	     );
      fc2->form.virtual_height -=
	2 * fc2->form.dy + 2 * children[2]->core.border_width;
      fc3->form.virtual_height -=
	2 * fc3->form.dy + 2 * children[3]->core.border_width;
    }

    fc0->form.virtual_width -=
      2* fc0->form.dx + 2 * children[0]->core.border_width;
    fc1->form.virtual_width -=
      2* fc1->form.dx + 2 * children[1]->core.border_width;


    XtResizeWidget(children[0], fc0->form.virtual_width,
		   fc0->form.virtual_height, children[0]->core.border_width);
    XtResizeWidget(children[1], fc1->form.virtual_width,
		   fc1->form.virtual_height, children[1]->core.border_width);
    XtResizeWidget(children[2], fc2->form.virtual_width,
		   fc2->form.virtual_height, children[2]->core.border_width);
    XtResizeWidget(children[3], fc3->form.virtual_width,
		   fc3->form.virtual_height, children[3]->core.border_width);
}


static void LayoutChild(
#if NeedFunctionPrototypes
    Widget w,
    FormWidget parent
#endif
);

/* this is almost a direct copy of the Form Widget's layout procedure */
/* ARGSUSED */
static Boolean Layout(fw, width, height)
    FormWidget fw;
    Dimension width, height;
{
    int num_children = fw->composite.num_children;
    WidgetList children = fw->composite.children;
    Widget *childP;
    Position maxx, maxy;
    Boolean ret_val;

    for (childP = children; childP - children < num_children; childP++) {
	FormConstraints form = (FormConstraints)(*childP)->core.constraints;
	form->form.layout_state = LayoutPending;
    }

    DoWierdStuff(fw, width, height);

    maxx = maxy = 1;
    for (childP = children; childP - children < num_children; childP++) {
	if (XtIsManaged(*childP)) {
	    Position x, y;
	    LayoutChild(*childP, fw);
	    x = (*childP)->core.x + (*childP)->core.width
		+ ((*childP)->core.border_width << 1);
	    y = (*childP)->core.y + (*childP)->core.height
		+ ((*childP)->core.border_width << 1);
	    if (maxx < x) maxx = x;
	    if (maxy < y) maxy = y;
	}
    }

    fw->form.preferred_width = (maxx += fw->form.default_spacing);
    fw->form.preferred_height = (maxy += fw->form.default_spacing);

    if (fw->form.resize_in_layout
	&& (maxx != fw->core.width || maxy != fw->core.height)) {
	XtGeometryResult result;
	result = XtMakeResizeRequest
	  ((Widget) fw, (Dimension)maxx, (Dimension)maxy,
	   (Dimension*)&maxx, (Dimension*)&maxy );
	if (result == XtGeometryAlmost)
	    result = XtMakeResizeRequest
	      ((Widget) fw, (Dimension)maxx, (Dimension)maxy, NULL, NULL );
	fw->form.old_width  = fw->core.width;
	fw->form.old_height = fw->core.height;
	ret_val = (result == XtGeometryYes);
    } else ret_val = False;

    return ret_val;
}

/* this is copied from the Form Widget too */
static void LayoutChild(w, parent)
    Widget w;
    FormWidget parent;
{
    FormConstraints form = (FormConstraints)w->core.constraints;
    Position x, y;
    Widget ref;

    switch (form->form.layout_state) {

      case LayoutPending:
	form->form.layout_state = LayoutInProgress;
	break;

      case LayoutDone:
	return;

      case LayoutInProgress:
	{
	String subs[2];
	Cardinal num_subs = 2;
	subs[0] = w->core.name;
	subs[1] = w->core.parent->core.name;
	XtAppWarningMsg(XtWidgetToApplicationContext(w),
			"constraintLoop","xawFormLayout","XawToolkitError",
   "constraint loop detected while laying out child '%s' in FormWidget '%s'",
			subs, &num_subs);
	return;
	}
    }


    x = form->form.dx;
    y = form->form.dy;
    if ((ref = form->form.horiz_base) != (Widget)NULL) {
	LayoutChild(ref, NULL);
	x += ref->core.x + ref->core.width + (ref->core.border_width << 1);
    }
    if ((ref = form->form.vert_base) != (Widget)NULL) {
	LayoutChild(ref, NULL);
	y += ref->core.y + ref->core.height + (ref->core.border_width << 1);
    }
    XtMoveWidget( w, x, y );
    form->form.layout_state = LayoutDone;
}

/***********************************/
/*                                 */
/* exported type converters        */
/*                                 */
/***********************************/

#if 1
/* this isn't available under R5 */
static void LowerCase(src, dest)
     char	*src;
     char	*dest;
{
  while (*src) {
    *dest = islower(*src) ? *src : tolower(*src);
    src++;
    dest++;
  }
}
#endif

#define   done(type, value) \
   do {                                    \
       if (toVal->addr != NULL) {       \
          if (toVal->size < sizeof(type)) {\
             toVal->size = sizeof(type);\
             return False;              \
          }                             \
          *(type*)(toVal->addr) = (value);\
       }                                \
       else {                           \
          static type static_val;       \
          static_val = (value);         \
          toVal->addr = (XtPointer)&static_val;\
       }                                \
       toVal->size = sizeof(type);      \
       return True;                     \
       /* NOTREACHED */			\
   } while (0) /* NOTREACHED */

Boolean CvtStringToAcceptableStatsProc(dpy, args, num_args, fromVal,
toVal, converter_data)
   Display   *dpy;
   XrmValue  *args;
   Cardinal  *num_args;
   XrmValue  *fromVal;
   XrmValue  *toVal;
   XtPointer *converter_data;
{
   char          message[1000];
   XfwfFCAcceptableStatsProc	proc = (XfwfFCAcceptableStatsProc)-1;

   if (*num_args != 0)
       XtAppWarningMsg
	 (XtDisplayToApplicationContext(dpy),
          "cvtStringToAcceptableFileProc", "wrongParameters",
	  "FileCompletion-util",
	  "String to AcceptableStatsProc takes no parameters",
          (String *)NULL, (Cardinal *)NULL);

   LowerCase((char *) fromVal->addr, message);

   if ( strcmp(message, "any") == 0)
     proc = NULL;
   else if ( strcmp(message, "dirorregexfiles") == 0)
     proc = XfwfFCDirsOrRegexFiles;
   else if ( strcmp(message, "dirorglobfiles") == 0)
     proc = XfwfFCDirsOrGlobFiles;
   else {
     XtDisplayStringConversionWarning
       (dpy,
	fromVal->addr, XtRAcceptableStatsProc);
     return False;
   }

   done(XfwfFCAcceptableStatsProc, proc);

   /* converter_data not used here */
}

Boolean CvtStringToAcceptableFileProc(dpy, args, num_args, fromVal,
toVal, converter_data)
   Display   *dpy;
   XrmValue  *args;
   Cardinal  *num_args;
   XrmValue  *fromVal;
   XrmValue  *toVal;
   XtPointer *converter_data;
{
   char          message[1000];
   XfwfFCAcceptableFilenameProc	proc = (XfwfFCAcceptableFilenameProc)-1;

   if (*num_args != 0)
       XtAppWarningMsg
	 (XtDisplayToApplicationContext(dpy),
          "cvtStringToAcceptableFileProc", "wrongParameters",
	  "FileCompletion-util",
	  "String to AcceptableFilenameProc takes no parameters",
          (String *)NULL, (Cardinal *)NULL);

   LowerCase((char *) fromVal->addr, message);

   if ( strcmp(message, "any") == 0)
     proc = NULL;
   else if ( strcmp(message, "nodotfiles") == 0)
     proc = XfwfFCNoDotFiles;
   else if ( strcmp(message, "notildebackups") == 0)
     proc = XfwfFCNoTildeBackups;
   else if ( strcmp(message, "notildebackupsordotfiles") == 0)
     proc = XfwfFCNoTildeBackupsOrDotFiles;
   else if ( strcmp(message, "regexfiles") == 0)
     proc = (XfwfFCAcceptableFilenameProc)XfwfFCRegexFiles;
   else if ( strcmp(message, "globfiles") == 0)
     proc = XfwfFCGlobFiles;
   else {
     XtDisplayStringConversionWarning
       (dpy,
	fromVal->addr, XtRAcceptableFileProc);
     return False;
   }

   done(XfwfFCAcceptableFilenameProc, proc);

   /* converter_data not used here */
}

Boolean CvtStringToFollowLinks(dpy, args, num_args, fromVal, toVal,
			       converter_data)
   Display   *dpy;
   XrmValue  *args;
   Cardinal  *num_args;
   XrmValue  *fromVal;
   XrmValue  *toVal;
   XtPointer *converter_data;
{
   char          message[1000];

   if (*num_args != 0)
       XtAppWarningMsg
	 (XtDisplayToApplicationContext(dpy),
          "cvtStringToFollowLinks", "wrongParameters",
	  "FileCompletion-util",
	  "String to AcceptableFilenameProc takes no parameters",
          (String *)NULL, (Cardinal *)NULL);

   LowerCase((char *) fromVal->addr, message);

   if ( strcmp(message, "never") == 0)
     done(enum XfwfFCFollowLinks, XfwfFCNever);
   else if ( strcmp(message, "whenshorter") == 0)
     done(enum XfwfFCFollowLinks, XfwfFCWhenShorter);
   else if ( strcmp(message, "always") == 0)
     done(enum XfwfFCFollowLinks, XfwfFCAlways);
   else {
     XtDisplayStringConversionWarning
       (dpy,
	fromVal->addr, XtRFollowLinks);
     return False;
   }

   /* converter_data not used here */
}

#undef done

/***********************************/
/*                                 */
/* exported convenience filters    */
/*                                 */
/***********************************/

Boolean XfwfFCNoDotFiles (s, ignored)
     char	*s;
     XtPointer	ignored;
{
  return (s[0]!='.' || s[1]==0 || (s[1]=='.' && s[2]==0));
}

Boolean XfwfFCNoTildeBackups (s, ignored)
     char	*s;
     XtPointer	ignored;
{
  int	len=strlen(s);
  return len<1 || s[len-1]!='~';
}

Boolean XfwfFCNoTildeBackupsOrDotFiles(s, ignored)
     char	*s;
     XtPointer	ignored;
{
  return XfwfFCNoTildeBackups(s,ignored) && XfwfFCNoDotFiles(s, ignored);
}




#if defined(USE_REGEX)

/* following regex routine was developed by reading Brian Totty's code
   and reading the man page.  Long live free source code! */

#if !defined(GNU_REGEX)

#define INIT		register char *sp = instring;
#define	GETC()		(*sp++)
#define	PEEKC()		(*sp)
#define	UNGETC(c)	-- sp
#define	RETURN(ptr)	return (ptr);
#define	ERROR(val)	{ regex_errno = (val); return NULL; }
static int	regex_errno;
static int	getrnge();
#include	<regexp.h>

#endif /* !GNU_REGEX */

/* This routine now uses either the regexp library or the GNU regex
   library based on the #define GNU_REGEX.  It can also be eliminated
   entirely by not #defining USE_REGEX 			EdW */
/* We don't want to completely eliminate it.  This would cause
   compatibility problems.  Instead we define some stub procedures
   that issue XtAppWarningMsgs.                         RF */

#if NeedFunctionPrototypes
Boolean XfwfFCRegexFiles (
     char	*s,
     _Xconst XtPointer	regex,
     XtAppContext	app_con)
#else
Boolean XfwfFCRegexFiles (s, regex, app_con)
     char	*s;
     XtPointer	regex;
     XtAppContext	app_con;
#endif
				/* app_con: super secret parameter so
				   I can issue warning messages when
				   regexps fail to compile. If I ever
				   find a better way to do this, this
				   parameter will disappear. */
{
  static char	*cache_regex = NULL,
		*busted_regex = NULL;
#if defined(GNU_REGEX)
  _Xconst char *	comp_result;
#else
  static char	compbuf[2048]; /* I should modify this to be a dynamically
				  grown array.  sigh */
#endif
  int	rval;

  if (busted_regex && 0==strcmp(regex,busted_regex))
    return TRUE;

  if (!cache_regex || 0!=strcmp(cache_regex,regex)) {
#if defined(GNU_REGEX)
    if (!(comp_result = re_compile_pattern(regex, strlen(regex), &compbuf))) {
#else
    if (NULL != compile(regex, compbuf, compbuf+sizeof(compbuf), '\0')) {
#endif
      XtFree(cache_regex);	/* safe for NULL */
      cache_regex = XtNewString(regex);
    } else {
      String	params[2];
      Cardinal	n;

#if defined(GNU_REGEX)
      params[0] = comp_result;
#else
      switch (regex_errno) {
      case 11: params[0] = "Range endpoint too large."; break;
      case 16: params[0] = "Bad number"; break;
      case 25: params[0] = "`\\digit' out of range."; break;
      case 36: params[0] = "Illegal or missing delimiter."; break;
      case 41: params[0] = "No remembered search string."; break;
      case 42: params[0] = "\\( \\) imbalance."; break;
      case 43: params[0] = "Too many \\(."; break;
      case 44: params[0] = "More than 2 numbers given in \\{ \\}."; break;
      case 45: params[0] = "} expected after \\."; break;
      case 46: params[0] = "First number exceeds second in \\{ \\}."; break;
      case 49: params[0] = "[] imbalance."; break;
      case 50: params[0] = "Regular expression too long."; break;
      default: params[0] = "unknown regex compilation error."; break;
      }
#endif

      params[1] = regex;
      n = 2;
      XtAppWarningMsg(app_con, "compileFailed", "xfwfFileCompRegex",
		      "XfwfUserError",
		      "error during regexp compilation (%s): \"%s\"",
		      params, &n);
      XtFree(busted_regex);
      busted_regex = XtNewString(regex);
      return TRUE;
    }
  }

#if defined(GNU_REGEX)
  rval = re_match(&compbuf, s, strlen(s), 0, NULL);
  return ((rval >= 0) && (rval == strlen(s)));
#else
  rval = advance(s, compbuf);
  return rval && *loc2==0;
#endif
}


Boolean XfwfFCDirsOrRegexFiles(filename, filestats, regex, app_con)
     char	*filename;
     struct stat *filestats;
     XtPointer	regex;
     XtAppContext	app_con;	/* super secret parameter so I can
					   issue warning messages when regexps
					   fail to compile. If I ever find a
					   better way to do this, this
					   parameter will disappear. */
{
  return S_ISDIR(filestats->st_mode) ||
    XfwfFCRegexFiles(filename, regex, app_con) ;
}

#else /* USE_REGEX */

#if NeedFunctionPrototypes
Boolean XfwfFCRegexFiles (
     char	*s,
     _Xconst XtPointer	regex,
     XtAppContext	app_con)
#else
Boolean XfwfFCRegexFiles (s, regex, app_con)
     char	*s;
     XtPointer	regex;
     XtAppContext	app_con;
#endif
{
  Cardinal	num_subs = 0;
  String	subs[1];
  static int	issued=0;

  if (issued)
    return;		/* they don't want to see this a million times */
  /* urgh, regex was not available at the time :( */
  XtAppWarningMsg(app_con, "packageUnavailable", "xfwfFileCompRegex",
		  "XfwfLibraryError",
		  "attempt to use regular expressions in a FileComplete widget (XfwfFCRegexFiles).  The FWF library was not compiled with that option.",
		  subs, &num_subs);
  issued = 1;
}

Boolean XfwfFCDirsOrRegexFiles(filename, filestats, regex, app_con)
     char	*filename;
     struct stat *filestats;
     XtPointer	regex;
     XtAppContext	app_con;
{
  Cardinal	num_subs = 0;
  String	subs[1];
  static int	issued=0;

  if (issued)
    return;		/* they don't want to see this a million times */

  /* urgh, regex was not available at the time :( */
  XtAppWarningMsg(app_con, "packageUnavailable", "xfwfFileCompRegex",
		  "XfwfLibraryError",
		  "attempt to use regular expressions in a FileComplete widget (XfwfFCDirsOrRegexFiles).  The FWF library was not compiled with that option.",
		  subs, &num_subs);
  issued = 1;
}

#endif /* USE_REGEX */


#ifdef USE_GLOB

Boolean XfwfFCDirsOrGlobFiles(filename, filestats, regex, app_con)
     char	*filename;
     struct stat *filestats;
     XtPointer	regex;
     XtAppContext	app_con;	/* super secret parameter so I can
					   issue warning messages when regexps
					   fail to compile. If I ever find a
					   better way to do this, this
					   parameter will disappear. */
{
  return S_ISDIR(filestats->st_mode) ||
    XfwfFCGlobFiles(filename, regex, app_con) ;
}

#else /* USE_GLOB */

Boolean XfwfFCDirsOrGlobFiles(filename, filestats, regex, app_con)
     char	*filename;
     struct stat *filestats;
     XtPointer	regex;
     XtAppContext	app_con;
{
  Cardinal	num_subs = 0;
  String	subs[1];
  static int	issued=0;

  if (issued)
    return;		/* they don't want to see this a million times */

  /* urgh, glob was not available at the time :( */
  XtAppWarningMsg(app_con, "packageUnavailable", "xfwfFileCompGlob",
		  "XfwfLibraryError",
		  "attempt to use regular expressions in a FileComplete widget (XfwfFCDirsOrGlobFiles).  The FWF library was not compiled with that option.",
		  subs, &num_subs);
  issued = 1;
}
#endif /* USE_GLOB */

/***********************************/
/*                                 */
/* exported convenience procedures */
/*                                 */
/***********************************/

Widget XfwfFileCompleteText(w)
     Widget	w;
{
  return ((XfwfFileCompleteWidget)w)->xfwf_filecomp.text_input;
}

Widget XfwfFileCompleteStatus(w)
     Widget	w;
{
  return ((XfwfFileCompleteWidget)w)->xfwf_filecomp.status_label;
}

Widget XfwfFileCompleteFileList(w)
     Widget	w;
{
  return ((XfwfFileCompleteWidget)w)->xfwf_filecomp.scrolled_list;
}

Widget XfwfFileCompleteBox(w)
     Widget	w;
{
  return ((XfwfFileCompleteWidget)w)->xfwf_filecomp.button_box;
}

Widget XfwfFileCompleteAddButton(w, name, proc, client_data)
     Widget	w;
     char	*name;
     XtCallbackProc	proc;
     XtPointer	client_data;
{
  Widget	rval;
  rval = XtCreateManagedWidget(name, commandWidgetClass,
			       XfwfFileCompleteBox(w), NULL, 0);
  XtAddCallback(rval, XtNcallback, proc, client_data);
  return rval;
}

/* trim off the trailing component of the current filespec and deselect
   items from the ScrolledList */
#if NeedFunctionPrototypes
void XfwfFileCompleteTrimToPath(
     Widget	w)
#else
void XfwfFileCompleteTrimToPath(w)
     Widget	w;
#endif
{
  XfwfFileCompleteWidget	fcw = (XfwfFileCompleteWidget)w;
  char	*s;

  XtVaGetValues(fcw->xfwf_filecomp.text_input, XtNstring, &s, (String)0);
  strcpy(fcw->xfwf_filecomp._filename, s);
  s = strrchr(fcw->xfwf_filecomp._filename, '/');
  if (s==NULL || s[1]) {
    if (s) {
      s[1] = 0;
    } else {
      fcw->xfwf_filecomp._filename[0] = 0;
    }
    set_filename(w, NULL);
  }
  XfwfScrolledListUnhighlightAll(fcw->xfwf_filecomp.scrolled_list);
}


enum xfwfFileCompleteRegexFlavor
XfwfFileCompleteRegexFlavor()
{
#ifdef USE_REGEX
#ifdef GNU_REGEX
  return xfwfFC_Emacs;
#else
  return xfwfFC_ATT;
#endif
#else
  return xfwfFC_NoRegex;
#endif
}

int XfwfFileCompleteHasGlob()
{
#ifdef USE_GLOB
  return 1;
#else
  return 0;
#endif
}

void XfwfFileCompleteForceRescan(w)
     Widget	w;
{
  if (!XtIsSubclass(w, xfwffileCompleteWidgetClass))
    return;			/* What ARE you trying to do? */
  /* force an update */
  _UpdateFilesList((XfwfFileCompleteWidget)w);
}
