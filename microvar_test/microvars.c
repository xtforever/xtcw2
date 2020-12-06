#include "mls.h"
#include "micro_vars.h"

/*
  mls to json test

  
  syntax:

  *VarName: 5001  - record
  VarName: x      - string 
  VarName: x y    - array

 JSON:

 {  "name" : "value", 





static void on_update(void *p)
{
    char *name = p;
    printf("update: %s %d\n", name, *mv_svar(name) );
}

int create(char *name)
{
   int q= mv_zerovar(name);
   mv_onwrite( q, on_update, name, 0 );
   return q;
}


int main()
{

    m_init();
    mv_init();

    int i;
    char *names[] = { "v1", "v2", "v3", "v4" }; 
    int q[4];
    for(i=0;i<4;i++) {
	q[i] = create( names[i] );
    }

    int status[4] = { 0 };
    for(i=0;i<10000;i++) {

	int val = rand() % 10;
	int var = rand() % 4;
	int quark = q[var];
	
	if( val == 0 ) {
	    mv_onwrite( quark, on_update, names[var], 0 );
	    status[var] = 0;
	}
	if( val == 1 ) {
	    mv_onwrite( quark, on_update, names[var], 1 ); 
	    status[var] = 1;
	}
	
	mv_write(quark, i );

    }
    
    mv_destroy();
    m_destruct();
}
