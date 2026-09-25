/* test_karoed.c — keystroke-injection test driver for the KaroEd widget.
 *
 * Creates a KaroEd widget, gives it keyboard focus, and replays a scenario
 * of synthetic key events via the XTEST extension (see xkey.c). KaroEd is
 * instrumented with TRACE(KARO_TESTING, ...) so the injected actions report
 * themselves to stderr; run_tests.sh captures stderr and asserts on those
 * lines (no screenshots).
 *
 * Usage: test_karoed [scenario]
 *   scenario ∈ { type | return | backspace | delete | arrows }   (default: type)
 *
 * Run under Xvfb:  xvfb-run --auto-servernum ./test_karoed arrows
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/keysym.h>
#include <X11/Xaw/XawInit.h>

#include "mls.h"
#include "micro_vars.h"
#include "xtcw/KaroEd.h"
#include "xkey.h"

/* Must match @def KARO_TESTING in KaroEd.widget. Defined locally so this
 * driver does not depend on the generated private header being present. */
#define KARO_TESTING 50

/* Delay between successive injection steps (ms). */
#define STEP_MS 100

extern int trace_level;

typedef enum { S_STR, S_KEY, S_CALL, S_WAIT } step_kind;

typedef struct {
    step_kind kind;
    const char *text;   /* S_STR */
    KeySym keysym;      /* S_KEY */
    unsigned int mods;  /* S_KEY */
    void (*fn)(Widget); /* S_CALL */
    int ms;             /* S_WAIT: extra settle time */
} step_t;

static const step_t sc_type[] = {
    { S_STR, "abc", 0, 0 },
};

static const step_t sc_return[] = {
    { S_STR, "ab", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "cd", 0, 0 },
};

static const step_t sc_backspace[] = {
    { S_STR, "abc", 0, 0 },
    { S_KEY, NULL, XK_BackSpace, 0 },
};

static const step_t sc_delete[] = {
    { S_STR, "abc", 0, 0 },
    { S_KEY, NULL, XK_Left, 0 },
    { S_KEY, NULL, XK_Delete, 0 },
};

static const step_t sc_arrows[] = {
    { S_STR, "ab", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "cd", 0, 0 },
    { S_KEY, NULL, XK_Up, 0 },
    { S_KEY, NULL, XK_Left, 0 },
    { S_KEY, NULL, XK_Down, 0 },
    { S_KEY, NULL, XK_Right, 0 },
};

/* Up/Down must clamp the cursor to the target line length via col_hint. */
static const step_t sc_clamp_column[] = {
    { S_STR, "a", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "long", 0, 0 },
    { S_KEY, NULL, XK_Up, 0 },
    { S_STR, "X", 0, 0 },
};

/* Home/End move the cursor to column 0 / end of line. */
static const step_t sc_home_end[] = {
    { S_STR, "abc", 0, 0 },
    { S_KEY, NULL, XK_Home, 0 },
    { S_KEY, NULL, XK_End, 0 },
};

typedef struct {
    const char *name;
    const step_t *steps;
    int count;
    String *resources;   /* per-scenario Xt fallback resources (NULL = default) */
} scenario_t;

/* Set `locked` before widget creation so initialize() sees it. */
static String locked_resources[] = {
    (String)"*xftFont: Sans-12",
    (String)"*grid_width: 80",
    (String)"*grid_height: 24",
    (String)"*auto_resize: 0",
    (String)"*locked: True",
    NULL
};

/* Load a form file at creation time (exercises the FRM/SCR cleanup path). */
static String fileform_resources[] = {
    (String)"*xftFont: Sans-12",
    (String)"*grid_width: 80",
    (String)"*grid_height: 24",
    (String)"*auto_resize: 0",
    (String)"*fileForm: formtest.frm2",
    NULL
};

/* Public API: writeln()/get_text() and the change callback. */
static void api_trace(const char *label, const char *s)
{
    fprintf(stderr, "[50]api %s='", label);
    for (; s && *s; s++) {
        if (*s == '\n')
            fputs("\\n", stderr);
        else
            fputc(*s, stderr);
    }
    fprintf(stderr, "'\n");
}

static void act_api(Widget w)
{
    char *t;
    karoEd_writeln(w, "hello");
    karoEd_writeln(w, "world");
    t = karoEd_get_text(w);
    api_trace("get_text", t);
    if (t)
        XtFree(t);
}

/* Return preserves the current line's leading whitespace. */
static const step_t sc_indent[] = {
    { S_STR, "  ab", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "cd", 0, 0 },
};

/* Tab inserts spaces up to the next tab stop (width 8). */
static const step_t sc_tab[] = {
    { S_STR, "a", 0, 0 },
    { S_KEY, NULL, XK_Tab, 0 },
    { S_STR, "b", 0, 0 },
};

/* Exercise the public API and change callback. */
static const step_t sc_api[] = {
    { S_CALL, NULL, 0, 0, act_api, 0 },
};

/* Just create (fileForm loads) and destroy. */
static const step_t sc_fileform[] = {
    { S_WAIT, NULL, 0, 0, NULL, 100 },
};

/* With `locked`, type/backspace/return/paste must not modify the buffer. */
static const step_t sc_locked[] = {
    { S_STR, "x", 0, 0 },
    { S_KEY, NULL, XK_BackSpace, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_KEY, NULL, XK_v, ControlMask },
};

/* Copy a multi-line selection and paste it back after deleting the buffer. */
static const step_t sc_copy_paste[] = {
    { S_STR, "xy", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "zw", 0, 0 },
    { S_KEY, NULL, XK_a, ControlMask },       /* select all + copy */
    { S_KEY, NULL, XK_BackSpace, 0 },         /* delete selection */
    { S_KEY, NULL, XK_v, ControlMask },       /* paste */
    { S_WAIT, NULL, 0, 0, NULL, 300 },        /* settle */
};

/* Ctrl+A selects all lines and copies them to PRIMARY/CUT_BUFFER0. */
static const step_t sc_select_all[] = {
    { S_STR, "ab", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "cd", 0, 0 },
    { S_KEY, NULL, XK_a, ControlMask },
};

/* Test-hook actions that impose an arbitrary selection range. */
static void act_sel_forward(Widget w)
{
    String p[] = { (String)"0", (String)"0", (String)"2", (String)"1" };
    XtCallActionProc(w, "select_range", NULL, p, 4);
}

static void act_sel_reverse(Widget w)
{
    String p[] = { (String)"2", (String)"1", (String)"0", (String)"0" };
    XtCallActionProc(w, "select_range", NULL, p, 4);
}

static void act_sel_first_char(Widget w)
{
    String p[] = { (String)"0", (String)"0", (String)"1", (String)"0" };
    XtCallActionProc(w, "select_range", NULL, p, 4);
}

/* Ctrl+C on a selection (no select_all shortcut). */
static const step_t sc_copy_action[] = {
    { S_STR, "ab", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "cd", 0, 0 },
    { S_CALL, NULL, 0, 0, act_sel_first_char, 0 },
    { S_KEY, NULL, XK_c, ControlMask },
};

/* Ctrl+X copies then deletes the selection. */
static const step_t sc_cut_action[] = {
    { S_STR, "ab", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "cd", 0, 0 },
    { S_CALL, NULL, 0, 0, act_sel_forward, 0 },
    { S_KEY, NULL, XK_x, ControlMask },
};

/* Typing over a multi-line selection replaces the whole range. */
static const step_t sc_type_over_selection[] = {
    { S_STR, "ab", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "cd", 0, 0 },
    { S_CALL, NULL, 0, 0, act_sel_forward, 0 },
    { S_STR, "X", 0, 0 },
};

/* Same, with the anchor after the cursor (reversed selection). */
static const step_t sc_type_over_selection_rev[] = {
    { S_STR, "ab", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "cd", 0, 0 },
    { S_CALL, NULL, 0, 0, act_sel_reverse, 0 },
    { S_STR, "X", 0, 0 },
};

/* Backspace at column 0 of line 1 joins it with line 0. */
static const step_t sc_join_backspace[] = {
    { S_STR, "ab", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "cd", 0, 0 },
    { S_KEY, NULL, XK_Left, 0 },
    { S_KEY, NULL, XK_Left, 0 },
    { S_KEY, NULL, XK_BackSpace, 0 },
};

/* Delete at end of line 0 joins line 1 into line 0. */
static const step_t sc_join_delete[] = {
    { S_STR, "ab", 0, 0 },
    { S_KEY, NULL, XK_Return, 0 },
    { S_STR, "cd", 0, 0 },
    { S_KEY, NULL, XK_Up, 0 },
    { S_KEY, NULL, XK_Delete, 0 },
};

static const scenario_t scenarios[] = {
    { "type",      sc_type,      (int)(sizeof(sc_type) / sizeof(sc_type[0])), NULL },
    { "return",    sc_return,    (int)(sizeof(sc_return) / sizeof(sc_return[0])), NULL },
    { "backspace", sc_backspace, (int)(sizeof(sc_backspace) / sizeof(sc_backspace[0])), NULL },
    { "delete",    sc_delete,    (int)(sizeof(sc_delete) / sizeof(sc_delete[0])), NULL },
    { "arrows",    sc_arrows,    (int)(sizeof(sc_arrows) / sizeof(sc_arrows[0])), NULL },
    { "clamp_column", sc_clamp_column, (int)(sizeof(sc_clamp_column) / sizeof(sc_clamp_column[0])), NULL },
    { "home_end",  sc_home_end,  (int)(sizeof(sc_home_end) / sizeof(sc_home_end[0])), NULL },
    { "locked",    sc_locked,    (int)(sizeof(sc_locked) / sizeof(sc_locked[0])), locked_resources },
    { "select_all", sc_select_all, (int)(sizeof(sc_select_all) / sizeof(sc_select_all[0])), NULL },
    { "type_over_selection", sc_type_over_selection, (int)(sizeof(sc_type_over_selection) / sizeof(sc_type_over_selection[0])), NULL },
    { "type_over_selection_rev", sc_type_over_selection_rev, (int)(sizeof(sc_type_over_selection_rev) / sizeof(sc_type_over_selection_rev[0])), NULL },
    { "join_backspace", sc_join_backspace, (int)(sizeof(sc_join_backspace) / sizeof(sc_join_backspace[0])), NULL },
    { "join_delete", sc_join_delete, (int)(sizeof(sc_join_delete) / sizeof(sc_join_delete[0])), NULL },
    { "copy_paste", sc_copy_paste, (int)(sizeof(sc_copy_paste) / sizeof(sc_copy_paste[0])), NULL },
    { "copy_action", sc_copy_action, (int)(sizeof(sc_copy_action) / sizeof(sc_copy_action[0])), NULL },
    { "cut_action", sc_cut_action, (int)(sizeof(sc_cut_action) / sizeof(sc_cut_action[0])), NULL },
    { "api", sc_api, (int)(sizeof(sc_api) / sizeof(sc_api[0])), NULL },
    { "fileform", sc_fileform, (int)(sizeof(sc_fileform) / sizeof(sc_fileform[0])), fileform_resources },
    { "indent", sc_indent, (int)(sizeof(sc_indent) / sizeof(sc_indent[0])), NULL },
    { "tab", sc_tab, (int)(sizeof(sc_tab) / sizeof(sc_tab[0])), NULL },
};

static const scenario_t *find_scenario(const char *name)
{
    for (unsigned i = 0; i < sizeof(scenarios) / sizeof(scenarios[0]); i++)
        if (strcmp(scenarios[i].name, name) == 0)
            return &scenarios[i];
    return NULL;
}

static Display *dpy;
static Widget ed;
static XtAppContext app;
static const scenario_t *scen;
static int step_i;

/* Set XftFont/grid resources through the resource database so Xt runs its
 * type converters. Passing them via XtVaCreateManagedWidget varargs would
 * store the raw string in the XftFont* field and crash. */
static String fallback_resources[] = {
    (String)"*xftFont: Sans-12",
    (String)"*grid_width: 80",
    (String)"*grid_height: 24",
    (String)"*auto_resize: 0",
    NULL
};

/* Change-callback probe: records every edit notification via TRACE so the
 * runner can assert that locked widgets emit none. */
static void karo_cb(Widget w, XtPointer client, XtPointer call)
{
    (void)w;
    (void)client;
    fprintf(stderr, "[50]KaroEd callback kind=%ld\n", (long)call);
}

static void do_step(XtPointer client_data, XtIntervalId *id)
{
    (void)client_data;
    (void)id;

    if (step_i < scen->count) {
        const step_t *s = &scen->steps[step_i++];
        int next_ms = STEP_MS;
        switch (s->kind) {
        case S_STR:
            xkey_type(dpy, s->text);
            break;
        case S_KEY:
            xkey_press(dpy, s->keysym, s->mods);
            break;
        case S_CALL:
            if (s->fn)
                s->fn(ed);
            break;
        case S_WAIT:
            /* bounded settle for asynchronous selection round-trips */
            next_ms = (s->ms > 0) ? s->ms : STEP_MS * 5;
            break;
        }
        XSync(dpy, False);
        XtAppAddTimeOut(app, next_ms, do_step, NULL);
    } else {
        /* All keys injected; the event loop has drained them by now. */
        XtCallActionProc(ed, "dump", NULL, NULL, 0);
        XSync(dpy, False);
        XtAppSetExitFlag(app);
    }
}

static int x_error_handler(Display *d, XErrorEvent *e)
{
    char buf[256];
    XGetErrorText(d, e->error_code, buf, sizeof(buf));
    fprintf(stderr, "test_karoed: X error: %s (request=%d minor=%d)\n",
            buf, e->request_code, e->minor_code);
    return 0;
}

int main(int argc, char **argv)
{
    const char *name = (argc > 1) ? argv[1] : "type";
    Widget top;

    scen = find_scenario(name);
    if (scen == NULL) {
        fprintf(stderr, "test_karoed: unknown scenario '%s'\n", name);
        fprintf(stderr, "scenarios:");
        for (unsigned i = 0; i < sizeof(scenarios) / sizeof(scenarios[0]); i++)
            fprintf(stderr, " %s", scenarios[i].name);
        fprintf(stderr, "\n");
        return 2;
    }

    m_init();
    mv_init();

    XtSetLanguageProc(NULL, NULL, NULL);
    XawInitializeWidgetSet();
    XSetErrorHandler(x_error_handler);

    top = XtOpenApplication(&app, "test_karoed", NULL, 0, &argc, argv,
                            scen->resources ? scen->resources : fallback_resources,
                            sessionShellWidgetClass, NULL, 0);

    trace_level = KARO_TESTING;

    ed = XtVaCreateManagedWidget("ed", karoEdWidgetClass, top, NULL);
    XtAddCallback(ed, "callback", karo_cb, NULL);

    XtRealizeWidget(top);
    XtMapWidget(top);
    XSync(XtDisplay(top), False);

    dpy = XtDisplay(top);
    if (xkey_focus(dpy, XtWindow(ed)) != 0) {
        fprintf(stderr, "test_karoed: could not focus KaroEd window\n");
        return 1;
    }

    step_i = 0;
    XtAppAddTimeOut(app, STEP_MS, do_step, NULL);
    XtAppMainLoop(app);

    XtDestroyWidget(top);
    mv_destroy();
    m_destruct();
    return 0;
}
