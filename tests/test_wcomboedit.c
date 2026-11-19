/* Functional test for WComboBoxEdit: editable text + filtered dropdown. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/keysym.h>
#include <X11/Xft/Xft.h>
#include <xtcw/WComboBoxEdit.h>
#include <xtcw/WComboBoxEditP.h>
#include <xtcw/WListViewP.h>
#include <xtcw/WeditP.h>
#include <xtcw/register_wb.h>
#include "mls.h"

static XtAppContext app;
static Widget top;
static int fired = -99;
static int changed_count = 0;

static void pump(void)
{
    XSync(XtDisplay(top), False);
    while (XtAppPending(app))
        XtAppProcessEvent(app, XtIMAll);
}

static void send_keycode_state(Widget w, KeyCode kc, unsigned int state)
{
    Display *dpy = XtDisplay(w);
    XEvent ev;

    memset(&ev, 0, sizeof ev);
    ev.xkey.type = KeyPress;
    ev.xkey.display = dpy;
    ev.xkey.window = XtWindow(w);
    ev.xkey.root = DefaultRootWindow(dpy);
    ev.xkey.time = CurrentTime;
    ev.xkey.x = 1;
    ev.xkey.y = 1;
    ev.xkey.x_root = 1;
    ev.xkey.y_root = 1;
    ev.xkey.same_screen = True;
    ev.xkey.keycode = kc;
    ev.xkey.state = state;
    XtDispatchEventToWidget(w, &ev);
    pump();
}

static void send_keycode(Widget w, KeyCode kc)
{
    send_keycode_state(w, kc, 0);
}

static void send_key(Widget w, KeySym ks)
{
    send_keycode(w, XKeysymToKeycode(XtDisplay(w), ks));
}

static void send_key_state(Widget w, KeySym ks, unsigned int state)
{
    send_keycode_state(w, XKeysymToKeycode(XtDisplay(w), ks), state);
}

static void send_button(Widget w, int type, int x, int y)
{
    Display *dpy = XtDisplay(w);
    XEvent ev;

    memset(&ev, 0, sizeof ev);
    ev.xbutton.type = type;
    ev.xbutton.display = dpy;
    ev.xbutton.window = XtWindow(w);
    ev.xbutton.root = DefaultRootWindow(dpy);
    ev.xbutton.time = CurrentTime;
    ev.xbutton.x = x;
    ev.xbutton.y = y;
    ev.xbutton.x_root = x;
    ev.xbutton.y_root = y;
    ev.xbutton.same_screen = True;
    ev.xbutton.button = Button1;
    ev.xbutton.state = (type == MotionNotify) ? Button1Mask : 0;
    XtDispatchEventToWidget(w, &ev);
    pump();
}

static Pixel pixel_at(Widget w, int x, int y)
{
    Display *dpy = XtDisplay(w);
    XImage *im = XGetImage(dpy, XtWindow(w), x, y, 1, 1, AllPlanes, ZPixmap);
    assert(im != NULL);
    Pixel p = XGetPixel(im, 0, 0);
    XDestroyImage(im);
    return p;
}

static void pump_for(int ms)
{
    int i;
    for (i = 0; i < ms; i += 10) {
        XSync(XtDisplay(top), False);
        while (XtAppPending(app))
            XtAppProcessEvent(app, XtIMAll);
        usleep(10000);
    }
}

/* Xvfb's default keymap has no diaeresis keys; map a spare keycode. */
static KeyCode map_keysym(Widget w, KeySym ks)
{
    Display *dpy = XtDisplay(w);
    KeyCode kc = XKeysymToKeycode(dpy, ks);
    int min_kc = 0, max_kc = 0;
    if (kc != 0) return kc;
    XDisplayKeycodes(dpy, &min_kc, &max_kc);
    (void)min_kc;
    kc = (KeyCode)max_kc;
    XChangeKeyboardMapping(dpy, kc, 1, &ks, 1);
    XSync(dpy, False);
    return kc;
}

static void on_combo(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd;
    fired = (int)(intptr_t)cb;
}

static void on_changed(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd; (void)cb;
    changed_count++;
}

static int list_int(Widget list, const char *name)
{
    int v = -1;
    XtVaGetValues(list, name, &v, NULL);
    return v;
}

int main(int argc, char **argv)
{
    int items, keys, uitems = 0, ukeys = 0;
    char *a = "Main";
    char *b = "Mango";
    char *c = "Starter";
    char *d = "Dessert";
    char *ka = "key-Main";
    char *kb = "key-Mango";
    char *kc = "key-Starter";
    char *kd = "key-Dessert";
    const char *label = NULL;
    const char *skey = NULL;
    WComboBoxEditWidget cw;

    m_init();

    items = m_create(10, sizeof(char *));
    m_put(items, &a);
    m_put(items, &b);
    m_put(items, &c);
    m_put(items, &d);

    keys = m_create(10, sizeof(char *));
    m_put(keys, &ka);
    m_put(keys, &kb);
    m_put(keys, &kc);
    m_put(keys, &kd);

    top = XtVaAppInitialize(&app, "TestWComboBoxEdit", NULL, 0,
                            &argc, argv, NULL,
                            XtNwidth, 400, XtNheight, 200, NULL);
    XtcwRegister(app);

    Widget combo = XtVaCreateManagedWidget("combo", wComboBoxEditWidgetClass, top,
                                           "items", items,
                                           "keys", keys,
                                           "text", "",
                                           "placeholder", "Type...",
                                           "linesHeight", 8,
                                           NULL);
    XtAddCallback(combo, "callback", on_combo, NULL);
    XtAddCallback(combo, "changed", on_changed, NULL);
    XtRealizeWidget(top);
    pump();

    cw = (WComboBoxEditWidget)combo;
    WeditWidget ew = (WeditWidget)cw->wComboBoxEdit.edit;
    assert(cw->wComboBoxEdit.edit != NULL);
    XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
    assert(label && strcmp(label, "") == 0);
    assert(cw->wComboBoxEdit.selected == -1);

    /* Open the full list. */
    XtCallActionProc(combo, "open_popup", NULL, NULL, 0);
    pump();
    assert(cw->wComboBoxEdit.entry_mode);
    assert(list_int(cw->wComboBoxEdit.list, "rowCount") == 4);
    assert(list_int(cw->wComboBoxEdit.list, "visibleRows") == 4);

    /* Typing filters the displayed list by the typed prefix. */
    send_key(cw->wComboBoxEdit.edit, XK_m);
    send_key(cw->wComboBoxEdit.edit, XK_a);
    assert(changed_count == 2);
    XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
    assert(label && strcmp(label, "Main") == 0);
    assert(ew->wedit.has_selection);
    assert(ew->wedit.sel_start == 2);
    assert(ew->wedit.sel_end == 4);
    assert(cw->wComboBoxEdit.selected == -1);
    assert(list_int(cw->wComboBoxEdit.list, "rowCount") == 2);
    assert(list_int(cw->wComboBoxEdit.list, "selected") == 0);

    /* Return commits the first filtered item and closes the popup. */
    send_key(cw->wComboBoxEdit.edit, XK_Return);
    assert(!cw->wComboBoxEdit.entry_mode);
    assert(cw->wComboBoxEdit.selected == 0);
    assert(fired == 0);
    XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
    assert(label && strcmp(label, "Main") == 0);
    XtVaGetValues(combo, "selectedKey", &skey, NULL);
    assert(skey && strcmp(skey, "key-Main") == 0);
    assert(ew->wedit.has_selection);
    assert(ew->wedit.sel_start == 0);
    assert(ew->wedit.sel_end == 4);

    /* A different prefix filters down to one item and commits it. */
    XtVaSetValues(cw->wComboBoxEdit.edit, XtNlabel, "", NULL);
    pump();
    XtCallActionProc(combo, "open_popup", NULL, NULL, 0);
    pump();
    assert(cw->wComboBoxEdit.entry_mode);
    send_key(cw->wComboBoxEdit.edit, XK_d);
    assert(list_int(cw->wComboBoxEdit.list, "rowCount") == 1);
    send_key(cw->wComboBoxEdit.edit, XK_Return);
    assert(!cw->wComboBoxEdit.entry_mode);
    assert(cw->wComboBoxEdit.selected == 3);
    assert(fired == 3);
    XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
    assert(label && strcmp(label, "Dessert") == 0);
    XtVaGetValues(combo, "selectedKey", &skey, NULL);
    assert(skey && strcmp(skey, "key-Dessert") == 0);

    /* Free text commits with selected == -1. */
    XtVaSetValues(cw->wComboBoxEdit.edit, XtNlabel, "Custom", NULL);
    pump();
    send_key(cw->wComboBoxEdit.edit, XK_Return);
    assert(cw->wComboBoxEdit.selected == -1);
    assert(fired == -1);
    XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
    assert(label && strcmp(label, "Custom") == 0);
    XtVaGetValues(combo, "selectedKey", &skey, NULL);
    assert(skey && strcmp(skey, "Custom") == 0);

    /* Escape closes without committing. */
    XtCallActionProc(combo, "open_popup", NULL, NULL, 0);
    pump();
    assert(cw->wComboBoxEdit.entry_mode);
    send_key(cw->wComboBoxEdit.edit, XK_Escape);
    assert(!cw->wComboBoxEdit.entry_mode);

    /* Down opens the list already filtered by the current text. */
    XtVaSetValues(cw->wComboBoxEdit.edit, XtNlabel, "s", NULL);
    pump();
    send_key(cw->wComboBoxEdit.edit, XK_Down);
    assert(cw->wComboBoxEdit.entry_mode);
    assert(list_int(cw->wComboBoxEdit.list, "rowCount") == 1);
    send_key(cw->wComboBoxEdit.edit, XK_Escape);
    assert(!cw->wComboBoxEdit.entry_mode);

    /* Type a prefix while the popup is closed, then Down, Down, Return. */
    XtVaSetValues(cw->wComboBoxEdit.edit, XtNlabel, "", NULL);
    pump();
    send_key(cw->wComboBoxEdit.edit, XK_m);
    assert(!cw->wComboBoxEdit.entry_mode);
    send_key(cw->wComboBoxEdit.edit, XK_Down);
    assert(cw->wComboBoxEdit.entry_mode);
    assert(list_int(cw->wComboBoxEdit.list, "rowCount") == 2);
    assert(list_int(cw->wComboBoxEdit.list, "selected") == 0);
    assert(list_int(cw->wComboBoxEdit.list, "selectedId") == 0);
    send_key(cw->wComboBoxEdit.edit, XK_Down);
    assert(list_int(cw->wComboBoxEdit.list, "selected") == 1);
    assert(list_int(cw->wComboBoxEdit.list, "selectedId") == 1);
    send_key(cw->wComboBoxEdit.edit, XK_Return);
    assert(!cw->wComboBoxEdit.entry_mode);
    assert(cw->wComboBoxEdit.selected == 1);
    assert(fired == 1);
    XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
    assert(label && strcmp(label, "Mango") == 0);

    /* Same sequence with the popup already open before typing. */
    XtVaSetValues(cw->wComboBoxEdit.edit, XtNlabel, "", NULL);
    pump();
    XtCallActionProc(combo, "open_popup", NULL, NULL, 0);
    pump();
    assert(cw->wComboBoxEdit.entry_mode);
    send_key(cw->wComboBoxEdit.edit, XK_m);
    assert(list_int(cw->wComboBoxEdit.list, "rowCount") == 2);
    assert(list_int(cw->wComboBoxEdit.list, "selected") == 0);
    assert(list_int(cw->wComboBoxEdit.list, "selectedId") == 0);
    send_key(cw->wComboBoxEdit.edit, XK_Down);
    assert(list_int(cw->wComboBoxEdit.list, "selected") == 1);
    assert(list_int(cw->wComboBoxEdit.list, "selectedId") == 1);
    send_key(cw->wComboBoxEdit.edit, XK_Return);
    assert(!cw->wComboBoxEdit.entry_mode);
    assert(cw->wComboBoxEdit.selected == 1);
    assert(fired == 1);
    XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
    assert(label && strcmp(label, "Mango") == 0);

    /* A large list must stay a bounded, scrollable popup. */
    {
        enum { N = 500 };
        static char bufs[N][16];
        static char *ptrs[N];
        int big = m_create(10, sizeof(char *));
        int i;
        Dimension sh = 0;
        for (i = 0; i < N; i++) {
            snprintf(bufs[i], sizeof bufs[i], "Item %d", i);
            ptrs[i] = bufs[i];
            m_put(big, &ptrs[i]);
        }
        XtVaSetValues(combo, "items", big, NULL);
        XtCallActionProc(combo, "open_popup", NULL, NULL, 0);
        pump();
        assert(cw->wComboBoxEdit.entry_mode);
        assert(list_int(cw->wComboBoxEdit.list, "rowCount") == N);
        assert(list_int(cw->wComboBoxEdit.list, "visibleRows") == 8);
        XtVaGetValues(cw->wComboBoxEdit.shell, XtNheight, &sh, NULL);
        assert(sh > 0 && sh < 500);
        XtCallActionProc(combo, "dismiss", NULL, NULL, 0);
        pump();
        XtVaSetValues(combo, "items", items, NULL);
        m_free(big);
    }

    /* Programmatic selection updates the edit text and fires the callback. */
    {
        String params[1] = { "2" };
        XtCallActionProc(combo, "select_index", NULL, params, 1);
        pump();
        assert(cw->wComboBoxEdit.selected == 2);
        assert(fired == 2);
        XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "Starter") == 0);
        XtVaGetValues(combo, "selectedKey", &skey, NULL);
        assert(skey && strcmp(skey, "key-Starter") == 0);
        assert(ew->wedit.has_selection);
        assert(ew->wedit.sel_start == 0);
        assert(ew->wedit.sel_end == 7);

        /* The clear button is drawn on the window, not into the text pixmap;
         * a click that repaints the field must not make it disappear. */
        {
            int cw_width = 18;
            int cx, cy;
            Pixel x_pixel, bg_pixel;
            XtVaGetValues(cw->wComboBoxEdit.edit, "clearWidth", &cw_width, NULL);
            cx = (int)cw->wComboBoxEdit.edit->core.width - cw_width / 2 - 2;
            cy = (int)cw->wComboBoxEdit.edit->core.height / 2;
            x_pixel = pixel_at(cw->wComboBoxEdit.edit, cx, cy);
            bg_pixel = pixel_at(cw->wComboBoxEdit.edit, cx, 2);
            assert(x_pixel != bg_pixel);
            send_button(cw->wComboBoxEdit.edit, ButtonPress, 2, cy);
            assert(pixel_at(cw->wComboBoxEdit.edit, cx, cy) == x_pixel);
        }
        /* The click above cleared the selection; restore it for the next
         * check (typing must replace the selected value). */
        XtCallActionProc(cw->wComboBoxEdit.edit, "select_all", NULL, NULL, 0);
        pump();

        /* The first typed character replaces the whole selected value. */
        send_key(cw->wComboBoxEdit.edit, XK_x);
        pump();
        XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "x") == 0);
        assert(cw->wComboBoxEdit.selected == -1);
    }

    /* UTF-8 input: an umlaut arrives from XLookupString as Latin-1 and must
     * be normalized to UTF-8 before it reaches the text buffer. */
    {
        char *u1 = "M\xc3\xbcller";
        char *u2 = "Mango";
        char *u3 = "\xc3\x9c" "ber";
        char *u4 = "Dessert";
        char *uk1 = "key-Mueller";
        char *uk2 = "key-Mango";
        char *uk3 = "key-Ueber";
        char *uk4 = "key-Dessert";
        KeyCode kc;
        uitems = m_create(10, sizeof(char *));
        ukeys = m_create(10, sizeof(char *));

        m_put(uitems, &u1);
        m_put(uitems, &u2);
        m_put(uitems, &u3);
        m_put(uitems, &u4);
        m_put(ukeys, &uk1);
        m_put(ukeys, &uk2);
        m_put(ukeys, &uk3);
        m_put(ukeys, &uk4);

        XtVaSetValues(combo, "items", uitems, "keys", ukeys,
                      "selected", -1, NULL);
        XtVaSetValues(cw->wComboBoxEdit.edit, XtNlabel, "", NULL);
        pump();
        XtCallActionProc(combo, "open_popup", NULL, NULL, 0);
        pump();
        assert(cw->wComboBoxEdit.entry_mode);

        send_key(cw->wComboBoxEdit.edit, XK_m);
        kc = map_keysym(cw->wComboBoxEdit.edit, XK_udiaeresis);
        send_keycode(cw->wComboBoxEdit.edit, kc);
        pump();

        XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "M\xc3\xbcller") == 0);
        {
            WeditWidget uew = (WeditWidget)cw->wComboBoxEdit.edit;
            assert(uew->wedit.has_selection);
            assert(uew->wedit.sel_start == 2);
            assert(uew->wedit.sel_end == 6);
        }
        assert(list_int(cw->wComboBoxEdit.list, "rowCount") == 1);

        send_key(cw->wComboBoxEdit.edit, XK_Return);
        pump();
        assert(!cw->wComboBoxEdit.entry_mode);
        assert(cw->wComboBoxEdit.selected == 0);
        XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "M\xc3\xbcller") == 0);
        XtVaGetValues(combo, "selectedKey", &skey, NULL);
        assert(skey && strcmp(skey, "key-Mueller") == 0);

        /* Unicode keysym fallback (CJK injected via XTEST). */
        XtVaSetValues(cw->wComboBoxEdit.edit, XtNlabel, "", NULL);
        pump();
        kc = map_keysym(cw->wComboBoxEdit.edit, (KeySym)0x01004e2d);
        send_keycode(cw->wComboBoxEdit.edit, kc);
        pump();
        XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "\xe4\xb8\xad") == 0);

        /* Small clear button: click the x area inside the edit. */
        XtCallActionProc(combo, "open_popup", NULL, NULL, 0);
        pump();
        assert(cw->wComboBoxEdit.entry_mode);
        {
            XButtonEvent cev;
            memset(&cev, 0, sizeof cev);
            cev.type = ButtonPress;
            cev.x = (int)cw->wComboBoxEdit.edit->core.width - 8;
            cev.y = 5;
            XtCallActionProc(cw->wComboBoxEdit.edit, "lineedit_click",
                             (XEvent *)&cev, NULL, 0);
            pump();
        }
        XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "") == 0);
        assert(cw->wComboBoxEdit.selected == -1);
        assert(cw->wComboBoxEdit.entry_mode);
        assert(list_int(cw->wComboBoxEdit.list, "rowCount") == 4);

        /* Ctrl+U clears the edit as well. */
        send_key(cw->wComboBoxEdit.edit, XK_m);
        assert(list_int(cw->wComboBoxEdit.list, "rowCount") == 2);
        send_keycode_state(cw->wComboBoxEdit.edit,
                           XKeysymToKeycode(XtDisplay(cw->wComboBoxEdit.edit),
                                            XK_u),
                           ControlMask);
        pump();
        XtVaGetValues(cw->wComboBoxEdit.edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "") == 0);
        assert(cw->wComboBoxEdit.selected == -1);
        assert(list_int(cw->wComboBoxEdit.list, "rowCount") == 4);
        XtCallActionProc(combo, "dismiss", NULL, NULL, 0);
        pump();
        assert(!cw->wComboBoxEdit.entry_mode);
    }

    /* Inline completion: type "ma" -> "Main" with "in" selected. */
    {
        Widget edit = cw->wComboBoxEdit.edit;
        WeditWidget ew = (WeditWidget)edit;

        XtVaSetValues(combo, "items", items, "keys", keys,
                      "selected", -1, NULL);
        XtCallActionProc(combo, "clear", NULL, NULL, 0);
        pump();

        send_key(edit, XK_m);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "m") == 0);
        assert(!ew->wedit.has_selection);

        send_key(edit, XK_a);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "Main") == 0);
        assert(ew->wedit.has_selection);
        assert(ew->wedit.sel_start == 2);
        assert(ew->wedit.sel_end == 4);

        send_key(edit, XK_n);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "Mango") == 0);
        assert(ew->wedit.has_selection);
        assert(ew->wedit.sel_start == 3);
        assert(ew->wedit.sel_end == 5);

        send_key(edit, XK_g);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "Mango") == 0);
        assert(ew->wedit.has_selection);
        assert(ew->wedit.sel_start == 4);
        assert(ew->wedit.sel_end == 5);

        send_key(edit, XK_o);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "Mango") == 0);
        assert(!ew->wedit.has_selection);
        assert(cw->wComboBoxEdit.selected == 1);
        XtVaGetValues(combo, "selectedKey", &skey, NULL);
        assert(skey && strcmp(skey, "key-Mango") == 0);

        /* Backspace removes the completion first, then the typed chars. */
        XtCallActionProc(combo, "clear", NULL, NULL, 0);
        pump();
        send_key(edit, XK_m);
        send_key(edit, XK_a);
        assert(ew->wedit.has_selection);
        send_key(edit, XK_BackSpace);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "ma") == 0);
        assert(!ew->wedit.has_selection);
        send_key(edit, XK_BackSpace);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "m") == 0);

        /* Right accepts the completion. */
        XtCallActionProc(combo, "clear", NULL, NULL, 0);
        pump();
        send_key(edit, XK_m);
        send_key(edit, XK_a);
        assert(ew->wedit.has_selection);
        send_key(edit, XK_Right);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "Main") == 0);
        assert(!ew->wedit.has_selection);

        /* Escape restores the typed prefix. */
        XtCallActionProc(combo, "clear", NULL, NULL, 0);
        pump();
        send_key(edit, XK_m);
        send_key(edit, XK_a);
        assert(ew->wedit.has_selection);
        send_key(edit, XK_Escape);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "ma") == 0);
        assert(!ew->wedit.has_selection);

        /* Clear then type "bn": regression for an empty label buffer. */
        {
            char *b1 = "bmi (db1)";
            char *b2 = "bna (db2)";
            char *b3 = "bni (db3)";
            int bitems = m_create(10, sizeof(char *));
            m_put(bitems, &b1);
            m_put(bitems, &b2);
            m_put(bitems, &b3);
            XtVaSetValues(combo, "items", bitems, "selected", -1, NULL);
            XtCallActionProc(combo, "clear", NULL, NULL, 0);
            pump();
            send_key(edit, XK_b);
            send_key(edit, XK_n);
            XtVaGetValues(edit, XtNlabel, &label, NULL);
            assert(label && strcmp(label, "bna (db2)") == 0);
            assert(ew->wedit.has_selection);
            XtCallActionProc(combo, "clear", NULL, NULL, 0);
            pump();
            XtVaSetValues(combo, "items", items, "keys", keys,
                          "selected", -1, NULL);
            m_free(bitems);
        }

        /* Popup must grow/shrink and stay anchored below the field when the
         * filtered list changes size. */
        {
            char *i1 = "alpha";
            char *i2 = "alpine";
            char *i3 = "beta";
            char *i4 = "score";
            int sitems = m_create(10, sizeof(char *));
            Dimension h1 = 0, h2 = 0, ch = 0;
            Position sx = 0, sy = 0;
            int rx = 0, ry = 0;
            Window child_win = None;

            m_put(sitems, &i1);
            m_put(sitems, &i2);
            m_put(sitems, &i3);
            m_put(sitems, &i4);
            XtVaSetValues(combo, "items", sitems, "selected", -1, NULL);
            XtCallActionProc(combo, "clear", NULL, NULL, 0);
            pump();
            XtCallActionProc(combo, "open_popup", NULL, NULL, 0);
            pump();
            send_key(edit, XK_s);          /* one match: score */
            XtVaGetValues(cw->wComboBoxEdit.shell, XtNheight, &h1, NULL);
            send_key(edit, XK_BackSpace);
            send_key(edit, XK_a);          /* two matches: alpha, alpine */
            XtVaGetValues(cw->wComboBoxEdit.shell, XtNheight, &h2, NULL);
            assert(h2 > h1);

            XtVaGetValues(combo, XtNheight, &ch, NULL);
            XTranslateCoordinates(XtDisplay(combo), XtWindow(combo),
                                  DefaultRootWindow(XtDisplay(combo)),
                                  0, (int)ch + 2, &rx, &ry, &child_win);
            XtVaGetValues(cw->wComboBoxEdit.shell, XtNx, &sx, XtNy, &sy, NULL);
            assert((int)sx == rx);
            assert((int)sy == ry);

            XtCallActionProc(combo, "dismiss", NULL, NULL, 0);
            pump();
            XtVaSetValues(combo, "items", items, "keys", keys,
                          "selected", -1, NULL);
            m_free(sitems);
        }

        /* Down opens the popup filtered by the typed prefix; Enter commits. */
        XtCallActionProc(combo, "clear", NULL, NULL, 0);
        pump();
        send_key(edit, XK_m);
        send_key(edit, XK_a);
        send_key(edit, XK_Down);
        assert(cw->wComboBoxEdit.entry_mode);
        assert(list_int(cw->wComboBoxEdit.list, "rowCount") == 2);
        send_key(edit, XK_Return);
        assert(!cw->wComboBoxEdit.entry_mode);
        assert(cw->wComboBoxEdit.selected == 0);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "Main") == 0);
    }

    /* Selection and clipboard. */
    {
        Widget edit = cw->wComboBoxEdit.edit;
        WeditWidget ew = (WeditWidget)edit;
        int x3 = 0, i;

        /* Ctrl+A, Ctrl+C, clear, Ctrl+V round trip. */
        XtVaSetValues(edit, XtNlabel, "abcdef", NULL);
        pump();
        send_key_state(edit, XK_a, ControlMask);
        pump();
        assert(ew->wedit.has_selection);
        send_key_state(edit, XK_c, ControlMask);
        pump_for(200);
        XtVaSetValues(edit, XtNlabel, "", NULL);
        pump();
        send_button(edit, ButtonPress, 0, 5);
        send_button(edit, ButtonRelease, 0, 5);
        send_key_state(edit, XK_v, ControlMask);
        pump_for(300);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "abcdef") == 0);

        /* Cut removes the selected text. */
        XtVaSetValues(edit, XtNlabel, "abcdef", NULL);
        pump();
        send_key_state(edit, XK_a, ControlMask);
        send_key_state(edit, XK_x, ControlMask);
        pump();
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "") == 0);
        send_key_state(edit, XK_v, ControlMask);
        pump_for(300);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "abcdef") == 0);

        /* Mouse drag selects "abc". */
        XtVaSetValues(edit, XtNlabel, "abcdef", NULL);
        pump();
        x3 = 0;
        for (i = 0; i < 3; i++) x3 += INT(ew->wedit.glyph_xoff_m, i);
        send_button(edit, ButtonPress, 0, 5);
        send_button(edit, MotionNotify, x3, 5);
        send_button(edit, ButtonRelease, x3, 5);
        assert(ew->wedit.has_selection);
        send_key_state(edit, XK_c, ControlMask);
        pump_for(200);
        XtVaSetValues(edit, XtNlabel, "", NULL);
        pump();
        send_button(edit, ButtonPress, 0, 5);
        send_button(edit, ButtonRelease, 0, 5);
        send_key_state(edit, XK_v, ControlMask);
        pump_for(300);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "abc") == 0);

        /* Shift+Right extends by one character. */
        XtVaSetValues(edit, XtNlabel, "abcdef", NULL);
        pump();
        send_button(edit, ButtonPress, 0, 5);
        send_button(edit, ButtonRelease, 0, 5);
        send_key_state(edit, XK_Right, ShiftMask);
        send_key_state(edit, XK_Right, ShiftMask);
        assert(ew->wedit.has_selection);
        send_key_state(edit, XK_c, ControlMask);
        pump_for(200);
        XtVaSetValues(edit, XtNlabel, "", NULL);
        pump();
        send_button(edit, ButtonPress, 0, 5);
        send_button(edit, ButtonRelease, 0, 5);
        send_key_state(edit, XK_v, ControlMask);
        pump_for(300);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "ab") == 0);

        /* Shift+End selects to the end; Home/End move the cursor. */
        XtVaSetValues(edit, XtNlabel, "abcdef", NULL);
        pump();
        send_button(edit, ButtonPress, 0, 5);
        send_button(edit, ButtonRelease, 0, 5);
        send_key_state(edit, XK_End, ShiftMask);
        assert(ew->wedit.has_selection);
        send_key_state(edit, XK_c, ControlMask);
        pump_for(200);
        XtVaSetValues(edit, XtNlabel, "", NULL);
        pump();
        send_button(edit, ButtonPress, 0, 5);
        send_button(edit, ButtonRelease, 0, 5);
        send_key_state(edit, XK_v, ControlMask);
        pump_for(300);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "abcdef") == 0);

        /* Single-character Backspace/Delete still update the glyph list. */
        XtVaSetValues(edit, XtNlabel, "abc", NULL);
        pump();
        send_key(edit, XK_End);
        send_key(edit, XK_BackSpace);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "ab") == 0);
        XtVaSetValues(edit, XtNlabel, "abc", NULL);
        pump();
        send_key(edit, XK_Home);
        send_key(edit, XK_Delete);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "bc") == 0);

        /* Backspace deletes the selection; typing replaces it. */
        XtVaSetValues(edit, XtNlabel, "abcdef", NULL);
        pump();
        send_key_state(edit, XK_a, ControlMask);
        send_key(edit, XK_BackSpace);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "") == 0);
        XtVaSetValues(edit, XtNlabel, "abcdef", NULL);
        pump();
        send_key_state(edit, XK_a, ControlMask);
        send_key(edit, XK_z);
        XtVaGetValues(edit, XtNlabel, &label, NULL);
        assert(label && strcmp(label, "z") == 0);
    }

    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    m_free(ukeys);
    m_free(uitems);
    m_free(keys);
    m_free(items);
    m_destruct();

    printf("test_wcomboedit: PASS\n");
    return 0;
}
