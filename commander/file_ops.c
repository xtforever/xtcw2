#include "task_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

typedef struct {
    char src[4096];
    char dst[4096];
} copy_args_t;

void *copy_task(task_thread_args_t *targs) {
    copy_args_t *args = (copy_args_t *)targs->user_arg;
    int job_id = targs->job_id;
    
    int sfd = open(args->src, O_RDONLY);
    if (sfd < 0) {
        task_report(job_id, TASK_EVENT_ERROR, 0, "Source error: %s", strerror(errno));
        goto cleanup;
    }
    
    struct stat st;
    fstat(sfd, &st);
    off_t total_size = st.st_size;
    off_t copied = 0;
    
    int dfd = open(args->dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dfd < 0) {
        task_report(job_id, TASK_EVENT_ERROR, 0, "Dest error: %s", strerror(errno));
        close(sfd);
        goto cleanup;
    }
    
    char buf[65536];
    ssize_t n;
    int paused = 0;
    
    while ((n = read(sfd, buf, sizeof(buf))) > 0) {
        /* Check for commands */
        task_cmd_t cmd = task_check_command(targs);
        while (cmd == TASK_CMD_STOP || paused) {
            paused = 1;
            if (cmd == TASK_CMD_CONT) { paused = 0; break; }
            if (cmd == TASK_CMD_ABORT) goto aborted;
            usleep(100000);
            cmd = task_check_command(targs);
        }
        if (cmd == TASK_CMD_ABORT) goto aborted;

        if (write(dfd, buf, n) != n) {
            task_report(job_id, TASK_EVENT_ERROR, 0, "Write error: %s", strerror(errno));
            break;
        }
        
        copied += n;
        float progress = total_size > 0 ? (float)copied * 100.0 / total_size : 100.0;
        task_report(job_id, TASK_EVENT_PROGRESS, progress, "Copying %s...", args->src);
    }
    
    if (n == 0) {
        task_report(job_id, TASK_EVENT_COMPLETE, 100.0, "Copy complete");
    }

    close(sfd);
    close(dfd);
    goto cleanup;

aborted:
    task_report(job_id, TASK_EVENT_ERROR, 0, "Aborted");
    close(sfd);
    close(dfd);
    unlink(args->dst);

cleanup:
    free(args);
    free(targs);
    return NULL;
}
