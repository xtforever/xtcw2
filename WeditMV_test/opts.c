#include "opts.h"

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>
#include <X11/Vendor.h>
#include <X11/Xaw/XawInit.h>
#include <stdarg.h>

#include "svar2.h"
#include "mls.h"

// min size is SVAR_MAX
int svar_strdup(char *str)
{
    int l,p,r;
    if( is_empty(str) ) { // return empty buffer
	r = m_create(SVAR_MAX,1);
	m_putc(r,0);
	return r;
    }
    
    l = strlen(str)+1; // return copy of string 
    r = m_create( Max(SVAR_MAX,l), 1);
    m_write(r,0,str,l);
    return r;
}


int register_widget_group( Widget wid, char *group )
{
    int buf = svar_strdup(group);
    int key = svar_lookup( buf, -1 );
    register_widget(wid,key);
    m_free(buf);
    return key;
}

svar_t *svar_t_str_lookup(char *group )
{
    int buf = svar_strdup(group);
    int key = svar_lookup( buf, -1 );
    svar_t *v = svar(key);
    m_free(buf);
    return v;
}

Widget get_active_widget(char *group )
{
    svar_t *v = svar_t_str_lookup(group);
    return (v->key>=0) ? *(Widget*) mls(v->value,v->key) : NULL;
}

void set_default_active_widget(Widget w, int key)
{
    svar_t *v = svar(key);
    int m = v->value;
    int p;
    Widget *d;
    m_foreach( m,p,d ) {
	if( *d == w ) { v->key = p; return; }
    }
    WARN("could not find widget, maybe not registered?");
    v->key = m_put(m, &w);
}


void register_widget(Widget wid, int key)
{
    svar_t *v = svar(key);
    if( v->type == 0 ) {
	v->type = SVAR_MARRAY;
	v->key = -1;
	v->value = m_create(3,sizeof(Widget));
    }
    m_put( v->value, &wid );
}

Widget change_active_widget(Widget new, int key)
{
    int p;
    Widget *w; 
    svar_t *v = svar(key);
    // call deactive/reset
    if( v->key >= 0 ) {
	w = mls(v->value,v->key);
	if( *w != new ) { // deactivate
	    XtCallActionProc( *w, "deactivate", 0,0,0 );
	    TRACE(1,"deactivate %d", v->key );
	}
    }
    
    // call activae
    m_foreach( v->value, p, w ) {
	if( *w == new ) {
	    v->key = p;
	    XtCallActionProc( *w, "activate", 0,0,0 );
	    TRACE(1,"activate %d", p );
	}
    }	
}

int s_strdup(int r, const char *str)
{
    int l,p;
    if( is_empty(str) ) { // return empty buffer
	if( r <= 0 ) r = m_create(1,1); else m_clear(r);
	m_putc(r,0);
	return r;
    }
    
    l = strlen(str)+1; // return copy of string 
    if( r <= 0 ) r = m_create(l,1); else m_clear(r);
    m_write(r,0,str,l);
    return r;
}

static int app_cstr_mstrl( int m, const char *name )
{
    int x = s_strdup(0,name);
    m_put(m,&x);
    return m;
}


static int vas_create_mstring_array(int m, va_list ap )
{
    char *name;
    while( (name = va_arg( ap, char* )) != NULL )
    {
	app_cstr_mstrl( m, name );
    }
    return m;
}


int append_mstring_array(int m,int offset, ... )
{
    va_list ap;
    if(!m) {
	m=m_create(10,sizeof(int));
    } else {
	int len = m_len(m);
	if( offset >= 0 ) {
	    while( offset < len-- ) {
		m_free( INT(m,len) );		
	    }
	}
	m_setlen(m,offset);
    }
    
    va_start(ap,offset);
    vas_create_mstring_array(m,ap);
    va_end(ap);
    return m;
}
