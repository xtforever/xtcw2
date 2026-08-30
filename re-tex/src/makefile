CC ?= gcc
CFLAGS ?= -I../utils -I../include -I. -DMLS_DEBUG
LIBS = $(shell pkg-config --libs cairo) -lm

SRCS = scaled.c glue.c node.c token.c builder.c linebreak.c retex.c renderer.c math_node.c math_layout.c backend_cairo.c backend_xpixmap.c
OBJS = $(SRCS:.c=.o)

LIB = libretex.a

all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

install: $(LIB)
	mkdir -p $(incdir) $(libdir)
	cp *.h $(incdir)/
	cp $(LIB) $(libdir)/

clean:
	rm -f *.o *.a
