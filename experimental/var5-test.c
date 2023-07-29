#include "var5.h"
#include "mls.h"
#include "m_tool.h"

#include <stdint.h>
#include <time.h>


static int  mvar_assign(int mstr )
{
    int ls =  m_split_list( m_str(mstr), "=" );
    if( m_len(ls) != 2 ) goto leave;
    int v = mvar_parse( INT(ls,0), VAR_STRING );
    mvar_put_string(v, m_str(INT(ls,1)), 0);

 leave:
    m_free_list(ls);
    return v;
}

int mvar_set(char *mvar, ...) {
    va_list ap;
    va_start(ap,mvar);
    int m = vas_printf( 0,0, mvar, ap );
    va_end(ap);
    int v = mvar_assign( m );
    m_free(m);
    return v;
}

void p_mvar_string(char *mvar, ...) {
    va_list ap;
    va_start(ap,mvar);
    int m = vas_printf( 0,0, mvar, ap );
    va_end(ap);
    
    int v = mvar_parse( m, -1 );
    puts( mvar_get_string(v,0) );

    m_free(m);
}


struct server_info {
    char *name;
    int port;
    char *pwd;
    char *res;
};

void put_server_info(struct server_info *si)
{
    mvar_set("server_info.%s.%s=%s", "pwd",  si->name, si->pwd );
    mvar_set("server_info.%s.%s=%s", "res",  si->name, si->res );
    mvar_set("server_info.%s.%s=%d", "port", si->name, si->port );
}

void dump_server_info(char *name)
{
    p_mvar_string("server_info.%s.%s", "pwd",  name );
    p_mvar_string("server_info.%s.%s", "res",  name );
    p_mvar_string("server_info.%s.%s", "port", name );
}

void dump_struct_fields(char *type_name, char *list, char *index) {
    int ls = m_split_list( list, "," );

    int p; int *d;
    m_foreach( ls, p, d ) {
	p_mvar_string("%s.%s.%s", type_name, m_str( *d ),  index );
    }
    m_free_list(ls);   
}


void test_struct(void) {

    struct server_info si = {
	.name = "127.0.0.1",
	.port = 3306,
	.pwd = "mimikatz",
	.res = "sha1" };

    /* database: port[name], pwd[name], res[name] */

    put_server_info( & si );
    dump_server_info( "127.0.0.1" );
    dump_struct_fields("server_info", "port,pwd,res", "127.0.0.1" );
}



void    mvar_estr_test()
{
    puts("estr started");

    int v = mvar_parse_string( "xgroup.xvar", VAR_STRING );

    mvar_put_string( v, "hello world", 0 );

    // something

    puts( mvar_get_string(v,0));

    int g = mvar_group(v);

    int v2 = mvar_lookup( g, "xvar", -1 );
    int mp = s_printf(0,0, "#%d.xvar", g, "xvar" );
    int v3 = mvar_parse( mp, -1 );
	
    printf("id=%d, g=%d, v2=%d v3=%d\n", v,g,v2, v3 );

    

    /* there is a variable called xvar, now
       we create a string inside the same group as xvar to test the expand function
    */
    int estr = mvar_lookup(g, "pat", VAR_ESTR );
    mvar_put_string( estr, "my patter is: $'xvar. end of msg.", 0);
    puts( mvar_get_string( estr ,0) );
    puts( mvar_get_string( estr ,0) );

    /* now the magic starts, we change xvar and the pattern changes */
    mvar_put_string( v, "Who are we", 0 );
    mvar_call_callbacks( v,0 );

    puts( mvar_get_string( estr ,0) );
    m_free(mp);


    /* lets check array expand */
    int xvar = v3, p=0;
    mvar_put_string( xvar, "stringexpand", p++ );
    mvar_put_string( xvar, "integrated", p++ );
    mvar_put_string( xvar, "in var5", p++ );
    mvar_put_string( xvar, "is a nice feature", p++ );
    mvar_call_callbacks( xvar,0 );

    char *word;
    for( int i=0; *(word=mvar_get_string( estr ,i))  ; i++ )
	puts(word);


    printf("first: %s\n", mvar_get_string( estr ,0) );
    printf("last: %s\n", mvar_get_string( estr ,-1) );
    printf("underflow: %s\n", mvar_get_string( estr ,-100) );
    printf("overflow: %s\n", mvar_get_string( estr , 100) );   
}



/*

  widget edit -- var x
  on action import - read text from var x
  on enter         - write text to var x 
                   

  widget list_select -- var x
  on action select write to var x 
     and call import-action on edit widget

  

*/




void var_dump(int id)
{
    int varname = mvar_path(id, 0);
    int t = mvar_type(id) ;
    char *typename = mvar_name_of_type( t );
    printf("varname: %s\n", m_str(varname)  );
    m_free(varname);
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

}



int  mt( int g, char *s, int t )
{
    char *n = "none";
    if( t >= 0 ) n=mvar_name_of_type(t);
    printf("lookup (%d:%s TYP:%s): id=",g,s,n );
    int id = mvar_lookup( g,s,t );
    printf("%d\n", id );
    return id;
}


void map_test(void)
{

    int vs = mvar_vset();    
    printf("created vset: %d\n", vs );

    int x= mt( vs, "my-int", VAR_INTEGER );
    mt( vs, "my-int", VAR_INTEGER );
    
    mt( 0, "hello", VAR_STRING );
    mt( vs, "hello", VAR_STRING );
    mt( 0, "hello", -1  );
    mt( vs, "hello", -1  );
    mt( vs+1, "hello", -1   );


    printf("adding own-list to integer var %d\n", x );
    int y = mt( x, "own-list", VAR_INTEGER );

    int mp = mvar_path(x,0);
    printf("my-int full name: %s\n", m_str(mp) );

    int g,p;
    p=mvar_parse_path(mp,&g);
    printf("parsed: name=%s, group=%d\n", (char*)mls(mp,p), g );

    int xx = mvar_lookup_path( mp, 0 );
    printf("mvar_lookup_path returns: %d\n", xx);
    mt( vs, "my-int", 0 );
    mt( vs, "2nd-int",VAR_INTEGER );
    printf( "elements in %s: %d\n", m_str(mp), mvar_length(xx));
    m_free(mp);

    var_dump(vs);
    var_dump(xx);
    var_dump(y);

    mvar_free(vs);
    mt( vs, "hello", -1  );
    mt( vs, "hello1", -1 );
    (void)y;

    int rec = mt( 0, "my-record",VAR_VSET );
    int col, cols = 3;
    col = mt( rec, "userid",VAR_INTEGER );
    col = mt( rec, "name",VAR_STRING );
    col = mt( rec, "age",VAR_INTEGER );
    (void)col;
    char *ent[3] = { "100", "jens", "42" };
    for(int i=0;i<cols;i++) {
	int var = mvar_get_integer(rec,i);
	mvar_put_string( var, ent[i], -1 );
    }
    
    var_dump( rec );
    mvar_free( rec );
}


void cb1(void *c, int q, int s)
{
    printf("callback for %d, signal %d\n", q,s );
    var_dump(q);
    puts("leaving cb");
}

void callback_test(void)
{
    int vs = mvar_vset(); 
    int q1 = mvar_lookup(vs,"cb-test1",VAR_INTEGER);
    int q2 = mvar_lookup(vs,"cb-test2",VAR_INTEGER);
    int q3 = mvar_lookup(vs,"cb-test3",VAR_INTEGER);

    mvar_put_integer( q1, 101, -1 );
    mvar_put_integer( q2, 202, -1 );
    mvar_put_integer( q3, 303, -1 );
    
    var_set_callback( q1, cb1, NULL, 0 );
    var_set_callback( q2, cb1, NULL, 0 );
    var_set_callback( q3, cb1, NULL, 0 );

    var_set_callback( q1, cb1, NULL, 1 );
    var_set_callback( q3, cb1, NULL, 1 );

    var_call_callbacks(q1,0);
    var_call_callbacks(q2,0);
    var_call_callbacks(q3,1);

    mvar_free( vs );
}




/*
  g  =   zvar_group_create();					
  v  =   zvar_create( g, name, STRING );
  sz =   zvar_get_name(); g[0..3]=  


 */

void prp( char *s )
{
    int b = s_printf(0,0, "%s", s );
    int id = mvar_lookup_path(b,-1);
    printf("id=%d path=%s\n", id, s );
    m_free(b);
}




void print_g_id(char *s)
{
    int v_path = s_printf(0,0, s );
    int var_id =  mvar_parse( v_path, -1 );
    printf("Path=%s, id=%d, Spec=(%s,%d)\n", m_str(v_path), var_id, mvar_name(var_id), mvar_group(var_id) );
    printf("Var: %s=%ld\n", mvar_name(var_id), mvar_get_integer( var_id, 0 ) );
    m_free(v_path);
}


void mvar_free_all();
void  parsing_test( void )
{
        /* create some test vars */
    int q0 = mvar_lookup(0, "vs",VAR_VSET );
    int q1 = mvar_lookup(0,  "global-1",VAR_INTEGER );
    int q2 = mvar_lookup(q0, "loc-1",VAR_INTEGER );
    int q3 = mvar_lookup(q0, "loc-2",VAR_INTEGER );

    
    
    /* should be id 1 */

    printf("id | name\n");
    int str = mvar_path(q0, 0 );
    printf( "%d - %s\n", q0,m_str(str) );

    mvar_path(q1, str );
    printf( "%d - %s\n", q1,m_str(str) );
   
    mvar_path(q2, str );
    printf( "%d - %s\n", q2,m_str(str) );
    
    mvar_path(q3, str );
    printf( "%d - %s\n", q3,m_str(str) );

    m_free(str);

    /*
      q0  #0.vs
      q1  #0.global-1
      q2  #1.loc-1
      q3  #1.loc-2
    */

    prp( "vs" );
    prp( "global-1" );
    prp( "#1.loc-1" );
    prp( "#1.loc-2" );
    

    int v_path = s_printf(0,0, "*app1.mod1.sub.linewidth" ); 
    int z = mvar_parse( v_path,VAR_INTEGER );
    mvar_put_integer(z,80,0);
    m_free(v_path);

    print_g_id( "*app1.mod1.sub.linewidth" );
    print_g_id( "*app1.mod1.sub" );
    print_g_id( "*app1.mod1" );
    print_g_id( "*app1" );    
}

   

void test1()
{
    puts("start");
    char *s;
    // mvar_registry( &REC_VAR_IF, "REC", VAR_REC );
    int z = mvar_anon(0,VAR_STRING );
    mvar_put_string( z, "hello world" , 0);
    s = mvar_get_string( z, 0 );
    printf("stored string is: %s\n", s );
    mvar_free(z);

    // int p = mvar_create(VAR_VSET );
    // int *pvar = vset_get_value( mvar_get(p) );

    map_test();

    callback_test();

    puts("parsing test");
    parsing_test();
}

void cb2(void *c, int q)
{
    printf("callback for %d\n", q );
    puts("leaving cb");
}


void db1()
{
    int q1 = mvar_lookup(0,"cb-test1",VAR_VSET);
    int q2 = mvar_lookup(q1,"cb-test2",VAR_INTEGER);
    int q3 = mvar_lookup(0,"cb-test1",VAR_VSET);    
    var_dump(q1);
    var_dump(q2);
    mvar_free( q1 );
    mvar_put_integer( q2, 0, -1 );

    (void) q3;
}



int read_keys_from_file( char *filename )
{
    int keys = m_create(1000, sizeof(int) );
    FILE *fp = fopen( filename, "r" );
    ASSERT(fp);
    int buf = m_create(100,1);
    while( m_fscan2(buf,'\n', fp ) == '\n' ) {
	int sbuf = m_create(MAX_VARNAME, 1);
	m_strncpy( sbuf, buf, MAX_VARNAME );
	m_put(keys, &sbuf );
	m_clear(buf);
    }
    fclose(fp);
    m_free(buf);
    return keys;
}


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


int hash_string_key( void *id )
{
    return mvar_lookup( 0,id,VAR_STRING );    
}

void speed_test(void)
{
    int keys = read_keys_from_file( "john.txt" );
    printf("number of keys: %d\n", m_len(keys) );
    check_hash_speed(5,keys, hash_string_key);
    m_free_list(keys);
}




void clash(void)
{
    int keys = read_keys_from_file( "john.txt" );
    printf("number of keys: %d\n", m_len(keys) );

    int xid=m_create(10,1);
    printf("nextid:%d\n", xid);
    
    int max = Min(m_len(keys),  m_len(keys) );
    char *s= m_str(INT(keys,10));
    int id = mvar_lookup( 0, s, VAR_STRING  );
    id = mvar_lookup( 0, s, VAR_STRING  );

    s= m_str(INT(keys,12));
    id = mvar_lookup( 0, s, VAR_STRING  );
    id = mvar_lookup( 0, s, VAR_STRING  );
    
    int u=100; while(u--)
    for(int i=0;i < max; i++ )
	{
	    s= m_str(INT(keys,i));
	    id = mvar_lookup( 0, s, VAR_STRING  );
	    // printf("Lookup %s id=%d\n", s, id );
	}

    dump_hash_statistics();
    printf("id=%d\n", id);
    m_free_list(keys); 
}



void app_test(void)
{
    int dummy = mvar_lookup(0,"dummy", 0 );
    printf("dummy-id: %d\n", dummy );
    trace_level=1;
    int ms = s_printf(0,0, "kunde1.kuerzel" );
    int id = mvar_lookup_path( ms, VAR_STRING );
    char *s;
    

    mvar_put_string( id, "hello World", 0 );
    s = mvar_get_string( id, 0);
    printf("mystring: %s\n", s );
    m_free(ms);
    
}


static void asgn(char *s)
{
    int ls =  m_split_list(s, "=" );
    if( m_len(ls) != 2 ) goto leave;
    int v = mvar_parse( INT(ls,0), VAR_STRING );
    mvar_put_string(v, m_str(INT(ls,1)), 0);

 leave:
    m_free_list(ls);
}

void exp_test(void)
{
    asgn("task1.t1=hello");
    asgn("task1.t2=world");
    const char *s = mvar_str_string( "task1", "form: $t1-$t2");
    fprintf(stderr,"%s\n", s );
}



int main()
{
    m_init();
    
    #ifdef MLS_DEBUG 
    #  define LVL 3
    #else
    #  define LVL 0
    #endif
    trace_level=LVL;
    
    mvar_init();
    // db1();
    // parsing_test();
    // clash();
    //speed_test();
    // app_test();
    // exp_test();


    mvar_estr_test();

    test_struct();
    
    mvar_destruct();
    //
    m_destruct();
}
