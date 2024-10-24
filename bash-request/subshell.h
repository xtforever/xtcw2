#ifndef SUBSHELL_H
#define SUBSHELL_H

#include <sys/wait.h>
#include "mrb.h"

enum fork2_proc_state {
    CHILD_NOT_INIT,
    CHILD_RUNNING,
    CHILD_EXIT_SUCCESS,
    CHILD_EXIT_FAILURE
};

enum fork2_pipes {
    CHILD_STDOUT_RD=0,
    CHILD_STDERR_RD=4,
    CHILD_STDIN_WR=3
};


struct fork2_info {
    int init;
    int fd[6];
    pid_t pid;
    int exit_value, sigchld;
    int flags;
    struct mrb *pipe_buf[2];
    enum fork2_proc_state stat;
    int error, last_read;
    int scan_pos[2];
    int masterfd;
};

extern int trace_child;

struct fork2_info *fork2_open(char *filename,...);
struct fork2_info *fork2_open2( int args );
void fork2_close( struct fork2_info *child );
int fork2_getline( struct fork2_info *child, int pipe, int lnbuf );
int fork2_read(struct fork2_info *child, int pipe );
int fork2_getchar(struct fork2_info *child, int pipe );
int fork2_write(struct fork2_info *child, char *s );
void fork2_open3( struct fork2_info *child, int args );
void fork2_close3( struct fork2_info *child );

int  shell_create( int args );
int  shell_create1( int filename );
void shell_close( int n );
int  shell_getline(int h, int p, int buf);
int  shell_signal(int pid, int exit_value);
void shell_signal_cb(int dum);
int  shell_write(int h, char *msg);
struct mrb *shell_queue(int h, int p);
int shell_fd(int h, int n);

#endif
