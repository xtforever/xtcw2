/* Self-checking test for the wbuild-ported Gridbox.
 *
 * This does not call XtcwRegister: it uses the class pointers directly so the
 * binary can link the ported Gridbox without pulling in libplainc.a.
 *
 * The same source is compiled twice by the Makefile:
 *   test_port_gridbox_orig  - original plainc Gridbox from libplainc.a
 *   test_port_gridbox       - wbuild wrapper + gridbox_core.a
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Gridbox.h>
#include "mls.h"
#include "m_tool.h"
#include "theme.h"

static int x11_errors = 0;
static int xt_warnings = 0;

static int x11_error_handler(Display *d, XErrorEvent *e)
{
    (void)d;
    (void)e;
    x11_errors++;
    return 0;
}

static void warning_handler(String msg)
{
    fprintf(stderr, "XT WARNING: %s\n", msg ? msg : "(null)");
    xt_warnings++;
}

static Widget make_child(Widget parent, const char *name, int gx, int gy,
                         int gw, int gh, int wx, int wy, FillType fill)
{
    return XtVaCreateManagedWidget(name, wlabelWidgetClass, parent,
        XtNlabel, name,
        XtNgridx, gx,
        XtNgridy, gy,
        XtNgridWidth, gw,
        XtNgridHeight, gh,
        XtNweightx, wx,
        XtNweighty, wy,
        XtNfill, (XtArgVal)fill,
        NULL);
}

static void get_geom(Widget w, Position *x, Position *y,
                     Dimension *width, Dimension *height)
{
    XtVaGetValues(w, XtNx, x, XtNy, y, XtNwidth, width, XtNheight, height, NULL);
}

int main(int argc, char **argv)
{
    XtAppContext app;

    m_init();
    conststr_init();

    XtSetWarningHandler(warning_handler);
    XSetErrorHandler(x11_error_handler);

    Widget shell = XtVaAppInitialize(&app, "PortGridbox", NULL, 0,
        &argc, argv, NULL,
        XtNallowShellResize, True,
        XtNmappedWhenManaged, False,
        NULL);

    /* 1. Basic 2x2 grid. */
    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, shell,
        XtNdefaultDistance, 4,
        NULL);

    Widget c[4];
    for (int i = 0; i < 4; i++) {
        char name[32];
        snprintf(name, sizeof(name), "c%d", i);
        c[i] = make_child(grid, name, i % 2, i / 2, 1, 1, 1, 1, FillBoth);
    }

    /* 2. Weight distribution: 1:2. */
    Widget grid2 = XtVaCreateManagedWidget("grid2", gridboxWidgetClass, shell,
        XtNdefaultDistance, 4,
        NULL);
    Widget w1 = make_child(grid2, "w1", 0, 0, 1, 1, 1, 0, FillBoth);
    Widget w2 = make_child(grid2, "w2", 1, 0, 1, 1, 2, 0, FillBoth);

    /* 3. Column spanning. */
    Widget grid3 = XtVaCreateManagedWidget("grid3", gridboxWidgetClass, shell,
        XtNdefaultDistance, 4,
        NULL);
    Widget span = make_child(grid3, "span", 0, 0, 2, 1, 1, 0, FillBoth);
    Widget n1 = make_child(grid3, "n1", 0, 1, 1, 1, 1, 0, FillBoth);
    Widget n2 = make_child(grid3, "n2", 1, 1, 1, 1, 1, 0, FillBoth);

    /* 4. Fill modes. */
    Widget grid4 = XtVaCreateManagedWidget("grid4", gridboxWidgetClass, shell,
        XtNdefaultDistance, 4,
        NULL);
    make_child(grid4, "f0", 0, 0, 1, 1, 0, 0, FillNone);
    make_child(grid4, "f1", 1, 0, 1, 1, 1, 0, FillWidth);
    make_child(grid4, "f2", 0, 1, 1, 1, 0, 1, FillHeight);
    make_child(grid4, "f3", 1, 1, 1, 1, 1, 1, FillBoth);

    /* 5. Exercise the string argument path through XtVaTypedArg and verify
     * that the FillType converter actually runs. */
    Widget fstr = XtVaCreateManagedWidget("fstr", wlabelWidgetClass, grid4,
        XtNlabel, "fstr",
        XtNgridx, 2, XtNgridy, 0,
        XtVaTypedArg, XtNfill, XtRString, "both", sizeof("both"),
        NULL);

    XtRealizeWidget(shell);

    /* A top-level Shell gives a managed child its preferred size, so force
     * the test geometry explicitly after realize. */
    XtConfigureWidget(grid, 0, 0, 240, 160, 0);
    XtConfigureWidget(grid2, 0, 0, 300, 60, 0);
    XtConfigureWidget(grid3, 0, 0, 300, 100, 0);
    XtConfigureWidget(grid4, 0, 0, 320, 120, 0);

    Position x[4], y[4];
    Dimension w[4], h[4];

    for (int i = 0; i < 4; i++) {
        get_geom(c[i], &x[i], &y[i], &w[i], &h[i]);
        assert(w[i] > 0 && h[i] > 0);
    }
    assert(x[0] != x[1]);
    assert(y[0] != y[2]);

    /* The ported class installs the plain-C constraint table in
     * class_initialize.  Read every constraint resource back through Xt: this
     * fails if the table is missing, if offsets/types are wrong, or if
     * constraint_initialize did not run (margin would stay -1). */
    {
        Position gx = -1, gy = -1;
        Dimension gw = 0, gh = 0;
        int wtx = -1, wty = -1, margin = -99;
        FillType fl = 0;
        XtVaGetValues(c[0],
            XtNgridx, &gx, XtNgridy, &gy,
            XtNgridWidth, &gw, XtNgridHeight, &gh,
            XtNweightx, &wtx, XtNweighty, &wty,
            XtNfill, &fl, XtNmargin, &margin, NULL);
        assert(gx == 0 && gy == 0);
        assert(gw == 1 && gh == 1);
        assert(wtx == 1 && wty == 1);
        assert(fl == FillBoth);
        assert(margin == 4);
    }

    Dimension ww1, ww2;
    get_geom(w1, &x[0], &y[0], &ww1, &h[0]);
    get_geom(w2, &x[1], &y[1], &ww2, &h[1]);
    assert(ww1 > 0 && ww2 > 0);
    assert(ww2 > ww1);

    Dimension sw, nw1, nw2;
    get_geom(span, &x[0], &y[0], &sw, &h[0]);
    get_geom(n1, &x[1], &y[1], &nw1, &h[1]);
    get_geom(n2, &x[2], &y[2], &nw2, &h[2]);
    assert(sw > nw1);
    assert(sw > nw2);

    {
        FillType fv = 0;
        XtVaGetValues(fstr, XtNfill, &fv, NULL);
        assert(fv == FillBoth);
    }

    assert(x11_errors == 0);
    assert(xt_warnings == 0);

    printf("PASS: %s (x11_errors=%d, xt_warnings=%d)\n",
           argv[0], x11_errors, xt_warnings);

    XtDestroyWidget(shell);
    theme_destroy();
    conststr_free();
    m_destruct();
    return 0;
}
