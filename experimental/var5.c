#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mls.h"
#include "ctx.h"

#define VAR_STRING 1
#define VAR_INTEGER 2
#define VAR_VSET 3

struct var_st;
typedef struct var_st var_t; 

struct var_if {
    int id;
    int if_type;
    int (*put_string) ( var_t *v, char *s, int p );
    char* (*get_string) (var_t *v, int p);
    int (*put_integer) ( var_t *v, long val, int p );
    long (*get_integer) (var_t *v, int p);
    var_t* (*create)(void);
    void   (*destroy)(var_t *v);
};

struct var_name {
    char name[28];
    uint32_t group;
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
    char *string;
    int alloced;
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

struct string_var_if STRING_VAR_IF = {
 .core = {
    .if_type = VAR_STRING,
    .put_string = string_impl_put_string,
    .get_string = string_impl_get_string,
    .create = string_impl_create,
    .destroy = string_impl_destroy
 }
 };

var_t *string_impl_create(void)
{
    string_var_t *s = calloc(1, sizeof(string_var_t) );
    s->fn = &STRING_VAR_IF;
    return (var_t*) s;
}

void string_impl_destroy(var_t *v)
{
    string_var_t *s = (string_var_t*)v;
    if( s->alloced ) free(s->string);
    free(v);
}

char *string_impl_get_string(var_t *v, int p)
{
    string_var_t *s = (string_var_t*)v;
    if( s->alloced ) return s->string;
    set_error(v, "this var isn't initialized");
    return "";    
}

int string_impl_put_string(var_t *v,char *str, int p)
{
    string_var_t *s = (string_var_t*)v;
    if( s->alloced ) free( s->string );
    s->string = strdup(str);
    s->alloced = 1;
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
    long value;
};

typedef struct integer_var_st integer_var_t;

var_t *integer_impl_create(void);
void  integer_impl_destroy(var_t *v);
char *integer_impl_get_string(var_t *v, int p);
int integer_impl_put_string(var_t *v,char *str, int p);
long integer_impl_get_integer(var_t *v, int p);
int integer_impl_put_integer(var_t *v, long val, int p);

struct integer_var_if INTEGER_VAR_IF = {
 .core = {
    .if_type = VAR_INTEGER,
    .put_string = integer_impl_put_string,
    .get_string = integer_impl_get_string,
    .put_integer = integer_impl_put_integer,
    .get_integer = integer_impl_get_integer,
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
    if( p > m_len(s->value) ) m_setlen(s->value,p+1);
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


#define vset_var_if integer_var_if

/* same as integer_var_st  */
struct vset_var_st {
    struct vset_var_if *fn;
    struct var_name name;
    long value;
};

typedef struct vset_var_st vset_var_t;
void mvar_destroy(int p);
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

struct vset_var_if VSET_VAR_IF = {
 .core = {
    .if_type = VAR_VSET,
    .put_string = integer_impl_put_string,
    .get_string = integer_impl_get_string,
    .put_integer = integer_impl_put_integer,
    .get_integer = integer_impl_get_integer,
    .create = integer_impl_create,
    .destroy = vset_impl_destroy
 }
 };


/* -----------------------------------------------------------*/


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

/* */



/* falls group==0 var_create
   sonst var_create und dem vset hinzufuegen
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
	    return p;
	}
    }

    if( !typename ) return -1;
    /* if no variable is found and a typename provided */
    /* create and init a new variable */
    
    p = mvar_create(typename);
    var_t *v = mvar_get(p);
    strncpy(v->name.name, name, sizeof(v->name.name) );
    v->name.group=group;    

    /* a group-id equals variable-id, it's a container to for variables */
    if(group) mvar_put_integer(group, p, -1);

    return p;
}




int  mt( int g, char *s, char *t )
{
    printf("lookup %d %s %s: id=",g,s,t ? t : "null" );
    int id = map_lookup( g,s,t );
    printf("%d\n", id );
    return id;
}


void map_test(void)
{
    mt( 0, "hello", "STRING" );
    int vs = mvar_create( "VSET" );
    mt( vs, "hello", "STRING" );
    mt( 0, "hello", NULL  );
    mt( vs, "hello", NULL  );
    mt( vs+1, "hello", NULL  );
    mvar_destroy(vs);
    mt( vs, "hello", NULL );
    mt( vs, "hello1", NULL );
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


