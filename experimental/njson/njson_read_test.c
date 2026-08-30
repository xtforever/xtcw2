#include "njson_read.h"
#include "mls.h"

void d_obj2(FILE *fp, int opts, char *c1, char *c2);
void d_obj(int opts, char *c1, char *c2);
void dump_element(struct njson_st *obj)
{
  if( obj->name )
	printf("\"%s\":",CHARP(obj->name));
  
  switch( obj->typ ) {
  case NJSON_OBJ:
    d_obj(obj->d," { "," } " );
    break;
  case NJSON_ARR:
    d_obj(obj->d," [ "," ] " );
    break;
  case NJSON_NUM:
    printf("%s", CHARP(obj->d));
    break;
  case NJSON_STRING:
    printf("\"%s\"", CHARP(obj->d));
    break;
  case NJSON_BOOL:
    printf("%s", CHARP(obj->d));
    break;
  case NJSON_NULL:
    printf("null");
    break;
  }    
}
void dump_element2(FILE *fp, struct njson_st *obj)
{
  if( obj->name )
      fprintf(fp,"\"%s\":",CHARP(obj->name));
  
  switch( obj->typ ) {
  case NJSON_OBJ:
      d_obj2(fp,obj->d," { "," } " );
      break;
  case NJSON_ARR:
      d_obj2(fp,obj->d," [ "," ] " );
      break;
  case NJSON_NUM:
      fprintf(fp,"%s", CHARP(obj->d));
      break;
  case NJSON_STRING:
      fprintf(fp,"\"%s\"", CHARP(obj->d));
      break;
  case NJSON_BOOL:
      fprintf(fp,"%s", CHARP(obj->d));
      break;
  case NJSON_NULL:
      fprintf(fp,"null");
      break;
  }    
}

void d_obj_list(int opts)
{
  int p;
  struct njson_st *d;
  m_foreach(opts,p,d) {
    dump_element(d);
    if( p < m_len(opts)-1 ) putchar(',');
  }
}

void d_obj_list2( FILE *fp, int opts)
{
  int p;
  struct njson_st *d;
  m_foreach(opts,p,d) {
      dump_element2(fp,d);
      if( p < m_len(opts)-1 ) fputc(',', fp);
  }
}

void d_obj(int opts, char *c1, char *c2)
{
  printf("%s",c1);
  d_obj_list(opts);
  printf("%s",c2);
}

void d_obj2(FILE *fp, int opts, char *c1, char *c2 )
{
    fprintf(fp,"%s",c1);
    d_obj_list2(fp, opts);
    fprintf(fp, "%s",c2);
}


int njson_cmp(char *name, struct njson_st *j)
{
  if( j->name ) return mstrcmp(j->name,0,name);
  return 1;
}

struct njson_st *njson_find_data(int opts, char *name)
{
  int p;
  struct njson_st *j;
  m_foreach(opts,p,j) {
    if( !njson_cmp(name,j) ) return j;
    if( j->typ >= NJSON_ARR ) {
      j=njson_find_data(j->d,name);
      if( j ) return j;
    }
  }
  return NULL;
}

char *njson_value( struct njson_st *j, int p )
{
  if( (!j) || (j->typ  == NJSON_OBJ) || (!j->d)) { return ""; }
  if(j->typ  == NJSON_ARR) {
    if( m_len(j->d) > p ) return njson_value( mls(j->d,p), 0 );
    return "";
  }  
  return CHARP(j->d);
}

char *njson_find(int opts, char *name, int p)
{
  struct njson_st *j = njson_find_data(opts, name);  
  return njson_value(j,p);
}


void test_read_write( char *inf, char *outf)
{
   FILE *fp;
   ASSERT( (fp=fopen( inf, "r" )));
   int opts = njson_from_file( fp );
   fclose(fp);
   ASSERT( (fp=fopen( outf, "w" )));
   d_obj2(fp, opts, "{\n", "\n}\n" );
   fclose(fp);
   njson_free(opts);
}


int main(int argc, char **argv)
{
  trace_level=1;
  m_init();
  TRACE(1,"start");
  test_read_write( "ex1.json", "ex1-o.json");
  m_destruct();
  return EXIT_SUCCESS;
}
