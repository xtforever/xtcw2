#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Gridbox.h>

static int x11_error_handler(Display *d, XErrorEvent *e) { (void)d; (void)e; return 0; }

int main(int argc, char **argv) {
    XtAppContext app;
    XSetErrorHandler(x11_error_handler);

    Widget top = XtVaAppInitialize(&app, "Test", NULL, 0, &argc, argv, NULL,
        XtNallowShellResize, True, XtNmappedWhenManaged, False, NULL);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, top,
        XtNwidth, 300, XtNheight, 200, NULL);

    Widget w1 = XtVaCreateManagedWidget("w1", wlabelWidgetClass, grid,
        XtNlabel, "Short",
        XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, "fill", "Both",
        NULL);

    XtRealizeWidget(top);

    Dimension gw=0, ww=0;
    Position px=0;
    XtVaGetValues(grid, XtNwidth, &gw, NULL);
    XtVaGetValues(w1, XtNwidth, &ww, XtNx, &px, NULL);
    printf("Grid=%d label w=%d x=%d (expected fill: ~%d)\n",
        (int)gw, (int)ww, (int)px, (int)gw-8);
    printf("%s\n", ww >= (unsigned)gw-10 ? "FILL OK" : "FILL FAIL");
    return 0;
}
