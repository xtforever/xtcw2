#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wlabel.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>

int main(int argc, char **argv) {
    m_init();
    conststr_init();
    extern int trace_level;
    trace_level = 2;

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWlabelSelection", NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL, 0);

    XtcwRegister(app);

    Widget label = XtVaCreateManagedWidget("label", wlabelWidgetClass, top,
                                           XtNlabel, "ABC",
                                           NULL);

    XtRealizeWidget(top);

    // Change selection after realize to trigger set_values -> dirty
    XtVaSetValues(label, XtNselection_start, 1, XtNselection_end, 1, NULL);

    // Process events
    XEvent event;
    int count = 0;
    while (count < 50) {
        if (XtAppPending(app)) {
            XtAppNextEvent(app, &event);
            XtDispatchEvent(&event);
        }
        count++;
    }

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    conststr_free();
    m_destruct();
    return 0;
}
