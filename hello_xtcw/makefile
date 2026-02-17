TARGET=hello

prefix      ?= ../build
incdir      ?= $(prefix)/include
libdir      ?= $(prefix)/lib

CC = gcc
CFLAGS += -D_GNU_SOURCE -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2
XORGLIB = -lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm -lcairo
LOADLIBES = $(libdir)/libxtcw.a $(XORGLIB)

ALL: $(TARGET)

$(TARGET): hello.o

clean:
	rm -f $(TARGET) *.o
