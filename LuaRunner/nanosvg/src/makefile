CC ?= gcc
CFLAGS += -I.

# nanosvg is header-only, but it needs an implementation file to create a library.
# We'll create a small C file that defines the implementation.

all: libnanosvg.a

nanosvg_impl.c:
	echo '#define NANOSVG_IMPLEMENTATION' > $@
	echo '#include "nanosvg.h"' >> $@
	echo '#define NANOSVGRAST_IMPLEMENTATION' >> $@
	echo '#include "nanosvgrast.h"' >> $@

libnanosvg.a: nanosvg_impl.o
	ar rcs $@ $^

install: libnanosvg.a
	mkdir -p $(incdir) $(libdir)
	cp -p *.h $(incdir)/
	cp -p libnanosvg.a $(libdir)/

clean:
	rm -f *.o *.a nanosvg_impl.c
