#ifndef CONSTSTR_H
#define CONSTSTR_H

/*
  s_cstr - returns a handle to a constant string equal to the given c-style string 
  s_mstr - returns a handle to a constant string equal to the given array 
  cs_printf - creates a string and returns a handle to a constant string equal to the created one
*/

/* how man strings before forced exit(122) to prevent OOM Killer */ 
#define CONSTSTR_MAX 9000





void conststr_stats(void);
void conststr_init(void);
void conststr_free(void);
int conststr_lookup_c(const char *s);
int conststr_lookup(int s);
int cs_printf( const char *format, ... );

inline static int s_cstr(const char *s) { return conststr_lookup_c(s); }
inline static int s_mstr(int m) { return conststr_lookup(m); }

#endif // CONSTSTR_H
