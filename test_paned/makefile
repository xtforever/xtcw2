TARGET=test_paned

prefix      ?= ../build
incdir      ?= $(prefix)/include
libdir      ?= $(prefix)/lib

CC = gcc
CFLAGS += -g -O0 -DMLS_DEBUG -D_GNU_SOURCE -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2
XORGLIB = -lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm -lcairo
LOADLIBES = $(libdir)/libxtcw.a $(XORGLIB)

ALL: $(TARGET)

$(TARGET): test_paned.o

clean:
	rm -f $(TARGET) *.o
