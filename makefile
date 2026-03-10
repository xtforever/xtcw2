# Master Makefile for XTCW Toolkit

# Export settings
export debug_enable ?= 1
export prefix      ?= $(CURDIR)/build
export libdir      ?= $(prefix)/lib
export incdir      ?= $(prefix)/include
export CC          ?= gcc

CAIRO_CFLAGS  = $(shell pkg-config --cflags cairo cairo-xlib)
X11_CFLAGS    = -I/usr/include/freetype2 -I/usr/local/include/freetype2
NANOSVG_CFLAGS = -I$(CURDIR)/LuaRunner/nanosvg/src

CFLAGS += -Wall -D_GNU_SOURCE -I$(CURDIR)/utils -I$(incdir) -I$(incdir)/xtcw $(CAIRO_CFLAGS) $(X11_CFLAGS) $(NANOSVG_CFLAGS)

ifeq ($(debug_enable),1)
CFLAGS += -g -DMLS_DEBUG -O0
else
CFLAGS += -O3
endif

export CFLAGS
export LDFLAGS += -L$(libdir) $(shell pkg-config --libs cairo cairo-xlib) -lXaw -lXmu -lXft -lfontconfig -lXrender -lXpm -lXext -lX11 -lXt -lm

.PHONY: all nanosvg plainc wcl core retex widgets luarunner tests clean run_tests build_tools libxtcw

# Reduced all target to skip problematic components for now
all: nanosvg plainc build_tools widgets wcl core retex libxtcw tests

nanosvg:
	$(MAKE) -C LuaRunner/nanosvg/src
	$(MAKE) -C LuaRunner/nanosvg/src install

plainc:
	$(MAKE) -C plainc_widgets
	$(MAKE) -C plainc_widgets install

build_tools:
	$(MAKE) -C wbuild bindir=$(prefix)/bin PKGDATADIR=$(prefix)/etc/xtcw
	$(MAKE) -C wbuild bindir=$(prefix)/bin PKGDATADIR=$(prefix)/etc/xtcw install

# widgets (pass 1) generates headers needed by core/utils and wcl
widgets: build_tools
	mkdir -p build/source
	cp wbuild_widgets/*.widget build/source/
	$(MAKE) -C wbuild_widgets
	$(MAKE) -C wbuild_widgets install

wcl: plainc widgets
	$(MAKE) -C wcl
	$(MAKE) -C wcl install

# core depends on widget headers (e.g., focus-group.c needs Wheel.h)
core: wcl widgets
	mkdir -p $(incdir)
	cp -r XPM $(incdir)/
	$(MAKE) -C utils
	$(MAKE) -C utils install

retex: core nanosvg
	$(MAKE) -C re-tex/src
	$(MAKE) -C re-tex/src install

# build libxtcw (pass 2) compiles all widget .c files
# it needs retex headers
libxtcw: core widgets retex
	# Copy auxiliary source and header files from wbuild_widgets
	mkdir -p $(incdir)/xtcw
	cp wbuild_widgets/*.h $(incdir)/xtcw/ 2>/dev/null || true
	cp wbuild_widgets/*.h build/source/ 2>/dev/null || true
	cp wbuild_widgets/*.c build/source/ 2>/dev/null || true
	# Create makefile in build/source if it doesn't exist
	printf 'CC ?= gcc\nSRCS = $$(wildcard *.c)\nOBJS = $$(SRCS:.c=.o)\nall: libxtcw.a\nlibxtcw.a: $$(OBJS)\n\tar rcs $$@ $$^\n' > build/source/makefile
	$(MAKE) -C build/source CFLAGS="$(CFLAGS)"
	cp build/source/libxtcw.a $(libdir)/

luarunner: libxtcw retex
	$(MAKE) -C LuaRunner

tests: libxtcw retex
	$(MAKE) -C tests

run_tests: tests
	./tests/run_all_tests.sh

clean:
	rm -rf build
	$(MAKE) -C utils clean
	$(MAKE) -C wbuild clean
	$(MAKE) -C wbuild_widgets clean
	$(MAKE) -C re-tex/src clean
	$(MAKE) -C LuaRunner clean
	$(MAKE) -C tests clean
	$(MAKE) -C wcl clean
	$(MAKE) -C plainc_widgets clean
	$(MAKE) -C LuaRunner/nanosvg/src clean
