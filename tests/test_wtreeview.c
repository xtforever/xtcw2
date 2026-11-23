/*
 * test_wtreeview - WTreeView scrolling, keyboard navigation and focus state.
 *
 * The widget keeps the full node list in MLS and draws only the rows that fit
 * into its allocated height.  These tests use a fixed-height tree (autoHeight
 * False) so the scroll window is deterministic under Xvfb.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WTreeView.h>
#include <xtcw/Gridbox.h>
#include <xtcw/register_wb.h>
#include "mls.h"

static XtAppContext app;
static Widget tree;
static int g_nodes = 0;
static int cb_count = 0;
static int cb_last = -1;

static void pump(void)
{
    XSync(XtDisplay(tree), False);
    while (XtAppPending(app))
        XtAppProcessEvent(app, XtIMAll);
}

static void act(const char *name)
{
    XtCallActionProc(tree, (String)name, NULL, NULL, 0);
    pump();
}

static void on_select(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd;
    cb_count++;
    cb_last = (int)(intptr_t)cb;
}

static void set_nodes(char **strs, int n)
{
    int i;
    if (g_nodes) { m_free_strings(g_nodes, 0); g_nodes = 0; }
    g_nodes = m_create(n, sizeof(char *));
    for (i = 0; i < n; i++) {
        char *s = strdup(strs[i]);
        m_put(g_nodes, &s);
    }
    XtVaSetValues(tree, "nodes", g_nodes, NULL);
    pump();
}

static int get_int(const char *res)
{
    int v = -999;
    XtVaGetValues(tree, res, &v, NULL);
    return v;
}

int main(int argc, char **argv)
{
    m_init();

    Widget top = XtOpenApplication(&app, "TestWTreeView", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);
    XtVaSetValues(top, XtNwidth, (Dimension)400, XtNheight, (Dimension)300, NULL);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, top,
                                          XtNwidth, (Dimension)400,
                                          XtNheight, (Dimension)300,
                                          XtNdefaultDistance, 4,
                                          NULL);
    tree = XtVaCreateManagedWidget("tree", wTreeViewWidgetClass, grid,
                                   XtNwidth, (Dimension)300,
                                   XtNheight, (Dimension)120,
                                   XtNgridx, 0,
                                   XtNgridy, 0,
                                   XtNweightx, 0,
                                   XtNweighty, 0,
                                   XtVaTypedArg, XtNfill, XtRString, "none", sizeof("none"),
                                   "itemHeight", 20,
                                   "indent", 16,
                                   "autoHeight", False,
                                   NULL);
    XtAddCallback(tree, "callback", on_select, NULL);

    /* 40 flat nodes -> 40 visible rows. */
    {
        char *rows[40];
        char buf[40][32];
        int i;
        for (i = 0; i < 40; i++) {
            snprintf(buf[i], sizeof buf[i], "0\tRow %d", i);
            rows[i] = buf[i];
        }
        set_nodes(rows, 40);
    }

    XtRealizeWidget(top);
    pump();

    assert(get_int("visibleCount") == 40);
    assert(get_int("visibleRows") == 5);   /* (120 - 8) / 20 */
    assert(get_int("firstVisible") == 0);
    assert(get_int("selected") == -1);

    /* Selecting a row below the window scrolls it into view. */
    {
        String p[1];
        p[0] = "20";
        XtCallActionProc(tree, "select_node", NULL, p, 1);
        pump();
        assert(get_int("selected") == 20);
        assert(get_int("firstVisible") == 16);
        assert(cb_last == 20);
    }

    /* Home/End jump to the first/last visible row and keep it visible. */
    act("move_home");
    assert(get_int("selected") == 0);
    assert(get_int("firstVisible") == 0);
    act("move_end");
    assert(get_int("selected") == 39);
    assert(get_int("firstVisible") == 35);

    /* PageUp/PageDown move by the number of rows in the viewport. */
    act("page_up");
    assert(get_int("selected") == 34);
    assert(get_int("firstVisible") == 34);
    act("page_down");
    assert(get_int("selected") == 39);
    assert(get_int("firstVisible") == 35);

    /* Wheel-style scrolling moves the window without moving the selection. */
    act("scroll_up");
    assert(get_int("firstVisible") == 32);
    assert(get_int("selected") == 39);
    act("scroll_down");
    assert(get_int("firstVisible") == 35);
    act("scroll_down");                    /* clamped at the end */
    assert(get_int("firstVisible") == 35);

    /* Up/Down keep the selection inside the window. */
    act("move_up");
    assert(get_int("selected") == 38);
    assert(get_int("firstVisible") == 35);

    /* Focus state is exposed and drives the focus-ring draw path. */
    {
        Boolean focused = True;
        XtVaGetValues(tree, "hasFocus", &focused, NULL);
        assert(focused == False);
        act("focus");
        XtVaGetValues(tree, "hasFocus", &focused, NULL);
        assert(focused == True);
        act("focus_out");
        XtVaGetValues(tree, "hasFocus", &focused, NULL);
        assert(focused == False);
        act("focus_in");
        XtVaGetValues(tree, "hasFocus", &focused, NULL);
        assert(focused == True);
        act("focus_out");
        XtVaGetValues(tree, "hasFocus", &focused, NULL);
        assert(focused == False);

        /* Direct X focus (no WM under Xvfb) delivers FocusIn to the tree. */
        XSetInputFocus(XtDisplay(tree), XtWindow(tree), RevertToParent, CurrentTime);
        pump();
        XtVaGetValues(tree, "hasFocus", &focused, NULL);
        assert(focused == True);
    }

    /* Collapsing/expanding changes visibleCount and keeps the window valid. */
    {
        char *hier[7] = {
            "0\tRoot", "1\tChild A", "2\tGrandchild",
            "1\tChild B", "0\tSecond", "1\tSecond child", "2\tDeep"
        };
        set_nodes(hier, 7);
        assert(get_int("visibleCount") == 2);   /* two collapsed roots */
        assert(get_int("firstVisible") == 0);
        {
            String p[1]; p[0] = "0";
            XtCallActionProc(tree, "select_node", NULL, p, 1);
        }
        act("expand");
        assert(get_int("visibleCount") == 4);   /* root + 2 children + second root */
        act("collapse");
        assert(get_int("visibleCount") == 2);
    }

    assert(cb_count > 0);
    XtDestroyWidget(top);
    m_free_strings(g_nodes, 0);
    g_nodes = 0;
    printf("test_wtreeview PASSED\n");
    return 0;
}
