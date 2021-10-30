#include "mls.h"
#include "m_tool.h"

#include <stdint.h>
#include <time.h>

#define HASH_SIZE (4*8)
#define HASH_BITS 14
#define HASH_TABLE_SIZE (1<<HASH_BITS)
#define HASH_TABLE_MASK (HASH_TABLE_SIZE-1)


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

int s_memcpy(int dst, int src, int max)
{
    m_clear(dst);
    int src_len = m_len(src);
    max = Min(src_len,max);
    int i; int ch;
    for( i = 0; i < max; i++ ) {
	ch = CHAR( src, i );
	m_putc(dst,ch);
	if( ch == 0 ) break;
    }
    return dst;
}

int HASH_ID=0;
struct hash_id_st {
    char name[HASH_SIZE];
    void *d;
};

int alloc_hash_id( void *ctx, void * a )
{
    if(!HASH_ID) HASH_ID=m_create(100, sizeof(struct hash_id_st) );
    int p = m_new( HASH_ID,1);
    struct hash_id_st *buf = mls( HASH_ID,p);
    memcpy(buf->name,a,HASH_SIZE);
    return p;
}

int cmp_hash_id( void *ctx, int p, void *buf)
{
    struct hash_id_st *old = mls( HASH_ID,p);
    return memcmp( old->name, buf, HASH_SIZE );
}

struct hash_id_st *hash_id( int p )
{
    return mls(HASH_ID, p);
}



// Dedicated to Pippip, the main character in the 'Das Totenschiff' roman, actually the B.Traven himself, his real name was Hermann Albert Otto Maksymilian Feige.
// CAUTION: Add 8 more bytes to the buffer being hashed, usually malloc(...+8) - to prevent out of boundary reads!
// Many thanks go to Yurii 'Hordi' Hordiienko, he lessened with 3 instructions the original 'Pippip', thus:
#define _PADr_KAZE(x, n) ( ((x) << (n))>>(n) )
static uint32_t FNV1A_Pippip_Yurii(const char *str, size_t wrdlen)
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
	return hash32;
} // Last update: 2019-Oct-30, 14 C lines strong, Kaze.


inline static uint32_t simple_hash( void *buf )
{
    return  FNV1A_Pippip_Yurii( buf, HASH_SIZE ) & HASH_TABLE_MASK;
}


/** find or insert variable (buf) in hash-table (hash)    
 */
int hash_lookup( int hash, void *buf,
		 int (*cmpf)(void *ctx, int pos, void *buf),
		 int (*newf)(void *ctx, void *a), void *ctx  )
{
    int p, *d;
    int hash_item;
    uint32_t c = simple_hash(buf);     /* lookup key in hash-table */
    int *hash_item_list = mls(hash, c); /* list of keys with same hash */

    // new entry
    if( *hash_item_list == 0 ) {
	// insert new item-list in hash-table
	*hash_item_list = m_create(1, sizeof(int) );
	goto new_item;	
    }

    // entry found
    // check if the same is already inside
    if(cmpf) { 
	m_foreach( *hash_item_list, p, d ) {
	    if( cmpf(ctx, *d, buf) == 0 ) return *d;
	}
    }
    
 new_item:
    // create new item in item-list in hash-table 
    ASSERT(newf);
    hash_item = newf(ctx,buf);    
    m_put(*hash_item_list, &hash_item);
    return hash_item;
}


int HASH_TABLE = 0;



int hash_simple_key( void* key )
{
    if(!HASH_TABLE) {
	HASH_TABLE=m_create( HASH_TABLE_SIZE, sizeof(int) );
	m_setlen(HASH_TABLE,HASH_TABLE_SIZE );
    }
    return hash_lookup( HASH_TABLE, key,  cmp_hash_id, alloc_hash_id, NULL );
}




int read_keys_from_file( char *filename )
{
    int keys = m_create(1000, sizeof(int) );
    FILE *fp = fopen( filename, "r" );
    ASSERT(fp);
    int buf = m_create(100,1);
    while( m_fscan2(buf,'\n', fp ) == '\n' ) {
	int sbuf = m_create(HASH_SIZE, 1);
	s_memcpy( sbuf, buf, HASH_SIZE );
	m_put(keys, &sbuf );
	m_clear(buf);
    }
    fclose(fp);
    m_free(buf);
    return keys;
}

static inline void nl(void) { putchar(10); };
void check_hash_speed( int seconds, int keys, int (*lookup)(void *)  )
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
	    lookup( m_buf(INT(keys, keynum)) );
	    clock_gettime(CLOCK_MONOTONIC, &tp1);
	    timespec_diff(&tp1,&tp0,&tp2);	    
	    if( tp2.tv_sec  >= seconds ) break;
	}


    
    long double n = tp2.tv_sec;
    n = n * 1E6 + (tp2.tv_nsec / 1E3); 
    n = (count * 1.0) / n;
    printf("hash/µsec: %Le, hash: %u,  %lu sec, %lu nsec\n\n", n, count, tp2.tv_sec,tp2.tv_nsec);
}


void verify_hash(int keys, int (*lookup)(void*) )
{
    int p, *k;
    m_foreach( keys, p, k ) {
	int hash_key = lookup( m_buf(*k) );
	struct hash_id_st *ent = hash_id( hash_key );

	if( strcmp( m_buf(*k), ent->name ) ) {
	    printf("ERROR Key: %s, hash_key: %s\n", m_str(*k), ent->name );
	    ERR("");
	}
    }
    puts("verify ok" );
}

void hash_stats(char *s)
{

    int collisions=0; int slots_free=0;
    int arrays_used=2;
    int keys = 0;
    int *item_list,p;


    m_foreach(HASH_TABLE,p,item_list) {
	if(*item_list) {
	    arrays_used+= 1;
	    keys += m_len(*item_list);
	    collisions+=m_len(*item_list)-1;
	} else slots_free++;
    }
    puts(s);
    printf("table size:  %d\n", m_len(HASH_TABLE) );
    printf("keys:        %d\n", keys );
    printf("collisions:  %d\n", collisions );
    printf("slots free:  %d\n", slots_free );
    printf("arrays used: %d\n", arrays_used );
}

int SORTED_KEYS;
int sk_cmp(const void *a, const void *b)
{
    return memcmp(a,b,HASH_SIZE);
}

/* struct hash_id_st */
int lookup_sorted_keys( void *key )
{
    if(!SORTED_KEYS) {
	SORTED_KEYS=m_create(4000,sizeof(struct hash_id_st));	
    }
    struct hash_id_st ent;
    memcpy(ent.name,key,HASH_SIZE);
    int pos = m_binsert( SORTED_KEYS, &ent, sk_cmp, 0 );
    return pos;
}


int main()
{
    m_init();
    trace_level=2;

    int keys = read_keys_from_file( "john.txt" );
    printf("number of keys: %d\n", m_len(keys) );
    check_hash_speed(10,keys, hash_simple_key);
    verify_hash( keys, hash_simple_key );
    hash_stats("statistics");

    puts("");
    check_hash_speed(10,keys, lookup_sorted_keys );
    printf("sorted keys: %d\n", m_len(SORTED_KEYS));
    m_free(SORTED_KEYS);
    
    { int p,*k; m_foreach(keys,p,k) m_free(*k); m_free(keys); }
    { int p,*k; m_foreach(HASH_TABLE,p,k) m_free(*k); m_free(HASH_TABLE); }
    m_free(HASH_ID);
    m_destruct();
}
