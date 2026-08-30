#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wlabel.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"

static void timeout_cb(XtPointer client_data, XtIntervalId *id) {
    XtAppContext app = (XtAppContext)client_data;
    XtAppSetExitFlag(app);
}

int main(int argc, char **argv) {
    m_init();
    conststr_init();
    extern int trace_level;
    trace_level = 2;

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWlabel", NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL, 0);

    XtcwRegister(app);

    XtVaCreateManagedWidget("label", wlabelWidgetClass, top,
                            XtNlabel, "Hello $\\frac{a}{b}$ World",
                            NULL);

    XtRealizeWidget(top);

    // Exit after 500ms to capture LAYOUT trace
    XtAppAddTimeOut(app, 500, timeout_cb, app);

    XtAppMainLoop(app);
    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    conststr_free();
    m_destruct();
    return 0;
}
