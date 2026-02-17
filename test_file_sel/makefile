TARGET=main

prefix      ?= ../build
incdir      ?= $(prefix)/include
libdir      ?= $(prefix)/lib

CC = gcc
CFLAGS += -D_GNU_SOURCE -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -DMLS_DEBUG
XORGLIB = -lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm
LOADLIBES = $(libdir)/libxtcw.a $(XORGLIB)

ALL: $(TARGET)

$(TARGET): main.o

clean:
	rm -f $(TARGET) *.o
