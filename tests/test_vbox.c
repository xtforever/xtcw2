#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Wbutton.h>
#include <xtcw/VBox.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"

int main(int argc, char **argv) {
    m_init();
    conststr_init();
    extern int trace_level;
    trace_level = 2;

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestVBox", NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL, 0);

    XtcwRegister(app);

    Widget vbox = XtVaCreateManagedWidget("vbox", vBoxWidgetClass, top,
                            "spacing", 10,
                            NULL);

    XtVaCreateManagedWidget("label1", wlabelWidgetClass, vbox,
                            XtNlabel, "First Label",
                            NULL);

    XtVaCreateManagedWidget("label2", wlabelWidgetClass, vbox,
                            XtNlabel, "Second Label",
                            NULL);

    XtVaCreateManagedWidget("button", wbuttonWidgetClass, vbox,
                            XtNlabel, "Click Me",
                            NULL);

    XtRealizeWidget(top);

    XtAppMainLoop(app);

    conststr_free();
    m_destruct();
    return 0;
}
