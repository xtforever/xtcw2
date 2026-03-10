#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WlistMulti.h>
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
    Widget top = XtOpenApplication(&app, "TestWlistMulti", NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL, 0);

    XtcwRegister(app);

    int list_data = m_create(10, sizeof(char*));
    char *row1 = "Item1\tSize1\tType1";
    char *row2 = "Item2\tSize2\tType2";
    m_put(list_data, &row1);
    m_put(list_data, &row2);

    Widget list = XtVaCreateManagedWidget("list", wlistMultiWidgetClass, top,
                                          "tableStrs", list_data,
                                          "columnWidths", "100,100,100",
                                          "retexCells", True,
                                          "height", 100,
                                          "width", 300,
                                          NULL);

    XtRealizeWidget(top);

    // Process events to trigger initial expose and cell rendering
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
