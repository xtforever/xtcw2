#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mls.h"
#include "svar2.h"

/* hash datastruct

   GIVEN str : string to hash

   code = FNV1A_Pippip_Yurii(const char *str, size_t wrdlen)

   code %= HASH_TABLE_SIZE
   
   HASH[ code ] = [ p1, p2, ... ]

   p1 : hash-unique_id
   SVAR[p1] = {   name-ptr : key-name-char-array,
                      var_...  
		  }

    SETS = svar_set_t = [ (hash,keys) ]  
       hash : list of hashed entries
       keys : list of var-id's

 */

static int SVAR, SVAR_FREE, SETS;


/* implementation */
static void svar_item_free( void *d );
/* ------------------------------------------------ */


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

static svar_t* svar( int key )
{
    return mls(SVAR, key);
}

/* static svar_set_t* svar_sets( int p ) */
/* { */
/*     return mls(SETS, p); */
/* } */



uint8_t *bitlist_byte(int m,int p)
{
    int len = m_len(m);
    if( p >= len ) {
	m_setlen( m, p+1 );
	memset( mls(m,len), 0, p - len + 1 );
    }
    return mls(m,p);
}

int  bitlist_test( int m, int p )
{
    int      offs  =  p/8;
    uint8_t  bit   =  p & 8;
    uint8_t  mask  =  ( 1 << bit );
    uint8_t *b     = bitlist_byte( m, offs );
    return  *b & mask;
}

void bitlist_set( int m, int p )
{
    int      offs  =  p/8;
    uint8_t  bit   =  p & 8;
    uint8_t  mask  =  ( 1 << bit );
    uint8_t *b     = bitlist_byte( m, offs );
    *b |= mask;
}

void bitlist_clr( int m, int p )
{
    int      offs  =  p/8;
    uint8_t  bit   =  p & 8;
    uint8_t  mask  =  ( 1 << bit );
    uint8_t *b     = bitlist_byte( m, offs );
    *b &= ~ mask;
}

int bitlist_find_zero( int m )
{
    uint8_t ch;
    int len = m_len(m);
    while( len-- ) {
	ch = UCHAR(m,len);
	if( ch == 0xff ) continue;
	uint8_t bit = 7;
	while( bit && (ch & (1 << bit)) ) bit--;
	return len*8+bit;
    }
    return -1;
}

int bitlist_find_one( int m )
{
    uint8_t ch;
    int len = m_len(m);
    while( len-- ) {
	ch = UCHAR(m,len);
	if( ch == 0 ) continue;
	uint8_t bit = 7;
	while( bit && ! (ch & (1 << bit)) ) bit--;
	return len*8+bit;
    }
    return -1;
}


static int svar_new(int buf)
{
    int hash_item;
    int *p = m_pop(SVAR_FREE);
    if( p == NULL ) hash_item = m_new( SVAR, 1);
    else hash_item = *p;
   svar_t *v = mls(SVAR, hash_item);
   memset(v,0,sizeof(*v));
   // use a list of integers to store a handle to the name
   v->name = m_create(2,sizeof(int)); // our list

   int str =  m_create( SVAR_MAX, 1 ); /* new string */
   s_strcpy( str, buf, SVAR_MAX );     /*  strcpy */
   m_put(v->name, &str);	       /* add to list */
   return hash_item;		       /* return this item */
}

/** @brief to be called by m_free_user, frees memory allocated by a svar 
 *  @param pp pointer to svar handle
 */
static void svar_free( void *pp )
{
    int p = *(int *)pp;
    svar_t *sv = svar( p );
    if( sv->name == 0 ) {
	// TRACE(1, "double free svar %d", p);
	return;
    }
    svar_item_free( sv );
    m_put( SVAR_FREE, &p );
}


/** find or insert variable (buf) in hash-table (hash)    
 */
static int hash_lookup(int hash, int buf, int byte_compare )
{
    int hash_item;

    // lookup key in hash-table
    uint32_t c = svar_hash(buf);
    assert( c < SVAR_TABLE_SIZE );
    int hash_item_list = INT(hash, c );

    // new entry
    if( hash_item_list == 0 ) {
	// insert new item-list in hash-table
	hash_item_list = m_create(1, sizeof(int) );
	INT(hash, c ) = hash_item_list;
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
	svar_t *v = mls(SVAR, *d);
	if( m_cmp( INT(v->name,0), buf ) == 0 ) return *d;
    }

 new_item:
    // create new item in item-list in hash-table 
    hash_item = svar_new(buf);    
    m_put(hash_item_list, &hash_item);
    return hash_item;
}

/* static int hash_to_string( int hash_item ) */
/* { */
/*     return svar(hash_item)->name; */
/* } */

// convert mstring to hash-code
// if buffer is not a multiple of 8 resize buffer
// @returns hash-code for sbuf
static int string_to_hash(int hash, int sbuf )
{
    ASSERT( m_width(sbuf) == 1 );
    int old_len = m_len(sbuf);
    int pad = (m_bufsize(sbuf) & 0x0f);
    if( pad  ) {
	m_setlen( sbuf, m_bufsize(sbuf) + (8-pad) );
	m_setlen( sbuf, old_len );
    }
    
    return hash_lookup(hash, sbuf, 1);
}

/* static int string_to_hash_fast(int hash, int sbuf ) */
/* { */
/*     return  hash_lookup(hash, sbuf, 0); */
/* } */

/* static int svar_key(int hash, int buf) */
/* { */
/*     return string_to_hash(hash, buf); */
/* } */

const char *svar_name( int v )
{
    return m_str( INT(svar(v)->name,0) );
}

int *svar_value( int v )
{
    return & svar(v)->value ;
}

uint8_t *svar_type( int v )
{
    return & svar(v)->type ;
}


void mlist_free(void *d)
{
    m_free( *(int*) d );
}

void sets_free( void *d )
{
    svar_set_t *item = d;
    int p,*svar_list;
    m_foreach( item->hash, p, svar_list) {
	m_free_user(*svar_list, svar_free );
    }
    m_free( item->hash );
}

static void svar_item_free( void *d )
{
    svar_t *item = d;
    m_free_list_of_list( item->name ); item->name=0;
    m_free( item->read_callbacks );
    m_free( item->write_callbacks );	
}

void m_free_user(int m, void (*free_h)(void*))
{
    void *d; int p;
    m_foreach(m,p,d) free_h(d);
    m_free(m);
}

void m_free_list_of_list(int m)
{
    m_free_user( m, mlist_free );
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


/* static int cstring_to_hash( int hash, int buf, const char *b ) */
/* { */
/*     int c; */
/*     m_write(buf,0,b, strlen(b)+1 ); */
/*     c = string_to_hash(hash, buf );  */
/*     return c; */
/* } */

void hash_new(int p)
{
    svar_set_t *set = mls(SETS,p);
    set->hash = m_create(SVAR_TABLE_SIZE+1, sizeof(int) );
    m_setlen( set->hash, SVAR_TABLE_SIZE+1 );
}

void svar_create(void)
{
    SVAR = m_create(100, sizeof(svar_t) );  m_add(SVAR);
    SETS = m_create(10, sizeof( svar_set_t) );
    m_add(SETS); hash_new(0); // set zero is reserved for globals
    SVAR_FREE= m_create(10, sizeof( int ));
}

void svar_destruct(void)
{
    m_free_user(SETS, sets_free );
    m_free_user(SVAR, svar_free );
    m_free(SVAR_FREE);
}


int     s_create( void )
{
    if( ! SVAR ) { svar_create(); }
    
    svar_set_t *set;
    int p,len;

    p = len = m_len(SETS);
    while(1) {
	p--;
	if( p < 0 ) { set = m_add(SETS); p=len; break; }
	set = mls( SETS, p);
	if( set->hash == 0 ) break;
    }
    hash_new(p);
    return p;
}

void    s_free( int vs )
{
    svar_set_t *set= mls(SETS,vs);
    if( set->hash == 0 ) {
	WARN("double free set %d", vs);
	return;
    }
    sets_free( set );
    set->hash = 0;
}

int s_lookup( int vs, int buf )
{
    svar_set_t *set= mls(SETS,vs);
    return string_to_hash(set->hash, buf ); 
}

int s_lookup_str( int vs, char *s )
{
    int buf = s_printf(0,0,s);
    int c = s_lookup( vs, buf );
    m_free(buf);
    return c;
}




/** @brief return array with var names 
 */
static int get_var_names( int q )
{
    return svar(q)->name;
}

/** @brief copy the list src to dest if dest is null create dest
 *  @returns dest
 */
int m_copy(int dest, int src)
{
    int len = m_len(src);
    if( dest <= 0 ) dest=m_create(len,1);
    m_setlen(dest,len);
    memcpy( m_buf(dest),m_buf(src), len );
    return dest;
}

/** @brief make an empty string. if str is null create str
 *  @returns str
 */
int s_clr( int str )
{
    if( str <= 0 ) str=m_create(1,1);
    m_setlen(str,1);
    CHAR(str,0)=0;
    return str;
}

// Access Stringlist Var
//
// row=0 - varname
// row=1 - first value ...
//
// row < 0 OR row==Len : Append val
// row > len : Error, exit
void    s_kset( int key, int buf, int row )
{
    int val;
    int var = get_var_names(key);

    if( row < 0 || row >= m_len(var) ) // append-value
	{
	    val = m_copy(0,buf); 
	    m_put(var,&val);
	    return;
	}
    
    int *pstr = mls(var,row);
    *pstr = m_copy(*pstr, buf); /* overwrite or create */
}

void    s_kclr( int key )
{
    int var = get_var_names(key);
    
    int *d, i=0;		/* start at second entry */
    while( m_next(var,&i,&d) )
	if( *d ) {
	    m_free(*d); *d=0; /* free mstring */
	}

    m_setlen(var,1);
}

int  s_kget( int key, int row )
{
    int var = get_var_names(key);
    int len=m_len(var);
    if( row >= len || row < 0 ) row=len-1;
    return INT(var,row);
}

int s_klen( int key )
{
    int var = get_var_names(key);
    return m_len(var);
}

int m_alloced_mem(int m)
{
    return m_bufsize(m) * m_width(m);
}

int svar_mem(int sp)
{
    svar_t *v = svar(sp);
    int mem=0;
    int p, *d;
    m_foreach(v->name,p,d) {
	mem+=m_alloced_mem(*d);
    }
    return mem;
}

void statistics_svar_allocated(int *a, int *mem, int *free)
{
    *a = *mem = *free = 0;
    int p; svar_t *d;
    m_foreach(SVAR,p,d) {
	(*mem) += svar_mem(p);	    
	if( d->name ) (*a)++;
    }
    *free = m_len(SVAR_FREE);
}
