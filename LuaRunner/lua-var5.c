#include "lua-var5.h"


#include "m_tool.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <xtcw/mls.h>
#include "var5.h"
#include "m_tool.h"

#define V5_MODULENAME "var5-single"
#define V5_LIBNAME "var5lib"


static int v5_get_name( lua_State *L )
{
    /* check the name of the metatable for the user-data table */
    int *p = (int *) luaL_testudata( L, 1, V5_MODULENAME );      
    if (p == NULL) {
	TRACE(1, "not var5 userdata table" );
	luaL_pushfail(L);  /* not a file */
        return 0;
    }
    
    TRACE(1, "var-id=%d", *p  );
    int var5 = *p;
    
    char *val = mvar_name( var5 );
    lua_pushstring(L, val );
    return 1;
}

static int v5_get (lua_State *L)
{
    /* check the name of the metatable for the user-data table */
    int *p = (int *) luaL_testudata( L, 1, V5_MODULENAME );      
    if (p == NULL) {
	TRACE(1, "not var5 userdata table" );
	luaL_pushfail(L);  /* not a file */
        return 0;
    }
    
    TRACE(1, "get something from index %d, var-id=%d", lua_tointeger(L,2), *p  );


    int var5 = *p;
    int index =  lua_tointeger(L,2);
    char *val = mvar_get_string( var5, index );
    lua_pushstring(L, val );
    return 1;
}

static int v5_put (lua_State *L)
{
    /* check the name of the metatable for the user-data table */
    int *p = (int *) luaL_testudata( L, 1, V5_MODULENAME );      
    if (p == NULL) {
	TRACE(1, "not var5 userdata table" );
	luaL_pushfail(L);  /* not a file */
        return 0;
    }

    TRACE(1, "put something at index %s <== %s, var-id: %d", lua_tostring(L,2), lua_tostring(L,3), *p );
    
    int var5 = *p;
    int index =  lua_tointeger(L,2);
    char *val = (char *) lua_tostring(L,3);
    mvar_put_string( var5, val, index );
    return 0;
}

/**
 * metamethods for add/lookup var5 handles
 */
static const luaL_Reg v5_metameth[] = {
  {"__index", v5_get },
  {"__newindex", v5_put },
  {"__tostring", v5_get_name },
  
  {NULL, NULL}
};


static const struct luaL_Reg v5_lualib [] = {
      {NULL, NULL}
};


static int  v5_lookup( lua_State *L  )
{
    char *varname = (char *)lua_tostring(L,2);
    TRACE(1, "Hell yeah %s\n", varname );

    /* check if variable exists */
    int var5 = mvar_parse_string( varname, -1 );

    /* varaiable does not exists, create a string list */
    if( var5 < 0 ) var5 =  mvar_parse_string( varname, VAR_STRING );

    /* create new userdata table and store var5 handle inside */
    int *v5p  = (int*) lua_newuserdatauv(L, sizeof(int), 0);
    *v5p = var5;

    /* assign the predifined metatable to this userdata-table */
    luaL_getmetatable(L, V5_MODULENAME );
    lua_setmetatable(L, -2);
    return 1;
}

static const luaL_Reg v5_lookupmetameth[] = {
  {"__index", v5_lookup },  
  {NULL, NULL}
};


/* 1. create a table for var5 type variables named: V5_MODULENAME
   2. create a table with metamethods for lookup/add 'var5' variables 
*/
static int v5_luaopen(lua_State *L)
{    
    luaL_newmetatable(L, V5_MODULENAME );  /* metatable for var5 handles */
    luaL_setfuncs(L, v5_metameth, 0);  /* add metamethods to new metatable */


    luaL_newlib( L, v5_lualib );
    luaL_newmetatable(L, "var5index" );	/* metatable for file handles */
    luaL_setfuncs(L, v5_lookupmetameth, 0); /* add metamethods to new metatable */
    lua_setmetatable(L, -2);  /* set table as metatable for strings */
    return 1;
}


/* register table var5 with metafunctions */
void luaopen_var5 (lua_State *L)
{
    luaL_requiref(L, "var5", v5_luaopen, 1);
    lua_pop(L, 1);  /* remove lib */
}

