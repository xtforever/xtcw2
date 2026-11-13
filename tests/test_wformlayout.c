/* Functional test for WFormLayout: label/field columns and row gaps. */
#include <assert.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WFormLayout.h>
#include <xtcw/Wlabel.h>
#include <xtcw/register_wb.h>
#include "mls.h"

static void geom(Widget w, int *x, int *y, int *width, int *height)
{
    Position px = 0, py = 0;
    Dimension pw = 0, ph = 0;
    XtVaGetValues(w, XtNx, &px, XtNy, &py,
                  XtNwidth, &pw, XtNheight, &ph, NULL);
    *x = px; *y = py; *width = pw; *height = ph;
}

int main(int argc, char **argv)
{
    m_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWFormLayout", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    Widget form = XtVaCreateManagedWidget("form", wFormLayoutWidgetClass, top,
                                          XtNwidth, (Dimension)400,
                                          "labelWidth", 100,
                                          "columnGap", 8,
                                          "rowGap", 8,
                                          "margin", 12,
                                          NULL);
    Widget l1 = XtVaCreateManagedWidget("l1", wlabelWidgetClass, form,
                                        "role", "label", "label", "Title:", NULL);
    Widget f1 = XtVaCreateManagedWidget("f1", wlabelWidgetClass, form,
                                        "role", "field", "label", "Recipe title", NULL);
    Widget l2 = XtVaCreateManagedWidget("l2", wlabelWidgetClass, form,
                                        "role", "label", "label", "Author:", NULL);
    Widget f2 = XtVaCreateManagedWidget("f2", wlabelWidgetClass, form,
                                        "role", "field", "label", "Author name", NULL);

    XtRealizeWidget(top);

    int fx, fy, fw, fh;
    int x1, y1, w1, h1;
    int x2, y2, w2, h2;
    int x3, y3, w3, h3;
    int x4, y4, w4, h4;
    geom(form, &fx, &fy, &fw, &fh);
    geom(l1, &x1, &y1, &w1, &h1);
    geom(f1, &x2, &y2, &w2, &h2);
    geom(l2, &x3, &y3, &w3, &h3);
    geom(f2, &x4, &y4, &w4, &h4);

    assert(x1 == 12 && y1 == 12 && w1 == 100);
    assert(x2 == 120 && y2 == 12);
    assert(w2 == 400 - 12 - 100 - 8 - 12);
    assert(y3 == y1 + h1 + 8);
    assert(x4 == 120 && y4 == y3);
    assert(w4 == w2);

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_destruct();

    printf("test_wformlayout: PASS\n");
    return 0;
}
