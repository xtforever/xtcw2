/* Regression test: WAudioChassis is a fixed-size Board container.
 *
 * Checks:
 *  - fixed 1160x770 size survives a programmatic resize attempt
 *  - 50 arbitrary children keep their exact pixel geometry
 *  - the widget realizes and destroys cleanly
 */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/register_wb.h>
#include <xtcw/WAudioChassis.h>
#include <xtcw/Wlabel.h>
#include "mls.h"
#include "conststr.h"
#include "theme.h"
#include <stdio.h>

#define CH_W 1160
#define CH_H 770
#define NKIDS 50

int main(int argc, char **argv)
{
    m_init();
    conststr_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestMp3Chassis", NULL, 0,
                                   &argc, argv, NULL,
                                   sessionShellWidgetClass, NULL, 0);
    XtcwRegister(app);

    Widget chassis = XtVaCreateManagedWidget("chassis",
        wAudioChassisWidgetClass, top,
        XtNwidth, CH_W,
        XtNheight, CH_H,
        XtNchassisWidth, CH_W,
        XtNchassisHeight, CH_H,
        XtNtitle, "chassis test",
        NULL);

    Widget kids[NKIDS];
    int i;
    for (i = 0; i < NKIDS; i++) {
        char name[32];
        snprintf(name, sizeof name, "k%d", i);
        kids[i] = XtVaCreateManagedWidget(name, wlabelWidgetClass, chassis,
            XtNlabel, "x",
            XtNx, (Position)((i % 10) * 100),
            XtNy, (Position)((i / 10) * 100),
            XtNwidth, 80,
            XtNheight, 20,
            NULL);
    }

    XtRealizeWidget(top);

    int fail = 0;
    Dimension w = 0, h = 0;
    XtVaGetValues(chassis, XtNwidth, &w, XtNheight, &h, NULL);
    if (w != CH_W || h != CH_H) {
        printf("FAIL: chassis size %ux%u, expected %dx%d\n", w, h, CH_W, CH_H);
        fail = 1;
    }

    for (i = 0; i < NKIDS; i++) {
        Position x = -1, y = -1;
        Dimension cw = 0, chh = 0;
        int ex = (i % 10) * 100;
        int ey = (i / 10) * 100;
        XtVaGetValues(kids[i], XtNx, &x, XtNy, &y,
                      XtNwidth, &cw, XtNheight, &chh, NULL);
        if (x != ex || y != ey || cw != 80 || chh != 20) {
            printf("FAIL: child %d at %d,%d %ux%u, expected %d,%d 80x20\n",
                   i, x, y, cw, chh, ex, ey);
            fail = 1;
            break;
        }
    }

    /* A programmatic resize must be rejected. */
    XtVaSetValues(chassis, XtNwidth, 800, XtNheight, 600, NULL);
    XtVaGetValues(chassis, XtNwidth, &w, XtNheight, &h, NULL);
    if (w != CH_W || h != CH_H) {
        printf("FAIL: resize accepted, chassis is %ux%u\n", w, h);
        fail = 1;
    }

    if (!fail)
        printf("PASS: WAudioChassis fixed %dx%d, %d children pixel-exact\n",
               CH_W, CH_H, NKIDS);

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    theme_destroy();
    conststr_free();
    m_destruct();
    return fail ? 1 : 0;
}
