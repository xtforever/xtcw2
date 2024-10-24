#include "m_tool.h"
#include "mls.h"


int m_str_va_app(int m, va_list ap)
{
    char *name;
    while( (name = va_arg( ap, char* )) != NULL )
    {
	name = strdup(name);
	m_put(m,&name);
    }
    return m;
}

int m_str_app(int m, ...)
{
    va_list ap;
    if(!m) m=m_create(3,sizeof(char*));
    va_start(ap,m);
    m_str_va_app(m,ap);
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


/** split string into list of c-strings
 **/
int m_str_split(int m, char *s, char* delim, int trimws )
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


/** @brief copy string from src to dest
 * the dst buffer is always resized to max and will contain a termination zero
 * an empty or not-allocated (i.e. zero) src is allowed
 * if dst is zero it will be allocated
 * @returns dst
 */
int m_strncpy(int dst, int src, int max)
{
    int c, src_bytes;
    ASSERT(max > 0);

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
int m_mcopy(int dest, int destp, int src, int srcp, int src_count  )
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





int compare_int(const void *a,const void *b)
{
    const int *a0 = a, *b0 = b;
    return (*a0) - (*b0);
}

void m_free_ptr(void *d)
{
    m_free( *(int*) d );
}

void m_free_user(int m, void (*free_h)(void*), int only_clear )
{
    void *d; int p;
    m_foreach(m,p,d) free_h(d);
    if(!only_clear) m_free(m);
}

void m_clear_user( int m, void (*free_h)(void*) )
{
    m_free_user(m,free_h,1);
}

/* free a list of lists */
void m_free_list(int m)
{
    m_free_user( m, m_free_ptr, 0 );
}

/* clear a list of lists */
void m_clear_list(int m)
{
    m_free_user( m, m_free_ptr, 1 );
}

/* clear a list of (malloced char*) */
void m_clear_stringlist(int m)
{
    m_free_user(m,free,1);
}

/* free a list of (malloced char*) */
void m_free_stringlist(int m)
{
    m_free_user(m,free,0);
}


void m_concat(int a, int b)
{
    if(!a || !b) return;
    int p; void *d;
    m_foreach( b,p,d )
	m_put( a,d );
}


/** split a string by one character *delm into multiple m-array strings
 * @returns list of m-arrays  
 **/
int m_split_list( const char *s, const char *delm )
{
    int ls =  m_create(2,sizeof(int));
    int w  =  0;
    int p  =  0;
    int ch;
    
    do {
	ch = s[p];
	if(!w) {
	    w =  m_create( 10,1 );
	    m_put( ls, &w );
	}

	if( ch == *delm || ch == 0 ) {
	    m_putc(w,0); w=0;    
	} else {
	    m_putc(w,s[p]);
	}
	p++;
    } while( ch );
    
    return ls;
}

/* copy from s to buf[p] until *s == ch */
int leftstr( int buf, int p, const char *s, int ch )
{
    int w;
    if(buf < 2 ) buf = m_create( 10,1 );
    m_setlen(buf,p);
    
    if(s) while( (w=*s++) ) {
	if( w == ch || w==0 ) break;
	m_putc(buf,w);
    }
    m_putc(buf,0);
    return buf; 
}



int cmp_int( const void *a0, const void *b0 )
{
        const int *a = a0;
        const int *b = b0;
        return (*a) - (*b);
}

/* einfuegen von key in das array m falls key noch nicht
   existiert
   return: pos von key
*/
int lookup_int(int m, int key)
{
	void *obj = calloc(1,m_width(m));  
	memcpy(obj, &key, sizeof(key) );              	    
        int p = m_binsert( m, obj, cmp_int, 0 );
	free(obj);
        if( p < 0 ) {
                return (-p)-1;
        }
        TRACE(1,"ADD pos:%d key:%d", p, key );
        return p;
}

/**
 * @brief Copies a portion of the list `m` starting at index `a` and ending at index `b` to a new list at position `offs`, and returns the new list.
 *
 * - If `dest` is zero, a new destination list is created.
 * - Indices can be positive or negative:
 *   - Negative indices count from the end to the start of the list.
 *   - The first element is 0, and the last element is -1.
 *
 * Example:
 * @code
 *   m:    0    1    2    3    4
 *       -5   -4   -3   -2   -1
 * @endcode
 *
 * @param dest The destination list where the portion is copied. If set to 0, a new list is created.
 * @param offs The offset position in the destination list where the copied portion is placed.
 * @param m The source list from which the portion is copied.
 * @param a The starting index of the portion to be copied.
 * @param b The ending index of the portion to be copied.
 * @return The new list with the copied portion.
 */
int m_slice(int dest, int offs, int m, int a, int b )
{
	int len = m ? m_len(m) : 0;
	if( b < 0 ) { b+= len; }
	if( a < 0 ) { a+= len; }
	if( b >= len ) b = len -1;
	if( a >= len ) a = len -1;
	if( a < 0 ) a=0;
	int cnt = b-a +1;
	if( cnt < 0 ) cnt=1;
	if( dest <= 0 ) dest = m_create(cnt+offs, m_width(m));
	m_setlen(dest,offs);

	for( int i = a; i <= b; i++ ) m_put(dest, mls(m,i) );
	return dest;
}

/* slice for strings: add a zero byte to the end */
int s_slice(int dest, int offs, int m, int a, int b )
{
	int ret = m_slice(dest,offs,m,a,b);
	if( m_len(ret) > 0 && CHAR( ret, m_len(ret)-1 ) != 0 ) m_putc(ret,0);
	return ret;
}



int s_warn(int m)
{
	if( m ) {
		if( m_width(m)!=1 ) WARN("mstring %d to width: %d bytes", m, m_width(m));
		if( m_len(m) && CHAR(m,m_len(m)-1) != 0 )  WARN("mstring %d not zero terminated", m );
	}
	return 0;
}

/* verify that m is a not zero-length zero terminated string */
int s_isempty(int m)
{
	return ( m == 0 || m_len(m) == 0 || CHAR(m,0) == 0 || m_width(m) != 1 || CHAR(m, m_len(m)-1 ) != 0 ); 
}

void s_write(int m,int n)
{
	if( s_isempty(m) ) return;
	char *d; int p;
	m_foreach(m,p,d) {		
		if( n <= 0 || *d == 0 ) return;
		putchar(*d);
		n--;
	}
}

void s_puts(int m)
{
	if(! s_isempty(m) ) {
		s_write(m,m_len(m));
	}
	putchar(10);
	return;	
}

int s_strncmp(int m,int offs,int pattern, int n)
{
	if( s_isempty(pattern) || n<=0 ) return 0;
	if( s_isempty(m) || offs >= m_len(m) ) return -1;
	
	int p;
	char *d;
	int diff;
	m_foreach( pattern, p, d ) {
		if( n-- <= 0 ) return 0;
		if( offs >= m_len(m) ) return -1;
		diff = *d - CHAR(m,offs);
		if( diff ) return diff;
	}
	return 0;
}

int s_strstr(int m, int offs, int pattern )
{
	if(  s_isempty(m) || s_isempty(pattern) || offs < 0 ) return -1;
	int plen = m_len(pattern);
	int max = m_len(m) - plen;
	for( ;  offs <= max; offs++ ) {
		if( s_strncmp(m,offs,pattern,plen-1) == 0 ) return offs; 
	}
	return -1;
}



int s_replace(int dest, int src, int pattern, int replace, int count )	
{
	if(dest==0) dest=m_create(20,1); else m_clear(dest);
	if( s_isempty(src) ||  s_isempty(pattern) ) return dest;
	int offs = 0;	
	int replace_last = s_strlen(replace) -1;
	int pattern_len  = s_strlen(pattern);
	for(;;) {
		int pos = s_strstr(src, offs, pattern );
		if( pos < 0 ) break;
		m_slice(dest, m_len(dest), src, offs, pos -1 );
		m_slice(dest, m_len(dest), replace,0,replace_last);
		offs = pos + pattern_len;
		if( --count == 0 ) break;
	}
	m_slice( dest, m_len(dest), src, offs, -1 ); 
	return dest;
}

void m_map( int m, int (*fn) ( int m, int p, void *ctx ), void *ctx  )
{
	if( m == 0 || fn == 0  ) return;
	int n=m_len(m);	
	for(int i=0; i<n; i++ ) fn( m,i,ctx);
}
