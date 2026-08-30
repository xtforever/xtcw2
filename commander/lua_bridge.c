#include "lua_bridge.h"
#include "luaxt.h"
#include <stdio.h>
#include <stdlib.h>
#include "xt_bridge.h"

lua_State *L_GLOBAL = NULL;

void lua_bridge_init(XtAppContext app) {
    luaxt_init();
    L_GLOBAL = luaL_newstate();
    luaL_openlibs(L_GLOBAL);
    lua_bridge_register_bindings();
}

int lua_bridge_load_script(const char *filename) {
    if (L_GLOBAL == NULL) return 1;
    if (luaL_loadfile(L_GLOBAL, filename) != LUA_OK) {
        fprintf(stderr, "Error loading script: %s\n", lua_tostring(L_GLOBAL, -1));
        return 1;
    }
    if (lua_pcall(L_GLOBAL, 0, 0, 0) != LUA_OK) {
        fprintf(stderr, "Error running script: %s\n", lua_tostring(L_GLOBAL, -1));
        return 1;
    }
    return 0;
}

static int l_xtappexitflag_lua(lua_State *L) {
    extern XtAppContext LUAXT_APP;
    if (LUAXT_APP) {
        XtAppSetExitFlag(LUAXT_APP);
    }
    return 0;
}

void lua_bridge_register_bindings(void) {
    lua_register(L_GLOBAL, "xtcreate", xtcreate_lua);
    lua_register(L_GLOBAL, "xtsetvalue", xtsetvalue_lua);
    lua_register(L_GLOBAL, "xtaction", xtaction_lua);
    lua_register(L_GLOBAL, "callF", xtaction_lua); // callF mapped to xtaction for now
    lua_register(L_GLOBAL, "xtgetvalue", xtgetvalue_lua);
    lua_register(L_GLOBAL, "xtmanage", xtmanage_lua);
    lua_register(L_GLOBAL, "xtunmanage", xtunmanage_lua);
    lua_register(L_GLOBAL, "xtdestroy", xtdestroy_lua);
    lua_register(L_GLOBAL, "mls_create", mls_create_lua);
    lua_register(L_GLOBAL, "mls_put_string", mls_put_string_lua);
    lua_register(L_GLOBAL, "mls_clear", mls_clear_lua);
    lua_register(L_GLOBAL, "mls_len", mls_len_lua);
    lua_register(L_GLOBAL, "mls_get_string", mls_get_string_lua);
    lua_register(L_GLOBAL, "xtappexitflag", l_xtappexitflag_lua);
    lua_register(L_GLOBAL, "xtapptimeout", xtapptimeout_lua);
    lua_register(L_GLOBAL, "xtgeometry", xtgeometry_lua);
    lua_register(L_GLOBAL, "xterror_count", xterror_count_lua);
}

void lua_bridge_destroy(void) {
    if (L_GLOBAL) {
        lua_close(L_GLOBAL);
        L_GLOBAL = NULL;
    }
}
