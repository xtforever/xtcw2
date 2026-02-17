#define APP_NAME "hello"

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/XawInit.h>
#include <WcCreate.h>
#include <Xp.h>
#include <xutil.h>
#include "wcreg2.h"
#include <xtcw/register_wb.h>
#include "mls.h"

void quit_cb(Widget w, void *u, void *c)
{
    XtAppSetExitFlag(XtWidgetToApplicationContext(w));
}

static void RegisterApplication(Widget top)
{
    /* Register application specific callbacks */
    RCB(top, quit_cb);
    RCB(top, xtcw_quit);
    
    /* Register all widgets from the XTCW library */
    XtcwRegister(XtWidgetToApplicationContext(top));
}

char *fallback_resources[] = {
    "*WclResFiles: hello.ad",
    NULL
};

static void destroy_done_cb(Widget w, XtPointer client_data, XtPointer call_data) {
    *(int*)client_data = 1;
}

int main(int argc, char **argv)
{
    XtAppContext app;
    m_init(); /* Initialize MLS library */

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
    grab_window_quit(appShell);

    XtAppMainLoop(app);

    int destroyed = 0;
    XtAddCallback(appShell, XtNdestroyCallback, destroy_done_cb, &destroyed);
    XtDestroyWidget(appShell);
    while(!destroyed) {
        XtAppProcessEvent(app, XtIMAll);
    }

    m_destruct();
    return 0;
}
