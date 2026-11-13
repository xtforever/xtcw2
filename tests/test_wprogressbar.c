/* Functional test for WProgressBar: geometry, value resource and fill pixels. */
#include <assert.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xft/Xft.h>
#include <xtcw/WProgressBar.h>
#include <xtcw/WProgressBarP.h>
#include <xtcw/register_wb.h>
#include "mls.h"

static unsigned long pixel_at(Widget w, int x, int y)
{
    XImage *img = XGetImage(XtDisplay(w), XtWindow(w), x, y, 1, 1,
                            AllPlanes, ZPixmap);
    assert(img != NULL);
    unsigned long px = XGetPixel(img, 0, 0);
    XDestroyImage(img);
    return px;
}

int main(int argc, char **argv)
{
    m_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWProgressBar", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    Widget pb = XtVaCreateManagedWidget("progress", wProgressBarWidgetClass, top,
                                        XtNwidth, (Dimension)200,
                                        "value", 50,
                                        "min", 0,
                                        "max", 100,
                                        "showText", False,
                                        NULL);
    XtRealizeWidget(top);
    XSync(XtDisplay(pb), False);

    WProgressBarWidget pw = (WProgressBarWidget)pb;
    assert(pw->core.width == 200);
    assert(pw->core.height >= 8);
    assert(pw->wProgressBar.value == 50);
    assert(pixel_at(pb, 40, 2) == pw->wProgressBar.fill);
    assert(pixel_at(pb, 160, 2) == pw->wProgressBar.bg);

    XtVaSetValues(pb, "value", 100, NULL);
    XSync(XtDisplay(pb), False);
    assert(pixel_at(pb, 160, 2) == pw->wProgressBar.fill);

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_destruct();

    printf("test_wprogressbar: PASS\n");
    return 0;
}
