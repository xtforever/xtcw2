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
YFLAGS  = -d
YACC    = bison
LEX     = flex



CFLAGS+=-fms-extensions
CFLAGS += -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I. -D_GNU_SOURCE
XORGLIB=-lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm
LOADLIBES +=  $(XORGLIB)
LOADLIBES +=  -lmysqlclient


DEBUGFLAGS=-g -Wall -DMLS_DEBUG
PRODUCTIONFLAGS=-O3

ifeq ($(production),1)
TAG="\"PROD_$(shell date)\""
CFLAGS+=$(PRODUCTIONFLAGS)
else
TAG="\"DEBUG_$(shell date)\""
CFLAGS+=$(DEBUGFLAGS)
endif

CFLAGS+=-D_GNU_SOURCE -Wall -I../utils
CFLAGS+=-DCOMP_TAG=$(TAG)


# where to find wbuild and wb.sh
wb_install=$(PKGDATADIR)
WBUILD=$(bindir)/wbuild
WBWRAP=$(bindir)/wb.sh
WBPREFIX=xtcw
WFLAGS= -d . -c . -i ./$(WBPREFIX) --include-prefix=$(WBPREFIX) $(PKGDATADIR)/tex.w $(PKGDATADIR)/nroff.w

xtcw/%.h %.c: %.widget
	wb_search=$(srcdir) $(WBWRAP) $^ $(WBUILD) $(WFLAGS)

%.tab.c %.tab.h: %.y
	$(YACC) $(YFLAGS) $<

%.lex.c %.lex.h: %.l
	$(LEX) -o$*.lex.c --header-file=$*.lex.h $<


WIDGET_OBJ=$(patsubst %.widget,%.o,$(wildcard *.widget))

ALL: $(TARGET)

njson_lex.lex.o: njson_parse.tab.c

njson_read.c: njson_parse.tab.c njson_lex.lex.c

Wls.o: Woptc.c Wlist.c xtcw/Woptc.h xtcw/Wlist.h

Wcombo.o: Wls.o

Wsqlcombo: Wcombo.o
# .PRECIOUS:  Wsqlcombo.c

$(TARGET): njson_read.o njson_lex.lex.o njson_parse.tab.o var5.o Wls.o Wlist.o Woptc.o  Wsqlcombo.o Wcombo.o WlistScroll.o m_tool.o m_mysql.o $(libdir)/libxtcw.a 

clean:
	${RM} -rf $(TARGET) *.o  xtcw *~ *.3 *.tex ClipView.c Wheel.c WeditMV.c Woption.c Radio2.c

