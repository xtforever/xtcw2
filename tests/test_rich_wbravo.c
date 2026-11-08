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

    XtDestroyWidget(toplevel);
    printf("test_rich_wbravo passed\n");
    return 0;
}
