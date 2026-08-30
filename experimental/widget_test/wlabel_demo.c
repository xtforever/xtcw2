#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Wbutton.h>
#include <xtcw/Gridbox.h>
#include "mls.h"
#include "conststr.h"
#include <stdio.h>
#include <stdlib.h>

static void quit_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    exit(0);
}

int main(int argc, char **argv)
{
    m_init();
    conststr_init();
    trace_level = 0;

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "WlabelDemo", NULL, 0,
        &argc, argv, NULL,
        sessionShellWidgetClass, NULL, 0);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, top, NULL);

    XtVaCreateManagedWidget("title", wlabelWidgetClass, grid,
        XtNlabel, "Wlabel Widget Test — 10px Font",
        XtNfontSize, 10,
        XtNalignment, 1,
        XtNautoWidth, True,
        XtNleftGap, 10, XtNrightGap, 10, XtNtopGap, 6, XtNbottomGap, 6,
        XtNweightx, 1, XtNweighty, 0,
        XtNfill, 1,  /* FillWidth */
        "gridy", 0,
        NULL);

    Widget w_left = XtVaCreateManagedWidget("left_aligned", wlabelWidgetClass, grid,
        XtNlabel, "Left-aligned wrapping text. This label demonstrates how Wlabel wraps long "
                  "lines to fit the container width. Resize the window to see reflow in action. "
                  "The re-tex engine handles word breaks and paragraph layout automatically.",
        XtNfontSize, 10,
        XtNalignment, 0,
        XtNautoHeight, True,
        XtNleftGap, 10, XtNrightGap, 10, XtNtopGap, 4, XtNbottomGap, 4,
        XtNweightx, 1, XtNweighty, 1,
        XtNfill, 3, "gridy", 1,
        NULL);
    XtVaSetValues(w_left, XtVaTypedArg, XtNbg_norm, XtRString, "#ecf0f1", 8, NULL);

    Widget w_center = XtVaCreateManagedWidget("centered", wlabelWidgetClass, grid,
        XtNlabel, "Center-aligned",
        XtNfontSize, 10,
        XtNalignment, 1,
        XtNautoHeight, True,
        XtNautoWidth, True,
        XtNleftGap, 10, XtNrightGap, 10, XtNtopGap, 4, XtNbottomGap, 4,
        XtNweightx, 1, XtNweighty, 1,
        XtNfill, 3, "gridy", 2,
        NULL);
    XtVaSetValues(w_center, XtVaTypedArg, XtNbg_norm, XtRString, "#bdc3c7", 8, NULL);

    Widget w_right = XtVaCreateManagedWidget("right_aligned", wlabelWidgetClass, grid,
        XtNlabel, "Right-aligned text with asymmetric gaps: left=40, right=6. "
                  "The text hugs the right side of its allocated space.",
        XtNfontSize, 10,
        XtNalignment, 2,
        XtNautoHeight, True,
        XtNleftGap, 40, XtNrightGap, 6, XtNtopGap, 4, XtNbottomGap, 4,
        XtNweightx, 1, XtNweighty, 1,
        XtNfill, 3, "gridy", 3,
        NULL);
    XtVaSetValues(w_right, XtVaTypedArg, XtNbg_norm, XtRString, "#95a5a6", 8, NULL);

    Widget w_just = XtVaCreateManagedWidget("justified", wlabelWidgetClass, grid,
        XtNlabel, "Justified text. The re-tex engine distributes space between words so "
                  "both left and right edges are flush. This mode works best with "
                  "sufficient line width for even word spacing.",
        XtNfontSize, 10,
        XtNalignment, 3,
        XtNautoHeight, True,
        XtNleftGap, 10, XtNrightGap, 10, XtNtopGap, 4, XtNbottomGap, 4,
        XtNweightx, 1, XtNweighty, 1,
        XtNfill, 3, "gridy", 4,
        NULL);
    XtVaSetValues(w_just, XtVaTypedArg, XtNbg_norm, XtRString, "#7f8c8d", 8, NULL);

    Widget w_multi = XtVaCreateManagedWidget("multi_para", wlabelWidgetClass, grid,
        XtNlabel, "Multi-paragraph text.\n\n"
                  "This is a second paragraph, separated by a double newline. "
                  "Each paragraph is laid out independently by the re-tex engine. "
                  "Paragraphs are stacked vertically with appropriate spacing.\n\n"
                  "Third paragraph — also independently wrapped and positioned.",
        XtNfontSize, 10,
        XtNalignment, 0,
        XtNautoHeight, True,
        XtNleftGap, 10, XtNrightGap, 10, XtNtopGap, 4, XtNbottomGap, 4,
        XtNweightx, 1, XtNweighty, 1,
        XtNfill, 3, "gridy", 5,
        NULL);
    XtVaSetValues(w_multi, XtVaTypedArg, XtNbg_norm, XtRString, "#d5dbdb", 8, NULL);

    Widget w_rev = XtVaCreateManagedWidget("reverse", wlabelWidgetClass, grid,
        XtNlabel, "Reverse mode: fg and bg swapped per node. "
                  "In this mode each glyph's foreground and background colors are inverted, "
                  "creating a highlight effect.",
        XtNfontSize, 10,
        XtNalignment, 0,
        XtNautoHeight, True,
        XtNautoWidth, True,
        XtNreverse_mode, True,
        XtNleftGap, 10, XtNrightGap, 10, XtNtopGap, 4, XtNbottomGap, 4,
        XtNweightx, 1, XtNweighty, 1,
        XtNfill, 3, "gridy", 6,
        NULL);
    XtVaSetValues(w_rev, XtVaTypedArg, XtNbg_norm, XtRString, "#f39c12", 8,
                  XtVaTypedArg, XtNfg_norm, XtRString, "#2c3e50", 8,
                  NULL);

    Widget quit = XtVaCreateManagedWidget("quit", wbuttonWidgetClass, grid,
        XtNlabel, "Quit",
        XtNweightx, 1, XtNweighty, 0,
        XtNfill, 1, "gridy", 7,
        NULL);
    XtAddCallback(quit, XtNcallback, quit_cb, NULL);

    XtRealizeWidget(top);
    XtAppMainLoop(app);

    return 0;
}
