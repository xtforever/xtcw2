/* Functional test for WAppWindow: slot-based menu/toolbar/content/status
 * layout, theme margin/gap defaults, and bottom-anchored status row. */
#include <assert.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WAppWindow.h>
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
    Widget top = XtOpenApplication(&app, "TestWAppWindow", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    XtVaSetValues(top, XtNwidth, (Dimension)700, XtNheight, (Dimension)600, NULL);

    Widget win = XtVaCreateManagedWidget("app", wAppWindowWidgetClass, top,
                                         XtNwidth, (Dimension)700,
                                         XtNheight, (Dimension)600,
                                         NULL);

    Widget menu = XtVaCreateManagedWidget("menu", wlabelWidgetClass, win,
                                          "slot", "menu", NULL);
    Widget toolbar = XtVaCreateManagedWidget("toolbar", wlabelWidgetClass, win,
                                             "slot", "toolbar", NULL);
    Widget content = XtVaCreateManagedWidget("content", wlabelWidgetClass, win,
                                             "slot", "content", NULL);
    Widget status = XtVaCreateManagedWidget("status", wlabelWidgetClass, win,
                                            "slot", "status", NULL);

    XtRealizeWidget(top);

    int mx, my, mw, mh;
    int tx, ty, tw, th;
    int cx, cy, cw, ch;
    int sx, sy, sw, sh;
    geom(menu, &mx, &my, &mw, &mh);
    geom(toolbar, &tx, &ty, &tw, &th);
    geom(content, &cx, &cy, &cw, &ch);
    geom(status, &sx, &sy, &sw, &sh);

    /* Default theme metrics: margin 16, group gap 12. */
    assert(mx == 16 && my == 16);
    assert(mw == 700 - 2 * 16);
    assert(tx == 16 && ty == my + mh + 12);
    assert(cx == 16 && cy == ty + th + 12);
    assert(cw == mw);
    assert(ch > 0);
    assert(sx == 16 && sw == mw);
    assert(sy + sh == 600 - 16);
    assert(sy > cy + ch);

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_destruct();

    printf("test_wappwindow: PASS\n");
    return 0;
}
