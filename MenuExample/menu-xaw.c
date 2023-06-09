#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <stdlib.h>

#include "xtcw/SmeLine2.h"

void quitCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    exit(0);  // Exit the program when the Quit option is selected
}

int main(int argc, char** argv)
{
    XtAppContext app_context;
    Widget topLevel, menuButton, menuPane, quitOption;

    topLevel = XtVaAppInitialize(&app_context, "SimpleMenuExample", NULL, 0, &argc, argv, NULL, NULL);



    // Create the menu pane
    menuPane = XtVaCreatePopupShell("menu", simpleMenuWidgetClass, topLevel, NULL);

    // create a line
    XtVaCreateManagedWidget("testOption", smeBSBObjectClass, menuPane, XtNlabel, "test", NULL);
    XtVaCreateManagedWidget("sep", smeLine2WidgetClass, menuPane, NULL);

    // Create the Quit option
    quitOption = XtVaCreateManagedWidget("quitOption", smeBSBObjectClass, menuPane, XtNlabel, "Quit", NULL);
    XtAddCallback(quitOption, XtNcallback, quitCallback, NULL);

    // Create the menu button and associate it with the menu pane
    menuButton = XtVaCreateManagedWidget("menuButton", menuButtonWidgetClass, topLevel,
                                          XtNmenuName, "menu", NULL);
    (void) menuButton ;


    XtRealizeWidget(topLevel);
    XtAppMainLoop(app_context);

    return 0;
}
