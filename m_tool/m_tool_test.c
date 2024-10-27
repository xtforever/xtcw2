#include "mls.h"
#include "m_tool.h"
#include "conststr.h"


void expect_ms_equals_cstr(int m, const char *s)
{
	if( strncmp( m_str(m), s, m_len(m) ) == 0 ) return;
	WARN("error in test. expected '%s', got '%s'", s, m_str(m)); 
}


void create_widget(int tl)
{
	int vclass=0, cs_class = s_cstr("class");
	int vname=0, cs_name = s_cstr("name");
	for( int i=0;i<m_len(tl);i+=2) {
		int k = INT(tl,i), v = INT(tl,i+1);
		if( k == cs_class )  vclass=v;
		else if( k == cs_name )  vname=v;
		else continue;
		m_remove(tl,i,2); i-=2;
	}
	printf("Create Widget: %s %s, with args:\n", m_str(vclass), m_str(vname) );
	for( int i=0;i<m_len(tl);i+=2) {
		int k = INT(tl,i), v = INT(tl,i+1);	
		printf("%s: %s\n" , m_str(k), m_str(v) );
	}
	putchar(10);		
}


void repl_test3(void)
{
	int *d; /* ptr to m-array handle in list 'args' */
	const char *s ="gridy: $$|"	
		"Class: Gauge2|Name: cpustat$$|"
		"label: CPU$$|sensor: cpustats|"
		"graph: 1000,white:$usage[$$],red:$iowait[$$]";	
	int cs_src = s_cstr(s); /* const-m-string */
	int cs_pat = s_cstr("$$");
	int cs_sep = s_cstr("|");
	int p; /* loop counter */
	int tupellist = m_create(2,sizeof(int));
	int args = 0; /* int[]char[]: list of strings 'key:value'*/
	int tmp =  m_create(10,1); /* char[]: buffer for replace and split */
	for(int i=0;i<5;i++) {
		int cs_num = cs_printf("%d", i );
		args = s_msplit( args, s_replace(tmp, cs_src, cs_pat, cs_num, 0 ), cs_sep );
		m_foreach(args,p,d) {
			int pos = s_index( *d,0, ':' );
			if( pos <= 0 ) continue;
			int k = s_mstr(s_lower(s_trim(s_slice(tmp,0, *d, 0, pos-1)) ));
			m_puti( tupellist, k);
			int v = s_mstr(s_trim(s_slice(tmp,0, *d, pos+1, -1 )));
			m_puti( tupellist, v);
		}
		/* free elements in list 'args', reset 'args' to zero length */
		m_clear_list(args);
		create_widget(tupellist); /* dummy function */
		m_clear(tupellist);
	}
	m_free(tmp);
	m_free_list(args); /* free int[]char[] */	
	m_free(tupellist);
}


void trim_test(void)
{
	int p2;
	p2 = s_printf(0,0, "  nice  " );
	s_trim(p2);
	expect_ms_equals_cstr( p2, "nice" );
	p2 = s_printf(p2,0, "nice  " );
	s_trim(p2);
	expect_ms_equals_cstr( p2, "nice" );
	p2 = s_printf(p2,0, "  nice" );
	s_trim(p2);
	expect_ms_equals_cstr( p2, "nice" );
	p2 = s_printf(p2,0, "    " );
	s_trim(p2);
	expect_ms_equals_cstr( p2, "" );
	p2 = s_printf(p2,0, "  n  " );
	s_trim(p2);
	expect_ms_equals_cstr( p2, "n" );
	m_free(p2);
}



int main()
{
	m_init();
	conststr_init();
	trace_level=4;
	repl_test3();
	goto leave;

	
	int src=s_printf(0,0, "$Hello $$ World $$,$$" );
	int d1,d2;

	int p1 = s_printf(0,0, "$$" );
	int p2 = s_printf(0,0, "nice" );

	ASSERT( m_width(src) == 1 );
	ASSERT( m_width(p1) == 1 );
	ASSERT( m_width(p2) == 1 );	
	
	
	d1 = s_replace(0,src, p1, p2, 0 ); /* all replace */	
	d2 = s_replace(0,src, p1, p2, 1 ); /* single replace */
	
	s_puts( d1 );
	s_puts( d2 );
	expect_ms_equals_cstr( d1, "$Hello nice World nice,nice" );
	expect_ms_equals_cstr( d2, "$Hello nice World $$,$$" );
	
	s_replace(d1,src, p2, p2, 0 ); /* search 'nice' */
	s_puts( d1 );
	expect_ms_equals_cstr( d1,  "$Hello $$ World $$,$$" );

	
	m_free(d1);	m_free(d2); 	m_free(p1); 	m_free(p2); 	m_free(src);

	trim_test();
	// repl_test2();

leave:
	conststr_free();
	m_destruct();
}
