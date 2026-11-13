/* Functional test for the native Wviewport scroll container.
 *
 * Verifies the structural contract (not pixel output):
 *   - the viewport owns a clip + two scrollbar internal windows;
 *   - the managed content's window is reparented into the clip;
 *   - wviewport_set_child() swaps the content at runtime.
 *
 * Runs headless under Xvfb.  The viewport is placed in a Gridbox with
 * weight/fill and the shell is sized smaller than the content, so both
 * scrollbars appear (a scroll container's preferred size is its content).
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Core.h>
#include <xtcw/Wviewport.h>
#include <xtcw/WviewportP.h>
#include <xtcw/Gridbox.h>
#include <xtcw/register_wb.h>
#include "mls.h"

int main(int argc, char **argv)
{
    m_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWviewport", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    /* force the shell smaller than the content */
    XtVaSetValues(top, XtNwidth, (Dimension)300, XtNheight, (Dimension)200, NULL);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, top, NULL);

    Widget vp = XtVaCreateManagedWidget("vp", wviewportWidgetClass, grid,
                                        "gridx", 0, "gridy", 0,
                                        "weightx", 1, "weighty", 1, "fill", 3,
                                        NULL);

    Widget big = XtVaCreateManagedWidget("big", widgetClass, vp,
                                         XtNwidth,  (Dimension)800,
                                         XtNheight, (Dimension)900,
                                         NULL);

    XtRealizeWidget(top);

    /* the first managed non-internal child becomes the content at realize */
    assert(wviewport_get_child(vp) == big);

    Display *dpy = XtDisplay(vp);
    Window vpw = XtWindow(vp);
    assert(vpw != None);

    /* viewport owns clip + vertical + horizontal scrollbar windows */
    Window root, parent, *kids = NULL;
    unsigned int nkids = 0;
    assert(XQueryTree(dpy, vpw, &root, &parent, &kids, &nkids));
    assert(nkids >= 3);

    /* both scrollbars are mapped and have the expected thin/thick shapes */
    int mapped_v = 0, mapped_h = 0;
    for (unsigned int i = 0; i < nkids; i++) {
        XWindowAttributes wa;
        if (!XGetWindowAttributes(dpy, kids[i], &wa)) continue;
        if (wa.map_state != IsViewable) continue;
        if ((int)wa.height > 2 * (int)wa.width)  mapped_v = 1;
        if ((int)wa.width  > 2 * (int)wa.height) mapped_h = 1;
    }
    assert(mapped_v && mapped_h);

    /* the content window was reparented: its window parent is not the viewport */
    Window bigw = XtWindow(big);
    assert(bigw != None);
    Window r2, p2, *kids2 = NULL;
    unsigned int n2 = 0;
    assert(XQueryTree(dpy, bigw, &r2, &p2, &kids2, &n2));
    assert(p2 != vpw);
    if (kids2) XFree(kids2);

    /* Regression: dragging the vertical thumb fully down must reach maxy.
     * The callback used maxy as the inverse extent while relayout() synced
     * with the total content height, so the thumb stopped early. */
    {
        WviewportWidget vw = (WviewportWidget)vp;
        int maxy = (int)big->core.height - (int)vw->wviewport.clip->core.height;
        XButtonEvent ev;

        assert(maxy > 0);
        memset(&ev, 0, sizeof ev);
        ev.type = ButtonPress;
        ev.y = (int)vw->wviewport.vsb->core.height;
        XtCallActionProc(vw->wviewport.vsb, "sl_motion", (XEvent *)&ev, NULL, 0);
        XSync(XtDisplay(vp), False);
        while (XtAppPending(app))
            XtAppProcessEvent(app, XtIMAll);
        assert(vw->wviewport.scroll_y >= maxy - 1);
        assert(vw->wviewport.scroll_y <= maxy);

        int maxx = (int)big->core.width - (int)vw->wviewport.clip->core.width;
        assert(maxx > 0);
        memset(&ev, 0, sizeof ev);
        ev.type = ButtonPress;
        ev.x = (int)vw->wviewport.hsb->core.width;
        XtCallActionProc(vw->wviewport.hsb, "sl_motion", (XEvent *)&ev, NULL, 0);
        XSync(XtDisplay(vp), False);
        while (XtAppPending(app))
            XtAppProcessEvent(app, XtIMAll);
        assert(vw->wviewport.scroll_x >= maxx - 1);
        assert(vw->wviewport.scroll_x <= maxx);
    }

    /* runtime child swap */
    Widget big2 = XtVaCreateManagedWidget("big2", widgetClass, vp,
                                          XtNwidth,  (Dimension)400,
                                          XtNheight, (Dimension)400,
                                          NULL);
    wviewport_set_child(vp, big2);
    assert(wviewport_get_child(vp) == big2);

    wviewport_set_child(vp, NULL);
    assert(wviewport_get_child(vp) == NULL);

    if (kids) XFree(kids);
    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_destruct();

    printf("test_wviewport: PASS\n");
    return 0;
}
