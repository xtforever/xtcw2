CC=gcc
CFLAGS+=-ggdb -D_GNU_SOURCE -DMLS_DEBUG -O0 -Wall

all: hello-world double-free

hello-world: mls.o

double-free: mls.o

clean:
	-${RM} *.o *~ hello-world
