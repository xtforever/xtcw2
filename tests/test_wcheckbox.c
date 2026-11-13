/* Functional test for Wcheckbox: toggle state and checked accessor. */
#include <assert.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wcheckbox.h>
#include <xtcw/register_wb.h>
#include "mls.h"

static int fired = 0;

static void on_toggle(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd; (void)cb;
    fired++;
}

int main(int argc, char **argv)
{
    m_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWCheckbox", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    Widget cb = XtVaCreateManagedWidget("cb", wcheckboxWidgetClass, top,
                                        "label", "Enabled",
                                        "checked", False,
                                        NULL);
    XtAddCallback(cb, "callback", on_toggle, NULL);
    XtRealizeWidget(top);

    assert(wcheckbox_is_checked(cb) == False);
    XtCallActionProc(cb, "toggle", NULL, NULL, 0);
    assert(wcheckbox_is_checked(cb) == True);
    assert(fired == 1);
    XtCallActionProc(cb, "toggle", NULL, NULL, 0);
    assert(wcheckbox_is_checked(cb) == False);
    assert(fired == 2);

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_destruct();

    printf("test_wcheckbox: PASS\n");
    return 0;
}
