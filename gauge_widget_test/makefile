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
CFLAGS += -D_GNU_SOURCE -fms-extensions -Wall -fsanitize=address -fno-omit-frame-pointer
XORGLIB=-lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm -lXaw
LOADLIBES +=  $(XORGLIB)


ifeq ($(debug_enable),1)
TAG="\"DEBUG_$(shell date)\""
CFLAGS+= -ggdb -DMLS_DEBUG -O0 -DTAG=$(TAG)
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


ALL: $(TARGET) sentest wlanstat cpustats

wlanstat: CFLAGS+=-DWITH_WLANSTAT_MAIN 


sentest:  sensorreg.o conststr.o mls.o

Gauge2.c: Gauge2.widget

$(TARGET): nbus.o Gauge.o $(libdir)/libxtcw.a Gauge2.o parsertool.o sensorreg.o conststr.o var5.o m_tool.o  subshell.o

clean:
	${RM} -rf $(TARGET) *.o  Gauge2.c xtcw *~ *.3 *.tex Gauge.c Wheel.c 

extra: clean ALL
