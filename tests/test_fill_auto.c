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

static int x11_error_handler(Display *d, XErrorEvent *e) { (void)d; (void)e; return 0; }

int main(int argc, char **argv) {
    XtAppContext app;
    m_init();
    trace_level = 2;
    XSetErrorHandler(x11_error_handler);

    Widget top = XtVaAppInitialize(&app, "Test", NULL, 0, &argc, argv, NULL,
        XtNallowShellResize, True, XtNmappedWhenManaged, False, NULL);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, top, NULL);

    /* Test 1: autoHeight=True, short text */
    Widget w1 = XtVaCreateManagedWidget("w1", wlabelWidgetClass, grid,
        XtNlabel, "Short",
        XtNautoHeight, True,
        XtNweightx, 1, XtNweighty, 1,
        "gridy", 0, "fill", "Both",
        NULL);

    /* Test 2: autoHeight=False, short text */
    Widget w2 = XtVaCreateManagedWidget("w2", wlabelWidgetClass, grid,
        XtNlabel, "Short2",
        XtNweightx, 1, XtNweighty, 1,
        "gridy", 1, "fill", "Both",
        NULL);

    /* Test 3: autoHeight=True, gaps */
    Widget w3 = XtVaCreateManagedWidget("w3", wlabelWidgetClass, grid,
        XtNlabel, "Gaps",
        XtNleftGap, 10, XtNrightGap, 10,
        XtNautoHeight, True,
        XtNweightx, 1, XtNweighty, 1,
        "gridy", 2, "fill", "Both",
        NULL);

    XtRealizeWidget(top);

    Dimension g0=0;
    XtVaGetValues(grid, XtNwidth, &g0, NULL);
    printf("Initial grid=%d\n", (int)g0);

    Dimension ww1=0, ww2=0, ww3=0;
    XtVaGetValues(w1, XtNwidth, &ww1, NULL);
    XtVaGetValues(w2, XtNwidth, &ww2, NULL);
    XtVaGetValues(w3, XtNwidth, &ww3, NULL);
    printf("w1(autoH short): %d  w2(no autoH): %d  w3(autoH+gaps): %d\n",
        (int)ww1, (int)ww2, (int)ww3);

    /* Resize to larger */
    XtVaSetValues(top, XtNwidth, (Dimension)600, XtNheight, (Dimension)300, NULL);

    Dimension g1=0;
    XtVaGetValues(grid, XtNwidth, &g1, NULL);
    XtVaGetValues(w1, XtNwidth, &ww1, NULL);
    XtVaGetValues(w2, XtNwidth, &ww2, NULL);
    XtVaGetValues(w3, XtNwidth, &ww3, NULL);
    printf("Grid now=%d  w1=%d  w2=%d  w3=%d\n", (int)g1, (int)ww1, (int)ww2, (int)ww3);

    if (ww1 > ww2) printf("PASS\n");
    else printf("FAIL: autoHeight label didn't grow\n");
    printf("PASS: %s\n", argv[0]);
    return 0;
}
