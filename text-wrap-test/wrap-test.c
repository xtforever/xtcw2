#include "mls.h"
#include "text-wrap.h"

/*
  idee: zur formatierung wird nur \n und \r im text verwendet
  \r ist ein erzwungener zeilenwechsel
  \n wird wie ein leerzeichen behandelt
  

  zuerst werden doppelte leerzeichen entfernt und \n und \t in leerzeichen umgewandelt
  ( squeeze_space )
  
  danach werden leerzeichen nach Satzzeichen eingefuegt
  ( format_paragraph )

  der text ist jetzt fuer weitere format optionen vorbereitet.
  dieser text kann gepuffert werden und in verschiedenen varianten formatiert werden.
  


*/


char *text1 = "					\
Betrachtet,wie\n	\nin\n\nAbendsonne-Glut	\
						\
Die grünumgebenen Hütten schimmern!		\
Sie rückt und weicht,der Tag ist überlebt,	\
Dort eilt sie hin und fördert neues Leben.	\
O !daß kein Flügel mich vom Boden hebt,	\
Ihr nach und immer nach zu streben!		\
Ich säh’ im ew’gen Abendstrahl			\
Die stille Welt zu meinen Füßen,		\
Entzündet alle Höhn, beruhigt jedes Tal,	\
Den Silberbach in goldene Ströme fließen.	\
Nicht hemmte dann den göttergleichen Lauf	\
Der wilde Berg mit allen seinen Schluchten;	\
Schon tut das Meer sich mit erwärmten Buchten	\
Vor den erstaunten Augen auf.			\
Doch scheint die Göttin endlich wegzusinken;	\
Allein der neue Trieb erwacht,			\
Ich eile fort, ihr ew’ges Licht zu trinken,	\
Vor mir den Tag und hinter mir die Nacht,	\
Den Himmel über mir und unter mir die Wellen.	\
Ein schöner Traum,indessen sie entweicht!	\
Ach, zu des Geistes Flügeln wird so leicht	\
Kein körperlicher Flügel sich gesellen.		\
Doch ist es jedem eingeboren,			\
Daß sein Gefühl hinauf und vorwärts dringt,	\
Wenn über uns, im blauen Raum verloren,		\
Ihr schmetternd Lied die Lerche singt,		\
Wenn über schroffen Fichtenhöhen		\
Der Adler ausgebreitet schwebt\r\r		\
Und über Flächen, über Seen			\
Der Kranich nach der Heimat strebt.";


void squeeze_space(int tbuf)
{
    int p=0;
    int ch=0;
    int ch1=0;
    while( p < m_len(tbuf) )
	{
	    ch1=ch;
	    ch = CHAR(tbuf,p);
	    if( ch == '\t' || ch == 10 ) {
		ch = CHAR(tbuf,p) = 32;
	    }	    
	    if( ch == 32 && ch1 == 32 ) {
		    m_del(tbuf,p);
	    } else p++;		
	}
}

void format_paragraph(int tbuf)
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

void wordwrap(int tbuf, int width)
{
    int p=0;
    int ch=0;
    int last_space = 0;
    int cur_width = 0;
    while( p < m_len(tbuf) )
	{
	    ch = CHAR(tbuf,p);    
	    if( ch == 32 ) last_space=p;
	    
	    if( ch == 13 ) {  // forced newline 
		cur_width=0;
		last_space=0;
		p++;
		continue;
	    }

	    // break or forced break
	    if(cur_width >= width ) {
		if( last_space ) {		    
		    CHAR(tbuf,last_space)=10;
		    cur_width=p - last_space;
		}
		else {
		    m_ins(tbuf,p,1);
		    CHAR(tbuf,p)=10;
		    cur_width=0;				    
		}
		last_space=0;
	    } else cur_width++;
	    p++;
	}
}


int utf8_strlen(int ln)
{
    int len = 0;
    while( m_utf8char(ln,&len) != -1 );
    return len;
}


/*a    b  c
      
  12345 6789  0

          a
          
          
	  a-c


   copy_part( line_start, last_space ? last_space : scan_pos );


 */

void t2wordwrap2(int tbuf, int width, int lines)
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


void t2wordwrap(int tbuf, int width, int lines)
{
    int ln=0;
    int p=0;
    int ch=0;
    int last_space = 0;
    int cur_width = 0;
    int line_start = 0;
    while( p < m_len(tbuf) )
	{
	    if( ! ln ) ln=m_create(1,1);

	    ch = CHAR(tbuf,p);    
	    if( ch == 32 ) last_space=p;

	    if( ch == 13 ) {  // forced newline
		m_write(ln,0, mls(tbuf,line_start),  p - line_start ); m_putc( ln, 0);
		m_put(lines,&ln); ln=0;
		cur_width=0;
		last_space=0;
		p++;
		line_start=p;
		continue;
	    }

	    // break or forced break
	    if(cur_width < width )
		{
		    cur_width++;
		}
	    else
		{
		    if( last_space )
			{
			    m_write(ln, 0,  mls(tbuf,line_start), last_space - line_start ); m_putc( ln, 0);
			    m_put(lines,&ln); ln=0;
			    line_start = last_space+1;
			    cur_width=p - last_space;
			    last_space=0;
			}
		    else
			{		    
			    m_write(ln, 0,  mls(tbuf,line_start), p - line_start ); m_putc( ln, 0);
			    m_put(lines,&ln); ln=0; 
			    line_start = p;
			    cur_width=0;			    
			}
		}	    
	    p++;
	}
    if( ln ) m_free(ln);
}




void print_rep(int len, int ch)
{
    while(len--) putchar(ch);
    putchar(10);
}


int output_line(int tbuf, int p)
{
    int ch=0;
    int cur_width = 0;
    while( p < m_len(tbuf) )
	{
	    ch = CHAR(tbuf,p); 
	    if( ch == 10 || ch == 13 ) break;
	    putchar(ch);
	    cur_width++;
	    p++;
	}
    
    printf("\t%u\n", cur_width);
    return p;
}


void print_text(int tbuf, int w)
{
     print_rep(w,'-');
     int p=0;
     while( p < m_len(tbuf) ) {
	 p=output_line(tbuf,p);
	 putchar(10);
	 p++;
     }
     putchar(10);
}


int copy_line(int tbuf, int p, int ln)
{
    int ch=0;
    int cur_width = 0;
    m_clear(ln);
    while( p < m_len(tbuf) )
	{
	    ch = CHAR(tbuf,p); 
	    if( ch == 10 || ch == 13 ) break;
	    m_putc(ln,ch);
	    cur_width++;
	    p++;
	}

    return p;
}

void t2convert(int tbuf, int lines)
{
    int p=0;
    while( p < m_len(tbuf) ) {
	int ln = m_create(1,1);
	p=copy_line(tbuf,p, ln);
	m_put(lines,&ln);
	p++;
    }    
}  

void t2dump_line(int ln)
{
    if( ln && m_len(ln) )
	if( write( 1, m_buf(ln), m_len(ln) ) < 0 ) ERR("");
}

void t2dump(int lines)
{
    int p,*d;
    m_foreach(lines,p,d)
	{
	    t2dump_line(*d);
	    putchar(10);
	}
}


void m_free_mlist(void *ctx, void *d)
{
    m_free( *(int*) d );
}

void m_free_user(int m, void (*free_h)(void *,void*), void *ctx)
{
    void *d; int p;
    m_foreach(m,p,d) free_h(ctx,d);
}

void m_free_list_of_list(int m)
{
    m_free_user( m, m_free_mlist, 0 );
    m_free(m);
}

void m_clear_list_of_list(int m)
{
    m_free_user( m, m_free_mlist, 0 );
    m_clear(m);
}


/* return ptr to buffer inside list of buffers */
char *mgets( int m, int p)
{
    return  p < m_len(m) ? m_str( INT(m,p) ) : "";
}

// static int len(int m,int p) {
//    return  p < m_len(m) ? m_len( INT(m,p) ) : 0;
//}

void print_width( int lines1, int p, int w1 )
{
    int out = 0;
    
    if( p < m_len(lines1) ) {
	// printf("%3d",p);

	int ch = -1;
	int pos = 0;
	int ln = INT(lines1,p);
	while( out < w1 && (ch = m_utf8char(ln,&pos) > 0)) { 
	    out++;	    
	}
	if( ch == 0 ) { pos--; }
	if( pos ) {
	    fwrite( m_buf(ln), pos, 1, stdout );	
	}
    }
    
    while( out < w1 ) {
	putchar( 32 );
	out++;
    }
    
}

void print_side_by_side(int lines1,int w1,int lines2,int w2)
{
    int p;
    int max = Max( m_len(lines2), m_len(lines1));
    for( p=0; p<max;p++ ) {
	// char *s1 = mgets(lines1,p);
	// char *s2 = mgets(lines2,p);
	// printf( "%s", s1 ); putchar('\t');
	// printf( "%s", s2 ); putchar(10);

	
	print_width( lines1, p, w1 );
	printf(" | ");
	print_width( lines2, p, w2 );
	printf("\n");	
    }
    putchar(10);
}

void print_column(int lines1,int w1)
{
    int p;
    int max = m_len(lines1);

    for( p=0; p<max;p++ ) {
	printf( "%4d ", p);
	print_width( lines1, p, w1 );
	printf(" |\n");	
    }
    putchar(10);
}


void wrap_test(void)
{
    puts("ok");
    int tbuf = m_create(1000,1);
    m_write(tbuf,0,text1,strlen(text1)+1);
    puts(m_str(tbuf));
    squeeze_space(tbuf);
    format_paragraph(tbuf);
    puts(m_str(tbuf));
    wordwrap(tbuf,40);
    puts(m_str(tbuf));
    print_text( tbuf, 40 );
    int lines = m_create(50,sizeof(int));
    t2convert(tbuf,lines);
    t2dump(lines);


    puts("test2");
    
    int lines2 = m_create(50,sizeof(int));
    m_clear(tbuf);
    m_write(tbuf,0,text1,strlen(text1)+1);
    squeeze_space(tbuf);
    format_paragraph(tbuf);
    puts(m_str(tbuf));
    print_rep( 10, '.');
    t2wordwrap( tbuf, 10, lines2 );
    t2dump(lines2);

    int lines3 = m_create(50,sizeof(int));
    m_clear(tbuf);
    m_write(tbuf,0,text1,strlen(text1)+1);
    squeeze_space(tbuf);
    format_paragraph(tbuf);
    t2wordwrap2(tbuf, 11, lines3);

        
    print_side_by_side(lines2,10,lines3,10);    
    m_clear_list_of_list(lines2);
    m_clear_list_of_list(lines3);
    

    
    m_clear(tbuf);
    s_printf(tbuf,0, "1234 5678 9012");
    t2wordwrap2(tbuf,4, lines3);
    t2dump(lines3);
    
    m_clear_list_of_list(lines3);
    s_printf(tbuf,0, "1234 5678 9012");
    t2wordwrap2(tbuf,40, lines3);
    t2dump(lines3);
    


    
    m_free_list_of_list(lines);
    m_free_list_of_list(lines2);
    m_free_list_of_list(lines3);
	
    m_free(tbuf);
}

void wrap_test2(void)
{
    int lines = t2_wrap_string( 0, text1, 12 );
    print_column(lines,12);
    m_free_list_of_list(lines);
}

 int main(int argc, char **argv)
{
    m_init();
    trace_level=2;
    wrap_test2();
    m_destruct();
}
