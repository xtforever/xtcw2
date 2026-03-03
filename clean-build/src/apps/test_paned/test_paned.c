#define APP_NAME "test_paned"

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
    "*WclResFiles: test_paned.ad",
    NULL
};

int main(int argc, char **argv)
{
    XtAppContext app;
    fprintf(stderr, "DEBUG: test_paned main start\n");
    m_init(); /* Initialize MLS library */
	trace_level=1;
    XtSetLanguageProc(NULL, NULL, NULL);
    XawInitializeWidgetSet();

    Widget appShell = XtOpenApplication(&app, APP_NAME,
                                        NULL, 0,
                                        &argc, argv,
                                        fallback_resources,
                                        sessionShellWidgetClass,
                                        NULL, 0);

    __auto_type  s = appShell;

    RegisterApplication(appShell);
    XpRegisterAll(app);
    WcWidgetCreation(appShell);
    XtRealizeWidget(appShell);
    grab_window_quit(appShell);

    XtAppMainLoop(app);
    TRACE(1,"main loop exit");
	
    m_destruct();
    return 0;
}
