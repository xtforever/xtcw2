#ifndef LIFECYCLE_H
#define LIFECYCLE_H

#include <X11/Intrinsic.h>
#include <lua.h>

/*
 * Application lifecycle helpers shared by commander_runner and the Lua demos.
 *
 * graceful_shutdown() requests a clean shutdown: it sets the Xt exit flag and
 * the main loop in commander_runner then performs the teardown
 * (window_close handlers, widget destroy, Lua close, m_destruct, ...).
 * It is safe to call from Lua callbacks; do not call os.exit() if you want the
 * cleanup to run.
 */

/* C window-close callback. Return 0 to veto the close, non-zero to allow it. */
typedef int (*window_close_cb_t)(void *data);

/* Called once after XtOpenApplication.  Registers the destroy callback and the
 * atexit safety net. */
void lifecycle_init(XtAppContext app, Widget top);

/* Called after XtRealizeWidget: installs WM_DELETE_WINDOW and the handler. */
void lifecycle_install_close_handler(void);

/* Request a clean shutdown (sets the Xt exit flag). Idempotent. */
void graceful_shutdown(void);

/* Perform the actual teardown after the main loop has returned. Idempotent. */
void lifecycle_teardown(void);

/* Register/unregister a C close callback.  Registration returns a handle. */
int  window_close_register(window_close_cb_t cb, void *data);
void window_close_unregister(window_close_cb_t cb, void *data);

/* Register the Lua-facing functions:
 *   graceful_shutdown()
 *   window_close_register(fn) -> handle
 *   window_close_unregister(handle)
 */
void lifecycle_lua_register(lua_State *L);

#endif /* LIFECYCLE_H */
