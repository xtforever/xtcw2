#ifndef SVAR_STRING_H
#define SVAR_STRING_H

#include "svar3.h"

int svar_get_str_count(int var);
int svar_get_str(int var,int index);
void svar_copy_from_cstring(int var, const char *s);
#endif
