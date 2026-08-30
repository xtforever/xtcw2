#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
        XtNwidth, 200, XtNheight, 200,
        NULL);

    Widget child[4];
    for (int i = 0; i < 4; i++) {
        char name[32]; snprintf(name, 32, "child%d", i);
        child[i] = XtVaCreateManagedWidget(name, wlabelWidgetClass, grid,
            XtNlabel, name,
            XtNgridx, i % 2,
            XtNgridy, i / 2,
            XtNweightx, 1, XtNweighty, 1,
            XtNfill, 3,
            NULL);
    }
    XtRealizeWidget(shell);

    for (int i = 0; i < 4; i++) {
        Dimension w = 0, h = 0;
        XtVaGetValues(child[i], XtNwidth, &w, XtNheight, &h, NULL);
        assert(w > 0 && h > 0);
    }

    Position x[4], y[4];
    for (int i = 0; i < 4; i++) {
        XtVaGetValues(child[i], XtNx, &x[i], XtNy, &y[i], NULL);
    }
    assert(x[0] != x[1]);
    assert(y[0] != y[2]);

    assert(x11_errors == 0);
    printf("PASS: %s\n", argv[0]);
    return 0;
}
