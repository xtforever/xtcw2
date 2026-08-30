# MenuBar -- menu bar widget
# Bert Bos <bert@let.rug.nl>
# Version 2.1 for FWF V4.0
# 26 Apr '94: changed superclass from XfwfRowCol to XfwfRows

@class XfwfMenuBar (XfwfRows) @file=MenuBar

@ The MenuBar widget is a special kind of Rows widget. It assumes
that all its children are PullDown buttons or other widgets that pop
up a menu. (But nothing will break if they are not.)

The MenuBar attaches itself to the |popup| callback of the menu shell
that is popped up by the children, and inserts itself in the list of
pointer grabs that Xt maintains. This is necessary to be able to drag
the mouse from one menu into another, popping down the first one and
popping up the second. If you use a RowCol widget instead of a
MenuBar, you will find that switching from one menu to the next
involves releasing the mouse and then pressing it again.

To find the menu shell, the children are asked for their |popup|
resource. PullDown buttons have this resource, others may not.

The MenuBar widget introduces no new resources.

@PUBLIC

	@var frameType = XfwfRaised

	@var traversalOn = False

@EXPORTS

@ The MenuBar widget has a method |process_menu| that can be used by
children (usually PullDown buttons) to pop up a menu and to register
it with the menu bar. If they do so, the menu bar is able to process
events elsewhere in the menu bar and pop down the menu in favour of
another one, when the user moves the mouse to another button in the
menu bar. A menu or a button is unable to do this by itself, since
they do not know what other menus are present.

To call the |process_menu| method, widgets should use the
|XfwfCallProcessMenu| function, which will check that the receiver of
the call is indeed a MenuBar widget. It returns |True| if the
|process_menu| method could be called, otherwise |False|.

The value for |cursor| can also be |None|, in which case the cursor
from the menu's parent (the root window, usually) will be used
instead.

@proc Boolean XfwfCallProcessMenu($, Widget menu, Cursor cursor)
{
    if (XtIsSubclass($, xfwfMenuBarWidgetClass) && $process_menu) {
	$process_menu($, menu, cursor);
	return True;
    } else
	return False;
}

@PRIVATE

@ A pointer to the currently popped up menu is kept in a private
variable |current_menu|. The variable is set in the |process_menu|
method.

	@var Widget current_menu

@METHODS

@proc initialize
{
    $current_menu = NULL;
}

@ The |process_menu| method is provided for use by children (usually
PullDown buttons) to pop up their menus. If they call this method
instead of popping up the menu themselves, the menu bar has a chance
to intercept events for other buttons in the menu bar and pop down the
menu when the user clicks on another button in the menubar.

The method adds the appropriate (passive) global grabs, establishes an
Xt local grab and adds a callback to the menu, so that the menu bar
will be informed when the menu is popped down again.

@proc process_menu($, Widget menu, Cursor cursor)
{
    if (menu == $current_menu) return;
    if ($current_menu) XtPopdown($current_menu);
    XtAddGrab($, True, False);
/*
    XtGrabButton(menu, AnyButton, AnyModifier, True, ButtonPressMask |
		 ButtonReleaseMask | EnterWindowMask | LeaveWindowMask
		 | PointerMotionMask, GrabModeAsync, GrabModeAsync,
		 None, cursor);
    XtGrabKey(menu, AnyKey, AnyModifier, True, GrabModeAsync,
	      GrabModeAsync);
*/
    XtAddCallback(menu, XtNpopdownCallback, popdown_cb, $);
    $current_menu = menu;
    XtPopup(menu, XtGrabNonexclusive);
}

@TRANSLATIONS

@ Since the MenuBar widget adds itself to the list of cascaded popups
that is maintained by Xt, it will receive pointer events when a menu
is popped up. If the event is a button release, the current menu will
have to be popped down.

	@trans <Btn1Up>: menu_popdown()

@ACTIONS

@ The |menu_popdown| action calls |XtPopdown| for the current menu,
if there is one.

@proc menu_popdown
{
    if ($current_menu != NULL) {
	XtPopdown($current_menu);
	$current_menu = NULL;
    }
}

@UTILITIES

@ The |popdown_cb| routine is a callback that is attached to the
currently popped up menu. When the menu pops down, the routine removes
the grabs and resets |current_menu| to |NULL|.

@proc popdown_cb(Widget menu, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;

    XtRemoveCallback(menu, XtNpopdownCallback, popdown_cb, $);
/*
    XtUngrabKey(menu, AnyKey, AnyModifier);
    XtUngrabButton(menu, AnyButton, AnyModifier);
*/
    XtRemoveGrab($);
    $current_menu = NULL;
}

@IMPORTS

@incl <Xfwf/PullDown.h>
@incl <X11/Shell.h>
@incl <stdio.h>
