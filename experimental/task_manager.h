#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <pthread.h>
#include <X11/Intrinsic.h>

typedef enum {
    TASK_STATE_IDLE,
    TASK_STATE_RUNNING,
    TASK_STATE_STOPPED,
    TASK_STATE_ABORTED,
    TASK_STATE_DONE
} task_state_t;

typedef enum {
    TASK_EVENT_PROGRESS,
    TASK_EVENT_COMPLETE,
    TASK_EVENT_ERROR
} task_event_t;

typedef struct {
    int job_id;
    task_event_t type;
    float progress;
    char message[256];
} task_msg_t;

typedef enum {
    TASK_CMD_NONE = 0,
    TASK_CMD_STOP,
    TASK_CMD_CONT,
    TASK_CMD_ABORT
} task_cmd_t;

/* Structure passed to the worker thread */
typedef struct {
    int job_id;
    int cmd_fd;
    void *user_arg;
} task_thread_args_t;

void task_manager_init(XtAppContext app);
int  task_spawn(void *(*func)(task_thread_args_t*), void *arg);
void task_control(int id, task_cmd_t cmd);
void task_cleanup(int id);

/* Helper for threads to report status */
void task_report(int id, task_event_t type, float progress, const char *fmt, ...);

/* Helper for threads to check for commands (non-blocking) */
task_cmd_t task_check_command(task_thread_args_t *args);

typedef void (*task_event_cb_t)(task_msg_t *msg);
void task_set_event_cb(task_event_cb_t cb);

/* Registry for task functions */
typedef void *(*task_func_t)(task_thread_args_t*);
void task_register_func(const char *name, task_func_t func);
task_func_t task_get_func(const char *name);

/* Lua event callback name */
extern char *task_lua_event_callback;

#endif
