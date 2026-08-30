# IconBox widget
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 1.0 for FWF V4.0

@class XfwfIconBox (XfwfBoard)  @file = IconBox

@ An IconBox widget is surface on which labeled icons are displayed.
The icons and the labels are actually a graphic representation of the
list of strings that is the value of the |list| resource. The user can
rearrange the icons by dragging them with the mouse, he can click on
them or double click and even select a number of them (selection is
indicated by reversing the colors of the icon). The application will
be notified of clicks and selections via callbacks. The |list|
resource also indicates whether icons are sensitive to other icons
being dropped on them. Icons that are, will give visual feedback and
cause a callback to be called.

The |list| resource contains three fields of information for every
icon: (1) the icon itself, (2) whether the icon is sensitive to
dropping, and (3) the label of the icon. Icons are specified by the
name of the file that contains the pixmap for it. Pixmaps are cached,
so that files are only read once. An application can also provide
pre-loaded icons.

The IconBox widget tries to adjust its height so that all icons can be
displayed, at least in canonical order.

@PUBLIC

@ The |list| resource is an array of |XfwfIconListItem|s. If the list
is not empty, there must be a pseudo-item after the last real one,
with -1 in the |flags| field. After setting the resource (e.g., with
|XtCreateWidget| or |XtSetValues|) the list can be deallocated or used
again elsewhere, since the IconBox will have made a private copy of it.

        @var XfwfIconList list = NULL

@ The IconBox widget has methods that can arrange the icons on a grid,
if the user so wishes. The size of that grid is set with two
resources: |horizontalGrid| and |verticalGrid|. Both are in pixels.

        @var Dimension horizontalGrid = 100

@ The vertical size of a grid cell in pixels.

        @var Dimension verticalGrid = 100

@ The first icons are placed a few pixels from the top of the IconBox.
|topskip| determines how much.

	@var int topskip = 3

@ When an icon is selected, either by a mouse click or by being
included in a rubber box, the |itemSelect| callback list is called.
The callback is called after the selection is complete, i.e., after
the click occurred or after the user completed the rubber box. The
callback is called once for every selected icon. The index of the icon
in the |list| is passed in the |call_data| argument.

        @var <Callback> XtCallbackList itemSelect = NULL

@ When an icon becomes unselected, the |itemDeselect| callback is
called, also with the icon's index as |call_data| argument.

        @var <Callback> XtCallbackList itemDeselect = NULL

@ When an icon is double-clicked, the icon is first selected (the
first mouse click causes the |itemSelect| callback to be called) and
then the |itemActivate| callback list is called. The icon's index is
passed in the |call_data| argument.

        @var <Callback> XtCallbackList itemActivate = NULL

@ When an icon or a group of icons is dropped on an icon that is
sensitive to this, the |itemDrop| callback list is invoked. The
|call_data| argument will contain a pointer to an |XfwfItemDropInfo|
structure, containing the numbers of the dropped icons and the icon
they were dropped upon.

        @var <Callback> XtCallbackList itemDrop = NULL

@ The font for the labels is usually smaller than the default font for
labels. The value of |labelFont| will be passed to the labels, this
means that setting the resource |Button.font| has no effect on the
labels below the icons.

	@var <FontStruct> XFontStruct *labelFont = <String> "fixed"

@ The translations that are installed on the icons and labels can be
changed by setting the |iconTranslations| resource. The default
translations provide for dragging, dropping, selecting and double
clicking, all with the first mouse button.

The |%1$d| in the actions will be replaced with the icon's number in
the |list| resource.

	@var String iconTranslations = "\
	    <Message>Drop: handle_drop(%1$d)\n\
	    Shift<Key>space: select_also(%1$d)\n\
	    <Key>space: select_or_drop(%1$d)\n\
	    <Key>Return: activate_item(%1$d)\n\
	    Shift<Btn1Up>: select_also(%1$d)\n\
	    <Btn1Up>: select_or_drop(%1$d)\n\
	    <Btn1Down>: prepare_drag(%1$d) check_double_click(%1$d)\n\
	    <Btn1Motion>: drag(%1$d)"

@ The cursor that is used during dragging.

	@var Cursor dragCursor = <String> "crosshair"

@EXPORTS

@ The header file for the |XfwfIcon| widget has to be included,
because it defines the |Icon| type, which is used in the
|XfwfCacheIcon| function.

	@incl <Xfwf/Icon.h>

@ The |XfwfIconListItem| structure has three fields: |icon|, which is
the name of file containing a pixmap; |flags|, which is an integer;
and |label|, which is a string, possibly containing newlines. The
|flags| field is interpreted as follows: a value of -1 means that this
item is a pseudo-item, used as sentinel at the end of a list,
otherwise an even number indicates an item that is not sensitive to
other items being dropped on top of it, an odd value indicates an item
that will cause a callback to be called when an icon is dropped on it.

        @type XfwfIconListItem = struct {
            String icon;
            int flags;
            String label;
        }
	@type XfwfIconList = XfwfIconListItem *

@ The |XfwfItemDropInfo| structure is used in the |itemDrop| callback. It
contains the number of the icon on which something was dropped and an
array with the numbers of the dropped icons. The |n| is the length of
the array.

        @type XfwfItemDropInfo = struct {
	    Widget sender;
            int target;
            int n;
            int *droplings;
        }

@ The |XfwfCleanUp| function calls the |clean_up| method, after
checking that the argument is indeed an IconBox. That method will move
the icons to the nearest unoccupied grid point.

@proc XfwfCleanUp($)
{
    if (XtIsSubclass($, xfwfIconBoxWidgetClass)) $clean_up($);
}

@ The |XfwfCanonicalOrder| function checks the class of the widget and
then calls the |canonical_order| method. The method will move the
icons to their original positions on the grid. The order of the icons
is the order in the |list| resource.

@proc XfwfCanonicalOrder($)
{
    if (XtIsSubclass($, xfwfIconBoxWidgetClass)) $canonical_order($);
}

@ The pixmaps for the icons are normally read from file by the IconBox
widget, but an application may provide pre-built icons and instruct
the IconBox to add them to its cache. In this way an application can
use compiled-in icons. The application should call the |XfwfCacheIcon|
function with the name under which the icon is kown (usually the file
name) and a complete |Icon| structure (see the XfwfIcon widget for a
description). The function is called with a particular widget, but the
icon cache is actually shared by all IconBox widgets.

The function simply stores the icon, without checking if another icon
of the same name already exists. If that is the case, the new icon
will override the old one.

@proc XfwfCacheIcon($, String name, Icon icon)
{
    int h;
    Bucket b;

    h = hashcode(name);
    new(b);
    b->name = XtNewString(name);
    b->icon = icon;
    b->next = $hashtable[h];
    $hashtable[h] = b;
}

@PRIVATE

@ The IconBox widget maintains additional info about each icon/label
pair in an array |item_info|. For each item this array contains the
widgets for the icon and label and the status (selected or
deselected).

	@var ItemInfo *item_info

@ The |ItemInfo| structure holds information about an item that cannot
be found in the |list| resource.

	@type ItemInfo = struct {
	    Widget icon, label;
	    Boolean selected;
	}

@ The number of items is also stored in a private variable, just for
convenience.

	@var int nitems

@ When one or more icons are being dragged, the |dragx0| and |dragy0|
variables hold the coordinates of the mouse when dragging began. The
previous mouse coordinates are held in |dragxprev| and |dragyprev|.

	@var int dragx0
	@var int dragy0
	@var int dragxprev
	@var int dragyprev

@ At the end of a drag action, a client message with a |message_type| of
|"Drop"| is sent to the widget on which the dragging stops. The message
type is an Atom (value |"Drop"|), that is created in the |initialize|
method.

The contents of the drop message are passed via a selection (root window
property), with the name |"DropSelection"|.

	@var Atom drop_atom
	@var Atom drop_selection

@ When a drag action is started, the |drag_in_progress| variable will
be set to |True|.

	@var Boolean drag_in_progress

@ The outlines of widgets being dragged are drawn with a GC |xor_gc|.

	@var GC xor_gc

@CLASSVARS

@ The hash table is an array of buckets. Each bucket is a list of
name/icon pairs.

	@var Bucket *hashtable = NULL

@ A hash bucket is a list of nodes. Each node holds a name and an
icon, and a pointer to the next node in the bucket.

	@type HashNode = struct _HashNode {
	    String name;
	    Icon icon;
	    struct _HashNode *next;
	}
	@type Bucket = HashNode *

@TRANSLATIONS

@ Note that the translations contain no click, double click or drag
actions involving a single icon. These events are not seen by the
IconBox, since they occur in an icon or a label. The IconBox will
install translations for these actions directly in the icons and
labels.

        @trans Shift<Btn1Down>: area_select_also()
        @trans <Btn1Down>: area_select()

@ A drag-and-drop operation can end on an icon or over unoccupied
space. In the first case, a client message will be sent to the icon,
in the second case, the client message will arrive at the IconBox
itself. The action will then move the dropped icons to the new
position.

	@trans <Message>Drop: move_icons()

@ACTIONS

@ The |area_select| action first unselect all selected icons and then
displays a rubberband. One corner of the rubberband box will follow
the mouse pointer, until the mouse button is released again. All icons
that are within the box at that moment will become selected and the
|itemSelect| callback will be called for them.

@proc area_select
{
    Widget icon, label;
    int i, l;
    Region reg;

    if (event->type != ButtonPress && event->type != ButtonRelease) {
	XtWarning("area_select action is not bound to button press/release");
	return;
    }
    rubberband_region($, event, &reg);

    for (i = 0; i < $nitems; i++) {
	icon = $item_info[i].icon;
	label = $item_info[i].label;
	if (XRectInRegion(reg, $icon$x, $icon$y, $icon$width,
			  $icon$height) == RectangleOut
	    && XRectInRegion(reg, $label$x, $label$y, $label$width,
			  $label$height) == RectangleOut) {
	    if ($item_info[i].selected) {
		XtVaSetValues($item_info[i].label, XtNrvLength, 0, NULL);
		$item_info[i].selected = False;
		XtCallCallbackList($, $itemDeselect, (XtPointer) i);
	    }
	} else {
	    if (! $item_info[i].selected) {
		l = strlen($list[i].label);
		XtVaSetValues($item_info[i].label, XtNrvLength, l, NULL);
		$item_info[i].selected = True;
		XtCallCallbackList($, $itemSelect, (XtPointer) i);
	    }
	}
    }
}

@ The |area_select_also| action does not unselect the selected items
before it starts, but otherwise it performs the same function as the
|area_select| action. |area_select_also| will only call the
|itemSelect| callback for items within the rubberband box that were
not already selected.

@proc area_select_also
{
    Widget icon, label;
    int i, l;
    Region reg;

    if (event->type != ButtonPress && event->type != ButtonRelease) {
	XtWarning("area_select action is not bound to button press/release");
	return;
    }
    rubberband_region($, event, &reg);

    for (i = 0; i < $nitems; i++) {
	icon = $item_info[i].icon;
	label = $item_info[i].label;
	if (XRectInRegion(reg, $icon$x, $icon$y, $icon$width,
			  $icon$height) == RectangleOut
	    && XRectInRegion(reg, $label$x, $label$y, $label$width,
			  $label$height) == RectangleOut) {
	    ; /* Outside the rectangle, no change */
	} else {
	    if (! $item_info[i].selected) {
		l = strlen($list[i].label);
		XtVaSetValues($item_info[i].label, XtNrvLength, l, NULL);
		$item_info[i].selected = True;
		XtCallCallbackList($, $itemSelect, (XtPointer) i);
	    }
	}
    }
}

@ The action |move_icons| should be called in response to the client
message that is sent at the end of a drag operation. The numbers of
the icons that are dropped are retrieved from the root window property
|"DropSelection"| and the icons are moved to the position where they
were dropped.

@proc move_icons
{
    Display *dpy = XtDisplay($);
    Window root = RootWindowOfScreen(XtScreen($));
    int stat, i, dx, dy, format;
    int *data;
    Atom type;
    unsigned long len, leftover;
    Widget icon, label;

    if (event->type != ClientMessage
	|| event->xclient.message_type != $drop_atom
	|| event->xclient.data.l[0] != (long) XtWindow($)) return;

    stat = XGetWindowProperty
	(dpy, root, $drop_selection, 0L, 100000L, False,
	 AnyPropertyType, &type, &format, &len, &leftover,
	 (unsigned char **) &data);
    /* Should check type, format and leftover here... */
    dx = $dragxprev - $dragx0;
    dy = $dragyprev - $dragy0;
    for (i = 0; i < len; i++) {
	icon = $item_info[data[i]].icon;
	label = $item_info[data[i]].label;
	XtMoveWidget(icon, $icon$x + dx, $icon$y + dy);
	XtMoveWidget(label, $label$x + dx, $label$y + dy);
    }
}

@ The following actions are installed automatically on the icons
within the IconBox. They should not be installed on the IconBox
itself.

@ The |select_also| action is also only installed on icons. The
|select_also| action selects the icon in which the event occurred,
without deselecting already selected icon. It will call the
|itemSelect| callback with the item's number as |call_data|.

@proc select_also
{
    int n, l;
    Widget iconbox = XtParent($);

    n = atoi(params[0]);
    /* fprintf(stderr, "select_also for: %s\n", $iconbox$list[n].label); */
    if (! $iconbox$item_info[n].selected) {
	$iconbox$item_info[n].selected = True;
	l = strlen($iconbox$list[n].label);
	XtVaSetValues($iconbox$item_info[n].label, XtNrvLength, l, NULL);
	XtCallCallbackList(iconbox, $iconbox$itemSelect, (XtPointer) n);
    }
}

@ When an icon is double-clicked, the |itemActivate| callback will be
called for it, with the item's number as |call_data|. Note that double
clicking will also invoke the |icon_select| action (or the
|select_also| action if the Shift key was pressed).

@proc activate_item
{
    int n;
    Widget iconbox = XtParent($);

    n = atoi(params[0]);
    /* fprintf(stderr, "activate_item for: %s\n", $iconbox$list[n].label); */
    XtCallCallbackList(iconbox, $iconbox$itemActivate, (XtPointer) n);
}

@ |check_double_click| compares the time of the click to the last
click time and if it is less than the multi-click time, a double click
is assumed and the |itemActivate| callback is called.

@proc check_double_click
{
    static Widget prev_widget = NULL;
    static Time prev_time;
    int n, multiclick;
    Widget iconbox;

    if ($ == prev_widget) {
	multiclick = XtGetMultiClickTime(XtDisplay($));
	if (event->xbutton.time - prev_time <= multiclick) {
	    n = atoi(params[0]);
	    iconbox = XtParent($);
	    XtCallCallbackList(iconbox, $iconbox$itemActivate, (XtPointer) n);
	}
    }
    prev_widget = $;
    prev_time = event->xbutton.time;
}

@ When the user starts to drag the mouse when the mouse pointer is on
an icon that is not (yet) selected, nothing happens. But when the
dragging starts with the cursor over a selected icon, all selected
icons will be dragged along with the mouse. (For speed, only an
outline of the icons is drawn.) If the mouse button is then released,
all icons will stay at the position where they are at that moment. No
callbacks are invoked. However, if the mouse button is released when
the mouse is on an icon that is sensitive to dropping, the icons will
return to their original positions and the |itemDrop| callback is
called.

|prepare_drag| only records the mouse coordinates. The drag is not
really started until the mouse is moved.

@proc prepare_drag
{
    Widget iconbox = XtParent($);
    Display *dpy = XtDisplay($);
    int n;

    n = atoi(params[0]);
    $iconbox$dragx0 = event->xmotion.x_root;
    $iconbox$dragy0 = event->xmotion.y_root;
    /* fprintf(stderr, "prepare_drag for: %s\n", $iconbox$list[n].label); */
}

@ When the mouse is moved, the previous outlines on the screen are
removed and replaced with new ones. If this is the first time the mouse
has moved after the button-down event, the pointer grab is changed to
install a temporary cursor.

@proc drag
{
    Widget iconbox = XtParent($);
    Display *dpy = XtDisplay($);
    int n, dx, dy, i;

    n = atoi(params[0]);
    dx = $iconbox$dragxprev - $iconbox$dragx0;
    dy = $iconbox$dragyprev - $iconbox$dragy0;

    if (! $iconbox$drag_in_progress) {
	/*
	XChangeActivePointerGrab
	    (dpy, ButtonMotionMask | ButtonPressMask | ButtonReleaseMask |
	     EnterWindowMask | LeaveWindowMask,
	     $iconbox$dragCursor, event->xmotion.time); 
	     */
	XGrabServer(dpy);
	/*
	XGrabPointer(dpy, XtWindow(iconbox), True, ButtonMotionMask |
		     ButtonPressMask | ButtonReleaseMask | EnterWindowMask |
		     LeaveWindowMask, GrabModeAsync, GrabModeAsync, None,
		     $iconbox$dragCursor, event->xmotion.time);
		     */
	$iconbox$drag_in_progress = True;
    } else if ($iconbox$item_info[n].selected) {
	for (i = $iconbox$nitems - 1; i >= 0; i--)
	    if ($iconbox$item_info[i].selected)
		draw_outlines(iconbox, i, dx, dy);
    } else {
	draw_outlines(iconbox, n, dx, dy);
    }

    $iconbox$dragxprev = event->xmotion.x_root;
    $iconbox$dragyprev = event->xmotion.y_root;
    dx = $iconbox$dragxprev - $iconbox$dragx0;
    dy = $iconbox$dragyprev - $iconbox$dragy0;

    if ($iconbox$item_info[n].selected) {
	for (i = $iconbox$nitems - 1; i >= 0; i--)
	    if ($iconbox$item_info[i].selected)
		draw_outlines(iconbox, i, dx, dy);
    } else {
	draw_outlines(iconbox, n, dx, dy);
    }
}

@ At the end of the drag, the outlins are removed and a message is sent
to the widget on which the mouse pointer ended. If there was no drag in
progress, the icon is selected instead.

If an icon is selected all other items become unselected. The action
will then call the |itemSelect| and possibly |itemDeselect| callbacks.
The icon's label will be shown in reverse.

@proc select_or_drop
{
    Widget iconbox = XtParent($);
    int n, dx, dy, i, l;

    n = atoi(params[0]);
    /*
      fprintf(stderr, "... select_or_drop for: %s\n", $iconbox$list[n].label);
      */
    if ($iconbox$drag_in_progress) {
	dx = $iconbox$dragxprev - $iconbox$dragx0;
	dy = $iconbox$dragyprev - $iconbox$dragy0;
	if ($iconbox$item_info[n].selected) {
	    for (i = $iconbox$nitems - 1; i >= 0; i--)
		if ($iconbox$item_info[i].selected)
		    draw_outlines(iconbox, i, dx, dy);
	} else {
	    draw_outlines(iconbox, n, dx, dy);
	}
	XFlush(XtDisplay($));
	XUngrabServer(XtDisplay($));
	set_selection(iconbox, n, event->xbutton.time);
	send_event(iconbox);
	$iconbox$drag_in_progress = False;
    } else {
	for (i = 0; i < $iconbox$nitems; i++) {
	    if (i != n && $iconbox$item_info[i].selected) {
		XtVaSetValues($iconbox$item_info[i].label, XtNrvLength,
			      0, NULL);
		$iconbox$item_info[i].selected = False;
		XtCallCallbackList(iconbox, $iconbox$itemDeselect,
				   (XtPointer) i);
	    }
	}
	if (! $iconbox$item_info[n].selected) {
	    $iconbox$item_info[n].selected = True;
	    l = strlen($iconbox$list[n].label);
	    XtVaSetValues($iconbox$item_info[n].label, XtNrvLength, l, NULL);
	    XtCallCallbackList(iconbox, $iconbox$itemSelect, (XtPointer) i);
	}
    }
}

@ When something is dropped on an icon that is sensitive to dropping,
the icon will call the |itemDrop| callbacks. The |call_data| will be a
pointer to an |XfwfItemDropInfo| structure with the numbers of the
dropped icons.

If the icon on which the drop occurred is not sensitive to dropping, the
dropped icons will simply be moved.

@proc handle_drop
{
    Display *dpy = XtDisplay($);
    Widget icon, label, iconbox, sender;
    Window root = RootWindowOfScreen(XtScreen($));
    int n, stat, format, i, dx, dy;
    unsigned long len, leftover;
    Atom type;
    XfwfItemDropInfo info;

    n = atoi(params[0]);
    iconbox = XtParent($);
    if (event->type != ClientMessage
	|| event->xclient.message_type != $iconbox$drop_atom) return;

    /*
      if (event->xclient.data.l[0] != (long) XtWindow(iconbox)) {
       XtWarning("Received drop from another IconBox!");
       return;
      }
    */
    sender = info.sender = (Widget) event->xclient.data.l[1];
    if (! XtIsSubclass(sender, xfwfIconBoxWidgetClass)) {
	XtWarning("Cannot handle drop from anything other than IconBox");
	return;
    }
    info.target = n;
    stat = XGetWindowProperty
	(dpy, root, $iconbox$drop_selection, 0L, 100000L, False,
	 AnyPropertyType, &type, &format, &len, &leftover,
	 (unsigned char **) &info.droplings);
    info.n = len;
    /* Should check type, format and leftover here... */
    if ($iconbox$list[n].flags & 1) {
	icon = $iconbox$item_info[n].icon;
	label = $iconbox$item_info[n].label;
	$icon$unhighlight_border(icon);
	$label$unhighlight_border(label);
	XtCallCallbackList(iconbox, $iconbox$itemDrop, &info);
    } else if (event->xclient.data.l[0] == (long) XtWindow(iconbox)) {
	dx = $iconbox$dragxprev - $iconbox$dragx0;
	dy = $iconbox$dragyprev - $iconbox$dragy0;
	for (i = 0; i < info.n; i++) {
	    icon = $iconbox$item_info[info.droplings[i]].icon;
	    label = $iconbox$item_info[info.droplings[i]].label;
	    XtMoveWidget(icon, $icon$x + dx, $icon$y + dy);
	    XtMoveWidget(label, $label$x + dx, $label$y + dy);
	}
    } else {
	/*
	 * Dropped from different iconbox on an item that is not
	 * sensitive to dropping; do nothing.
	 */
    }
}

@ When the pointer enters an icon during a drag operation, the icon (and
label) will be highlighted. The highlighting is the same as for keyboard
focus, since the same (Common) method is used.  Hopefully this is not too
confusing.

@proc drag_into
{
    Widget icon, label, iconbox = XtParent($);
    int n;

    n = atoi(params[0]);
    if ($iconbox$drag_in_progress && ($iconbox$list[n].flags & 1)) {
	icon = $iconbox$item_info[n].icon;
	label = $iconbox$item_info[n].label;
	$icon$highlight_border(icon);
	$label$highlight_border(label);
    }
}

@ When the pointer leave an icon during a drag operation, the highlight is
removed again.

@proc drag_out_of
{
    Widget icon, label, iconbox = XtParent($);
    int n;

    n = atoi(params[0]);
    if ($iconbox$drag_in_progress && ($iconbox$list[n].flags & 1)) {
	icon = $iconbox$item_info[n].icon;
	label = $iconbox$item_info[n].label;
	$icon$unhighlight_border(icon);
	$label$unhighlight_border(label);
    }
}

@METHODS

@ The |clean_up| method is usually called via the |XfwfCleanUp|
function. It lines up the icons on a grid, moving each to the closest
grid point, but taking care not to put two icons in the same space.

@proc clean_up($)
{
    /* not implemented yet */
}

@ The |canonical_order| method lines up the icons on a grid, in the
order of the |list| array. Applications normally call the exported
function |XfwfCanonicalOrder|, which then calls this method.

@proc canonical_order($)
{
    int i, x, y, w;
    Widget icon, label;

    x = $horizontalGrid/2;
    y = $topskip;
    w = $width - $horizontalGrid/2;
    for (i = 0; i < $nitems; i++) {
	icon = $item_info[i].icon;
	label = $item_info[i].label;
	XtMoveWidget(icon, x - $icon$width/2, y);
	XtMoveWidget(label, x - $label$width/2, y + $icon$height);
	x += $horizontalGrid;
	if (x >= w) {
	    x = $horizontalGrid/2;
	    y += $verticalGrid;
	}
    }
}

@ The |class_initialize| method clears the hash table. It would be
possible to initialize hash buvkets only when needed, but since the
hash table is quite small anyway, the overhead might as well all be
put into this routine.

@proc class_initialize
{
    int i;

    xfwfIconBoxClassRec.xfwfIconBox_class.hashtable =
	mymalloc(PRIME * sizeof(Bucket));
    for (i = 0; i < PRIME; i++)
	xfwfIconBoxClassRec.xfwfIconBox_class.hashtable[i] = NULL;
    /* Install a type converter for XfwfItemList here */
}

@ |initialize| creates the default icon, if it has not been created
before. Then it creates the icons and labels as indicated by the
|list| resource. The |list| resource itself is copied to private
memory.

@proc initialize
{
    XtGCMask mask = GCFunction | GCForeground | GCSubwindowMode;
    XGCValues values;
    Widget icon;

    if (! defaultIcon_created) {
	defaultIcon.attributes.valuemask = XpmSize;
	(void) XpmCreatePixmapFromData
	    (XtDisplay($), RootWindowOfScreen(XtScreen($)),
	     default_icon, &defaultIcon.pixmap,
	     &defaultIcon.mask, &defaultIcon.attributes);
	defaultIcon_created = True;
    }
    $drop_atom = XInternAtom(XtDisplay($), "Drop", False);
    $drop_selection = XInternAtom(XtDisplay($), "DropSelection", False);
    $drag_in_progress = False;
    values.function = GXxor;
    values.foreground = 1;
    values.subwindow_mode = IncludeInferiors;
    $xor_gc = XtGetGC($, mask, &values);
    $nitems = 0;
    $item_info = NULL;
    copy_list($);
    create_icons($);
    if ($nitems > 0) {
	icon = $item_info[$nitems-1].icon;
	$set_abs_location($, CWHeight, 0, 0, 0, $icon$y + $verticalGrid);
    }
}

@ The |set_values| method creates new icons if the |list| resource has
changed.

@proc set_values
{
    Boolean recreate = False;
    int i;
    Widget icon;

    if ($old$list != $list) {
	deallocate_list($old$list);
	copy_list($);
	create_icons($);
	if ($nitems > 0) {
	    icon = $item_info[$nitems-1].icon;
	    $set_abs_location($, CWHeight, 0, 0, 0, $icon$y + $verticalGrid);
	}
    }
    return False;
}

@ When the IconBox is resized, the icons are not moved. This is
different from the inherited behaviour (from Board), in which the
children are asked their preferred positions. The |resize| method is
therefore redefined (as nothing).

@proc resize
{
    /* skip */
}


@UTILITIES

@def ALLOC = 1024
@def new(p) = p = (void*) XtMalloc(sizeof(*(p)))
@def myrealloc(p, size) =
    (void*) XtRealloc((void*) p, (size + ALLOC - 1)/ALLOC * ALLOC)
@def mymalloc(size) = (void*) XtMalloc(size)
@def myfree(p) = XtFree((void*) p)

@ |deallocate_list| frees the memory occupied by a list.

@proc deallocate_list(XfwfIconList list)
{
    int i;

    if (list != NULL) {
	for (i = 0; list[i].flags != -1; i++) {
	    myfree(list[i].icon);
	    myfree(list[i].label);
	}
	myfree(list);
    }
}

@ The |copy_list| function makes a fresh copy of the |list| resource.

@proc copy_list($)
{
    XfwfIconList list = NULL;
    int i, n;

    if ($list != NULL) {
	for (n = 0; $list[n].flags != -1; n++) ; /* skip */
	list = mymalloc((n + 1) * sizeof(list[0]));
	for (i = 0; i < n; i++) {
	    list[i].icon = XtNewString($list[i].icon);
	    list[i].flags = $list[i].flags;
	    list[i].label = XtNewString($list[i].label);
	}
	list[n].flags = -1;
    }
    $list = list;
}

@ The |draw_outlines| function is used by the |drag| action. It draws
the outline of an icon and label, offset by |dx|, |dy|. The outline is a
simple rectangle, drawn with XOR, so the same routime can be used to
both draw and remove the outlines.

@proc draw_outlines($, int n, int dx, int dy)
{
    Widget icon, label;
    Display *dpy = XtDisplay($);
    Window root = RootWindowOfScreen(XtScreen($));
    Position x, y;

    icon = $item_info[n].icon;
    XtTranslateCoords($, $icon$x + dx, $icon$y + dy, &x, &y);
    XDrawRectangle(dpy, root, $xor_gc, x, y, $icon$width, $icon$height);

    label = $item_info[n].label;
    XtTranslateCoords($, $label$x + dx, $label$y + dy, &x, &y);
    XDrawRectangle(dpy, root, $xor_gc, x, y, $label$width, $label$height);
}

@ |send_event| is called at the end of the |select_or_drop| action, when
a drop has just ended. It sends a client message to the widget on which
the cursor is now.

@proc send_event($)
{
    static XClientMessageEvent ev;
    Display *dpy = XtDisplay($);
    Window root, child, child0;
    int dummy1, dummy2, dummy3, dummy4;
    unsigned int dummy5;

    child = RootWindowOfScreen(XtScreen($));
    while (XQueryPointer(dpy, child, &root, &child0, &dummy1, &dummy2,
			 &dummy3, &dummy4, &dummy5) && child0 != None)
	child = child0;

    ev.type = ClientMessage;
    ev.display = dpy;
    ev.message_type = $drop_atom;
    ev.format = 32;
    ev.data.l[0] = (long) XtWindow($);
    ev.data.l[1] = (long) $;
    ev.window = child;
    ++ev.serial;
    XSendEvent(dpy, child, True, NoEventMask, (XEvent*)&ev);
}

@ |set_selection| installs the list of dropped icons as a property
(|"DropSelection"|) of the root window. The dropped icons are stored in
a list of integers, either one integer if the dragged icon was not
selected, or a list of all selected icons.

@proc set_selection($, int n, Time time)
{
    Display *dpy = XtDisplay($);
    Window win = RootWindowOfScreen(XtScreen($));
    static int *data = NULL;
    int len, i, j, stat;

    XSetSelectionOwner(dpy, $drop_selection, win, time);
    if (XGetSelectionOwner(dpy, $drop_selection) != win)
	XtWarning("XGetSelectionOwner failed");
    else if ($item_info[n].selected) {
	for (len = 0, i = $nitems - 1; i >= 0; i--)
	    if ($item_info[i].selected) len++;
	/* fprintf(stderr, "set_selection len=%d: ", len); */
	myfree(data);
	data = mymalloc(len * sizeof(*data));
	for (j = 0, i = 0; i < $nitems; i++)
	    if ($item_info[i].selected) {
		/* fprintf(stderr, " %d", i);  */
		data[j++] = i;
	    }
	/* fprintf(stderr, "\n"); */
    } else {
	myfree(data);
	data = mymalloc(sizeof(*data));
	data[0] = n;
	len = 1;
	/* fprintf(stderr, "set_selection len=1: %d\n", n); */
    }
    stat = XChangeProperty
	(dpy, win, $drop_selection, XA_INTEGER, sizeof(*data) * 8,
	 PropModeReplace, (unsigned char *) data, len);
    /* Test stat here? */
}

@ The default icon is a global variable. It must be created once. The
|defaultIcon_created| variable is set to |True| in |initialize|, after
the icon has been created.

	@var Boolean defaultIcon_created = False
	@var Icon defaultIcon

@ The pixmaps for the icons are cached, to ensure that the pixmap file
for a particular icon is only read once. The icons are stored in an
open hash table, indexed on their name. The hash table is part of the
IconBox class, so all instances share the same icon cache. Icons are
never removed from the cache.

The exported function |XfwfCacheIcon| adds an icon to the hash table,
|lookup| retrieves an icon, |hashcode| computes the hash index.

@def PRIME = 211

@proc int hashcode(String name)
{
    String p;
    unsigned int h = 0, g;
    
    for (p = name; *p; p++) {
	h = (h << 4) + (*p);
	if (g = h & 0xf0000000) {
	    h = h ^ (g >> 24);
	    h = h ^ g;
	}
    }
    return h % PRIME;

}

@ |lookup| returns |True| if it found the icon with name |name|,
otherwise it returns |False|. The icon itself will be stored in the
|icon| argument.

@proc Boolean lookup($, String name, Icon *icon)
{
    Bucket b;

    for (b = $hashtable[hashcode(name)]; b != NULL; b = b->next)
	if (strcmp(b->name, name) == 0) {
	    *icon = b->icon;
	    return True;
	}
    return False;
}

@ The |read_icon| function tries to read an icon from file. If it
fails, it prints a warning an returns a default icon instead.

@proc read_icon($, String name, Icon *icon)
{
    Display *dpy;
    int status;
    XtAppContext apc;

    dpy = XtDisplay($);
    apc = XtDisplayToApplicationContext(dpy);
    icon->attributes.valuemask = XpmSize;
    status = XpmReadFileToPixmap(dpy, DefaultRootWindow(dpy), name,
				 &icon->pixmap, &icon->mask,
				 &icon->attributes);
    if (status != XpmSuccess && status != XpmColorError) {
	XtAppWarning(apc, "Failed to create an icon");
	*icon = defaultIcon;
    }
}

@ The function |get_icon| first tries to find the icon with |lookup|
and if that fails tries |read_icon|.

@proc get_icon($, String name, Icon *icon)
{
    if (! lookup($, name, icon)) {
        read_icon($, name, icon);
        XfwfCacheIcon($, name, *icon);
    }
}

@ The function |create_icons| is called by the |initialize| and
|set_values| methods to create the icons for all items in the |list|
resource. Its task is simple: for all items it calls |get_icon| and
then creates an icon widget and a label widget, with a particular set
of resources. When all icons have been created, it calls the
|canonical_order| method to move the icons to their initial positions.

The icons and their labels are given the following translations, the
`|%d|' will become the items number.

|check_double_click| is a kludge, because |<Btn1Down>(2)| breaks down
the other translations.

I would like to also have something like
    Button1<Enter>: drag_into(%1$d)\n
    Button1<Leave>: drag_out_of(%1$d)\n

@proc create_icons($)
{
    int i, len;
    Icon *icon;
    String s;

    s = mymalloc((strlen($iconTranslations) + 50) * sizeof(char));
    for (i = 0; i < $nitems; i++) {
	XtDestroyWidget($item_info[i].icon);
	XtDestroyWidget($item_info[i].label);
    }
    if ($list == NULL)
        $nitems = 0;
    else {
        for (i = 0; $list[i].flags != -1; i++) ; /* skip */
	$nitems = i;
	$item_info = mymalloc(i * sizeof(*$item_info));
        for (i = 0; i < $nitems; i++) {
            new(icon);
            get_icon($, $list[i].icon, icon);
            len = sprintf(s, $iconTranslations, i);
            $item_info[i].icon = XtVaCreateManagedWidget
		($list[i].icon, xfwfIconWidgetClass, $, XtVaTypedArg,
		 XtNtranslations, XtRString, s, len+1, XtNimage, icon,
		 XtNtraversalOn, False, NULL);
            $item_info[i].label = XtVaCreateManagedWidget
		($list[i].label, xfwfButtonWidgetClass, $,
		 XtVaTypedArg, XtNtranslations, XtRString, s, len+1,
		 XtNlabel, $list[i].label, XtNshrinkToFit, True,
		 XtNfont, $labelFont, NULL);
	    $item_info[i].selected = False;
        }
    }
    $canonical_order($);
    myfree(s);
}

@proc rubberband_region($, XEvent *event, Region *reg)
{
    Display *dpy = XtDisplay($);
    Window root, child, win = XtWindow($);
    int minx, miny, wd, ht, x0, y0, rx, ry, x, y, l;
    Boolean wait_press;
    unsigned int mask, button;
    XRectangle rect;

    wait_press = (event->type == ButtonRelease);
    button = Button1Mask << (event->xbutton.button - 1);
    x0 = event->xbutton.x;  wd = 0;  minx = x0;
    y0 = event->xbutton.y;  ht = 0;  miny = y0;
    XDrawRectangle(dpy, win, $xor_gc, minx, miny, wd, ht);
    while (XQueryPointer(dpy, win, &root, &child, &rx, &ry, &x, &y, &mask)) {
	if (wait_press && (mask & button)) wait_press = False;
	else if (! wait_press && ! (mask & button)) break;
	if (minx != min(x, x0) || miny != min(y, y0)
	    || wd != abs(x - x0) || ht != abs(y - y0)) {
	    XDrawRectangle(dpy, win, $xor_gc, minx, miny, wd, ht);
	    minx = min(x, x0);  wd = abs(x - x0);
	    miny = min(y, y0);  ht = abs(y - y0);
	    XDrawRectangle(dpy, win, $xor_gc, minx, miny, wd, ht);
	}
    }
    XDrawRectangle(dpy, win, $xor_gc, minx, miny, wd, ht);

    rect.x = minx; rect.y = miny; rect.width = wd; rect.height = ht;
    *reg = XCreateRegion();
    XUnionRectWithRegion(&rect, *reg, *reg);
}

@IMPORTS

@incl <X11/xpm.h>
@incl <Xfwf/Button.h>
@incl "default_icon.xpm"
@incl <stdio.h>
@incl <X11/Xatom.h>
