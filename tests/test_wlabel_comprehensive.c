#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wlabel.h>
#include <xtcw/VBox.h>
#include <xtcw/register_wb.h>
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>

static Widget label_dynamic;
static int step = 0;

static void timeout_cb(XtPointer client_data, XtIntervalId *id) {
    XtAppContext app = (XtAppContext)client_data;
    
    step++;
    switch(step) {
        case 1:
            printf("--- Step 1: Reformatting label text ---\n");
            XtVaSetValues(label_dynamic, XtNlabel, "Updated Label: This is a much longer text to trigger re-layout and possibly line wrapping if width was restricted.", NULL);
            XtAppAddTimeOut(app, 500, timeout_cb, app);
            break;
        case 2:
            printf("--- Step 2: Changing font size and face ---\n");
            XtVaSetValues(label_dynamic, "fontSize", 20, "fontFace", "Serif", NULL);
            XtAppAddTimeOut(app, 500, timeout_cb, app);
            break;
        case 3:
            printf("--- Step 3: Changing colors and enabling reverse mode ---\n");
            XtVaSetValues(label_dynamic, XtNforeground, "yellow", XtNbackground, "blue", "reverse_mode", True, NULL);
            XtAppAddTimeOut(app, 500, timeout_cb, app);
            break;
        case 4:
            printf("--- Step 4: Testing Selection ---\n");
            XtVaSetValues(label_dynamic, "selection_start", 0, "selection_end", 15, "bg_sel", "red", "fg_sel", "white", NULL);
            XtAppAddTimeOut(app, 500, timeout_cb, app);
            break;
        default:
            printf("--- Comprehensive Test Completed ---\n");
            XtAppSetExitFlag(app);
            break;
    }
}

int main(int argc, char **argv) {
    m_init();
    conststr_init();
    extern int trace_level;
    trace_level = 2;

    XtAppContext app;
    Widget top = XtOpenApplication(&app, "TestWlabelComprehensive", NULL, 0, &argc, argv, NULL, sessionShellWidgetClass, NULL, 0);

    XtcwRegister(app);

    Widget vbox = XtVaCreateManagedWidget("vbox", vBoxWidgetClass, top, NULL);

    // 1. Font Faces and Sizes
    XtVaCreateManagedWidget("label_fonts", wlabelWidgetClass, vbox,
                            XtNlabel, "Sans 12pt, \\fontface{Serif}{Serif 12pt}, \\fontface{Monospace}{Mono 12pt}",
                            "fontSize", 12,
                            NULL);

    XtVaCreateManagedWidget("label_sizes", wlabelWidgetClass, vbox,
                            XtNlabel, "Tiny (8pt) to \\huge{Huge (24pt)}",
                            "fontSize", 12,
                            NULL);

    // 2. Styles and Math
    XtVaCreateManagedWidget("label_styles", wlabelWidgetClass, vbox,
                            XtNlabel, "Styles: \\bf{Bold}, \\it{Italic}, \\bf{\\it{BoldItalic}}, Math: $E=mc^2$",
                            "fontSize", 14,
                            NULL);

    // 3. Colors and Initial Selection
    XtVaCreateManagedWidget("label_colors", wlabelWidgetClass, vbox,
                            XtNlabel, "White on Green with Blue Selection",
                            XtNforeground, "white",
                            XtNbackground, "green",
                            "bg_sel", "blue",
                            "fg_sel", "yellow",
                            "selection_start", 9,
                            "selection_end", 14,
                            NULL);

    // 4. Dynamic Widget (for Step tests)
    label_dynamic = XtVaCreateManagedWidget("label_dynamic", wlabelWidgetClass, vbox,
                            XtNlabel, "Initial Dynamic Label",
                            "autoHeight", True,
                            NULL);

    XtRealizeWidget(top);

    // Start steps
    XtAppAddTimeOut(app, 1000, timeout_cb, app);

    XtAppMainLoop(app);
    XtDestroyWidget(top);
    XtDestroyApplicationContext(app);
    conststr_free();
    m_destruct();
    return 0;
}
