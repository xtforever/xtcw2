/* Non-blocking pty-based script runner for LUI.
 *
 * Lua API:
 *   script_start(argv_table_or_command, opts) -> job_id | nil, err
 *   script_write(job_id, text) -> true | nil, err
 *   script_stop(job_id) -> true
 *   script_is_running(job_id) -> boolean
 *
 * opts = { on_event = "lua_global", cwd = "...", term = "dumb",
 *          cols = 120, rows = 40 }
 *
 * A string command is run through /bin/sh -c. A table is passed to execvp.
 */

#include "script_process.h"
#include "lua_bridge.h"

#include <X11/Intrinsic.h>
#include <pty.h>
#include <utmp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern XtAppContext LUAXT_APP;

typedef struct script_job {
    int id;
    pid_t pid;
    int fd;
    XtInputId input_id;
    XtIntervalId poll_id;
    int running;
    int stop_requested;
    int partial_emitted;
    int exit_code;
    int signal_num;
    char *event_fn;
    char *buf;
    size_t len;
    size_t cap;
} script_job_t;

static script_job_t **jobs = NULL;
static int job_count = 0;
static int next_id = 1;

static script_job_t *find_job(int id)
{
    for (int i = 0; i < job_count; i++) {
        if (jobs[i] && jobs[i]->id == id) return jobs[i];
    }
    return NULL;
}

static int add_job(script_job_t *job)
{
    script_job_t **p = realloc(jobs, sizeof(*jobs) * (size_t)(job_count + 1));
    if (!p) return 0;
    jobs = p;
    jobs[job_count++] = job;
    return 1;
}

static void remove_job(script_job_t *job)
{
    for (int i = 0; i < job_count; i++) {
        if (jobs[i] == job) {
            jobs[i] = jobs[job_count - 1];
            job_count--;
            return;
        }
    }
}

static void emit_event(script_job_t *job, const char *kind,
                       const char *text, int code, int signal_num)
{
    lua_State *L = L_GLOBAL;
    if (!L || !job || !job->event_fn) return;

    lua_getglobal(L, job->event_fn);
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    lua_newtable(L);
    lua_pushinteger(L, job->id);
    lua_setfield(L, -2, "job");
    lua_pushstring(L, kind);
    lua_setfield(L, -2, "kind");
    if (text) {
        lua_pushstring(L, text);
        lua_setfield(L, -2, "text");
    }
    if (code >= 0) {
        lua_pushinteger(L, code);
        lua_setfield(L, -2, "code");
    }
    if (signal_num > 0) {
        lua_pushinteger(L, signal_num);
        lua_setfield(L, -2, "signal");
    }

    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        fprintf(stderr, "script_process: Lua event error: %s\n",
                lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static void job_free(script_job_t *job)
{
    if (!job) return;
    if (job->poll_id) {
        XtRemoveTimeOut(job->poll_id);
        job->poll_id = 0;
    }
    if (job->input_id) {
        XtRemoveInput(job->input_id);
        job->input_id = 0;
    }
    if (job->fd >= 0) {
        close(job->fd);
        job->fd = -1;
    }
    free(job->event_fn);
    free(job->buf);
    remove_job(job);
    free(job);
}

static void job_finish(script_job_t *job)
{
    if (!job) return;

    int status = 0;
    pid_t r = waitpid(job->pid, &status, WNOHANG);
    if (r == 0) {
        /* The child is still alive; poll_cb will reap it. */
        if (job->fd >= 0) {
            if (job->input_id) {
                XtRemoveInput(job->input_id);
                job->input_id = 0;
            }
            close(job->fd);
            job->fd = -1;
        }
        return;
    }

    job->running = 0;
    if (r == job->pid) {
        if (WIFEXITED(status)) job->exit_code = WEXITSTATUS(status);
        if (WIFSIGNALED(status)) job->signal_num = WTERMSIG(status);
    }
    emit_event(job, "exit", NULL,
               r == job->pid ? job->exit_code : -1,
               r == job->pid ? job->signal_num : 0);
    job_free(job);
}

static int buffer_ends_with_question(const char *buf, size_t len)
{
    size_t i = len;
    while (i > 0) {
        char c = buf[i - 1];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            i--;
            continue;
        }
        return c == '?';
    }
    return 0;
}

static void process_buffer(script_job_t *job)
{
    while (job->len > 0) {
        size_t sep = 0;
        int found = 0;
        for (size_t i = 0; i < job->len; i++) {
            if (job->buf[i] == '\n' || job->buf[i] == '\r') {
                sep = i;
                found = 1;
                break;
            }
        }
        if (!found) break;

        size_t line_len = sep;
        char *line = malloc(line_len + 1);
        if (line) {
            memcpy(line, job->buf, line_len);
            line[line_len] = 0;
            emit_event(job, "line", line, -1, 0);
            free(line);
        }
        size_t consumed = sep + 1;
        if (job->buf[sep] == '\r' && consumed < job->len &&
            job->buf[consumed] == '\n') {
            consumed++;
        }
        memmove(job->buf, job->buf + consumed, job->len - consumed);
        job->len -= consumed;
    }

    /* A prompt such as "Continue (Y/n)? " has no trailing newline. */
    if (job->len > 0 && !job->partial_emitted &&
        buffer_ends_with_question(job->buf, job->len)) {
        char *line = malloc(job->len + 1);
        if (line) {
            memcpy(line, job->buf, job->len);
            line[job->len] = 0;
            emit_event(job, "partial", line, -1, 0);
            free(line);
        }
        job->len = 0;
        job->partial_emitted = 0;
    }
}

static void read_cb(XtPointer client_data, int *source, XtInputId *id)
{
    (void)id;
    script_job_t *job = (script_job_t *)client_data;
    if (!job || !job->running || job->fd < 0) return;

    char chunk[4096];
    ssize_t n = read(*source, chunk, sizeof(chunk));
    if (n > 0) {
        size_t need = job->len + (size_t)n;
        if (need > job->cap) {
            size_t cap = job->cap ? job->cap : 4096;
            while (cap < need) cap *= 2;
            char *p = realloc(job->buf, cap);
            if (!p) return;
            job->buf = p;
            job->cap = cap;
        }
        memcpy(job->buf + job->len, chunk, (size_t)n);
        job->len += (size_t)n;
        job->partial_emitted = 0;
        process_buffer(job);
        return;
    }

    if (n == 0) {
        /* Flush a non-prompt remainder, then stop reading. */
        if (job->len > 0) {
            char *line = malloc(job->len + 1);
            if (line) {
                memcpy(line, job->buf, job->len);
                line[job->len] = 0;
                emit_event(job, "line", line, -1, 0);
                free(line);
            }
            job->len = 0;
        }
        if (job->input_id) {
            XtRemoveInput(job->input_id);
            job->input_id = 0;
        }
        if (job->fd >= 0) {
            close(job->fd);
            job->fd = -1;
        }
        job_finish(job);
        return;
    }

    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
        emit_event(job, "error", strerror(errno), -1, 0);
        job_finish(job);
    }
}

static void poll_cb(XtPointer client_data, XtIntervalId *id)
{
    (void)id;
    script_job_t *job = (script_job_t *)client_data;
    if (!job || !job->running) return;

    job->poll_id = 0;

    /* While the pty is still open, let read_cb drain pending output and see
     * EOF.  Reaping here would race with the last lines the child wrote just
     * before exiting. */
    if (job->fd >= 0) {
        job->poll_id = XtAppAddTimeOut(LUAXT_APP, 200, poll_cb, job);
        return;
    }

    int status = 0;
    pid_t r = waitpid(job->pid, &status, WNOHANG);
    if (r == job->pid) {
        job->running = 0;
        if (WIFEXITED(status)) job->exit_code = WEXITSTATUS(status);
        if (WIFSIGNALED(status)) job->signal_num = WTERMSIG(status);
        emit_event(job, "exit", NULL, job->exit_code, job->signal_num);
        job_free(job);
        return;
    }

    job->poll_id = XtAppAddTimeOut(LUAXT_APP, 200, poll_cb, job);
}

static char **argv_from_lua(lua_State *L, int index, int *argc_out)
{
    if (lua_istable(L, index)) {
        int n = (int)luaL_len(L, index);
        char **argv = calloc((size_t)n + 1, sizeof(char *));
        if (!argv) return NULL;
        for (int i = 1; i <= n; i++) {
            lua_rawgeti(L, index, i);
            const char *s = lua_tostring(L, -1);
            argv[i - 1] = strdup(s ? s : "");
            lua_pop(L, 1);
            if (!argv[i - 1]) {
                for (int j = 0; j < i - 1; j++) free(argv[j]);
                free(argv);
                return NULL;
            }
        }
        argv[n] = NULL;
        *argc_out = n;
        return argv;
    }

    if (lua_isstring(L, index)) {
        const char *cmd = lua_tostring(L, index);
        char **argv = calloc(4, sizeof(char *));
        if (!argv) return NULL;
        argv[0] = strdup("/bin/sh");
        argv[1] = strdup("-c");
        argv[2] = strdup(cmd);
        argv[3] = NULL;
        *argc_out = 3;
        return argv;
    }

    return NULL;
}

static int l_script_start(lua_State *L)
{
    if (lua_gettop(L) < 1) {
        return luaL_error(L, "script_start requires argv or command");
    }

    int argc = 0;
    char **argv = argv_from_lua(L, 1, &argc);
    if (!argv || argc < 1) {
        if (argv) free(argv);
        return luaL_error(L, "script_start: empty argv");
    }

    const char *on_event = NULL;
    const char *cwd = NULL;
    const char *term = NULL;
    int cols = 120;
    int rows = 40;

    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "on_event");
        if (lua_isstring(L, -1)) on_event = lua_tostring(L, -1);
        lua_getfield(L, 2, "cwd");
        if (lua_isstring(L, -1)) cwd = lua_tostring(L, -1);
        lua_getfield(L, 2, "term");
        if (lua_isstring(L, -1)) term = lua_tostring(L, -1);
        lua_getfield(L, 2, "cols");
        if (lua_isnumber(L, -1)) cols = (int)lua_tointeger(L, -1);
        lua_getfield(L, 2, "rows");
        if (lua_isnumber(L, -1)) rows = (int)lua_tointeger(L, -1);
        lua_pop(L, 5);
    }

    if (!on_event) {
        for (int i = 0; i < argc; i++) free(argv[i]);
        free(argv);
        return luaL_error(L, "script_start: opts.on_event is required");
    }

    char *event_copy = strdup(on_event);
    char *cwd_copy = cwd ? strdup(cwd) : NULL;
    char *term_copy = term ? strdup(term) : NULL;

    struct winsize ws;
    memset(&ws, 0, sizeof(ws));
    ws.ws_col = (unsigned short)(cols > 0 ? cols : 120);
    ws.ws_row = (unsigned short)(rows > 0 ? rows : 40);

    int master = -1;
    pid_t pid = forkpty(&master, NULL, NULL, &ws);
    if (pid < 0) {
        int e = errno;
        for (int i = 0; i < argc; i++) free(argv[i]);
        free(argv);
        free(event_copy);
        free(cwd_copy);
        free(term_copy);
        lua_pushnil(L);
        lua_pushstring(L, strerror(e));
        return 2;
    }

    if (pid == 0) {
        if (cwd_copy && chdir(cwd_copy) != 0) {
            perror("script_process: chdir");
            _exit(127);
        }
        setenv("TERM", term_copy ? term_copy : "dumb", 1);
        execvp(argv[0], argv);
        perror("script_process: execvp");
        _exit(127);
    }

    for (int i = 0; i < argc; i++) free(argv[i]);
    free(argv);
    free(cwd_copy);
    free(term_copy);

    int flags = fcntl(master, F_GETFL, 0);
    if (flags >= 0) fcntl(master, F_SETFL, flags | O_NONBLOCK);

    script_job_t *job = calloc(1, sizeof(*job));
    if (!job) {
        close(master);
        kill(pid, SIGTERM);
        free(event_copy);
        lua_pushnil(L);
        lua_pushstring(L, "out of memory");
        return 2;
    }

    job->id = next_id++;
    job->pid = pid;
    job->fd = master;
    job->running = 1;
    job->event_fn = event_copy;
    job->cap = 4096;
    job->buf = malloc(job->cap);
    job->len = 0;
    if (!job->buf) {
        close(master);
        kill(pid, SIGTERM);
        free(event_copy);
        free(job);
        lua_pushnil(L);
        lua_pushstring(L, "out of memory");
        return 2;
    }

    if (!add_job(job)) {
        close(master);
        kill(pid, SIGTERM);
        free(event_copy);
        free(job->buf);
        free(job);
        lua_pushnil(L);
        lua_pushstring(L, "out of memory");
        return 2;
    }
    job->input_id = XtAppAddInput(LUAXT_APP, master,
                                  (XtPointer)XtInputReadMask,
                                  read_cb, job);
    job->poll_id = XtAppAddTimeOut(LUAXT_APP, 200, poll_cb, job);

    lua_pushinteger(L, job->id);
    return 1;
}

static int l_script_write(lua_State *L)
{
    int id = (int)luaL_checkinteger(L, 1);
    const char *text = luaL_checkstring(L, 2);
    script_job_t *job = find_job(id);
    if (!job || !job->running || job->fd < 0) {
        lua_pushnil(L);
        lua_pushstring(L, "script is not running");
        return 2;
    }

    size_t len = strlen(text);
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(job->fd, text + off, len - off);
        if (n > 0) {
            off += (size_t)n;
        } else if (n < 0 && errno == EINTR) {
            continue;
        } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            usleep(1000);
            continue;
        } else {
            lua_pushnil(L);
            lua_pushstring(L, strerror(errno));
            return 2;
        }
    }

    lua_pushboolean(L, 1);
    return 1;
}

static int l_script_stop(lua_State *L)
{
    int id = (int)luaL_checkinteger(L, 1);
    script_job_t *job = find_job(id);
    if (!job || !job->running) {
        lua_pushboolean(L, 1);
        return 1;
    }
    job->stop_requested = 1;
    kill(job->pid, SIGTERM);
    lua_pushboolean(L, 1);
    return 1;
}

static int l_script_is_running(lua_State *L)
{
    int id = (int)luaL_checkinteger(L, 1);
    script_job_t *job = find_job(id);
    lua_pushboolean(L, job && job->running);
    return 1;
}

void script_process_cleanup(void)
{
    while (job_count > 0) {
        script_job_t *job = jobs[0];
        if (job && job->running && job->pid > 0) {
            kill(job->pid, SIGTERM);
        }
        job_free(job);
    }
    free(jobs);
    jobs = NULL;
    job_count = 0;
}

void script_process_register(lua_State *L)
{
    signal(SIGPIPE, SIG_IGN);
    lua_register(L, "script_start", l_script_start);
    lua_register(L, "script_write", l_script_write);
    lua_register(L, "script_stop", l_script_stop);
    lua_register(L, "script_is_running", l_script_is_running);
}
