#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Gridbox.h>
#include "mls.h"

static int x11_errors = 0;
static int x11_error_handler(Display *d, XErrorEvent *e) { (void)d; (void)e; x11_errors++; return 0; }
static int warnings = 0;
static void warning_handler(String msg) { (void)msg; warnings++; }

int main(int argc, char **argv) {
    XtAppContext app;
    m_init();
    trace_level = 2;
    XtSetWarningHandler(warning_handler);
    XSetErrorHandler(x11_error_handler);

    Widget shell = XtVaAppInitialize(&app, "Test", NULL, 0,
        &argc, argv, NULL,
        XtNallowShellResize, True,
        XtNmappedWhenManaged, False,
        NULL);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, shell,
        XtNwidth, 300, XtNheight, 200,
        NULL);

    Widget label = XtVaCreateManagedWidget("label", wlabelWidgetClass, grid,
        XtNlabel, "This is a test label with fill=Both",
        XtNfontSize, 12,
        XtNleftGap, 0, XtNrightGap, 0, XtNtopGap, 0, XtNbottomGap, 0,
        XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, "fill", "Both",
        NULL);

    XtRealizeWidget(shell);

    Dimension gw0 = 0, gh0 = 0, lw0 = 0, lh0 = 0;
    XtVaGetValues(grid, XtNwidth, &gw0, XtNheight, &gh0, NULL);
    XtVaGetValues(label, XtNwidth, &lw0, XtNheight, &lh0, NULL);
    printf("Initial: grid=%dx%d label=%dx%d\n", (int)gw0, (int)gh0, (int)lw0, (int)lh0);

    /* Query grid's preferred size */
    XtWidgetGeometry req, reply;
    req.request_mode = CWWidth | CWHeight | XtCWQueryOnly;
    req.width = 500; req.height = 200;
    XtGeometryResult r = XtQueryGeometry((Widget)grid, &req, &reply);
    printf("Grid query_geometry(500,200): result=%d reply=%dx%d (grid.total_wid=%d total_hgt=%d)\n", 
        (int)r, (int)reply.width, (int)reply.height, (int)0, (int)0);

    /* Force resize grid via set_values — this will trigger GridboxResize */
    printf("=== Now calling XtVaSetValues(grid, XtNwidth, 500) ===\n");
    XtVaSetValues(grid, XtNwidth, (Dimension)500, XtNheight, (Dimension)200, NULL);

    Dimension gw1 = 0, gh1 = 0, lw1 = 0, lh1 = 0;
    XtVaGetValues(grid, XtNwidth, &gw1, XtNheight, &gh1, NULL);
    XtVaGetValues(label, XtNwidth, &lw1, XtNheight, &lh1, NULL);
    printf("After XtVaSetValues(500): grid=%dx%d label=%dx%d\n", (int)gw1, (int)gh1, (int)lw1, (int)lh1);

    if (lw1 > lw0)
        printf("PASS: label grew from %d to %d\n", (int)lw0, (int)lw1);
    else
        printf("FAIL: label did not grow (was %d, now %d)\n", (int)lw0, (int)lw1);
    assert(x11_errors == 0);

    printf("PASS: %s\n", argv[0]);
    return 0;
}
