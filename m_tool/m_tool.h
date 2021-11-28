#ifndef M_TOOL_H
#define M_TOOL_H

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

#endif
