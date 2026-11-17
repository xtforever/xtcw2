#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Converters.h>
#include <WcCreate.h>
#include <Xp.h>
#include <xtcw/register_wb.h>
#include "wcreg2.h"
#include "mls.h"
#include "task_manager.h"
#include "luaxt.h"
#include "lua_bridge.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xtcw/Wheel.h>
#include "theme.h"
#include "theme_parse.h"

void register_task_lua(lua_State *L);
void *copy_task(task_thread_args_t *args);
void drawcanvas_plugin_init(Widget top);
void snake_plugin_init(Widget top);

#define APP_NAME "luarunner"

static XrmOptionDescRec options[] = {
    { "-Luafile",    "*luafile",    XrmoptionSepArg, NULL },
    { "-Tracelevel", "*traceLevel", XrmoptionSepArg, NULL },
    { "-theme",      "*theme",      XrmoptionSepArg, NULL },
    { "-themeFile",  "*themeFile",  XrmoptionSepArg, NULL },
    WCL_XRM_OPTIONS
};

char *fallback_resources[] = {
    APP_NAME ".allowShellResize: False",
    APP_NAME ".name: " APP_NAME,
    APP_NAME ".width: 600",
    APP_NAME ".height: 400",
    "*traceLevel: 2",
    "*luafile: ex.lua",
    "*themeFile: themes.hdf",
    "*theme: dracula",
    NULL
};

typedef struct COMMANDER_CONFIG {
    int traceLevel;
    char *luafile;
    char *theme;
    char *themeFile;
} COMMANDER_CONFIG;

#define FLD(n)  XtOffsetOf(COMMANDER_CONFIG, n)

static XtResource COMMANDER_CONFIG_RES[] = {
    { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
      FLD(traceLevel), XtRImmediate, 0 },
    { "luafile", "Luafile", XtRString, sizeof(String),
      FLD(luafile), XtRString, NULL },
    { "theme", "Theme", XtRString, sizeof(String),
      FLD(theme), XtRString, NULL },
    { "themeFile", "ThemeFile", XtRString, sizeof(String),
      FLD(themeFile), XtRString, NULL },
};

#undef FLD

static struct COMMANDER_CONFIG COMMANDER = { 0 };

extern lua_State *L_GLOBAL;
XtAppContext LUAXT_APP;
Widget TopLevel;

static void run_lua_chunk(const char *code);
static int top_destroyed = 0;

void quit_cb(Widget w, void *u, void *c)
{
    XtAppSetExitFlag(XtWidgetToApplicationContext(w));
}

/* Remember whether the shell was already torn down by the window manager, so
 * the shutdown path below does not dereference a dead widget pointer. */
static void top_destroy_cb(Widget w, XtPointer cd, XtPointer cb)
{
    (void)w; (void)cd; (void)cb;
    top_destroyed = 1;
}

/* LUI scripts commonly call os.exit() from their quit callback, which skips
 * the normal shutdown path below. Run the same widget teardown from atexit so
 * widget-owned caches (WListView SVG icons, Xft/font caches) are released. */
static void top_cleanup_atexit(void)
{
    if (TopLevel && !top_destroyed) {
        XtDestroyWidget(TopLevel);
        TopLevel = NULL;
        top_destroyed = 1;
    }
}

/* LUA callback function - called when callback "LUA(funcname)" or "LUA(funcname, data)" is triggered */
static void LUA_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (!client_data) return;
    /* Parse funcname and optional data argument from client_data
     * Format: "funcname" or "funcname data" or "funcname, data" */
    char *argstr = (char*)client_data;
    char funcname[256] = {0};
    char data[256] = {0};
    
    /* Skip leading whitespace */
    while (*argstr && (*argstr == ' ' || *argstr == '\t')) argstr++;
    
    /* Extract function name (until space, comma, or end) */
    int i = 0;
    while (*argstr && *argstr != ' ' && *argstr != '\t' && *argstr != ',' && i < 255) {
        funcname[i++] = *argstr++;
    }
    funcname[i] = '\0';
    
    /* Skip separator (space, comma) */
    while (*argstr && (*argstr == ' ' || *argstr == '\t' || *argstr == ',')) argstr++;
    
    /* Extract data argument if present */
    i = 0;
    while (*argstr && *argstr != ')' && i < 255) {
        data[i++] = *argstr++;
    }
    data[i] = '\0';
    
    /* Build the Lua callback string */
    char buf[512];
    if (data[0]) {
        /* Has data argument: funcname("data") */
        snprintf(buf, sizeof(buf), "%s(\"%s\")", funcname, data);
    } else {
        /* No data: funcname() */
        snprintf(buf, sizeof(buf), "%s()", funcname);
    }
    run_lua_chunk(buf);
}

/* LUA action function - called when action "LUA(funcname)" or "LUA(funcname, data)" is triggered */
static void LUA_action(Widget w, XEvent* e, String* s, Cardinal* n)
{
    int args = *n;
    if(! args ) return;
    
    /* Xt passes the entire parenthetical content as a single string.
     * Parse it to extract func name and optional data argument.
     * Format: "funcname" or "funcname data" or "funcname, data" */
    char *argstr = *s;
    char funcname[256] = {0};
    char data[256] = {0};
    
    /* Skip leading whitespace */
    while (*argstr && (*argstr == ' ' || *argstr == '\t')) argstr++;
    
    /* Extract function name (until space, comma, or end) */
    int i = 0;
    while (*argstr && *argstr != ' ' && *argstr != '\t' && *argstr != ',' && i < 255) {
        funcname[i++] = *argstr++;
    }
    funcname[i] = '\0';
    
    /* Skip separator (space, comma) */
    while (*argstr && (*argstr == ' ' || *argstr == '\t' || *argstr == ',')) argstr++;
    
    /* Extract data argument if present */
    i = 0;
    while (*argstr && *argstr != ')' && i < 255) {
        data[i++] = *argstr++;
    }
    data[i] = '\0';
    
    /* Build the Lua callback string */
    int m;
    if (data[0]) {
        /* Has data argument: funcname("data") */
        m = s_printf(0, 0, "%s(\"%s\")", funcname, data);
    } else {
        /* No data: funcname() */
        m = s_printf(0, 0, "%s()", funcname);
    }
    
    run_lua_chunk(m_str(m));
    m_free(m);
}

static void run_lua_chunk(const char *code)
{
    if (luaL_loadstring(L_GLOBAL, code) != LUA_OK) {
        fprintf(stderr, "Lua CB Error: %s\n", lua_tostring(L_GLOBAL, -1));
        lua_pop(L_GLOBAL, 1);
        return;
    }
    if (lua_pcall(L_GLOBAL, 0, 0, 0) != LUA_OK) {
        fprintf(stderr, "Lua CB Error: %s\n", lua_tostring(L_GLOBAL, -1));
        lua_pop(L_GLOBAL, 1);
    }
}

static int lui_bootstrap(lua_State *L, const char *luafile, const char *shell_name)
{
    char bootstrap_code[4096];
    snprintf(bootstrap_code, sizeof(bootstrap_code),
        "package.path = '../lui/?.lua;' .. package.path\n"
        "package.cpath = '../lui/?.so;' .. package.cpath\n"
        "local gui = require('gui_xt')\n"
        "local lui = require('lui')\n"
        "lui.loop = function() end\n"
        "gui.loop = function() end\n"
        "_G.__shell_name__ = '%s'\n"
        "local __ok, __err = lui.load('%s')\n"
        "if not __ok then print('LUI load error: ' .. tostring(__err)) end\n",
        shell_name, luafile);

    if (luaL_dostring(L, bootstrap_code) != LUA_OK) {
        fprintf(stderr, "LUI Bootstrap Error: %s\n", lua_tostring(L, -1));
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    XtAppContext app;

    XInitThreads();
    m_init();
    trace_level = 1;

    TopLevel = XtOpenApplication(&app, APP_NAME,
                                  options, XtNumber(options),
                                  &argc, argv,
                                  fallback_resources,
                                  sessionShellWidgetClass,
                                  NULL, 0);
    LUAXT_APP = app;
    XtAddCallback(TopLevel, XtNdestroyCallback, top_destroy_cb, NULL);
    atexit(top_cleanup_atexit);

    XtcwRegister(app);
    XpRegisterAll(app);

    theme_init(TopLevel);

    task_manager_init(app);
    task_register_func("copy_task", (task_func_t)copy_task);

    luaxt_init();
    L_GLOBAL = luaL_newstate();
    luaL_openlibs(L_GLOBAL);

    lua_bridge_register_bindings();
    register_task_lua(L_GLOBAL);

    extern int luaopen_luaxt(lua_State* L);
    luaopen_luaxt(L_GLOBAL);
    lua_setglobal(L_GLOBAL, "luaxt");

    static XtActionsRec actions[] = {
        {"LUA", LUA_action},
    };
    XtAppAddActions(app, actions, XtNumber(actions));
    wcreg_action(TopLevel, "LUA", LUA_action);

    RCB(TopLevel, quit_cb);
    wcreg_callback(TopLevel, LUA_callback, "LUA" );
    drawcanvas_plugin_init(TopLevel);
    snake_plugin_init(TopLevel);
    
    WcInitialize(TopLevel);
    WcRootWidget(TopLevel);

    XtGetApplicationResources(TopLevel, (XtPointer)&COMMANDER,
                              COMMANDER_CONFIG_RES,
                              XtNumber(COMMANDER_CONFIG_RES),
                              (ArgList)0, 0);

    trace_level = COMMANDER.traceLevel;

    /* Load and select the theme store.  Try the configured file first,
     * then locations relative to the usual run directories. */
    {
        const char *cand[3];
        int ci;
        cand[0] = COMMANDER.themeFile;
        cand[1] = "../themes.hdf";
        cand[2] = "themes.hdf";
        for (ci = 0; ci < 3; ci++) {
            if (cand[ci] && access(cand[ci], R_OK) == 0) {
                theme_load(cand[ci]);
                break;
            }
        }
    }
    if (COMMANDER.theme) theme_select(COMMANDER.theme);
    /* Fallback: the default theme is "dracula", but a theme file may not
     * define it (e.g. the cookbook ships only its own "default").  If the
     * requested theme was not found, fall back to "default" so widgets
     * still get colours. */
    if (theme_active() == NULL) {
        char msg[256];
        snprintf(msg, sizeof msg, "theme '%s' not found; falling back to 'default'",
                 COMMANDER.theme ? COMMANDER.theme : "(null)");
        XtWarning(msg);
        theme_select("default");
    }

    /* Load LUI early - before realize, so widgets are created first */
    if (COMMANDER.luafile && strlen(COMMANDER.luafile) > 0) {
        /* Hide shell temporarily - LUI will create managed children */
        XtVaSetValues(TopLevel, XtNmappedWhenManaged, False, NULL);
        if (lui_bootstrap(L_GLOBAL, COMMANDER.luafile, XtName(TopLevel)) != 0) {
            fprintf(stderr, "Failed to bootstrap LUI\n");
            if (TopLevel && !top_destroyed) {
                XtDestroyWidget(TopLevel);
                TopLevel = NULL;
                top_destroyed = 1;
            }
            luaxt_destroy();
            lua_close(L_GLOBAL);
            task_manager_cleanup();
            theme_destroy();
            m_destruct();
            exit(1);
        }
    } else {
        WcWidgetCreation(TopLevel);
    }

    /* Apply the root container's requested size to the shell BEFORE realize.
     * If we wait until after XtRealizeWidget, the shell's own (fallback)
     * size has already been forced onto the child during the initial
     * geometry negotiation, and the LUI `:width`/`:height` are lost. */
    if (COMMANDER.luafile && strlen(COMMANDER.luafile) > 0) {
        WidgetList children = NULL;
        Cardinal num_children = 0;
        XtVaGetValues(TopLevel, XtNchildren, &children, XtNnumChildren, &num_children, NULL);
        /* Find the first managed child with a positive size (skip the
         * shell's internal "shellext" child, which has size 0x0).
         *
         * `width`/`height` MUST be initialised: the shell's internal child
         * does not expose XtNwidth/XtNheight through XtVaGetValues, so the
         * values are left untouched.  Without this the stale stack contents
         * are mistaken for a real size, and at -O3 the shell gets sized to
         * garbage (e.g. 58576x1), which overflows Gridbox and aborts with
         * `X_CreatePixmap BadAlloc`. */
        for (Cardinal i = 0; i < num_children; i++) {
            Dimension width = 0, height = 0;
            Widget child = children ? children[i] : NULL;
            if (!child)
                continue;
            XtVaGetValues(child, XtNwidth, &width, XtNheight, &height, NULL);
            if (width > 0 && height > 0) {
                XtVaSetValues(TopLevel, XtNwidth, width, XtNheight, height, NULL);
                break;
            }
        }
    }

    /* Apply the active theme to all widgets created so far, before realize. */
    if (theme_active()) theme_apply();

    XtRealizeWidget(TopLevel);

    /* Make shell visible now that LUI widgets are created */
    if (COMMANDER.luafile && strlen(COMMANDER.luafile) > 0) {
        XtMapWidget(TopLevel);
    }

    /* Run the Xt event loop. Lua callbacks execute immediately when their
     * widget action/callback fires (see LUA_callback / LUA_action). */
    XtAppMainLoop(app);

    /* Tear down the widget tree before Lua/theme shutdown. Widget destroy
     * methods unregister from the theme and release caches (for example
     * WListView's SVG icon cache); without this ASan reports those caches as
     * leaks when the process exits. */
    if (TopLevel && !top_destroyed) {
        XtDestroyWidget(TopLevel);
        TopLevel = NULL;
    }

    luaxt_destroy();
    lua_close(L_GLOBAL);
    theme_destroy();
    m_destruct();
	printf("terminated normaly\n");
    return 0;
}
