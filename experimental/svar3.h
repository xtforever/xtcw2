#ifndef SVAR_H
#define SVAR_H
#include <time.h>
#include <stdint.h>
#include <stdarg.h>
#include "svar-util.h"
#include "mls.h"

/*
STORAGE:
int SVAR - all variables 
int HASH - hash-table (list of list of SVAR-indices)


svar_lookup( "myVar", SVAR_FLOAT )
svar_get_float(k, 0);
svar_get_int(k, 0);
svar_get_svar(k, 0);

svar_get_key(k,0);
svar_get_val(k,0);
svar_put_key(k,0, m );
svar_put_val(k,0, m );

svar_put_float(k, 0,    3.1);
svar_put_int(k, 0,  svarid );
svar_put_svar(k, 0,  num   );

svar_get_keys(k);
svar_get_vals(k);
svar_write_callbacks( int q );
*/




svar_t *svar(int k);
void  svar_create(void);
void  svar_destruct(void);
int   svar_lookup(int buf, int type );
int   svar_lookup_str(char *s, int type );
int   svar_glookup(int group, int buf, int type);
int   svar_glookup_str(int group, char *s, int type);
int   svar_gcreate(void);
// int   svar_free( int k );

typedef struct svar_class *svarc_t;
struct svar_class {
    void *class_data;
    int type;
    void (*new)(svarc_t *c, svar_t *v);
    void (*delete)(svarc_t *c, svar_t *v);
};



#endif
