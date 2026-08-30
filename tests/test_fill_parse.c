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

int main(int argc, char **argv) {
    XtAppContext app;
    m_init();
    XSetErrorHandler(x11_error_handler);

    Widget shell = XtVaAppInitialize(&app, "Test", NULL, 0,
        &argc, argv, NULL,
        XtNallowShellResize, True,
        XtNmappedWhenManaged, False,
        NULL);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, shell,
        XtNwidth, 500, XtNheight, 200,
        NULL);

    /* Test 1: String "Both" */
    Widget l1 = XtVaCreateManagedWidget("l1", wlabelWidgetClass, grid,
        XtNlabel, "FillBoth string",
        XtNleftGap, 0, XtNrightGap, 0, XtNtopGap, 0, XtNbottomGap, 0,
        "gridy", 0, "fill", "Both",
        NULL);

    /* Test 2: Integer 3 (FillBoth) */
    Widget l2 = XtVaCreateManagedWidget("l2", wlabelWidgetClass, grid,
        XtNlabel, "FillBoth int",
        XtNleftGap, 0, XtNrightGap, 0, XtNtopGap, 0, XtNbottomGap, 0,
        XtNfill, 3,
        "gridy", 1,
        NULL);

    /* Test 3: String "Width" */
    Widget l3 = XtVaCreateManagedWidget("l3", wlabelWidgetClass, grid,
        XtNlabel, "FillWidth string",
        XtNleftGap, 0, XtNrightGap, 0, XtNtopGap, 0, XtNbottomGap, 0,
        "gridy", 2, "fill", "Width",
        NULL);

    XtRealizeWidget(shell);

    Dimension w1=0, w2=0, w3=0, gw=0, gh=0;
    Position x1=0, x2=0, x3=0;
    XtVaGetValues(l1, XtNwidth, &w1, XtNx, &x1, NULL);
    XtVaGetValues(l2, XtNwidth, &w2, XtNx, &x2, NULL);
    XtVaGetValues(l3, XtNwidth, &w3, XtNx, &x3, NULL);
    XtVaGetValues(grid, XtNwidth, &gw, XtNheight, &gh, NULL);

    printf("Grid: %dx%d\n", (int)gw, (int)gh);
    printf("l1 (\"Both\" str):   w=%d x=%d\n", (int)w1, (int)x1);
    printf("l2 (FillBoth int):   w=%d x=%d\n", (int)w2, (int)x2);
    printf("l3 (\"Width\" str):   w=%d x=%d\n", (int)w3, (int)x3);

    /* If FillWidth is applied, the widget should fill almost the entire grid width (minus margin*2) */
    /* If NOT applied, the widget stays at text width and is centered (x > margin) */
    int expected_fill_width = (int)gw - 2*4; /* margin=4 */
    printf("Expected fill width: ~%d\n", expected_fill_width);

    if (w1 < expected_fill_width - 10) printf("FAIL: \"Both\" string — width not filling\n");
    else printf("OK: \"Both\" string works\n");

    if (w2 < expected_fill_width - 10) printf("FAIL: FillBoth int — width not filling\n");
    else printf("OK: FillBoth int works\n");

    if (w3 < expected_fill_width - 10) printf("FAIL: \"Width\" string — width not filling\n");
    else printf("OK: \"Width\" string works\n");

    assert(x11_errors == 0);
    printf("PASS: %s\n", argv[0]);
    return 0;
}
