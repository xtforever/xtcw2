#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/VSlider.h>

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

    Widget w = XtVaCreateManagedWidget("test", vSliderWidgetClass, shell, NULL);
    XtRealizeWidget(shell);

    Dimension width = 0, height = 0;
    XtVaGetValues(w, XtNwidth, &width, XtNheight, &height, NULL);
    assert(width > 0);
    assert(height > 0);
    assert(x11_errors == 0);

    XtDestroyWidget(shell);
    printf("PASS: %s (direct create)\n", argv[0]);
    return 0;
}
