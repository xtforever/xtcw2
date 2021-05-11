#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mls.h"

/* hash datastruct

   GIVEN str : string to hash


   code = FNV1A_Pippip_Yurii(const char *str, size_t wrdlen)

   code %= HASH_TABLE_SIZE

   SVAR[ code ] = [ p1, p2, ... ]

   p1 : hash-unique_id
   SVAR_DAT[p1] = {   name-ptr : key-name-char-array,
                      var_...  
		  }
 */
static inline const char *m_str(const int m) { return m_buf(m); }


struct svar_signal {
    void *d;
    void (*fn) (void*, int );
};

// must be multiple of 8
#define SVAR_MAX 32

typedef struct svar_st {
    int name;
    int value;
    uint8_t type, locked;
    int write_callbacks;
    int read_callbacks;
} svar_t;

int SVAR, SVAR_DAT;
#define SVAR_TABLE_SIZE (0xfff)
void svar_init();
void svar_destroy();
void svar_onwrite( int q, void (*fn) (void*, int), void *d, int remove );
svar_t *svar(int q);
int* svar_value(int q);
uint8_t* svar_type(int q);
void svar_write(int q, int d);
int svar_zero(char *key);

// Dedicated to Pippip, the main character in the 'Das Totenschiff' roman, actually the B.Traven himself, his real name was Hermann Albert Otto Maksymilian Feige.
// CAUTION: Add 8 more bytes to the buffer being hashed, usually malloc(...+8) - to prevent out of boundary reads!
// Many thanks go to Yurii 'Hordi' Hordiienko, he lessened with 3 instructions the original 'Pippip', thus:
#define _PADr_KAZE(x, n) ( ((x) << (n))>>(n) )
uint32_t FNV1A_Pippip_Yurii(const char *str, size_t wrdlen)
{
	const uint32_t PRIME = 591798841; uint32_t hash32; uint64_t hash64 = 14695981039346656037UL;
	size_t Cycles, NDhead;
	if (wrdlen > 8) {
		Cycles = ((wrdlen - 1)>>4) + 1; NDhead = wrdlen - (Cycles<<3);

        	for(; Cycles--; str += 8) {
			hash64 = ( hash64 ^ (*(uint64_t *)(str)) ) * PRIME;
			hash64 = ( hash64 ^ (*(uint64_t *)(str+NDhead)) ) * PRIME;
		}
	} else {
		hash64 = ( hash64 ^ _PADr_KAZE(*(uint64_t *)(str+0), (8-wrdlen)<<3) ) * PRIME;
	}
	hash32 = (uint32_t)(hash64 ^ (hash64>>32));
	hash32 ^= (hash32 >> 16);
	return hash32 & SVAR_TABLE_SIZE;
} // Last update: 2019-Oct-30, 14 C lines strong, Kaze.


uint32_t svar_hash(int sbuf)
{
    return FNV1A_Pippip_Yurii( m_buf(sbuf), m_len(sbuf) );
}

svar_t* svar( int key )
{
    return mls(SVAR_DAT,key);
}

#if 1
int s_strcpy(int dst, int src, int max)
{
    m_clear(dst);
    int src_len = m_len(src);
    int i; int ch;
    for( i = 0; i < max -1; i++ ) {
	if( i >= src_len ) break;	
	ch = CHAR( src, i );
	if( ch == 0 ) break;
	m_putc(dst,ch);
    }
    m_putc(dst,0);
    return m_len(dst);
}
#else

int s_strcpy(int dst, int src, int max)
{
    assert( m_bufsize(dst) >= max );
    if( m_bufsize(dst) < max ) {
	m_setlen(dst,max);
    }


    int c = Min(max,m_len(src));
    memcpy( m_buf(dst), m_buf(src), c-1 );
    m_setlen( dst, c );
    CHAR(dst,c-1)=0;

    return c;
}
#endif



int new_svar(int buf)
{
   int hash_item = m_new( SVAR_DAT, 1);
   svar_t *v = mls(SVAR_DAT, hash_item);
   memset(v,0,sizeof(*v));
   v->name = m_create( SVAR_MAX, 1 );	
   s_strcpy( v->name, buf, SVAR_MAX );
   return hash_item;
}

int check_hash2(int buf, int byte_compare )
{
    uint32_t c = svar_hash(buf);
    assert( c < SVAR_TABLE_SIZE );
    int hash_item;
    int hash_item_list = INT(SVAR, c );

    // new entry
    if( hash_item_list == 0 ) {
	hash_item_list = m_create(1, sizeof(int) );
	INT(SVAR, c ) = hash_item_list;
	goto new_item;	
    }

    if( m_len( hash_item_list ) == 1 &&
	! byte_compare )
	{ 
	    return INT(hash_item_list,0);
	}
    
    // entry found
    // check if the same is already inside
    int p, *d;
    m_foreach( hash_item_list, p, d ) {
	svar_t *v = mls(SVAR_DAT, *d);
	if( m_cmp( v->name, buf ) == 0 ) return *d;
    }

 new_item:
    // new item
    hash_item = new_svar(buf);    
    m_put(hash_item_list, &hash_item);
    return hash_item;
}


int hash_to_string( int hash_item )
{
    return svar(hash_item)->name;
}

int string_to_hash( int sbuf )
{
    ASSERT( m_width(sbuf) == 1 );
    int old_len = m_len(sbuf);
    int pad = (m_bufsize(sbuf) & 0x0f);
    if( pad  ) {
	m_setlen( sbuf, m_bufsize(sbuf) + (8-pad) );
	m_setlen( sbuf, old_len );
    }
    
    return check_hash2(sbuf, 1);
}

int string_to_hash_fast( int sbuf )
{
    return check_hash2(sbuf, 0);
}

void svar_free(void)
{
    int *item_list,p;
    m_foreach(SVAR,p,item_list) m_free(*item_list);
    m_free(SVAR);
    svar_t *item;
    m_foreach(SVAR_DAT, p, item ) {
	m_free( item->name );
	m_free( item->read_callbacks );
	m_free( item->write_callbacks );	
    }
    m_free( SVAR_DAT ); 
}
/**
 * @fn timespec_diff(struct timespec *, struct timespec *, struct timespec *)
 * @brief Compute the diff of two timespecs, that is a - b = result.
 * @param a the minuend
 * @param b the subtrahend
 * @param result a - b
 */
static inline void timespec_diff(struct timespec *a, struct timespec *b,
    struct timespec *result) {
    result->tv_sec  = a->tv_sec  - b->tv_sec;
    result->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (result->tv_nsec < 0) {
        --result->tv_sec;
        result->tv_nsec += 1000000000L;
    }
}

void svar_onwrite( int q, void (*fn) (void*, int), void *d, int remove )
{
    int p;
    struct svar_signal *sig;
    svar_t* ent = svar(q);
    if(!ent->write_callbacks)
	ent->write_callbacks = m_create( 2, sizeof(struct svar_signal));
    else {
	/* finde signal und falls remove==TRUE entfernen */
	m_foreach( ent->write_callbacks, p, sig )
	    {
		if( sig->fn == fn ) {
		    if( remove ) m_del(ent->write_callbacks,p);
		    return;
		}
	    }
    }
    if( remove ) return;

    /* signal noch nicht vorhanden, remove==FALSE, jetzt hinzufügen */
    sig = m_add( ent->write_callbacks );
    sig->d = d;
    sig->fn = fn;
}

/** write to var q and call signal handler
 **/
void svar_write( int q, int data )
{
    int p;
    struct svar_signal *sig;
    svar_t* ent = svar(q);
    ent->value = data;
    if(! ent->write_callbacks ) return;
    
    /* um rekursion zu verhindern wird signal blockiert */
    if( ent->locked ) return;
    ent->locked = 1;
    m_foreach( ent->write_callbacks, p, sig ) {
	if( sig->fn ) sig->fn( sig->d, q );
    }
    ent->locked = 0;
}

struct ctx {
    int keys;
    int num;
    int hash;
};

const char *svar_name( int v )
{
    return m_str( svar(v)->name );
}
int *svar_value( int v )
{
    return & svar(v)->value ;
}


void key_callback(void *c, int v )
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
    int k  = string_to_hash( INT(keys,v1 ) );
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
	int hash_key = string_to_hash( *k );
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
	    string_to_hash( INT(keys, keynum) );
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


void check_svar(void)
{
    int p;
    svar_t *k;
    m_foreach( SVAR_DAT, p, k ) {
	if( k->read_callbacks || k->write_callbacks ) ERR("");
    }
    puts("check svar ok");
}

int cstring_to_hash( char *s )
{

	int sbuf = m_create(SVAR_MAX, 1);
	s_printf(sbuf,0, s );
	int x = string_to_hash(sbuf);
	m_free(sbuf);
	return x;
}

void test_sstrcopy(void)
{
    return;
    int src = m_create(32, 1);
    s_printf(src,0, "hello" );
    int dst = m_create(32,1);
    s_strcpy( dst, src, 32 );
    printf("%s %s\n", m_str(src), m_str(dst) );
    // m_destruct();
    // exit(1);
}

int main()
{
	m_init();
	trace_level=2;
	test_sstrcopy();
	int sbuf = m_create(SVAR_MAX, 1);
	SVAR = m_create(SVAR_TABLE_SIZE+1, sizeof(int) );
	m_setlen( SVAR, SVAR_TABLE_SIZE+1 );
	SVAR_DAT = m_create(100, sizeof(svar_t) );
	check_svar();
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

	check_svar();
	x = cstring_to_hash( "internet" );
	svar(x)->value = 100;
	printf("%s=%d\n", (char*) m_buf( hash_to_string(x) ), svar(x)->value );
	check_svar();

	check_callbacks(keys);

	
	m_free(sbuf);
	svar_free();
	{ int p,*k; m_foreach(keys,p,k) m_free(*k); m_free(keys); }
	m_destruct();
	printf("success\n");
}

