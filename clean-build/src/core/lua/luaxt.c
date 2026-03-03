#include "luaxt.h"
#include "X11/ThreadsI.h"
#include "X11/Intrinsic.h"
#include "X11/StringDefs.h"

#include <xtcw/mls.h>
#include <xtcw/xutil.h>
#include <WcCreate.h>

static int cb_list=0;
static char* temp_str=0;
static char* temp_data=0;

void   luaxt_init(void)
{
    if(!cb_list) cb_list=m_create(20,sizeof(char*));
}

void   luaxt_destroy(void)
{
    m_free_strings( cb_list, 0);
    cb_list=0;
    free( temp_str ); temp_str=0; 
    free( temp_data ); temp_data=0;
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
    free(temp_data); temp_data=0;
    if( m_len(cb_list) == 0 ) return "";
    temp_str = *(char **)m_pop(cb_list);
    if( m_len(cb_list) > 0 ) {
        temp_data = *(char **)m_pop(cb_list);
    }
    return temp_str;
}

char* luaxt_pulldata( void )
{
    return temp_data ? temp_data : "";
}

Widget luaxt_nametowidget(char *s)
{
    char buffer[4096];
    if( is_empty(s) || (s[0] == '.' && s[1] == 0) ) return TopLevel;
    if( strlen(s) >= sizeof(buffer)) return 0;
    WcCleanName( s, buffer );
    Widget w = WcFullNameToWidget( TopLevel, buffer );
    if (!w) printf("luaxt_nametowidget: widget %s (%s) NOT FOUND\n", s, buffer);
    return w;
}

#include "task_manager.h"

void *copy_task(task_thread_args_t *targs);

void   luaxt_spawn_copy( char *src, char *dst )
{
    typedef struct {
        char src[4096];
        char dst[4096];
    } copy_args_t;

    copy_args_t *args = malloc(sizeof(copy_args_t));
    strncpy(args->src, src, 4096);
    strncpy(args->dst, dst, 4096);
    task_spawn(copy_task, args);
}

void   luaxt_setvalue( char *name, char *res, char *val )
{
    Widget w = luaxt_nametowidget(name);
    if(!w) return;
    XtVaSetValues( w, XtVaTypedArg, res, XtRString, val, strlen(val)+1, NULL );
}

