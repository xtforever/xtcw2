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

YFLAGS  = -d
YACC    = bison
LEX     = flex
CC = gcc

CFLAGS += -D_GNU_SOURCE -fms-extensions
CFLAGS += -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I. -I../utils -Ixtcw

XORGLIB=-lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm -lutil  -lmysqlclient
LOADLIBES=$(libdir)/libxtcw.a $(XORGLIB)


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


ifeq ($(debug_enable),1)
COMPTAG="\"DEBUG_$(shell date)\""
CFLAGS+= -g -Wall -DMLS_DEBUG -O0 -DCOMPTAG=$(COMPTAG)
else
COMPTAG="\"PROD_$(shell date)\"" 
CFLAGS+= -O3 -DTAG=$(COMPTAG)
endif

DEPS=SelectReq.o WeditMV.o Woption.o  Wswitch.o Radio2.o opts.o svar2.o script_call.o m_tool.o subshell.o
DEPS+=njson_read.o njson_lex.lex.o njson_parse.tab.o var5.o m_mysql.o




WIDGET_OBJ=$(patsubst %.widget,%.o,$(wildcard *.widget))
WIDGET_SRC=$(patsubst %.widget,%.c,$(wildcard *.widget))




ALL:  $(WIDGET_SRC) $(TARGET) njson-to-wcl rlayout jsn

njson-to-wcl: njson_read.o njson_lex.lex.o njson_parse.tab.o m_tool.o var5.o

jsn:  njson_read.o njson_lex.lex.o njson_parse.tab.o m_tool.o

njson_lex.lex.o: njson_parse.tab.c

njson_read.c: njson_parse.tab.c njson_lex.lex.c

Radio2.o: Woption.o

$(TARGET):  $(DEPS)   $(WIDGET_OBJ) 

clean:
	${RM} -rf $(TARGET) *.o src doc xtcw *~ *.3 *.tex script.fifo *.tab.* *.lex.* $(WIDGET_SRC) $(WIDGET_OBJ) 
