all: clean form_parse
VPATH=../utils
CC = gcc
CFLAGS += -D_GNU_SOURCE -DMLS_DEBUG
CFLAGS += -g -Wall
CFLAGS += -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I. -I../utils

form_parse: mls.o

clean:
	-rm -f *.o form_parse
