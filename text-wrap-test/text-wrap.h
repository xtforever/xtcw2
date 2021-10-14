#ifndef TEXT_WRAP_H
#define TEXT_WRAP_H



#include "mls.h"



void t2_wrap(int tbuf, int width, int lines);


void t2_squeeze_space(int tbuf);


void t2_format_paragraph(int tbuf);


int m_strdup(const char *s);

int t2_wrap_string( int lines, const char *s, int width );


#endif
