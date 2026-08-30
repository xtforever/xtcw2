#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Wbutton.h>
#include <xtcw/Gridbox.h>

static int x11_error_handler(Display *d, XErrorEvent *e) { (void)d; (void)e; return 0; }

#define TEST(name, ...) do { \
    XtAppContext app; \
    Widget top = XtVaAppInitialize(&app, "Test", NULL, 0, NULL, NULL, \
        XtNallowShellResize, True, XtNmappedWhenManaged, False, NULL); \
    Widget grid = XtVaCreateManagedWidget("g", gridboxWidgetClass, top, NULL); \
    Widget w = XtVaCreateManagedWidget("w", wlabelWidgetClass, grid, __VA_ARGS__); \
    XtRealizeWidget(top); \
    Dimension gw=0, ww=0; Position px=0; \
    XtVaGetValues(grid, XtNwidth, &gw, NULL); \
    XtVaGetValues(w, XtNwidth, &ww, XtNx, &px, NULL); \
    printf("%-50s: grid=%d w=%d x=%d fill=%s\n", name, (int)gw, (int)ww, (int)px, ww+8>gw?"Y":"N"); \
} while(0)

int main(int argc, char **argv) {
    XSetErrorHandler(x11_error_handler);

    TEST("A: basic label, fill=Both",
        XtNlabel, "Short", XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, "fill", "Both", NULL);

    TEST("B: autoHeight label, fill=Both",
        XtNlabel, "Short", XtNautoHeight, True,
        XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, "fill", "Both", NULL);

    TEST("C: weighty=0 label, fill=Width",
        XtNlabel, "Title", XtNweightx, 1, XtNweighty, 0,
        "gridy", 0, "fill", "Width", NULL);

    TEST("D: no fill setting",
        XtNlabel, "NoFill", XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, NULL);

    TEST("E: fill=Width",
        XtNlabel, "Width", XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, "fill", "Width", NULL);

    printf("Done\n");
    return 0;
}
