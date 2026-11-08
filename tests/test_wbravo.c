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

    XtDestroyWidget(toplevel);
    printf("test_wbravo passed\n");
    return 0;
}
