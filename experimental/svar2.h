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


  int     s_create( void );
  void    s_free( int vs );
  // using variable name
  int     s_set( int vs, const char* name, const char* value, int pos );
  void    s_vaset( int vs, ... );
  void    s_clear( int vs, const char* name );
  char*   s_get( int vs, const char* name, int pos );
  int     s_len( int vs, const char* name );
  void    s_remove( int vs,const  char* name );
  int     s_lookup( int vs, int buf );
  int     s_lookup_str( int vs, char *s );
  int     s_find_key(int vs, const char *name);

// using variable index
  void    s_kset( int key, int buf, int pos );
  void    s_kclr( int key );
  int     s_kget( int key, int pos );
  int     s_klen( int key );
  char    s_ktype( int key );
  void    s_konwrite( int key, void (*fn) (void*, int), void *d, int remove );

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
