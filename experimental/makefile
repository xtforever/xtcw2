# Legacy experiments makefile.
# The proven build chain lives in the top-level makefile and commander/makefile.
# Everything in this directory is kept for reference only.

VPATH=../utils
prefix      ?= ../build
libdir      ?= $(prefix)/lib
incdir      ?= $(prefix)/include

CC=gcc -I. -I../utils -ggdb -O0 -DMLS_DEBUG -Wall
CFLAGS += -D_GNU_SOURCE -fms-extensions
CFLAGS += -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -I. -I../utils

LOADLIBES=$(libdir)/libxtcw.a $(libdir)/libwcl.a $(libdir)/libutils.a $(libdir)/libretex.a $(libdir)/libplainc.a -lm
XORGLIB=-lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm -lutil -ldl -lcairo

T=var5-test hash_test thread_pipe_test svar2-test svar-util-test

ALL: ${T}

var5-test: mls.o var5.o m_tool.o
	$(CC) $(CFLAGS) -o $@ $^ $(LOADLIBES) $(XORGLIB) -lpthread

hash_test: mls.o hash_test.o m_tool.o
	$(CC) $(CFLAGS) -o $@ $^ $(LOADLIBES) $(XORGLIB) -lpthread

thread_pipe_test: thread_pipe_test.o
	$(CC) $(CFLAGS) -o $@ $^ $(LOADLIBES) $(XORGLIB) -lpthread

svar2-test: mls.o svar2.o
	$(CC) $(CFLAGS) -o $@ $^ $(LOADLIBES) $(XORGLIB)

svar-util-test: svar-util.o svar3.o mls.o
	$(CC) $(CFLAGS) -o $@ $^ $(LOADLIBES) $(XORGLIB)

clean:
	-${RM} *.o ${T}
