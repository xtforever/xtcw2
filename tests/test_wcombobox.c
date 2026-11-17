/* Functional test for WComboBox: items, selected label, selection action. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xft/Xft.h>
#include <xtcw/WComboBox.h>
#include <xtcw/WComboBoxP.h>
#include <xtcw/register_wb.h>
#include "mls.h"

static int fired = -1;

static void on_combo(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd;
    fired = (int)(intptr_t)cb;
}

int main(int argc, char **argv)
{
    m_init();

    int items = m_create(10, sizeof(char *));
    char *a = "Starter";
    char *b = "Main";
    char *c = "Dessert";
    m_put(items, &a);
    m_put(items, &b);
    m_put(items, &c);

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWComboBox", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    Widget combo = XtVaCreateManagedWidget("combo", wComboBoxWidgetClass, top,
                                           "items", items,
                                           "selected", 1,
                                           "placeholder", "Choose...",
                                           NULL);
    XtAddCallback(combo, "callback", on_combo, NULL);
    XtRealizeWidget(top);

    String label = NULL;
    int selected = -1;
    XtVaGetValues(combo, XtNlabel, &label, "selected", &selected, NULL);
    assert(selected == 1);
    assert(label && strcmp(label, "Main") == 0);

    String params[1] = { "2" };
    XtCallActionProc(combo, "select_index", NULL, params, 1);
    XtVaGetValues(combo, XtNlabel, &label, "selected", &selected, NULL);
    assert(selected == 2);
    assert(label && strcmp(label, "Dessert") == 0);
    assert(fired == 2);

    /* Popup open/close must not crash or leave the popup behind. */
    XtCallActionProc(combo, "notify", NULL, NULL, 0);
    WComboBoxWidget cw = (WComboBoxWidget)combo;
    assert(cw->wComboBox.shell != NULL);
    Position sx = 0, sy = 0;
    Dimension sw = 0, sh = 0;
    XtVaGetValues(cw->wComboBox.shell, XtNx, &sx, XtNy, &sy,
                  XtNwidth, &sw, XtNheight, &sh, NULL);
    int cx = 0, cy = 0;
    Window child = None;
    XTranslateCoordinates(XtDisplay(combo), XtWindow(combo),
                          DefaultRootWindow(XtDisplay(combo)),
                          0, 0, &cx, &cy, &child);
    assert(sy >= cy + (int)combo->core.height);
    WidgetList buttons = NULL;
    Cardinal nbuttons = 0;
    XtVaGetValues(cw->wComboBox.menu, XtNchildren, &buttons,
                  XtNnumChildren, &nbuttons, NULL);
    assert(nbuttons == 3);
    XtCallActionProc(buttons[2], "notify", NULL, NULL, 0);
    XtVaGetValues(combo, XtNlabel, &label, "selected", &selected, NULL);
    assert(selected == 2);
    assert(label && strcmp(label, "Dessert") == 0);
    assert(!cw->wComboBox.entry_mode);

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_free(items);
    m_destruct();

    printf("test_wcombobox: PASS\n");
    return 0;
}
