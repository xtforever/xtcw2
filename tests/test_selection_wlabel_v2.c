#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <xtcw/Wlabel.h>
#include <xtcw/Wbutton.h>
#include <xtcw/register_wb.h>
#include <xtcw/Gridbox.h>
#include "mls.h"
#include "conststr.h"
#include <stdio.h>
#include <stdlib.h>

Widget label_wid;
Widget info_wid;

void update_info(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    int s, e;
    Arg args[2];
    XtSetArg(args[0], XtNselection_start, &s);
    XtSetArg(args[1], XtNselection_end, &e);
    XtGetValues(label_wid, args, 2);
    
    char buf[256];
    snprintf(buf, sizeof(buf), "Selection: %d to %d", s, e);
    XtVaSetValues(info_wid, XtNlabel, buf, NULL);
}

XtActionsRec actions[] = {
    {"update_info", (XtActionProc)update_info},
};

void quit_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    exit(0);
}

/* Callback for receiving the selection */
void selection_callback(Widget w, XtPointer client_data, Atom *selection, Atom *type, XtPointer value, unsigned long *length, int *format)
{
    if (*type == XA_STRING && value) {
        char *text = (char *)value;
        char *msg = malloc(*length + 64);
        sprintf(msg, "Pasted: %.*s", (int)*length, text);
        XtVaSetValues(info_wid, XtNlabel, msg, NULL);
        free(msg);
        XtFree(value);
    } else {
        XtVaSetValues(info_wid, XtNlabel, "Paste failed or no selection", NULL);
    }
}

void paste_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    XtGetSelectionValue(w, XA_PRIMARY, XA_STRING, selection_callback, NULL, XtLastTimestampProcessed(XtDisplay(w)));
}

int main(int argc, char **argv) {
    m_init();
    conststr_init();
    trace_level = 2;
    
    XtAppContext app;
    Widget top = XtOpenApplication(&app, "WlabelSelectionTest", NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL, 0);

    XtcwRegister(app);
    XtAppAddActions(app, actions, XtNumber(actions));

    Widget box = XtVaCreateManagedWidget("box", gridboxWidgetClass, top, NULL);

    const char *test_text = 
        "Interactive Selection Test.\n"
        "1. Click and drag to select text.\n"
        "2. The indices should update below (if polled or on click).\n"
        "3. Click 'Paste' to see what's in the PRIMARY selection.\n"
        "4. This widget uses re-tex for layout.";

    label_wid = XtVaCreateManagedWidget("label", wlabelWidgetClass, box,
                                           XtNlabel, test_text,
                                           "gridy", 0,
                                           "fill", "Both",
                                           "weightx", 1,
                                           "weighty", 1,
                                           "leftGap", 10, "rightGap", 10, "topGap", 10, "bottomGap", 10,
                                           "bg_norm", "gray80",
                                           NULL);

    XtOverrideTranslations(label_wid, XtParseTranslationTable("<Btn1Up>: select_end() update_info()"));

    info_wid = XtVaCreateManagedWidget("info", wlabelWidgetClass, box,
                                          XtNlabel, "Selection: none",
                                          "gridy", 1,
                                          "fill", "Width",
                                          "bg_norm", "gray90",
                                          NULL);

    Widget btn_box = XtVaCreateManagedWidget("btn_box", gridboxWidgetClass, box, 
                                            "gridy", 2, "fill", "Width", NULL);

    Widget paste_btn = XtVaCreateManagedWidget("paste", wbuttonWidgetClass, btn_box,
                                             XtNlabel, "Paste Selection",
                                             "gridx", 0,
                                             NULL);
    XtAddCallback(paste_btn, XtNcallback, paste_cb, NULL);

    Widget quit_btn = XtVaCreateManagedWidget("quit", wbuttonWidgetClass, btn_box,
                                            XtNlabel, "Quit",
                                            "gridx", 1,
                                            NULL);
    XtAddCallback(quit_btn, XtNcallback, quit_cb, NULL);

    XtRealizeWidget(top);
    XtAppMainLoop(app);

    return 0;
}
