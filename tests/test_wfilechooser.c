/*
 * test_wfilechooser - WfileChooser over WListView: directory listing,
 * filtering, hidden files, navigation, dialog states and callbacks.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/keysym.h>
#include <X11/Xft/Xft.h>
#include "mls.h"
#include "WfileChooser.h"
#include "WfileChooserP.h"
#include "WListView.h"
#include "WfileSelector.h"
#include "WfileSelectorP.h"

static Widget fc;
static Widget top;
static XtAppContext app;
static int accept_count = 0;
static int cancel_count = 0;
static char accepted[4096];
static int selector_accept_count = 0;
static int selector_cancel_count = 0;
static char selector_accepted[4096];

static void pump(void)
{
    XSync(XtDisplay(top), False);
    while (XtAppPending(app))
        XtAppProcessEvent(app, XtIMAll);
}

static void on_accept(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd;
    accept_count++;
    snprintf(accepted, sizeof(accepted), "%s", (char *)cb ? (char *)cb : "");
}

static void on_cancel(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd; (void)cb;
    cancel_count++;
}

static void on_selector_accept(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd;
    selector_accept_count++;
    snprintf(selector_accepted, sizeof(selector_accepted), "%s",
             (char *)cb ? (char *)cb : "");
}

static void on_selector_cancel(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd; (void)cb;
    selector_cancel_count++;
}

static int rows_len(void)
{
    int h = ((WfileChooserWidget)fc)->wfileChooser.rows;
    return h ? m_len(h) : 0;
}

static const char *row_at(int i)
{
    int h = ((WfileChooserWidget)fc)->wfileChooser.rows;
    return (h && i < m_len(h)) ? STR(h, i) : "";
}

static int names_len(void)
{
    int h = ((WfileChooserWidget)fc)->wfileChooser.names;
    return h ? m_len(h) : 0;
}

static const char *name_at(int i)
{
    int h = ((WfileChooserWidget)fc)->wfileChooser.names;
    return (h && i < m_len(h)) ? STR(h, i) : "";
}

static const char *status_text(void)
{
    Widget st = ((WfileChooserWidget)fc)->wfileChooser.status;
    char *s = NULL;
    if (st) XtVaGetValues(st, "label", &s, NULL);
    return s ? s : "";
}

static int find_row(const char *name)
{
    int i;
    for (i = 0; i < rows_len(); i++) {
        const char *r = row_at(i);
        const char *tab = strchr(r, '\t');
        int len = tab ? (int)(tab - r) : (int)strlen(r);
        if ((int)strlen(name) == len && strncmp(r, name, (size_t)len) == 0)
            return i;
    }
    return -1;
}

static int find_name(const char *name)
{
    int i;
    for (i = 0; i < names_len(); i++)
        if (strcmp(name_at(i), name) == 0)
            return i;
    return -1;
}

static void touch(const char *dir, const char *name)
{
    char path[4096];
    FILE *f;
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    f = fopen(path, "w");
    assert(f != NULL);
    fputs("x", f);
    fclose(f);
}

static void remove_file(const char *dir, const char *name)
{
    char path[4096];
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    remove(path);
}

static void send_key(Widget w, KeySym ks)
{
    Display *dpy = XtDisplay(w);
    XKeyEvent ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = KeyPress;
    ev.display = dpy;
    ev.window = XtWindow(w);
    ev.root = DefaultRootWindow(dpy);
    ev.time = CurrentTime;
    ev.x = 1;
    ev.y = 1;
    ev.x_root = 1;
    ev.y_root = 1;
    ev.same_screen = True;
    ev.keycode = XKeysymToKeycode(dpy, ks);

    {
        XEvent xev;
        memset(&xev, 0, sizeof(xev));
        xev.xkey = ev;
        XtDispatchEventToWidget(w, &xev);
    }
    pump();
}

static void send_wm_delete(Widget shell)
{
    Display *dpy = XtDisplay(shell);
    Atom protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
    Atom deletew = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XEvent ev;

    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = XtWindow(shell);
    ev.xclient.message_type = protocols;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = (long)deletew;
    ev.xclient.data.l[1] = CurrentTime;

    XSendEvent(dpy, XtWindow(shell), False, NoEventMask, &ev);
    XFlush(dpy);
    pump();
}

static Widget open_selector(Widget fs, const char *dir)
{
    Widget ch;

    XtCallActionProc(fs, "popup_files", NULL, NULL, 0);
    pump();
    ch = ((WfileSelectorWidget)fs)->wfileSelector.chooser;
    assert(ch != NULL);
    XtVaSetValues(ch, "directory", dir, "pattern", "*", "showHidden", 0, NULL);
    pump();
    fc = ch;
    return ch;
}

static void select_row(Widget ch, int idx)
{
    XtVaSetValues(((WfileChooserWidget)ch)->wfileChooser.list,
                  "selected", idx, NULL);
    pump();
}

static void press_ok(Widget ch)
{
    XtCallActionProc(((WfileChooserWidget)ch)->wfileChooser.ok_btn,
                     "notify", NULL, NULL, 0);
    pump();
}

int main(int argc, char **argv)
{
    Widget toplevel;
    char tmpl[] = "/tmp/wfc_test_XXXXXX";
    char auxtmpl[] = "/tmp/wfc_aux_XXXXXX";
    char *dir;
    char *aux;
    char subdir[4096];
    char emptydir[4096];
    char noread[4096];

    dir = mkdtemp(tmpl);
    assert(dir != NULL);
    touch(dir, "alpha.txt");
    touch(dir, "beta.c");
    touch(dir, ".hidden");
    snprintf(subdir, sizeof(subdir), "%s/sub", dir);
    assert(mkdir(subdir, 0700) == 0);
    aux = mkdtemp(auxtmpl);
    assert(aux != NULL);
    snprintf(emptydir, sizeof(emptydir), "%s/empty", aux);
    assert(mkdir(emptydir, 0700) == 0);
    snprintf(noread, sizeof(noread), "%s/noread", aux);
    assert(mkdir(noread, 0700) == 0);
    assert(chmod(noread, 0000) == 0);

    toplevel = XtVaAppInitialize(&app, "TestWfileChooser", NULL, 0,
                                 &argc, argv, NULL,
                                 XtNwidth, 760, XtNheight, 560, NULL);
    top = toplevel;
    fc = XtVaCreateManagedWidget("fc", wfileChooserWidgetClass, toplevel,
                                 XtNwidth, 720, XtNheight, 520,
                                 "directory", dir,
                                 "pattern", "*",
                                 "showHidden", 0,
                                 NULL);
    XtAddCallback(fc, "accept", on_accept, NULL);
    XtAddCallback(fc, "cancel", on_cancel, NULL);
    XtRealizeWidget(toplevel);
    pump();

    /* dirs first, then files, hidden skipped */
    assert(rows_len() == 3);
    assert(find_row("sub") == 0);
    assert(find_row("alpha.txt") >= 0);
    assert(find_row("beta.c") >= 0);
    assert(find_row(".hidden") < 0);

    /* pattern only filters files, directories stay visible */
    XtVaSetValues(fc, "pattern", "*.txt", NULL);
    pump();
    assert(rows_len() == 2);
    assert(find_row("sub") >= 0);
    assert(find_row("alpha.txt") >= 0);
    assert(find_row("beta.c") < 0);

    /* hidden files */
    XtVaSetValues(fc, "pattern", "*", "showHidden", 1, NULL);
    pump();
    assert(rows_len() == 4);
    assert(find_row(".hidden") >= 0);
    XtVaSetValues(fc, "showHidden", 0, NULL);
    pump();

    /* empty directory shows a helpful state instead of a blank list */
    XtVaSetValues(fc, "directory", emptydir, NULL);
    pump();
    assert(rows_len() == 0);
    assert(strstr(status_text(), "empty") != NULL);
    XtVaSetValues(fc, "directory", dir, NULL);
    pump();

    /* filter without matches has its own message */
    XtVaSetValues(fc, "directory", emptydir, NULL);
    pump();
    XtVaSetValues(fc, "pattern", "*.nomatch", NULL);
    pump();
    assert(rows_len() == 0);
    assert(strstr(status_text(), "No files match") != NULL);
    XtVaSetValues(fc, "pattern", "*", NULL);
    XtVaSetValues(fc, "directory", dir, NULL);
    pump();

    /* unreadable directory reports a recoverable error, no fake row */
    if (geteuid() != 0) {
        XtVaSetValues(fc, "directory", noread, NULL);
        pump();
        assert(rows_len() == 0);
        assert(strstr(status_text(), "open") != NULL);
        XtVaSetValues(fc, "directory", dir, NULL);
        pump();
    }

    /* Open without a selection keeps the dialog open and explains why */
    {
        int before = accept_count;
        XtVaSetValues(((WfileChooserWidget)fc)->wfileChooser.list,
                      "selected", -1, NULL);
        pump();
        press_ok(fc);
        assert(accept_count == before);
        assert(strstr(status_text(), "Select a file") != NULL);
    }

    /* selectDirs accepts a selected directory (navigation stays on activate) */
    {
        int idx = find_name("sub");
        int before = accept_count;
        assert(idx >= 0);
        XtVaSetValues(fc, "selectDirs", 1, NULL);
        select_row(fc, idx);
        press_ok(fc);
        assert(accept_count == before + 1);
        assert(strstr(accepted, "/sub") != NULL);
        XtVaSetValues(fc, "selectDirs", 0, NULL);
        pump();
    }

    /* double-click / activate a directory navigates into it */
    select_row(fc, 0);
    XtCallActionProc(((WfileChooserWidget)fc)->wfileChooser.list,
                     "activate", NULL, NULL, 0);
    pump();
    assert(strstr(((WfileChooserWidget)fc)->wfileChooser.dir_mem, "/sub") != NULL);
    assert(rows_len() == 0);

    /* accept a file: selecting + activate calls the accept callback */
    XtVaSetValues(fc, "directory", dir, NULL);
    pump();
    {
        int idx = find_name("alpha.txt");
        assert(idx >= 0);
        select_row(fc, idx);
        XtCallActionProc(((WfileChooserWidget)fc)->wfileChooser.list,
                         "activate", NULL, NULL, 0);
        pump();
    }
    assert(accept_count >= 1);
    assert(strstr(accepted, "alpha.txt") != NULL);

    /* cancel button fires the cancel callback */
    XtCallActionProc(((WfileChooserWidget)fc)->wfileChooser.cancel_btn,
                     "notify", NULL, NULL, 0);
    pump();
    assert(cancel_count == 1);

    /* Filenames with the list's icon marker, a pipe or control characters
     * must round-trip through the owned-name table. */
    touch(dir, "@at.txt");
    touch(dir, "a|b.txt");
    touch(dir, "tab\tname.txt");
    XtCallActionProc(((WfileChooserWidget)fc)->wfileChooser.refresh_btn,
                     "notify", NULL, NULL, 0);
    pump();
    {
        const char *special[] = { "@at.txt", "a|b.txt", "tab\tname.txt" };
        int i;
        for (i = 0; i < 3; i++) {
            int idx = find_name(special[i]);
            int before = accept_count;
            assert(idx >= 0);
            select_row(fc, idx);
            XtCallActionProc(((WfileChooserWidget)fc)->wfileChooser.list,
                             "activate", NULL, NULL, 0);
            pump();
            assert(accept_count == before + 1);
            assert(strstr(accepted, special[i]) != NULL);
        }
    }
    remove_file(dir, "@at.txt");
    remove_file(dir, "a|b.txt");
    remove_file(dir, "tab\tname.txt");

    /* WfileSelector integration: sheet geometry, focus, Escape, Return and
     * WM_DELETE_WINDOW. */
    {
        Widget fs = XtVaCreateManagedWidget("fs", wfileSelectorWidgetClass, toplevel,
                                            XtNwidth, 180, XtNheight, 30,
                                            "directory", dir,
                                            "pattern", "*",
                                            NULL);
        Widget ch;
        Widget shell;
        Dimension w = 0, h = 0;
        Widget parent = NULL;
        XtAddCallback(fs, "callback", on_selector_accept, NULL);
        XtAddCallback(fs, "cancel", on_selector_cancel, NULL);
        pump();

        XtSetKeyboardFocus(toplevel, fs);
        ch = open_selector(fs, dir);
        shell = ((WfileSelectorWidget)fs)->wfileSelector.popup_shell;
        assert(shell != NULL);

        XtVaGetValues(shell, XtNwidth, &w, XtNheight, &h,
                      "transientFor", &parent, NULL);
        assert(parent == toplevel);
        assert(w == 520);
        assert(w <= (760 * 4) / 5);
        assert(h >= 320);

        /* Escape cancels and restores focus to the invoker. */
        XtCallActionProc(ch, "focus_default", NULL, NULL, 0);
        pump();
        {
            Widget focus = XtGetKeyboardFocusWidget(shell);
            assert(focus != NULL);
            send_key(focus, XK_Escape);
        }
        assert(selector_cancel_count == 1);
        assert(XtGetKeyboardFocusWidget(toplevel) == fs);

        /* Escape also cancels when the filter field has focus. */
        ch = open_selector(fs, dir);
        {
            Widget shell2 = ((WfileSelectorWidget)fs)->wfileSelector.popup_shell;
            Widget filter = ((WfileChooserWidget)ch)->wfileChooser.filter_ed;
            XtSetKeyboardFocus(shell2, filter);
            send_key(filter, XK_Escape);
        }
        assert(selector_cancel_count == 2);
        assert(XtGetKeyboardFocusWidget(toplevel) == fs);

        /* Return on the focused list accepts the selected file. */
        ch = open_selector(fs, dir);
        {
            int idx = find_name("alpha.txt");
            assert(idx >= 0);
            select_row(ch, idx);
            XtCallActionProc(ch, "focus_default", NULL, NULL, 0);
            pump();
            {
                Widget focus = XtGetKeyboardFocusWidget(
                    ((WfileSelectorWidget)fs)->wfileSelector.popup_shell);
                assert(focus != NULL);
                send_key(focus, XK_Return);
            }
        }
        assert(selector_accept_count == 1);
        assert(strstr(selector_accepted, "alpha.txt") != NULL);
        assert(strstr(((WfileSelectorWidget)fs)->wfileSelector.selectedPath,
                      "alpha.txt") != NULL);
        assert(XtGetKeyboardFocusWidget(toplevel) == fs);

        /* The selector owns its selectedPath copy and updates it on the next
         * accepted file. */
        ch = open_selector(fs, dir);
        {
            int idx = find_name("beta.c");
            assert(idx >= 0);
            select_row(ch, idx);
            XtCallActionProc(ch, "focus_default", NULL, NULL, 0);
            pump();
            send_key(XtGetKeyboardFocusWidget(
                ((WfileSelectorWidget)fs)->wfileSelector.popup_shell), XK_Return);
        }
        assert(selector_accept_count == 2);
        assert(strstr(((WfileSelectorWidget)fs)->wfileSelector.selectedPath,
                      "beta.c") != NULL);

        /* Window-manager close is treated as Cancel. */
        ch = open_selector(fs, dir);
        (void)ch;
        shell = ((WfileSelectorWidget)fs)->wfileSelector.popup_shell;
        send_wm_delete(shell);
        assert(selector_cancel_count == 3);
        assert(XtGetKeyboardFocusWidget(toplevel) == fs);
    }

    chmod(noread, 0700);
    rmdir(noread);
    rmdir(emptydir);
    rmdir(aux);
    rmdir(subdir);
    {
        char path[4096];
        snprintf(path, sizeof(path), "%s/alpha.txt", dir); remove(path);
        snprintf(path, sizeof(path), "%s/beta.c", dir); remove(path);
        snprintf(path, sizeof(path), "%s/.hidden", dir); remove(path);
    }
    rmdir(dir);

    XtDestroyWidget(toplevel);
    m_destruct();
    printf("test_wfilechooser passed\n");
    return 0;
}
