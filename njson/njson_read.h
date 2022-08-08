#ifndef NJSON_READ_H
#define NJSON_READ_H

#include <stdio.h>

/*interface*/
int  njson_from_file( FILE *fp );
int  njson_from_string( char *s );
void njson_free(int opts );

/* internals*/
int   njson_parse(void);
void  njson_stack( char *arg );
void  njson_new(char *value, int t);
void  njson_close(void);
void  njson_name(char *name);

enum njson_typ {
	       NJSON_STRING,
	       NJSON_NUM,
	       NJSON_BOOL,
	       NJSON_NULL,
	       NJSON_ARR,
	       NJSON_OBJ
};

struct njson_st {
    int name;
    int typ;
    int d;
};

#endif

