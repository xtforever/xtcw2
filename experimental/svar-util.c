#include "svar-util.h"
#include "mls.h"

svar_t* svar( int key );
int svar_lookup(int name, int type );
int svar_get_str_count(int var);
char* svar_get_str(int var, int pos);
void svar_copy_from_mstring(int var, int buf);


/** @brief copy string from src to dest
 * the dst buffer is always resized to max and will contain a termination zero
 * an empty or not-allocated (i.e. zero) src is allowed
 * if dst is zero it will be allocated
 * @returns dst
 */
int s_strncpy(int dst, int src, int max)
{
    int c, src_bytes;
    
    /* create string if none given */
    if( ! dst ) dst = m_create(max,1);
    else if( m_bufsize(dst) < max ) {
	/* realloc if string is too short */
	m_setlen(dst,max);
    }

    /* if src not exists or has zero length, return empty string */
    if( src < 1 || !(src_bytes=m_len(src))) {
	m_putc(dst,0);
	return dst;
    }

    /* copy src to dst, but not more than max-bytes */
    c = Min(max, src_bytes );
    memcpy( m_buf(dst), m_buf(src), c );
    m_setlen( dst, c );

    /* if the string is not zero terminated, append zero */
    if( CHAR(dst,c-1) != 0 ) {
	if( c < max )
	    m_putc(dst,0);
	else
	    CHAR(dst,c-1)=0;
    }

    return dst;
}


static void array_copy( int dest, int destp, int src, int srcp, int src_count  )
{
    void *from = mls(src,srcp);
    void *to   = mls(dest,destp);
    size_t n = src_count * m_width(src); 
    memcpy( to, from, n );
}


static void element_copy( int dest, int destp, int src, int srcp, int src_count, int width  )
{
    for(int i=0;i<src_count;i++) {
	void *from = mls(src,srcp);
	memcpy( mls(dest,destp), from, width );
	srcp++;
	destp++;
    }
}


/** @brief copy *src_count* elements from *src* to *dest*
 * if *dest* is <=0 create a new array
 * if *src_count* is -1 or too big it is set to copy all of *src* starting at *srcp*
 * if *destp* is <0 append *src* to *dest* 
 * @returns dest 
 */  
int m_copy(int dest, int destp, int src, int srcp, int src_count  )
{
    int dest_len;
    int src_len = m_len(src);
    
    if(srcp < 0) srcp=0;
    /* if no lenght is given or length outside of array, copy full array */

    if( src_count < 0 || src_count + srcp > m_len(src) )
	{
	    src_count = src_len - srcp;	
	}

    /* so far, we have checked srcp,src_count, now look at the others */

    /* check number of elements in destination array: dest_len,
       create dest array if it doesn't exists
       if no dest offset is given assume user wants to append
    */
    if( dest <=0 ) {
	if( destp > 0 ) dest_len = destp + src_count; else dest_len = src_count;
        dest = m_create(  dest_len,  m_width(src) );
    } else {
	
	if( destp < 0 ) {
	    destp=m_len(dest);	/* this means: append to src to dest array */
	}
	dest_len = destp + src_count;
    }

    /* resize dest to fit src array only if it is currently too small */
    if( m_len(dest) < dest_len )
	m_setlen(dest, destp + src_count );

    /* shortcurt, nothing to copy */
    if( src_count == 0 )
	return dest;

    /* now we have checked: dest_len,dest,src_count,srcp,destp */
    
    
    /* check if src and dest are of same element size */
    if( m_width(src) == m_width(dest) ) {
	array_copy( dest,destp,src,srcp,src_count);
	return dest;
    }

    /* copy each element from src to dest */
    int width = Min( m_width(src), m_width(dest) );    
    element_copy( dest, destp, src, srcp, src_count, width  );
    return dest;
}



/** @brief insert data into sorted list buf
 * @returns: position of new element 
 */
int m_binsert( int buf, const void *data, int (*cmpf) (const void *a,const void *b ), int with_duplicates )
{
    int left = 0;
    int right = m_len(buf)+1;
    int cur = 1;
    void *obj;
    int cmp;

    if( m_len(buf)==0 ) {
	m_put(buf,data);
	return 0;
    }
    
    while(1) {
	cur = (left+right) / 2;
	TRACE(1, "[%d %d] cur:%d", left, right, cur );	
	obj = mls( buf, cur - 1 );
	cmp = cmpf( data, obj );
	if( cmp == 0 ) {
	    if( ! with_duplicates ) return -1;
	    break;
	}
	if( cmp < 0 ) {
	    right=cur;
	    if( left+1 == right ) break;
	} else {
	    left  = cur;
	    if( left+1 == right ) {
		cur++;
		break;
	    }
	}
    }
    

    cur--;
    TRACE(1, "insert at %d", cur );
    m_ins( buf, cur, 1 );
    m_write( buf, cur, data, 1 );
    return cur;
}

int seperate_by_dot(char *dst, int src, int *start, int max)
{
    while(1) {
	char ch = CHAR(src,*start);
	if( ch == '.' || ch == 0 ) {
	    if( max > 0 ) memset(dst,0,max);
	    return ch;
	}
	if( max > 0 ) *dst++ = ch;
	(*start)++;
	if( max-- < 1000 ) return -1; /* very bad */
    }
}

int compare_int(const void *a,const void *b)
{
    const int *a0 = a, *b0 = b;
    return (*a0) - (*b0);
}

// Dedicated to Pippip, the main character in the 'Das Totenschiff' roman, actually the B.Traven himself, his real name was Hermann Albert Otto Maksymilian Feige.
// CAUTION: Add 8 more bytes to the buffer being hashed, usually malloc(...+8) - to prevent out of boundary reads!
// Many thanks go to Yurii 'Hordi' Hordiienko, he lessened with 3 instructions the original 'Pippip', thus:
#define _PADr_KAZE(x, n) ( ((x) << (n))>>(n) )
static uint32_t FNV1A_Pippip_Yurii(const char *str, size_t wrdlen)
{
	const uint32_t PRIME = 591798841; uint32_t hash32; uint64_t hash64 = 14695981039346656037UL;
	size_t Cycles, NDhead;
	if (wrdlen > 8) {
		Cycles = ((wrdlen - 1)>>4) + 1; NDhead = wrdlen - (Cycles<<3);

        	for(; Cycles--; str += 8) {
			hash64 = ( hash64 ^ (*(uint64_t *)(str)) ) * PRIME;
			hash64 = ( hash64 ^ (*(uint64_t *)(str+NDhead)) ) * PRIME;
		}
	} else {
		hash64 = ( hash64 ^ _PADr_KAZE(*(uint64_t *)(str+0), (8-wrdlen)<<3) ) * PRIME;
	}
	hash32 = (uint32_t)(hash64 ^ (hash64>>32));
	hash32 ^= (hash32 >> 16);
	return hash32 & SVAR_TABLE_SIZE;
} // Last update: 2019-Oct-30, 14 C lines strong, Kaze.


static uint32_t svar_hash(void *sbuf)
{
    // svar_name_t *sn = sbuf;
    uint32_t h = FNV1A_Pippip_Yurii( sbuf, SVAR_MAX2 );
    //    TRACE(2,"hash: %u, parent: %d, name: %s", h, sn->parent, sn->str );
    //    for(int i=0;i<SVAR_MAX;i++) printf("%02x ", (unsigned char) sn->str[i] );
    //    puts("");
    return h;
}



/** find or insert variable (buf) in hash-table (hash)    
 */
int svar_hash_lookup(int hash, void *buf,
		     int (*cmpf)(void *a, void *b),
		     int (*newf)(void *ctx, void *a), void *ctx
		     )
{
    int p, *d; void *var;
    int hash_item;
    uint32_t c = svar_hash(buf); /* lookup key in hash-table */
    int hash_item_list = INT(hash, c); /* list of keys with same hash */

    // new entry
    if( hash_item_list == 0 ) {
	// insert new item-list in hash-table
	hash_item_list = m_create(1, sizeof(int) );
	INT(hash, c ) = hash_item_list;
	goto new_item;	
    }

    // entry found
    // check if the same is already inside
    if(cmpf ) { 
	m_foreach( hash_item_list, p, d ) {
	    var = svar(*d);
	    if( cmpf(var,buf) == 0 ) return *d;
	}
    }
    
 new_item:
    // create new item in item-list in hash-table 
    ASSERT(newf);
    hash_item = newf(buf);    
    m_put(hash_item_list, &hash_item);
    return hash_item;
}

void mlist_free(void *d)
{
    m_free( *(int*) d );
}

void m_free_user(int m, void (*free_h)(void*))
{
    void *d; int p;
    m_foreach(m,p,d) free_h(d);
    m_free(m);
}

void m_free_list_of_list(int m)
{
    m_free_user( m, mlist_free );
}

void m_clear_mstring_array(int m)
{
    int p, *d;
    m_foreach(m,p,d) m_free(*d);
    m_clear(m);
}

void svar_write_callbacks( int q )
{
    int p;
    struct svar_signal *sig;
    svar_t* ent = svar(q);
    if(! ent->write_callbacks ) return;
    
    /* um rekursion zu verhindern wird signal blockiert */
    if( ent->locked ) return;
    ent->locked = 1;
    m_foreach( ent->write_callbacks, p, sig ) {
	if( sig->fn ) sig->fn( sig->d, q );
    }
    ent->locked = 0;
}


int svar_find_signal_handler( int m, void (*fn) (void*, int) )
{
    int p;
    struct svar_signal *sig;
    /* finde signal und falls remove==TRUE entfernen */
    m_foreach( m, p, sig ) {
	if( sig->fn == fn ) {
	    return p;
	}
    }
    return -1;
}

void svar_onwrite( int q, void (*fn) (void*, int), void *d, int remove )
{
    int p;
    struct svar_signal *sig;
    svar_t* ent = svar(q);
    
    p = svar_find_signal_handler( ent->write_callbacks, fn );

    if( remove ) {
	if( p >=0 ) m_del(ent->write_callbacks, p);
	return;
    }

    if( p >= 0 ) return; // signal schon vorhanden
    
    if(!ent->write_callbacks)
	ent->write_callbacks = m_create( 1, sizeof(struct svar_signal));

    /* signal noch nicht vorhanden, remove==FALSE, jetzt hinzufügen */
    sig = m_add( ent->write_callbacks );
    sig->d = d;
    sig->fn = fn;
}

int s_strdup(int r, const char *str)
{
    int l=0;    
    if( ! is_empty(str) )  l = strlen(str); 
    l++;
    if( r <= 0 ) r = m_create(l,1); else m_clear(r);

    if( l == 1 ) {
	m_putc(r,0);
	return r;
    }
    
    m_write(r,0,str,l);
    return r;
}


int app_cstr_mstrl( int m, const char *name )
{
    int x = s_strdup(0,name);
    m_put(m,&x);
    return m;
}


int vas_mstring_list_create(int m, va_list ap )
{
    char *name;
    while( (name = va_arg( ap, char* )) != NULL )
    {
	app_cstr_mstrl( m, name );
    }
    return m;
}

int vas_mstring_list_append(int m,int offset, va_list ap)
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
    vas_mstring_list_create(m,ap);
    return m;

}

int mstring_list_append(int m,int offset, ... )
{
    va_list ap;
    va_start(ap,offset);
    m=vas_mstring_list_append(m,offset,ap);
    va_end(ap);
    return m;
}


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
 *  @param s2 - 0 oder mls liste
 *  @param s - ein string der umgewandelt werden soll
 *  @param quotes - falls quotes==1 werden einfache anführungszeichen um die variable gesetzt
 *  @return gültiger mls string (liste mit breite 1, string ohne Nullbyte)
 */
int field_escape(int s2, const char *s, int quotes)
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
    svar_copy_from_mstring(var, se.buf);

    svexp_free(&se);
    m_free(varname);
    return svar_get_str(var,0);
}

