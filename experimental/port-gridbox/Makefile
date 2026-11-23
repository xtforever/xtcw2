# Isolated wbuild port experiment for Gridbox.
# Nothing here is installed into ../../build or referenced by the root makefile.

PORT   := $(CURDIR)
ROOT   := ../..
OUT    := $(PORT)/build
INC    := $(OUT)/include
SRC    := $(OUT)/src
CC     ?= gcc

# The current ../../build tree is a debug build; libutils uses the MLS_DEBUG
# wrappers, so the experiment must use the same wrapper API.
CFLAGS += -Wall -D_GNU_SOURCE -DMLS_DEBUG -I$(INC) -I$(ROOT)/build/include \
          -I$(ROOT)/utils -I$(ROOT)/wcl -I$(PORT)

WBUILD := $(ROOT)/build/bin/wbuild
WBWRAP := $(ROOT)/build/bin/wb.sh
WFLAGS := -d $(OUT)/doc -c $(SRC) -i $(INC)/xtcw \
          --include-prefix=xtcw --hdf-dir=$(OUT)/doc/hdf \
          $(ROOT)/build/etc/xtcw/tex.w $(ROOT)/build/etc/xtcw/nroff.w

LIBS := -L$(ROOT)/build/lib \
  -Wl,--start-group -lxtcw -lwcl -lretex -lnanosvg -lutils -lutf32 -lutf8 -Wl,--end-group \
  $(shell pkg-config --libs cairo cairo-xlib) \
  -lXaw -lXmu -lXft -lfontconfig -lXrender -lXpm -lXext -lX11 -lXt -lm \
  $(shell pkg-config --libs harfbuzz freetype2 fontconfig)

LIBS_ORIG := -L$(ROOT)/build/lib \
  -Wl,--start-group -lxtcw -lwcl -lretex -lnanosvg -lutils -lutf32 -lutf8 -lplainc -Wl,--end-group \
  $(shell pkg-config --libs cairo cairo-xlib) \
  -lXaw -lXmu -lXft -lfontconfig -lXrender -lXpm -lXext -lX11 -lXt -lm \
  $(shell pkg-config --libs harfbuzz freetype2 fontconfig)

all: test_port_gridbox test_port_gridbox_orig abi_check_orig abi_check_port

$(OUT)/.wbuild.stamp: Gridbox.widget gridbox_compat.h
	mkdir -p $(INC)/xtcw $(SRC) $(OUT)/doc/hdf
	wb_search=$(PORT) $(WBWRAP) Gridbox.widget $(WBUILD) $(WFLAGS)
	cp -p gridbox_compat.h $(INC)/xtcw/
	touch $@

$(OUT)/Gridbox.o: $(OUT)/.wbuild.stamp
	$(CC) $(CFLAGS) -c $(SRC)/Gridbox.c -o $@

$(OUT)/gridbox_core.o: gridbox_core.c $(OUT)/.wbuild.stamp
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/libgridboxport.a: $(OUT)/Gridbox.o $(OUT)/gridbox_core.o
	ar rcs $@ $^

test_port_gridbox: test_port_gridbox.c $(OUT)/libgridboxport.a
	$(CC) $(CFLAGS) $< $(OUT)/libgridboxport.a -o $@ $(LIBS)

test_port_gridbox_orig: test_port_gridbox.c
	$(CC) -I$(ROOT)/build/include -I$(ROOT)/build/include/xtcw $(CFLAGS) \
	  $< -o $@ $(LIBS_ORIG)

abi_check_orig: abi_check.c
	$(CC) $(CFLAGS) -I$(ROOT)/plainc_widgets $< -o $@ $(LIBS_ORIG)

abi_check_port: abi_check.c $(OUT)/.wbuild.stamp
	$(CC) $(CFLAGS) -DUSE_PORT $< -o $@ $(LIBS)

run: all
	xvfb-run -a ./test_port_gridbox_orig
	xvfb-run -a ./test_port_gridbox
	./abi_check_orig > $(OUT)/abi.orig
	./abi_check_port > $(OUT)/abi.port
	diff -u $(OUT)/abi.orig $(OUT)/abi.port

clean:
	rm -rf $(OUT) test_port_gridbox test_port_gridbox_orig \
	       abi_check_orig abi_check_port

.PHONY: all run clean
