#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <unistd.h>
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
    Widget top = XtOpenApplication(&app, "TestWlabelMultiPara", NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL, 0);

    XtcwRegister(app);

    // Paragraph 1: "Para1" (5 chars)
    // Paragraph 2: "Para2" (5 chars)
    Widget label = XtVaCreateManagedWidget("label", wlabelWidgetClass, top,
                                           XtNlabel, "Para1\n\nPara2",
                                           "leftGap", 10,
                                           "rightGap", 10,
                                           "topGap", 20,
                                           "bottomGap", 5,
                                           XtNselection_start, 5,
                                           XtNselection_end, 9,
                                           NULL);

    XtRealizeWidget(top);

    // Process events non-blockingly
    XEvent event;
    int count = 0;
    while (count < 100) {
        if (XtAppPending(app)) {
            XtAppNextEvent(app, &event);
            XtDispatchEvent(&event);
        } else {
            // No events, just sleep a tiny bit to allow server to catch up
            usleep(1000);
        }
        count++;
    }

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    conststr_free();
    m_destruct();
    return 0;
}
