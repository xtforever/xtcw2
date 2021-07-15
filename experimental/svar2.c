#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

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

/** @brief fail safe get string from svar
 * p==-1 returns last value
 * p==0  returns first value
 * if svar is mstring p==0 and p==-1 returns the array
 * p > 0 : returns m_str(value[p]) 
 */
const char* svar_get_str(int svar_key, int p)
{
    svar_t *v = svar(svar_key);
    int buf = -1;
    if( p == -1 || p == 0 ) { /* return svar value if SVAR_STRING */
	if( (v->type & SVAR_MASK) == SVAR_MSTRING ) {
	    buf = v->value;
	    goto check_return_str;
	}
    }

    if( (v->type & SVAR_MASK) != SVAR_MSTRING_ARRAY ) {
	WARN("list is not an mstring array");
	return "";
    }

    if( !v->value || p > m_len(v->value) ) {
	WARN("index %d is greater than list %d:len=%d", p,
	     v->value, v->value ? m_len(v->value) : 0 );
	return "";
    }
    if( p < 0 ) p = m_len(v->value)-1;
    buf = INT(v->value,p);

 check_return_str:
    if( buf <= 0 ) return "";
    if( m_len(buf) == 0 ) return "";
    return m_str(buf);   
}

int svar_get_str_count(int svar_key)
{
    svar_t *v = svar(svar_key);
    if( (v->type & SVAR_MASK) == SVAR_MSTRING ) {
	return 1;
    }
    if( (v->type & SVAR_MASK) != SVAR_MSTRING_ARRAY ) {
	WARN("list is not an mstring array");
	return -1;
    }

    if( v->value <= 0 ) {
	WARN("this var is not initialized: %s", svar_name(svar_key) );
	return 0;
    }
    
    return m_len(v->value);
}


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
    int *v = (int *)svar_value(key);
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

int fast_string_to_hash( svar_name_t *name )
{
    int h = hash_lookup(HASH, name, 1);
    append_key_to_parent_collection(name->parent,h);
    return h;
}

int create_container( int keystr )
{
    int h = string_to_hash( HASH, keystr );
    new_svar_container(h);
    return h;
}

int lookup_container( int container, int keystr )
{
    svar_name_t name;
    name.parent = container;
    int len = m_len(keystr);
    len = Min( len, SVAR_MAX);
    memcpy(name.str, m_buf(keystr), len );
    return fast_string_to_hash( &name );
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

intptr_t *svar_value( int v )
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
    int is_svar    = t == SVAR_SVAR;
    int is_string  = t == SVAR_STRING;
	
    item->type = 0;		/* we do not want recursion */
    /* if var1 contains var2 and var2 contains var1 we would have an
       endless loop */
    
    /* handle simple case first */
    if( ! is_array ) {
	if( is_marray ) { m_free(item->value); return; } 
	if( is_svar ) { svar_free_cb(& item->value ); return; }
	if( is_string ) { free( (void*)(intptr_t)item->value ); item->value=0; return; }
	return; 		/* no destructor needed */
    }
    
    /* this svar->value contains an array, lets check the type of objects */
    if( is_marray ) {		/* values are simple arrays */
	m_free_list_of_list(item->value);
	return;
    }
    
    if( is_string ) {
	m_free_strings( item->value, 0);
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

static void create_typed_svar( svar_t *v, uint8_t type )
{
    v->type = type;
    /* non-array int32 types do not need init */
    /* SVAR_INT, SVAR_FLOAT, SVAR_STRING */
    if( type & SVAR_ARRAY ) {
	v->value = m_create(10,sizeof(intptr_t));	
    }
    else { // kein array
	if( (type & SVAR_MASK) == SVAR_MSTRING ) {
	    v->value = m_create(10,1);
	    return;
	}
    }
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
	create_typed_svar( v, type );
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




void svexp_init(  svexp_t *se  )
{
  memset( se, 0, sizeof *se );
}

void svexp_free(  svexp_t *se )
{
  m_free_strings( se->splitbuf,0 );
  m_free( se->values );
  m_free( se->indices );
  m_free( se->buf );
  memset( se,0,sizeof *se);
}


void svexp_realloc_buffers( svexp_t *se )
{
  if( !se->buf ) {
    se->splitbuf = m_create(10,sizeof(char*));
    se->values=m_create(10,sizeof(char*));
    se->indices=m_create(10,sizeof(int));
    se->buf=m_create(100,1);
    return;
  }

  m_free_strings(se->splitbuf,1);
  m_clear(se->values); m_clear(se->indices);
  m_clear(se->buf);
  se->max_row=0;
}

// tbd
// returns:
//  1 ALL
//  2..n SINGLE
//  0 ERROR
//
static int parse_index(const char **s)
{
  int val;
  const char *p = *s;

  if( *p !='[' ) return 0;
  p++;
  if( *p == '*' && p[1] == ']') {
    *s=p+2;
    return 1;
  }

  val=0;
  while( isdigit(*p) )
    {
      val *= 10; val+= *p - '0';
      p++;
    }
  if( *p == ']' ) { *s=p+1; return val+2; }

  return 0;
}


void svexp_parse(svexp_t *se, const char *frm)
{
  ASSERT( frm && se );

  svexp_realloc_buffers(se); // alloc, or clear buffer

  int b=se->splitbuf;
  char *cp,prev; const char *s,*s0;

  prev=0; s = frm; s0=s;

  for(;;) {

    if( *s == 0 || (*s == '$' && prev != '\\') )
      {
        // prefix ?
        if( s0 != s ) {
          cp = strndup(s0,s-s0); // copy without *s
          m_put(b,&cp);
        }
        if( *s == 0 ) break; // exit

        // cut out varname
        s0=s; //  token start (with $-prefix)
        s++; // skip leading $

        if( *s == '\'' ) { s++; } // expand with single quotes

        while( isalnum(*s) || *s=='_' ) s++; // UNTIL DELIMITER FOUND
        // copy without delimiter
        cp=strndup(s0,s-s0);
        m_put(b,&cp); m_put(se->values,&cp);

        int index = parse_index(&s);
        m_put( se->indices, &index );
        if( *s == 0 ) break; // exit

        s0=s; // s0 points to delimiter
      }
    prev = *s;
    s++;
  }
};

/** @brief erzeugt aus *s einen string mit escape zeichen
    @param s2 - 0 oder mls liste
    @param s - ein string der umgewandelt werden soll
    @param quotes - falls quotes==1 werden einfache anführungszeichen um die variable gesetzt
    @return gültiger mls string (liste mit breite 1, string ohne Nullbyte)
 */

static int field_escape(int s2, const char *s, int quotes)
{
  // "*s" ist der zu speichernde string
  // um das sql-kommando zu generieren werden sonderzeichen
  // "escaped". dies ist ein gutes beispiel warum die "mls"
  // speicherverwaltung vorteile bietet. der benötigte speicher
  // von mysql_escape_string muss abgeschätzt und reserviert werden.
  // die gleiche funktion in mls ist viel einfacher zu verwenden
  if( quotes ) m_putc( s2, '\'');
  escape_buf( s2, (char*)s );
  if( quotes ) m_putc( s2, '\'');
  return s2;
}

/**
    @return einen gültigen string - immer
 */
const char* svexp_expand( svexp_t *se, char *prefix, int row )
{
    const char *val;
    int count,var, index;
    int p,vn; char **d, *s;
    int quotes = 0;
    int varname = m_create(100,1);
    m_clear(se->buf); int buf = se->buf;
    vn=0; // number of variables


  // string zusammenfügen
  // variablen werden durch ihren wert ersetzt
  // variablen werden durch ein führendes "$" erkannt
  // folgt dem $ ein "'" wird der eingesetzte wert durch "'" umschlossen
  //
  m_foreach( se->splitbuf, p,d ) {
    s = *d;

    if( *s != '$' ) { // einfacher text-baustein, nur anhängen
      m_write( buf, m_len(buf), s, strlen(s) );
    }
    else  // variable found
      {
        if( s[1] == '\'' ) { quotes=1; s++; } else quotes=0;
	s_printf(varname,0,"%s.%s", prefix, s+1 );
        var = svar_lookup(varname,-1);
        index = INT(se->indices, vn ); vn++;
	count = svar_get_str_count(var);
        // expand var
	if( index == 1 ) { // erzeuge eine liste von werten
	    val = svar_get_str(var,0);
	    field_escape(buf, val, quotes);
	    for( index=1; index < count; index++ ) {
		m_putc(buf, ',' );
		val = svar_get_str(var,index);
		field_escape(buf, val, quotes);
	    }
	}
	else { // index != 1  i.e. not expand all i.e. index != [*]
	    if( index == 0 ) index=row; // falls kein index angegeben wurde, benutze (row)
	    else index -= 2;

	    if( index < count ) {
		val = svar_get_str(var,index);
		field_escape(buf, val, quotes);
	    }
	    
	}
      } // variable expandiert
  }

  m_putc( buf, 0 );
  m_free(varname);
  return   mls( buf, 0 );
}

/** expandiert den string frm mit den variablen aus vl
 * @return	einen Zeiger auf den expandierten string
 *		dieser string wird auch als variable
 *		unter dem namen se_string in vl gespeichert
 */
const char*   svexp_string( char *prefix, const char *frm )
{
    svexp_t se;
    int var, varname;
    
    svexp_init( &se );
    svexp_parse( &se, frm );
    svexp_expand( &se, prefix, 0 );
    
    varname = s_printf(0,0,"%s.se_string", prefix );
    var = svar_lookup(varname,SVAR_MSTRING);

    svar_t *v = svar(var);
    if(v->value==0) v->value=m_create(100,1);
    m_copy( v->value, se.buf );
    svexp_free(&se);
    m_free(varname);
    return m_str(v->value);
}



int s_strdup(int r, const char *str)
{
    int l;
    if( is_empty(str) ) { // return empty buffer
	if( r <= 0 ) r = m_create(1,1); else m_clear(r);
	m_putc(r,0);
	return r;
    }
    
    l = strlen(str)+1; // return copy of string 
    if( r <= 0 ) r = m_create(l,1); else m_clear(r);
    m_write(r,0,str,l);
    return r;
}

static int app_cstr_mstrl( int m, const char *name )
{
    int x = s_strdup(0,name);
    m_put(m,&x);
    return m;
}


static int vas_create_mstring_array(int m, va_list ap )
{
    char *name;
    while( (name = va_arg( ap, char* )) != NULL )
    {
	app_cstr_mstrl( m, name );
    }
    return m;
}

int vas_append_mstring_array(int m,int offset, va_list ap)
{
    if(!m) {
	m=m_create(10,sizeof(int));
    } else {
	int len = m_len(m);
	if( offset >= 0 ) {
	    while( offset < len-- ) {
		m_free( INT(m,len) );		
	    }
	}
	m_setlen(m,offset);
    }
    
    vas_create_mstring_array(m,ap);
    return m;

}



int append_mstring_array(int m,int offset, ... )
{
    va_list ap;
    va_start(ap,offset);
    m=vas_append_mstring_array(m,offset,ap);
    va_end(ap);
    return m;
}


static void element_copy( int dest, int destp, int src, int srcp, int src_count, int width  )
{
    for(int i=0;i<src_count;i++) {
	void *from = mls(src,srcp);
	memcpy( mls(dest,destp), from, width );
	srcp++;
	destp++;
    }
}

static void array_copy( int dest, int destp, int src, int srcp, int src_count  )
{
    void *from = mls(src,srcp);
    void *to   = mls(dest,destp);
    size_t n = src_count * m_width(src); 
    memcpy( to, from, n );
}

int m_mcopy(int dest, int destp, int src, int srcp, int src_count  )
{
    if(srcp < 0) srcp=0;
    /* if no lenght is given or length outside of array, copy full array */
    int src_len = m_len(src);
    if( src_count < 0 || src_count + srcp > m_len(src) )
	{
	    src_count = src_len - srcp;	
	}

    /* so far, we have checked srcp,src_count, now look at the others */

    
    /* create dest if not exists */
    if( dest <=0 ) {
	if( destp <0 ) destp=0;	
	int dest_len = destp + src_count;
        dest = m_create( dest_len,  m_width(src) );
    }

    /* shortcurt */
    if( src_count == 0 )
	return dest;

    /* if no dest offset is given assume user wants to append */
    if( destp < 0 ) destp=m_len(dest);
    
    /* resize dest to fit src array */
    if( m_len(dest) < destp + src_count )
	m_setlen(dest, destp + src_count );


    // printf("normalized: srcoff=%d,dstoff=%d,count=%d\n", srcp,destp,src_count);
    
    /* check if src and dest are of same element size */
    if( m_width(src) == m_width(dest) ) {
	array_copy( dest,destp,src,srcp,src_count);
	return dest;
    }
       
    int width = Min( m_width(src), m_width(dest) );
    
    element_copy( dest, destp, src, srcp, src_count, width  );
    return dest;
}




/* class interfacce */
void   s_construct( int v3 );
void   s_destruct( int  v3  );
void   s_set_value( int v3, int pos, void *data );
void*  s_get_value( int v3, int pos );
void*  s_clear(int v3);  // remove all items 


/* mgmt interface */
/* same as class interface */


svar3_t* svar3( int v3 );



/* string list interface */
void    s_kset( int sl, const  char* value, int pos );
void    s_kclr( int sl );
char*   s_kget( int sl, int pos );
int     s_klen( int sl );
void    s_kfree( int sl );
void    s_kdel( int sl, int pos );


/* integer interface */
int     s_iget( int v3 );
void    s_iput( int v3, int data );


/* groups and lookup */
int lookup( int group, int buf, int type );
int lookup_str( int group, char *buf, int type );
int lookup_path( int  path, int type );
int lookup_path_str( char *path, int type );




