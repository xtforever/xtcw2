#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>
#include <X11/Vendor.h>
#include <X11/Xaw/XawInit.h>
#include "svar2.h"
#include "mls.h"

void register_widget(int key, Widget wid )
{
    svar_t *v = svar(key);
    if( v->type == 0 ) {
	v->type = SVAR_MARRAY;
	v->key = -1;
	v->value = m_create(3,sizeof(widget));
    }
    m_put( v->value, &wid );
}

Widget change_active_widget( int key, Widget new )
{
    Widget *w; 
    svar_t *v = svar(key);
    // call deactive
    if( v->key >= 0 ) {
	w = mls(v->value,v->key);
	if( *w != new ) { // deactivate
	    XtCallActionProc( *w, "reset", 0,0,0 );
	    TRACE(1,"deactivate %d", v->key );
	}
    }
    
    // call activae
    m_foreach( v->value, p, w ) {
	if( *w == new ) {
	    v->key = p;
	    XtCallActionProc( *w, "highlight", 0,0,0 );
	    TRACE(1,"activate %d", p );
	}
    }	
}

    
