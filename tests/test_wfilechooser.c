/*
 * test_wfilechooser - WfileChooser over WListView: directory listing,
 * filtering, hidden files, navigation and accept/cancel callbacks.
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
#include <X11/Xft/Xft.h>
#include "mls.h"
#include "WfileChooser.h"
#include "WfileChooserP.h"
#include "WListView.h"
#include "WfileSelector.h"
#include "WfileSelectorP.h"

static Widget fc;
static XtAppContext app;
static int accept_count = 0;
static int cancel_count = 0;
static char accepted[4096];
static int selector_accept_count = 0;
static char selector_accepted[4096];

static void pump(void)
{
    XSync(XtDisplay(fc), False);
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

int main(int argc, char **argv)
{
    Widget toplevel;
    char tmpl[] = "/tmp/wfc_test_XXXXXX";
    char *dir;
    char subdir[4096];

    dir = mkdtemp(tmpl);
    assert(dir != NULL);
    touch(dir, "alpha.txt");
    touch(dir, "beta.c");
    touch(dir, ".hidden");
    snprintf(subdir, sizeof(subdir), "%s/sub", dir);
    assert(mkdir(subdir, 0700) == 0);

    toplevel = XtVaAppInitialize(&app, "TestWfileChooser", NULL, 0,
                                 &argc, argv, NULL,
                                 XtNwidth, 760, XtNheight, 560, NULL);
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

    /* double-click / activate a directory navigates into it */
    XtVaSetValues(((WfileChooserWidget)fc)->wfileChooser.list,
                  "selected", 0, NULL);
    XtCallActionProc(((WfileChooserWidget)fc)->wfileChooser.list,
                     "activate", NULL, NULL, 0);
    pump();
    assert(strstr(((WfileChooserWidget)fc)->wfileChooser.dir_mem, "/sub") != NULL);
    assert(rows_len() == 0);

    /* accept a file: selecting + activate calls the accept callback */
    XtVaSetValues(fc, "directory", dir, NULL);
    pump();
    {
        int idx = find_row("alpha.txt");
        assert(idx >= 0);
        XtVaSetValues(((WfileChooserWidget)fc)->wfileChooser.list,
                      "selected", idx, NULL);
        XtCallActionProc(((WfileChooserWidget)fc)->wfileChooser.list,
                         "activate", NULL, NULL, 0);
        pump();
    }
    assert(accept_count == 1);
    assert(strstr(accepted, "alpha.txt") != NULL);

    /* cancel button fires the cancel callback */
    XtCallActionProc(((WfileChooserWidget)fc)->wfileChooser.cancel_btn,
                     "notify", NULL, NULL, 0);
    pump();
    assert(cancel_count == 1);

    /* WfileSelector integration: popup the chooser, pick a file, and verify
     * the selector exposes the full path and its own callback fires. */
    {
        Widget fs = XtVaCreateManagedWidget("fs", wfileSelectorWidgetClass, toplevel,
                                            XtNwidth, 180, XtNheight, 30,
                                            "directory", dir,
                                            "pattern", "*",
                                            NULL);
        Widget ch;
        int idx;
        XtAddCallback(fs, "callback", on_selector_accept, NULL);
        pump();
        XtCallActionProc(fs, "popup_files", NULL, NULL, 0);
        pump();
        ch = ((WfileSelectorWidget)fs)->wfileSelector.chooser;
        assert(ch != NULL);
        XtVaSetValues(ch, "directory", dir, "pattern", "*", "showHidden", 0, NULL);
        pump();
        fc = ch;
        idx = find_row("alpha.txt");
        assert(idx >= 0);
        XtVaSetValues(((WfileChooserWidget)ch)->wfileChooser.list,
                      "selected", idx, NULL);
        XtCallActionProc(((WfileChooserWidget)ch)->wfileChooser.list,
                         "activate", NULL, NULL, 0);
        pump();
        assert(selector_accept_count == 1);
        assert(strstr(selector_accepted, "alpha.txt") != NULL);
        assert(strstr(((WfileSelectorWidget)fs)->wfileSelector.selectedPath,
                      "alpha.txt") != NULL);
    }

    rmdir(subdir);
    {
        char path[4096];
        snprintf(path, sizeof(path), "%s/alpha.txt", dir); remove(path);
        snprintf(path, sizeof(path), "%s/beta.c", dir); remove(path);
        snprintf(path, sizeof(path), "%s/.hidden", dir); remove(path);
    }
    rmdir(dir);

    XtDestroyWidget(toplevel);
    printf("test_wfilechooser passed\n");
    return 0;
}
