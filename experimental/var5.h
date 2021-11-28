#ifndef VAR5_H
#define VAR5_H
#include <stdint.h>

#define MAX_VAR 32
#define MAX_VARNAME (MAX_VAR-4)

#define VAR_INTEGER 0
#define VAR_STRING 1
#define VAR_VSET 2
#define VAR_REC 3

#define LIST_APPEND (-1)

#define VAR_S_INTEGER "INTEGER"
#define VAR_S_STRING  "STRING"
#define VAR_S_VSET    "VSET"
#define VAR_S_REC     "REC"


typedef void (*varsig_t) (void*ctx,int var,int sig);

void mvar_init(void);
void mvar_destruct(void);

void var_set_callback( int q, varsig_t fn, void *ctx, int remove );
void var_call_callbacks( int q, int s );

void mvar_free(int p);
int mvar_put_string( int id, char *s, int p );
char* mvar_get_string( int id, int p );
int mvar_put_integer( int id, long s, int p );
long mvar_get_integer( int id, int p );
int mvar_type(int id);
char* mvar_name_of_type( int id );
int mvar_group(int id);
char* mvar_name(int id);
int mvar_length(int id);
int mvar_path(int id, int mp);
int mvar_parse_path(int mp, int *group);
int mvar_parse( int mp, int typ );
int mvar_lookup( int group, char *name, int typename );
int mvar_vset(void);
int mvar_lookup_path( int mp, int t );
int mvar_anon( int g, int t );
int mvar_create( int type_id );

int  mvar_registry( void *funcs, char *name, int id );
int  mvar_registry_new( void *funcs, char *name );
char* mvar_name_of_type( int id );

struct var_name {
    uint32_t group;
    char name[MAX_VARNAME+1]; /* one padding zero */
};

typedef
struct var_st {
    struct var_name;
    uint8_t var_if;
} var_t;

var_t **mvar_get(int p );

#endif
