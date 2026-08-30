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

    Widget shell = XtVaAppInitialize(&app, "Test", NULL, 0,
        &argc, argv, NULL,
        XtNallowShellResize, True,
        XtNmappedWhenManaged, False,
        NULL);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, shell, NULL);

    Widget wlab = XtVaCreateManagedWidget("wlab", wlabelWidgetClass, grid,
        XtNlabel, "This is a Wlabel with fill=Both",
        XtNfontSize, 12,
        XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, "fill", "Both",
        NULL);

    XtRealizeWidget(shell);

    Dimension gw0=0, gh0=0, lw0=0, lh0=0;
    XtVaGetValues(grid, XtNwidth, &gw0, XtNheight, &gh0, NULL);
    XtVaGetValues(wlab, XtNwidth, &lw0, XtNheight, &lh0, NULL);
    printf("Initial: grid=%dx%d wlabel=%dx%d\n", (int)gw0, (int)gh0, (int)lw0, (int)lh0);

    printf("=== About to call XtVaSetValues(shell, XtNwidth, 800) ===\n");
    XtVaSetValues(shell, XtNwidth, (Dimension)800, XtNheight, (Dimension)400, NULL);
    printf("=== Returned from XtVaSetValues ===\n");

    Dimension gw1=0, gh1=0, lw1=0, lh1=0;
    XtVaGetValues(grid, XtNwidth, &gw1, XtNheight, &gh1, NULL);
    XtVaGetValues(wlab, XtNwidth, &lw1, XtNheight, &lh1, NULL);
    printf("After:   grid=%dx%d wlabel=%dx%d\n", (int)gw1, (int)gh1, (int)lw1, (int)lh1);

    assert(x11_errors == 0);
    printf("PASS: %s\n", argv[0]);
    return 0;
}
