/* IconBoxT -- test file for IconBox widget */

/*
 * 13 May 1993: replaced strerror() with sys_errlist[]
 */

#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <Xfwf/IconBox.h>
#include <Xfwf/ScrollWin.h>
#include <Xfwf/MenuBar.h>
#include <Xfwf/PullDown.h>
#include <Xfwf/Board.h>
#include <Xfwf/Alert.h>
#include "basket1.xpm"
#include "doc.xpm"
#include <Xfwf/TextMenu.h>

#define MAX_LIST 1000
#define new(p) p = (void*) XtMalloc(sizeof(*(p)))

static String fallback_res[] = {
    "*board.location: 0 0 500 300",
    "*board*background: aquamarine",
    "*board.highlightThickness: 0",
    "*menubar.highlightThickness: 0",
    "*menubar.location: 0 0 1.0 20",
    "*quitmenu.label: Menu",
    "*quitmenu.menu: \
        E_xit",
    "*scrwin.highlightThickness: 0",
    "*scrwin.location: 0 20 1.0 1.0-20",
    "*scrwin.hideHScrollbar: True",
    /* "*labelFont: -*-times-bold-i-normal--*-120-*-*-*-*-*", */
    NULL,
};

extern int errno;
extern char *sys_errlist[];

static XtAppContext app_context;
static int hlen, vlen;
static float hunit = 1.0, vunit;
static char vs[20], hs[20];
static Widget toplevel;


/*
 * read_directory -- read the contents of the current directory into list
 */
static void read_directory(list, curdir)
    XfwfIconListItem *list;
    String curdir;
{
    DIR *dir;
    struct dirent *entry;
    int i = 0;
    struct stat buf;

    dir = opendir(curdir);
    while ((entry = readdir(dir))) {
	(void) stat(entry->d_name, &buf);
	list[i].label = XtNewString(entry->d_name);
	list[i].flags = S_ISDIR(buf.st_mode);
	list[i].icon = S_ISDIR(buf.st_mode) ? "basket1.xpm" : "doc.xpm";
	i++;
    }
    list[i].flags = -1;
    (void) closedir(dir);
}


/*
 * append_path -- append a file name to a directory, result in fullpath
 */
static void append_path(fullpath, dir, file)
    char fullpath[], *dir, *file;
{
    if (dir[0] == '/' && dir[1] == '\0')
	(void) strcat(strcpy(fullpath, dir), file);
    else
	(void) strcat(strcat(strcpy(fullpath, dir), "/"), file);
}


/*
 * activate -- if the double click occurred on a directory, cd to it
 */
static void activate(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    int n = (int) call_data;			/* Clicked on this item */
    struct stat buf;
    XfwfIconListItem *lst0, list[MAX_LIST];
    char *dir, *newdir;

    XtVaGetValues(w, XtNuserData, &dir, XtNlist, &lst0, NULL);
    newdir = XtMalloc(MAXPATHLEN+1);
    append_path(newdir, dir, lst0[n].label);
    if (stat(newdir, &buf) == -1)
	(void) XfwfDialog("dialog", toplevel, "ERROR", sys_errlist[errno], NULL);
    else if (! S_ISDIR(buf.st_mode))
	(void) XfwfDialog("dialog", toplevel, "WARNING",
			  "\nThere is no action associated with this file.\n",
			  NULL);
    else if (chdir(newdir) == -1)
	(void) XfwfDialog("dialog", toplevel, "ERROR", sys_errlist[errno], NULL);
    else {
	XtFree(dir);
	XtVaSetValues(w, XtVaTypedArg, XtNcursor, XtRString, "watch",6,NULL);
	read_directory(list, newdir);
	XtVaSetValues(w, XtNlist, list, XtNuserData, newdir, NULL);
	XtVaSetValues(w, XtVaTypedArg, XtNcursor, XtRString, "left_ptr",9,NULL);
    }
}


/*
 * drop -- move the dropped files to the directory they were dropped on
 */
static void drop(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XfwfItemDropInfo *info = (XfwfItemDropInfo*) call_data;
    int i;
    char from[MAXPATHLEN], to[MAXPATHLEN], *fromdir, *todir, err[200];
    XfwfIconListItem *lst0, *lst1, list[MAX_LIST];

    (void) printf("Drop from %s to %s\n",
		  XtName(info->sender), XtName(w));

    XtVaGetValues(info->sender, XtNuserData, &fromdir, XtNlist, &lst0, NULL);
    XtVaGetValues(w, XtNuserData, &todir, XtNlist, &lst1, NULL);

    for (i = 0; i < info->n; i++) {
	append_path(from, fromdir, lst0[info->droplings[i]].label);
	append_path(to, todir, lst1[info->target].label);
	append_path(to, to, lst0[info->droplings[i]].label);

	(void) printf("  moving %s to %s\n", from, to);

	if (link(from, to) == -1) {
	    (void) sprintf(err, "\nError while trying to move %s:\n\n%s\n",
			   lst0[info->droplings[i]].label, sys_errlist[errno]);
	    (void) XfwfDialog("dialog", toplevel, "ERROR", err, NULL);
	} else if (unlink(from) == -1) {
	    (void) sprintf(err, "\nError while trying to unlink %s:\n\n%s\n",
			   lst0[info->droplings[i]].label, sys_errlist[errno]);
	    (void) XfwfDialog("dialog", toplevel, "ERROR", sys_errlist[errno], NULL);
	}
    }
    XtVaSetValues(info->sender,
		  XtVaTypedArg, XtNcursor, XtRString, "watch", 6, NULL);
    XtVaSetValues(w,
		  XtVaTypedArg, XtNcursor, XtRString, "watch", 6, NULL);

    read_directory(list, fromdir);
    XtVaSetValues(info->sender, XtNlist, list, NULL);
    if (w != info->sender) {
	read_directory(list, todir);
	XtVaSetValues(w, XtNlist, list, NULL);
    }

    XtVaSetValues(info->sender,
		  XtVaTypedArg, XtNcursor, XtRString, "left_ptr", 9, NULL);
    XtVaSetValues(w,
		  XtVaTypedArg, XtNcursor, XtRString, "left_ptr", 9, NULL);
}


/*
 * menuselect -- callback for menu
 */
static void menuselect(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XfwfTextMenuData *data = (XfwfTextMenuData *) call_data;

    switch (data->n) {
    case 0:
	exit(0);
    }
}


/*
 * The font is used to compute the vunit and hunit resources, so that
 * the locations can be expressed as multiples of characters. The
 * hunit and vunit resources of the widgets will be set
 * to 1/10 the em width and baseline height, resp.
 */
typedef struct { XFontStruct *font; } AppData;
static AppData app_data;

static XtResource app_resources[] = {
{
    XtNfont,
    XtCFont,
    XtRFontStruct,
    sizeof(XFontStruct*),
    XtOffsetOf(AppData, font),
    XtRString,
    XtDefaultFont }
};

main(argc, argv)
    int argc;
    String argv[];
{
    Widget iconbox, scrwin, menubar, quitmenu, board;
    XfwfIconListItem list[MAX_LIST];
    char *current_dir;
    int status;
    Icon *icon1, *icon2;
    int i;

    toplevel = XtVaAppInitialize
	(&app_context, "IconT", NULL, 0, &argc, argv, fallback_res, NULL);
    /*
     * Find width and height of a QUAD (em) in the current font
     */
    XtVaGetApplicationResources
	(toplevel, &app_data, app_resources, XtNumber(app_resources), NULL);
    vunit = (app_data.font->ascent + app_data.font->descent)/10.0;
    for (i = 0; i < app_data.font->n_properties; i++)
	if (app_data.font->properties[i].name == XA_QUAD_WIDTH) {
	    hunit = app_data.font->properties[i].card32/10.0;
	    break;
	}
    /*
     * Floating point values must be passed as strings,
     * it doesn't work otherwise...
     */

    hlen = 1 + sprintf(hs, "%f", hunit);
    vlen = 1 + sprintf(vs, "%f", vunit);

    current_dir = XtMalloc(MAXPATHLEN+1);
    (void) getcwd(current_dir, MAXPATHLEN);
    read_directory(list, current_dir);

    board = XtVaCreateManagedWidget
	("board", xfwfBoardWidgetClass, toplevel,
	 XtVaTypedArg, XtNhunit, XtRString, hs, hlen,
	 XtVaTypedArg, XtNvunit, XtRString, vs, vlen,
	 NULL);
    menubar = XtVaCreateManagedWidget
	("menubar", xfwfMenuBarWidgetClass, board,
	 XtVaTypedArg, XtNhunit, XtRString, hs, hlen,
	 XtVaTypedArg, XtNvunit, XtRString, vs, vlen,
	 NULL);
    quitmenu = XtVaCreateManagedWidget
	("quitmenu", xfwfPullDownWidgetClass, menubar,
	 XtVaTypedArg, XtNhunit, XtRString, hs, hlen,
	 XtVaTypedArg, XtNvunit, XtRString, vs, vlen,
	 NULL);
    scrwin = XtVaCreateManagedWidget
	("scrwin", xfwfScrolledWindowWidgetClass, board,
	 XtVaTypedArg, XtNhunit, XtRString, hs, hlen,
	 XtVaTypedArg, XtNvunit, XtRString, vs, vlen,
	 NULL);
    iconbox = XtVaCreateManagedWidget
	("iconbox", xfwfIconBoxWidgetClass, scrwin,
	 XtVaTypedArg, XtNhunit, XtRString, hs, hlen,
	 XtVaTypedArg, XtNvunit, XtRString, vs, vlen,
	 XtNuserData, current_dir,
	 NULL);

    /*
     * Create and cache the two icons
     */
    new(icon1);
    new(icon2);
    icon1->attributes.valuemask = XpmSize;
    icon2->attributes.valuemask = XpmSize;
    status = XpmCreatePixmapFromData
	(XtDisplay(toplevel), DefaultRootWindow(XtDisplay(toplevel)),
	 basket1, &icon1->pixmap, &icon1->mask, &icon1->attributes);
    if (status == XpmSuccess || status == XpmColorError)
	XfwfCacheIcon(iconbox, "basket1.xpm", *icon1);
    else
	(void) XfwfDialog("dialog", toplevel, "WARNING",
			  "\nFailed to create icon \"basket1.xpm\"\n",
			  NULL);
    status = XpmCreatePixmapFromData
	(XtDisplay(toplevel), DefaultRootWindow(XtDisplay(toplevel)),
	 doc, &icon2->pixmap, &icon2->mask, &icon2->attributes);
    if (status == XpmSuccess || status == XpmColorError)
	XfwfCacheIcon(iconbox, "doc.xpm", *icon2);
    else
	(void) XfwfDialog("dialog", toplevel, "WARNING",
			  "\nFailed to create icon \"doc.xpm\"\n", NULL);

    XtVaSetValues(iconbox, XtNlist, list, NULL);

    XtAddCallback(quitmenu, XtNactivate, menuselect, NULL);
    XtAddCallback(iconbox, XtNitemActivate, activate, NULL);
    XtAddCallback(iconbox, XtNitemDrop, drop, NULL);

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
