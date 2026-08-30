#include "text-wrap.h"
#include "mls.h"


void t2_wrap(int tbuf, int width, int lines)
{
    int p=0;
    int ch=0;
    int last_space = 0;
    int line_start = 0;
    while( p < m_len(tbuf) )
	{
	    ch = CHAR(tbuf,p);
	    if( ch == 32 && line_start == p ) { // lines do not start with space
		line_start = ++p;
		continue;
	    }

	    // break if END OF BUFFER or string end or hard-break
	    int forced_break = ( ch == 13 || p > m_len(tbuf)-2 || ch == 0 );
	    if( ch == 32 || forced_break ) last_space=p;
	    p++;
	    
	    if( p - line_start >= width || forced_break ) {
		int line_end = last_space ? last_space : p; 
		int line_len = line_end - line_start;
		int line_buf = m_create( line_len+1, 1);
		m_write( line_buf, 0, mls(tbuf,line_start), line_len );
		m_putc( line_buf, 0);
		m_put(lines, &line_buf);
		line_start = last_space ? last_space+1 : p;
		last_space = 0;	       
	    }
	}
}


// replace tab with space, remove space/space
// replace nl/nl with cr,
// replace nl with space
void t2_squeeze_space(int tbuf)
{
    int p=0;
    int ch=0;
    int ch1=0;

    for(p=0; p < m_len(tbuf); p++ ) {
	ch1=ch;
	ch = CHAR(tbuf,p);
	if( ch == 9 ) ch = CHAR(tbuf,p) = 32;
	if( ch == 32 && ch1 == 32 ) {
	    m_del(tbuf,p--);
	}
	else if( ch==10 && ch1 == 32 ) {
	    p--;
	    m_del(tbuf,p);
	}
    }

    
    ch=CHAR(tbuf,0);
    for(p=1; p < m_len(tbuf); p++ ) {
	ch1=ch;
	ch = CHAR(tbuf,p);
	if( ch == 10 && ch1 == 10 ) {
	    TRACE(2,"hard break: %d",p);
	    m_del(tbuf,p--);
	    ch = CHAR(tbuf,p)=13; // hard break
	}
    }
    
    for(p=0; p < m_len(tbuf); p++ ) {
	ch1=ch;
	ch = CHAR(tbuf,p);
	if( ch == 10 ) ch = CHAR(tbuf,p) = 32;
    }    
}


void t2_format_paragraph(int tbuf)
{
    char *delim = ":;.?!,";
    int p=0;
    int ch=0;
    int ch1=0;
    while( p < m_len(tbuf) )
	{
	    ch1=ch;
	    ch = CHAR(tbuf,p);
	    if( strchr( delim, ch1) && ch != 32 && ch != '\r' ) {
		m_ins(tbuf,p,1);
		ch=CHAR(tbuf,p)=32;
	    }
	    p++;
	}
}


int m_strdup(const char *s)
{
    int len = strlen(s)+1;
    int tbuf = m_create(len,1);
    m_write( tbuf, 0, s, len);
    return tbuf;
}

int t2_wrap_string( int lines, const char *s, int width )
{
    int tbuf = m_strdup(s);
    t2_squeeze_space(tbuf);
    t2_format_paragraph(tbuf);
    if(!lines) lines=m_create(10,sizeof(int));
    t2_wrap(tbuf,width,lines);
    m_free(tbuf);
    return lines;
}
