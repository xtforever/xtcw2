#include "repeatergb-tool.h"
#include "m_tool.h"
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
	int xtargl = m_create( count, sizeof(XtTypedArg) );
	XtTypedArg *xtarg;
	int p; /* loop counter */
	int args = 0; /* int[]char[]: list of strings 'key:value'*/
	int tmp =  m_create(10,1); /* char[]: buffer for replace and split */
	int wl=  m_create(count,sizeof(Widget));
	for(int i=0;i<count;i++) {
		int cs_num = cs_printf("%d", i );
		int cs_name = cs_printf("%s-%d", XtName(parent), i );
		/* replace $$ with str(i) and split into 'key:value' substrings */
		args = s_msplit( args, s_replace(tmp, cs_src, cs_pat, cs_num, 0 ), cs_sep );
		/* split  'key:value' into key,value and extract class,name */

		m_foreach(args,p,d) {
			int pos = s_index( *d,0, ':' );
			if( pos <= 0 ) continue;
			int k = s_mstr(s_lower(s_trim(s_slice(tmp,0, *d, 0, pos-1)) ));
			int v =  s_mstr(s_trim(s_slice(tmp,0, *d, pos+1, -1 )));
			TRACE(2,"'%s' = '%s'", m_str(k), m_str(v) );
			if( k == cs_class )  cs_class_name = s_mstr( s_lower(tmp) );
			else {		       
				xtarg=m_add(xtargl);
				xtarg->name  = m_str(k);
				xtarg->value = (XtArgVal) m_str( v );
				xtarg->size  = m_len(v);
				xtarg->type  = XtRString;
			}
		}
		TRACE(2, "Create Widget (class: name) %s: %s", m_str(cs_class_name), m_str(cs_name) );
		/* create widget: lookup WidgetClass by lowercase(ClassName) */
		XtAppContext app = XtWidgetToApplicationContext( parent );
		XrmQuark q = XrmStringToQuark(m_str(cs_class_name));
		WidgetClass class = WcMapClassFind( app, (intptr_t) q );
		if( !class ) {
			ERR("Widget class not found. Resource: %s", s );
		}				
		/* exit if class not found */
		Widget xtw = _XtCreateWidget(m_str(cs_name), class, parent,NULL,0, m_buf(xtargl), m_len(xtargl) );
		m_put(wl, &xtw); /* collect all created widgets in one list */		
		/* free elements in list 'args', reset 'args' to zero length */
		m_clear_list(args);
		/* set 'xtargsl' to zero length */
		m_clear(xtargl);
	}
	XtManageChildren( m_buf(wl), m_len(wl) );
	m_free(tmp);
	m_free_list(args); /* free int[]char[] */	
	m_free(xtargl);
	m_free(wl);
}




void repgb_parse(Widget w, char *args, char *count )
{
	TRACE(4,"%s:%s", args,count );	
	create_widget_from_args( w,args,count);
}

