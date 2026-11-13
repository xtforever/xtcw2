/* Functional test for WMenuBar + WmenuButton + WPopupMenu.
 * Verifies natural-width menu triggers, horizontal layout and popup open/close. */
#include <assert.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xft/Xft.h>
#include <xtcw/WMenuBar.h>
#include <xtcw/WmenuButton.h>
#include <xtcw/WmenuButtonP.h>
#include <xtcw/WPopupMenu.h>
#include <xtcw/Wbutton.h>
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
    Widget top = XtOpenApplication(&app, "TestWMenuBar", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    Widget bar = XtVaCreateManagedWidget("bar", wMenuBarWidgetClass, top,
                                         XtNwidth, (Dimension)400,
                                         XtNheight, (Dimension)30,
                                         "barHeight", 30,
                                         NULL);

    Widget file = XtVaCreateManagedWidget("file", wmenuButtonWidgetClass, bar,
                                          "label", "File", NULL);
    XtVaSetValues(file, XtNbuttonWidth, (Dimension)64,
                        XtNbuttonHeight, (Dimension)30, NULL);
    Widget file_menu = XtVaCreateManagedWidget("file_menu", wPopupMenuWidgetClass,
                                               file, NULL);
    XtVaCreateManagedWidget("open", wbuttonWidgetClass, file_menu,
                            XtNlabel, "Open", NULL);

    Widget view = XtVaCreateManagedWidget("view", wmenuButtonWidgetClass, bar,
                                          "label", "View", NULL);
    XtVaSetValues(view, XtNbuttonWidth, (Dimension)64,
                        XtNbuttonHeight, (Dimension)30, NULL);
    Widget view_menu = XtVaCreateManagedWidget("view_menu", wPopupMenuWidgetClass,
                                               view, NULL);
    XtVaCreateManagedWidget("zoom", wbuttonWidgetClass, view_menu,
                            XtNlabel, "Zoom", NULL);

    XtRealizeWidget(top);

    int fx, fy, fw, fh;
    int vx, vy, vw, vh;
    geom(file, &fx, &fy, &fw, &fh);
    geom(view, &vx, &vy, &vw, &vh);

    assert(fx == 0 && fy == 0);
    assert(fw == 64 && fh == 30);
    assert(vx == fw + 2);
    assert(vw == 64 && vh == 30);

    {
        WmenuButtonWidget mb = (WmenuButtonWidget)file;
        assert(mb->wmenuButton.label_font != NULL);
        assert(mb->wmenuButton.label_font->ascent +
               mb->wmenuButton.label_font->descent > 0);
    }

    assert(!wmenubutton_is_open(file));
    wmenubutton_popup(file);
    assert(wmenubutton_is_open(file));
    wmenubutton_popdown(file);
    assert(!wmenubutton_is_open(file));

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_destruct();

    printf("test_wmenubar: PASS\n");
    return 0;
}
