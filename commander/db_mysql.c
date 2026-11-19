/* Minimal MySQL query binding for LUI.
 *
 * Lua API:
 *   db_query(opts, sql) -> { columns = {...}, rows = {...} } | nil, err
 *
 * opts: group (default "adminpanel"), host, port, user, password, database,
 *       socket, connect_timeout, read_timeout.
 *
 * Credentials normally come from ~/.my.cnf via MYSQL_READ_DEFAULT_GROUP.
 */

#include "db_mysql.h"
#include "lua_bridge.h"

#include <mysql.h>
#include <lua.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *group;
    char *host;
    char *user;
    char *password;
    char *database;
    char *socket;
    unsigned int port;
    unsigned int connect_timeout;
    unsigned int read_timeout;
} db_opts_t;

static char *dup_field(lua_State *L, int index, const char *key)
{
    char *out = NULL;
    lua_getfield(L, index, key);
    if (lua_isstring(L, -1)) out = strdup(lua_tostring(L, -1));
    lua_pop(L, 1);
    return out;
}

static unsigned int int_field(lua_State *L, int index, const char *key)
{
    unsigned int out = 0;
    lua_getfield(L, index, key);
    if (lua_isnumber(L, -1)) out = (unsigned int)lua_tointeger(L, -1);
    lua_pop(L, 1);
    return out;
}

static void opts_free(db_opts_t *o)
{
    free(o->group);
    free(o->host);
    free(o->user);
    free(o->password);
    free(o->database);
    free(o->socket);
    memset(o, 0, sizeof(*o));
}

static void push_error(lua_State *L, const char *msg)
{
    lua_pushnil(L);
    lua_pushstring(L, msg ? msg : "unknown MySQL error");
}

static int l_db_query(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    const char *sql = luaL_checkstring(L, 2);

    db_opts_t o;
    memset(&o, 0, sizeof(o));
    o.group = dup_field(L, 1, "group");
    o.host = dup_field(L, 1, "host");
    o.user = dup_field(L, 1, "user");
    o.password = dup_field(L, 1, "password");
    o.database = dup_field(L, 1, "database");
    o.socket = dup_field(L, 1, "socket");
    o.port = int_field(L, 1, "port");
    o.connect_timeout = int_field(L, 1, "connect_timeout");
    o.read_timeout = int_field(L, 1, "read_timeout");
    if (!o.group) o.group = strdup("adminpanel");

    MYSQL *conn = mysql_init(NULL);
    if (!conn) {
        opts_free(&o);
        push_error(L, "mysql_init failed");
        return 2;
    }

    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    if (o.group && o.group[0]) {
        mysql_options(conn, MYSQL_READ_DEFAULT_GROUP, o.group);
    }
    unsigned int connect_timeout = o.connect_timeout ? o.connect_timeout : 5;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &connect_timeout);
    if (o.read_timeout) {
        mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &o.read_timeout);
    }

    if (!mysql_real_connect(conn, o.host, o.user, o.password, o.database,
                            o.port, o.socket, 0)) {
        char err[512];
        snprintf(err, sizeof(err), "%s", mysql_error(conn));
        mysql_close(conn);
        opts_free(&o);
        push_error(L, err);
        return 2;
    }

    if (mysql_real_query(conn, sql, (unsigned long)strlen(sql)) != 0) {
        char err[512];
        snprintf(err, sizeof(err), "%s", mysql_error(conn));
        mysql_close(conn);
        opts_free(&o);
        push_error(L, err);
        return 2;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res && mysql_field_count(conn) != 0) {
        char err[512];
        snprintf(err, sizeof(err), "%s", mysql_error(conn));
        mysql_close(conn);
        opts_free(&o);
        push_error(L, err);
        return 2;
    }

    lua_newtable(L); /* result */

    lua_newtable(L); /* columns */
    unsigned int ncols = res ? mysql_num_fields(res) : 0;
    MYSQL_FIELD *fields = res ? mysql_fetch_fields(res) : NULL;
    for (unsigned int i = 0; i < ncols; i++) {
        lua_pushstring(L, fields[i].name ? fields[i].name : "");
        lua_rawseti(L, -2, (lua_Integer)i + 1);
    }
    lua_setfield(L, -2, "columns");

    lua_newtable(L); /* rows */
    int row_index = 1;
    if (res) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            unsigned long *lengths = mysql_fetch_lengths(res);
            lua_newtable(L);
            for (unsigned int i = 0; i < ncols; i++) {
                if (row[i]) {
                    lua_pushlstring(L, row[i], (size_t)lengths[i]);
                    lua_setfield(L, -2, fields[i].name ? fields[i].name : "");
                } else {
                    lua_pushnil(L);
                    lua_setfield(L, -2, fields[i].name ? fields[i].name : "");
                }
            }
            lua_rawseti(L, -2, row_index++);
        }
    }
    lua_setfield(L, -2, "rows");

    if (res) mysql_free_result(res);
    mysql_close(conn);
    opts_free(&o);
    return 1;
}

void db_mysql_register(lua_State *L)
{
    mysql_library_init(0, NULL, NULL);
    lua_register(L, "db_query", l_db_query);
}
