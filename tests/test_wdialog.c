/* Functional test for WDialog: title/message resources, open/close, result,
 * showCancel toggling, details/checkbox and the input row. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <xtcw/WDialog.h>
#include <xtcw/WDialogP.h>
#include <xtcw/register_wb.h>
#include "mls.h"

static XtAppContext app;

static void pump(Widget w)
{
    int i;
    for (i = 0; i < 20; i++) {
        XSync(XtDisplay(w), False);
        while (XtAppPending(app))
            XtAppProcessEvent(app, XtIMAll);
        usleep(1000);
    }
}

static void send_key(Widget w, KeySym ks)
{
    XKeyEvent ev;
    memset(&ev, 0, sizeof ev);
    ev.type = KeyPress;
    ev.display = XtDisplay(w);
    ev.window = XtWindow(w);
    ev.root = DefaultRootWindow(ev.display);
    ev.subwindow = None;
    ev.time = CurrentTime;
    ev.x = 1; ev.y = 1;
    ev.x_root = 1; ev.y_root = 1;
    ev.same_screen = True;
    ev.keycode = XKeysymToKeycode(ev.display, ks);
    ev.state = 0;
    XSendEvent(ev.display, ev.window, True, KeyPressMask, (XEvent *)&ev);
    XFlush(ev.display);
    pump(w);
}

static int open_state(Widget d)
{
    Boolean open = False;
    XtVaGetValues(d, "open", &open, NULL);
    return open ? 1 : 0;
}

int main(int argc, char **argv)
{
    m_init();

    Widget top = XtOpenApplication(&app, "TestWDialog", NULL, 0,
                                   &argc, argv, NULL, sessionShellWidgetClass,
                                   NULL, 0);
    XtcwRegister(app);

    XtVaSetValues(top, XtNwidth, (Dimension)400, XtNheight, (Dimension)300, NULL);

    Widget dlg = XtVaCreateWidget("dlg", wDialogWidgetClass, top,
                                         XtNwidth, (Dimension)320,
                                         XtNheight, (Dimension)160,
                                         "title", "Delete recipe",
                                         "message", "Delete this recipe?",
                                         "okLabel", "Delete",
                                         "cancelLabel", "Cancel",
                                         "showCancel", False,
                                         "details", "This cannot be undone.",
                                         "checkboxLabel", "Also remove the photo",
                                         "checkboxChecked", True,
                                         "inputKind", 1,
                                         "inputValue", "Bob",
                                         NULL);
    XtRealizeWidget(top);
    pump(top);

    char *s = NULL;
    XtVaGetValues(dlg, "title", &s, NULL);
    assert(s && strcmp(s, "Delete recipe") == 0);
    XtVaGetValues(dlg, "message", &s, NULL);
    assert(s && strcmp(s, "Delete this recipe?") == 0);
    XtVaGetValues(dlg, "okLabel", &s, NULL);
    assert(s && strcmp(s, "Delete") == 0);

    WDialogWidget dw = (WDialogWidget)dlg;
    assert(!XtIsManaged(dw->wDialog.cancel_button));
    XtVaSetValues(dlg, "showCancel", True, NULL);
    assert(XtIsManaged(dw->wDialog.cancel_button));

    Boolean checked = False;
    XtVaGetValues(dw->wDialog.checkbox, "checked", &checked, NULL);
    assert(checked == True);
    assert(XtIsManaged(dw->wDialog.details_label));
    assert(XtIsManaged(dw->wDialog.field_box));
    assert(dw->wDialog.field != NULL);

    /* Open/OK path: result 1 and the input value round-trips. */
    assert(wdialog_is_open(dlg) == False);
    XtVaSetValues(dlg, "open", True, NULL);
    assert(wdialog_is_open(dlg) == True);
    assert(open_state(dlg) == 1);
    assert(XtGetKeyboardFocusWidget(dw->wDialog.shell) == dw->wDialog.field);

    /* Default button: Return in the focused input commits the dialog. */
    send_key(dw->wDialog.field, XK_Return);
    assert(wdialog_is_open(dlg) == False);
    assert(wdialog_result(dlg) == 1);
    XtVaGetValues(dlg, "inputValue", &s, NULL);
    assert(s && strcmp(s, "Bob") == 0);

    /* Escape cancels even while the input field has focus. */
    XtVaSetValues(dlg, "open", True, NULL);
    assert(wdialog_is_open(dlg) == True);
    send_key(dw->wDialog.field, XK_Escape);
    assert(wdialog_is_open(dlg) == False);
    assert(wdialog_result(dlg) == 0);

    /* Cancel action path. */
    XtVaSetValues(dlg, "open", True, NULL);
    XtCallActionProc(dlg, "cancel", NULL, NULL, 0);
    assert(wdialog_result(dlg) == 0);

    /* Input kind changes rebuild the field; int values round-trip. */
    XtVaSetValues(dlg, "inputKind", 2, "inputValue", "42", NULL);
    XtCallActionProc(dlg, "ok", NULL, NULL, 0);
    XtVaGetValues(dlg, "inputValue", &s, NULL);
    assert(s && strcmp(s, "42") == 0);

    /* Choice input. */
    XtVaSetValues(dlg, "inputKind", 4, "inputItems", "Small\nMedium\nLarge",
                  "inputValue", "Medium", NULL);
    XtCallActionProc(dlg, "ok", NULL, NULL, 0);
    XtVaGetValues(dlg, "inputValue", &s, NULL);
    assert(s && strcmp(s, "Medium") == 0);

    /* Third (aux) action: result 2 and it closes the dialog. */
    XtVaSetValues(dlg, "showAux", True, "auxLabel", "Cancel", NULL);
    assert(XtIsManaged(dw->wDialog.aux_button));
    XtVaSetValues(dlg, "open", True, NULL);
    XtCallActionProc(dlg, "aux", NULL, NULL, 0);
    assert(wdialog_is_open(dlg) == False);
    assert(wdialog_result(dlg) == 2);

    /* Hiding the input row must not crash. */
    XtVaSetValues(dlg, "inputKind", 0, NULL);
    assert(!XtIsManaged(dw->wDialog.field_box));

    printf("test_wdialog PASSED\n");
    return 0;
}
