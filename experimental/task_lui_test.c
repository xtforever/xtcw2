#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include "task_manager.h"
#include "luaxt.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* Define globals declared in luaxt.h */
XtAppContext LUAXT_APP;
Widget TopLevel;

/* Forward declaration of registration function */
void register_task_lua(lua_State *L);

void *dummy_task(task_thread_args_t *args) {
    int job_id = args->job_id;
    float progress = 0.0;
    int paused = 0;

    while (progress < 100.0) {
        task_cmd_t cmd = task_check_command(args);
        if (cmd == TASK_CMD_ABORT) {
            task_report(job_id, TASK_EVENT_ERROR, progress, "Aborted");
            free(args); return NULL;
        } else if (cmd == TASK_CMD_STOP) {
            paused = 1;
            task_report(job_id, TASK_EVENT_PROGRESS, progress, "PAUSED");
        } else if (cmd == TASK_CMD_CONT) {
            paused = 0;
            task_report(job_id, TASK_EVENT_PROGRESS, progress, "RESUMED");
        }

        if (!paused) {
            progress += 10.0;
            task_report(job_id, TASK_EVENT_PROGRESS, progress, "Working...");
            usleep(500000);
        } else {
            usleep(100000);
        }
    }
    task_report(job_id, TASK_EVENT_COMPLETE, 100.0, "DONE");
    free(args); return NULL;
}

static void process_lua_cbs(XtPointer data, XtIntervalId *id) {
    lua_State *L = (lua_State *)data;
    char *cb;
    while (1) {
        cb = luaxt_pullcallback();
        if (!cb || !*cb) break;
        if (luaL_dostring(L, cb) != LUA_OK) {
            fprintf(stderr, "Lua CB Error: %s\n", lua_tostring(L, -1));
        }
    }
    XtAppAddTimeOut(LUAXT_APP, 100, process_lua_cbs, data);
}

static void send_resume(XtPointer data, XtIntervalId *id) {
    printf("Simulating RESUME from C side for job 1\n");
    task_control(1, TASK_CMD_CONT);
}

int main(int argc, char **argv) {
    XtAppContext app;
    lua_State *L;

    XInitThreads();
    TopLevel = XtOpenApplication(&app, "TaskLuiTest", NULL, 0, &argc, argv, NULL,
                                 applicationShellWidgetClass, NULL, 0);
    LUAXT_APP = app;

    task_manager_init(app);
    task_register_func("dummy_task", (task_func_t)dummy_task);

    luaxt_init();
    L = luaL_newstate();
    luaL_openlibs(L);
    register_task_lua(L);

    if (luaL_dofile(L, "task_test.lua") != LUA_OK) {
        fprintf(stderr, "Lua Error: %s\n", lua_tostring(L, -1));
        exit(1);
    }

    XtAppAddTimeOut(app, 100, process_lua_cbs, L);
    XtAppAddTimeOut(app, 4000, send_resume, NULL);

    XtAppMainLoop(app);
    return 0;
}
