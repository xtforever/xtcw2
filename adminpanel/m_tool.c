#include "m_tool.h"
#include "mls.h"


int ms_va_app(int m, va_list ap)
{
    char *name;
    while( (name = va_arg( ap, char* )) != NULL )
    {
	name = strdup(name);
	m_put(m,&name);
    }
    return m;
}

int ms_app(int m, ...)
{
    va_list ap;
    if(!m) m=m_create(3,sizeof(char*));
    va_start(ap,m);
    ms_va_app(m,ap);
    va_end(ap);
    return m;
}



static void cskip_ws(char**s)
{
    while( isspace(**s))  (*s)++; 
}

static void ctrim( char **a, char **b )
{
    while( *a < *b && isspace(	(*b)[-1] )) {
	(*b)--; 
    }
}
static void skip_delim(char **s, char *delim )
{
    while( **s && *delim && **s == *delim ) { (*s)++; delim++; }
}

static void skip_unitl_delim( char **s, char *delim )
{
    char *p = strstr( *s, delim );

    if( p ) { 			/* delim found, set s to start of delim */
	*s = p;
	return;
    }

    while( **s ) (*s)++;	/* delim not found, set s to end of string */
}


static int cut_word( char **s, char *delim, int trimws, char **a, char **b )
{
 
    // remove ws from start word
    if( trimws ) cskip_ws( s);
    *a = *s; // start of word
    skip_unitl_delim( s, delim ); // s points to first char of delimiter
    *b = *s; // b points to first char after word    
    if( trimws ) ctrim( a, b ); // start at b and remove ws until a 
    return (*b) - (*a);
    
}

static char* dup_word( char **s, char *delim, int trimws )
{
    char *a,*b;
    int len = cut_word(s,delim,trimws,&a,&b);
    if( len ) {
	char *word = calloc(1, len + 1 );
	memcpy( word, a, b-a );
	word[len] = 0;
	return word;
    }
    return NULL;   
}

int ms_split(int m, char *s, char* delim, int trimws )
{
     if(!m) m=m_create(3,sizeof(char*));
     char *w;

     while( *s ) {
	 w = dup_word( &s, delim, trimws );
	 if( w ) m_put(m,&w);
	 skip_delim( &s, delim );
     }
     
     return m;
}
