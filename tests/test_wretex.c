/* Regression test for Wretex.realize: the window must select Expose events,
 * otherwise the widget is never repainted and stays black. */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wretex.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"
#include "theme.h"
#include <stdio.h>
#include <string.h>

extern void svg_factory_destroy(void);

int main(int argc, char **argv)
{
    m_init();
    conststr_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWretex", NULL, 0, &argc, argv,
                                   NULL, sessionShellWidgetClass, NULL, 0);
    XtcwRegister(app);

    Widget w = XtVaCreateManagedWidget("w", wretexWidgetClass, top,
        XtNtext, "Hello \\bf bold\\rm  \\includesvg{LuaRunner/file.svg}{12pt}{12pt}",
        XtNwidth, 300,
        XtNheight, 100,
        NULL);

    XtRealizeWidget(top);

    int ok = 1;
    XWindowAttributes wa;
    if (!XGetWindowAttributes(XtDisplay(w), XtWindow(w), &wa)) {
        fprintf(stderr, "FAIL: cannot query Wretex window attributes\n");
        ok = 0;
    } else if (!(wa.your_event_mask & ExposureMask)) {
        fprintf(stderr, "FAIL: Wretex window does not select ExposureMask\n");
        ok = 0;
    }

    char *text = NULL;
    XtVaGetValues(w, XtNtext, &text, NULL);
    if (!text || !strstr(text, "includesvg")) {
        fprintf(stderr, "FAIL: Wretex text resource not set\n");
        ok = 0;
    }

    printf("%s: Wretex realize/expose\n", ok ? "PASS" : "FAIL");

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    theme_destroy();
    svg_factory_destroy();
    conststr_free();
    m_destruct();
    return ok ? 0 : 1;
}
