#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include "task_manager.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void *dummy_task(task_thread_args_t *args) {
    int job_id = args->job_id;
    float progress = 0.0;
    int paused = 0;

    while (progress < 100.0) {
        task_cmd_t cmd = task_check_command(args);
        if (cmd == TASK_CMD_ABORT) {
            task_report(job_id, TASK_EVENT_ERROR, progress, "Aborted by user");
            free(args);
            return NULL;
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
    free(args);
    return NULL;
}

static void send_stop(XtPointer data, XtIntervalId *id) {
    printf("Simulating STOP click for job %d\n", (int)(intptr_t)data);
    task_control((int)(intptr_t)data, TASK_CMD_STOP);
}

static void send_cont(XtPointer data, XtIntervalId *id) {
    printf("Simulating CONTINUE click for job %d\n", (int)(intptr_t)data);
    task_control((int)(intptr_t)data, TASK_CMD_CONT);
}

int main(int argc, char **argv) {
    XtAppContext app;
    Widget toplevel;

    XInitThreads();
    toplevel = XtOpenApplication(&app, "TaskTest", NULL, 0, &argc, argv, NULL,
                                 applicationShellWidgetClass, NULL, 0);

    task_manager_init(app);

    int id = task_spawn(dummy_task, NULL);
    printf("Spawned job %d\n", id);

    XtAppAddTimeOut(app, 1500, send_stop, (XtPointer)(intptr_t)id);
    XtAppAddTimeOut(app, 3500, send_cont, (XtPointer)(intptr_t)id);

    XtAppMainLoop(app);

    return 0;
}
