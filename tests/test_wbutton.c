#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wbutton.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"

int main(int argc, char **argv) {
    m_init();
    conststr_init();
    extern int trace_level;
    trace_level = 2;

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWbutton", NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL, 0);

    XtcwRegister(app);

    Widget button = XtVaCreateManagedWidget("button", wbuttonWidgetClass, top,
                            XtNlabel, "Hello $\\frac{a}{b}$ World",
                            NULL);

    XtRealizeWidget(top);
    redraw_label(button);

    // Simulate EnterWindow (highlight)
    XtVaSetValues(button, "state", 1, NULL); // STATE_SELECTED = 1
    redraw_label(button);

    // After realize we can see if it draws correctly. 
    // We can't really "check" it programmatically easily without a headless X server or similar,
    // but we can at least ensure it doesn't crash and maybe check some outputs.
    // For now, let's just make it possible to run it.

    // I will use a short timeout and quit for automated check if it doesn't crash
    // but the user wants me to check if it correctly displays labels.
    
    XtAppMainLoop(app);

    conststr_free();
    m_destruct();
    return 0;
}
