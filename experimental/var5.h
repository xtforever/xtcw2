#ifndef VAR5_H
#define VAR5_H

int mvar_init();
void mvar_destruct(void);


void var_set_callback( int q, varsig_t fn, void *d, int remove );
void var_call_callbacks( int q );

void mvar_free(int p);
int mvar_put_string( int id, char *s, int p );
char* mvar_get_string( int id, int p );
int mvar_put_integer( int id, long s, int p );
long mvar_get_integer( int id, int p );
int mvar_type(int id);
int mvar_group(int id);
char* mvar_name(int id);
int mvar_length(int id);
int mvar_path(int id, int mp);
int mvar_parse_path(int mp, int *group);
int mvar_lookup( int group, char *name, char *typename );
int mvar_vset(void);
int mvar_lookup_path( int mp, char *t );


int  mvar_registry( void *funcs, char *name, int id );
int  mvar_registry_new( void *funcs, char *name );


#endif
