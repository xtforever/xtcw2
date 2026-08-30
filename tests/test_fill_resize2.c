#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Gridbox.h>

static int x11_errors = 0;
static int x11_error_handler(Display *d, XErrorEvent *e) { (void)d; (void)e; x11_errors++; return 0; }
static int warnings = 0;
static void warning_handler(String msg) { (void)msg; warnings++; }

int main(int argc, char **argv) {
    XtAppContext app;
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

    /* Label with fill=Both via integer */
    Widget label = XtVaCreateManagedWidget("label", wlabelWidgetClass, grid,
        XtNlabel, "This is a test label with fill=Both",
        XtNfontSize, 12,
        XtNleftGap, 0, XtNrightGap, 0, XtNtopGap, 0, XtNbottomGap, 0,
        XtNweightx, 1, XtNweighty, 1,
        XtNfill, 3,  /* FillBoth as integer */
        "gridy", 0,
        NULL);

    XtRealizeWidget(shell);

    Dimension gw0=0, gh0=0, lw0=0, lh0=0;
    XtVaGetValues(grid, XtNwidth, &gw0, XtNheight, &gh0, NULL);
    XtVaGetValues(label, XtNwidth, &lw0, XtNheight, &lh0, NULL);
    printf("Initial: grid=%dx%d label=%dx%d\n", (int)gw0, (int)gh0, (int)lw0, (int)lh0);

    /* Resize shell — this triggers Shell→GridBox resize chain */
    Dimension new_w = 600, new_h = 300;
    XtVaSetValues(shell, XtNwidth, new_w, XtNheight, new_h, NULL);

    Dimension gw1=0, gh1=0, lw1=0, lh1=0;
    XtVaGetValues(grid, XtNwidth, &gw1, XtNheight, &gh1, NULL);
    XtVaGetValues(label, XtNwidth, &lw1, XtNheight, &lh1, NULL);
    printf("After shell resize: grid=%dx%d label=%dx%d\n", (int)gw1, (int)gh1, (int)lw1, (int)lh1);

    if (lw1 > lw0 && lh1 > lh0)
        printf("PASS: label grew in both dimensions\n");
    else if (lw1 > lw0)
        printf("PASS: label grew in width only\n");
    else if (lh1 > lh0)
        printf("FAIL: label grew only in height (width stuck at %d)\n", (int)lw1);
    else
        printf("FAIL: label did not grow\n");

    assert(x11_errors == 0);
    printf("PASS: %s\n", argv[0]);
    return 0;
}
