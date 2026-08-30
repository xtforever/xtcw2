#ifndef LUA_BRIDGE_H
#define LUA_BRIDGE_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <X11/Intrinsic.h>

/* Global Lua state for external access */
extern lua_State *L_GLOBAL;

void lua_bridge_init(XtAppContext app);
int lua_bridge_load_script(const char *filename);
void lua_bridge_register_bindings(void);
void lua_bridge_destroy(void);

/* Helper functions for xt bindings */
Widget luaarg_to_widget(lua_State *L, int index);
const char *luastring(lua_State *L, int i);

#endif /* LUA_BRIDGE_H */
