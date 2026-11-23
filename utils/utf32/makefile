# libutf32: UTF-32 code point array conversions and helpers.
# Depends on libutf8 for the single code point codec.

CC ?= gcc
CFLAGS ?= -Wall
CFLAGS += -I. -I../utf8

SRCS = utf32.c
OBJS = $(SRCS:.c=.o)

LIB = libutf32.a

all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

install: $(LIB)
	mkdir -p $(incdir) $(libdir)
	cp -p *.h $(incdir)/
	cp -p $(LIB) $(libdir)/

clean:
	rm -f *.o *.a
