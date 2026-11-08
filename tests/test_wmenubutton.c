/* Functional test for WmenuButton: popup open/close API, the toggle action,
 * the settable `open` resource, and safe teardown while the menu is open
 * (the click-catcher and popup shell must be cleaned up -- no lingering
 * full-screen window, no crash).  Runs headless under Xvfb.
 */
#include <assert.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WmenuButton.h>
#include <xtcw/VBox.h>
#include <xtcw/register_wb.h>
#include "mls.h"

int main(int argc, char **argv)
{
    m_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWmenuButton", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    Widget vbox = XtVaCreateManagedWidget("vbox", vBoxWidgetClass, top, NULL);
    Widget mb = XtVaCreateManagedWidget("mb", wmenuButtonWidgetClass, vbox,
                                        "popupWidth",  (Dimension)120,
                                        "popupHeight", (Dimension)80,
                                        "buttonSize",  (Dimension)32,
                                        NULL);

    /* one arbitrary content child (a plain box -- no retex, keeps the test
     * focused on the popup lifecycle) */
    XtVaCreateManagedWidget("content", vBoxWidgetClass, mb,
                            XtNwidth,  (Dimension)120,
                            XtNheight, (Dimension)80,
                            NULL);

    XtRealizeWidget(top);

    assert(wmenubutton_is_open(mb) == False);

    /* exported API */
    wmenubutton_popup(mb);
    assert(wmenubutton_is_open(mb) == True);
    wmenubutton_popdown(mb);
    assert(wmenubutton_is_open(mb) == False);

    /* toggle action */
    XtCallActionProc(mb, "toggle", NULL, NULL, 0);
    assert(wmenubutton_is_open(mb) == True);
    XtCallActionProc(mb, "toggle", NULL, NULL, 0);
    assert(wmenubutton_is_open(mb) == False);

    /* settable `open` resource (what LUI gui.set uses to close on selection) */
    XtVaSetValues(mb, "open", True, NULL);
    assert(wmenubutton_is_open(mb) == True);
    XtVaSetValues(mb, "open", False, NULL);
    assert(wmenubutton_is_open(mb) == False);

    /* destroy while open: teardown must remove the click catcher and popdown
     * the shell (ASan would flag a lingering window / use-after-free). */
    wmenubutton_popup(mb);
    assert(wmenubutton_is_open(mb) == True);
    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);

    m_destruct();
    printf("test_wmenubutton: PASS\n");
    return 0;
}
