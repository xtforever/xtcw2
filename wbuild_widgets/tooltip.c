/*
 * Hover tooltip manager (see tooltip.h).
 *
 * One event handler per widget with a `tooltip` resource. Enter starts a
 * timeout; Leave cancels it and dismisses the presenter. The timeout
 * re-checks the pointer with XQueryPointer so a fast mouse cannot leave a
 * stale tooltip behind.
 */
#include "tooltip.h"

#include <X11/IntrinsicP.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <string.h>

typedef struct tip_entry {
    Widget w;
    char *text;
    XtIntervalId timer;
    int active;
    struct tip_entry *next;
} tip_entry;

static tip_entry *tips;
static Widget presenter;
static int tip_delay = 600;
static int tip_enabled = 1;

static tip_entry *find_entry(Widget w)
{
    for (tip_entry *e = tips; e; e = e->next) {
        if (e->w == w) return e;
    }
    return NULL;
}

static void cancel_timer(tip_entry *e)
{
    if (e && e->timer) {
        XtRemoveTimeOut(e->timer);
        e->timer = 0;
    }
}

static void hide_presenter(void)
{
    if (presenter) wtooltip_dismiss(presenter);
}

static Widget toplevel_of(Widget w)
{
    Widget top = w;
    while (top && !XtIsShell(top)) top = XtParent(top);
    return top ? top : w;
}

static void ensure_presenter(Widget w)
{
    if (presenter) return;
    Widget top = toplevel_of(w);
    if (!top) return;
    /* WToolTip.initialize() calls wtooltip_set_presenter(). */
    wtooltip_create(top);
}

static void tip_timeout(XtPointer client, XtIntervalId *id)
{
    (void)id;
    tip_entry *e = (tip_entry *)client;
    if (!e || !e->w || !e->text || !e->text[0] || !tip_enabled) return;
    e->timer = 0;

    if (!XtIsRealized(e->w)) return;

    Window root_ret, child_ret;
    int root_x = 0, root_y = 0, win_x = 0, win_y = 0;
    unsigned int mask = 0;
    if (!XQueryPointer(XtDisplay(e->w), XtWindow(e->w), &root_ret, &child_ret,
                       &root_x, &root_y, &win_x, &win_y, &mask))
        return;

    int width = (int)e->w->core.width;
    int height = (int)e->w->core.height;
    if (win_x < 0 || win_y < 0 || win_x >= width || win_y >= height)
        return;

    ensure_presenter(e->w);
    if (!presenter) return;
    e->active = 1;
    wtooltip_present(presenter, e->text, root_x, root_y);
}

static void tip_event(Widget w, XtPointer client, XEvent *ev, Boolean *cont)
{
    (void)w;
    tip_entry *e = (tip_entry *)client;
    if (!e) return;
    if (cont) *cont = False;

    if (ev->type == EnterNotify) {
        if (!tip_enabled || !e->text || !e->text[0]) return;
        cancel_timer(e);
        e->timer = XtAppAddTimeOut(XtWidgetToApplicationContext(w),
                                   (unsigned long)(tip_delay > 0 ? tip_delay : 1),
                                   tip_timeout, e);
    } else if (ev->type == LeaveNotify) {
        cancel_timer(e);
        if (e->active) {
            e->active = 0;
            hide_presenter();
        }
    }
}

void wtooltip_set(Widget w, const char *text)
{
    if (!w) return;
    if (!text || !text[0]) {
        wtooltip_unregister(w);
        return;
    }

    tip_entry *e = find_entry(w);
    if (!e) {
        e = (tip_entry *)calloc(1, sizeof *e);
        if (!e) return;
        e->w = w;
        e->next = tips;
        tips = e;
        XtAddEventHandler(w, EnterWindowMask | LeaveWindowMask, False,
                          tip_event, (XtPointer)e);
    }
    free(e->text);
    e->text = strdup(text);
}

void wtooltip_unregister(Widget w)
{
    if (!w) return;
    tip_entry **link = &tips;
    while (*link) {
        tip_entry *e = *link;
        if (e->w == w) {
            cancel_timer(e);
            if (e->active) {
                e->active = 0;
                hide_presenter();
            }
            XtRemoveEventHandler(w, EnterWindowMask | LeaveWindowMask, False,
                                 tip_event, (XtPointer)e);
            *link = e->next;
            free(e->text);
            free(e);
            return;
        }
        link = &e->next;
    }
}

void wtooltip_configure(int delay_ms, int enabled)
{
    if (delay_ms > 0) tip_delay = delay_ms;
    tip_enabled = enabled ? 1 : 0;
    if (!tip_enabled) wtooltip_hide();
}

int wtooltip_delay(void)
{
    return tip_delay;
}

void wtooltip_set_presenter(Widget w)
{
    presenter = w;
}

void wtooltip_clear_presenter(Widget w)
{
    if (presenter == w) {
        presenter = NULL;
        wtooltip_hide();
    }
}

Widget wtooltip_presenter(void)
{
    return presenter;
}

void wtooltip_hide(void)
{
    if (presenter) wtooltip_dismiss(presenter);
}

int wtooltip_visible(void)
{
    if (!presenter) return 0;
    Boolean open = False;
    XtVaGetValues(presenter, "open", &open, NULL);
    return open ? 1 : 0;
}

int wtooltip_count(void)
{
    int n = 0;
    for (tip_entry *e = tips; e; e = e->next) n++;
    return n;
}
