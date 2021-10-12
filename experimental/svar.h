#ifndef SVAR_H
#define SVAR_H
#include <time.h>
#include <stdint.h>

// must be multiple of 8
#define SVAR_MAX 32
// size of hash-table
#define SVAR_TABLE_BITS 12

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


void svar_init(void);
void svar_free(void);
void svar_onwrite( int q, void (*fn) (void*, int), void *d, int remove );
svar_t *svar(int q);
int svar_key(int buf);
const char *svar_name( int v );
void svar_write(int q, int d);
int* svar_value(int q);
uint8_t* svar_type(int q);
int svar_key_cstr( const char *s );


int string_to_hash_fast(int sbuf);
void svar_get_internals( int *svar, int *svar_dat );

#endif
