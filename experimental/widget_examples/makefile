TARGET=cwri

debug_enable=1

prefix      ?= ../build
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
WFLAGS= -d . -c . -i ./$(WBPREFIX) --include-prefix=$(WBPREFIX) $(PKGDATADIR)/tex.w


xtcw/%.h %.c: %.widget
	wb_search=$(srcdir) $(WBWRAP) $^ $(WBUILD) $(WFLAGS)


VPATH=../utils
ALL: $(TARGET)

Wdemo.o:  Wdemo.c
Wdemo2.o:  Wdemo2.c
Wdemo3.o:  Wdemo3.c

$(TARGET): Wdemo.o Wdemo2.o Wdemo3.o $(libdir)/libxtcw.a

clean:
	${RM} -rf $(TARGET) *.o  xtcw *~ *.3 *.tex Wdemo.c Wheel.c Wdemo2.c Wdemo3.c

