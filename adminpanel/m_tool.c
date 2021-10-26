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

int m_split(int m, char *s, char* delim, int trimws )
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



/** @brief insert *data into sorted list buf
 * @returns: position of new element, or -1 if elem. exists 
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
    m_ins( buf, cur, 1 );
    m_write( buf, cur, data, 1 );
    return cur;
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

