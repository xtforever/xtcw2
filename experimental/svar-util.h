#ifndef SVAR_UTIL_H
#define SVAR_UTIL_H

#include <stdarg.h>
#include <stdint.h>
#include "mls.h" 

// must be multiple of 8
#define SVAR_LEN_MULTIPLIER 3
// size of hash-table
#define SVAR_TABLE_BITS 12
#define SVAR_MAX2 (SVAR_LEN_MULTIPLIER * 8)
#define SVAR_MAX  (SVAR_MAX2 - sizeof(int))
#define SVAR_TABLE_SIZE ((1 << SVAR_TABLE_BITS)-1)


#define SVAR_INT     0
#define SVAR_FLOAT   1
#define SVAR_STRING  2
#define SVAR_MSTRING 4
#define SVAR_SVAR    5
#define SVAR_MARRAY  6
#define SVAR_ARRAY   8
#define SVAR_ARRAY_CSTRING   9

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
    intptr_t key,value;		/* same type */
    uint8_t type, locked;
    int write_callbacks;
    int read_callbacks;
} svar_t;


// expand strings with embedded variables
typedef struct svexp_exp_st {
  int splitbuf;
  int max_row;
  int values; // [char*]
  int indices; // [int]
  int buf;
} svexp_t;

int m_copy(int dest, int destp, int src, int srcp, int src_count  );

int s_strncpy(int dst, int src, int max);
int s_strdup(int r, const char *str);
int svar_hash_lookup(int hash, void *buf,
		       int (*cmpf)(void *svar,void *b),
		       int (*newf)(void *key)
		     );

int seperate_by_dot(char *dst, int src, int *start, int max);
int compare_int(const void *a,const void *b);

int svar_find_signal_handler( int m, void (*fn) (void*, int) );
void svar_onwrite( int q, void (*fn) (void*, int), void *d, int remove );
void svar_write_callbacks( int q );

int m_binsert( int buf, const void *data, int (*cmpf) (const void *a,const void *b ), int with_duplicates );
void mlist_free(void *d);
void m_free_user(int m, void (*free_h)(void*));
void m_free_list_of_list(int m);
void m_clear_mstring_array(int m);


int app_cstr_mstrl( int m, const char *name );
int vas_mstring_list_create(int m, va_list ap );
int vas_mstring_list_append(int m,int offset, va_list ap);
int mstring_list_append(int m,int offset, ... );

int field_escape(int s2, const char *s, int quotes);
const char* svexp_expand( svexp_t *se, char *prefix, int row );
const char*   svexp_string( char *prefix, const char *frm );
void svexp_parse(svexp_t *se, const char *frm);
void svexp_realloc_buffers( svexp_t *se );
void svexp_free(  svexp_t *se );
void svexp_init(  svexp_t *se  );

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



#endif
