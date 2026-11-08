/* Functional test for WtabWidget: tab selection, callback semantics and
 * constraint/title teardown.  Runs headless under Xvfb (ASan build catches
 * any double free of the owned tab titles).
 */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WtabWidget.h>
#include <xtcw/VBox.h>
#include <xtcw/register_wb.h>
#include "mls.h"

typedef struct {
    XtAppContext app;
    Widget       tabs;
    Widget       p1;
    Widget       p2;
    int          fired;
    intptr_t     last;
} Ctx;

static void on_tab(Widget w, XtPointer client, XtPointer call)
{
    Ctx *c = (Ctx*)client;
    (void)w;
    c->fired++;
    c->last = (intptr_t)call;
}

int main(int argc, char **argv)
{
    m_init();

    Ctx c;
    c.fired = 0;
    c.last  = -1;

    Widget top = XtOpenApplication(&c.app, "TestWtabWidget", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(c.app);

    Widget vbox = XtVaCreateManagedWidget("vbox", vBoxWidgetClass, top, NULL);
    c.tabs = XtVaCreateManagedWidget("tabs", wtabWidgetWidgetClass, vbox,
                                     XtNwidth,  (Dimension)300,
                                     XtNheight, (Dimension)200,
                                     NULL);
    XtAddCallback(c.tabs, "callback", on_tab, &c);

    c.p1 = XtVaCreateManagedWidget("p1", vBoxWidgetClass, c.tabs,
                                   "tabTitle", "One",
                                   XtNwidth,  (Dimension)120,
                                   XtNheight, (Dimension)80,
                                   NULL);
    c.p2 = XtVaCreateManagedWidget("p2", vBoxWidgetClass, c.tabs,
                                   "tabTitle", "Two",
                                   XtNwidth,  (Dimension)120,
                                   XtNheight, (Dimension)80,
                                   NULL);

    XtRealizeWidget(top);

    /* tab 0 is selected; only it is managed; no callback during realize */
    assert(wtabwidget_get_selected(c.tabs) == 0);
    assert(XtIsManaged(c.p1));
    assert(!XtIsManaged(c.p2));
    assert(c.fired == 0);

    /* selecting tab 1 fires the callback exactly once with the new index */
    XtVaSetValues(c.tabs, "selected", 1, NULL);
    assert(wtabwidget_get_selected(c.tabs) == 1);
    assert(XtIsManaged(c.p2));
    assert(!XtIsManaged(c.p1));
    assert(c.fired == 1);
    assert(c.last == 1);

    /* setting the same value again must not re-fire (no spurious callback) */
    XtVaSetValues(c.tabs, "selected", 1, NULL);
    assert(c.fired == 1);

    /* teardown: destroy one page individually (delete_child + constraint
     * destroy), then destroy the parent (remaining page via constraint
     * destroy).  Both paths run the idempotent title free -> no double free. */
    XtDestroyWidget(c.p2);
    c.p2 = NULL;
    XtDestroyWidget(top);
    XtDestroyApplicationContext(c.app);

    m_destruct();
    printf("test_wtabwidget: PASS\n");
    return 0;
}
