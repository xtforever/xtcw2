#include "luaxt.h"
#include "X11/ThreadsI.h"
#include "X11/Intrinsic.h"
#include "X11/StringDefs.h"

#include <xtcw/mls.h>
#include <xtcw/xutil.h>
#include <WcCreate.h>

static int cb_list=0;
static char* temp_str=0;

void   luaxt_init(void)
{
    if(!cb_list) cb_list=m_create(20,sizeof(char*));
}

void   luaxt_destroy(void)
{
    m_free_strings( cb_list, 0);
    cb_list=0;
    free( temp_str ); temp_str=0; 
}


int luaxt_processevent(void)
{
    XtAppProcessEvent(LUAXT_APP, XtIMAll);
    return XtAppGetExitFlag(LUAXT_APP);
}

void m_put_strdup(int m, const char *s1)
{
    char *s=strdup(s1); m_put(m,&s);
}

void luaxt_pushcallback( char *callback_str, char *class_data )
{
    if( is_empty(callback_str) ) return;
    m_put_strdup(cb_list, class_data   );
    m_put_strdup(cb_list, callback_str );
}

char*  luaxt_pullcallback( void )
{
    free(temp_str); temp_str=0;
    if( m_len(cb_list) == 0 ) return "";
    temp_str = *(char **)m_pop(cb_list);
    return temp_str;
}

Widget luaxt_nametowidget(char *s)
{
    char buffer[4096];
    if( strlen(s) >= sizeof(buffer)) return 0;
    char *args = WcCleanName( s, buffer );
    return  WcFullNameToWidget( TopLevel, buffer );
}

void   luaxt_setvalue( char *name, char *res, char *val )
{
    Widget w = luaxt_nametowidget(name);
    if(!w) return;
    XtVaSetValues( w, XtVaTypedArg, val, XtRString, val, strlen(val)+1, NULL );
}

