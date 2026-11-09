/*
 * test_wlistview - WListView multi-column list: columns, rows, selection,
 * keyboard navigation, scrolling and icon cells.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xft/Xft.h>
#include "mls.h"
#include "WListView.h"
#include "WListViewP.h"

static Widget lv;
static XtAppContext app;
static int notify_count = 0;
static int activate_count = 0;
static int last_sel = -2;
static int g_rows = 0;

static void pump(void)
{
    XSync(XtDisplay(lv), False);
    while (XtAppPending(app))
        XtAppProcessEvent(app, XtIMAll);
}

static void act(const char *name)
{
    XtCallActionProc(lv, (String)name, NULL, NULL, 0);
    pump();
}

static void on_notify(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd;
    notify_count++;
    last_sel = (int)(intptr_t)cb;
}

static void on_activate(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd; (void)cb;
    activate_count++;
}

static void set_int_array(const char *res, const int *vals, int n)
{
    int list = m_create(n, sizeof(int));
    int i;
    for (i = 0; i < n; i++) m_put(list, &vals[i]);
    XtVaSetValues(lv, res, list, NULL);
}

static void set_rows(char **strs, int n)
{
    int i;
    if (g_rows) { m_free_strings(g_rows, 0); g_rows = 0; }
    g_rows = m_create(n, sizeof(char *));
    for (i = 0; i < n; i++) {
        char *s = strdup(strs[i]);
        m_put(g_rows, &s);
    }
    XtVaSetValues(lv, "rows", g_rows, NULL);
}

static const char *icon_path(void)
{
    static char buf[512];
    if (access("LuaRunner/alert.svg", R_OK) == 0)
        snprintf(buf, sizeof(buf), "LuaRunner/alert.svg");
    else if (access("../LuaRunner/alert.svg", R_OK) == 0)
        snprintf(buf, sizeof(buf), "../LuaRunner/alert.svg");
    else
        snprintf(buf, sizeof(buf), "/nonexistent-icon.svg");
    return buf;
}

int main(int argc, char **argv)
{
    Widget toplevel;
    int widths[3] = { 0, 90, 70 };
    int aligns[3] = { 0, 2, 0 };
    int kinds[3] = { 2, 0, 0 };
    char **rows;
    char iconrow[600];
    int i, n = 100;
    WListViewWidget lww;
    int pos = 0;

    toplevel = XtVaAppInitialize(&app, "TestWListView", NULL, 0,
                                 &argc, argv, NULL,
                                 XtNwidth, 520, XtNheight, 420, NULL);

    lv = XtVaCreateManagedWidget("lv", wListViewWidgetClass, toplevel,
                                 XtNwidth, 460, XtNheight, 300,
                                 "columnCount", 3,
                                 "columnTitles", "Name\tSize\tType",
                                 "selected", -1,
                                 NULL);
    XtAddCallback(lv, "notify", on_notify, NULL);
    XtAddCallback(lv, "activate", on_activate, NULL);

    set_int_array("columnWidths", widths, 3);
    set_int_array("columnAligns", aligns, 3);
    set_int_array("columnKinds", kinds, 3);

    rows = calloc((size_t)n, sizeof(char *));
    snprintf(iconrow, sizeof(iconrow), "@%s|folder", icon_path());
    rows[0] = iconrow;
    for (i = 1; i < n; i++) {
        char *s = malloc(160);
        snprintf(s, 160, "file_%03d.txt\t%4d KB\tfile", i, i * 3);
        rows[i] = s;
    }

    XtRealizeWidget(toplevel);
    pump();

    /* rows must be set after realize too, to exercise set_values */
    set_rows(rows, n);
    pump();

    lww = (WListViewWidget)lv;
    assert(lww->wListView.ncols == 3);
    assert(lww->wListView.row_h > 0);
    assert(lww->wListView.list_w > 0);
    assert(lww->wListView.col_w[1] == 90);
    assert(lww->wListView.col_w[2] == 70);
    assert(lww->wListView.col_w[0] > 0);
    assert(lww->wListView.total_w == lww->wListView.list_w);
    assert(lww->wListView.selected == -1);

    /* keyboard navigation + callbacks */
    act("move_down");
    assert(lww->wListView.selected == 0);
    assert(notify_count == 1 && last_sel == 0);
    act("move_down");
    assert(lww->wListView.selected == 1);
    assert(notify_count == 2 && last_sel == 1);
    act("activate");
    assert(activate_count == 1);

    act("move_end");
    assert(lww->wListView.selected == n - 1);
    assert(lww->wListView.top_y > 0);
    act("move_home");
    assert(lww->wListView.selected == 0);
    assert(lww->wListView.top_y == 0);

    act("page_down");
    assert(lww->wListView.top_y > 0);
    act("scroll_up");
    act("scroll_down");
    assert(lww->wListView.top_y > 0);

    /* scrollbar position is synced */
    XtVaGetValues(lww->wListView.vsb, "pos", &pos, NULL);
    assert(pos > 0);

    /* resizing the widget must resize the backing cache and the list area,
     * not only the slider children. */
    {
        int old_w = lww->wListView.list_w;
        int old_h = lww->wListView.list_h;
        int old_core_w = (int)lww->core.width;
        int old_core_h = (int)lww->core.height;
        /* Simulate the parent geometry manager resizing the child. */
        XtConfigureWidget(lv, lww->core.x, lww->core.y,
                          (Dimension)(old_core_w + 100),
                          (Dimension)(old_core_h + 80), 0);
        pump();
        assert((int)lww->core.width > old_core_w);
        assert(lww->wListView.cache_w == (int)lww->core.width);
        assert(lww->wListView.cache_h == (int)lww->core.height);
        assert(lww->wListView.list_w > old_w);
        assert(lww->wListView.list_h > old_h);
    }

    /* Regression: a partially scrolled row must not paint over the header. */
    {
        int rh = lww->wListView.row_h;
        int hh = lww->wListView.header_h;
        XImage *img;
        unsigned long px;
        Pixel hdr_bg, sel_bg;

        assert(rh > 12 && hh > 8);
        XtVaSetValues(lv, "selected", 0, NULL);
        lww->wListView.top_y = rh / 2;      /* fractional scroll */
        XtCallActionProc(lv, "refresh", NULL, NULL, 0);
        pump();

        hdr_bg = lww->wListView.col_px[4];  /* C_HDR_BG */
        sel_bg = lww->wListView.col_px[2];  /* C_SEL_BG */
        img = XGetImage(XtDisplay(lv), lww->wListView.pixmap,
                        5, hh - 5, 1, 1, AllPlanes, ZPixmap);
        assert(img != NULL);
        px = XGetPixel(img, 0, 0);
        XDestroyImage(img);
        assert(px == hdr_bg);
        assert(px != sel_bg);
    }

    /* selection survives a repaint / new rows */
    act("move_down");
    XtVaSetValues(lv, "selected", 5, NULL);
    pump();
    assert(lww->wListView.selected == 5);
    assert(notify_count >= 3);

    for (i = 0; i < n; i++) if (i != 0 && rows[i]) free(rows[i]);
    free(rows);
    m_free_strings(g_rows, 0);
    g_rows = 0;

    XtDestroyWidget(toplevel);
    printf("test_wlistview passed\n");
    return 0;
}
