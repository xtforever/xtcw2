#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <WcCreate.h>
#include <register_wb.h>
#include <mls.h>
#include <m_tool.h>
#include <xutil.h>

char *fallback_resources[] = {
    "*WclResFiles: main.ad",
    NULL
};

static void destroy_done_cb(Widget w, XtPointer client_data, XtPointer call_data) {
    *(int*)client_data = 1;
}

int main(int argc, char **argv)
{
    XtAppContext app;
    Widget top;

    m_init();
    conststr_init();
    trace_level = 1;
    top = XtOpenApplication(&app, "RetexTest", NULL, 0, &argc, argv, 
                            fallback_resources,
                            applicationShellWidgetClass, NULL, 0);

    XtcwRegister(app);
    WcWidgetCreation(top);

    XtRealizeWidget(top);
    grab_window_quit(top);

    XtAppMainLoop(app);

    int destroyed = 0;
    XtAddCallback(top, XtNdestroyCallback, destroy_done_cb, &destroyed);
    XtDestroyWidget(top);
    while (!destroyed) {
        XtAppProcessEvent(app, XtIMAll);
    }

    void conststr_free(void);
    conststr_free();
    m_destroy();
    return 0;
}
