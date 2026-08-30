#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Wbutton.h>
#include <xtcw/Gridbox.h>

static int x11_error_handler(Display *d, XErrorEvent *e) { (void)d; (void)e; return 0; }

static void check(const char *label, Widget grid, Widget w1) {
    Dimension gw=0, ww=0;
    Position px=0;
    XtVaGetValues(grid, XtNwidth, &gw, NULL);
    XtVaGetValues(w1, XtNwidth, &ww, XtNx, &px, NULL);
    int fill_width = (int)gw - 2*4;
    printf("%s: grid=%d w=%d x=%d (fill=%s rest=%s)\n", label,
        (int)gw, (int)ww, (int)px,
        (int)ww >= fill_width-3 ? "YES" : "NO ",
        (int)px <= 5 ? "at-edge" : "centered");
}

int main(int argc, char **argv) {
    XtAppContext app;
    XSetErrorHandler(x11_error_handler);
    Widget top = XtVaAppInitialize(&app, "Test", NULL, 0, &argc, argv, NULL,
        XtNallowShellResize, True, XtNmappedWhenManaged, False, NULL);

    /* --- Case 1: auto grid + weighty=0 child (like test_fill_resize5) --- */
    Widget g1 = XtVaCreateManagedWidget("g1", gridboxWidgetClass, top, NULL);
    Widget a1 = XtVaCreateManagedWidget("a1", wlabelWidgetClass, g1,
        XtNlabel, "Test", XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, "fill", "Both", NULL);
    Widget b1 = XtVaCreateManagedWidget("b1", wbuttonWidgetClass, g1,
        XtNlabel, "Q", XtNweightx, 1, XtNweighty, 0,
        "gridy", 1, "fill", "Width", NULL);
    XtRealizeWidget(top);
    check("A: auto grid + weighty=0 child", g1, a1);

    /* --- Case 2: auto grid + all weighty=1 --- */
    Widget g2 = XtVaCreateManagedWidget("g2", gridboxWidgetClass, top, NULL);
    Widget a2 = XtVaCreateManagedWidget("a2", wlabelWidgetClass, g2,
        XtNlabel, "Test", XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, "fill", "Both", NULL);
    Widget b2 = XtVaCreateManagedWidget("b2", wlabelWidgetClass, g2,
        XtNlabel, "Q", XtNweightx, 1, XtNweighty, 1,
        "gridy", 1, "fill", "Both", NULL);
    check("B: auto grid + all weighty=1", g2, a2);

    /* --- Case 3: explicit grid + all weighty=1 --- */
    Widget g3 = XtVaCreateManagedWidget("g3", gridboxWidgetClass, top,
        XtNwidth, 300, XtNheight, 200, NULL);
    Widget a3 = XtVaCreateManagedWidget("a3", wlabelWidgetClass, g3,
        XtNlabel, "Test", XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, "fill", "Both", NULL);
    Widget b3 = XtVaCreateManagedWidget("b3", wlabelWidgetClass, g3,
        XtNlabel, "Q", XtNweightx, 1, XtNweighty, 1,
        "gridy", 1, "fill", "Both", NULL);
    check("C: explicit grid + all weighty=1", g3, a3);

    printf("Done\n");
    return 0;
}
