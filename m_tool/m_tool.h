#ifndef M_TOOL_H
#define M_TOOL_H

#include "mls.h"
#include <stdarg.h>

/* experimental tools */

/* m string (ms) : m-array of (char*) */
int m_str_va_app(int ms, va_list ap);
int m_str_app(int ms, ...);
int m_str_split(int ms, char *s, char* delim, int trimws );


int m_strncpy(int dst, int src, int max);
int m_mcopy(int dest, int destp, int src, int srcp, int src_count  );
int m_binsert( int buf, const void *data, int (*cmpf) (const void *a,const void *b ), int with_duplicates );
int compare_int(const void *a,const void *b);

void m_free_ptr(void *d);
void m_free_user(int m, void (*free_h)(void*), int only_clear );
void m_clear_user( int m, void (*free_h)(void*) );
void m_free_list(int m);
void m_clear_list(int m);
void m_clear_stringlist(int m);
void m_free_stringlist(int m);

void m_concat(int a, int b);
int m_split_list( const char *s, const char *delm );

int leftstr(int buf, int p, const char *s, int ch);

int cmp_int( const void *a0, const void *b0 );
int lookup_int(int m, int key);
int m_slice(int dest, int offs, int m, int a, int b );
int s_slice(int dest, int offs, int m, int a, int b );
int s_replace(int dest, int src, int pattern, int replace, int count );
void s_puts(int m);
int s_strstr(int m, int offs, int pattern );
int s_strncmp(int m,int offs,int pattern, int n);
void s_write(int m,int n);
int s_isempty(int m);
int s_strdup_c(const char *s);
int s_trim(int m);
int s_lower(int m);
int s_upper(int m);
int s_msplit(int dest, int src, int pattern );

void m_map( int m, int (*fn) ( int m, int p, void *ctx ), void *ctx  );
#endif
