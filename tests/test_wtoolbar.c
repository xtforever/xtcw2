/* Functional test for WToolBar: height, padding, item gap and order. */
#include <assert.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WToolBar.h>
#include <xtcw/Wbutton.h>
#include <xtcw/register_wb.h>
#include "mls.h"

static void geom(Widget w, int *x, int *y, int *width, int *height)
{
    Position px = 0, py = 0;
    Dimension pw = 0, ph = 0;
    XtVaGetValues(w, XtNx, &px, XtNy, &py,
                  XtNwidth, &pw, XtNheight, &ph, NULL);
    *x = px; *y = py; *width = pw; *height = ph;
}

int main(int argc, char **argv)
{
    m_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWToolBar", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    Widget tb = XtVaCreateManagedWidget("tb", wToolBarWidgetClass, top,
                                        XtNwidth, (Dimension)300,
                                        "toolbarHeight", 36,
                                        "itemGap", 8,
                                        "toolbarPadding", 4,
                                        NULL);
    Widget b1 = XtVaCreateManagedWidget("b1", wbuttonWidgetClass, tb,
                                        XtNlabel, "New", NULL);
    Widget b2 = XtVaCreateManagedWidget("b2", wbuttonWidgetClass, tb,
                                        XtNlabel, "Open", NULL);
    Widget b3 = XtVaCreateManagedWidget("b3", wbuttonWidgetClass, tb,
                                        XtNlabel, "Quit", NULL);

    /* The toolbar must not squeeze children below their preferred height:
     * toolbarHeight is a minimum. */
    XtWidgetGeometry req, pref;
    memset(&req, 0, sizeof req);
    pref.request_mode = 0;
    assert(XtQueryGeometry(b1, &req, &pref) == XtGeometryAlmost);
    assert(pref.request_mode & CWHeight);
    int need_h = (int)pref.height;

    XtRealizeWidget(top);

    int bx, by, bw, bh;
    int x1, y1, w1, h1;
    int x2, y2, w2, h2;
    int x3, y3, w3, h3;
    geom(tb, &bx, &by, &bw, &bh);
    geom(b1, &x1, &y1, &w1, &h1);
    geom(b2, &x2, &y2, &w2, &h2);
    geom(b3, &x3, &y3, &w3, &h3);

    /* query_geometry advertises the height needed for the tallest child,
     * even though the explicit toolbarHeight is smaller. */
    {
        XtWidgetGeometry treq, tpref;
        memset(&treq, 0, sizeof treq);
        tpref.request_mode = 0;
        assert(XtQueryGeometry(tb, &treq, &tpref) == XtGeometryAlmost);
        assert(tpref.request_mode & CWHeight);
        assert((int)tpref.height == need_h + 8);
    }

    /* A parent that honours the request gives the toolbar that height and the
     * children keep their preferred height instead of being squeezed. */
    XtResizeWidget(tb, (Dimension)bw, (Dimension)(need_h + 8), 0);
    geom(tb, &bx, &by, &bw, &bh);
    geom(b1, &x1, &y1, &w1, &h1);
    geom(b2, &x2, &y2, &w2, &h2);
    geom(b3, &x3, &y3, &w3, &h3);

    assert(bh == need_h + 8);
    assert(x1 == 4 && y1 == 4 && h1 == need_h);
    assert(x2 == x1 + w1 + 8);
    assert(x3 == x2 + w2 + 8);
    assert(y2 == y1 && y3 == y1);
    assert(h2 == h1 && h3 == h1);

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_destruct();

    printf("test_wtoolbar: PASS\n");
    return 0;
}
