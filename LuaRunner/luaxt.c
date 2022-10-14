#include "luaxt.h"
#include "X11/ThreadsI.h"
#include "X11/Intrinsic.h"
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


int    luaxt_pushcallback( char *callback_str )
{
    char *s=strdup(callback_str);
    m_put(cb_list, &s);
}

char*  luaxt_pullcallback( void )
{
    free(temp_str); temp_str=0;
    char **s =  m_pop(cb_list);
    return s ? temp_str = *s : "";
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
    XtVaSetValues( w, res, val, NULL );
}
