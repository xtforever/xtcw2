CC ?= gcc
CFLAGS ?= -I../utils -I../include -I. -DMLS_DEBUG
CFLAGS += -MMD -MP
LIBS = $(shell pkg-config --libs cairo) -lm

SRCS = scaled.c glue.c node.c token.c builder.c linebreak.c retex.c renderer.c math_node.c math_layout.c backend_cairo.c backend_xpixmap.c svg_factory.c shaping.c hyphen.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

LIB = libretex.a

all: $(LIB)

# Header changes must rebuild all objects; the library is small and the
# extra rebuild is cheaper than an ABI mismatch.
$(OBJS): $(wildcard *.h)

$(LIB): $(OBJS)
	ar rcs $@ $^

install: $(LIB)
	mkdir -p $(incdir) $(libdir)
	cp -p *.h $(incdir)/
	cp -p $(LIB) $(libdir)/

-include $(DEPS)

clean:
	rm -f *.o *.a *.d
