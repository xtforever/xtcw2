/*
 * Headless widget test for Wmarkdown: parse/render, resize and scrolling.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Xft/Xft.h>

#include "Wmarkdown.h"
#include "WmarkdownP.h"

static Widget md;
static XtAppContext app;

static void pump(void)
{
    XSync(XtDisplay(md), False);
    while (XtAppPending(app))
        XtAppProcessEvent(app, XtIMAll);
}

static void act(const char *name, String *params, Cardinal n)
{
    XtCallActionProc(md, (String)name, NULL, params, n);
    pump();
}

int main(int argc, char **argv)
{
    Widget toplevel;
    WmarkdownWidget m = NULL;
    String p[1];
    const char *long_text =
        "# Help\n"
        "\n"
        "This is a paragraph with **bold**, *italic* and `code` text. "
        "It should wrap over several lines when the viewer is narrow.\n"
        "\n"
        "- first bullet\n"
        "- second bullet\n"
        "\n"
        "1. ordered one\n"
        "2. ordered two\n"
        "\n"
        "> A quote that is indented.\n"
        "\n"
        "```\n"
        "int main(void) { return 0; }\n"
        "```\n"
        "\n"
        "Line A\n"
        "Line B\n"
        "Line C\n"
        "Line D\n"
        "Line E\n"
        "Line F\n"
        "Line G\n"
        "Line H\n"
        "Line I\n"
        "Line J\n";

    toplevel = XtVaAppInitialize(&app, "TestWmarkdown", NULL, 0,
                                 &argc, argv, NULL,
                                 XtNwidth, 420, XtNheight, 260, NULL);
    md = XtVaCreateManagedWidget("md", wmarkdownWidgetClass, toplevel,
                                 XtNwidth, 400, XtNheight, 240,
                                 XtNtext, "# Title\n\nHello **world**.",
                                 XtNuseThemeFont, False,
                                 XtNfontFace, "Sans",
                                 XtNfontSize, 13,
                                 NULL);
    XtRealizeWidget(toplevel);
    pump();

    m = (WmarkdownWidget)md;
    assert(m->wmarkdown.doc != NULL);
    assert(m->wmarkdown.para_ptr != NULL);
    assert(m->wmarkdown.lines_total > 0);
    assert(m->wmarkdown.cache_w > 0);
    assert(m->wmarkdown.cache_h > 0);

    /* Text replacement reparses and relayouts. */
    XtVaSetValues(md, XtNtext, long_text, NULL);
    pump();
    assert(m->wmarkdown.lines_total >= 10);

    /* Programmatic selection + clipboard text. */
    {
        String sel[2];
        sel[0] = (String)"6";
        sel[1] = (String)"10";
        act("set_selection", sel, 2);
        assert(m->wmarkdown.sel_anchor == 6);
        assert(m->wmarkdown.sel_caret == 10);
        act("copy_selection", NULL, 0);
        assert(m->wmarkdown.sel_len == 4);
        assert(strncmp(m->wmarkdown.sel_buf, "This", 4) == 0);

        act("select_all", NULL, 0);
        assert(m->wmarkdown.sel_anchor == 0);
        assert(m->wmarkdown.sel_caret == m->wmarkdown.flat_len);
        act("copy_selection", NULL, 0);
        assert(m->wmarkdown.sel_len == m->wmarkdown.flat_len);

        act("clear_selection", NULL, 0);
        assert(m->wmarkdown.sel_anchor == -1);
        assert(m->wmarkdown.sel_caret == -1);
    }

    /* Mouse drag selection: synthesize button/motion events. */
    {
        XEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.type = ButtonPress;
        ev.xbutton.x = m->wmarkdown.leftGap + 2;
        ev.xbutton.y = m->wmarkdown.topGap + 5;
        XtCallActionProc(md, (String)"select_start", &ev, NULL, 0);

        ev.type = MotionNotify;
        ev.xmotion.x = m->wmarkdown.leftGap + 40;
        ev.xmotion.y = m->wmarkdown.topGap + 5;
        XtCallActionProc(md, (String)"select_extend", &ev, NULL, 0);

        ev.type = ButtonRelease;
        XtCallActionProc(md, (String)"select_end", &ev, NULL, 0);
        pump();
        assert(m->wmarkdown.sel_anchor >= 0);
        assert(m->wmarkdown.sel_caret > m->wmarkdown.sel_anchor);
        act("copy_selection", NULL, 0);
        assert(m->wmarkdown.sel_len > 0);
        act("clear_selection", NULL, 0);
    }

    /* Scrolling is clamped to the line count. */
    p[0] = (String)"1";
    act("set_scroll", p, 1);
    assert(m->wmarkdown.scrollLine == 1);

    act("scroll_end", NULL, 0);
    assert(m->wmarkdown.scrollLine > 0);
    assert(m->wmarkdown.scrollLine < m->wmarkdown.lines_total);

    act("scroll_home", NULL, 0);
    assert(m->wmarkdown.scrollLine == 0);

    act("page_down", NULL, 0);
    assert(m->wmarkdown.scrollLine > 0);

    /* Resizing rebuilds the pixmap cache and relayouts. */
    XtResizeWidget(md, 300, 180, 0);
    pump();
    assert(m->wmarkdown.cache_w == 300);
    assert(m->wmarkdown.cache_h == 180);
    assert(m->wmarkdown.lines_total > 0);

    /* Empty input is valid and clears the layout. */
    XtVaSetValues(md, XtNtext, "", NULL);
    pump();
    assert(m->wmarkdown.doc == NULL);
    assert(m->wmarkdown.lines_total == 0);
    assert(m->wmarkdown.scrollLine == 0);

    printf("test_wmarkdown passed\n");
    return 0;
}
