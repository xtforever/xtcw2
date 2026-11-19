#include "lifecycle.h"

#include "font.h"
#include "lua_bridge.h"
#include "luaxt.h"
#include "m_tool.h"
#include "mls.h"
#include "task_manager.h"
#include "theme.h"
#include "xt_bridge.h"

#include <X11/StringDefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    window_close_cb_t cb;
    void *data;
} close_entry_t;

typedef struct {
    int id;
    int ref;
} lua_close_ref_t;

static XtAppContext app_ctx;
static Widget top_widget;
static int top_destroyed;
static int shutting_down;
static int torn_down;
static int close_handler_installed;
static int atexit_registered;

static Atom wm_protocols_atom;
static Atom wm_delete_window_atom;

static int close_entries;
static int lua_close_refs;
static int next_close_id = 1;

static void top_destroy_cb(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd; (void)cb;
    top_destroyed = 1;
}

/* os.exit() bypasses main(), so at least release the widget tree.  Do not run
 * lifecycle_teardown() here: it closes the Lua state, which is unsafe while a
 * Lua callback is still on the stack.  Use graceful_shutdown() for a full
 * teardown. */
static void top_cleanup_atexit(void)
{
    if (top_widget && !top_destroyed) {
        XtDestroyWidget(top_widget);
        top_widget = NULL;
        top_destroyed = 1;
    }
}

void lifecycle_init(XtAppContext app, Widget top)
{
    app_ctx = app;
    top_widget = top;
    top_destroyed = 0;
    shutting_down = 0;
    torn_down = 0;
    close_handler_installed = 0;

    if (top) XtAddCallback(top, XtNdestroyCallback, top_destroy_cb, NULL);
    if (!atexit_registered) {
        atexit(top_cleanup_atexit);
        atexit_registered = 1;
    }
}

void graceful_shutdown(void)
{
    if (shutting_down) return;
    shutting_down = 1;
    if (app_ctx) XtAppSetExitFlag(app_ctx);
}

static int call_lua_close_refs(void)
{
    if (!L_GLOBAL || !lua_close_refs) return 1;

    for (int i = 0; i < m_len(lua_close_refs); i++) {
        lua_close_ref_t *e = mls(lua_close_refs, i);
        if (!e || e->ref == LUA_NOREF || e->ref == LUA_REFNIL) continue;

        lua_rawgeti(L_GLOBAL, LUA_REGISTRYINDEX, e->ref);
        if (lua_pcall(L_GLOBAL, 0, 1, 0) != LUA_OK) {
            fprintf(stderr, "window_close handler error: %s\n",
                    lua_tostring(L_GLOBAL, -1));
            lua_pop(L_GLOBAL, 1);
            continue;
        }
        if (lua_isboolean(L_GLOBAL, -1) && !lua_toboolean(L_GLOBAL, -1)) {
            lua_pop(L_GLOBAL, 1);
            return 0;
        }
        lua_pop(L_GLOBAL, 1);
    }
    return 1;
}

/* Backwards-compatible single hook: a global on_close() function. */
static int call_on_close(void)
{
    int allowed = 1;

    if (!L_GLOBAL) return 1;
    lua_getglobal(L_GLOBAL, "on_close");
    if (!lua_isfunction(L_GLOBAL, -1)) {
        lua_pop(L_GLOBAL, 1);
        return 1;
    }
    if (lua_pcall(L_GLOBAL, 0, 1, 0) != LUA_OK) {
        fprintf(stderr, "on_close error: %s\n", lua_tostring(L_GLOBAL, -1));
        lua_pop(L_GLOBAL, 1);
        return 1;
    }
    if (lua_isboolean(L_GLOBAL, -1) && !lua_toboolean(L_GLOBAL, -1))
        allowed = 0;
    lua_pop(L_GLOBAL, 1);
    return allowed;
}

static int window_close_allowed(void)
{
    if (close_entries) {
        for (int i = 0; i < m_len(close_entries); i++) {
            close_entry_t *e = mls(close_entries, i);
            if (e && e->cb && !e->cb(e->data)) return 0;
        }
    }
    if (!call_lua_close_refs()) return 0;
    return call_on_close();
}

static void close_request_cb(Widget w, XtPointer client, XEvent *ev,
                             Boolean *cont)
{
    (void)client;
    if (!ev || ev->type != ClientMessage) return;
    if (ev->xclient.message_type != wm_protocols_atom) return;
    if ((Atom)ev->xclient.data.l[0] != wm_delete_window_atom) return;

    if (cont) *cont = False;   /* stop the default shell handler */
    if (!window_close_allowed()) return;
    graceful_shutdown();
}

void lifecycle_install_close_handler(void)
{
    if (!top_widget || close_handler_installed) return;

    wm_protocols_atom = XInternAtom(XtDisplay(top_widget), "WM_PROTOCOLS", False);
    wm_delete_window_atom = XInternAtom(XtDisplay(top_widget),
                                        "WM_DELETE_WINDOW", False);
    XSetWMProtocols(XtDisplay(top_widget), XtWindow(top_widget),
                    &wm_delete_window_atom, 1);
    XtAddEventHandler(top_widget, NoEventMask, True,
                      (XtEventHandler)close_request_cb, NULL);
    close_handler_installed = 1;
}

void lifecycle_teardown(void)
{
    if (torn_down) return;
    torn_down = 1;

    if (top_widget && !top_destroyed)
        XtDestroyWidget(top_widget);
    top_widget = NULL;
    top_destroyed = 1;

    /* Free lifecycle bookkeeping before m_destruct() tears down the MLS
     * allocator. */
    if (close_entries) {
        m_free(close_entries);
        close_entries = 0;
    }
    if (lua_close_refs) {
        m_free(lua_close_refs);
        lua_close_refs = 0;
    }

    luaxt_destroy();
    if (L_GLOBAL) {
        lua_close(L_GLOBAL);
        L_GLOBAL = NULL;
    }
    xt_bridge_cleanup();
    font_cache_flush();
    conststr_free();
    task_manager_cleanup();
    theme_destroy();
    m_destruct();
}

int window_close_register(window_close_cb_t cb, void *data)
{
    close_entry_t e;

    if (!cb) return -1;
    if (!close_entries)
        close_entries = m_create(4, sizeof(close_entry_t));
    e.cb = cb;
    e.data = data;
    m_put(close_entries, &e);
    return m_len(close_entries) - 1;
}

void window_close_unregister(window_close_cb_t cb, void *data)
{
    if (!close_entries) return;

    for (int i = 0; i < m_len(close_entries); i++) {
        close_entry_t *e = mls(close_entries, i);
        if (e && e->cb == cb && e->data == data) {
            m_del(close_entries, i);
            break;
        }
    }
}

static int l_graceful_shutdown(lua_State *L)
{
    (void)L;
    graceful_shutdown();
    return 0;
}

static int l_window_close_register(lua_State *L)
{
    lua_close_ref_t e;

    if (!lua_isfunction(L, 1)) {
        lua_pushnil(L);
        return 1;
    }
    if (!lua_close_refs)
        lua_close_refs = m_create(4, sizeof(lua_close_ref_t));

    lua_pushvalue(L, 1);
    e.ref = luaL_ref(L, LUA_REGISTRYINDEX);
    e.id = next_close_id++;
    m_put(lua_close_refs, &e);

    lua_pushinteger(L, e.id);
    return 1;
}

static int l_window_close_unregister(lua_State *L)
{
    int id = (int)luaL_checkinteger(L, 1);

    if (lua_close_refs) {
        for (int i = 0; i < m_len(lua_close_refs); i++) {
            lua_close_ref_t *e = mls(lua_close_refs, i);
            if (e && e->id == id) {
                luaL_unref(L, LUA_REGISTRYINDEX, e->ref);
                m_del(lua_close_refs, i);
                break;
            }
        }
    }
    return 0;
}

void lifecycle_lua_register(lua_State *L)
{
    lua_register(L, "graceful_shutdown", l_graceful_shutdown);
    lua_register(L, "window_close_register", l_window_close_register);
    lua_register(L, "window_close_unregister", l_window_close_unregister);
}
