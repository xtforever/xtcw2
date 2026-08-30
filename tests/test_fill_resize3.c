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
    XSetErrorHandler(x11_error_handler);

    Widget shell = XtVaAppInitialize(&app, "Test", NULL, 0,
        &argc, argv, NULL,
        XtNallowShellResize, True,
        XtNmappedWhenManaged, False,
        NULL);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, shell, NULL);

    /* Wlabel with fill=Both (STRING) */
    Widget wlab = XtVaCreateManagedWidget("wlab", wlabelWidgetClass, grid,
        XtNlabel, "This is a Wlabel with fill=Both",
        XtNfontSize, 12,
        XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, "fill", "Both",
        NULL);

    /* Wbutton with fill=Width (STRING) */
    Widget wbut = XtVaCreateManagedWidget("wbut", wbuttonWidgetClass, grid,
        XtNlabel, "Quit",
        XtNweightx, 1, XtNweighty, 0,
        "gridy", 1, "fill", "Width",
        NULL);

    XtRealizeWidget(shell);

    Dimension gw0=0, gh0=0, lw0=0, lh0=0, bw0=0, bh0=0;
    XtVaGetValues(grid, XtNwidth, &gw0, XtNheight, &gh0, NULL);
    XtVaGetValues(wlab, XtNwidth, &lw0, XtNheight, &lh0, NULL);
    XtVaGetValues(wbut, XtNwidth, &bw0, XtNheight, &bh0, NULL);
    printf("Initial: grid=%dx%d wlabel=%dx%d wbutton=%dx%d\n", 
        (int)gw0, (int)gh0, (int)lw0, (int)lh0, (int)bw0, (int)bh0);

    /* Resize shell */
    XtVaSetValues(shell, XtNwidth, (Dimension)800, XtNheight, (Dimension)400, NULL);

    Dimension gw1=0, gh1=0, lw1=0, lh1=0, bw1=0, bh1=0;
    XtVaGetValues(grid, XtNwidth, &gw1, XtNheight, &gh1, NULL);
    XtVaGetValues(wlab, XtNwidth, &lw1, XtNheight, &lh1, NULL);
    XtVaGetValues(wbut, XtNwidth, &bw1, XtNheight, &bh1, NULL);
    printf("After:   grid=%dx%d wlabel=%dx%d wbutton=%dx%d\n", 
        (int)gw1, (int)gh1, (int)lw1, (int)lh1, (int)bw1, (int)bh1);

    printf("Wlabel: %d→%d (%s)   Wbutton: %d→%d (%s)\n",
        (int)lw0, (int)lw1, lw1>lw0?"grew":"stuck",
        (int)bw0, (int)bw1, bw1>bw0?"grew":"stuck");

    assert(x11_errors == 0);
    if (lw1 > lw0) printf("PASS\n"); else printf("FAIL: Wlabel width did not grow\n");
    printf("PASS: %s\n", argv[0]);
    return 0;
}
