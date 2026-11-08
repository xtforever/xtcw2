# libutf8: stateless UTF-8 <-> Unicode code point primitives.
# Standalone, no dependency on mls or X11.

CC ?= gcc
CFLAGS ?= -Wall
CFLAGS += -I.

SRCS = utf8.c
OBJS = $(SRCS:.c=.o)

LIB = libutf8.a

all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

install: $(LIB)
	mkdir -p $(incdir) $(libdir)
	cp *.h $(incdir)/
	cp $(LIB) $(libdir)/

clean:
	rm -f *.o *.a
