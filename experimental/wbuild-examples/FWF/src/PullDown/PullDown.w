# PullDown -- A button with a pull down menu
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 1.0 for FWF V4.0

@class XfwfPullDown (XfwfButton)  @file=PullDown

@ A PullDown button looks like a normal button, but when it is clicked or
pressed, a menu drops down. The menu can be specified in two ways: as a
string or as a widget. If a string is given, it must be in the correct
syntax for the XfwfTextMenu widget and a widget will be created
automatically. If a widget is given instead, it must be a widget that is
able to pop up: in other words: a Shell widget or a subclass of Shell.

The most likely application of this widget is as a menu in a menu bar.  A
menu bar can be created, e.g., with a RowCol widget and a number of
PullDown buttons.

When the popup is passed in as a string in the |menu| resource, the widget
will call the |activate| callback (see XfwfButton) when the user exits the
menu with a valid selection. It will also call the |changeSelection|
callback (see below) when the user moves from one menu item to the next.
Neither of these callbacks will be called when the popup is passed in as a
ready-made widget in the |popup| resource.

Note however, that the |call_data| argument of the |activate| callback will
contain the number of the selected item (0, 1, 2, etc.) and not the event,
as is the case in an XfwfButton widget.


@PUBLIC

@ Two resources determine the menu. The |popup| resource can be
pointed at the widget that should pop up when the button is pressed,
but when the |popup| resource is NULL, the menu will be created
from the |menu| resource. If the |popup| resource is set, it must
be set to a widget that is (a subclass of) a Shell widget.

	@var Widget popup = NULL

@ When |popup| is |NULL|, the widget will be created from the
textual description in |menu|. For the syntax of this string, see the
XfwfTextMenu.

	@var String menu = NULL

@ When the button is part of a dialog or menu, that has already
caused a grab of the pointer, the resource |cascaded| should be
|True|. If the button is inserted into an XfwfMenuBar, the resource is
set to |True| automatically.

	@var Boolean cascaded = False

@ The pull down button installs an event handler that catches all
keyboard events for the whole application and checks for a special key
combination. If that key is detected, the menu will be popped up.
Thus, the menu can be used even if the keyboard focus is somewhere
else. The |hotkey| resource is usually a string such as |"Alt<Key>a"|,
or |"Ctrl<Key>f"|.

When the |hotkey| resource is used, any |accelerators| will be ignored.

	@var String hotkey = NULL

@ The cursor shape that is used in the menu can be set with the
|menuCursor| resource. By default it is an arrow pointing to the upper
right.

	@var Cursor menuCursor = <String> "arrow"

@ The |prepare| callback is invoked by the |prepare| action, normally just
before the menu is popped up.

	@var <Callback> XtCallbackList prepare = NULL

@ When the user moves from one item to another in the menu that was created
from the |menu| resource, the |changeSelection| callback is called, with
the number of the new selection as |call_data|. This callback is called
even when the new selection is an inactive (grayed out) item. It is also
called when the mouse leaves the menu, in which case the argument will be
-1. This callback will notbe called when the menu is a ready-made widget,
passed in via the |popup| resource.

	@var <Callback> XtCallbackList changeSelection = NULL

@ Since the most likely place for this type of button will be in a
menubar, the default frame width is reset to zero.

	@var frameWidth = 0

@ The |shrinkToFit| resource is set to a new default of |True|.

	@var shrinkToFit = True


@PRIVATE

@ When the popup menu is created from the |menu| resource, the
variable |own_popup| is set to |True|. In this way it is possible to
destroy the popup again when it is no longer needed.

	@var Boolean own_popup


@METHODS

@proc class_initialize
{
    XtRegisterGrabAction(open_menu, True, ButtonPressMask |
			 ButtonReleaseMask | EnterWindowMask |
			 LeaveWindowMask | PointerMotionMask,
			 GrabModeAsync, GrabModeAsync);
}

@ Both the |initialize| and |set_values| methods need to create a new
menu widget, if the |menu| resource is set and the |popup| resource
is not. A utility function is used to do the actual work. If both
|popup| and |menu| are set, |popup| is used and |menu| is
ignored.

@proc initialize
{
    if ($menu) $menu = XtNewString($menu);
    if (! $popup && $menu) {
	$popup = XtVaCreatePopupShell
	    ("menu", xfwfTextMenuWidgetClass, $, XtNmenu, $menu, XtNcursor,
	     $menuCursor, NULL);
	$own_popup = True;
	XtAddCallback($popup, XtNactivate, activate_cb, $);
	XtAddCallback($popup, XtNchangeSelection, change_cb, $);
    } else
	$own_popup = False;
    if (! $popup && $hotkey) create_accelerator($);
}


@ |set_values| is similar. Changes in these resources never cause a redraw,
so the method always returns |False|. A change in |menu| is only recognized
if there is no change in |popup| at the same time.

@proc set_values
{
    if ($old$menu != $menu) {
	XtFree($old$menu);
	$menu = XtNewString($menu);
    }
    if ($old$popup != $popup) {
	if ($own_popup && $old$popup) XtDestroyWidget($old$popup);
	$own_popup = False;
    } else if ($old$menu != $menu) {
	$popup = XtVaCreatePopupShell
	    ("menu", xfwfTextMenuWidgetClass, $, XtNmenu, $menu, XtNcursor,
	     $menuCursor, NULL);
	$own_popup = True;
	XtAddCallback($popup, XtNactivate, activate_cb, $);
	XtAddCallback($popup, XtNchangeSelection, change_cb, $);
    }
    if ($old$menuCursor != $menuCursor)
	if ($popup) XtVaSetValues($popup, XtNcursor, $menuCursor, NULL);
    if ($old$hotkey != $hotkey)
	create_accelerator($);
    return False;
}



@TRANSLATIONS

	@trans <Btn1Down>: prepare() open_menu()
	@trans Button1<Enter>: prepare() open_menu() enter()
	@trans <Key>Return: prepare() open_menu()
	@trans <Btn1Up>: popdown_maybe()
	@trans <EnterNotify>: enter()
	@trans <LeaveNotify>: leave()


@ACTIONS

@ |note| is for debugging only.

@proc note
{
    static String Xeventname[] = {
	"", "", "KeyPress", "KeyRelease", "ButtonPress", "ButtonRelease",
	"MotionNotify", "EnterNotify", "LeaveNotify", "FocusIn",
	"FocusOut", "KeymapNotify", "Expose", "GraphicsExpose",
	"NoExpose", "VisisbilityNotify", "CreateNotify", "DestroyNotify",
	"UnmapNotify", "MapNotify", "MapRequest", "ReparentNotify",
	"ConfigureNotify", "ConfigureRequest", "GravityNotify",
	"ResizeRequest", "CirculateNotify", "CirculateRequest",
	"PropertyNotify", "SelectionClear", "SelectionRequest",
	"SelectionNotify", "ColormapNotify", "ClientMessage",
	"MappingNotify", };
    (void) fprintf(stderr, "%s received %s\n",
		   XtName($), Xeventname[event->type]);
}

@ The |prepare| action just calls the |prepare| callback functions, passing
the |XEvent| pointer in the |call_data| argument.

@proc prepare
{
    XtCallCallbackList($, $prepare, event);
}

@ The |open_menu| action computes the coordinates of the menu, such that it
will appear directly below the button. A local grab is added, so that the
button itself will continu to receive events, even after the popped up menu
has grabbed the pointer and keyboard. The accelerators of the menu are
installed in the pulldown button, which will continue to have the focus.
It then pops up the menu.

@proc open_menu
{
    Position x, y;

    if ($popup == NULL) return;
    /* fprintf(stderr, "open_menu \"%s\"\n", XtName($)); */
    XtTranslateCoords($, 0, $height, &x, &y);
    XtVaSetValues($popup, XtNx, x, XtNy, y, NULL);
    if (! XfwfCallProcessMenu(XtParent($), $popup, $menuCursor))
	XtPopupSpringLoaded($popup);
}

@ The |podown_maybe| action only pops down the menu when the pointer
is outside the button. When the pointer is on the button, the menu
stays posted.

@proc popdown_maybe
{
    Position x, y;

    switch (event->type) {
    case ButtonRelease:
    case ButtonPress:
	x = event->xbutton.x;
	y = event->xbutton.y;
	break;
    case MotionNotify:
	x = event->xmotion.x;
	y = event->xmotion.y;
	break;
    case EnterNotify:
    case LeaveNotify:
	x = event->xcrossing.x;
	y = event->xcrossing.y;
	break;
    default:
	XtAppWarning(XtWidgetToApplicationContext($),
		   "Incorrect event for popdown_maybe()");
    }
    if (x < 0 || $width <= x || y < 0 || $height <= y) XtPopdown($popup);
}

@UTILITIES

@ |create_accelerator| takes the |hotkey| and expands it into a
correct |accelerators| resource.

@proc create_accelerator($)
{
    char s[100];
    XtAccelerators a;

    (void) strcpy(s, $hotkey);
    (void) strcat(s, ": prepare() open_menu()");
    a = XtParseAcceleratorTable(s);
    XtVaSetValues($, XtNaccelerators, a, NULL);
}

@ The |activate_cb| routine is attached to the |activate| callback of the
menu, but only if the menu is created by the PullDown widget itself. It
simply calls the PullDown widget's |activate| callback. The |call_data|
argument contains the number of the selected menu item.

@proc activate_cb(Widget menu, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;

    XtCallCallbackList($, $activate, call_data);
}

@ The |change_cb| routine is attached to the |changeSelection| callback of
the menu, but only if the menu is created by the PullDown widget itself. It
simply calls the PullDown widget's |changeSelection| callback. The
|call_data| argument contains the number of the selected menu item.

@proc change_cb(Widget menu, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;

    XtCallCallbackList($, $changeSelection, call_data);
}

@IMPORTS

@incl <Xfwf/TextMenu.h>
@incl <Xfwf/MenuBar.h>
@incl <stdio.h>
