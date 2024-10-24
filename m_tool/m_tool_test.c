#include "mls.h"
#include "m_tool.h"



void expect_ms_equals_cstr(int m, const char *s)
{
	if( strncmp( m_str(m), s, m_len(m) ) == 0 ) return;
	WARN("error in test. expected %s, got %s", s, m_str(m)); 
}



int main()
{
	m_init();

	
	int src=s_printf(0,0, "Hello $$ World $$,$$" );
	int d1,d2;

	int p1 = s_printf(0,0, "$$" );
	int p2 = s_printf(0,0, "nice" );
	
	d1 = s_replace(0,src, p1, p2, 0 ); /* all replace */	
	d2 = s_replace(0,src, p1, p2, 1 ); /* single replace */
	
	s_puts( d1 );
	s_puts( d2 );
	expect_ms_equals_cstr( d1, "Hello nice World nice,nice" );
	expect_ms_equals_cstr( d2, "Hello nice World $$,$$" );
	
	s_replace(d1,src, p2, p2, 0 ); /* search 'nice' */
	s_puts( d1 );
	expect_ms_equals_cstr( d1,  "Hello $$ World $$,$$" );

	
	m_free(d1);	m_free(d2); 	m_free(p1); 	m_free(p2); 	m_free(src);
	m_destruct();
}
