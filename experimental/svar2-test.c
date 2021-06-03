#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mls.h"
#include "svar2.h"

char *mstr(int k) { return m_buf(k); }


int read_keys_from_file( char *filename )
{
    int keys = m_create(1000, sizeof(int) );
    FILE *fp = fopen( filename, "r" );
    assert(fp);
    int buf = m_create(100,1);
    while( m_fscan2(buf,'\n', fp ) == '\n' ) {
	int sbuf = m_create(SVAR_MAX, 1);
	s_strcpy( sbuf, buf, SVAR_MAX );
	m_put(keys, &sbuf );
	m_clear(buf);
    }
    fclose(fp);
    m_free(buf);
    return keys;
}

static inline void nl(void) { putchar(10); };
void check_hash_speed( int seconds, int keys, int vs  )
{
    printf("trying %d seconds hashing\n",seconds );
    struct timespec tp0,tp1, tp2; // tv_sec, tv_nsec
    clock_gettime(CLOCK_MONOTONIC, &tp0);
    srand( (int) tp0.tv_nsec );
    unsigned count=0;
    while(1)
	{
	    count++;
	    int keynum = rand() % m_len(keys);
	    s_lookup( vs, INT(keys, keynum) );
	    clock_gettime(CLOCK_MONOTONIC, &tp1);
	    timespec_diff(&tp1,&tp0,&tp2);	    
	    if( tp2.tv_sec  >= seconds ) break;
	}
    
    long double n = tp2.tv_sec;
    n = n * 1E6 + (tp2.tv_nsec / 1E3); 
    n = (count * 1.0) / n;
    printf("hash/µsec: %Le, hash: %u,  %lu sec, %lu nsec\n\n", n, count, tp2.tv_sec,tp2.tv_nsec);
}


void dump_svar(int key)
{
    int ch=0;
    printf( "%s: [", m_str( s_kget(key,0)));
    for( int p=1; p < s_klen(key); p++ ) {
	if( ch ) putchar(ch);
	ch=',';
	printf( " %s", m_str( s_kget(key,p)));
    }
    printf(" ]\n");

    printf("TYPE: %s\n", svar_typename(key) );
    int t = *svar_type(key);
    t &= 0x0f;

    if ( t & 1 ) {
	t >>= 1;
	if( t == SVAR_SVAR ) {
	    printf("----------------\n");
	    int m,p,*d;
	    m=*svar_value(key);
	    m_foreach(m,p,d) {
		dump_svar(*d);
	    }
	    printf("----------------\n");
	    return;
	}	

	if( t == SVAR_INT ) {
	    printf("----------------\n");
	    int m,p,*d;
	    m=*svar_value(key);
	    m_foreach(m,p,d) {
		printf("%d ", *d );
	    }
	    printf("\n----------------\n");
	    return;
	}	

    }

    t >>= 1;
    TRACE(1,"type: %d\n", t );
    if( t == SVAR_MSTRING ) {
	printf("value: %s\n", mstr(*svar_value(key)) );
	return;
    }
    if( t == SVAR_INT ) {
	printf("value: %d\n", *svar_value(key) );
	return;
    }
    
    
    
}


void type_test(void)
{
    trace_level=0;
    int vs = s_create();  
    int key = s_lookup_str( vs, "myVar" );
    dump_svar(key);

    int mstr = s_printf(0,0,"Hello");
    uint8_t *t = svar_type(key);
    *t = (*t & 0x0f) | (SVAR_MSTRING << 1);
    svar_write(key, mstr );
    dump_svar(key);
    int other_key = key;
    
    key = s_lookup_str( vs, "myVarArray" );
    t = svar_type(key);
    *t = (*t & 0x0f) | (SVAR_SVAR << 1) | SVAR_ARRAY;
    int val = m_create(10,sizeof(int));
    m_put(val, &other_key);
    svar_write(key, val );
    dump_svar(key);


    key = s_lookup_str( vs, "myVarArrayInt" );
    t = svar_type(key);
    *t = (*t & 0x0f) | (SVAR_INT << 1) | SVAR_ARRAY;
    val = m_create(10,sizeof(int));
    for(int i=10;i<20;i++) {
	m_put(val, &i);
    }
    svar_write(key, val );
    dump_svar(key);


    key = s_lookup_str( vs, "myVarArray" );
    val = *svar_value(key);
    m_put(val, &key);
    dump_svar(key);


    s_free(vs);
}



int print_stacksize(void);
void print_stats(void)
{
    int a,m,f;
    statistics_svar_allocated(&a, &m, &f);
    printf("allocated: %d\n", a );
    printf("free     : %d\n", f );
    printf("mem      : %d\n", m );
    print_stacksize();
}




int main()
{
    m_init();
    trace_level=2;
    svar_create();

    type_test();
    goto leave;

    int keys = read_keys_from_file( "john.txt" ); 
    int str = s_printf(0,0, "hello world");

    print_stacksize();
    int vs = s_create();

  
    print_stats();
    
    int key = s_lookup_str( vs, "myVar" );
    s_kset(key,str,-1);
    dump_svar(key);

    key = s_lookup_str( vs, "myVar" );
    s_printf(str,0, "hello");
    s_kset(key,str,-1);
    dump_svar(key);

    key = s_lookup_str( vs, "myVar2" );
    s_printf(str,0, "hello");
    s_kset(key,str,-1);
    dump_svar(key);
    print_stats();
    s_free(vs);


    vs = s_create();
    key = s_lookup_str( vs, "myVar" );
    s_printf(str,0, "hello");
    s_kset(key,str,-1);
    dump_svar(key);
    
    print_stats();

    s_free(vs);
    vs = s_create();
    printf("now we should have 0 allocated and 2 free\n");
    print_stats();
    for( int i=0; i < 100; i++ ) {
	s_lookup( vs, INT(keys,i) );
    }
    printf("now we  have 100 allocated and 0 free\n");
    print_stats();

    s_free(vs);
    print_stats();

    
    vs = s_create();


    // check_hash_speed( 100, keys, vs );
    s_free(vs);
    
    
    /*
    key = s_lookup_str( vs, "myVar" );
    int key2 = s_lookup_str( vs, "myVar2" );
    s_kset(key2,str,-1);
    */
    m_free_list_of_list(keys);
    m_free(str);
    s_free(vs);
 leave:
    svar_destruct();
    m_destruct();
}
