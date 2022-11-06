#ifndef LUAXT_H
#define LUAXT_H

#include "X11/Intrinsic.h"
void   luaxt_init(void);
void   luaxt_destroy(void);

int    luaxt_processevent( void );
void   luaxt_pushcallback( char *callback_str, char *class_data );
char*  luaxt_pullcallback( void );
Widget luaxt_nametowidget(char *s);
void   luaxt_setvalue( char *w, char *res, char *val );

extern XtAppContext LUAXT_APP;
extern Widget TopLevel;
#endif
