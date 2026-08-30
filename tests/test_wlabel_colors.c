#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wlabel.h>
#include <xtcw/VBox.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>

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
    Widget top = XtOpenApplication(&app, "TestWlabelColors", NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL, 0);

    XtcwRegister(app);

    Widget vbox = XtVaCreateManagedWidget("vbox", vBoxWidgetClass, top, NULL);

    // 1. Normal state
    XtVaCreateManagedWidget("normal", wlabelWidgetClass, vbox,
                            XtNlabel, "Normal Text",
                            XtNforeground, "black",
                            XtNbackground, "white",
                            NULL);

    // 2. Selected state
    // We set selection_start=0, selection_end=7 ("Selecte")
    XtVaCreateManagedWidget("selected", wlabelWidgetClass, vbox,
                            XtNlabel, "Selected Text",
                            XtNforeground, "black",
                            XtNbackground, "white",
                            "fg_sel", "white",
                            "bg_sel", "blue",
                            "selection_start", 0,
                            "selection_end", 7,
                            NULL);

    // 3. Reverse mode
    XtVaCreateManagedWidget("reverse", wlabelWidgetClass, vbox,
                            XtNlabel, "Reverse Mode",
                            XtNforeground, "black",
                            XtNbackground, "white",
                            "fg_hi", "red", // In reverse mode, reverse_bg = fg_hi
                            "bg_hi", "yellow", // In reverse mode, reverse_text = bg_hi
                            "reverse_mode", True,
                            NULL);

    // 4. Highlight state (state=1)
    XtVaCreateManagedWidget("highlight", wlabelWidgetClass, vbox,
                            XtNlabel, "Highlight State",
                            "state", 1,
                            "fg_hi", "green",
                            "bg_hi", "black",
                            NULL);

    XtRealizeWidget(top);

    // Exit after 1000ms to allow rendering and trace output
    XtAppAddTimeOut(app, 1000, timeout_cb, app);

    XtAppMainLoop(app);
    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    conststr_free();
    m_destruct();
    return 0;
}
