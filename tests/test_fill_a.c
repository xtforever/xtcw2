#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Wbutton.h>
#include <xtcw/Gridbox.h>

static int xerrors = 0;
static int x11_err(Display *d, XErrorEvent *e) { (void)d;(void)e; xerrors++; return 0; }

int main(int argc, char **argv) {
    XtAppContext app;
    XSetErrorHandler(x11_err);
    Widget top = XtVaAppInitialize(&app, "T", NULL, 0, &argc, argv, NULL,
        XtNallowShellResize, 1, XtNmappedWhenManaged, 0, NULL);

    Widget grid = XtVaCreateManagedWidget("g", gridboxWidgetClass, top, NULL);
    Widget wlab = XtVaCreateManagedWidget("wl", wlabelWidgetClass, grid,
        XtNlabel, "Label", XtNweightx,1,XtNweighty,1,
        "gridy",0,"fill","Both",NULL);
    Widget wbut = XtVaCreateManagedWidget("wb", wbuttonWidgetClass, grid,
        XtNlabel,"B",XtNweightx,1,XtNweighty,0,
        "gridy",1,"fill","Width",NULL);
    XtRealizeWidget(top);

    Dimension gw=0, lw=0, bw=0;
    XtVaGetValues(grid, XtNwidth, &gw, NULL);
    XtVaGetValues(wlab, XtNwidth, &lw, NULL);
    XtVaGetValues(wbut, XtNwidth, &bw, NULL);
    printf("grid=%d label=%d button=%d\n", (int)gw, (int)lw, (int)bw);
    printf("label%s fill\n", (int)lw+8>(int)gw ? "" : " DOES NOT");
    return xerrors;
}
