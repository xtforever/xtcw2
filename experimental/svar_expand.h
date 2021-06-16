#ifndef SVAR_EXPAND_H
#define SVAR_EXPAND_H

#include <stdarg.h>

// expand strings with embedded variables
typedef struct svexp_exp_st {
  int splitbuf;
  int max_row;
  int values; // [char*]
  int indices; // [int]
  int buf;
} svexp_t;

void    svexp_init(  svexp_t *se  );
void    svexp_free(  svexp_t *se );
void    svexp_realloc_buffers( svexp_t *se );
void    svexp_parse(svexp_t *se, const char *frm);
const char*   svexp_expand( svexp_t *se, char *p, int row );
const char*   svexp_string( char *p, const char *frm );


int svar_strdup(char *s);
int append_mstring_array(int v,int offset, ... );
int vas_append_mstring_array(int m,int offset, va_list ap);
    
#endif
