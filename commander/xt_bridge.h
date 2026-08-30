#ifndef XT_BRIDGE_H
#define XT_BRIDGE_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <X11/Intrinsic.h>

const char *luastring(lua_State *L, int i);
Widget luaarg_to_widget(lua_State *L, int index);

int xtcreate_lua(lua_State *L);
int xtsetvalue_lua(lua_State *L);
int xtgetvalue_lua(lua_State *L);
int xtaction_lua(lua_State *L);
int xtmanage_lua(lua_State *L);
int xtunmanage_lua(lua_State *L);
int xtdestroy_lua(lua_State *L);

int mls_create_lua(lua_State *L);
int mls_put_string_lua(lua_State *L);
int mls_clear_lua(lua_State *L);
int mls_len_lua(lua_State *L);
int mls_get_string_lua(lua_State *L);

int xtapptimeout_lua(lua_State *L);

int xtgeometry_lua(lua_State *L);
int xterror_count_lua(lua_State *L);

#endif
