#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "task_manager.h"
#include "luaxt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "mls.h"
#include "m_tool.h"
#include <WcCreate.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

/* Global Lua callback name set from Lua */
static char *lua_handler_name = NULL;
extern lua_State *L_GLOBAL;

static void internal_event_cb(task_msg_t *msg) {
    if (!lua_handler_name) return;
    
    char buf[512];
    /* Format event as a Lua function call string: handler({job_id=1, type=0, ...}) */
    snprintf(buf, sizeof(buf), "%s({job_id=%d, type=%d, progress=%.2f, message=[[%s]]})",
             lua_handler_name, msg->job_id, msg->type, msg->progress, msg->message);
    
    /* Execute immediately (no callback queue — matches the commander_runner
     * event-loop model where callbacks run as soon as they fire). */
    if (luaL_loadstring(L_GLOBAL, buf) != LUA_OK) {
        fprintf(stderr, "Task event error: %s\n", lua_tostring(L_GLOBAL, -1));
        lua_pop(L_GLOBAL, 1);
        return;
    }
    if (lua_pcall(L_GLOBAL, 0, 0, 0) != LUA_OK) {
        fprintf(stderr, "Task event error: %s\n", lua_tostring(L_GLOBAL, -1));
        lua_pop(L_GLOBAL, 1);
    }
}

static Widget luaarg_to_widget( lua_State *L, int index )
{
    Widget w;
    if( lua_isuserdata( L, index ) ) {
	w = lua_touserdata( L, index );
	return w;
    }
    const char *s=lua_tostring(L,index);
    if( s==NULL) return NULL;
    w = luaxt_nametowidget( (char*)s );
    return w;
}

#include "Wheel.h"
#include "theme.h"

static int l_wheel_exec_command(lua_State *L) {
    Widget w = luaarg_to_widget(L, 1);
    int cmd = luaL_checkinteger(L, 2);
    int val = luaL_optinteger(L, 3, 0);
    if (w) {
        lua_pushinteger(L, wheel_exec_command(w, cmd, val));
        return 1;
    }
    return 0;
}

static int l_ls(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    DIR *d = opendir(path);
    if (!d) {
        lua_pushnil(L);
        lua_pushstring(L, strerror(errno));
        return 2;
    }
    
    lua_newtable(L);
    struct dirent *de;
    int i = 1;
    while ((de = readdir(d))) {
        if (strcmp(de->d_name, ".") == 0) continue;
        
        lua_pushinteger(L, i++);
        lua_newtable(L);
        
        lua_pushstring(L, "name");
        lua_pushstring(L, de->d_name);
        lua_settable(L, -3);
        
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, de->d_name);
        struct stat st;
        if (stat(full_path, &st) == 0) {
            lua_pushstring(L, "is_dir");
            lua_pushboolean(L, S_ISDIR(st.st_mode));
            lua_settable(L, -3);
            
            lua_pushstring(L, "size");
            lua_pushinteger(L, st.st_size);
            lua_settable(L, -3);
        }
        
        lua_settable(L, -3);
    }
    closedir(d);
    return 1;
}

static int l_task_copy(lua_State *L) {
    const char *src = luaL_checkstring(L, 1);
    const char *dst = luaL_checkstring(L, 2);
    
    typedef struct {
        char src[4096];
        char dst[4096];
    } copy_args_t;
    
    copy_args_t *args = malloc(sizeof(copy_args_t));
    strncpy(args->src, src, sizeof(args->src));
    strncpy(args->dst, dst, sizeof(args->dst));
    
    task_func_t func = task_get_func("copy_task");
    if (!func) {
        free(args);
        lua_pushnil(L);
        lua_pushstring(L, "copy_task not registered");
        return 2;
    }
    
    int id = task_spawn(func, args);
    lua_pushinteger(L, id);
    return 1;
}

static int l_task_spawn(lua_State *L) {
    const char *func_name = luaL_checkstring(L, 1);
    void *user_arg = (void*)lua_tostring(L, 2);
    
    task_func_t func = task_get_func(func_name);
    if (!func) {
        lua_pushnil(L);
        lua_pushstring(L, "Task function not found");
        return 2;
    }
    
    int id = task_spawn(func, user_arg);
    lua_pushinteger(L, id);
    return 1;
}

static int l_task_control(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    int cmd = luaL_checkinteger(L, 2);
    task_control(id, (task_cmd_t)cmd);
    return 0;
}

static int l_task_set_handler(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    if (lua_handler_name) free(lua_handler_name);
    lua_handler_name = strdup(name);
    task_set_event_cb(internal_event_cb);
    return 0;
}

static int l_theme_select(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    theme_select(name);
    lua_pushstring(L, theme_active() ? theme_active() : "");
    return 1;
}

void register_task_lua(lua_State *L) {
    lua_register(L, "wheel_exec_command", l_wheel_exec_command);
    lua_register(L, "theme_select", l_theme_select);
    lua_register(L, "task_copy", l_task_copy);
    lua_register(L, "ls", l_ls);
    lua_register(L, "task_spawn", l_task_spawn);
    lua_register(L, "task_control", l_task_control);
    lua_register(L, "task_set_handler", l_task_set_handler);
    
    /* Export constants */
    lua_pushinteger(L, TASK_CMD_STOP); lua_setglobal(L, "TASK_CMD_STOP");
    lua_pushinteger(L, TASK_CMD_CONT); lua_setglobal(L, "TASK_CMD_CONT");
    lua_pushinteger(L, TASK_CMD_ABORT); lua_setglobal(L, "TASK_CMD_ABORT");
    
    lua_pushinteger(L, TASK_EVENT_PROGRESS); lua_setglobal(L, "TASK_EVENT_PROGRESS");
    lua_pushinteger(L, TASK_EVENT_COMPLETE); lua_setglobal(L, "TASK_EVENT_COMPLETE");
    lua_pushinteger(L, TASK_EVENT_ERROR); lua_setglobal(L, "TASK_EVENT_ERROR");

    lua_pushinteger(L, 1); lua_setglobal(L, "WHEEL_UP");
    lua_pushinteger(L, 2); lua_setglobal(L, "WHEEL_DOWN");
    lua_pushinteger(L, 3); lua_setglobal(L, "WHEEL_FIRE");
}
