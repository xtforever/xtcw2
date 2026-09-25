/* xkey.c — synthetic keyboard input via the XTEST extension.
 *
 * Used by the KaroEd keystroke-injection test driver (test_karoed.c).
 * Link with -lXtst.
 */
#include "xkey.h"

#include <X11/extensions/XTest.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

KeyCode xkey_keycode(Display *dpy, KeySym keysym)
{
    return XKeysymToKeycode(dpy, keysym);
}

static void fake(Display *dpy, KeyCode kc, Bool down)
{
    if (kc == 0)
        return;
    XTestFakeKeyEvent(dpy, kc, down, 0);
    XFlush(dpy);
}

void xkey_press(Display *dpy, KeySym keysym, unsigned int mods)
{
    KeyCode kc = XKeysymToKeycode(dpy, keysym);
    KeyCode mod_kc[3];
    int nmods = 0;

    if (kc == 0)
        return;

    if (mods & ShiftMask)
        mod_kc[nmods++] = XKeysymToKeycode(dpy, XK_Shift_L);
    if (mods & ControlMask)
        mod_kc[nmods++] = XKeysymToKeycode(dpy, XK_Control_L);
    if (mods & Mod1Mask)
        mod_kc[nmods++] = XKeysymToKeycode(dpy, XK_Alt_L);

    for (int i = 0; i < nmods; i++)
        fake(dpy, mod_kc[i], True);

    fake(dpy, kc, True);
    fake(dpy, kc, False);

    for (int i = nmods - 1; i >= 0; i--)
        fake(dpy, mod_kc[i], False);
}

void xkey_type(Display *dpy, const char *ascii)
{
    const unsigned char *p = (const unsigned char *)ascii;

    for (; *p; p++) {
        char s[2] = { (char)*p, '\0' };
        KeySym ks = XStringToKeysym(s);
        KeyCode kc;
        unsigned int mods = 0;

        if (ks == NoSymbol)
            continue;

        kc = XKeysymToKeycode(dpy, ks);
        if (kc == 0)
            continue;

        /* If this keysym is not the unshifted level of its key, Shift it. */
        if (XkbKeycodeToKeysym(dpy, kc, 0, 0) != ks &&
            XkbKeycodeToKeysym(dpy, kc, 0, 1) == ks)
            mods = ShiftMask;

        xkey_press(dpy, ks, mods);
    }
}

int xkey_focus(Display *dpy, Window win)
{
    if (dpy == NULL || win == None)
        return -1;
    XSetInputFocus(dpy, win, RevertToParent, CurrentTime);
    XSync(dpy, False);
    return 0;
}
