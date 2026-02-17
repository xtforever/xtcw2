#define APP_NAME "main"

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/XawInit.h>
#include <WcCreate.h>
#include <Xp.h>
#include "wcreg2.h"
#include <xtcw/register_wb.h>
#include "mls.h"

void quit_cb(Widget w, void *u, void *c)
{
    XtAppSetExitFlag(XtWidgetToApplicationContext(w));
}

static void RegisterApplication(Widget top)
{
    RCB(top, quit_cb);
    XtcwRegister(XtWidgetToApplicationContext(top));
}

char *fallback_resources[] = {
    "*WclResFiles: main.ad",
    NULL
};

int main(int argc, char **argv)
{
    XtAppContext app;
    m_init();

    XtSetLanguageProc(NULL, NULL, NULL);
    XawInitializeWidgetSet();

    Widget appShell = XtOpenApplication(&app, APP_NAME,
                                        NULL, 0,
                                        &argc, argv,
                                        fallback_resources,
                                        sessionShellWidgetClass,
                                        NULL, 0);

    RegisterApplication(appShell);
    XpRegisterAll(app);
    WcWidgetCreation(appShell);
    XtRealizeWidget(appShell);

    XtAppMainLoop(app);

    XtDestroyWidget(appShell);
    m_destruct();
    return 0;
}
