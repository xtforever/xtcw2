/* Functional test for WRadioGroup: items, selection actions and focus state. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WRadioGroup.h>
#include <xtcw/register_wb.h>
#include "mls.h"

static XtAppContext app;
static Widget radio;
static int fired = -1;

static void pump(void)
{
    XSync(XtDisplay(radio), False);
    while (XtAppPending(app))
        XtAppProcessEvent(app, XtIMAll);
}

static void act(const char *name)
{
    XtCallActionProc(radio, (String)name, NULL, NULL, 0);
    pump();
}

static void on_select(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd;
    fired = (int)(intptr_t)cb;
}

static int get_int(const char *res)
{
    int v = -999;
    XtVaGetValues(radio, res, &v, NULL);
    return v;
}

int main(int argc, char **argv)
{
    m_init();

    Widget top = XtOpenApplication(&app, "TestWRadioGroup", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);
    XtVaSetValues(top, XtNwidth, (Dimension)400, XtNheight, (Dimension)300, NULL);

    int items = m_create(8, sizeof(char *));
    char *small = "Small";
    char *medium = "Medium";
    char *large = "Large";
    m_put(items, &small);
    m_put(items, &medium);
    m_put(items, &large);

    radio = XtVaCreateManagedWidget("radio", wRadioGroupWidgetClass, top,
                                    "items", items,
                                    "selected", 0,
                                    "itemHeight", 24,
                                    NULL);
    XtAddCallback(radio, "callback", on_select, NULL);
    XtRealizeWidget(top);
    pump();

    assert(get_int("selected") == 0);

    act("move_down");
    assert(get_int("selected") == 1);
    assert(fired == 1);
    act("move_up");
    assert(get_int("selected") == 0);
    act("last");
    assert(get_int("selected") == 2);
    act("first");
    assert(get_int("selected") == 0);

    {
        String p[1] = { "1" };
        XtCallActionProc(radio, "select_index", NULL, p, 1);
        pump();
        assert(get_int("selected") == 1);
        assert(fired == 1);
    }

    /* Focus state drives the accent focus ring. */
    {
        Boolean focused = True;
        XtVaGetValues(radio, "hasFocus", &focused, NULL);
        assert(focused == False);
        act("focus");
        XtVaGetValues(radio, "hasFocus", &focused, NULL);
        assert(focused == True);
        act("focus_out");
        XtVaGetValues(radio, "hasFocus", &focused, NULL);
        assert(focused == False);
    }

    XtDestroyWidget(top);
    m_free(items);
    printf("test_wradiogroup PASSED\n");
    return 0;
}
