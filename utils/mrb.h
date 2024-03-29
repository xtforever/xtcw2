#ifndef MRB
#define MRB

#ifdef USE_CONFIG_H
#include "config.h"
#endif

#ifndef MRB_BUFSIZE
#define MRB_BUFSIZE 1024
#endif

#include <stdio.h>
#include <unistd.h>
#include "mls.h"

struct mrb {

    int fd;
    int rd,wr;
    int size;
    int read_error;

    unsigned char buf[0];
};

int mrb_bytesused(struct mrb *b);
int mrb_peek(struct mrb *b, int *n);
int mrb_get(struct mrb *b);
int mrb_bufsize(struct mrb *b);
char *mrb_read_chunk(struct mrb *b, int *n);
struct mrb* mrb_create(int size);
void *mrb_maxsize(struct mrb *b, int *bytes );
void *mrb_alloc(struct mrb *b, int bytes );
void mrb_free( struct mrb *b, int bytes );
void mrb_destroy(struct mrb *m);
int mrb_sock_read(struct mrb *q, int fd );
int mrb_get_line(struct mrb *q, int line );
int mrb_last(struct mrb *q);

int mrb_error(struct mrb *q);
void mrb_error_clear(struct mrb *q);

int mro_init(int length, int size );
void mro_free(int num);
void mro_destruct(void);
void *mro_alloc(int num); /* nächste freie object allocieren, returns ptr oder NULL */
void *mro_dequeue(int num); /* element freigeben */
void* mro_peek(int num);  /* ptr to next alloced element or NULL */
    
#endif
