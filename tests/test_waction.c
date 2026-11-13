/* Functional test for WAction: checkable toggle, label update, callback. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WAction.h>
#include <xtcw/register_wb.h>
#include "mls.h"

static int fired = 0;

static void on_action(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd; (void)cb;
    fired++;
}

int main(int argc, char **argv)
{
    m_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWAction", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    Widget act = XtVaCreateManagedWidget("act", wActionWidgetClass, top,
                                         "label", "Show status",
                                         "checkable", True,
                                         "shortcut", "Ctrl+S",
                                         NULL);
    XtAddCallback(act, "callback", on_action, NULL);
    XtRealizeWidget(top);

    Boolean checked = True;
    String label = NULL;

    XtVaGetValues(act, "checked", &checked, XtNlabel, &label, NULL);
    assert(checked == False);
    assert(label && strstr(label, "[ ]") != NULL);
    assert(strstr(label, "Ctrl+S") != NULL);

    XtCallActionProc(act, "notify", NULL, NULL, 0);
    XtVaGetValues(act, "checked", &checked, XtNlabel, &label, NULL);
    assert(checked == True);
    assert(label && strstr(label, "[x]") != NULL);
    assert(strstr(label, "Ctrl+S") != NULL);
    assert(fired == 1);

    XtCallActionProc(act, "notify", NULL, NULL, 0);
    XtVaGetValues(act, "checked", &checked, XtNlabel, &label, NULL);
    assert(checked == False);
    assert(label && strstr(label, "[ ]") != NULL);
    assert(fired == 2);

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_destruct();

    printf("test_waction: PASS\n");
    return 0;
}
