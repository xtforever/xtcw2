#include "m_tool.h"
#include "mls.h"
#include "subshell.h"
#include <unistd.h>
#include <ctx.h>




void test_signal(void)
{

    int args = m_str_split(0, "sh -c ./test-error.sh", " ",1 );
    int sh = shell_create( args );
    m_free_strings(args,0);
    signal (SIGCHLD, shell_signal_cb);
    usleep(100000);	    
    int buf = m_create(100,1);
    
    while( shell_getline(sh,0, buf) != -1 )
	{
	    if( m_len(buf) > 1 )
		puts( m_str(buf) );
	    usleep(100000);	    
	}
    m_free(buf);
    
    shell_close(sh);    

}


void test1( char *s, char *c, int t )
{
    char **d; int p;
    int a = m_str_split(0, s, c, t );
    printf( "%s\n", s );
    m_foreach(a,p,d) {
	printf( "[%s] ", *d );
    }
    printf( "\n\n" );
    m_free_strings(a,0);
}

int main(int argc, char **argv)
{
    m_init();
    trace_level=1;


    test_signal(); 
    
    test1( "  hello, 123, 456, 789  ", " ",1 );
    test1( "  hello, 123, 456, 789  ", ",",1 );
    test1( "  hello, 123, 456, 789  ", " ",0 );
    test1( "  hello, 123, 456, 789  ", ",",0 );

    test1( "  hello, 123, 456, 789  ", ", ",0 );
    test1( "  hello, 123, 456, 789  ", ", ",1 );


    test1( "", ",",0 );
    test1( "", " ",1 );



    test1( "hello 123 456 789", " ",1 );


	
    m_destruct();
}
