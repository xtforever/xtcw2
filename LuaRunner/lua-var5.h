#ifndef LUA_VAR5_H
#define LUA_VAR5_H

#include <lua.h>
#if LUA_VERSION_NUM < 504
#define lua_newuserdatauv(L, s, u) lua_newuserdata(L, s)
#define luaL_pushfail(L) lua_pushnil(L)
#endif
void luaopen_var5 (lua_State *L);


#endif
