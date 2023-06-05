TARGET=luarunner
LUA=../../lua


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

CFLAGS += -D_GNU_SOURCE -fms-extensions -I$(LUA)
CFLAGS += -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I. -I../utils -Ixtcw

XORGLIB=-lXaw -lX11 -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm -lutil  -lmysqlclient -ldl
LOADLIBES=$(libdir)/libxtcw.a $(LUA)/liblua.a $(XORGLIB)


LOADLIBES+=~/git/libxt-1.1.5/src/.libs/libXt.a -lICE -lSM




# where to find wbuild and wb.sh
wb_install=$(PKGDATADIR)
WBUILD=$(bindir)/wbuild
WBWRAP=$(bindir)/wb.sh
WBPREFIX=xtcw
WFLAGS= -d . -c . -i ./$(WBPREFIX) --include-prefix=$(WBPREFIX) $(PKGDATADIR)/tex.w $(PKGDATADIR)/nroff.w

%_wrap.c: %.i
	swig -lua -o $@ $<

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

DEPS=m_tool.o opts.o svar2.o widreg.o
DEPS+=njson_read.o njson_lex.lex.o njson_parse.tab.o var5.o m_mysql.o
DEPS+=luaxt_wrap.o luaxt.o lua-var5.o

WIDGET_OBJ=$(patsubst %.widget,%.o,$(wildcard *.widget))
WIDGET_SRC=$(patsubst %.widget,%.c,$(wildcard *.widget))


ALL: $(WIDGET_SRC) $(TARGET)

luaxt_wrap.c: luaxt.c  luaxt.i

njson_lex.lex.o: njson_parse.tab.c

njson_read.c: njson_parse.tab.c njson_lex.lex.c


$(TARGET):  $(DEPS)   $(WIDGET_OBJ)

widreg.c widreg.h: $(WIDGET_SRC)
	./widreg.sh


clean:
	${RM} -rf $(TARGET) luaxt_wrap.c widreg.c widreg.h *.o src doc xtcw *~ *.3 *.tex script.fifo *.tab.* *.lex.* $(WIDGET_SRC) $(WIDGET_OBJ)
