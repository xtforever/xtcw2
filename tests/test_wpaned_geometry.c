#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/WPaned.h>
#include <xtcw/Wlabel.h>

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

    Widget paned = XtVaCreateManagedWidget("paned", wPanedWidgetClass, shell,
        XtNwidth, 200, XtNheight, 200,
        XtNorientation, 1,
        NULL);

    Widget child = XtVaCreateManagedWidget("pane1", wlabelWidgetClass, paned,
        XtNlabel, "Pane 1",
        XtNallowResize, True,
        XtNpreferredPaneSize, 50,
        NULL);
    (void)child;

    XtRealizeWidget(shell);

    Dimension paned_w = 0, paned_h = 0;
    XtVaGetValues(paned, XtNwidth, &paned_w, XtNheight, &paned_h, NULL);
    assert(paned_w > 0);
    assert(paned_h > 0);
    assert(x11_errors == 0);

    XtDestroyWidget(shell);
    printf("PASS: %s (child layout)\n", argv[0]);
    return 0;
}
