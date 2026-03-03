#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Converters.h>
#include <WcCreate.h>
#include <Xp.h>
#include <xtcw/register_wb.h>
#include "task_manager.h"
#include "luaxt.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/IntrinsicP.h>

/* Forward declaration of registration function */
void register_task_lua(lua_State *L);
void *copy_task(task_thread_args_t *args);

static void LUA_action(Widget w, XEvent* e, String* s, Cardinal* n)
{
    int args = *n;
    if(! args ) return;
    int m = s_printf(0,0,"%s(", *s++ );
    int comma=32; while( --args ) s_printf(m,-1,"%c\"%s\"", comma, *s++ ), comma=',';
    s_app1(m,")");
    luaxt_pushcallback( m_str(m), "" );
    m_free(m);
}

/* Global Lua state */
lua_State *L_GLOBAL;
XtAppContext LUAXT_APP;
Widget TopLevel;

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

static void send_f5(XtPointer data, XtIntervalId *id) {
    printf("Simulating F5 press...\n");
    luaxt_pushcallback("handle_key(\"F5\", \"\")", "");
}

int main(int argc, char **argv) {
    XtAppContext app;
    
    XInitThreads();
    m_init();

    /* Set up Wcl fallback resources if needed */
    char *fallbacks[] = {
        "Commander.WcChildren: paned",
        "*WclResFiles: commander.ad",
        NULL
    };

    TopLevel = XtOpenApplication(&app, "Commander", NULL, 0, &argc, argv, fallbacks,
                                 applicationShellWidgetClass, NULL, 0);
    LUAXT_APP = app;

    /* Register widgets */
    XtcwRegister(app);
    XpRegisterAll(app);

    /* Initialize task system */
    task_manager_init(app);
    task_register_func("copy_task", (task_func_t)copy_task);

    /* Initialize Lua */
    luaxt_init();
    L_GLOBAL = luaL_newstate();
    luaL_openlibs(L_GLOBAL);
    
    /* Register our custom bindings */
    register_task_lua(L_GLOBAL);

    /* Register LUA action */
    static XtActionsRec actions[] = {
        {"LUA", LUA_action},
    };
    XtAppAddActions(app, actions, XtNumber(actions));

    /* Create widgets from .ad */
    WcWidgetCreation(TopLevel);

    if (argc > 1) {
        if (luaL_dofile(L_GLOBAL, argv[1]) != LUA_OK) {
            fprintf(stderr, "Lua Error: %s\n", lua_tostring(L_GLOBAL, -1));
            exit(1);
        }
    }

    XtAppAddTimeOut(app, 100, process_lua_cbs, NULL);
    XtAppAddTimeOut(app, 3000, send_f5, NULL);

    XtRealizeWidget(TopLevel);
    XtAppMainLoop(app);

    return 0;
}
