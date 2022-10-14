%module luaxt
%{
#include "luaxt.h"
%}

void   luaxt_init(void);
void   luaxt_destroy(void);

int    luaxt_processevent( void );
int    luaxt_pushcallback( char *callback_str );
char*  luaxt_pullcallback( void );
Widget luaxt_nametowidget( char *s );
void   luaxt_setvalue( char *w, char *res, char *val );

extern XtAppContext LUAXT_APP;
