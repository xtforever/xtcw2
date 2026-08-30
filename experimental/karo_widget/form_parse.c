#include "mls.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "form_parse.h"




/* 0-9A-Za-z isalnum */
static int skipalnum(int ln, int *p)
{
  int ch;
  int start=1;
  for(; *p < m_len(ln); (*p)++ ) {
    ch=CHAR(ln,*p);
    if( ch == 'X' ) {
      if( start ) return ch;
      continue;
    }
    start = ! isalnum(ch);
  }
  return 0;
}

static int above_label(int scr,int line_num, int pos)
{
  int label;
  if( line_num == 0 ) return 0;
  int ln = INT(scr,line_num-1);
  if( pos >= m_len(ln) ) return 0;
  if( CHAR(ln,0) != '#' ) return 0;
  int ch = CHAR(ln,pos);
  if( !isalnum(ch) ) return 0;
  label=m_create(10,1);
  do {
    m_putc(label,ch);
    ++pos;
    if( pos >= m_len(ln) ) break;
    ch=CHAR(ln,pos);
  } while( isalnum(ch) );
  m_putc(label,0);
  return label;
}

static int skipx(int ln,int *pos)
{
  int len=0;
  while( *pos < m_len(ln) && CHAR(ln,*pos)=='X' ) { len++; (*pos)++; }
  return len;
}



int form_scan(int scr)
{
  struct form_entry fe;

  int ln;
  int line_num = 0;
  int liney = -1;
  int form = m_create(10,sizeof(struct form_entry));
  
  
  while( line_num < m_len(scr) ) {
    ln = INT(scr,line_num);
    if( CHAR(ln,0) != '#' ) {
      liney++;
      for(int i=0; i<m_len(ln); i++) {
	int ch = skipalnum(ln,&i);
	if( ch == 'X' ) {
	  fe.label=above_label(scr,line_num,i);
	  if( fe.label ) {
	    fe.x = i; fe.y = liney;
	    fe.len = skipx(ln,&i);
	    fe.typ=0;
	    m_put( form, &fe );
	    TRACE(1,"%d %d %s", fe.x, fe.y, m_buf(fe.label));
	  }
	}
      }
    }
    line_num++;
  }
  return form;
}

void form_dump(int f)
{
  struct form_entry *fe;
  int n;
  m_foreach(f,n,fe) {
    printf("%s %d %d %d\n",
	   (char*)m_buf(fe->label),
	   fe->x,fe->y,fe->len );
  }
  
}
void form_free(int f)
{
  struct form_entry *fe;
  int n;
  m_foreach(f,n,fe) { m_free(fe->label); }
  m_free(f);
    
}

static int file_to_marr(FILE *fp)
{
    int scr=m_create(50,sizeof(int));
    while(1) {
	int ln = m_create(50,1);
	int ret = m_fscan(ln,'\n',fp);
	if( ret != '\n' ) break;
	m_put(scr,&ln);
    }
    return scr;
}

void scr_to_display(int scr)
{
    /* kill comment lines, kill first character each line */
    int p=0, ln;
    while( p < m_len(scr) ) {    
	ln = INT(scr,p);
	if( CHAR(ln,0) == '#' ) {
	    m_free(ln);		/* free line buffer */ 
	    m_del(scr,p);	/* remove line frm line - list */
	    continue;
	}
	m_del(ln,0); /* remove first character in line */ 
	p++;
    }
}


int form_from_file(char *name, int *pscr, int *pfrm)
{
    /* read screen to buffer */  
    FILE *fp = fopen( name, "r");
    ASERR(fp, "file not found: %s", name );
    int scr   = file_to_marr(fp);
    fclose(fp);
  
    int forms = form_scan( scr );

    scr_to_display(scr);
    /* we removed the first character from each line, now we have to 
       adjust the field positions */
    int n; struct form_entry *fe;
    m_foreach(forms,n,fe) { fe->x --; }
    
    *pscr = scr;
    *pfrm = forms;
    return 0;
}


