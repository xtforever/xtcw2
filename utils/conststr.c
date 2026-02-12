#include "mls.h"
#include <stdarg.h>


/* alphabetisch sortierte liste von mstr */
static int CONSTSTR_DATA=0;
static int CS_ZERO = -1;

static void m_free_list(int m)
{
	int p,*d;
	m_foreach( m, p, d ) {
	  TRACE(1, "STR: %s", CHARP(*d));
	  m_free(*d);
	}
	m_free(m);
}

void  conststr_free(void)
{
  m_free_list( CONSTSTR_DATA);
  CONSTSTR_DATA=0;
}

void conststr_init(void)
{
  if( !  CONSTSTR_DATA ) {
    CONSTSTR_DATA=m_create(10,sizeof(int));
    CS_ZERO = m_create(1,1);
    m_putc(CS_ZERO,0);
    m_puti(CONSTSTR_DATA,CS_ZERO);
  }
}

static int mscmp(const void *a, const void *b)
{
	int k1 = *(const int *)a;
	int k2 = *(const int *)b;
	TRACE(1,"cmp %s %s", CHARP(k1), CHARP(k2));
	return m_cmp(k1,k2);
}
static int mscmpc( const void *a,const void *b )
{
  const char * const *s = a;
  const int *d = b; 
  return -mstrcmp( *d, 0, *s );
}

/* schau nach ob s schon vorhanden ist, wenn ja liefere die vorhandene kopie, sonst fuege ein kopie von s hinzu
   IDEE: x=m_create(); m_put(x,data); z= conststr_lookup(x); free(x); 
 */
int conststr_lookup(int s)
{
  if( s_empty(s) ) return CS_ZERO;
  int p = m_binsert( CONSTSTR_DATA, &s, mscmp, 0 );
  if( p < 0 ) { // schon vorhanden
    return INT( CONSTSTR_DATA, (-p)-1 );
  }
  TRACE(1,"ADD %d %s", p, CHARP(s) );
  return INT(CONSTSTR_DATA,p) = m_dub(s);
}

int conststr_lookup_c(const char *s)
{
  if( is_empty(s) ) return CS_ZERO;
  union { const char *s; int key; } key;
  key.s = s;
  int p = m_binsert( CONSTSTR_DATA, &key, mscmpc, 0 );
  if( p < 0 ) {
    return INT( CONSTSTR_DATA, (-p)-1 );
  }
  int k = s_printf(0,0, "%s", s );
  INT(CONSTSTR_DATA,p) = k;
  return k;
}


/** @brief use vaprintf to create a a const-string that you do not need to free.
    use our internal buffer('b') to create a string.
    lookup this string.
    if found, return handle to the found string,
    otherwise return 'b', append 'b' and
    alloc a new internal buffer.
*/
int cs_printf( char *format, ... )
{
	va_list ap;
	va_start(ap,format);
	int m = vas_printf( 0,0, format, ap );
	va_end(ap);
	int p = m_binsert( CONSTSTR_DATA, &m, mscmp, 0 );
	if( p < 0 ) { // schon vorhanden
		m_free(m);
		return INT( CONSTSTR_DATA, (-p)-1 );
	}
	TRACE(1,"ADD const[%d]=%d:%s", p, m, CHARP(m) );
	return m;
}


void conststr_stats(void)
{
	int len,*d,p;
	int cnt =  m_len(CONSTSTR_DATA);
	TRACE(4,"Count: %d", cnt );
	len = cnt * (sizeof(int) + sizeof (struct ls_st));
	m_foreach(CONSTSTR_DATA, p, d ) {
		len += m_len(*d);
	}
	TRACE(4,"Memory: %d", len );
}



#ifdef WITH_CONSTR_MAIN

void test_sort_lookup(void)
{
	char *s[] = { "prepare","test10.b", "test1.a", "test1.b" };
	for(int i=0;i<10;i++) {
	  conststr_lookup_c(s[i % 4]);
	}

	for( int i=0; i< 20; i++ ) {
	  int k  = s_printf(0,0, "%s", s[ i % 4] );
	  int k2 = conststr_lookup( k );
	  TRACE(1, "%d != %d", k, k2 );
	  m_free(k);
	}
}

	

int main()
{
  trace_level=1;
  m_init();
  TRACE(1,"start");

  conststr_init();
  test_sort_lookup();
  conststr_free();
  
  m_destruct();
 
}
#endif

