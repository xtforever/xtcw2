/* Regression test: WSpectrum and WVuMeter realize and expose their resources. */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/register_wb.h>
#include <xtcw/WAudioChassis.h>
#include <xtcw/WSpectrum.h>
#include <xtcw/WVuMeter.h>
#include <xtcw/WSeekBar.h>
#include <xtcw/WVfdDisplay.h>
#include <xtcw/WTransportButton.h>
#include <xtcw/WModulePanel.h>
#include "mls.h"
#include "conststr.h"
#include "theme.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    m_init();
    conststr_init();

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestMp3Widgets", NULL, 0,
                                   &argc, argv, NULL,
                                   sessionShellWidgetClass, NULL, 0);
    XtcwRegister(app);

    Widget chassis = XtVaCreateManagedWidget("chassis",
        wAudioChassisWidgetClass, top,
        XtNwidth, 800, XtNheight, 500,
        XtNchassisWidth, 800, XtNchassisHeight, 500,
        NULL);

    Widget spec = XtVaCreateManagedWidget("spec", wSpectrumWidgetClass, chassis,
        XtNx, 20, XtNy, 20, XtNwidth, 300, XtNheight, 220,
        XtNlevels, "-10,-20,-30,-40,-50",
        XtNbands, 24,
        NULL);

    Widget panel = XtVaCreateManagedWidget("panel", wModulePanelWidgetClass, chassis,
        XtNx, 20, XtNy, 380, XtNwidth, 400, XtNheight, 120,
        XtNcornerRadius, 12,
        NULL);

    Widget tbtn = XtVaCreateManagedWidget("tbtn", wTransportButtonWidgetClass, chassis,
        XtNx, 20, XtNy, 200, XtNwidth, 120, XtNheight, 56,
        XtNicon, "play", XtNlabel, "PLAY/PAUSE",
        NULL);

    Widget vfd = XtVaCreateManagedWidget("vfd", wVfdDisplayWidgetClass, chassis,
        XtNx, 20, XtNy, 20, XtNwidth, 600, XtNheight, 160,
        XtNalbum, "Album", XtNtitle, "Title",
        XtNplayState, "PLAYING",
        NULL);

    Widget seek = XtVaCreateManagedWidget("seek", wSeekBarWidgetClass, chassis,
        XtNx, 20, XtNy, 360, XtNwidth, 500, XtNheight, 52,
        XtNpositionMs, 120000, XtNdurationMs, 300000,
        NULL);

    Widget vu = XtVaCreateManagedWidget("vu", wVuMeterWidgetClass, chassis,
        XtNx, 20, XtNy, 260, XtNwidth, 400, XtNheight, 84,
        XtNlabel, "LEFT",
        XtNlevel, 0,
        XtNpeak, 0,
        XtNtransparent, True,
        NULL);

    XtRealizeWidget(top);

    int fail = 0;
    String levels = NULL, label = NULL;
    int bands = 0, level = 0, peak = 0;
    Dimension w = 0, h = 0;

    XtVaGetValues(spec, XtNlevels, &levels, XtNbands, &bands,
                  XtNwidth, &w, XtNheight, &h, NULL);
    if (!levels || strcmp(levels, "-10,-20,-30,-40,-50") != 0) {
        printf("FAIL: spectrum levels\n"); fail = 1;
    }
    if (bands != 24 || w != 300 || h != 220) {
        printf("FAIL: spectrum geometry/bands\n"); fail = 1;
    }

    XtVaGetValues(vu, XtNlabel, &label, XtNlevel, &level, XtNpeak, &peak,
                  XtNwidth, &w, XtNheight, &h, NULL);
    if (!label || strcmp(label, "LEFT") != 0) {
        printf("FAIL: vu label\n"); fail = 1;
    }
    Boolean transparent = False;
    XtVaGetValues(vu, XtNtransparent, &transparent, NULL);
    if (level != 0 || peak != 0 || w != 400 || h != 84 || !transparent) {
        printf("FAIL: vu geometry/values\n"); fail = 1;
    }

    int pos = 0, dur = 0, scorner = 0;
    XtVaGetValues(seek, XtNpositionMs, &pos, XtNdurationMs, &dur,
                  XtNcornerRadius, &scorner,
                  XtNwidth, &w, XtNheight, &h, NULL);
    if (pos != 120000 || dur != 300000 || scorner != 8 || w != 500 || h != 52) {
        printf("FAIL: seek geometry/values\n"); fail = 1;
    }

    String album = NULL, title = NULL, pstate = NULL;
    XtVaGetValues(vfd, XtNalbum, &album, XtNtitle, &title,
                  XtNplayState, &pstate, XtNwidth, &w, XtNheight, &h, NULL);
    if (!album || strcmp(album, "Album") != 0 ||
        !title || strcmp(title, "Title") != 0 ||
        !pstate || strcmp(pstate, "PLAYING") != 0 ||
        w != 600 || h != 160) {
        printf("FAIL: vfd geometry/values\n"); fail = 1;
    }

    String ticon = NULL, tlabel = NULL;
    int tcorner = 0;
    XtVaGetValues(tbtn, XtNicon, &ticon, XtNlabel, &tlabel,
                  XtNcornerRadius, &tcorner,
                  XtNwidth, &w, XtNheight, &h, NULL);
    if (!ticon || strcmp(ticon, "play") != 0 ||
        !tlabel || strcmp(tlabel, "PLAY/PAUSE") != 0 ||
        tcorner != 8 || w != 120 || h != 56) {
        printf("FAIL: transport button geometry/values\n"); fail = 1;
    }

    int radius = 0;
    XtVaGetValues(panel, XtNcornerRadius, &radius, XtNwidth, &w, XtNheight, &h, NULL);
    if (radius != 12 || w != 400 || h != 120) {
        printf("FAIL: module panel geometry/values\n"); fail = 1;
    }

    if (!fail)
        printf("PASS: VFD, transport, panel, spectrum, VU and seek widgets realize\n");

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    theme_destroy();
    conststr_free();
    m_destruct();
    return fail ? 1 : 0;
}
