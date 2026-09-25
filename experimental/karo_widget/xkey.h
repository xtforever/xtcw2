#ifndef XTCW_KAROED_XKEY_H
#define XTCW_KAROED_XKEY_H

#include <X11/Xlib.h>

/* Synthetic keyboard input via the XTEST extension, for headless tests.
 *
 * Requires the XTEST extension (present under Xvfb) and linking with -lXtst.
 */

/* Keycode for a keysym (0 if unmapped). */
KeyCode xkey_keycode(Display *dpy, KeySym keysym);

/* Press and release a single key. `mods` is a mask of X modifier bits
 * (ShiftMask, ControlMask, Mod1Mask), held down for the key press. */
void xkey_press(Display *dpy, KeySym keysym, unsigned int mods);

/* Type a NUL-terminated ASCII string, applying Shift when the keymap needs it. */
void xkey_type(Display *dpy, const char *ascii);

/* Give keyboard input focus to `win` and flush. Returns 0 on success. */
int xkey_focus(Display *dpy, Window win);

#endif /* XTCW_KAROED_XKEY_H */
