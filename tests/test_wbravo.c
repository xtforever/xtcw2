#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "Wbravo.h"

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

static void expect(const char *want)
{
    char *got = NULL;
    XtVaGetValues(ed, XtNtext, &got, NULL);
    if (!got || strcmp(got, want) != 0) {
        fprintf(stderr, "test_wbravo: expected [%s] got [%s]\n",
                want, got ? got : "(null)");
        assert(0);
    }
}

int main(int argc, char **argv)
{
    Widget toplevel;
    String ins[1];
    int i;

    toplevel = XtVaAppInitialize(&app, "TestWbravo", NULL, 0,
                                 &argc, argv, NULL,
                                 XtNwidth, 400, XtNheight, 220, NULL);

    ed = XtVaCreateManagedWidget("ed", wbravoWidgetClass, toplevel,
                                 XtNwidth, 380, XtNheight, 200,
                                 XtNtext, "abc",
                                 NULL);
    XtRealizeWidget(toplevel);
    pump();

    /* The realize method must select the events it relies on.  With an empty
     * mask (XtValueMask 0) the widget never receives KeyPress/ButtonPress and
     * never repaints on Expose -- i.e. "no keyboard, no screen update". */
    {
        XWindowAttributes wa;
        assert(XGetWindowAttributes(XtDisplay(ed), XtWindow(ed), &wa));
        assert(wa.your_event_mask & KeyPressMask);
        assert(wa.your_event_mask & ButtonPressMask);
        assert(wa.your_event_mask & ExposureMask);
    }

    expect("abc");

    /* insert at caret 0 */
    ins[0] = (String)"X";
    act("insert_text", ins, 1);
    expect("Xabc");

    /* move right twice, insert */
    act("forward_char", NULL, 0);
    act("forward_char", NULL, 0);
    ins[0] = (String)"Y";
    act("insert_text", ins, 1);
    expect("XabYc");

    /* delete the Y (caret is just after it) */
    act("delete_backward", NULL, 0);
    expect("Xabc");

    /* undo restores the deletion, redo removes it again */
    act("undo", NULL, 0);
    expect("XabYc");
    act("redo", NULL, 0);
    expect("Xabc");

    /* move to end and append, then a newline */
    act("move_end", NULL, 0);
    ins[0] = (String)"Z";
    act("insert_text", ins, 1);
    expect("XabcZ");
    act("insert_newline", NULL, 0);
    expect("XabcZ\n");

    /* Unicode round-trip via the text resource */
    XtVaSetValues(ed, XtNtext, "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80", NULL);
    pump();
    expect("A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80");

    /* append a Unicode code point at the end */
    act("move_end", NULL, 0);
    ins[0] = (String)"\xF0\x9F\x98\x80"; /* 😀 */
    act("insert_text", ins, 1);
    expect("A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80\xF0\x9F\x98\x80");

    /* page up/down and vertical/horizontal movement should not crash */
    for (i = 0; i < 5; i++) {
        act("page_down", NULL, 0);
        act("move_down", NULL, 0);
        act("move_up", NULL, 0);
        act("move_home", NULL, 0);
        act("move_end", NULL, 0);
    }

    /* ---- external control interface (XtCallActionProc) --------------- */
    {
        String a1[1], sell[2];

        /* replace the document */
        a1[0] = (String)"hello world";
        act("set_text", a1, 1);
        expect("hello world");

        /* absolute caret (code points) */
        a1[0] = (String)"5";
        act("set_caret", a1, 1);
        ins[0] = (String)"!";
        act("insert_text", ins, 1);
        expect("hello! world");

        /* select all, then replace the selection */
        act("select_all", NULL, 0);
        ins[0] = (String)"Z";
        act("insert_text", ins, 1);
        expect("Z");

        /* explicit selection (anchor, caret) then delete it */
        a1[0] = (String)"abcdef";
        act("set_text", a1, 1);
        sell[0] = (String)"1";
        sell[1] = (String)"4";
        act("set_selection", sell, 2);
        act("delete_backward", NULL, 0);
        expect("aef");

        /* caret by UTF-8 byte offset with a multi-byte character */
        a1[0] = (String)"a\xC3\xA9";
        act("set_text", a1, 1);
        a1[0] = (String)"1";       /* just after 'a' */
        act("set_caret_byte", a1, 1);
        ins[0] = (String)"X";
        act("insert_text", ins, 1);
        expect("aX\xC3\xA9");

        /* caret by code point at the end */
        a1[0] = (String)"3";
        act("set_caret", a1, 1);
        ins[0] = (String)"Y";
        act("insert_text", ins, 1);
        expect("aX\xC3\xA9Y");

    }

    XtDestroyWidget(toplevel);
    printf("test_wbravo passed\n");
    return 0;
}
