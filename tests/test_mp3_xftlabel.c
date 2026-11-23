/* Regression test: WXftLabel is a minimal Xft label with a parent-relative
 * transparent window.  It must realize inside WAudioChassis without an
 * opaque background. */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xft/Xft.h>
#include <xtcw/register_wb.h>
#include <xtcw/WAudioChassis.h>
#include <xtcw/WXftLabel.h>
#include "mls.h"
#include "conststr.h"
#include "theme.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    m_init();
    conststr_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestMp3XftLabel", NULL, 0,
                                   &argc, argv, NULL,
                                   sessionShellWidgetClass, NULL, 0);
    XtcwRegister(app);

    Widget chassis = XtVaCreateManagedWidget("chassis",
        wAudioChassisWidgetClass, top,
        XtNwidth, 600, XtNheight, 300,
        XtNchassisWidth, 600, XtNchassisHeight, 300,
        NULL);

    Widget label = XtVaCreateManagedWidget("xftlabel",
        wXftLabelWidgetClass, chassis,
        XtNlabel, "transparent Xft label",
        XtNx, 40, XtNy, 60, XtNwidth, 400, XtNheight, 44,
        XtVaTypedArg, XtNxftFont, XtRString, "DejaVu Sans-22",
        (int)sizeof("DejaVu Sans-22"),
        XtNfg_norm, 0xffcc66,
        NULL);

    XtRealizeWidget(top);

    int fail = 0;
    String text = NULL;
    XftFont *font = NULL;
    Pixmap bg = None;
    Dimension w = 0, h = 0;

    XtVaGetValues(label, XtNlabel, &text, XtNxftFont, &font,
                  XtNbackgroundPixmap, &bg,
                  XtNwidth, &w, XtNheight, &h, NULL);

    if (!text || strcmp(text, "transparent Xft label") != 0) {
        printf("FAIL: label text\n");
        fail = 1;
    }
    if (!font) {
        printf("FAIL: xft font not opened\n");
        fail = 1;
    }
    if (bg != ParentRelative) {
        printf("FAIL: backgroundPixmap is not ParentRelative\n");
        fail = 1;
    }
    if (w != 400 || h != 44) {
        printf("FAIL: label geometry %ux%u\n", w, h);
        fail = 1;
    }

    if (!fail)
        printf("PASS: WXftLabel Xft text, ParentRelative transparent window\n");

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    theme_destroy();
    conststr_free();
    m_destruct();
    return fail ? 1 : 0;
}
