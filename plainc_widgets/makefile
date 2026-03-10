CC ?= gcc
CFLAGS += -I. -I../utils -I../wcl -I$(incdir)

SRCS = Gridbox.c
OBJS = $(SRCS:.c=.o)

LIB = libplainc.a

all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

install: $(LIB)
	mkdir -p $(incdir)/xtcw $(libdir)
	cp *.h $(incdir)/xtcw/
	cp $(LIB) $(libdir)/

clean:
	rm -f *.o *.a
