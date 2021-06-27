#ifndef SVAR_H
#define SVAR_H
#include <time.h>
#include <stdint.h>
#include <stdarg.h>


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


int append_mstring_array(int v,int offset, ... );
int vas_append_mstring_array(int m,int offset, va_list ap);
int s_clr( int str );
int s_memcpy(int dst, int src, int max);
int m_copy(int dest, int src);
void m_free_user(int m, void (*free_h)(void*));
void m_free_list_of_list(int m);
void m_clear_mstring_array(int m);
int m_mcopy(int dest, int destp, int src, int srcp, int src_count  ); /* todo: extensive testing needed */


/* some functions to access data stored in typed svar */
const char* svar_get_str(int svar_key, int p);
int svar_get_str_count(int svar_key);
int svar_strdup(char *s);



void  svar_create(void);
static inline const void svar_init(void) { svar_create(); }
void  svar_destruct(void);
int   svar_lookup(int buf, int type );
int   svar_lookup_str(char *s, int type );
void  svar_free(int key);
void  svar_onwrite( int key, void (*fn) (void*, int), void *d, int remove );
void  svar_write_callbacks( int q );
int*  svar_value( int q );
uint8_t* svar_type(int v);
const char* svar_name(int v);
const char *svar_typename(int v);


#define SVAR_ARRAY   8
#define SVAR_INT     0
#define SVAR_FLOAT   1
#define SVAR_STRING  2
#define SVAR_MSTRING 4
#define SVAR_SVAR    5
#define SVAR_MARRAY  6
#define SVAR_MASK    0x0f
#define SVAR_INT_ARRAY     (SVAR_INT | SVAR_ARRAY)
#define SVAR_FLOAT_ARRAY   (SVAR_FLOAT | SVAR_ARRAY)
#define SVAR_MSTRING_ARRAY (SVAR_MSTRING | SVAR_ARRAY)
#define SVAR_SVAR_ARRAY    (SVAR_SVAR | SVAR_ARRAY)
#define SVAR_MARRAY_ARRAY  (SVAR_MARRAY | SVAR_ARRAY)
#define SVAR_STRING_ARRAY  (SVAR_STRING | SVAR_ARRAY)
 
#define svar_is_array(t) (t&SVAR_ARRAY)
#define svar_typeof(t,typ) ((t&SVAR_MASK)==typ)
#define svar_define(t,typ) t = ((t&~SVAR_MASK) | typ)

// must be multiple of 8
#define SVAR_LEN_MULTIPLIER 2
// size of hash-table
#define SVAR_TABLE_BITS 12

#define SVAR_MAX2 (8 << SVAR_LEN_MULTIPLIER)
#define SVAR_MAX (SVAR_MAX2 - sizeof(int))
#define SVAR_TABLE_SIZE ((1 << SVAR_TABLE_BITS)-1)

struct svar_signal {
    void *d;
    void (*fn) (void*, int );
};

typedef struct svar_name_st {
    int parent;
    char str[SVAR_MAX];
} svar_name_t;
    

typedef struct svar_st {
    svar_name_t name;		/* hash friendly name */
    int key,value;		/* same type */
    uint8_t type, locked;
    int write_callbacks;
    int read_callbacks;
} svar_t;

svar_t *svar(int k);

static inline const char *m_str(const int m) { return m_buf(m); }
/**
 * @fn timespec_diff(struct timespec *, struct timespec *, struct timespec *)
 * @brief Compute the diff of two timespecs, that is a - b = result.
 * @param a the minuend
 * @param b the subtrahend
 * @param result a - b
 */
static inline void timespec_diff(struct timespec *a, struct timespec *b,
    struct timespec *result) {
    result->tv_sec  = a->tv_sec  - b->tv_sec;
    result->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (result->tv_nsec < 0) {
        --result->tv_sec;
        result->tv_nsec += 1000000000L;
    }
}

void statistics_svar_allocated(int *a, int *mem, int *free);




// expand strings with embedded variables
typedef struct svexp_exp_st {
  int splitbuf;
  int max_row;
  int values; // [char*]
  int indices; // [int]
  int buf;
} svexp_t;

void    svexp_init(  svexp_t *se  );
void    svexp_free(  svexp_t *se );
void    svexp_realloc_buffers( svexp_t *se );
void    svexp_parse(svexp_t *se, const char *frm);
const char*   svexp_expand( svexp_t *se, char *p, int row );
const char*   svexp_string( char *p, const char *frm );

#endif
