#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/IconSVG.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Gridbox.h>
#include <stdio.h>

int main(int argc, char **argv) {
    XtAppContext app;
    Widget top, grid, icon, label;

    top = XtOpenApplication(&app, "DemoSVG", NULL, 0, &argc, argv, NULL,
                            sessionShellWidgetClass, NULL, 0);

    /* Create a Gridbox to manage the layout */
    grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, top,
                                   NULL);

    /* Add the IconSVG widget to the grid */
    icon = XtVaCreateManagedWidget("icon", iconSVGWidgetClass, grid,
                                   XtNfilename, "../LuaRunner/alert.svg",
                                   XtNwidth, 100,
                                   XtNheight, 100,
                                   "gridx", 0,
                                   "gridy", 0,
                                   NULL);

    /* Add a Wlabel widget next to the icon */
    label = XtVaCreateManagedWidget("label", wlabelWidgetClass, grid,
                                    XtNlabel, "SVG Icon Demo",
                                    "gridx", 1,
                                    "gridy", 0,
                                    "weightx", 1,
                                    NULL);

    XtRealizeWidget(top);
    XtAppMainLoop(app);

    return 0;
}
