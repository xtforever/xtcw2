/*
 * Phase 3: WYSIWYG style editing.  Exercises the Wbravo formatting actions
 * (bold/italic/roman, set_font, set_size) and verifies the resulting atom
 * looks in the rich document model (white-box via WbravoP.h).
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xft/Xft.h>
#include "rtable.h"
#include "conststr.h"
#include "retex.h"
#include "Wbravo.h"
#include "WbravoP.h"

static Widget ed;
static XtAppContext app;

static void pump(void)
{
    XSync(XtDisplay(ed), False);
    while (XtAppPending(app))
        XtAppProcessEvent(app, XtIMAll);
}

static void act(const char *name, String *params, Cardinal n)
{
    XtCallActionProc(ed, (String)name, NULL, params, n);
    pump();
}

static RAtom atom_at(int pos)
{
    RAtom a;
    RTable *t = (RTable *)((WbravoWidget)ed)->wbravo.ptable;
    assert(t != NULL);
    assert(rtable_get(t, pos, &a));
    return a;
}

static void set_sel(int caret, int anchor)
{
    ((WbravoWidget)ed)->wbravo.caret_cp = caret;
    ((WbravoWidget)ed)->wbravo.anchor_cp = anchor;
}

int main(int argc, char **argv)
{
    Widget toplevel;
    String ins[1], p[1];
    char *txt = NULL;

    toplevel = XtVaAppInitialize(&app, "TestRichWbravo", NULL, 0,
                                 &argc, argv, NULL,
                                 XtNwidth, 400, XtNheight, 220, NULL);
    ed = XtVaCreateManagedWidget("ed", wbravoWidgetClass, toplevel,
                                 XtNwidth, 380, XtNheight, 200,
                                 XtNtext, "hello",
                                 NULL);
    XtRealizeWidget(toplevel);
    pump();

    /* pending bold (no selection) -> typed text is bold and stays bold */
    act("bold", NULL, 0);
    ins[0] = (String)"X";
    act("insert_text", ins, 1);
    assert(atom_at(0).look.style == 1);
    ins[0] = (String)"Y";
    act("insert_text", ins, 1);
    assert(atom_at(1).look.style == 1);   /* typing continues the look */

    /* undo both inserts */
    act("undo", NULL, 0);
    act("undo", NULL, 0);
    txt = NULL;
    XtVaGetValues(ed, XtNtext, &txt, NULL);
    assert(txt && strcmp(txt, "hello") == 0);

    /* selection "ell" (1..4) italicized */
    set_sel(1, 4);
    act("italic", NULL, 0);
    assert(atom_at(1).look.style & 2);
    assert(atom_at(3).look.style & 2);
    assert((atom_at(0).look.style & 2) == 0);

    /* undo the italic change */
    act("undo", NULL, 0);
    assert((atom_at(1).look.style & 2) == 0);

    /* roman clears emphasis on a selection */
    set_sel(0, 5);
    act("bold", NULL, 0);
    assert(atom_at(0).look.style & 1);
    act("roman", NULL, 0);
    assert((atom_at(0).look.style & 3) == 0);

    /* set_font / set_size with a parameter */
    set_sel(0, 5);
    p[0] = (String)"Serif";
    act("set_font", p, 1);
    assert(atom_at(0).look.face == conststr_lookup_c("Serif"));

    p[0] = (String)"24";
    act("set_size", p, 1);
    assert(atom_at(0).look.size == 24.0f);
    assert(atom_at(4).look.size == 24.0f);

    /* ---- paragraph formatting ---------------------------------------- */
    XtVaSetValues(ed, XtNtext, (XtArgVal)"aaa\n\nbbb\n\nccc", NULL);
    pump();
    assert(((WbravoWidget)ed)->wbravo.para_n == 3);

    /* caret in the second paragraph (atom 5) -> centre it only */
    set_sel(5, 5);
    act("align_center", NULL, 0);
    {
        RetexParaFormat *pf = (RetexParaFormat *)((WbravoWidget)ed)->wbravo.para_fmt;
        assert(pf[0].align == RETEX_ALIGN_LEFT);
        assert(pf[1].align == RETEX_ALIGN_CENTER);
        assert(pf[2].align == RETEX_ALIGN_LEFT);
    }

    /* undo/redo carries the per-paragraph format along */
    act("undo", NULL, 0);
    {
        RetexParaFormat *pf = (RetexParaFormat *)((WbravoWidget)ed)->wbravo.para_fmt;
        assert(pf[1].align == RETEX_ALIGN_LEFT);
    }
    act("redo", NULL, 0);
    {
        RetexParaFormat *pf = (RetexParaFormat *)((WbravoWidget)ed)->wbravo.para_fmt;
        assert(pf[1].align == RETEX_ALIGN_CENTER);
    }

    /* indentation and spacing apply to the caret paragraph */
    set_sel(5, 5);
    act("indent", NULL, 0);
    p[0] = (String)"12";
    act("set_space_after", p, 1);
    {
        RetexParaFormat *pf = (RetexParaFormat *)((WbravoWidget)ed)->wbravo.para_fmt;
        assert(pf[1].indent_left >= 20.0);
        assert(pf[1].space_after == 12.0);
        assert(pf[0].indent_left == 0.0);
    }

    /* inserting a blank line keeps the format attached to 'bbb' */
    set_sel(3, 3);
    ins[0] = (String)"\n";
    act("insert_text", ins, 1);
    assert(((WbravoWidget)ed)->wbravo.para_n == 3);
    {
        RetexParaFormat *pf = (RetexParaFormat *)((WbravoWidget)ed)->wbravo.para_fmt;
        assert(pf[1].align == RETEX_ALIGN_CENTER);
        assert(pf[1].indent_left >= 20.0);
    }

    /* ---- external control interface: explicit style + paragraph setters -- */
    {
        String arg[1];

        arg[0] = (String)"one\n\ntwo";
        act("set_text", arg, 1);
        assert(((WbravoWidget)ed)->wbravo.para_n == 2);

        /* set_bold/set_italic are explicit (not toggles) on a selection */
        set_sel(0, 3);
        act("set_bold", NULL, 0);
        assert(atom_at(0).look.style & 1);
        act("set_bold", NULL, 0);          /* already bold, still bold */
        assert(atom_at(0).look.style & 1);
        arg[0] = (String)"0";
        act("set_bold", arg, 1);
        assert((atom_at(0).look.style & 1) == 0);
        act("set_italic", NULL, 0);
        assert(atom_at(0).look.style & 2);
        act("italic", NULL, 0);            /* toggle off */
        assert((atom_at(0).look.style & 2) == 0);

        /* set_alignment accepts names and numbers */
        set_sel(5, 5);                     /* second paragraph ("two") */
        arg[0] = (String)"right";
        act("set_alignment", arg, 1);
        {
            RetexParaFormat *pf = (RetexParaFormat *)((WbravoWidget)ed)->wbravo.para_fmt;
            assert(pf[1].align == RETEX_ALIGN_RIGHT);
            assert(pf[0].align == RETEX_ALIGN_LEFT);
        }
        arg[0] = (String)"1";              /* 1 = center */
        act("set_alignment", arg, 1);
        {
            RetexParaFormat *pf = (RetexParaFormat *)((WbravoWidget)ed)->wbravo.para_fmt;
            assert(pf[1].align == RETEX_ALIGN_CENTER);
        }

        /* absolute indents */
        arg[0] = (String)"36";
        act("set_indent_left", arg, 1);
        arg[0] = (String)"8";
        act("set_indent_right", arg, 1);
        arg[0] = (String)"12";
        act("set_first_line_indent", arg, 1);
        {
            RetexParaFormat *pf = (RetexParaFormat *)((WbravoWidget)ed)->wbravo.para_fmt;
            assert(pf[1].indent_left == 36.0);
            assert(pf[1].indent_right == 8.0);
            assert(pf[1].first_line == 12.0);
            assert(pf[0].indent_left == 0.0);
        }
    }

    XtDestroyWidget(toplevel);
    printf("test_rich_wbravo passed\n");
    return 0;
}
