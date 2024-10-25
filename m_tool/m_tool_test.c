#include "mls.h"
#include "m_tool.h"
#include "conststr.h"


void expect_ms_equals_cstr(int m, const char *s)
{
	if( strncmp( m_str(m), s, m_len(m) ) == 0 ) return;
	WARN("error in test. expected '%s', got '%s'", s, m_str(m)); 
}

struct args {
	int cs_key;
	int cs_val;
};

void create_widget(int arglist)
{
	struct args *d;
	int p;
	m_foreach(arglist,p,d) {
		s_puts(d->cs_key);
		s_puts(d->cs_val);
		putchar(10);
	}	
	putchar(10);
}

void repl_test2(void)
{
	const char *s ="Class: Gauge2, Name: cpustat$$, label: CPU$$, sensor: cpustats, graph: 1000,white:$usage[$$],red:$iowait[$$], gridx: $$";
	int cs_src = s_cstr(s);
	int cs_pat = s_cstr("$$");
	int str=0;
	int p;
	char **d;
	int arglist = m_create(10,sizeof(struct args));
	int args = 0;
	int pair = 0;
	for(int i=0;i<5;i++) {
		int cs_num = cs_printf(0,0, "%d", i );
		str = s_replace(str,cs_src, cs_pat, cs_num, 0 );
		/* generate list of "key:value" strings */
		args = m_str_split( args, m_str(str), ",", 1 );
		m_foreach(args,p,d) {
		        pair = m_str_split( pair, *d, ":", 1 );
			if( m_len(pair) == 2 ) {
				struct args *a = m_add(arglist);
				a->cs_key = s_cstr( STR(pair,0) );
				a->cs_val = s_cstr( STR(pair,1) );				
			}
			m_free_strings(pair,1);
		}
		m_free_strings(args,1);
		create_widget(arglist);
	}
	m_free(str);
	m_free(pair);
	m_free(args);	
	m_free(arglist);
	/* do not free vars with prefix 'cs_' i.e. constant string */
}
void repl_test3(void)
{
	const char *s ="Class: Gauge2|Name: cpustat$$|label: CPU$$|sensor: cpustats|graph: 1000,white:$usage[$$],red:$iowait[$$]|gridx: $$";
	int cs_src = s_cstr(s);
	int cs_pat = s_cstr("$$");
	int cs_sep = s_cstr("|");
	int str=0;
	int p; /* loop counter */
	int *d; /* ptr to m-array handle in list 'args' */
	int arglist = m_create(10,sizeof(struct args));
	int args = 0; /* list of strings 'key:value'*/
	int tmp = 0; /* buffer to split key:value buffer */
	for(int i=0;i<5;i++) {
		int cs_num = cs_printf(0,0, "%d", i );
		str = s_replace(str,cs_src, cs_pat, cs_num, 0 );
		args = s_msplit( args, str, cs_sep );
		m_foreach(args,p,d) {
			int pos = s_index( *d,0, ':' );
			if( pos > 0 ) {
				struct args *a = m_add(arglist);
				tmp = s_trim(s_slice(tmp,0, *d, 0, pos-1) );
				a->cs_key = s_mstr( tmp );
				tmp = s_trim(s_slice(tmp,0, *d, pos+1, -1 ));
				a->cs_val = s_mstr(tmp);
			}
		}
		m_clear_list(args);
		create_widget(arglist);
	}
	m_free(tmp);
	m_free(str);			
	m_free(args);	
	m_free(arglist);
	/* do not free vars with prefix 'cs_' i.e. constant string */
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


	repl_test2();
	repl_test3();
	trim_test();
	conststr_free();
	m_destruct();
}
