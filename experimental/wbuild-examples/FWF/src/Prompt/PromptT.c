#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/Prompt.h>

static XtAppContext app_context;
static Widget toplevel, board, tester1, tester2, tester3;
static String fallback_resources[] = {
    "*board.width:		500",
    "*board.height:		250",
    "*board*background:		aquamarine",

    "*tester1.location:		10 10 1.0-20 0.50-20",
    "*tester1.template: \
Enter a date: __-__-__ (DD-MM-YY)\\n\
and a place:  __________________",
    "*tester1.pattern: \
99-99-99 (DD-MM-YY)\\n\
and a place:  XXXXXXXXXXXXXXXXXX",
    "*tester1.translations:	#override\\n<Key>Return: traverseNext()",

    "*tester2.location:		10 0.50+10 1.0-20 0.25-20",
    "*tester2.template:		Enter a file name: ______________",
    "*tester2.pattern:		                   FFFFFFFFFFFFFFF",
    "*tester2.translations:	#override\\n<Key>Return: traverseNext()",

    "*tester3.location:		10 0.75+10 1.0-20 0.25-20",
    "*tester3.text:		Edit this; <Return> here will exit",
    "*tester3.foreground:	yellow",

    NULL,
};


static void print_out(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    String t = (String) call_data;

    (void) printf("\n*** You left <%s> with the text:\n%s\n", XtName(w), t);
}


static void quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    exit(0);
}


int main(argc, argv)
    int argc;
    char *argv[];
{
    toplevel = XtVaAppInitialize(&app_context, "PromptT", NULL, 0,
				  &argc, argv, fallback_resources, NULL);
    board = XtVaCreateManagedWidget("board", xfwfBoardWidgetClass,
				    toplevel, NULL);
    tester1 = XtVaCreateManagedWidget("tester1", xfwfPromptWidgetClass,
				      board, NULL);
    tester2 = XtVaCreateManagedWidget("tester2", xfwfPromptWidgetClass,
				      board, NULL);
    tester3 = XtVaCreateManagedWidget("tester3", xfwfPromptWidgetClass,
				      board, NULL);
    XtAddCallback(tester1, XtNleave, print_out, NULL);
    XtAddCallback(tester2, XtNleave, print_out, NULL);
    XtAddCallback(tester3, XtNleave, print_out, NULL);
    XtAddCallback(tester3, XtNactivate, quit, NULL);
    XtRealizeWidget(toplevel);
    XSync(XtDisplay(toplevel), False);
    XtAppMainLoop(app_context);
}
