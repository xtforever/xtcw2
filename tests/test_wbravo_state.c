/*
 * Wbravo full-state snapshots: save_state/load_state must reproduce the
 * document (atoms + looks), paragraph formats, editor resources, caret and
 * the complete undo/redo history in a second widget.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xft/Xft.h>
#include "mls.h"
#include "conststr.h"
#include "rtable.h"
#include "retex.h"
#include "Wbravo.h"
#include "WbravoP.h"

static Widget ed1, ed2;
static XtAppContext app;

static void pump(void)
{
    XSync(XtDisplay(ed1), False);
    while (XtAppPending(app))
        XtAppProcessEvent(app, XtIMAll);
}

static void act(Widget w, const char *name, String *params, Cardinal n)
{
    XtCallActionProc(w, (String)name, NULL, params, n);
    pump();
}

static void set_sel(Widget w, int caret, int anchor)
{
    ((WbravoWidget)w)->wbravo.caret_cp = caret;
    ((WbravoWidget)w)->wbravo.anchor_cp = anchor;
}

static char *get_text(Widget w)
{
    char *s = NULL;
    XtVaGetValues(w, XtNtext, &s, NULL);
    return s ? s : "";
}

static int files_equal(const char *a, const char *b)
{
    FILE *fa = fopen(a, "rb");
    FILE *fb = fopen(b, "rb");
    int eq = 1;
    if (!fa || !fb) {
        if (fa) fclose(fa);
        if (fb) fclose(fb);
        return 0;
    }
    for (;;) {
        int ca = fgetc(fa);
        int cb = fgetc(fb);
        if (ca != cb) { eq = 0; break; }
        if (ca == EOF) break;
    }
    fclose(fa);
    fclose(fb);
    return eq;
}

static void compare_atoms(Widget a, Widget b)
{
    RAtom *aa, *ab;
    int na = 0, nb = 0, i;
    aa = rtable_to_array((RTable *)((WbravoWidget)a)->wbravo.ptable, &na);
    ab = rtable_to_array((RTable *)((WbravoWidget)b)->wbravo.ptable, &nb);
    assert(na == nb);
    for (i = 0; i < na; i++) {
        assert(aa[i].kind == ab[i].kind);
        assert(aa[i].cp == ab[i].cp);
        assert(aa[i].look.style == ab[i].look.style);
        assert(fabs((double)aa[i].look.size - (double)ab[i].look.size) < 1e-6);
        assert(strcmp(m_str(aa[i].look.face), m_str(ab[i].look.face)) == 0);
        assert(strcmp(m_str(aa[i].payload), m_str(ab[i].payload)) == 0);
    }
    free(aa);
    free(ab);
}

static void compare_paragraphs(Widget a, Widget b)
{
    WbravoWidget wa = (WbravoWidget)a;
    WbravoWidget wb = (WbravoWidget)b;
    RetexParaFormat *pa = (RetexParaFormat *)wa->wbravo.para_fmt;
    RetexParaFormat *pb = (RetexParaFormat *)wb->wbravo.para_fmt;
    int i;

    assert(wa->wbravo.para_n == wb->wbravo.para_n);
    for (i = 0; i < wa->wbravo.para_n; i++) {
        assert(pa[i].align == pb[i].align);
        assert(fabs(pa[i].indent_left - pb[i].indent_left) < 1e-6);
        assert(fabs(pa[i].indent_right - pb[i].indent_right) < 1e-6);
        assert(fabs(pa[i].first_line - pb[i].first_line) < 1e-6);
        assert(fabs(pa[i].space_before - pb[i].space_before) < 1e-6);
        assert(fabs(pa[i].space_after - pb[i].space_after) < 1e-6);
    }
}

static void compare_editor(Widget a, Widget b)
{
    WbravoWidget wa = (WbravoWidget)a;
    WbravoWidget wb = (WbravoWidget)b;
    char *fa = NULL, *fb = NULL;

    if (strcmp(get_text(a), get_text(b)) != 0) {
        fprintf(stderr, "text mismatch:\n  ed1=[%s]\n  ed2=[%s]\n", get_text(a), get_text(b));
        fprintf(stderr, "para_n %d/%d undo %d/%d redo %d/%d\n",
                wa->wbravo.para_n, wb->wbravo.para_n,
                wa->wbravo.undo_n, wb->wbravo.undo_n,
                wa->wbravo.redo_n, wb->wbravo.redo_n);
    }
    assert(strcmp(get_text(a), get_text(b)) == 0);
    assert(wa->wbravo.caret_cp == wb->wbravo.caret_cp);
    assert(wa->wbravo.anchor_cp == wb->wbravo.anchor_cp);
    assert(wa->wbravo.scroll_line == wb->wbravo.scroll_line);
    assert(wa->wbravo.fontSize == wb->wbravo.fontSize);
    assert(wa->wbravo.alignment == wb->wbravo.alignment);
    assert(wa->wbravo.topGap == wb->wbravo.topGap);
    assert(wa->wbravo.bottomGap == wb->wbravo.bottomGap);
    assert(wa->wbravo.leftGap == wb->wbravo.leftGap);
    assert(wa->wbravo.rightGap == wb->wbravo.rightGap);
    assert(wa->wbravo.undo_n == wb->wbravo.undo_n);
    assert(wa->wbravo.redo_n == wb->wbravo.redo_n);
    assert(rtable_undo_depth((RTable *)wa->wbravo.ptable) ==
           rtable_undo_depth((RTable *)wb->wbravo.ptable));
    assert(rtable_redo_depth((RTable *)wa->wbravo.ptable) ==
           rtable_redo_depth((RTable *)wb->wbravo.ptable));
    assert(wa->wbravo.look.style == wb->wbravo.look.style);
    assert(fabs((double)wa->wbravo.look.size - (double)wb->wbravo.look.size) < 1e-6);
    assert(strcmp(m_str(wa->wbravo.look.face), m_str(wb->wbravo.look.face)) == 0);

    XtVaGetValues(a, XtNfontFace, &fa, NULL);
    XtVaGetValues(b, XtNfontFace, &fb, NULL);
    assert(fa && fb && strcmp(fa, fb) == 0);

    compare_paragraphs(a, b);
    compare_atoms(a, b);
}

int main(int argc, char **argv)
{
    Widget toplevel;
    String arg[2];
    const char *snap1 = "/tmp/test_wbravo_state.snap";
    const char *snap2 = "/tmp/test_wbravo_state2.snap";

    toplevel = XtVaAppInitialize(&app, "TestWbravoState", NULL, 0,
                                 &argc, argv, NULL,
                                 XtNwidth, 500, XtNheight, 400, NULL);

    ed1 = XtVaCreateManagedWidget("ed1", wbravoWidgetClass, toplevel,
                                  XtNwidth, 460, XtNheight, 320,
                                  XtNtext, "one\n\ntwo\n\nthree",
                                  XtNfontFace, "Sans",
                                  XtNfontSize, 16,
                                  NULL);
    XtRealizeWidget(toplevel);
    pump();

    /* Build a rich state: font, size, bold, per-paragraph alignment/indent. */
    act(ed1, "select_all", NULL, 0);
    arg[0] = (String)"Serif";
    act(ed1, "set_font", arg, 1);
    arg[0] = (String)"18";
    act(ed1, "set_size", arg, 1);
    act(ed1, "set_bold", NULL, 0);

    set_sel(ed1, 0, 0);
    arg[0] = (String)"center";
    act(ed1, "set_alignment", arg, 1);

    set_sel(ed1, 5, 5);       /* second paragraph */
    arg[0] = (String)"right";
    act(ed1, "set_alignment", arg, 1);
    arg[0] = (String)"12";
    act(ed1, "set_indent_left", arg, 1);

    set_sel(ed1, 10, 10);     /* third paragraph */
    arg[0] = (String)"6";
    act(ed1, "set_space_before", arg, 1);

    /* A few edits so there is real undo/redo history. */
    arg[0] = (String)"15";
    act(ed1, "set_caret", arg, 1);
    arg[0] = (String)"!";
    act(ed1, "insert_text", arg, 1);
    arg[0] = (String)"\n\nfour";
    act(ed1, "insert_text", arg, 1);
    set_sel(ed1, 0, 3);
    arg[0] = (String)"ONE";
    act(ed1, "insert_text", arg, 1);
    set_sel(ed1, 7, 9);       /* leave a non-empty selection */

    /* Create a redo entry as well. */
    act(ed1, "undo", NULL, 0);
    assert(((WbravoWidget)ed1)->wbravo.redo_n > 0);

    /* Save, then load into a fresh widget. */
    arg[0] = (String)snap1;
    act(ed1, "save_state", arg, 1);

    ed2 = XtVaCreateManagedWidget("ed2", wbravoWidgetClass, toplevel,
                                  XtNwidth, 460, XtNheight, 320,
                                  XtNtext, "",
                                  XtNfontFace, "Sans",
                                  XtNfontSize, 16,
                                  NULL);
    pump();
    act(ed2, "load_state", arg, 1);

    compare_editor(ed1, ed2);

    /* Resaving the restored state must produce a byte-identical snapshot. */
    arg[0] = (String)snap2;
    act(ed2, "save_state", arg, 1);
    assert(files_equal(snap1, snap2));

    /* The restored undo/redo history behaves like the original. */
    act(ed1, "redo", NULL, 0);
    act(ed2, "redo", NULL, 0);
    assert(strcmp(get_text(ed1), get_text(ed2)) == 0);
    compare_editor(ed1, ed2);

    act(ed1, "undo", NULL, 0);
    act(ed2, "undo", NULL, 0);
    assert(strcmp(get_text(ed1), get_text(ed2)) == 0);
    compare_editor(ed1, ed2);

    remove(snap1);
    remove(snap2);

    XtDestroyWidget(toplevel);
    printf("test_wbravo_state passed\n");
    return 0;
}
