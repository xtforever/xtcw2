#include "repeatergb-tool.h"
#include "m_tool.h"
#include "xutil.h"
#include "conststr.h"
#include <X11/IntrinsicP.h> /*  XtTypedArg */
#include "WcCreateP.h"
#include "xtcw/Gauge2.h"

/* defined in libxt:create.c */
Widget _XtCreateWidget(String name,
                WidgetClass widget_class,
                Widget parent,
                ArgList args,
                Cardinal num_args,
                XtTypedArgList typed_args,
                Cardinal num_typed_args);

static void create_widget_from_args(Widget parent, const char *s, char *count_str )
{
	int count =  atoi(count_str);
	if( count > 100 || count < 1 ) {
		WARN("Widget count is: %d", count );
		return;
	}
	
	int *d; /* ptr to m-array handle in list 'args' */
//	const char *s ="gridy: $$|"	
//		"Class: Gauge2|Name: cpustat$$|"
//		"label: CPU$$|sensor: cpustats|"
//		"graph: 1000,white:$usage[$$],red:$iowait[$$]";	
	int cs_src = s_cstr(s); /* const-m-string */
	int cs_pat = s_cstr("$$");
	int cs_sep = s_cstr("|");
	int cs_class_name, cs_class = s_cstr("class");
	int p; 
	int args = 0; /* int[]char[]: list of strings 'key:value'*/
	int tmp =  m_create(10,1); /* char[]: buffer for replace and split */
	int wl=  m_create(count,sizeof(Widget));
	int xargs = m_create(10,sizeof(int)); /* key,value resource list for createwidget */

	/* foreach widget */
	for(int i=0;i<count;i++) {
		int cs_num = cs_printf("%d", i );
		int cs_name = cs_printf("%s-%d", XtName(parent), i );
		/* replace $$ with str(i), write res to tmp, and split tmp into 'key:value' substrings */
		args = s_msplit( args, s_replace(tmp, cs_src, cs_pat, cs_num, 0 ), cs_sep );
		/* split  'key:value' into key,value strings, and extract class,name,
		   put  key,value strings into a list */
		m_foreach(args,p,d) {
			int pos = s_index( *d,0, ':' );
			if( pos <= 0 ) continue;
			int k = s_mstr(s_lower(s_trim(s_slice(tmp,0, *d, 0, pos-1)) ));
			int v = s_mstr(s_trim(s_slice(tmp,0, *d, pos+1, -1 )));
			TRACE(2,"'%s' = '%s'", m_str(k), m_str(v) );
			if( k == cs_class ) {
				cs_class_name = s_mstr( s_lower(tmp) );
			}
			else {		       
				m_puti(xargs,k);
				m_puti(xargs,v);
			}
		}
		Widget xtw = XtArgCreateWidget(cs_name, cs_class_name, parent, xargs );
		m_clear(xargs);
		m_put(wl, &xtw); /* collect all created widgets in one list */		
		m_clear_list(args); /* free elements in 'args', set 'args' to zero length */
	}
	XtManageChildren( m_buf(wl), m_len(wl) );
	m_free(tmp);
	m_free_list(args); /* free int[]char[] */	
	m_free(xargs);
	m_free(wl);
}




void repgb_parse(Widget w, char *args, char *count )
{
	TRACE(4,"%s:%s", args,count );	
	create_widget_from_args( w,args,count);
}

