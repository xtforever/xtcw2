#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mls.h"
#include "svar.h"


struct ctx {
    int keys;
    int num;
    int hash;
};

static void key_callback(void *c, int v )
{
    struct ctx *ctx = c;
    // svar_t *s = svar(v);
    printf("value: %d, svar: %d, name: %s, hash: %d, keys[%d]=%s\n",
	   *svar_value(v),
	   v, svar_name(v), ctx->hash, ctx->num, m_str( INT(ctx->keys,ctx->num) ) );
    
}

// add callback to some vars, then write to var 
void check_callbacks(int keys)
{
    int v1 = m_len(keys) / 2;
    int k  = svar_key( INT(keys,v1 ) );
    struct ctx ctx;
    ctx.keys = keys;
    ctx.num  = v1;
    ctx.hash = k;
    svar_onwrite( k, key_callback, &ctx, 0 );
    svar_write( k, 100 );
    svar_write( k, 101 );
}


int m_memory(int h)
{
    return  h ? sizeof(void*)          // 1 entry in MLS   
	+ sizeof( struct ls_st )      // header
	+ m_bufsize(h)*m_width(h) : 0;    // array data
}


void hash_stats(char *s)
{

    int collisions=0; int slots_free=0;
    int arrays_used=2;
    int keys = 0;
    int *item_list,p;
    int SVAR,SVAR_DAT;
    svar_get_internals(&SVAR,&SVAR_DAT);
    
    int mem = m_memory(SVAR) + m_memory(SVAR_DAT);
    m_foreach(SVAR,p,item_list) {
	if(*item_list) {
	    mem += m_memory(*item_list);
	    arrays_used+= 1;
	    keys += m_len(*item_list);
	    collisions+=m_len(*item_list)-1;
	    svar_t *s;
	    int n, *d;
	    m_foreach(*item_list,n,d) {
		s = svar(*d);
		mem += m_memory(s->name);
		mem += m_memory(s->read_callbacks);
		mem += m_memory(s->write_callbacks);
	    }
	} else slots_free++;
    }
    puts(s);
    printf("table size:  %d\n", m_len(SVAR) );
    printf("keys:        %d\n", keys );
    printf("collisions:  %d\n", collisions );
    printf("slots free:  %d\n", slots_free );
    printf("arrays used: %d\n", arrays_used );
    printf("mem used:    %d\n\n", mem  );    
}

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

void verify_hash(int keys )
{
    int p, *k;
    m_foreach( keys, p, k ) {
	int hash_key = svar_key( *k );
	int name = svar(hash_key)->name;
	if( m_cmp( *k, name ) ) {
	    printf("ERROR Key: %s, hash_key: %s\n", m_str(*k), m_str(name) );
	    ERR("");
	}
    }
    puts("verify ok" );
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
	    svar_key( INT(keys, keynum) );
	    clock_gettime(CLOCK_MONOTONIC, &tp1);
	    timespec_diff(&tp1,&tp0,&tp2);	    
	    if( tp2.tv_sec  >= seconds ) break;
	}
    hash_stats( "" );

    
    long double n = tp2.tv_sec;
    n = n * 1E6 + (tp2.tv_nsec / 1E3); 
    n = (count * 1.0) / n;
    printf("hash/µsec: %Le, hash: %u,  %lu sec, %lu nsec\n\n", n, count, tp2.tv_sec,tp2.tv_nsec);
}



void test_sstrcopy(void)
{
    return;
    int src = m_create(3, 1);
    s_printf(src,0, "hello" );
    int dst = m_create(2,1);
    s_strcpy( dst, src, 32 );
    if( m_cmp(src,dst) ) ERR("s_strcpy corrupted");
    printf("%s %s\n", m_str(src), m_str(dst) );
}


int main()
{
	m_init();
	trace_level=2;
	test_sstrcopy();
	svar_init();
	
	
	int sbuf = m_create(SVAR_MAX, 1);
	int keys = read_keys_from_file( "john.txt" );
	hash_stats( "empty hash líst" );
	s_printf(sbuf,0, "internet" );
	int x = string_to_hash_fast(sbuf);
	printf("%d %s\n", x, svar_name(x) );
	hash_stats( "just on entry in hash líst" );

	check_hash_speed( 1, keys );
	check_hash_speed( 2, keys );
	check_hash_speed( 4, keys );

	verify_hash(keys);

	
	x = svar_key_cstr( "internet" );
	svar(x)->value = 100;
	printf("%s=%d\n", svar_name(x), *svar_value(x) );
	

	check_callbacks(keys);

	
	m_free(sbuf);
	svar_free();
	{ int p,*k; m_foreach(keys,p,k) m_free(*k); m_free(keys); }
	m_destruct();
	printf("success\n");
}

