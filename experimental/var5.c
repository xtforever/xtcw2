/* var5 interface 

   init

   mvar_registry( &STRING_VAR_IF, "STRING", VAR_STRING );
     var_register_destroy					removed - integrated in mvar_destruct

   create anon variables:
     int id = map_anon_create(0, "VSET" );			mvar_anon

   create anon vset variables:
     int  g = vset_create();					mvar_group()

   find/create a variable
     int q =  map_lookup( group,sname,stype );			mvar_lookup
     int q =  map_lookup_path(mp,stype);			mvar_lookup_path
     int offs = mvar_parse_path(mp,&group);
     mvar_destroy(q);						mvar_free
     mvar_destroy_all						removed - integrated in mvar_destruct

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
     var_callback_destroy

     
     
 */
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mls.h"
#include "ctx.h"

#define VAR_INTEGER 0
#define VAR_STRING 1
#define VAR_VSET 2
#define VAR_REC 3

#define MAX_VAR 32
#define MAX_VARNAME (MAX_VAR-4)

struct var_st;
typedef struct var_st var_t; 
typedef void (*varsig_t) (void*,int);


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

struct var_name {
    uint32_t group;
    char name[MAX_VARNAME+1]; /* one padding zero */
};

typedef
struct var_st {
    uint8_t var_if;
    struct var_name;
} var_t;

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

void var_call_callbacks( int q )
{
    struct var_callback *ent = callback_lookup(q);
    if(! ent->signal ) return;

    /* um rekursion zu verhindern wird signal blockiert */
    if( ent->locked ) return;
    ent->locked = 1;
    
    int p;
    struct var_signal *sig;
    m_foreach( ent->signal, p, sig ) {
	if( sig->fn ) sig->fn( sig->d, q );
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

void mvar_destroy(int p);
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
	mvar_destroy(*d);
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

int  var_register_new( void *funcs, char *name )
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

struct var_if *vreg_getif(uint8_t id)
{
    return vreg_get(id)->fn;
}

struct var_if *var_register_lookup(char *name)
{
    int p=vreg_ifnum(name);
    if( p < 0 )
	ERR("could not find registerd class %s", name );
    return vreg_getif(p);
}

char* var_register_typename( int id )
{
    return vreg_get(id)->name;
}


static int MVAR_MEM = 0;

struct mvar_mem_st {
    int init;
    var_t *var;
};

static inline var_t *mvar_get(int p )
{
    ASERR(p>0,"var <=0: %d", p);
    return ((struct mvar_mem_st*)mls(MVAR_MEM,p -1)) -> var;
}

/* returns number of var +1, because that garanties 
   that var-number is never zero
   maybe a bad idea 
*/
int mvar_create( char *type_name )
{
    struct mvar_mem_st *v;
    int ifnum = vreg_ifnum(type_name);
    if( ifnum < 0 ) ERR("interface %s not defined", type_name);
    int p = ctx_init(&MVAR_MEM, 10, sizeof(*v) );
    v = mls(MVAR_MEM,p);
    v->var = var_create( vreg_getif(ifnum) );
    v->var->var_if = ifnum;
    return p +1;
}

void mvar_free_cb( int *ctx, int p )
{
    struct mvar_mem_st *v = mls(*ctx,p);
    var_destroy(v->var);
    v->var=0;
}

void mvar_destroy_all(void)
{
    ctx_destruct( &MVAR_MEM, mvar_free_cb );
}

void mvar_destroy(int p)
{
    ctx_free( &MVAR_MEM, p-1, mvar_free_cb );
}

int mvar_put_string( int id, char *s, int p )
{
    return var_put_string( mvar_get(id), s, p );
}
char* mvar_get_string( int id, int p )
{
    return var_get_string( mvar_get(id), p );
}

int mvar_put_integer( int id, long s, int p )
{
    return var_put_integer( mvar_get(id), s, p );
}
long mvar_get_integer( int id, int p )
{
    return var_get_integer( mvar_get(id), p );
}
int mvar_type(int id)
{
    return mvar_get(id)->var_if;
}
int mvar_group(int id)
{
    return mvar_get(id)->group;
}

/* dangerous - could be not null terminated */
char* mvar_name(int id)
{
    return mvar_get(id)->name;
}

int mvar_length(int id)
{
    return var_length( mvar_get(id) );
}

int mvar_path(int id, int mp)
{
    return s_printf(mp,0,"#%u.%.28s",
		  mvar_group(id), mvar_name(id) );
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
int map_lookup( int group, char *name, char *typename )
{
    /* find a variable with name=(group,name) */
    int p;
    struct mvar_mem_st *vm;
    m_foreach(MVAR_MEM,p,vm) {
	if(! vm->init ||  ! vm->var ) continue; 
	var_t *v = vm->var;
	if( v->group == group && strncmp(v->name,name,sizeof(v->name)) == 0 ) {
	    return p +1; /* never returns zero */
	}
    }

    if( !typename ) return -1;
    /* if no variable is found and a typename provided */
    /* create and init a new variable */
    
    p = mvar_create(typename);
    var_t *v = mvar_get(p);
    strncpy(v->name, name, sizeof(v->name)-1 );
    v->group=group;    

    /* a group-id equals variable-id, it's a container to for variables */
    if(group) {
	if( mvar_type(group) != VAR_VSET ) 
	    WARN("Var: %d is not a VSET", group);
	mvar_put_integer(group, p, -1);
    }

    return p;
}

/* create a new anon-variable, t must be specified */ 
int map_anon_create( int g, char *t )
{
    ASERR( !is_empty(t), "type must be specified" );
    /* make it easier to create unique names */
    static unsigned long n = 0;
    char buf[29] = { 0 };
    /* create a unique name - should never iterate :-) */
    do {
	n++;
	sprintf(buf, "€%lx", n );
    } while( map_lookup( g, buf, NULL ) > 0 );
    
    return map_lookup(g,buf,t);
}

int vset_create(void)
{
    return map_anon_create(0,"VSET");
}

int map_lookup_path( int mp, char *t )
{
    int p,g;
    p=mvar_parse_path(mp,&g);
    if( p < 0 ) return -1;
    return map_lookup(g, mls(mp,p), t);
}




void var_dump(int id)
{
    int tt = trace_level;
    trace_level=2;
    int varname = mvar_path(id, 0);
    int t = mvar_type(id) ;
    char *typename = var_register_typename( t );
    printf("varname: %s\n", m_str(varname)  ); m_free(varname);
    printf("typename: %s\n", typename  );
    int l = mvar_length(id);
    printf("length: %d\n", l );
    printf("content: ");

    if( t == VAR_VSET ) {
	for(int i=0;i<l;i++) {
	    int x = mvar_get_integer(id,i);
	    printf("dumping var: %d\n", x );
	    var_dump(x);
	}
    }
    else {
	int ch = 32;
	for(int i=0;i<l;i++) {
	    char *s = mvar_get_string(id,i);
	    printf("%c%s", ch, s); ch=',';
	}	
	putchar(10);
    }

    putchar(10);
    trace_level=tt;
}



int  mt( int g, char *s, char *t )
{
    printf("lookup (%d:%s) %s: id=",g,s,t ? t : "null" );
    int id = map_lookup( g,s,t );
    printf("%d\n", id );
    return id;
}


void map_test(void)
{

    int vs = vset_create();    
    printf("created vset: %d\n", vs );

    int x= mt( vs, "my-int", "INTEGER" );
    mt( vs, "my-int", "INTEGER" );
    
    mt( 0, "hello", "STRING" );
    mt( vs, "hello", "STRING" );
    mt( 0, "hello", NULL  );
    mt( vs, "hello", NULL  );
    mt( vs+1, "hello", NULL  );


    printf("adding own-list to integer var %d\n", x );
    int y = mt( x, "own-list", "INTEGER" );

    int mp = mvar_path(x,0);
    printf("my-int full name: %s\n", m_str(mp) );

    int g,p;
    p=mvar_parse_path(mp,&g);
    printf("parsed: name=%s, group=%d\n", (char*)mls(mp,p), g );

    int xx = map_lookup_path( mp, 0 );
    printf("map_lookup_path returns: %d\n", xx);
    mt( vs, "my-int", 0 );
    mt( vs, "2nd-int", "INTEGER" );
    printf( "elements in %s: %d\n", m_str(mp), mvar_length(xx));
    m_free(mp);

    var_dump(vs);
    var_dump(xx);
    var_dump(y);

    mvar_destroy(vs);
    mt( vs, "hello", NULL );
    mt( vs, "hello1", NULL );
    (void)y;

    int rec = mt( 0, "my-record", "VSET" );
    int col, cols = 3;
    col = mt( rec, "userid", "INTEGER" );
    col = mt( rec, "name", "STRING" );
    col = mt( rec, "age", "INTEGER" );

    char *ent[3] = { "100", "jens", "42" };
    for(int i=0;i<cols;i++) {
	int var = mvar_get_integer(rec,i);
	mvar_put_string( var, ent[i], -1 );
    }
    
    var_dump( rec );
    mvar_destroy( rec );
}

void cb1(void *c, int q)
{
    printf("callback for %d\n", q );
    var_dump(q);
    puts("leaving cb");
}


void callback_test(void)
{
    int vs = vset_create(); 
    int q1 = map_lookup(vs,"cb-test1", "INTEGER");
    int q2 = map_lookup(vs,"cb-test2", "INTEGER");
    int q3 = map_lookup(vs,"cb-test3", "INTEGER");

    mvar_put_integer( q1, 101, -1 );
    mvar_put_integer( q2, 202, -1 );
    mvar_put_integer( q3, 303, -1 );
    
    var_set_callback( q1, cb1, NULL, 0 );
    var_set_callback( q2, cb1, NULL, 0 );
    var_set_callback( q3, cb1, NULL, 0 );

    var_set_callback( q1, cb1, NULL, 1 );
    var_set_callback( q3, cb1, NULL, 1 );

    var_call_callbacks(q1);
    var_call_callbacks(q2);
    var_call_callbacks(q3);

    mvar_destroy( vs );
}




/*
  g  =   zvar_group_create();					
  v  =   zvar_create( g, name, STRING );
  sz =   zvar_get_name(); g[0..3]=  


 */


void test1()
{
    puts("start");
    mvar_registry( &STRING_VAR_IF, "STRING", VAR_STRING );
    mvar_registry( &INTEGER_VAR_IF, "INTEGER", VAR_INTEGER );
    mvar_registry( &VSET_VAR_IF, "VSET", VAR_VSET );
   
    char *s;
    var_t *m;
    m = var_create( &STRING_VAR_IF );
    var_put_string( m, "hello", 0 );
    s = var_get_string( m, 0 );
    printf("stored string is: %s\n", s );
    var_destroy( m ); 

    m = var_create( &INTEGER_VAR_IF );
    var_put_string( m, "1002", 0 );
    s = var_get_string( m, 0 );
    printf("stored string is: %s\n", s );
    var_destroy( m );    

    // mvar_registry( &REC_VAR_IF, "REC", VAR_REC );
    int z = mvar_create( "STRING" );
    mvar_put_string( z, "hello world" , 0);
    s = mvar_get_string( z, 0 );
    printf("stored string is: %s\n", s );
    mvar_destroy(z);

    // int p = mvar_create( "VSET" );
    // int *pvar = vset_get_value( mvar_get(p) );

    map_test();

    callback_test();
    
    mvar_destroy_all();
    var_register_destroy();
    var_callback_destroy();
}


    

int main()
{
    m_init();
    trace_level=3;
    test1();
    m_destruct();
}
