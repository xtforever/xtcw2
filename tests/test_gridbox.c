#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Wbutton.h>
#include <xtcw/Gridbox.h>
#include <xtcw/VBox.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"

static Widget make_cell(Widget parent, char *name, char *label,
                        int gx, int gy, int gw, int gh,
                        int wx, int wy, char *fill)
{
    return XtVaCreateManagedWidget(name, wbuttonWidgetClass, parent,
        XtNgridx, gx, XtNgridy, gy,
        XtNgridWidth, gw, XtNgridHeight, gh,
        XtNweightx, wx, XtNweighty, wy,
        XtNfill, fill, XtNlabel, label, NULL);
}

int main(int argc, char **argv) {
    m_init();
    conststr_init();
    extern int trace_level;
    trace_level = 2;

    XtAppContext app;
    Widget top = XtVaOpenApplication(&app, "TestGridbox", NULL, 0, &argc, argv, NULL,
                            sessionShellWidgetClass, NULL, 0);

    XtcwRegister(app);

    Widget vbox = XtVaCreateManagedWidget("vbox", vBoxWidgetClass, top,
                            XtNheight, 200, NULL);

    XtVaCreateManagedWidget("title1", wlabelWidgetClass, vbox,
                            XtNlabel, "=== Test 1: Basic 2x2 Grid ===", NULL);

    Widget grid1 = XtVaCreateManagedWidget("grid1", gridboxWidgetClass, vbox,
                            XtNdefaultDistance, 4, XtNwidth, 300, XtNheight, 100, NULL);

    make_cell(grid1, "btn1", "Cell (0,0)", 0, 0, 1, 1, 0, 0, "both");
    make_cell(grid1, "btn2", "Cell (1,0)", 1, 0, 1, 1, 0, 0, "both");
    make_cell(grid1, "btn3", "Cell (0,1)", 0, 1, 1, 1, 0, 0, "both");
    make_cell(grid1, "btn4", "Cell (1,1)", 1, 1, 1, 1, 0, 0, "both");

    XtVaCreateManagedWidget("title2", wlabelWidgetClass, vbox,
                            XtNlabel, "=== Test 2: Weight Distribution (2:1) ===", NULL);

    Widget grid2 = XtVaCreateManagedWidget("grid2", gridboxWidgetClass, vbox,
                            XtNdefaultDistance, 4, XtNwidth, 400, XtNheight, 60, NULL);

    make_cell(grid2, "w1", "Weight=1", 0, 0, 1, 1, 1, 0, "both");
    make_cell(grid2, "w2", "Weight=2", 1, 0, 1, 1, 2, 0, "both");

    XtVaCreateManagedWidget("title3", wlabelWidgetClass, vbox,
                            XtNlabel, "=== Test 3: Column Spanning ===", NULL);

    Widget grid3 = XtVaCreateManagedWidget("grid3", gridboxWidgetClass, vbox,
                            XtNdefaultDistance, 4, XtNwidth, 300, XtNheight, 100, NULL);

    make_cell(grid3, "span2", "Spans 2 Cols", 0, 0, 2, 1, 0, 0, "both");
    make_cell(grid3, "n1", "Cell (0,1)", 0, 1, 1, 1, 0, 0, "both");
    make_cell(grid3, "n2", "Cell (1,1)", 1, 1, 1, 1, 0, 0, "both");

    XtVaCreateManagedWidget("title4", wlabelWidgetClass, vbox,
                            XtNlabel, "=== Test 4: Fill Modes ===", NULL);

    Widget grid4 = XtVaCreateManagedWidget("grid4", gridboxWidgetClass, vbox,
                            XtNdefaultDistance, 4, XtNwidth, 400, XtNheight, 120, NULL);

    make_cell(grid4, "f1", "Fill: none", 0, 0, 1, 1, 0, 0, "none");
    make_cell(grid4, "f2", "Fill: width", 1, 0, 1, 1, 1, 0, "width");
    make_cell(grid4, "f3", "Fill: height", 0, 1, 1, 1, 0, 1, "height");
    make_cell(grid4, "f4", "Fill: both", 1, 1, 1, 1, 1, 1, "both");

    XtRealizeWidget(top);

    printf("Gridbox Test - Resize window to see weight distribution\n");
    printf("Tests: 2x2 grid, weight ratio, column spanning, fill modes\n");

    XtAppMainLoop(app);

    conststr_free();
    m_destruct();
    return 0;
}
