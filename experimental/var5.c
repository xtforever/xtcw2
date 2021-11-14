#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mls.h"
#include "ctx.h"

#define VAR_STRING 1
#define VAR_INTEGER 2
#define VAR_VSET 3
#define VAR_REC 4



struct var_st;
typedef struct var_st var_t; 

struct var_if {
    int id;
    int if_type;
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
    char name[29]; /* one padding zero */
};

typedef struct var_st {
    struct var_if *fn;
    struct var_name name;
} var_t;


void set_error(var_t *v, char *s ) {
};


/* same as var_if */
struct string_var_if {
    struct var_if core;
};

/* same as var_st but with extensions */
struct string_var_st {
    struct string_var_if *fn;
    struct var_name name;
    int lst;
};
typedef struct string_var_st string_var_t;

char *string_get_string(var_t *v, int p)
{
    if( v->fn->if_type != VAR_STRING ) {
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

var_t *string_impl_create(void);
void  string_impl_destroy(var_t *v);
char *string_impl_get_string(var_t *v, int p);
int string_impl_put_string(var_t *v,char *str, int p);
int string_impl_length(var_t *v);

struct string_var_if STRING_VAR_IF = {
 .core = {
    .if_type = VAR_STRING,
    .put_string = string_impl_put_string,
    .get_string = string_impl_get_string,
    .length     = string_impl_length,
    .create = string_impl_create,
    .destroy = string_impl_destroy
 }
 };

int string_impl_length(var_t *v)
{
    string_var_t *s = (string_var_t*)v;
    return m_len(s->lst);
}

var_t *string_impl_create(void)
{
    string_var_t *s = calloc(1, sizeof(string_var_t) );
    s->fn = &STRING_VAR_IF;
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
    struct var_if core;
    char buffer[100];
};

/* same as var_st but with extensions */
struct integer_var_st {
    struct integer_var_if *fn;
    struct var_name name;
    int value;
};

typedef struct integer_var_st integer_var_t;

var_t* integer_impl_create(void);
void   integer_impl_destroy(var_t *v);
char*  integer_impl_get_string(var_t *v, int p);
int    integer_impl_put_string(var_t *v,char *str, int p);
long   integer_impl_get_integer(var_t *v, int p);
int    integer_impl_put_integer(var_t *v, long val, int p);
int    integer_impl_length(var_t *v);

struct integer_var_if INTEGER_VAR_IF = {
 .core = {
    .if_type = VAR_INTEGER,
    .put_string = integer_impl_put_string,
    .get_string = integer_impl_get_string,
    .put_integer = integer_impl_put_integer,
    .get_integer = integer_impl_get_integer,
    .length      = integer_impl_length,
    .create = integer_impl_create,
    .destroy = integer_impl_destroy
 }
 };

var_t *integer_impl_create(void)
{
    integer_var_t *s = calloc(1, sizeof(integer_var_t) );
    s->fn = &INTEGER_VAR_IF;
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
    long val = integer_impl_get_integer(v,p);
    integer_var_t *s = (integer_var_t*)v;
    snprintf( s->fn->buffer, sizeof(s->fn->buffer),
	      "%ld", val );
    return s->fn->buffer;    
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
struct vset_var_st {
    struct vset_var_if *fn;
    struct var_name name;
    int value;
};

typedef struct vset_var_st vset_var_t;
struct vset_var_if VSET_VAR_IF = {
 .core = {
    .if_type = VAR_VSET,
    .put_string = integer_impl_put_string,
    .get_string = vset_impl_get_string,
    .put_integer = integer_impl_put_integer,
    .get_integer = integer_impl_get_integer,
    .length = integer_impl_length,
    .create =  vset_impl_create,
    .destroy = vset_impl_destroy,
 }
 
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

var_t *vset_impl_create(void)
{
    var_t *v = integer_impl_create();
    v->fn = (struct var_if*)&VSET_VAR_IF;
    return v;
}



/* -----------------------------------------------------------*/

/* same as var_if */
struct rec_var_if {
    struct var_if core;
    char buffer[100];
};

/* same as var_st but with extensions */
struct rec_var_st {
    struct rec_var_if *fn;
    struct var_name name;
    int lst;
};

typedef struct rec_var_st rec_var_t;

var_t* rec_impl_create(void);
void   rec_impl_destroy(var_t *v);
char*  rec_impl_get_string(var_t *v, int p);
int    rec_impl_put_string(var_t *v,char *str, int p);
long   rec_impl_get_integer(var_t *v, int p);
int    rec_impl_put_integer(var_t *v, long val, int p);
int    rec_impl_length(var_t *v);
int    rec_to_string (var_t *v, int buf, int p);

struct rec_var_if REC_VAR_IF = {
 .core = {
    .if_type = VAR_REC,
    .put_string = rec_impl_put_string,
    .get_string = rec_impl_get_string,
    .put_integer = rec_impl_put_integer,
    .get_integer = rec_impl_get_integer,
    .length      = rec_impl_length,
    .to_string   = rec_to_string,
    .create = rec_impl_create,
    .destroy = rec_impl_destroy
 }
 };

var_t *rec_impl_create(void)
{
    rec_var_t *s = calloc(1, sizeof(rec_var_t) );
    s->fn = &REC_VAR_IF;
    s->lst=m_create(1,sizeof(int));
    return (var_t*) s;
}

int rec_impl_length(var_t *v)
{
    rec_var_t *s = (rec_var_t*)v;
    return m_len(s->lst); 
}

void rec_impl_destroy(var_t *v)
{
    rec_var_t *s = (rec_var_t*)v;
    m_free(s->lst); 
    free(s);
}

char *rec_impl_get_string(var_t *v, int p)
{
    long val = rec_impl_get_integer(v,p);
    rec_var_t *s = (rec_var_t*)v;
    
    int mp = mvar_path(val,0);    
    snprintf( s->fn->buffer, sizeof(s->fn->buffer),
	      "%s", m_str(mp) );
    m_free(mp);
    return s->fn->buffer;    
}

int    rec_to_string (var_t *v, int buf, int p)
{
    return -1;
}



int rec_impl_put_string(var_t *v,char *str, int p)
{
    return rec_impl_put_integer(v,atol(str),p);
}

int rec_impl_put_integer(var_t *v, long value, int p)
{
    rec_var_t *s = (rec_var_t*)v;
    if( p < 0 ) {
	m_put(s->lst,&value);
	return 0;
    }
    if( p >= m_len(s->lst) ) m_setlen(s->lst,p+1);
    *(int *)mls(s->lst,p)=value;
    return 0;
}

long rec_impl_get_integer(var_t *v, int p)
{
    rec_var_t *s = (rec_var_t*)v;
    if( p >= m_len(s->lst) ) return -1;
    return *(int *)mls(s->lst,p);
}
/* --------------------------------------------------------*/

var_t *var_create( void *dfn )
{
    struct var_if *fn = dfn;
    return fn->create();
}

void var_destroy(var_t*v)
{
    v->fn->destroy(v);
}

int var_put_string( var_t *v, char *s, int p )
{
    return v->fn->put_string ? v->fn->put_string(v,s,p) : -1;
}
char* var_get_string( var_t *v, int p )
{
    return v->fn->get_string ? v->fn->get_string(v,p) : "";
}
long var_get_integer( var_t *v, int p )
{
    return v->fn->get_integer ? v->fn->get_integer(v,p) : -1;
}
int var_put_integer( var_t *v, long val, int p )
{
    return v->fn->put_integer ? v->fn->put_integer(v,val,p) : -1;
}
int var_length( var_t *v )
{
    return v->fn->length ? v->fn->length(v) : -1;
}

static int VAR_IF = 0;

struct var_register_st {
    int init;
    char *name;
    struct var_if *fn;
};

void var_register( void *funcs, char *name )
{
    struct var_if *fn = (struct var_if *) funcs;
    struct var_register_st *v;
    int p = ctx_init(&VAR_IF, 10, sizeof(*v) );
    v = mls(VAR_IF, p);
    v->name = name;
    v->fn = fn;
}

char* var_register_typename( int t )
{
    int p;
    struct var_register_st *d;
    m_foreach( VAR_IF, p, d ) {
	if( d->init ) {
	    if( d->fn->if_type == t ) return d->name;
	}
    }
    ERR("could not find registerd type %d", t);
    return NULL; /* never reached */
    
}


void var_register_destroy(void)
{
    ctx_destruct( &VAR_IF, NULL );
}

struct var_if *var_register_lookup(char *name)
{
    int p;
    struct var_register_st *d;
    m_foreach( VAR_IF, p, d ) {
	if( d->init ) {
	    if( strcmp(name,d->name) == 0 ) return d->fn;
	}
    }
    ERR("could not find registerd class %s", name );
    return NULL; /* never reached */
}


static int MVAR_MEM = 0;

struct mvar_mem_st {
    int init;
    var_t *var;
};

static inline var_t *mvar_get(int p )
{
    return ((struct mvar_mem_st*)mls(MVAR_MEM,p -1)) -> var;
}

/* returns number of var +1, because that garanties 
   that var-number is never zero
   maybe a bad idea 
*/
int mvar_create( char *type_name )
{
    struct mvar_mem_st *v;
    int p = ctx_init(&MVAR_MEM, 10, sizeof(*v) );
    v = mls(MVAR_MEM,p);
    v->var = var_create( var_register_lookup(type_name) );    
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
    return mvar_get(id)->fn->if_type;
}
int mvar_group(int id)
{
    return mvar_get(id)->name.group;
}

/* dangerous - could be not null terminated */
char* mvar_name(int id)
{
    return mvar_get(id)->name.name;
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
	struct var_name *n = & vm->var->name;
	if( n->group == group && strncmp(n->name,name,sizeof(n->name)) == 0 ) {
	    return p +1; /* never returns zero */
	}
    }

    if( !typename ) return -1;
    /* if no variable is found and a typename provided */
    /* create and init a new variable */
    
    p = mvar_create(typename);
    var_t *v = mvar_get(p);
    strncpy(v->name.name, name, sizeof(v->name.name)-1 );
    v->name.group=group;    

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


/* var5 interface 

   create anon variables:
     int id = map_anon_create(0, "VSET" );

   create anon vset variables:
     int  g = vset_create();

   find/create a variable
     int id =  map_lookup( group,sname,stype );
     int id =  map_lookup_path(mp,stype);
     int offs = mvar_parse_path(mp,&group);

   read/write
     mvar_put_string
     mvar_get_string
     mvar_put_integer
     mvar_get_integer

   inspect
     mvar_type
     mvar_group
     mvar_name
     mvar_id
     mvar_path
     mvar_length

   signals
     mvar_onchange 

     
 */


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


    int rec = mt( 0, "my-record", "REC" );
    mt( rec, "my-int", "INTEGER" );
    mt( rec, "my-string", "STRING" );
    var_dump( rec );
    mvar_destroy( rec );
}






/*
  g  =   zvar_group_create();					
  v  =   zvar_create( g, name, STRING );
  sz =   zvar_get_name(); g[0..3]=  


 */


void test1()
{
    puts("start");
   
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

    var_register( &STRING_VAR_IF, "STRING" );
    var_register( &INTEGER_VAR_IF, "INTEGER" );
    var_register( &VSET_VAR_IF, "VSET" );
    var_register( &REC_VAR_IF, "REC" );
    int z = mvar_create( "STRING" );
    mvar_put_string( z, "hello world" , 0);
    s = mvar_get_string( z, 0 );
    printf("stored string is: %s\n", s );
    mvar_destroy(z);

    // int p = mvar_create( "VSET" );
    // int *pvar = vset_get_value( mvar_get(p) );

    map_test();
    
    
    mvar_destroy_all();
    var_register_destroy();
    
}

int main()
{
    m_init();
    trace_level=1;
    test1();
    m_destruct();
}


