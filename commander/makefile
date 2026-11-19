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


# Build mode. When driven from the root makefile, debug_enable and CFLAGS are
# exported (debug = -g -DMLS_DEBUG -O0, release = -O3). Standalone builds of
# commander/ default to debug for backward compatibility; always prefer building
# from the repository root so that the toolkit libraries and commander_runner
# use the same MLS_DEBUG setting (mixing them crashes at runtime).
debug_enable ?= 1

CC = gcc
CFLAGS += -Wall -D_GNU_SOURCE -fms-extensions
CFLAGS += -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I. -I../utils -Ixtcw

ifeq ($(debug_enable),1)
CFLAGS += -g -O0 -DMLS_DEBUG -fsanitize=address -fno-omit-frame-pointer
else
CFLAGS += -O3
endif

LOADLIBES=$(libdir)/libxtcw.a $(libdir)/libwcl.a $(libdir)/libutils.a $(libdir)/libretex.a $(libdir)/libplainc.a $(libdir)/libutf32.a $(libdir)/libutf8.a -lm

# Prefer a system-wide lua5.3 development package. If pkg-config cannot find
# one, fall back to the local prefix created by extracting liblua5.3-dev into
# build/lua5.3 (see build/lua5.3/usr/lib/x86_64-linux-gnu/pkgconfig/lua5.3.pc).
LUA_MULTIARCH ?= $(shell gcc -dumpmachine)
LUA_PC_DIR ?= $(CURDIR)/../build/lua5.3/usr/lib/$(LUA_MULTIARCH)/pkgconfig
LUA_CFLAGS=$(shell pkg-config --cflags lua5.3 2>/dev/null || PKG_CONFIG_PATH=$(LUA_PC_DIR) pkg-config --cflags lua5.3 2>/dev/null)
LUA_LIBS=$(shell pkg-config --libs lua5.3 2>/dev/null || PKG_CONFIG_PATH=$(LUA_PC_DIR) pkg-config --libs lua5.3 2>/dev/null)
MYSQL_CFLAGS=$(shell pkg-config --cflags mysqlclient 2>/dev/null || echo -I/usr/include/mysql)
MYSQL_LIBS=$(shell pkg-config --libs mysqlclient 2>/dev/null || echo -lmysqlclient)
XORGLIB=-lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm -lutil -ldl -lcairo $(shell pkg-config --libs harfbuzz freetype2 fontconfig)
CFLAGS += $(LUA_CFLAGS) $(MYSQL_CFLAGS) -I../LuaRunner

T=commander_runner
D=mls.o ctx.o m_tool.o
ALL: ${T}

${T}: ${D}

# Rule to generate SWIG wrapper
../LuaRunner/luaxt_wrap.c: ../LuaRunner/luaxt.i ../LuaRunner/luaxt.c
	swig -lua -o $@ $<

# Depend on the toolkit archives so a rebuilt widget library forces a relink.
COMMANDER_LIBS=$(libdir)/libxtcw.a $(libdir)/libwcl.a $(libdir)/libutils.a $(libdir)/libretex.a $(libdir)/libplainc.a $(libdir)/libutf32.a $(libdir)/libutf8.a

commander_runner: commander_runner.o task_manager.o lua_task_bindings.o lua_bridge.o xt_bridge.o lifecycle.o file_ops.o script_process.o db_mysql.o ../LuaRunner/luaxt.o ../LuaRunner/luaxt_wrap.o $(COMMANDER_LIBS)
	$(CC) $(CFLAGS) -o $@ $(filter %.o,$^) $(LOADLIBES) $(LUA_LIBS) $(MYSQL_LIBS) $(XORGLIB) -lpthread

clean:
	-${RM} *.o ${T}
	rm -f ../LuaRunner/luaxt.o ../LuaRunner/luaxt_wrap.o ../LuaRunner/luaxt_wrap.c
