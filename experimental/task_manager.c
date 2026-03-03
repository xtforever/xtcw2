#include "task_manager.h"
#include "mls.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

typedef struct {
    int id;
    pthread_t thread;
    task_state_t state;
    int cmd_pipe[2];
} task_job_t;

typedef struct {
    char *name;
    task_func_t func;
} task_reg_t;

static int status_pipe[2];
static int jobs_list = -1;
static int func_registry = -1;
static XtAppContext xt_app;
static task_event_cb_t event_callback = NULL;

static void status_pipe_cb(XtPointer client_data, int *source, XtInputId *id) {
    task_msg_t msg;
    int n = read(*source, &msg, sizeof(msg));
    if (n == sizeof(msg)) {
        if (event_callback) {
            event_callback(&msg);
        } else {
            printf("[Task %d] Event %d: %.1f%% - %s\n", msg.job_id, msg.type, msg.progress, msg.message);
        }
        
        if (msg.type == TASK_EVENT_COMPLETE || msg.type == TASK_EVENT_ERROR) {
            task_cleanup(msg.job_id);
        }
    }
}

void task_manager_init(XtAppContext app) {
    m_init();
    xt_app = app;
    if (pipe(status_pipe) == -1) {
        perror("task_manager_init: pipe");
        return;
    }
    XtAppAddInput(app, status_pipe[0], (XtPointer)XtInputReadMask, status_pipe_cb, NULL);
    jobs_list = m_create(10, sizeof(task_job_t));
    func_registry = m_create(5, sizeof(task_reg_t));
}

int task_spawn(void *(*func)(task_thread_args_t*), void *arg) {
    task_job_t job;
    static int next_id = 1;
    
    job.id = next_id++;
    job.state = TASK_STATE_RUNNING;
    
    if (pipe(job.cmd_pipe) == -1) {
        perror("task_spawn: cmd pipe");
        return -1;
    }
    
    int flags = fcntl(job.cmd_pipe[0], F_GETFL, 0);
    fcntl(job.cmd_pipe[0], F_SETFL, flags | O_NONBLOCK);

    task_thread_args_t *targs = malloc(sizeof(task_thread_args_t));
    targs->job_id = job.id;
    targs->cmd_fd = job.cmd_pipe[0];
    targs->user_arg = arg;

    if (pthread_create(&job.thread, NULL, (void*(*)(void*))func, targs) != 0) {
        perror("task_spawn: pthread_create");
        close(job.cmd_pipe[0]);
        close(job.cmd_pipe[1]);
        free(targs);
        return -1;
    }
    pthread_detach(job.thread);
    
    m_put(jobs_list, &job);
    return job.id;
}

void task_control(int id, task_cmd_t cmd) {
    int i;
    task_job_t *job;
    m_foreach(jobs_list, i, job) {
        if (job->id == id) {
            write(job->cmd_pipe[1], &cmd, sizeof(cmd));
            return;
        }
    }
}

void task_cleanup(int id) {
    int i;
    task_job_t *job;
    m_foreach(jobs_list, i, job) {
        if (job->id == id) {
            close(job->cmd_pipe[0]);
            close(job->cmd_pipe[1]);
            m_del(jobs_list, i);
            return;
        }
    }
}

void task_report(int id, task_event_t type, float progress, const char *fmt, ...) {
    task_msg_t msg;
    msg.job_id = id;
    msg.type = type;
    msg.progress = progress;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg.message, sizeof(msg.message), fmt, ap);
    va_end(ap);
    write(status_pipe[1], &msg, sizeof(msg));
}

task_cmd_t task_check_command(task_thread_args_t *args) {
    task_cmd_t cmd;
    if (read(args->cmd_fd, &cmd, sizeof(cmd)) == sizeof(cmd)) {
        return cmd;
    }
    return TASK_CMD_NONE;
}

void task_set_event_cb(task_event_cb_t cb) {
    event_callback = cb;
}

void task_register_func(const char *name, task_func_t func) {
    task_reg_t reg;
    reg.name = strdup(name);
    reg.func = func;
    m_put(func_registry, &reg);
}

task_func_t task_get_func(const char *name) {
    int i;
    task_reg_t *reg;
    m_foreach(func_registry, i, reg) {
        if (strcmp(reg->name, name) == 0) return reg->func;
    }
    return NULL;
}
