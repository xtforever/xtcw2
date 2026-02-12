#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <WcCreate.h>
#include <register_wb.h>
#include <mls.h>
#include <xutil.h>
#include <stdio.h>
#include <stdlib.h>

static void input_cb(Widget w, XtPointer client_data, XtPointer call_data) {
    char *input = NULL;
    XtVaGetValues(w, "label", &input, NULL);
    printf("Input received: %s\n", input ? input : "NULL");

    Widget greeting = WcFullNameToWidget(w, "*greeting");
    if (greeting) {
        char buf[1024];
        snprintf(buf, sizeof(buf), "Hello, %s! Welcome to the XTCW toolkit.", input ? input : "Stranger");
        XtVaSetValues(greeting, "label", buf, NULL);
        XtManageChild(greeting);
    } else {
        fprintf(stderr, "Could not find greeting widget\n");
    }
}

static void quit_cb(Widget w, XtPointer client_data, XtPointer call_data) {
    exit(0);
}

static void destroy_done_cb(Widget w, XtPointer client_data, XtPointer call_data) {
    *(int*)client_data = 1;
}

int main(int argc, char **argv)
{
    XtAppContext app;
    Widget top;

    m_init();
    void conststr_init(void);
    conststr_init();
    trace_level = 1;

    top = XtOpenApplication(&app, "InputTest", NULL, 0, &argc, argv, 
                            NULL,
                            applicationShellWidgetClass, NULL, 0);

    XtcwRegister(app);
    WcRegisterCallback(app, "input_cb", input_cb, NULL);
    WcRegisterCallback(app, "quit_cb", quit_cb, NULL);
    WcWidgetCreation(top);

    XtRealizeWidget(top);
    grab_window_quit(top);

    XtAppMainLoop(app);

    int destroyed = 0;
    XtAddCallback(top, XtNdestroyCallback, destroy_done_cb, &destroyed);
    XtDestroyWidget(top);
    while (!destroyed) {
        XtAppProcessEvent(app, XtIMAll);
    }

    void conststr_free(void);
    conststr_free();
    m_destroy();
    return 0;
}
