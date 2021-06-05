#ifndef SVAR_H
#define SVAR_H
#include <time.h>
#include <stdint.h>

/*
STORAGE:

int SVAR - all variables 
int HASH - all hash-tables (list of list of SVAR-indices)
int SETS - all variable sets (list of svar_set_t)


interface:
*/


void m_free_user(int m, void (*free_h)(void*));
void m_free_list_of_list(int m);
int s_clr( int str );
int m_copy(int dest, int src);
void statistics_svar_allocated(int *a, int *mem, int *free);


void svar_create(void);
void svar_destruct(void);
int svar_lookup(int buf);
void svar_free(int key);



// using variable index
  void    s_kset( int key, int buf, int pos );
  void    s_kclr( int key );
  int     s_kget( int key, int pos );
  int     s_klen( int key );
  char    s_ktype( int key );
  void    s_konwrite( int key, void (*fn) (void*, int), void *d, int remove );

void svar_write( int q, int data );
int  *svar_value( int q );
const char *svar_typename(int v);
uint8_t *svar_type(int v);
#define SVAR_ARRAY   8
#define SVAR_INT     0
#define SVAR_FLOAT   1
#define SVAR_MSTRING 4
#define SVAR_SVAR    5
#define SVAR_MARRAY  6
#define SVAR_MASK    0x0f

#define SVAR_INT_ARRAY     (0+8)
#define SVAR_FLOAT_ARRAY   (1+8)
#define SVAR_MSTRING_ARRAY (4+8)
#define SVAR_SVAR_ARRAY    (5+8)
#define SVAR_MARRAY_ARRAY  (6+8)

#define svar_is_array(t) (t&SVAR_ARRAY)



/*
 */



// must be multiple of 8
#define SVAR_LEN_MULTIPLIER 2
// size of hash-table
#define SVAR_TABLE_BITS 12

#define SVAR_MAX (8 << SVAR_LEN_MULTIPLIER)
#define SVAR_TABLE_SIZE ((1 << SVAR_TABLE_BITS)-1)

struct svar_signal {
    void *d;
    void (*fn) (void*, int );
};

typedef struct svar_st {
    int name;
    int value;
    uint8_t type, locked;
    int write_callbacks;
    int read_callbacks;
} svar_t;

typedef struct svar_set_st {
    int hash;
    int svar_key;
} svar_set_t;



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
int s_strcpy(int dst, int src, int max);
static inline const char *m_str(const int m) { return m_buf(m); }


#endif
