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


       key1.key2.key3

       0 + key1
       hash(key1)+key2
       hash(key2)+key3
       
       key = key1

       h=lookup(key)
       h1 = lookup ( #h + next_key )
       add h1 to key
       key = #h + key1
       

 */

static int SVAR = -1, SVAR_FREE;
static int HASH;

/* implementation */
static void svar_item_free( void *d );
const char *svar_name( int v );

/* ------------------------------------------------ */

/** @brief copy string from src to dest
 * the dst is always resized to max and will contain a termination zero
 * an empty or not-allocated (i.e. zero) src is allowed
 * if dst is zero it will be allocated
 * @returns dst
 */
int s_memcpy(int dst, int src, int max)
{
    if( ! dst ) dst = m_create(max,1);
    
    if( m_bufsize(dst) < max ) {
	m_setlen(dst,max);
    }
    int c=1;    
    if( src && m_len(src) ) { /* not empty string */
	c = Min(max,m_len(src));
	memcpy( m_buf(dst), m_buf(src), c-1 );
    }
    m_setlen( dst, c );
    CHAR(dst,c-1)=0;
    return dst;
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
	return hash32 & SVAR_TABLE_SIZE;
} // Last update: 2019-Oct-30, 14 C lines strong, Kaze.


static uint32_t svar_hash(void *sbuf)
{
    // svar_name_t *sn = sbuf;
    uint32_t h = FNV1A_Pippip_Yurii( sbuf, SVAR_MAX2 );
    //    TRACE(2,"hash: %u, parent: %d, name: %s", h, sn->parent, sn->str );
    //    for(int i=0;i<SVAR_MAX;i++) printf("%02x ", (unsigned char) sn->str[i] );
    //    puts("");
    return h;
}

svar_t* svar( int key )
{
    return mls(SVAR, key);
}




static int svar_new( void *buf )
{
    int hash_item;
    int *p = m_pop(SVAR_FREE);	/* get last deleted svar */
    if( p )			/* svar to recycle found */
	hash_item = *p;		/* use old svar */
    else
	hash_item = m_new( SVAR, 1); /* create new svar */
    svar_t *v = mls(SVAR, hash_item); /* get svar memory */
    memset(v,0,sizeof(*v));	      /* clear memory  */
    memcpy( &(v->name), buf, sizeof(svar_name_t) );
    TRACE(2,"Created %d %s", hash_item, svar_name(hash_item) );
    return hash_item;		       /* return this item */
}

/** @brief to be called by m_free_user, frees memory allocated by a svar 
 *  @param pp pointer to svar handle
 */
static void svar_free_cb( void *pp )
{
    int p = *(int *)pp;
    TRACE(2,"id: %d", p );
    if( p < 0 || p > m_len(SVAR) )
	{
	    puts("halt");
	}
    
    svar_t *sv = svar( p );
    if( sv->name.parent < 0 ) {
	TRACE(1, "double free svar %d", p);
	return;
    }
    svar_item_free( sv );
    m_put( SVAR_FREE, &p );
}

#define sizeofm(type, member) sizeof(((type *)0)->member)

static const inline int svar_cmp( int id, void *buf )
{
    return memcmp( & (svar(id)->name), buf, sizeof(svar_name_t));
}

/** find or insert variable (buf) in hash-table (hash)    
 */
static int hash_lookup(int hash, void *buf, int byte_compare )
{
    int hash_item;
    uint32_t c = svar_hash(buf); /* lookup key in hash-table */
    int hash_item_list = INT(hash, c ); /* list of keys with same hash */

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
	if( svar_cmp(*d,buf) == 0 ) return *d;
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
/* static int key_to_hash(int hash, int sbuf ) */
/* { */
/*     ASSERT( m_width(sbuf) == 1 ); */
/*     int pad = (m_bufsize(sbuf) & 0x07); */
/*     if( pad  ) { */
/* 	int old_len = m_len(sbuf); */
/* 	m_setlen( sbuf, m_bufsize(sbuf) + (8-pad) ); */
/* 	m_setlen( sbuf, old_len ); */
/*     } */
    
/*     return hash_lookup(hash, sbuf, 1); */
/* } */

/* 
   lookup:
   returns hash of key
   example key = atest,btest,ctest
   loop:
     append(x , get key until dot )
     h = hash(x)
     if( seperator is zero ) return h
     skip separator
     write h to x
     goto loop

 */
/* returns: position of new element */
int m_binsert( int buf, const void *data, int (*cmpf) (const void *a,const void *b ), int with_duplicates )
{
    int left = 0;
    int right = m_len(buf)+1;
    int cur = 1;
    void *obj;
    int cmp;

    if( m_len(buf)==0 ) {
	m_put(buf,data);
	return 0;
    }
    
    while(1) {
	cur = (left+right) / 2;
	TRACE(1, "[%d %d] cur:%d", left, right, cur );	
	obj = mls( buf, cur - 1 );
	cmp = cmpf( data, obj );
	if( cmp == 0 ) {
	    if( ! with_duplicates ) return -1;
	    break;
	}
	if( cmp < 0 ) {
	    right=cur;
	    if( left+1 == right ) break;
	} else {
	    left  = cur;
	    if( left+1 == right ) {
		cur++;
		break;
	    }
	}
    }
    

    cur--;
    TRACE(1, "insert at %d", cur );
    m_ins( buf, cur, 1 );
    m_write( buf, cur, data, 1 );
    return cur;
}

static int seperate(char *dst, int src, int *start)
{
    int i = 0;
    while(1) {
	assert(i < SVAR_MAX);	/* paranoia */
	char ch = CHAR(src,*start);
	if( ch == '.' || ch == 0 ) {
	    memset(dst,0,SVAR_MAX - i );
	    return ch;
	}
	*dst++ = ch;
	i++;
	(*start)++;
    }
}

static int compare_int(const void *a,const void *b)
{
    const int *a0 = a, *b0 = b;
    return (*a0) - (*b0);
}

static void append_key_to_parent_collection(int parent, int key)
{
    int collection = *(int *) svar_value(parent);
    m_binsert( collection, &key, compare_int, 0 );
}


static int  new_svar_container(int key )
{
    uint8_t *t = (uint8_t*)svar_type(key);
    if( *t ) {
	if( ((*t) & SVAR_MASK) == SVAR_SVAR_ARRAY ) return key;
	ERR("Variable %s is not a container", svar_name(key) );
    }
    *t = ((*t) & SVAR_MASK) | SVAR_SVAR_ARRAY;
    int *v = svar_value(key);
    *v = m_create(10,sizeof(int));
    return key;
}


static int string_to_hash(int hash, int buf)
{
    int ch;
    int h;
    int i=0;
    svar_name_t name;
    name.parent = 0;
    
    while(1) {
	ch = seperate(name.str, buf, & i );
	h = hash_lookup(hash, &name, 1);
	TRACE(2,"SVARID:%d NAME:%s PARENT:%d\n", h, name.str, name.parent );
	append_key_to_parent_collection(name.parent,h);
	if( ch == 0 ) break;
	i++;	
	name.parent = new_svar_container(h); /* type of key is container */
	
    }
    return h;
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
    return svar(v)->name.str;
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

/* should have a list of destructors for each type */
static void svar_item_free( void *d )
{
    svar_t *item = d;

    TRACE(1,"svar-name: %s", item->name.str );
    if( item->name.parent < 0 ) {
	TRACE(1,"double free");
	return;
    }
    item->name.parent = -1;
    m_free( item->read_callbacks ); item->read_callbacks=0;
    m_free( item->write_callbacks ); item->write_callbacks=0;
    if(! item->type ) return;	/* no type info */

    int t = (item->type & SVAR_MASK); /* 4 bits */
    int is_array   = t & SVAR_ARRAY;
    t &= ~ SVAR_ARRAY;
    
    int is_marray = ( t==SVAR_MSTRING || t==SVAR_MARRAY );
    int is_svar    = SVAR_SVAR;
    item->type = 0;		/* we do not want recursion */
    /* if var1 contains var2 and var2 contains var1 we would have an
       endless loop */
    
    /* handle simple case first */
    if( ! is_array ) {
	if( is_marray ) { m_free(item->value); return; } 
	if( is_svar ) { svar_free_cb(& item->value ); return; }    
	return; 		/* no destructor needed */
    }
    
    /* this svar->value contains an array, lets check the type of objects */
    if( is_marray ) {		/* values are simple arrays */
	m_free_list_of_list(item->value);
	return;
    }

    if( is_svar ) {		/* each value is a svar */
	m_free_user(item->value, svar_free_cb ); /* recursion! */
	return;
    }

    m_free( item->value ); /* it's an array of something completly different */
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

void m_clear_mstring_array(int m)
{
    int p, *d;
    m_foreach(m,p,d) m_free(*d);
    m_clear(m);
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
void svar_write_callbacks( int q )
{
    int p;
    struct svar_signal *sig;
    svar_t* ent = svar(q);
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

int new_hashtable(void)
{
   int h =  m_create(SVAR_TABLE_SIZE+1, sizeof(int) );
   m_setlen( h, SVAR_TABLE_SIZE+1 );
   return h;
}

void svar_create(void)
{
    if( SVAR >=0 ) return;
    SVAR = m_create(100, sizeof(svar_t) );
    new_svar_container( m_new(SVAR,1)); // one global svar with index 0
    SVAR_FREE = m_create(10, sizeof( int ));
    HASH =  new_hashtable();
}

void svar_destruct(void)
{
    
    m_free_user(SVAR, svar_item_free );
    m_free(SVAR_FREE);
    m_free_list_of_list( HASH );
    SVAR=-1;
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

int m_alloced_mem(int m)
{
    return m_bufsize(m) * m_width(m);
}

int svar_mem(int sp)
{
    svar_t *v = svar(sp);
    (void)v;
    int mem=0;
    return mem;
}

void statistics_svar_allocated(int *a, int *mem, int *free)
{
    *a = *mem = *free = 0;
    int p; svar_t *d;
    m_foreach(SVAR,p,d) {
	(*mem) += svar_mem(p);	    
	if( d->name.parent != -1 ) (*a)++;
    }
    *free = m_len(SVAR_FREE);
}


/* svar types
   bit
   0		0 = single value
		1 = array
   3,2,1	000 = integer
   		001 = float           
                100 = mstring
		101 = svar
		110 = marray 

   4 5 6 7      FREE
*/
const char *svar_typename(int v)
{
    static char name[40];
    int t = (*svar_type(v)) & SVAR_MASK;
    *name=0;			/* clear string */
    if( svar_is_array(t)) {
	strcpy(name, "array of "); /* 9 chars */
	t ^= SVAR_ARRAY;
    }
    
    switch( t ) {
    case SVAR_INT: strcat(name,"integer"); break;
    case SVAR_FLOAT: strcat(name,"float"); break;
    case SVAR_MSTRING: strcat(name,"mstring"); break;
    case SVAR_SVAR: strcat(name,"svar"); break;
    case SVAR_MARRAY: strcat(name,"marray"); break;
    default: strcat(name,"unknown"); break;
    }
    return name;
}


/* 
 * key1 - key
 * key1.key2 - make key1 a container and key2 a key inside
 * key1.key2.key3 - make key2 a container with key3 inside
 * key1.key2 - list all keys inside container (lookup its value field) 
 */
int svar_lookup(int buf, int type )
{
    int h = string_to_hash(HASH,buf);
    if( type < 0 ) return h;

    svar_t *v = svar(h);
    uint8_t t = v->type;
    // if type is undefined set type 
    if( t == 0 ) {
	v->type = type;
	return h;
    }

    // lookup already defined variable and do a type compare 
    if( t != type ) {
	ERR("wrong type: svarname:%s id:%d", svar(h)->name.str, h );
    }

    return h;
}

int svar_lookup_str(char *s, int type )
{
    int buf = m_create(SVAR_MAX,1);
    m_write(buf,0,s,strlen(s)+1);
    int k = svar_lookup(buf,type);
    m_free(buf);
    return k;
}


void svar_free( int key )
{
    svar_free_cb( &key ); 
}

