#include "svar_expand.h"
#include "mls.h"
#include "svar2.h"
#include <stdarg.h>

/* svar_get_str */


void svexp_init(  svexp_t *se  )
{
  memset( se, 0, sizeof *se );
}

void svexp_free(  svexp_t *se )
{
  m_free_strings( se->splitbuf,0 );
  m_free( se->values );
  m_free( se->indices );
  m_free( se->buf );
  memset( se,0,sizeof *se);
}


void svexp_realloc_buffers( svexp_t *se )
{
  if( !se->buf ) {
    se->splitbuf = m_create(10,sizeof(char*));
    se->values=m_create(10,sizeof(char*));
    se->indices=m_create(10,sizeof(int));
    se->buf=m_create(100,1);
    return;
  }

  m_free_strings(se->splitbuf,1);
  m_clear(se->values); m_clear(se->indices);
  m_clear(se->buf);
  se->max_row=0;
}

// tbd
// returns:
//  1 ALL
//  2..n SINGLE
//  0 ERROR
//
static int parse_index(const char **s)
{
  int val;
  const char *p = *s;

  if( *p !='[' ) return 0;
  p++;
  if( *p == '*' && p[1] == ']') {
    *s=p+2;
    return 1;
  }

  val=0;
  while( isdigit(*p) )
    {
      val *= 10; val+= *p - '0';
      p++;
    }
  if( *p == ']' ) { *s=p+1; return val+2; }

  return 0;
}


void svexp_parse(svexp_t *se, const char *frm)
{
  ASSERT( frm && se );

  svexp_realloc_buffers(se); // alloc, or clear buffer

  int b=se->splitbuf;
  char *cp,prev; const char *s,*s0;

  prev=0; s = frm; s0=s;

  for(;;) {

    if( *s == 0 || (*s == '$' && prev != '\\') )
      {
        // prefix ?
        if( s0 != s ) {
          cp = strndup(s0,s-s0); // copy without *s
          m_put(b,&cp);
        }
        if( *s == 0 ) break; // exit

        // cut out varname
        s0=s; //  token start (with $-prefix)
        s++; // skip leading $

        if( *s == '\'' ) { s++; } // expand with single quotes

        while( isalnum(*s) || *s=='_' ) s++; // UNTIL DELIMITER FOUND
        // copy without delimiter
        cp=strndup(s0,s-s0);
        m_put(b,&cp); m_put(se->values,&cp);

        int index = parse_index(&s);
        m_put( se->indices, &index );
        if( *s == 0 ) break; // exit

        s0=s; // s0 points to delimiter
      }
    prev = *s;
    s++;
  }
};

/** @brief erzeugt aus *s einen string mit escape zeichen
    @param s2 - 0 oder mls liste
    @param s - ein string der umgewandelt werden soll
    @param quotes - falls quotes==1 werden einfache anführungszeichen um die variable gesetzt
    @return gültiger mls string (liste mit breite 1, string ohne Nullbyte)
 */

static int field_escape(int s2, const char *s, int quotes)
{
  // "*s" ist der zu speichernde string
  // um das sql-kommando zu generieren werden sonderzeichen
  // "escaped". dies ist ein gutes beispiel warum die "mls"
  // speicherverwaltung vorteile bietet. der benötigte speicher
  // von mysql_escape_string muss abgeschätzt und reserviert werden.
  // die gleiche funktion in mls ist viel einfacher zu verwenden
  if( quotes ) m_putc( s2, '\'');
  escape_buf( s2, (char*)s );
  if( quotes ) m_putc( s2, '\'');
  return s2;
}

/**
    @return einen gültigen string - immer
 */
const char* svexp_expand( svexp_t *se, char *prefix, int row )
{
    const char *val;
    int count,var, index;
    int p,vn; char **d, *s;
    int quotes = 0;
    int varname = m_create(100,1);
    m_clear(se->buf); int buf = se->buf;
    vn=0; // number of variables


  // string zusammenfügen
  // variablen werden durch ihren wert ersetzt
  // variablen werden durch ein führendes "$" erkannt
  // folgt dem $ ein "'" wird der eingesetzte wert durch "'" umschlossen
  //
  m_foreach( se->splitbuf, p,d ) {
    s = *d;

    if( *s != '$' ) { // einfacher text-baustein, nur anhängen
      m_write( buf, m_len(buf), s, strlen(s) );
    }
    else  // variable found
      {
        if( s[1] == '\'' ) { quotes=1; s++; } else quotes=0;
	s_printf(varname,0,"%s.%s", prefix, s+1 );
        var = svar_lookup(varname,-1);
        index = INT(se->indices, vn ); vn++;
	count = svar_get_str_count(var);
        // expand var
	if( index == 1 ) { // erzeuge eine liste von werten
	    val = svar_get_str(var,0);
	    field_escape(buf, val, quotes);
	    for( index=1; index < count; index++ ) {
		m_putc(buf, ',' );
		val = svar_get_str(var,index);
		field_escape(buf, val, quotes);
	    }
	}
	else { // index != 1  i.e. not expand all i.e. index != [*]
	    if( index == 0 ) index=row; // falls kein index angegeben wurde, benutze (row)
	    else index -= 2;

	    if( index < count ) {
		val = svar_get_str(var,index);
		field_escape(buf, val, quotes);
	    }
	    
	}
      } // variable expandiert
  }

  m_putc( buf, 0 );
  m_free(varname);
  return   mls( buf, 0 );
}

/** expandiert den string frm mit den variablen aus vl
 * @return	einen Zeiger auf den expandierten string
 *		dieser string wird auch als variable
 *		unter dem namen se_string in vl gespeichert
 */
const char*   svexp_string( char *prefix, const char *frm )
{
    svexp_t se;
    int var, varname;
    
    svexp_init( &se );
    svexp_parse( &se, frm );
    svexp_expand( &se, prefix, 0 );
    
    varname = s_printf(0,0,"%s.se_string", prefix );
    var = svar_lookup(varname,SVAR_MSTRING);

    svar_t *v = svar(var);
    if(v->value==0) v->value=m_create(100,1);
    m_copy( v->value, se.buf );
    svexp_free(&se);
    m_free(varname);
    return m_str(v->value);
}



int s_strdup(int r, const char *str)
{
    int l;
    if( is_empty(str) ) { // return empty buffer
	if( r <= 0 ) r = m_create(1,1); else m_clear(r);
	m_putc(r,0);
	return r;
    }
    
    l = strlen(str)+1; // return copy of string 
    if( r <= 0 ) r = m_create(l,1); else m_clear(r);
    m_write(r,0,str,l);
    return r;
}

static int app_cstr_mstrl( int m, const char *name )
{
    int x = s_strdup(0,name);
    m_put(m,&x);
    return m;
}


static int vas_create_mstring_array(int m, va_list ap )
{
    char *name;
    while( (name = va_arg( ap, char* )) != NULL )
    {
	app_cstr_mstrl( m, name );
    }
    return m;
}

int vas_append_mstring_array(int m,int offset, va_list ap)
{
    if(!m) {
	m=m_create(10,sizeof(int));
    } else {
	int len = m_len(m);
	if( offset >= 0 ) {
	    while( offset < len-- ) {
		m_free( INT(m,len) );		
	    }
	}
	m_setlen(m,offset);
    }
    
    vas_create_mstring_array(m,ap);
    return m;

}



int append_mstring_array(int m,int offset, ... )
{
    va_list ap;
    va_start(ap,offset);
    vas_append_mstring_array(m,offset,ap);
    va_end(ap);
    return m;
}
