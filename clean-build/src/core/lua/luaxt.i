%module luaxt
%{
#include <lua.h>
#if LUA_VERSION_NUM < 504
#define lua_newuserdatauv(L, s, u) lua_newuserdata(L, s)
#define luaL_pushfail(L) lua_pushnil(L)
#endif
#include "luaxt.h"
%}

void   luaxt_init(void);
void   luaxt_destroy(void);

int    luaxt_processevent( void );
void   luaxt_pushcallback( char *callback_str, char *class_data );
char*  luaxt_pullcallback( void );
char*  luaxt_pulldata( void );
Widget luaxt_nametowidget( char *s );
void   luaxt_setvalue( char *w, char *res, char *val );
void   luaxt_spawn_copy( char *src, char *dst );
void   MessageBox(Widget w, XtPointer client_data, XtPointer call_data);

extern XtAppContext LUAXT_APP;
