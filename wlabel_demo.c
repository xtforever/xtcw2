#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <WcCreate.h>
#include <xtcw/register_wb.h>
#include <conststr.h>
#include "wcreg2.h"
#include "mls.h"
#include <stdio.h>
#include <stdlib.h>

void quit_cb(Widget w, void *u, void *c)
{
    exit(0);
}

static char *fallback_resources[] = {
    "*WclResFiles: WlabelDemo.ad",
    NULL
};

int main(int argc, char **argv) {
    XtAppContext app;
    Widget top;

    m_init();
    conststr_init();
    trace_level = 1;

    top = XtOpenApplication(&app, "wlabel_demo", NULL, 0, &argc, argv, fallback_resources,
                            applicationShellWidgetClass, NULL, 0);

    /* Register application specific callbacks */
    RCB(top, quit_cb);

    /* Register all widgets for Wcl */
    XtcwRegister(app);

    /* Create the UI from resource file */
    WcWidgetCreation(top);

    XtRealizeWidget(top);
    XtAppMainLoop(app);

    return 0;
}
