VPATH=../utils
prefix      ?= ../build
exec_prefix ?= $(prefix)
bindir      ?= $(exec_prefix)/bin
libdir      ?= $(exec_prefix)/lib
incdir      ?= $(prefix)/include
srcdir      ?= $(prefix)/source
docdir      ?= $(prefix)/doc
libexecdir  ?= $(exec_prefix)/libexec
PKGDATADIR  ?= $(prefix)/etc/xtcw


CC=gcc -I. -I../utils -ggdb -O0 -DMLS_DEBUG -Wall -fsanitize=address -fno-omit-frame-pointer
CFLAGS += -D_GNU_SOURCE -fms-extensions
CFLAGS += -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I. -I../utils -Ixtcw

LOADLIBES=$(libdir)/libxtcw.a $(libdir)/libwcl.a $(libdir)/libutils.a $(libdir)/libretex.a $(libdir)/libplainc.a -lm

LUA_CFLAGS=$(shell pkg-config --cflags lua5.3)
LUA_LIBS=$(shell pkg-config --libs lua5.3)
XORGLIB=-lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm -lutil -ldl -lcairo
CFLAGS += $(LUA_CFLAGS) -I../LuaRunner

T=commander_runner
D=mls.o ctx.o m_tool.o
ALL: ${T}

${T}: ${D}

# Rule to generate SWIG wrapper
../LuaRunner/luaxt_wrap.c: ../LuaRunner/luaxt.i ../LuaRunner/luaxt.c
	swig -lua -o $@ $<

# Depend on the toolkit archives so a rebuilt widget library forces a relink.
COMMANDER_LIBS=$(libdir)/libxtcw.a $(libdir)/libwcl.a $(libdir)/libutils.a $(libdir)/libretex.a $(libdir)/libplainc.a

commander_runner: commander_runner.o task_manager.o lua_task_bindings.o lua_bridge.o xt_bridge.o file_ops.o ../LuaRunner/luaxt.o ../LuaRunner/luaxt_wrap.o $(COMMANDER_LIBS)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) $(LOADLIBES) $(LUA_LIBS) $(XORGLIB) -lpthread

clean:
	-${RM} *.o ${T}
	rm -f ../LuaRunner/luaxt_wrap.c
