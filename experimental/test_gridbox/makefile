TARGET=test_gridbox

prefix      ?= ../build
incdir      ?= $(prefix)/include
libdir      ?= $(prefix)/lib
plainc      ?= ../plainc_widgets

CC = gcc
CFLAGS += -g -O0 -I$(incdir) -I$(incdir)/xtcw -I$(plainc) -I../utils -I/usr/include/freetype2
XORGLIB = -lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm -lcairo
LOADLIBES = $(libdir)/libplainc.a $(libdir)/libutils.a $(XORGLIB)

ALL: $(TARGET)

$(TARGET): test_gridbox.o

clean:
	rm -f $(TARGET) *.o