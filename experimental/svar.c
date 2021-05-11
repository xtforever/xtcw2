#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mls.h"
#include "svar.h"

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

static int SVAR, SVAR_DAT;

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
	return hash32 & SVAR_TABLE_SIZE;
} // Last update: 2019-Oct-30, 14 C lines strong, Kaze.


static uint32_t svar_hash(int sbuf)
{
    return FNV1A_Pippip_Yurii( m_buf(sbuf), m_len(sbuf) );
}

svar_t* svar( int key )
{
    return mls(SVAR_DAT,key);
}


static int svar_new(int buf)
{
   int hash_item = m_new( SVAR_DAT, 1);
   svar_t *v = mls(SVAR_DAT, hash_item);
   memset(v,0,sizeof(*v));
   v->name = m_create( SVAR_MAX, 1 );	
   s_strcpy( v->name, buf, SVAR_MAX );
   return hash_item;
}

/**  
 */
int check_hash2(int buf, int byte_compare )
{
    int hash_item;

    // lookup key in hash-table
    uint32_t c = svar_hash(buf);
    assert( c < SVAR_TABLE_SIZE );
    int hash_item_list = INT(SVAR, c );

    // new entry
    if( hash_item_list == 0 ) {
	// insert new item in hash-table
	hash_item_list = m_create(1, sizeof(int) );
	INT(SVAR, c ) = hash_item_list;
	goto new_item;	
    }

    // only check for hash-code and return hash-value aka key for svar
    // if byte_compare is false and only one entry inside
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
    // create new item in hash-table 
    hash_item = svar_new(buf);    
    m_put(hash_item_list, &hash_item);
    return hash_item;
}

int hash_to_string( int hash_item )
{
    return svar(hash_item)->name;
}

// convert mstring to hash-code
// if buffer is not a multiple of 8 resize buffer
static int string_to_hash( int sbuf )
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

int svar_key(int buf)
{
    return string_to_hash(buf);
}
const char *svar_name( int v )
{
    return m_str( svar(v)->name );
}

int *svar_value( int v )
{
    return & svar(v)->value ;
}

uint8_t *svar_type( int v )
{
    return & svar(v)->type ;
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

static int svar_find_signal_handler( int m, void (*fn) (void*, int) )
{
    int p;
    struct svar_signal *sig;
    /* finde signal und falls remove==TRUE entfernen */
    m_foreach( m, p, sig ) {
	if( sig->fn == fn ) {
	    return p;
	}
    }
    return -1;
}

void svar_onwrite( int q, void (*fn) (void*, int), void *d, int remove )
{
    int p;
    struct svar_signal *sig;
    svar_t* ent = svar(q);
    
    p = svar_find_signal_handler( ent->write_callbacks, fn );

    if( remove ) {
	if( p >=0 ) m_del(ent->write_callbacks, p);
	return;
    }

    if( p >= 0 ) return; // signal schon vorhanden
    
    if(!ent->write_callbacks)
	ent->write_callbacks = m_create( 1, sizeof(struct svar_signal));

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



static int cstring_to_hash( const char *b )
{
    int s,c;
    s = s_printf(0,0, (char*)b );
    c = string_to_hash( s ); 
    m_free(s);
    return c;
}

int svar_key_cstr( const char *s )
{
    return cstring_to_hash(s);
}

void svar_init(void)
{
    SVAR = m_create(SVAR_TABLE_SIZE+1, sizeof(int) );
    m_setlen( SVAR, SVAR_TABLE_SIZE+1 );
    SVAR_DAT = m_create(100, sizeof(svar_t) );    
}


void svar_get_internals( int *svar, int *svar_dat )
{
    *svar=SVAR;
    *svar_dat=SVAR_DAT;
}

