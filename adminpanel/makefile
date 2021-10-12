TARGET=admpanel

debug_enable ?= 1



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
CFLAGS += -D_GNU_SOURCE
CFLAGS += -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I. -I../utils -Ixtcw

XORGLIB=-lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm -lutil
LOADLIBES +=  $(XORGLIB)

# where to find wbuild and wb.sh
wb_install=$(PKGDATADIR)
WBUILD=$(bindir)/wbuild
WBWRAP=$(bindir)/wb.sh
WBPREFIX=xtcw
WFLAGS= -d . -c . -i ./$(WBPREFIX) --include-prefix=$(WBPREFIX) $(PKGDATADIR)/tex.w $(PKGDATADIR)/nroff.w

xtcw/%.h %.c: %.widget
	wb_search=$(srcdir) $(WBWRAP) $^ $(WBUILD) $(WFLAGS)


ifeq ($(debug_enable),1)
COMPTAG="\"DEBUG_$(shell date)\""
CFLAGS+= -g -Wall -DMLS_DEBUG -O0 -DCOMPTAG=$(COMPTAG)
else
COMPTAG="\"PROD_$(shell date)\"" 
CFLAGS+= -O3 -DTAG=$(COMPTAG)
endif

DEPS=SelectReq.o WeditMV.o Woption.o  Wswitch.o Radio2.o opts.o svar2.o script_call.o m_tool.o subshell.o
LOADLIBES=$(libdir)/libxtcw.a $(XORGLIB)
ALL: $(TARGET) mttest

$(TARGET):  $(DEPS)   

mttest: m_tool.o subshell.o ../utils/mls.c

clean:
	${RM} -rf $(TARGET) *.o src doc xtcw *~
