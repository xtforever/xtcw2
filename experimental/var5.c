/* 

   storage:

   VAR_CALLBACK		structs with callback pointer
			one per variable
			primary-key: var-id

   VAR_IF		structs with interface functions 
			one per variable-type
			primary-key: index
     
   MVAR_MEM		pointer to variables
			primary-key: (group,name)
			        key: index (var-id)

   Each variable has two attributes, name and group.
   Together they form a unique identifier.
   
   They can be used to form a graph:
   tree: list of (node1,node2)

   node1 = group
   node2 = var-id
   
   A leave node is a node with arbitrary type
   A branch starts at a node with type VSET
  
   
            (1:(root,0) )

       (2:(n1,1))     (4:(n3,1))         

       (3:(n2,2))        

       root,n1 - must have type VSET (or INTEGER) 
       n2,n3   - type is arbitrary


   mvar_parse can be used to create/access the tree.
   example:
      node = mv_parse( *root.n1.n2, "STRING" )      
      - this call will construct three nodes (root,n1,n2)

      node = mv_parse( *root.n3, "INTEGER" )
      - this call will construct one node (n3)
      
   The complete tree is accessable with the id of the
   root-node 
   to query the root id you have multiple possibilities:
   id = mv_lookup( 0, "root", NULL );
   id = mv_lookup_path( "#0.root", NULL );
   id = mv_lookup_path( "root", NULL );
   id = mv_parse( "root", NULL );
   id = mv_parse( "*root", NULL );
   
   

   var5 interface 

   types:
     VAR_INTEGER 
     VAR_STRING 
     VAR_VSET 

   init
     mvar_init

   create anon variables:   
     int id = mvar_anon(0, VAR_VSET );


   create anon vset variables:
     int  g = mvar_vset();

   find/create a variable
     int q =  mvar_lookup( group,sname,stype );			
     int q =  mvar_lookup_path(mp,stype);			
     int offs = mvar_parse_path(mp,&group);
     mvar_free(q);


   read/write
     mvar_put_string(q,s,p)
     mvar_get_string(q,p)
     mvar_put_integer(q,v,p)
     mvar_get_integer(q,p)

   inspect
     mvar_type(q)
     mvar_group(q)
     mvar_name(q)
     mvar_id(q)
     mvar_path(q)
     mvar_length(q)

   signals
     var_set_callback( q1, cb1, ctx, 0 )			
     var_call_callbacks( q1 )
     var_callback_destroyv

     
     
 */
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mls.h"
#include "ctx.h"
#include "var5.h"
#include "m_tool.h"

#define HASH_SIZE (4*8)
#define HASH_BITS 14
#define HASH_TABLE_SIZE (1<<HASH_BITS)
#define HASH_TABLE_MASK (HASH_TABLE_SIZE-1)
int HASH_TABLE = 0;
int var5_lookup( int group, char *name, int  typeid );
void var5_delete_hash( var_t *v );


struct var_st;
typedef struct var_st var_t; 

struct var_if {
    int    (*put_string) ( var_t *v, char *s, int p );
    char*  (*get_string) (var_t *v, int p);
    int    (*put_integer) ( var_t *v, long val, int p );
    long   (*get_integer) (var_t *v, int p);    
    int    (*to_string) (var_t *v, int buf, int p);
    int    (*length)  (var_t *v);
    var_t* (*create)  (void);
    void   (*destroy) (var_t *v);
};


void set_error(var_t *v, char *s ) {
};

int VAR_CALLBACK = 0;
struct var_callback {
    int var_id;
    int signal, locked;
};

struct var_signal {
    void *d;
    varsig_t fn;
};

int callback_lookup_pos( int q )
{
    if(!VAR_CALLBACK) VAR_CALLBACK=m_create(1,sizeof(struct var_callback));
    int p;
    struct var_callback *cb;
    m_foreach(VAR_CALLBACK,p,cb) {
	if( cb->var_id == q ) return p;
    }
    cb = m_add(VAR_CALLBACK);
    cb->var_id = q;
    cb->signal=0; cb->locked=0;
    return m_len(VAR_CALLBACK)-1;    
}

struct var_callback * callback_lookup( int q )
{
    int p = callback_lookup_pos( q );
    return mls(VAR_CALLBACK,p);
}

void var_set_callback( int q, varsig_t fn, void *d, int remove )
{
    struct var_callback *ent = callback_lookup(q);
    int p;
    struct var_signal *sig;
    
    if(!ent->signal) ent->signal = m_create( 1, sizeof(struct var_signal));

    /* finde signal und falls remove==TRUE entfernen */
    m_foreach( ent->signal, p, sig ) {
        if( sig->fn == fn && sig->d == d ) {
            if( remove ) m_del(ent->signal,p);
            return;
        }
    }
    if( remove ) return;

    /* signal noch nicht vorhanden, remove==FALSE, jetzt hinzufügen */
    sig = m_add(ent->signal);
    sig->d = d;
    sig->fn = fn;
}

void var_call_callbacks( int q, int s )
{
    struct var_callback *ent = callback_lookup(q);
    if(! ent->signal ) return;

    /* um rekursion zu verhindern wird signal blockiert */
    if( ent->locked ) return;
    ent->locked = 1;
    
    int p;
    struct var_signal *sig;
    m_foreach( ent->signal, p, sig ) {
	if( sig->fn ) sig->fn( sig->d, q, s );
    }
    ent->locked = 0;
}

void var_callback_destroy(void)
{
    int p;
    struct var_callback *ent;
    m_foreach( VAR_CALLBACK, p, ent ) {
	if( ent->signal ) m_free(ent->signal);
    }
    m_free(VAR_CALLBACK);
    VAR_CALLBACK=0;
}


/* same as var_if */
struct string_var_if {
    struct var_if;
};

/* same as var_st but with extensions */
struct string_var_st {
    struct var_st;
    int lst;
};
typedef struct string_var_st string_var_t;

/* later 
char *string_get_string(var_t *v, int p)
{
    if( v->var_if != VAR_STRING ) {
	set_error(v, "this isn't a string class");
	return "";
    }    
    string_var_t *s = (string_var_t*)v;
    return s->fn->core.get_string(v,p);
}

int string_put_string(var_t *v, char *str, int p)
{
    if( v->fn->if_type != VAR_STRING ) {
	set_error(v, "this isn't a string class");
	return -1;
    }
    string_var_t *s = (string_var_t*)v;
    return s->fn->core.put_string(v,str,p);
}
*/

var_t *string_impl_create(void);
void  string_impl_destroy(var_t *v);
char *string_impl_get_string(var_t *v, int p);
int string_impl_put_string(var_t *v,char *str, int p);
int string_impl_length(var_t *v);

struct string_var_if STRING_VAR_IF = {
    .put_string = string_impl_put_string,
    .get_string = string_impl_get_string,
    .length     = string_impl_length,
    .create = string_impl_create,
    .destroy = string_impl_destroy
};

int string_impl_length(var_t *v)
{
    string_var_t *s = (string_var_t*)v;
    return m_len(s->lst);
}

var_t *string_impl_create(void)
{
    string_var_t *s = calloc(1, sizeof(string_var_t) );
    s->lst = m_create(2,sizeof(char*));
    return (var_t*) s;
}

void string_impl_destroy(var_t *v)
{
    string_var_t *s = (string_var_t*)v;
    m_free_strings(s->lst,0);
    free(v);
}

char *string_impl_get_string(var_t *v, int p)
{
    string_var_t *s = (string_var_t*)v;
    return v_kget(s->lst,p);
}

int string_impl_put_string(var_t *v,char *str, int p)
{
    string_var_t *s = (string_var_t*)v;
    v_kset(s->lst,str,p);
    return 0;
}

/*-----------------------------------------------------------------------*/

/* same as var_if */
struct integer_var_if {
    struct var_if;
    char buffer[100];
};

/* same as var_st but with extensions */
typedef 
struct integer_var_st {
    struct var_st;
    int value;
} integer_var_t;

var_t* integer_impl_create(void);
void   integer_impl_destroy(var_t *v);
char*  integer_impl_get_string(var_t *v, int p);
int    integer_impl_put_string(var_t *v,char *str, int p);
long   integer_impl_get_integer(var_t *v, int p);
int    integer_impl_put_integer(var_t *v, long val, int p);
int    integer_impl_length(var_t *v);

struct integer_var_if INTEGER_VAR_IF = {
    .put_string = integer_impl_put_string,
    .get_string = integer_impl_get_string,
    .put_integer = integer_impl_put_integer,
    .get_integer = integer_impl_get_integer,
    .length      = integer_impl_length,
    .create = integer_impl_create,
    .destroy = integer_impl_destroy
};

var_t *integer_impl_create(void)
{
    integer_var_t *s = calloc(1, sizeof(integer_var_t) );
    s->value=m_create(1,sizeof(long));
    return (var_t*) s;
}

int integer_impl_length(var_t *v)
{
    integer_var_t *s = (integer_var_t*)v;
    return m_len(s->value); 
}

void integer_impl_destroy(var_t *v)
{
    integer_var_t *s = (integer_var_t*)v;
    m_free(s->value); 
    free(s);
}

char *integer_impl_get_string(var_t *v, int p)
{
    static char buffer[100];
    long val = integer_impl_get_integer(v,p);
    snprintf( buffer, sizeof(buffer),
	      "%ld", val );
    return buffer;    
}

int integer_impl_put_string(var_t *v,char *str, int p)
{
    return integer_impl_put_integer(v,atol(str),p);
}

int integer_impl_put_integer(var_t *v, long value, int p)
{
    integer_var_t *s = (integer_var_t*)v;
    if( p < 0 ) {
	m_put(s->value,&value);
	return 0;
    }
    if( p >= m_len(s->value) ) m_setlen(s->value,p+1);
    *(long *)mls(s->value,p)=value;
    return 0;
}

long integer_impl_get_integer(var_t *v, int p)
{
    integer_var_t *s = (integer_var_t*)v;
    if( p >= m_len(s->value) ) return -1;
    return *(long *)mls(s->value,p);
}

/* ---------------------------------------------------------------------*/

void  vset_impl_destroy(var_t *v);
var_t *vset_impl_create(void);
char *vset_impl_get_string(var_t *v, int p);

void mvar_free(int p);
int mvar_path(int id, int mp);

#define vset_var_if integer_var_if

/* same as integer_var_st  */
typedef
struct vset_var_st {
    struct integer_var_st;
} vset_var_t;

struct vset_var_if VSET_VAR_IF = {
    .put_string = integer_impl_put_string,
    .get_string = vset_impl_get_string,
    .put_integer = integer_impl_put_integer,
    .get_integer = integer_impl_get_integer,
    .length = integer_impl_length,
    .create =  integer_impl_create,
    .destroy = vset_impl_destroy,
 };

char *vset_impl_get_string(var_t *v, int p)
{
    static char buf[100];
    integer_var_t *s = (integer_var_t*)v;
    if( p >= m_len(s->value) ) return "";  
    int x = *(long *)mls(s->value,p);
    int str = mvar_path(x,0);
    strncpy(buf,m_str(str), sizeof(buf));
    return buf;
}

void  vset_impl_destroy(var_t *v)
{
    long *d; int p;
    integer_var_t *s = (integer_var_t*)v;
    m_foreach(s->value,p,d) {
	mvar_free(*d);
    }
    m_free(s->value);
    free(s);
}

/* -----------------------------------------------------------*/

/* --------------------------------------------------------*/
struct var_if *vreg_getif(uint8_t id);

struct var_if *var_getif(var_t *v)
{
    return vreg_getif( v->var_if );
}


var_t *var_create( void *dfn )
{
    struct var_if *fn = dfn;
    return fn->create();
}

void var_destroy(var_t*v)
{
    var_getif(v)->destroy(v);
}

int var_put_string( var_t *v, char *s, int p )
{
    struct var_if *fn = var_getif(v);
    return fn->put_string ? fn->put_string(v,s,p) : -1;
}
char* var_get_string( var_t *v, int p )
{
    struct var_if *fn = var_getif(v);
    return fn->get_string ? fn->get_string(v,p) : "";
}
long var_get_integer( var_t *v, int p )
{
    struct var_if *fn = var_getif(v);
    return fn->get_integer ? fn->get_integer(v,p) : -1;
}
int var_put_integer( var_t *v, long val, int p )
{
    struct var_if *fn = var_getif(v);
    return fn->put_integer ? fn->put_integer(v,val,p) : -1;
}
int var_length( var_t *v )
{
    struct var_if *fn = var_getif(v);
    return fn->length ? fn->length(v) : -1;
}

static int VAR_IF = 0;
struct var_register_st {
    char *name;
    struct var_if *fn;
};

struct var_register_st *vreg_get(int id)
{
    return mls(VAR_IF,id);
}


int  mvar_registry( void *funcs, char *name, int id )
{
    if( !VAR_IF ) VAR_IF=m_create(10,sizeof(struct var_register_st)); 
    if( id >= m_len(VAR_IF)) m_setlen(VAR_IF, id+1);    
    struct var_register_st *v = vreg_get(id);
    v->name = strdup(name);
    v->fn = funcs;
    return id;
}

int  mvar_registry_new( void *funcs, char *name )
{
    int id = m_len(VAR_IF);
    return mvar_registry( funcs,name,id);
}


void var_register_destroy(void)
{
    int p; struct var_register_st *v;
    m_foreach(VAR_IF,p,v) { free(v->name); }
    m_free(VAR_IF);    
}

int vreg_ifnum(char *name)
{
    return m_lookup_str(VAR_IF,name,1);
}

// find interface by id
struct var_if *vreg_getif(uint8_t id)
{
    return vreg_get(id)->fn;
}

// find interface by name
struct var_if *var_register_lookup(char *name)
{
    int p=vreg_ifnum(name);
    if( p < 0 )
	ERR("could not find registerd class %s", name );
    return vreg_getif(p);
}

char* mvar_name_of_type( int id )
{
    return vreg_get(id)->name;
}


static int MVAR_MEM = 0;
static int MVAR_FREE = 0;

var_t **mvar_get(int p )
{
    ASERR(p>0,"var <=0: %d", p);
    return mls(MVAR_MEM,p -1);
}

int mvar_alloc( void )
{
    if( !MVAR_MEM ) {
	MVAR_MEM = m_create( 10, sizeof(var_t*) );
	MVAR_FREE = m_create(10, sizeof(int) );
    }
    int *id = m_pop(MVAR_FREE);
    if(id) return *id;
    return m_new(MVAR_MEM,1) +1;
}

void mvar_create_var( int id, int ifnum )
{
    ASSERT( id > 0 );
    var_t **v = mvar_get(id);
    if( ifnum < 0 ) ERR("interface %d not defined", ifnum );
    *v = var_create(vreg_getif(ifnum) );
    (*v)->var_if = ifnum;
    TRACE(1,"new var %d : %s", id,  mvar_name_of_type(ifnum) );    
}

/* returns number of var +1, because that garanties 
   that var-number is never zero
   maybe a bad idea 
*/
int mvar_create( int id )
{
    int p = mvar_alloc();
    mvar_create_var( p, id );
    return p;
}

/* free mvar 
   - remove from hash table
   - remove from mvar_mem 
   - put id in MVAR_FREE for reuse of this slot
*/ 
void mvar_free( int id )
{
    var_t **v = mvar_get(id);
    if( ! *v ) {
	WARN("double-free Var %d", id);
	return;
    }
    TRACE(2, "FREE %s", (**v).name );
    var5_delete_hash( *v );
    var_destroy( *v );
    *v=0;
    m_put(MVAR_FREE, &id );
}

static void mvar_free_all(void)
{
    int p = MVAR_MEM ? m_len(MVAR_MEM) : 0;
    while( p ) {
	if( *mvar_get(p) ) mvar_free(p);
	p--;
    }
	
    m_free(MVAR_FREE);
    m_free(MVAR_MEM);
    MVAR_MEM=0;
    MVAR_FREE=0;
}


int mvar_put_string( int id, char *s, int p )
{
    var_t *v =  *mvar_get(id);
    return ( v == NULL ) ? -1 : var_put_string( v, s, p );
}

char* mvar_get_string( int id, int p )
{
    var_t *v =  *mvar_get(id);
    return ( v == NULL ) ? "" : var_get_string( v, p );
}

int mvar_put_integer( int id, long s, int p )
{
    var_t *v =  *mvar_get(id);
    return ( v == NULL ) ? -1 : var_put_integer( v, s, p );
}

long mvar_get_integer( int id, int p )
{
    var_t *v =  *mvar_get(id);
    return ( v == NULL ) ? -1 : var_get_integer( v, p );
}

int mvar_type(int id)
{
    var_t *v =  *mvar_get(id);
    return ( v == NULL ) ? -1 : v->var_if; 
}

int mvar_group(int id)
{
    var_t *v =  *mvar_get(id);
    return ( v == NULL ) ? -1 : v->group;
}

/* dangerous - could be not null terminated */
char* mvar_name(int id)
{
    var_t *v =  *mvar_get(id);
    return ( v == NULL ) ? "" : v->name;
}

int mvar_length(int id)
{
    var_t *v =  *mvar_get(id);
    return  ( v == NULL ) ? -1 : var_length( v );
}

int mvar_path(int id, int mp)
{
    char *s = mvar_name(id);
    int g =  mvar_group(id);
    
    return s_printf(mp,0,"#%u.%.28s", g,s );
}


int mvar_parse_path(int mp, int *group)
{
    *group=0;
    if( mp <=0 ) return -1;
    if( CHAR(mp,0) != '#' ) return 0;
    char *endp;
    *group = (int) strtoul(mls(mp,1),&endp,10);
    if(! endp || *endp != '.' ) return -1;    
    return ( endp - m_str(mp) + 1 );
}


static int next_dot(int m, int *p, int w)
{    
    int l = m_len(m);
    while( *p < l ) {
	int ch =  CHAR(m,*p);
	(*p)++;	
	if( !ch || ch  ==  '.' ) {
	    m_putc(w, 0);
	    return ch;
	}
	m_putc(w,ch);
    }
    return -1;
}



/* specify a variable like a x11 resource
   start with asterix *
   each component will get type VSET
   last component will get given type
*/
int mvar_parse( int mp, int type_id  )
{
    int ch = CHAR(mp,0);
    if( ch != '*' ) return mvar_lookup_path(mp,type_id);

    /* multi-point-parser */
    int id    =  0;
    int group =  0;
    int start =  1;
    int w     =  m_create(20,1);
    int t     =  VAR_VSET;
    
    do {
	m_clear(w);
	ch = next_dot( mp, &start, w );
	if( ch < 0 ) ERR("error parsing %s", m_str(mp));
	if( ch == 0 ) t = type_id;  
	group = id;	
	id = mvar_lookup(group, m_str(w), t);

    } while( ch );
    m_free(w);
    return id;
}


/* suche nach einer variable mit name (group,name)
   wird die variable gefunden wird ihre ID übergeben.
   wird die variable nicht gefunden und der typename 
   ist nicht gesetzt wird -1 übergeben

   wird die variable nicht gefunden und der typename 
   ist gesetzt wird eine neue variable angelegt:

   falls group==0 var_create
   sonst var_create und der variable group die id hinzufuegen
   die var mit der der nummer x wird im
   array MVAR_MEM ab position (x-1) gespeichert!

   
*/
int mvar_lookup( int group, char *name, int type_id )
{
    int key = var5_lookup( group,name,type_id );
    if(key < 0 && type_id >= 0) {
	ERR("INTERNAL ERROR: could not create a var with name: %s and  type: %d", name, type_id );
    }
    return key;
}

/* create a new anon-variable, t must be specified */ 
int mvar_anon( int g, int t )
{
    ASERR( t>=0, "type must be specified" );
    /* make it easier to create unique names */
    static unsigned long n = 0;
    char buf[29] = { 0 };
    /* create a unique name - should never iterate :-) */
    do {
	n++;
	sprintf(buf, "€%lx", n );
    } while( mvar_lookup( g, buf, -1 ) > 0 );
    
    return mvar_lookup(g,buf,t);
}

int mvar_vset(void)
{
    return mvar_anon(0,VAR_VSET);
}

int mvar_lookup_path( int mp, int  t )
{
    int p,g;
    p=mvar_parse_path(mp,&g);
    if( p < 0 ) return -1;
    return mvar_lookup(g, mls(mp,p), t);
}

void mvar_destruct(void)
{
    mvar_free_all();
    var_register_destroy();
    var_callback_destroy();
    m_free_list(HASH_TABLE);
}


void mvar_init(void)
{
    mvar_registry( &STRING_VAR_IF, "STRING", VAR_STRING );
    mvar_registry( &INTEGER_VAR_IF, "INTEGER", VAR_INTEGER );
    mvar_registry( &VSET_VAR_IF, "VSET", VAR_VSET );
}

/*        -------------------------------------------------------------------------------------       */
/* var5 hashmap ------------------------------------------------------------------------------------- */
// Dedicated to Pippip, the main character in the 'Das Totenschiff'
// roman, actually the B.Traven himself, his real name was Hermann
// Albert Otto Maksymilian Feige.  CAUTION: Add 8 more bytes to the
// buffer being hashed, usually malloc(...+8) - to prevent out of
// boundary reads!  Many thanks go to Yurii 'Hordi' Hordiienko, he
// lessened with 3 instructions the original 'Pippip', thus:
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
    hash - list of integer
    buf  - pointer to buffer with HASH_SIZE bytes
 */ 
inline static int hash_lookup( int hash, void *buf,
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
	    if( cmpf(ctx, *d, buf) ) return *d;
	}
    }
    
 new_item:
    // create new item in item-list in hash-table 
    if( ! newf ) return -1;
    hash_item = newf(ctx,buf);    
    m_put(*hash_item_list, &hash_item);
    return hash_item;
}




inline static char cmp64( void *a, void *b )
{
    register uint64_t *u0 = a;
    register uint64_t *u1 = b;
    return (u0[0] == u1[0]) && (u0[1] == u1[1]) && (u0[2] == u1[2]) && (u0[3] == u1[3]);
}

/** key2 points to the keybuffer that was presented to hash_lookup
 *  key1 is the index of one already stored element in the hashmap
 * returns: 1 if keys match, 0 if keys differ 
 */
int var5_compare_keys( void *ctx, int key1, void *key2 )
{
    var_t *v1 = *mvar_get(key1);
    var_t *v2 = (var_t*) key2;
    return cmp64(v1,v2);

    //    return
    //	( v1->group == v2->group ) &&
    //	( strncmp(v1->name,v2->name,MAX_VARNAME)==0 );
}

/** create a new variable  
 *
 * 
 */
int var5_create_var( void *ctx, void *keyptr )
{
    var_t *key = keyptr;
    int typeid = (intptr_t)ctx;
    int p = mvar_create( typeid );
    var_t *v = *mvar_get(p);
    memcpy(v->name, key->name, MAX_VARNAME  );
    v->group=key->group;    

    /* a group-id equals variable-id, it's a container to for variables */
    if(key->group) {
	if( mvar_type(v->group) != VAR_VSET ) 
	    WARN("Var: %d is not a VSET", v->group);
	mvar_put_integer(v->group, p, -1);
    }
    return p;
}



int var5_lookup( int group, char *name, int  typeid )
{
    if(!HASH_TABLE) {
	HASH_TABLE=m_create( HASH_TABLE_SIZE, sizeof(int) );
	m_setlen(HASH_TABLE,HASH_TABLE_SIZE );
    }
    
    var_t v;
    v.group = group;
    strncpy( v.name, name, MAX_VARNAME );
    return hash_lookup( HASH_TABLE, &v,
			var5_compare_keys,
			typeid >= 0 ? var5_create_var : NULL,
			(void*)(intptr_t)typeid );	
}

void var5_delete_hash( var_t *v )
{
    int p, *d;
    uint32_t c = simple_hash(v); /* lookup key in hash-table */
    int *hash_item_list = mls(HASH_TABLE, c); /* list of keys with same hash */
    m_foreach( *hash_item_list, p, d ) {
	var_t *v1 = *mvar_get(*d);
	if( v1->group == v->group && strncmp(v1->name,v->name, MAX_VARNAME)==0 ) {
	    m_del( *hash_item_list, p);
	    return;
	}
    }
}


