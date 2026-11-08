/* Functional test for WToggleSwitch: state, notify semantics and the
 * animation timer lifecycle.  Runs headless under Xvfb.
 *
 *   - resource assignment (XtVaSetValues / wtoggleswitch_set_checked) flips
 *     the state immediately but does NOT invoke the callback;
 *   - the user action press_toggle flips the state and notifies once;
 *   - destroying the widget while an animation is pending cancels the timer
 *     (no crash / use-after-free).
 */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WToggleSwitch.h>
#include <xtcw/VBox.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"

typedef struct {
    XtAppContext app;
    Widget       sw;
    int          fired;
    intptr_t     last;
} Ctx;

static void on_toggle(Widget w, XtPointer client, XtPointer call)
{
    Ctx *c = (Ctx*)client;
    (void)w;
    c->fired++;
    c->last = (intptr_t)call;
}

static void step2(XtPointer client, XtIntervalId *id)
{
    Ctx *c = (Ctx*)client;
    (void)id;

    /* the programmatic set flipped the state but did not notify */
    assert(wtoggleswitch_is_checked(c->sw) == True);
    assert(c->fired == 0);

    /* a user toggle notifies exactly once with the new value (False == 0) */
    XtCallActionProc(c->sw, "press_toggle", NULL, NULL, 0);
    assert(wtoggleswitch_is_checked(c->sw) == False);
    assert(c->fired == 1);
    assert(c->last == 0);

    /* start an animation and destroy immediately: destroy must cancel the
     * pending timer */
    wtoggleswitch_set_checked(c->sw, True);
    XtDestroyWidget(c->sw);
    c->sw = NULL;

    XtAppSetExitFlag(c->app);
}

int main(int argc, char **argv)
{
    m_init();
    conststr_init();
    extern int trace_level;
    trace_level = 2;

    Ctx c;
    c.fired = 0;
    c.last = -1;

    Widget top = XtOpenApplication(&c.app, "TestWToggleSwitch", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(c.app);

    Widget vbox = XtVaCreateManagedWidget("vbox", vBoxWidgetClass, top, NULL);
    c.sw = XtVaCreateManagedWidget("sw", wToggleSwitchWidgetClass, vbox,
                                   XtNduration, 200,
                                   NULL);
    XtAddCallback(c.sw, XtNcallback, on_toggle, &c);

    XtRealizeWidget(top);

    /* initial state is off */
    assert(wtoggleswitch_is_checked(c.sw) == False);

    /* programmatic set: state flips, callback stays silent */
    wtoggleswitch_set_checked(c.sw, True);
    assert(wtoggleswitch_is_checked(c.sw) == True);
    assert(c.fired == 0);

    XtAppAddTimeOut(c.app, 500, step2, &c);
    XtAppMainLoop(c.app);

    XtDestroyWidget(top);
    XtDestroyApplicationContext(c.app);
    conststr_free();
    m_destruct();

    printf("test_wtoggleswitch: PASS\n");
    return 0;
}
