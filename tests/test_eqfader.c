/* Regression test: EqFader has a separate, smaller font for the dB scale
 * labels (scaleFont), independent of the frequency label (xftFont). */
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xft/Xft.h>
#include <cairo.h>
#include <xtcw/EqFader.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"
#include "theme.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    m_init();
    conststr_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestEqFader", NULL, 0, &argc, argv,
                                   NULL, sessionShellWidgetClass, NULL, 0);
    XtcwRegister(app);

    Widget ef = XtVaCreateManagedWidget("ef", eqFaderWidgetClass, top,
        XtNlabel, "1 kHz",
        XtVaTypedArg, XtNxftFont, XtRString, "Sans-10",
        (int)sizeof("Sans-10"),
        XtVaTypedArg, XtNscaleFont, XtRString, "Sans-8",
        (int)sizeof("Sans-8"),
        XtNwidth, 64,
        XtNheight, 240,
        NULL);

    XtRealizeWidget(top);

    XftFont *mainf = NULL;
    XftFont *scale = NULL;
    XtVaGetValues(ef, XtNxftFont, &mainf, XtNscaleFont, &scale, NULL);

    int ok = mainf && scale && mainf != scale && scale->height < mainf->height;
    printf("%s: EqFader scaleFont (scale %dpx < label %dpx)\n",
           ok ? "PASS" : "FAIL",
           scale ? scale->height : -1,
           mainf ? mainf->height : -1);

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    theme_destroy();
    conststr_free();
    m_destruct();
    return ok ? 0 : 1;
}
