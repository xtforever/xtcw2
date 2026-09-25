TARGET=cwri

debug_enable=1

prefix      ?= ../../build
exec_prefix ?= $(prefix)
bindir      ?= $(exec_prefix)/bin
libdir      ?= $(exec_prefix)/lib
incdir      ?= $(prefix)/include
srcdir      ?= $(prefix)/source
docdir      ?= $(prefix)/doc
libexecdir  ?= $(exec_prefix)/libexec
PKGDATADIR  ?= $(prefix)/etc/xtcw

CC = gcc


CFLAGS += -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I.
XORGLIB=-lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm
LOADLIBES +=  $(XORGLIB)


ifeq ($(debug_enable),1)
TAG="\"DEBUG_$(shell date)\""
CFLAGS+= -g -DMLS_DEBUG -O0 -DTAG=$(TAG)
else
TAG="\"PROD_$(shell date)\"" 
CFLAGS+= -O3 -DTAG=$(TAG)
endif

# where to find wbuild and wb.sh
wb_install=$(PKGDATADIR)
WBUILD=$(bindir)/wbuild
WBWRAP=$(bindir)/wb.sh
WBPREFIX=xtcw
WFLAGS= -d . -c . -i ./$(WBPREFIX) --include-prefix=$(WBPREFIX) $(PKGDATADIR)/tex.w $(PKGDATADIR)/nroff.w

xtcw/%.h %.c: %.widget
	wb_search=$(srcdir) $(WBWRAP) $^ $(WBUILD) $(WFLAGS)


ALL: $(TARGET)

$(TARGET): KaroEd.o form_parse.o $(libdir)/libxtcw.a

# --- KaroEd keystroke-injection test suite ---------------------------------
TEST_TARGET=test_karoed

# libxtcw/libutils/libwcl are mutually dependent -> --start-group.
# -lXtst provides the XTEST key injection used by test_karoed/xkey.
TEST_LIBS = -L$(libdir) -Wl,--start-group -lxtcw -lwcl -lutils -Wl,--end-group \
	$(shell pkg-config --libs cairo cairo-xlib) \
	-lXaw -lXmu -lXft -lfontconfig -lXrender -lXpm -lXext -lX11 -lXt -lm -lXtst

$(TEST_TARGET): test_karoed.o xkey.o KaroEd.o form_parse.o | $(libdir)/libxtcw.a
	$(CC) $(CFLAGS) $^ -o $@ $(TEST_LIBS)

# Ensure the generated widget header exists before compiling the driver.
test_karoed.o: | xtcw/KaroEd.h

test: $(TEST_TARGET)
	./run_tests.sh

clean:
	${RM} -rf $(TARGET) $(TEST_TARGET) *.o  xtcw *~ *.3 *.tex KaroEd.c Wheel.c 
