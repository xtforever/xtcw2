#ifndef M_TOOL_H
#define M_TOOL_H

#include <stdarg.h>

/* experimental tools */

/* m string list (ms) : m-array of (char*) */
int ms_va_app(int m, va_list ap);
int ms_app(int m, ...);
int ms_split(int m, char *s, char* delim, int trimws );

#endif
