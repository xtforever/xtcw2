/*
  njson reader
  Jens Harms, 05.12.2021, <au1064> <at> <gmail.com>
*/

#include "mls.h"
#include "njson_read.h"
#include "njson_parse.tab.h"
#include "njson_lex.lex.h"
#include <limits.h>
#include <stdlib.h>
int yyparse(void);
void set_input_string(const char* in);
void end_lexical_scan(void);


static int stack2 = 0;
static int current;

inline static void* m_last(int d)
{
  return mls(d, m_len(d)-1);
}

inline static struct njson_st *add_element(int list, int typ)
{
  struct njson_st *e = m_add(list);
  *e = (struct njson_st){ .typ = typ }; 
  return e;
}

void njson_new(char *value, int typ)
{
    TRACE(3,"NEW %s", value );
    struct njson_st *njson;
    if(m_len(stack2)==0) {
      m_put(stack2,&current);
      return;
    }
    TRACE(3,"append to list: %d", current );
    njson = add_element(current,typ);
    if((njson->typ == NJSON_OBJ) || (njson->typ == NJSON_ARR)) {
	m_put(stack2, &current);
	current = njson->d = m_create(10,sizeof(struct njson_st));
	return;
    }
    /* number, boolean, null, string --  */
    /* It's all the same, only the names will change */
    /* Every day, it seems we're wastin' away */
    njson->d = s_printf(0,0,"%s", value );
}

void njson_close(void)
{
  if(!m_len(stack2)) {
      WARN("njson syntax error. closing parenthesis without opening");
      return;
  }
  current = *(int*)m_pop(stack2);
  TRACE(3,"current list: %d", current );
}

void njson_name(char*name)
{
  struct njson_st *j = m_last(current);
  j->name = s_printf(0,0,"%s", name );
}

int njson_from_file( FILE *fp )
{
    njsonin=fp;
    return njson_parse();
}

int njson_from_string(char *s)
{
  set_input_string(s);
  int ret = njson_parse();
  return ret;
}

int njson_parse(void)
{
    int root;
    stack2=m_create(50,sizeof(int));
    root = current = m_create(10,sizeof(struct njson_st));
    int ret = njsonparse(); /* sets global int root */
    m_free(stack2); stack2=0;
    if(ret) { njson_free(root); root=0; WARN("njson read error"); }
    end_lexical_scan();
    return root;
}

void njson_free(int opt)
{
  if(!opt) return;
  int p;  struct njson_st *j; 
  m_foreach( opt, p, j ) {
    m_free(j->name);
    if( (j->typ == NJSON_OBJ) || (j->typ == NJSON_ARR) )
      njson_free(j->d);
    else m_free(j->d);
  }
  m_free(opt);
}
