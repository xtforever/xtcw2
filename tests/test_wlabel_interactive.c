#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wlabel.h>
#include <xtcw/VBox.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>

int main(int argc, char **argv) {
    m_init();
    conststr_init();
    
    // Set trace level to see LAYOUT logs during resizing
    extern int trace_level;
    trace_level = 2;

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "WlabelInteractive", NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL, 0);

    XtcwRegister(app);

    Widget vbox = XtVaCreateManagedWidget("vbox", vBoxWidgetClass, top, NULL);

    const char *text = 
        "This interactive test allows you to resize the window.\n"
        "Observe how the text reformats based on the width.\n\n"
        "Styles: \\bf{Bold}, \\it{Italic}, \\huge{Huge}.\n"
        "Math: $\\sum_{i=0}^n i = \\frac{n(n+1)}{2}$";

    // 1. Justified Alignment (default)
    XtVaCreateManagedWidget("label1", wlabelWidgetClass, vbox,
                            XtNlabel, text,
                            "alignment", 3, // Justify
                            "leftGap", 10, "rightGap", 10, "topGap", 10, "bottomGap", 10,
                            NULL);

    // 2. Centered Alignment
    XtVaCreateManagedWidget("label2", wlabelWidgetClass, vbox,
                            XtNlabel, "--- Centered Alignment ---",
                            "alignment", 1, // Center
                            "fontSize", 16,
                            "topGap", 20,
                            NULL);

    // 3. Selection and Colors
    XtVaCreateManagedWidget("label3", wlabelWidgetClass, vbox,
                            XtNlabel, "Try selecting this text with the mouse.\nIt has a custom blue background.",
                            "bg_norm", "lightblue",
                            "leftGap", 10, "rightGap", 10,
                            NULL);

    XtRealizeWidget(top);
    XtAppMainLoop(app);

    conststr_free();
    m_destruct();
    return 0;
}
