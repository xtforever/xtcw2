# Alert.w -- Alert widget
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 1.2 (24 Feb '94)

@class XfwfAlert (TransientShell) @file=Alert

@ The XfwfAlert widget is a pop-up dialog box with three parts: an
icon in the top left corner, a text area, and a row of buttons along
the bottom. Usually the alert widget is used for error messages,
warnings and other kinds of informational messages. The icon indicates
the type of message: (fatal) error, warning, other message, request
for confirmation, etc. There are some built-in icons for these types.
Most alerts will have one or two buttons, e.g., OK and CANCEL.

An alert widget can be created in the usual way for shells, with
|XtVaCreatePopup| and similar routines, but a much more convenient way
is a call to |XfwfDialog|. |XfwfDialog| creates a temporary alert
widget and waits for the user to acknowledge it by clicking one of the
buttons. The function returns the number of the button that was
clicked.

An alert is made up of XfwfBoard(3X11), XfwfIcon(3X11),
Xfwflabel(3X11) and XfwfButton(3X11) widgets. The label widget will be
set to the minimum size that will show the full message. The alert
widget will be as small as possible to show all its children.

@PUBLIC

@ The icon is of type |Icon| (see XfwfIcon(3X11)). The converter can load
images from file and it knows about 6 built-in icons: |"FATAL"|, |"ERROR"|,
|"WARNING"|, |"QUESTION"|, |"INFO"| and |"NONE"|. The |NONE| icon is an
invisible (i.e., transparent) image of the same size as the others.

@var Icon* icon = <String> "NONE"

@ The message is a simple string, that may include newlines.

@var String message = NULL

@ The buttons are created with labels which will be taken from the
|buttons| resource. |buttons| is a string with the labels separated by
vertical bars. If the string is empty, a single button with label
``Ok'' will be created.

@var String buttons = NULL

@ The last button that was selected is also the default for the next
time (as soon as keyboard traversal has been implemented\dots) Buttons
are numbered starting with 0. Set this resource to -1 if you don't
want a default button.

@var int lastChoice = 0

@ When a button is clicked, the number of the button is stored in
|lastChoice|, before the |callback| functions are called. The
callbacks is passed the number in the |call_data| argument. The
buttons are numbered starting at 0.

@var <Callback> XtCallbackList callback = NULL

@PRIVATE

@ The various widgets that make up the alert box are stored in private
variables. |button| is an array of widgets, |num_buttons| is the length
of the array.

@var Widget outer
@var Widget iconw
@var Widget labelw
@var Widget *buttonw
@var int num_buttons

@EXPORTS

@ As stated above, the alert widget can be created in the normal way
as a Shell widget with a particular set of resources, but a more
convenient way in many cases will be the |XfwfDialog| function. The
function creates an alert box with an icon, message and buttons, pops
it up, and returns only when the user has clicked one of the buttons.
The function's result will be the number of the button that was
clicked.

The first argument, |toplevel|, will be the widget that is used as the
alert's parent. Usually that will be the top level window of the
application, but it need not be. The alert box will appear
approximately in the middle of the |toplevel| widget (if the window
manager allows it).

The second argument is the name of the icon that will be shown in the
upper left corner of the alert. It can be one of the built-in names
|"FATAL"|, |"ERROR"|, |"WARNING"|, |"QUESTION"|, |"INFO"| and
|"NONE"|; or it can be the name of a file that contains an icon in XPM
format. The icon widget is of class XfwfIcon(3X11). It is named
|"_icon"|.

The third argument is the message to display in the center of the
alert. It may contain newlines. The message will be put into an
XfwfLabel(3X11) widget, named, appropriately, |"_message"|.

The last argument, |buttons|, is a single string containing the labels
for all buttons, separated with a vertical bar. If this argument is
|NULL|, that doesn't mean that there will be no buttons. Instead, a
single button with the label |"OK"| will be created. After all, if
there are no buttons, how will the alert box ever disappear? The
buttons are of type XfwfButton(3X11) and are named |"1"|, |"2"|, etc.

The alert itself will simply be called |"alert"| and its child, an
XfwfBoard(3X11) widget, will be called |"_board"|.

The implementation is as follows: After the alert has been created,
|XfwfDialog| adds a callback to the alert widget, which will set the
|choice| variable when a button is pressed. The function then enters a
special purpose event loop, which dispatches events normally, but only
until the |choice| variable is changed. The function returns with the
value of |choice|

@def max(a, b) = ((a) > (b) ? (a) : (b))
@def min(a, b) = ((a) < (b) ? (a) : (b))

@proc int XfwfDialog(String name, Widget parent, String icon, String msg, String buttons)
{
    Widget alert;
    int choice = -1;
    XEvent event;
    Position x, y;
    Dimension wd, ht, wd1, ht1;

    x = $parent$x;
    y = $parent$y;
    wd = $parent$width;
    ht = $parent$height;
    alert = XtVaCreatePopupShell
	(name, xfwfAlertWidgetClass, parent,
	 XtVaTypedArg, XtNicon, XtRString, icon, strlen(icon) + 1,
	 XtNmessage, msg,
	 XtNbuttons, buttons,
	 NULL);
    XtAddCallback(alert, XtNcallback, button_pressed, &choice);
    XtRealizeWidget(alert);
    wd1 = $alert$width;
    ht1 = $alert$height;
    x = max(0, x + wd/2 - wd1/2);
    y = max(0, y + ht/2 - ht1/2);
    XtMoveWidget(alert, x, y);
    XtPopup(alert, XtGrabExclusive);
    while (choice == -1)
	XtAppProcessEvent(XtWidgetToApplicationContext(parent),
			  XtIMXEvent | XtIMTimer);
    XtDestroyWidget(alert);
    return choice;
}

@UTILITIES

@def ALLOC = 1024
@def myrealloc(p, size) =
    (void*) XtRealloc((void*) p, (size + ALLOC - 1)/ALLOC * ALLOC)
@def mymalloc(size) = (void*) XtMalloc(size)
@def myfree(p) = XtFree((void*) p)

@ The |button_pressed| callback is used by the |XfwfDialog| function.
It receives the number of the button that was pressed in the alert box
as |call_data| and it has a reference to a variable as |client_data|.
It simply copies the number to the variable and pops down the alert.

@proc button_pressed(Widget w, XtPointer client_data, XtPointer call_data)
{
    XtPopdown(w);
    *((int*) client_data) = (int) call_data;
}

@ The |click| callback is attached to each of the buttons. It sets the
|lastChoice| resource and calls the |callback| routines of the alert widget.

@proc click(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget $ = XtParent(XtParent(w));

    $lastChoice = (int) client_data;
    XtCallCallbackList($, $callback, (XtPointer) $lastChoice);
}

@ The |create_children| function is called by |initialize| and
|set_values|. It destroys the old children, except for the board, and
creates new ones. The board is only resized, to fit the new children.

@proc create_children($)
{
    Dimension ht, maxht, iconwd, iconht, labelwd, wd, bframe, labelht;
    Widget toprow, bottomrow, iconw, labelw, btn;
    Position x, buttony;
    String buttons, p;
    char name[20];
    int i;

    /*
     * Destroy old copies of the children
     */
    if ($iconw) XtDestroyWidget($iconw);
    if ($labelw) XtDestroyWidget($labelw);
    for (i = 0; i < $num_buttons; i++) XtDestroyWidget($buttonw[i]);

    bframe = XfwfCallFrameWidth($outer);	/* Board's frame */
    if ($buttons) buttons = XtNewString($buttons); /* Temporary copy */

    iconw = XtVaCreateManagedWidget
	("icon", xfwfIconWidgetClass, $outer,
	 XtNx, bframe,
	 XtNy, bframe,
	 XtNimage, $icon,
	 XtNtraversalOn, False,
	 NULL);
    iconwd = $iconw$width;
    iconht = $iconw$height;

    labelw = XtVaCreateManagedWidget
	("label", xfwfLabelWidgetClass, $outer,
	 XtNx, bframe + iconwd,
	 XtNy, bframe,
	 XtNshrinkToFit, True,
	 XtNlabel, $message,
	 NULL);
    labelwd = $labelw$width;
    labelht = $labelw$height;

    buttony = bframe + max(iconht, labelht);
    x = bframe + iconwd;
    maxht = 0;
    $num_buttons = 0;
    if ($buttons) {
	i = 0;
	p = strtok(buttons, "|");
	while (p) {
	    $buttonw = myrealloc($buttonw, (i + 1) * sizeof(*$buttonw));
	    sprintf(name, "%d", i);
	    btn = XtVaCreateManagedWidget
		(name, xfwfButtonWidgetClass, $outer,
		 XtNx, x,
		 XtNy, buttony,
		 XtNshrinkToFit, True,
		 XtNlabel, p,
		 NULL);
	    wd = $btn$width;
	    ht = $btn$height;
	    XtAddCallback(btn, XtNactivate, click, (XtPointer) i);
	    maxht = max(maxht, ht);
	    x += wd;
	    $buttonw[i] = btn;
	    i++;
	    p = strtok(NULL, "|");
	    $num_buttons++;
	}
	$num_buttons = i;
    } else {
	$buttonw = mymalloc(2 * sizeof(*$buttonw));
	$num_buttons = 1;
	btn = XtVaCreateManagedWidget
	    ("0", xfwfButtonWidgetClass, $outer,
	     XtNx, x,
	     XtNy, buttony,
	     XtNshrinkToFit, True,
	     XtNlabel, " OK ",
	     NULL);
	wd = $btn$width;
	ht = $btn$height;
	XtAddCallback(btn, XtNactivate, click, 0);
	maxht = ht;
	x += wd;
	$buttonw[0] = btn;
    }
    if ($buttons) XtFree(buttons);

    /*
     * And finally resize the outer Board
     */
    XtResizeWidget($outer, max(x + bframe, iconwd + labelwd + 2 * bframe),
		   buttony + maxht + bframe, 0);

    $iconw = iconw;
    $labelw = labelw;
}

@METHODS

@ The converter from String to Icon needs one extra argument, viz., the
widget for which the icon is loaded. An offset of 0 should give a
pointer to the widget itself.

@proc class_initialize
{
    static XtConvertArgRec args[] = {
    { XtWidgetBaseOffset, 0, sizeof(Widget) } };

    XtSetTypeConverter(XtRString, "Icon", cvtStringToIcon,
		       args, XtNumber(args), XtCacheNone, NULL);
}

@ The |initialize| methods sets the local variables to |NULL| and then
calls a utility function to create the children. The same function is
used by the |set_values| method.

@proc initialize
{
    $outer = NULL;
    $buttonw = NULL;
    $labelw = NULL;
    $iconw = NULL;
    $num_buttons = 0;
    $buttons = XtNewString($buttons);
    $message = XtNewString($message);
    $outer = XtVaCreateManagedWidget("outer", xfwfBoardWidgetClass, $, NULL);
    create_children($);
}

@ |set_values| checks to see if any of the resources changed. If so,
the old children are destroyed and new ones are made.

@proc set_values
{
    Boolean recreate = False;

    if ($old$message != $message) {
	XtFree($old$message);
	$message = XtNewString($message);
	recreate = True;
    }
    if ($old$buttons != $buttons) {
	XtFree($old$buttons);
	$buttons = XtNewString($buttons);
	recreate = True;
    }
    if ($old$icon != $icon || recreate) {
	create_children($);
    }
    return False;
}

@proc destroy
{
    XtFree($buttons);
    XtFree($message);
    XtFree((String) $buttonw);
}

@IMPORTS

@incl <stdlib.h>
@incl <stdio.h>
@incl <Xfwf/Icon.h>
@incl <Xfwf/Label.h>
@incl <Xfwf/RowCol.h>
@incl <Xfwf/Button.h>
@incl <Xfwf/Converters.h>
