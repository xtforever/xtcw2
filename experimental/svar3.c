#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "svar3.h"
#include "svar-util.h"


static int SVAR = -1, SVAR_FREE, HASH;

svar_t* svar( int key )
{
    return mls(SVAR, key);
}

static int svar_alloc( void *buf )
{
    int *p = m_pop(SVAR_FREE);	/* get last deleted svar */
    if( p ) return *p;
    return m_new( SVAR, 1); /* create new svar */
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
	    ERR("illegal svar handle %d", p);
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




