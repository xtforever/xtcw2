#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>
#include <X11/Shell.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

/* Global pipe file descriptors */
int pipe_fds[2];
Widget label_widget;

/* Thread function that simulates a long-running task */
void *worker_thread(void *arg) {
    char buf[64];
    for (int i = 0; i <= 100; i += 10) {
        snprintf(buf, sizeof(buf), "Progress: %d%%", i);
        write(pipe_fds[1], buf, strlen(buf) + 1);
        sleep(1);
    }
    write(pipe_fds[1], "DONE", 5);
    return NULL;
}

/* Xt callback triggered when data is available on the pipe */
void pipe_input_cb(XtPointer client_data, int *source, XtInputId *id) {
    char buf[64];
    int n = read(*source, buf, sizeof(buf));
    if (n > 0) {
        printf("Received from thread: %s\n", buf);
        Arg args[1];
        XtSetArg(args[0], XtNlabel, buf);
        XtSetValues(label_widget, args, 1);
        
        if (strcmp(buf, "DONE") == 0) {
            printf("Task complete. Cleaning up.\n");
            XtRemoveInput(*id);
            close(pipe_fds[0]);
            close(pipe_fds[1]);
        }
    }
}

int main(int argc, char **argv) {
    XtAppContext app;
    Widget toplevel;

    /* Required for multi-threaded X applications */
    XInitThreads();

    toplevel = XtOpenApplication(&app, "ThreadPipeTest", NULL, 0, &argc, argv, NULL,
                                 sessionShellWidgetClass, NULL, 0);

    label_widget = XtVaCreateManagedWidget("label", labelWidgetClass, toplevel,
                                           XtNlabel, "Waiting for thread...",
                                           XtNwidth, 200,
                                           XtNheight, 50,
                                           NULL);

    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        exit(1);
    }

    /* Register the read-end of the pipe with the Xt event loop */
    XtAppAddInput(app, pipe_fds[0], (XtPointer)XtInputReadMask, pipe_input_cb, NULL);

    XtRealizeWidget(toplevel);

    /* Spawn the worker thread */
    pthread_t thread;
    if (pthread_create(&thread, NULL, worker_thread, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }
    pthread_detach(thread);

    XtAppMainLoop(app);

    return 0;
}
