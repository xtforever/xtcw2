#include "var5.h"
#include "mls.h"
void var_dump(int id)
{
    int tt = trace_level;
    trace_level=2;
    int varname = mvar_path(id, 0);
    int t = mvar_type(id) ;
    char *typename = mvar_name_of_type( t );
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
    int id = mvar_lookup( g,s,t );
    printf("%d\n", id );
    return id;
}


void map_test(void)
{

    int vs = mvar_vset();    
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

    int xx = mvar_lookup_path( mp, 0 );
    printf("mvar_lookup_path returns: %d\n", xx);
    mt( vs, "my-int", 0 );
    mt( vs, "2nd-int", "INTEGER" );
    printf( "elements in %s: %d\n", m_str(mp), mvar_length(xx));
    m_free(mp);

    var_dump(vs);
    var_dump(xx);
    var_dump(y);

    mvar_free(vs);
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
    mvar_free( rec );
}

void cb1(void *c, int q)
{
    printf("callback for %d\n", q );
    var_dump(q);
    puts("leaving cb");
}


void callback_test(void)
{
    int vs = mvar_vset(); 
    int q1 = mvar_lookup(vs,"cb-test1", "INTEGER");
    int q2 = mvar_lookup(vs,"cb-test2", "INTEGER");
    int q3 = mvar_lookup(vs,"cb-test3", "INTEGER");

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
    int id = mvar_lookup_path(b,NULL);
    printf("id=%d path=%s\n", id, s );
    m_free(b);
}




void print_g_id(char *s)
{
    int v_path = s_printf(0,0, s );
    int var_id =  mvar_parse( v_path, NULL );
    printf("Path=%s, id=%d, Spec=(%s,%d)\n", m_str(v_path), var_id, mvar_name(var_id), mvar_group(var_id) ); 
    m_free(v_path);
}


void mvar_free_all();
void  parsing_test( void )
{
    mvar_free_all();
    /* create some test vars */
    int q0 = mvar_lookup(0, "vs", "VSET" ); 
    int q1 = mvar_lookup(0,  "global-1", "INTEGER" );
    int q2 = mvar_lookup(q0, "loc-1", "INTEGER" );
    int q3 = mvar_lookup(q0, "loc-2", "INTEGER" );

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
    int z = mvar_parse( v_path, "INTEGER" );
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
    int z = mvar_anon(0, "STRING" );
    mvar_put_string( z, "hello world" , 0);
    s = mvar_get_string( z, 0 );
    printf("stored string is: %s\n", s );
    mvar_free(z);

    // int p = mvar_create( "VSET" );
    // int *pvar = vset_get_value( mvar_get(p) );

    map_test();

    callback_test();

    puts("parsing test");
    parsing_test();
}


int main()
{
    m_init();
    trace_level=1;

    mvar_init();
    test1();
    mvar_destruct();

    m_destruct();
}
