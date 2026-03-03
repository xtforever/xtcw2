CC := gcc
AR := ar
YACC := bison
LEX := flex

# Directories
WIDGET_DIR := widgets
SRC_WIDGETS_DIR := src/widgets
SRC_CORE_DIR := src/core
SRC_UTIL_DIR := src/util
SRC_LAYOUT_DIR := src/layout
SRC_APPS_DIR := src/apps
GEN_DIR := generated
BUILD_DIR := build
INC_DIR := include

# Tool: wbuild
WBUILD_SRC_DIR := $(SRC_UTIL_DIR)/wbuild
WBUILD_EXE := $(BUILD_DIR)/wbuild
WB_WRAP := $(WBUILD_SRC_DIR)/wb.sh

# Lua source and build
LUA_SRC_DIR := $(SRC_UTIL_DIR)/lua
LUA_BUILD_DIR := $(BUILD_DIR)/util/lua
LUA_LIB := $(LUA_BUILD_DIR)/liblua.a

# Lpeg source and build
LPEG_SRC_DIR := $(SRC_UTIL_DIR)/lpeg
LPEG_BUILD_DIR := $(BUILD_DIR)/util/lpeg
LPEG_OBJ := $(LPEG_BUILD_DIR)/lpcap.o $(LPEG_BUILD_DIR)/lpcode.o $(LPEG_BUILD_DIR)/lpprint.o $(LPEG_BUILD_DIR)/lptree.o $(LPEG_BUILD_DIR)/lpvm.o $(LPEG_BUILD_DIR)/lpcset.o

# Nanosvg include
NANOSVG_INC := $(SRC_UTIL_DIR)/nanosvg

# Flags
unexport CPPFLAGS
unexport CFLAGS
COMPTAG := "\"$(shell date)\""
CFLAGS := -Wall -Wextra -std=c11 -fPIC -fms-extensions -g -D_GNU_SOURCE -DCOMPTAG=$(COMPTAG)
CPPFLAGS := -I$(INC_DIR)/project -I$(INC_DIR)/project/xtcw -I$(GEN_DIR) -I$(GEN_DIR)/xtcw \
	-I$(LUA_SRC_DIR) -I$(NANOSVG_INC) -I$(SRC_UTIL_DIR) -I$(SRC_LAYOUT_DIR) \
	$(shell pkg-config --cflags cairo cairo-xlib) -I/usr/include/freetype2 \
	-include xtcw_resources.h

LDFLAGS := -lXaw -lX11 -lXt -lXpm -lXext -lXmu -lXft -lfontconfig -lXrender -lm \
	$(shell pkg-config --libs cairo cairo-xlib) -ldl -lreadline

# Library Targets
LIB_NAME := xtcw
STATIC_LIB := $(BUILD_DIR)/lib/lib$(LIB_NAME).a
SHARED_LIB := $(BUILD_DIR)/lib/lib$(LIB_NAME).so

# Find widgets
WIDGET_SOURCES := $(wildcard $(WIDGET_DIR)/*.widget)
# Gridbox is @nocode, we have it in src/widgets/
WIDGET_SOURCES_WITH_CODE := $(filter-out $(WIDGET_DIR)/Gridbox.widget,$(WIDGET_SOURCES))

# Generated files from .widget files
GEN_C_WIDGETS := $(patsubst $(WIDGET_DIR)/%.widget,$(GEN_DIR)/%.c,$(WIDGET_SOURCES_WITH_CODE))
GEN_H_WIDGETS := $(patsubst $(WIDGET_DIR)/%.widget,$(GEN_DIR)/%.h,$(WIDGET_SOURCES_WITH_CODE))
GEN_C_WIDGETS += $(GEN_DIR)/register_wb.c $(GEN_DIR)/widreg.c

# Core logic sources (WCL)
CORE_SOURCES := $(wildcard $(SRC_CORE_DIR)/*.c)
CORE_SOURCES += $(wildcard $(SRC_CORE_DIR)/tasks/*.c)

# Layout engine sources (re-tex)
LAYOUT_SOURCES := $(wildcard $(SRC_LAYOUT_DIR)/*.c)

# Lua integration sources
LUA_INT_SOURCES := $(filter-out $(SRC_CORE_DIR)/lua/m_mysql.c, $(wildcard $(SRC_CORE_DIR)/lua/*.c))

# Utility sources from makefile_library
UTIL_OBJS_LIST := focus-group.o converters-xft.o xutil.o mls.o ini_read2.o njson_read.o \
	converters.o util.o xlayout.o wcreg2.o xprint2.o micro_vars.o parser_util.o \
	communication.o socket_log.o XCC.o long.o icon.o choosecol.o strarray.o \
	StrToPmap.o Pen.o str2color.o font.o ctx.o table256-crc16.o timer.o \
	cp2.o slop4.o slopnet.o command-parser.o mrb.o m_tool.o svar2.o var5.o opts.o \
	sig_xt.o

# Plain C widget sources
PLAIN_C_WIDGET_SOURCES := $(wildcard $(SRC_WIDGETS_DIR)/*.c)

# Object files mapping
OBJ_WIDGETS_GEN := $(patsubst $(GEN_DIR)/%.c,$(BUILD_DIR)/widgets/%.o,$(GEN_C_WIDGETS))
OBJ_WIDGETS_PLAIN := $(patsubst $(SRC_WIDGETS_DIR)/%.c,$(BUILD_DIR)/widgets/%.o,$(PLAIN_C_WIDGET_SOURCES))
OBJ_CORE := $(patsubst $(SRC_CORE_DIR)/%.c,$(BUILD_DIR)/core/%.o,$(CORE_SOURCES))
OBJ_CORE_TASKS := $(patsubst $(SRC_CORE_DIR)/tasks/%.c,$(BUILD_DIR)/core/tasks/%.o,$(wildcard $(SRC_CORE_DIR)/tasks/*.c))
OBJ_CORE_LUA := $(patsubst $(SRC_CORE_DIR)/lua/%.c,$(BUILD_DIR)/core/lua/%.o,$(LUA_INT_SOURCES))
OBJ_UTIL := $(addprefix $(BUILD_DIR)/util/, $(UTIL_OBJS_LIST))
OBJ_LAYOUT := $(patsubst $(SRC_LAYOUT_DIR)/%.c,$(BUILD_DIR)/layout/%.o,$(LAYOUT_SOURCES))

# Lex/Yacc generated
OBJ_UTIL += $(BUILD_DIR)/util/ini_parse.tab.o $(BUILD_DIR)/util/ini_lex.lex.o
OBJ_UTIL += $(BUILD_DIR)/util/njson_parse.tab.o $(BUILD_DIR)/util/njson_lex.lex.o

# Library objects (Everything except apps)
LIB_OBJ := $(OBJ_WIDGETS_GEN) $(OBJ_WIDGETS_PLAIN) $(OBJ_CORE) $(OBJ_CORE_TASKS) $(OBJ_CORE_LUA) $(OBJ_UTIL) $(OBJ_LAYOUT) $(LPEG_OBJ)

# Lua objects
LUA_CORE_O := lapi.o lcode.o lctype.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o \
	lmem.o lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o \
	ltm.o lundump.o lvm.o lzio.o ltests.o
LUA_AUX_O := lauxlib.o
LUA_LIB_O := lbaselib.o ldblib.o liolib.o lmathlib.o loslib.o ltablib.o lstrlib.o \
	lutf8lib.o loadlib.o lcorolib.o linit.o
ALL_LUA_OBJ := $(addprefix $(LUA_BUILD_DIR)/, $(LUA_CORE_O) $(LUA_AUX_O) $(LUA_LIB_O))

# Apps
LUARUNNER_EXE := $(BUILD_DIR)/bin/luarunner
RETEXTEST_EXE := $(BUILD_DIR)/bin/retex_test
TESTPANED_EXE := $(BUILD_DIR)/bin/test_paned
RETEX_DEMOS := $(addprefix $(BUILD_DIR)/bin/, demo_png demo_x11 demo_lines demo_x11_lines)

all: $(WBUILD_EXE) $(GEN_H_WIDGETS) $(GEN_DIR)/register_wb.h $(GEN_DIR)/widreg.h $(LUA_LIB) $(STATIC_LIB) $(SHARED_LIB) $(LUARUNNER_EXE) $(RETEXTEST_EXE) $(TESTPANED_EXE) $(RETEX_DEMOS)

# Build the precompiler
$(WBUILD_EXE): 
	@mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(WBUILD_SRC_DIR)
	cp $(WBUILD_SRC_DIR)/wbuild $(WBUILD_EXE)

# Lua library rule
$(LUA_LIB): $(ALL_LUA_OBJ)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(LUA_BUILD_DIR)/%.o: $(LUA_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -O2 -std=c99 -DLUA_USE_LINUX -fPIC -c $< -o $@

$(LPEG_BUILD_DIR)/%.o: $(LPEG_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(LUA_SRC_DIR) -fPIC -c $< -o $@

# Widget Precompiler Rule
$(GEN_DIR)/%.c $(GEN_DIR)/%.h: $(WIDGET_DIR)/%.widget $(WBUILD_EXE)
	@mkdir -p $(GEN_DIR)
	@if [ ! -L $(GEN_DIR)/xtcw ]; then ln -sf . $(GEN_DIR)/xtcw; fi
	@if [ ! -L $(GEN_DIR)/XWBUILD ]; then ln -sf . $(GEN_DIR)/XWBUILD; fi
	@if [ ! -L $(GEN_DIR)/vexmo ]; then ln -sf . $(GEN_DIR)/vexmo; fi
	wb_search=$(WIDGET_DIR) $(WB_WRAP) $< $(WBUILD_EXE) -d $(GEN_DIR) -c $(GEN_DIR) -i $(GEN_DIR) --include-prefix=xtcw

$(GEN_DIR)/register_wb.c $(GEN_DIR)/register_wb.h $(GEN_DIR)/widreg.c $(GEN_DIR)/widreg.h: $(WIDGET_SOURCES)
	@mkdir -p $(GEN_DIR)
	@if [ ! -L $(GEN_DIR)/xtcw ]; then ln -sf . $(GEN_DIR)/xtcw; fi
	./register-widgets.sh $(GEN_DIR) $(GEN_DIR)
	./widreg.sh

# Library rules
$(STATIC_LIB): $(LIB_OBJ)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(SHARED_LIB): $(LIB_OBJ)
	@mkdir -p $(dir $@)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

# Lex/Yacc Rules
$(SRC_UTIL_DIR)/%.tab.c $(SRC_UTIL_DIR)/%.tab.h: $(SRC_UTIL_DIR)/%.y
	$(YACC) -d -o $(SRC_UTIL_DIR)/$*.tab.c $<

$(SRC_UTIL_DIR)/%.lex.c $(SRC_UTIL_DIR)/%.lex.h: $(SRC_UTIL_DIR)/%.l
	$(LEX) -o$(SRC_UTIL_DIR)/$*.lex.c --header-file=$(SRC_UTIL_DIR)/$*.lex.h $<

$(SRC_CORE_DIR)/lua/luaxt_wrap.c: $(SRC_CORE_DIR)/lua/luaxt.i
	swig -lua -o $@ $<

# Compilation Rules
$(BUILD_DIR)/widgets/%.o: $(GEN_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/widgets/%.o: $(SRC_WIDGETS_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/core/%.o: $(SRC_CORE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/core/lua/%.o: $(SRC_CORE_DIR)/lua/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/core/tasks/%.o: $(SRC_CORE_DIR)/tasks/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/layout/%.o: $(SRC_LAYOUT_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/util/%.o: $(SRC_UTIL_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/util/%.tab.o: $(SRC_UTIL_DIR)/%.tab.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/util/%.lex.o: $(SRC_UTIL_DIR)/%.lex.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Apps rules
$(LUARUNNER_EXE): $(BUILD_DIR)/apps/luarunner/luarunner.o $(STATIC_LIB) $(LUA_LIB)
	@mkdir -p $(BUILD_DIR)/bin
	$(CC) $(CFLAGS) $< $(STATIC_LIB) $(LUA_LIB) $(LDFLAGS) -o $@
	cp lib/lua/*.lua $(BUILD_DIR)/bin/
	cp src/apps/luarunner/*.ad $(BUILD_DIR)/bin/
	mkdir -p $(BUILD_DIR)/bin/SVG
	cp lib/SVG/*.svg $(BUILD_DIR)/bin/SVG/

$(BUILD_DIR)/apps/luarunner/luarunner.o: $(SRC_APPS_DIR)/luarunner/luarunner.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Retex Test App
$(RETEXTEST_EXE): $(BUILD_DIR)/apps/retex_test/main.o $(STATIC_LIB)
	@mkdir -p $(BUILD_DIR)/bin
	$(CC) $(CFLAGS) $< $(STATIC_LIB) $(LDFLAGS) -o $@
	cp $(SRC_APPS_DIR)/retex_test/main.ad $(BUILD_DIR)/bin/

# Test Paned App
$(TESTPANED_EXE): $(BUILD_DIR)/apps/test_paned/test_paned.o $(STATIC_LIB)
	@mkdir -p $(BUILD_DIR)/bin
	$(CC) $(CFLAGS) $< $(STATIC_LIB) $(LDFLAGS) -o $@
	cp $(SRC_APPS_DIR)/test_paned/test_paned.ad $(BUILD_DIR)/bin/

$(BUILD_DIR)/apps/test_paned/test_paned.o: $(SRC_APPS_DIR)/test_paned/test_paned.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/apps/retex_test/main.o: $(SRC_APPS_DIR)/retex_test/main.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Retex Demos
$(BUILD_DIR)/bin/%: $(SRC_APPS_DIR)/retex-demos/%.o $(STATIC_LIB)
	@mkdir -p $(BUILD_DIR)/bin
	$(CC) $(CFLAGS) $< $(STATIC_LIB) $(LDFLAGS) -o $@

$(BUILD_DIR)/apps/retex-demos/%.o: $(SRC_APPS_DIR)/retex-demos/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Dependency rules
$(LIB_OBJ): $(GEN_H_WIDGETS) $(GEN_DIR)/register_wb.h $(GEN_DIR)/widreg.h
$(BUILD_DIR)/util/ini_read2.o: $(SRC_UTIL_DIR)/ini_lex.lex.h $(SRC_UTIL_DIR)/ini_parse.tab.h
$(BUILD_DIR)/util/njson_read.o: $(SRC_UTIL_DIR)/njson_lex.lex.h $(SRC_UTIL_DIR)/njson_parse.tab.h

clean:
	rm -rf $(GEN_DIR) build
	rm -f $(SRC_UTIL_DIR)/*.tab.* $(SRC_UTIL_DIR)/*.lex.*
	$(MAKE) -C $(WBUILD_SRC_DIR) clean

.PHONY: all clean
