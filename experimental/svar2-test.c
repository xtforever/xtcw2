#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mls.h"
#include "svar2.h"


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

    int keys = read_keys_from_file( "john.txt" );
    
    svar_create();

    
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
    svar_destruct();
    m_destruct();
}
