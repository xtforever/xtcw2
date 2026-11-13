/* Regression test: WspinBox displays its numeric value and keeps the label
 * in sync with the spin actions. */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WspinBox.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"
#include "theme.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    m_init();
    conststr_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWspinBox", NULL, 0, &argc, argv,
                                   NULL, sessionShellWidgetClass, NULL, 0);
    XtcwRegister(app);

    Widget sp = XtVaCreateManagedWidget("sp", wspinBoxWidgetClass, top,
        XtNvalue, 25,
        XtNmin, 0,
        XtNmax, 120,
        NULL);
    XtRealizeWidget(top);

    char *label = NULL;
    int value = 0;
    int ok = 1;

    XtVaGetValues(sp, XtNlabel, &label, XtNvalue, &value, NULL);
    if (value != 25 || !label || strcmp(label, "25") != 0) {
        fprintf(stderr, "FAIL initial value/label (%d, %s)\n",
                value, label ? label : "(null)");
        ok = 0;
    }

    XtCallActionProc(sp, "spin_up", NULL, NULL, 0);
    label = NULL;
    XtVaGetValues(sp, XtNlabel, &label, XtNvalue, &value, NULL);
    if (value != 26 || !label || strcmp(label, "26") != 0) {
        fprintf(stderr, "FAIL after spin_up (%d, %s)\n",
                value, label ? label : "(null)");
        ok = 0;
    }

    XtCallActionProc(sp, "spin_down", NULL, NULL, 0);
    label = NULL;
    XtVaGetValues(sp, XtNlabel, &label, XtNvalue, &value, NULL);
    if (value != 25 || !label || strcmp(label, "25") != 0) {
        fprintf(stderr, "FAIL after spin_down (%d, %s)\n",
                value, label ? label : "(null)");
        ok = 0;
    }

    String params[1] = { "120" };
    XtCallActionProc(sp, "set_value", NULL, params, 1);
    label = NULL;
    XtVaGetValues(sp, XtNlabel, &label, XtNvalue, &value, NULL);
    if (value != 120 || !label || strcmp(label, "120") != 0) {
        fprintf(stderr, "FAIL after set_value (%d, %s)\n",
                value, label ? label : "(null)");
        ok = 0;
    }

    XtCallActionProc(sp, "spin_up", NULL, NULL, 0);
    label = NULL;
    XtVaGetValues(sp, XtNlabel, &label, XtNvalue, &value, NULL);
    if (value != 120 || !label || strcmp(label, "120") != 0) {
        fprintf(stderr, "FAIL max clamp (%d, %s)\n",
                value, label ? label : "(null)");
        ok = 0;
    }

    XtCallActionProc(sp, "spin_down", NULL, NULL, 0);
    label = NULL;
    XtVaGetValues(sp, XtNlabel, &label, XtNvalue, &value, NULL);
    if (value != 119 || !label || strcmp(label, "119") != 0) {
        fprintf(stderr, "FAIL after spin_down from max (%d, %s)\n",
                value, label ? label : "(null)");
        ok = 0;
    }

    printf("%s: WspinBox value label\n", ok ? "PASS" : "FAIL");

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    theme_destroy();
    conststr_free();
    m_destruct();
    return ok ? 0 : 1;
}
