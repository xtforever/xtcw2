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

void register_task_lua(lua_State *L);
void *copy_task(task_thread_args_t *args);

#define APP_NAME "luarunner"

static XrmOptionDescRec options[] = {
    { "-Luafile",    "*luafile",    XrmoptionSepArg, NULL },
    { "-Tracelevel", "*traceLevel", XrmoptionSepArg, NULL },
    WCL_XRM_OPTIONS
};

char *fallback_resources[] = {
    APP_NAME ".allowShellResize: False",
    APP_NAME ".name: " APP_NAME,
    "*width: 600",
    "*height: 400",
    "*traceLevel: 2",
    "*luafile: ex.lua",
    NULL
};

typedef struct COMMANDER_CONFIG {
    int traceLevel;
    char *luafile;
} COMMANDER_CONFIG;

#define FLD(n)  XtOffsetOf(COMMANDER_CONFIG, n)

static XtResource COMMANDER_CONFIG_RES[] = {
    { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
      FLD(traceLevel), XtRImmediate, 0 },
    { "luafile", "Luafile", XtRString, sizeof(String),
      FLD(luafile), XtRString, NULL },
};

#undef FLD

static struct COMMANDER_CONFIG COMMANDER = { 0 };

extern lua_State *L_GLOBAL;
XtAppContext LUAXT_APP;
Widget TopLevel;

void quit_cb(Widget w, void *u, void *c)
{
    XtAppSetExitFlag(XtWidgetToApplicationContext(w));
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
    luaxt_pushcallback(buf, "");
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
    
    luaxt_pushcallback(m_str(m), "");
    m_free(m);
}

static void process_lua_cbs(XtPointer data, XtIntervalId *id) {
    char *cb;
    while (1) {
        cb = luaxt_pullcallback();
        if (!cb || !*cb) break;
        if (luaL_dostring(L_GLOBAL, cb) != LUA_OK) {
            fprintf(stderr, "Lua CB Error: %s\n", lua_tostring(L_GLOBAL, -1));
        }
    }
    XtAppAddTimeOut(LUAXT_APP, 100, process_lua_cbs, NULL);
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

    XtcwRegister(app);
    XpRegisterAll(app);

    task_manager_init(app);
    task_register_func("copy_task", (task_func_t)copy_task);

    luaxt_init();
    L_GLOBAL = luaL_newstate();
    luaL_openlibs(L_GLOBAL);

    lua_bridge_register_bindings();

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
    
    WcInitialize(TopLevel);
    WcRootWidget(TopLevel);

    XtGetApplicationResources(TopLevel, (XtPointer)&COMMANDER,
                              COMMANDER_CONFIG_RES,
                              XtNumber(COMMANDER_CONFIG_RES),
                              (ArgList)0, 0);

    trace_level = COMMANDER.traceLevel;

    /* Load LUI early - before realize, so widgets are created first */
    if (COMMANDER.luafile && strlen(COMMANDER.luafile) > 0) {
        /* Hide shell temporarily - LUI will create managed children */
        XtVaSetValues(TopLevel, XtNmappedWhenManaged, False, NULL);
        if (lui_bootstrap(L_GLOBAL, COMMANDER.luafile, XtName(TopLevel)) != 0) {
            fprintf(stderr, "Failed to bootstrap LUI\n");
            luaxt_destroy();
            lua_close(L_GLOBAL);
            task_manager_cleanup();
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
        WidgetList children;
        Cardinal num_children;
        XtVaGetValues(TopLevel, XtNchildren, &children, XtNnumChildren, &num_children, NULL);
        /* Find the first managed child with a positive size (skip the
         * shell's internal "shellext" child, which has size 0x0). */
        for (Cardinal i = 0; i < num_children; i++) {
            Dimension width, height;
            XtVaGetValues(children[i], XtNwidth, &width, XtNheight, &height, NULL);
            if (width > 0 && height > 0) {
                XtVaSetValues(TopLevel, XtNwidth, width, XtNheight, height, NULL);
                break;
            }
        }
    }

    XtRealizeWidget(TopLevel);

    /* Make shell visible now that LUI widgets are created */
    if (COMMANDER.luafile && strlen(COMMANDER.luafile) > 0) {
        XtMapWidget(TopLevel);
    }

    XtAppAddTimeOut(app, 100, process_lua_cbs, NULL);

    XtAppMainLoop(app);

    luaxt_destroy();
    lua_close(L_GLOBAL);
    m_destruct();
	printf("terminated normaly\n");
    return 0;
}
