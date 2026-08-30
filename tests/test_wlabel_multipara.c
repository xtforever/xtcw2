#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
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

    Widget w = XtVaCreateManagedWidget("test", wlabelWidgetClass, shell,
        XtNlabel, "Paragraph 1 line 1\n\nParagraph 2 line 1\n\nParagraph 3 line 1",
        XtNwidth, 400, XtNheight, 200,
        XtNfontSize, 22,
        NULL);
    XtRealizeWidget(shell);

    Dimension w0 = 0, h0 = 0;
    XtVaGetValues(w, XtNwidth, &w0, XtNheight, &h0, NULL);
    assert(w0 > 0);
    assert(h0 > 0);
    assert(x11_errors == 0);
    printf("Test 1 PASS: initial size %dx%d\n", (int)w0, (int)h0);

    Dimension w1 = w0 * 7 / 10;
    XtVaSetValues(w, XtNwidth, w1, NULL);
    Dimension w1_out = 0, h1_out = 0;
    XtVaGetValues(w, XtNwidth, &w1_out, XtNheight, &h1_out, NULL);
    assert(w1_out > 0);
    assert(h1_out > 0);
    assert(x11_errors == 0);
    printf("Test 2 PASS: resize to %dx%d (was %dx%d)\n", (int)w1_out, (int)h1_out, (int)w0, (int)h0);

    XtVaSetValues(w, XtNstate, 1, NULL);
    XtVaSetValues(w, XtNwidth, w0, XtNheight, h0, NULL);
    Dimension w2 = 0, h2 = 0;
    XtVaGetValues(w, XtNwidth, &w2, XtNheight, &h2, NULL);
    assert(w2 > 0);
    assert(h2 > 0);
    assert(x11_errors == 0);
    printf("Test 5 PASS: state transition + resize, size %dx%d\n", (int)w2, (int)h2);

    XtDestroyWidget(shell);
    printf("PASS: %s\n", argv[0]);
    return 0;
}
