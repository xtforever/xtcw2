#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Wbutton.h>
#include <xtcw/Gridbox.h>

static int x11_error_handler(Display *d, XErrorEvent *e) { (void)d; (void)e; return 0; }

static void check(const char *label, Widget grid, Widget child) {
    Dimension gw=0, ww=0;
    Position px=0;
    XtVaGetValues(grid, XtNwidth, &gw, NULL);
    XtVaGetValues(child, XtNwidth, &ww, XtNx, &px, NULL);
    int fill_width = (int)gw - 2*4;
    printf("%s: grid=%d w=%d x=%d (fill=%s %s)\n", label,
        (int)gw, (int)ww, (int)px,
        (int)ww >= fill_width-3 ? "YES" : "NO ",
        (int)px <= 5 ? "edge" : "center");
}

int main(int argc, char **argv) {
    XtAppContext app;
    XSetErrorHandler(x11_error_handler);
    Widget top = XtVaAppInitialize(&app, "Test", NULL, 0, &argc, argv, NULL,
        XtNallowShellResize, True, XtNmappedWhenManaged, False, NULL);

    /* Test 1: weighty=0 at top (title), weighty=1 below */
    {
        Widget g = XtVaCreateManagedWidget("g1", gridboxWidgetClass, top, NULL);
        Widget title = XtVaCreateManagedWidget("title", wlabelWidgetClass, g,
            XtNlabel, "Title", XtNweightx, 1, XtNweighty, 0,
            "gridy", 0, "fill", "Width", NULL);
        Widget body = XtVaCreateManagedWidget("body", wlabelWidgetClass, g,
            XtNlabel, "Body text here", XtNweightx, 1, XtNweighty, 1,
            "gridy", 1, "fill", "Both", NULL);
        XtRealizeWidget(top);
        check("T1: weighty=0 FIRST, then weighty=1", g, body);
    }

    /* Test 2: weighty=1 at top, weighty=0 below */
    {
        Widget g = XtVaCreateManagedWidget("g2", gridboxWidgetClass, top, NULL);
        Widget body = XtVaCreateManagedWidget("body", wlabelWidgetClass, g,
            XtNlabel, "Body text here", XtNweightx, 1, XtNweighty, 1,
            "gridy", 0, "fill", "Both", NULL);
        Widget footer = XtVaCreateManagedWidget("footer", wbuttonWidgetClass, g,
            XtNlabel, "Q", XtNweightx, 1, XtNweighty, 0,
            "gridy", 1, "fill", "Width", NULL);
        check("T2: weighty=1 FIRST, then weighty=0", g, body);
    }

    /* Test 3: weighty=0 first and last, weighty=1 middle (like demo) */
    {
        Widget g = XtVaCreateManagedWidget("g3", gridboxWidgetClass, top, NULL);
        Widget title = XtVaCreateManagedWidget("title", wlabelWidgetClass, g,
            XtNlabel, "Title", XtNweightx, 1, XtNweighty, 0,
            "gridy", 0, "fill", "Width", NULL);
        Widget body = XtVaCreateManagedWidget("body", wlabelWidgetClass, g,
            XtNlabel, "Body", XtNweightx, 1, XtNweighty, 1,
            "gridy", 1, "fill", "Both", NULL);
        Widget footer = XtVaCreateManagedWidget("footer", wbuttonWidgetClass, g,
            XtNlabel, "Q", XtNweightx, 1, XtNweighty, 0,
            "gridy", 2, "fill", "Width", NULL);
        check("T3: weighty=0 FIRST+LAST, weighty=1 middle", g, body);
    }

    printf("Done\n");
    return 0;
}
