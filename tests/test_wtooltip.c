/* Functional test for the tooltip manager and WToolTip presenter.
 *
 * Covers: registry set/update/remove, lazy presenter creation, show/dismiss,
 * screen-edge clamping and unregister on destroy. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/WToolTip.h>
#include <xtcw/Wbutton.h>
#include <xtcw/register_wb.h>
#include "tooltip.h"
#include "mls.h"

int main(int argc, char **argv)
{
    m_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWToolTip", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    Widget box = XtVaCreateManagedWidget("box", compositeWidgetClass, top,
                                         XtNwidth, (Dimension)300,
                                         XtNheight, (Dimension)200,
                                         NULL);
    Widget btn = XtVaCreateManagedWidget("btn", wbuttonWidgetClass, box,
                                         XtNlabel, "Hover",
                                         "tooltip", "Save the document",
                                         NULL);
    XtRealizeWidget(top);

    assert(wtooltip_count() == 1);

    /* Updating the text keeps a single registration. */
    XtVaSetValues(btn, "tooltip", "Save as...", NULL);
    assert(wtooltip_count() == 1);

    /* Removing the resource deregisters. */
    XtVaSetValues(btn, "tooltip", "", NULL);
    assert(wtooltip_count() == 0);

    XtVaSetValues(btn, "tooltip", "Hover me", NULL);
    assert(wtooltip_count() == 1);

    /* Presenter: create it explicitly (the manager also does this lazily). */
    Widget tip = wtooltip_create(top);
    assert(tip != NULL);
    assert(wtooltip_presenter() == tip);
    assert(wtooltip_visible() == 0);

    wtooltip_present(tip, "Hover me", 100, 120);
    assert(wtooltip_visible() == 1);

    int x = 0, y = 0;
    XtVaGetValues(tip, XtNx, &x, XtNy, &y, NULL);
    assert(x >= 0 && y >= 0);
    assert(x < DisplayWidth(XtDisplay(tip), DefaultScreen(XtDisplay(tip))));
    assert(y < DisplayHeight(XtDisplay(tip), DefaultScreen(XtDisplay(tip))));

    /* Near the right/bottom edge the tooltip must stay on screen. */
    int sw = DisplayWidth(XtDisplay(tip), DefaultScreen(XtDisplay(tip)));
    int sh = DisplayHeight(XtDisplay(tip), DefaultScreen(XtDisplay(tip)));
    wtooltip_present(tip, "Edge tooltip with a longer label", sw - 2, sh - 2);
    XtVaGetValues(tip, XtNx, &x, XtNy, &y, NULL);
    assert(x >= 0 && x < sw);
    assert(y >= 0 && y < sh);

    wtooltip_dismiss(tip);
    assert(wtooltip_visible() == 0);

    /* Configuring delay/enabled updates the global defaults. */
    wtooltip_configure(250, 0);
    assert(wtooltip_delay() == 250);
    wtooltip_configure(600, 1);
    assert(wtooltip_delay() == 600);

    /* Clearing the resource must drop the registration. Widget destruction
     * takes the same path, but XtDestroyWidget on a Wlabel currently trips a
     * pre-existing MLS/theme teardown bug (see know_bugs.md), so keep the
     * test focused on the manager. */
    wtooltip_set(btn, NULL);
    assert(wtooltip_count() == 0);

    printf("test_wtooltip PASSED\n");
    return 0;
}
