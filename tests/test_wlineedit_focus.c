/* Mouse click must move Xt's keyboard focus to a WLineEdit. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WLineEdit.h>
#include <xtcw/WComboBox.h>
#include <xtcw/Gridbox.h>
#include <xtcw/register_wb.h>
#include "mls.h"

static void click_widget(Widget w, int x)
{
    XButtonEvent ev;
    memset(&ev, 0, sizeof ev);
    ev.type = ButtonPress;
    ev.window = XtWindow(w);
    ev.x = x;
    ev.y = 5;
    ev.time = 1000;
    XtCallActionProc(w, "lineedit_click", (XEvent *)&ev, NULL, 0);
}

int main(int argc, char **argv)
{
    m_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWLineEditFocus", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, top,
                                          XtNdefaultDistance, 4,
                                          XtNwidth, 400, XtNheight, 120, NULL);
    Widget first = XtVaCreateManagedWidget("first", wLineEditWidgetClass, grid,
                                           XtNgridx, 0, XtNgridy, 0,
                                           XtNweightx, 1, XtNweighty, 0,
                                           XtNlabel, "first", NULL);
    Widget second = XtVaCreateManagedWidget("second", wLineEditWidgetClass, grid,
                                            XtNgridx, 0, XtNgridy, 1,
                                            XtNweightx, 1, XtNweighty, 0,
                                            XtNlabel, "second", NULL);
    Widget combo = XtVaCreateManagedWidget("combo", wComboBoxWidgetClass, grid,
                                           XtNgridx, 0, XtNgridy, 2,
                                           XtNweightx, 1, XtNweighty, 0, NULL);
    XtRealizeWidget(top);

    /* Start with the first line edit focused. */
    XtCallActionProc(first, "focus", NULL, NULL, 0);
    assert(XtGetKeyboardFocusWidget(top) == first);

    /* Clicking another line edit must move keyboard focus to it. */
    click_widget(second, 5);
    if (XtGetKeyboardFocusWidget(top) != second) {
        fprintf(stderr, "click on WLineEdit did not take keyboard focus\n");
        return 1;
    }

    /* Focus the combo, then click the first line edit. */
    XtCallActionProc(combo, "focus", NULL, NULL, 0);
    assert(XtGetKeyboardFocusWidget(top) == combo);
    click_widget(first, 5);
    if (XtGetKeyboardFocusWidget(top) != first) {
        fprintf(stderr, "click on WLineEdit did not take focus from combo\n");
        return 1;
    }

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_destruct();

    printf("test_wlineedit_focus: PASS\n");
    return 0;
}
