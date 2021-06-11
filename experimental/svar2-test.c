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
	int sbuf = s_memcpy( 0, buf, SVAR_MAX );
	m_put(keys, &sbuf );
	m_clear(buf);
    }
    fclose(fp);
    m_free(buf);
    return keys;
}

static inline void nl(void) { putchar(10); };
void check_hash_speed( int seconds, int keys  )
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
	    svar_lookup( INT(keys, keynum), SVAR_FLOAT );
	    clock_gettime(CLOCK_MONOTONIC, &tp1);
	    timespec_diff(&tp1,&tp0,&tp2);	    
	    if( tp2.tv_sec  >= seconds ) break;
	}
    
    long double n = tp2.tv_sec;
    n = n * 1E6 + (tp2.tv_nsec / 1E3); 
    n = (count * 1.0) / n;
    printf("hash/µsec: %Le, hash: %u,  %lu sec, %lu nsec\n\n", n, count, tp2.tv_sec,tp2.tv_nsec);
}


/* lookup key 
 * key1@key2@key3@key4 
 * monster@t1 
 * monster - lookup create svar-array, create hash table
 *         - k=lookup t1, add k, return k
 * recursive!
 */

   
void dump_svar(int key);

void print_svar_value(int t, void *val )
{
    t &= ~ SVAR_ARRAY;
    int x = *(int*) val;
    switch( t ) {
    case SVAR_INT: printf("%d", x); break;
    case SVAR_FLOAT: printf("%f", *(float*) val); break;
    case SVAR_MSTRING: printf("%s", m_str(x)); break;
    case SVAR_MARRAY: printf("%d %d %d", x, m_len(x), m_width(x)); break;
    case SVAR_SVAR: dump_svar(x); break;
    }
}

void dump_svar(int key)
{
    int p,*d;
    int t = *svar_type(key) & 0x0f;
    printf( "svarname: %s:\n", svar_name(key));
    printf("TYPE: (%d) %s\n", t, svar_typename(key) );


    if ( svar_is_array(t) ) {
	printf("----------------\n");
	m_foreach(*svar_value(key),p,d) {
	    print_svar_value( t, d );
	}
	printf("----------------\n");
	return;
    }	

    printf("value: "  );
    print_svar_value( t, svar_value(key) );
    printf("\n" );    
}





/* void type_test(void) */
/* { */
/*     trace_level=0; */
/*     int vs = s_create();   */
/*     int key = s_lookup_str( vs, "myVar" ); */
/*     dump_svar(key); */

/*     int mstr = s_printf(0,0,"Hello"); */
/*     uint8_t *t = svar_type(key); */
/*     *t = (*t & 0x0f) | (SVAR_MSTRING << 1); */
/*     svar_write(key, mstr ); */
/*     dump_svar(key); */
/*     int other_key = key; */
    
/*     key = s_lookup_str( vs, "myVarArray" ); */
/*     t = svar_type(key); */
/*     *t = (*t & 0x0f) | (SVAR_SVAR << 1) | SVAR_ARRAY; */
/*     int val = m_create(10,sizeof(int)); */
/*     m_put(val, &other_key); */
/*     svar_write(key, val ); */
/*     dump_svar(key); */


/*     key = s_lookup_str( vs, "myVarArrayInt" ); */
/*     t = svar_type(key); */
/*     *t = (*t & 0x0f) | (SVAR_INT << 1) | SVAR_ARRAY; */
/*     val = m_create(10,sizeof(int)); */
/*     for(int i=10;i<20;i++) { */
/* 	m_put(val, &i); */
/*     } */
/*     svar_write(key, val ); */
/*     dump_svar(key); */


/*     key = s_lookup_str( vs, "myVarArrayInt" ); */
/*     val = *svar_value(key); */
/*     m_put(val, &key); */
/*     dump_svar(key); */
    
    

/*     key = s_lookup_str( vs, "myFloat" ); */
/*     t = svar_type(key); */
/*     *t = (*t & 0x0f) | (SVAR_FLOAT << 1); */
/*     float *fval = (float*) svar_value(key); */
/*     *fval = 3.141; */
/*     dump_svar(key); */

/*     /\* */
/*       key = s_lookup_str_typed( vs, "myFloat", SVAR_FLOAT ); */
/*       svar_float_set(key, 0, 3.141 ); */
/*       dump_svar(key); */

/*       key = s_lookup_str_typed( vs, "myFloat", SVAR_FLOAT_ARRAY ); */
/*       svar_float_set(key, -1, 3.141 ); */
/*       svar_float_set(key, -1, 6.941 ); */
/*       dump_svar(key); */
/*     *\/ */
    
/*     s_free(vs); */
/* } */



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

void check1()
{
    int name = s_printf(0,0, "hello2" );
    int key = svar_lookup( name,-1 );
    printf("found: %s %d\n", m_str(name), key );
    dump_svar(key);
    
    s_printf(name,0, "hello.test1.test2" );
    int key2 = svar_lookup( name, -1 );
    printf("found: %s %d\n", m_str(name), key2 );

    dump_svar(key);

    s_printf(name,0, "hello.test1" );
    int key3 = svar_lookup( name,-1 );
    printf("found: %s %d\n", m_str(name), key3 );

    dump_svar(key);
    
    trace_level=1;
    svar_free(key);
    trace_level=2;
        
    m_free(name);
    printf("READY____________________________________________\n\n");

}



int main()
{
    m_init();
    trace_level=0;
    svar_create();
    // check1();


    int name = s_printf(0,0, "hello2.hello3.hello4" );
    int key = svar_lookup( name,-1 );
    s_printf(name,0, "hello2.hello3.hello4b" );
    key = svar_lookup( name,-1 );

    s_printf(name,0, "hello2.hello3" );
    key = svar_lookup( name,-1 );

    dump_svar(key);

    m_free(name);

    int keys = read_keys_from_file( "john.txt" ); 
    check_hash_speed( 5, keys  );

    m_free_list_of_list(keys);


    svar_destruct();
    m_destruct();
}
