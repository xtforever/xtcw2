#ifndef XTCW_TOOLTIP_H
#define XTCW_TOOLTIP_H

/*
 * Hover tooltip manager.
 *
 * Widgets expose a `tooltip` string resource; their set_values() calls
 * wtooltip_set() and destroy() calls wtooltip_unregister(). The manager adds
 * Enter/Leave handlers, waits `delay` milliseconds and then asks the
 * WToolTip presenter to show the text at the pointer position.
 *
 * A presenter is created lazily on first use (wtooltip_create) and registers
 * itself via wtooltip_set_presenter(). Apps that want to configure delay or
 * placement can create a (WToolTip) in LUI; the last instance wins.
 */
#include <X11/Intrinsic.h>

#ifdef __cplusplus
extern "C" {
#endif

void wtooltip_set(Widget w, const char *text);
void wtooltip_unregister(Widget w);

/* Global defaults; a WToolTip widget overrides them while it exists. */
void wtooltip_configure(int delay_ms, int enabled);
int  wtooltip_delay(void);

/* Presenter hookup (called by WToolTip.widget). */
void wtooltip_set_presenter(Widget w);
void wtooltip_clear_presenter(Widget w);
Widget wtooltip_presenter(void);

/* Implemented by WToolTip.widget. */
Widget wtooltip_create(Widget parent);
void wtooltip_present(Widget w, const char *text, int root_x, int root_y);
void wtooltip_dismiss(Widget w);

/* Test/diagnostic helpers. */
void wtooltip_hide(void);
int  wtooltip_visible(void);
int  wtooltip_count(void);

#ifdef __cplusplus
}
#endif

#endif /* XTCW_TOOLTIP_H */
