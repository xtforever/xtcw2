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

CFLAGS+=-fms-extensions
CFLAGS += -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I. -D_GNU_SOURCE
XORGLIB=-lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm
LOADLIBES +=  $(XORGLIB)
LOADLIBES +=  -lmysqlclient


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


WIDGET_OBJ=$(patsubst %.widget,%.o,$(wildcard *.widget))
WIDGET_SRC=$(patsubst %.widget,%.c,$(wildcard *.widget))




ALL: $(TARGET)

Wls.o: Woptc.c Wlist.c xtcw/Woptc.h xtcw/Wlist.h

Wcombo.o: Wls.o

Wsqlcombo: Wcombo.o
.PRECIOUS:  Wsqlcombo.c

$(TARGET): var5.o Wls.o Wlist.o Woptc.o Wsqlcombo.o Wcombo.o WlistScroll.o ClipView.o Woption.o WeditMV.o Wswitch.o Radio2.o opts.o svar2.o m_tool.o m_mysql.o $(libdir)/libxtcw.a 

clean:
	${RM} -rf $(TARGET) *.o  xtcw *~ *.3 *.tex ClipView.c Wheel.c WeditMV.c Woption.c Radio2.c $(WIDGET_SRC) $(WIDGET_OBJ)
