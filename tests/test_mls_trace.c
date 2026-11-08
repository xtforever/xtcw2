/*
 * test_mls_trace.c - verify MLS_DEBUG trace level and post-mortem analysis.
 *
 * Scenario: allocate three lists, free the middle one, then access the
 * freed handle. With trace_level = 1 the library logs every allocation and
 * free together with the source line. When the invalid access aborts, the
 * post-mortem analyser must name
 *   - the call site that performed the failing operation, and
 *   - the source line that created the handle (and where it was freed).
 *
 * The whole scenario runs in a child process because the MLS debug build
 * calls exit(1) on the first error. The parent captures stderr and verifies
 * the expected messages, so this test is usable in the automated suite.
 *
 * Run with "--demo" to see the raw output in the current process instead
 * (this process will exit with status 1, which is the expected result).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mls.h"

extern int trace_level;

struct trace_meta {
    int handle;
    int created_line;
    int freed_line;
    int access_line;
};

static void write_meta(int fd, const struct trace_meta *meta)
{
    if (fd < 0) /* --demo mode: no parent to report to */
        return;

    const char *p = (const char *)meta;
    size_t left = sizeof(*meta);
    while (left > 0) {
        ssize_t n = write(fd, p, left);
        if (n <= 0)
            return;
        p += n;
        left -= (size_t)n;
    }
}

/*
 * Allocate a, b, c, free b and access it. Never returns normally: the
 * invalid access is reported by MLS_DEBUG and terminates the process.
 */
static void trigger_error(int meta_fd)
{
    struct trace_meta meta;

    m_init();
    trace_level = 1; /* show every NEW LIST / FREE LIST message */

    int a = m_alloc(10, sizeof(int), MFREE);
    int b = m_alloc(10, sizeof(int), MFREE); meta.created_line = __LINE__;
    int c = m_alloc(10, sizeof(int), MFREE);

    int value = 4711;
    m_put(a, &value);
    m_put(b, &value);
    m_put(c, &value);

    m_free(b); meta.freed_line = __LINE__;
    meta.handle = b & 0xffffff;

    /* The invalid access is two physical lines below (write_meta is
       in between). */
    meta.access_line = __LINE__ + 2;
    write_meta(meta_fd, &meta);
    (void) mls(b, 0); /* use-after-free: aborts via MLS_DEBUG */

    /* Not reached. */
    m_free(a);
    m_free(c);
    m_destruct();
}

static char *read_file(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp)
        return NULL;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    long size = ftell(fp);
    if (size < 0 || fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }

    char *buf = malloc((size_t)size + 1);
    if (!buf) {
        fclose(fp);
        return NULL;
    }
    if (fread(buf, 1, (size_t)size, fp) != (size_t)size) {
        free(buf);
        fclose(fp);
        return NULL;
    }
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

static int expect(const char *output, const char *needle, const char *what)
{
    if (strstr(output, needle))
        return 0;
    fprintf(stderr, "FAIL: missing %s\n  expected: %s\n", what, needle);
    return 1;
}

int main(int argc, char **argv)
{
#ifndef MLS_DEBUG
    (void)argc;
    (void)argv;
    printf("test_mls_trace: skipped (built without -DMLS_DEBUG)\n");
    return 0;
#else
    if (argc > 1 && strcmp(argv[1], "--demo") == 0) {
        /* Show the real output. MLS_DEBUG will exit(1) on the bad access. */
        trigger_error(-1);
        return 0;
    }

    char path[] = "/tmp/mls_trace_XXXXXX";
    int out_fd = mkstemp(path);
    if (out_fd < 0) {
        perror("mkstemp");
        return 2;
    }

    int meta_pipe[2];
    if (pipe(meta_pipe) != 0) {
        perror("pipe");
        close(out_fd);
        unlink(path);
        return 2;
    }

    fflush(stdout);
    fflush(stderr);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(out_fd);
        close(meta_pipe[0]);
        close(meta_pipe[1]);
        unlink(path);
        return 2;
    }

    if (pid == 0) {
        close(meta_pipe[0]);
        if (dup2(out_fd, STDERR_FILENO) < 0)
            _exit(2);
        close(out_fd);
        trigger_error(meta_pipe[1]);
        _exit(0); /* only reached if the error is not detected */
    }

    close(meta_pipe[1]);
    close(out_fd);

    struct trace_meta meta;
    memset(&meta, 0, sizeof(meta));
    ssize_t got = read(meta_pipe[0], &meta, sizeof(meta));
    close(meta_pipe[0]);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        unlink(path);
        return 2;
    }

    char *output = read_file(path);
    unlink(path);
    if (!output) {
        perror("read output");
        return 2;
    }

    char expected[1024];
    int failures = 0;

    if (got != (ssize_t)sizeof(meta)) {
        fprintf(stderr, "FAIL: child did not send metadata\n");
        failures++;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 1) {
        fprintf(stderr, "FAIL: expected exit status 1 from MLS_DEBUG, got %d\n",
                WIFEXITED(status) ? WEXITSTATUS(status) : -1);
        failures++;
    }

    /* trace_level = 1 must show allocation and free with source location */
    snprintf(expected, sizeof(expected),
             "NEW LIST %d allocated by trigger_error:%d in %s",
             meta.handle, meta.created_line, __FILE__);
    failures += expect(output, expected, "allocation trace");

    snprintf(expected, sizeof(expected),
             "FREE LIST %d freed by trigger_error:%d in %s",
             meta.handle, meta.freed_line, __FILE__);
    failures += expect(output, expected, "free trace");

    /* post-mortem must name the failing call site ... */
    snprintf(expected, sizeof(expected),
             "POST MORTEM ANALYSER STARTED");
    failures += expect(output, expected, "post-mortem header");

    snprintf(expected, sizeof(expected),
             "ERROR in function: '_mls'. Handle: %d. "
             "Called by 'trigger_error()' in '%s:%d'",
             meta.handle, __FILE__, meta.access_line);
    failures += expect(output, expected, "failing call site");

    /* ... and the source line that created the handle, plus the free site */
    snprintf(expected, sizeof(expected),
             "Array was created by trigger_error() in %s:%d",
             __FILE__, meta.created_line);
    failures += expect(output, expected, "handle creation site");

    snprintf(expected, sizeof(expected),
             "Array was previously freed by trigger_error() in %s:%d",
             __FILE__, meta.freed_line);
    failures += expect(output, expected, "handle free site");

    if (failures) {
        fprintf(stderr, "\n--- captured MLS output ---\n%s\n--- end ---\n",
                output);
        free(output);
        return 1;
    }

    printf("test_mls_trace passed (handle %d created at %s:%d, "
           "freed at :%d, invalid access at :%d)\n",
           meta.handle, __FILE__, meta.created_line, meta.freed_line,
           meta.access_line);
    free(output);
    return 0;
#endif
}
