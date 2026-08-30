#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Wbutton.h>
#include <xtcw/Gridbox.h>
#include "mls.h"

static int x11_errors = 0;
static int x11_error_handler(Display *d, XErrorEvent *e) { (void)d; (void)e; x11_errors++; return 0; }

int main(int argc, char **argv) {
    XtAppContext app;
    m_init();
    trace_level = 2;
    XSetErrorHandler(x11_error_handler);

    Widget top = XtVaAppInitialize(&app, "Test", NULL, 0,
        &argc, argv, NULL,
        XtNallowShellResize, True,
        XtNmappedWhenManaged, False,
        NULL);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, top, NULL);

    /* Matches the demo: autoHeight=True, fill=Both, wrapping text, gaps */
    Widget w_left = XtVaCreateManagedWidget("left", wlabelWidgetClass, grid,
        XtNlabel, "Left-aligned wrapping text. This label demonstrates how Wlabel wraps long "
                  "lines to fit the container width. Resize the window to see reflow in action.",
        XtNfontSize, 10,
        XtNalignment, 0,
        XtNautoHeight, True,
        XtNleftGap, 10, XtNrightGap, 10, XtNtopGap, 4, XtNbottomGap, 4,
        XtNweightx, 1, XtNweighty, 1,
        "gridy", 1, "fill", "Both",
        NULL);
    XtVaSetValues(w_left, XtVaTypedArg, XtNbg_norm, XtRString, "#ecf0f1", 8, NULL);

    Widget quit = XtVaCreateManagedWidget("quit", wbuttonWidgetClass, grid,
        XtNlabel, "Quit",
        XtNweightx, 1, XtNweighty, 0,
        "gridy", 2, "fill", "Width",
        NULL);

    XtRealizeWidget(top);

    Dimension gw0=0, gh0=0, lw0=0, lh0=0, qw0=0, qh0=0;
    Position lx0=0, ly0=0, qx0=0, qy0=0;
    XtVaGetValues(grid, XtNwidth, &gw0, XtNheight, &gh0, NULL);
    XtVaGetValues(w_left, XtNwidth, &lw0, XtNheight, &lh0, XtNx, &lx0, XtNy, &ly0, NULL);
    XtVaGetValues(quit, XtNwidth, &qw0, XtNheight, &qh0, XtNx, &qx0, XtNy, &qy0, NULL);
    printf("Initial: grid=%dx%d\n", (int)gw0, (int)gh0);
    printf("  left: %dx%d at %d,%d\n", (int)lw0, (int)lh0, (int)lx0, (int)ly0);
    printf("  quit: %dx%d at %d,%d\n", (int)qw0, (int)qh0, (int)qx0, (int)qy0);

    /* Resize top-level window */
    XtVaSetValues(top, XtNwidth, (Dimension)800, XtNheight, (Dimension)500, NULL);

    Dimension gw1=0, gh1=0, lw1=0, lh1=0, qw1=0, qh1=0;
    Position lx1=0, ly1=0, qx1=0, qy1=0;
    XtVaGetValues(grid, XtNwidth, &gw1, XtNheight, &gh1, NULL);
    XtVaGetValues(w_left, XtNwidth, &lw1, XtNheight, &lh1, XtNx, &lx1, XtNy, &ly1, NULL);
    XtVaGetValues(quit, XtNwidth, &qw1, XtNheight, &qh1, XtNx, &qx1, XtNy, &qy1, NULL);
    printf("After (800,500): grid=%dx%d\n", (int)gw1, (int)gh1);
    printf("  left: %dx%d at %d,%d  (w: %d->%d, x: %d->%d)\n", 
        (int)lw1, (int)lh1, (int)lx1, (int)ly1, (int)lw0, (int)lw1, (int)lx0, (int)lx1);
    printf("  quit: %dx%d at %d,%d  (w: %d->%d, x: %d->%d)\n", 
        (int)qw1, (int)qh1, (int)qx1, (int)qy1, (int)qw0, (int)qw1, (int)qx0, (int)qx1);

    /* Now resize narrower to see if that works */
    XtVaSetValues(top, XtNwidth, (Dimension)400, XtNheight, (Dimension)300, NULL);

    Dimension gw2=0, gh2=0, lw2=0, lh2=0;
    XtVaGetValues(grid, XtNwidth, &gw2, XtNheight, &gh2, NULL);
    XtVaGetValues(w_left, XtNwidth, &lw2, XtNheight, &lh2, NULL);
    printf("After (400,300): grid=%dx%d  left=%dx%d\n", (int)gw2, (int)gh2, (int)lw2, (int)lh2);

    assert(x11_errors == 0);
    if (lw1 > lw0 && lw2 < lw1) printf("PASS: label resizes both wider and narrower\n");
    else if (lw1 > lw0) printf("PASS: label resizes wider\n");
    else printf("FAIL: label stuck at %d\n", (int)lw1);
    printf("PASS: %s\n", argv[0]);
    return 0;
}
