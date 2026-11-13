/* Functional test for WStatusBar: message label, right-side permanent widget,
 * padding/gap geometry and live message update. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WStatusBar.h>
#include <xtcw/WStatusBarP.h>
#include <xtcw/Wbutton.h>
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
    Widget top = XtOpenApplication(&app, "TestWStatusBar", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    Widget sb = XtVaCreateManagedWidget("sb", wStatusBarWidgetClass, top,
                                        XtNwidth, (Dimension)400,
                                        "message", "Ready",
                                        "itemGap", 8,
                                        "barPadding", 4,
                                        NULL);
    Widget ok = XtVaCreateManagedWidget("ok", wbuttonWidgetClass, sb,
                                        XtNlabel, "OK", NULL);

    XtRealizeWidget(top);

    WStatusBarWidget sbw = (WStatusBarWidget)sb;
    Widget msg = sbw->wStatusBar.message_label;
    assert(msg != NULL);

    int bar_h = 0;
    XtVaGetValues(sb, "barHeight", &bar_h, NULL);
    assert(bar_h >= 32);

    XtWidgetGeometry req, pref;
    memset(&req, 0, sizeof req);
    pref.request_mode = 0;
    assert(XtQueryGeometry(msg, &req, &pref) == XtGeometryAlmost);
    assert((pref.request_mode & CWHeight) && (int)pref.height > 0);
    assert(bar_h >= (int)pref.height + 2 * 4);

    int mx, my, mw, mh;
    int ox, oy, ow, oh;
    geom(msg, &mx, &my, &mw, &mh);
    geom(ok, &ox, &oy, &ow, &oh);

    assert(mx == 4 && my == 4 && mh == bar_h - 8);
    assert(ox == 400 - 4 - ow);
    assert(oy == 4 && oh == bar_h - 8);
    assert(mx + mw + 8 == ox);

    String label = NULL;
    XtVaGetValues(msg, XtNlabel, &label, NULL);
    assert(label && strcmp(label, "Ready") == 0);

    XtVaSetValues(sb, "message", "Saved", NULL);
    XtVaGetValues(msg, XtNlabel, &label, NULL);
    assert(label && strcmp(label, "Saved") == 0);

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_destruct();

    printf("test_wstatusbar: PASS\n");
    return 0;
}
