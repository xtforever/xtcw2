#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mls.h"
#include "svar2.h"



int main()
{
    m_init();
    trace_level=2;
    svar_create();
    
    int str = s_printf(0,0, "hello.world");
    int v = svar_lookup( str, SVAR_FLOAT );
    printf("float %d\n", v );
    v = svar_lookup( str, SVAR_FLOAT  );
    printf("float %d\n", v );

    s_printf(str,0, "hello");
    v = svar_lookup( str, -1  );
    printf("var:%d type:%s\n", v, svar_typename(v) );

    m_free(str);
    
    /* int keys = read_keys_from_file( "john.txt" );  */
    /* int str = s_printf(0,0, "hello world"); */
    /* print_stacksize(); */
    /* m_free(str); */
    /* m_free_list_of_list(keys); */


    svar_destruct();
    m_destruct();
}
