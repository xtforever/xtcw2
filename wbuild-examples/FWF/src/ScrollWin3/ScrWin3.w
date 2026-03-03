# ScrollWin3.w -- ScrolledWindow widget with column label
#
# By Rick Richardson <rick@digibd.com> from original source
# by Bert Bos <bert@let.rug.nl>.
#
# version 1.2 (11 Oct '94)

@CLASS XfwfScrolledWindow3 (XfwfBoard)  @file = ScrWin3

@ The ScrolledWindow3 widget is a composite widget that contains 4
Boards and two scroll bars. The first Board is a small area in the top
left corner, where a label widget can be put. A Board along the top
and one along the left side are used for column and row titles
(usually Label widgets). The two scroll bars are along the right side
and the bottom. In the middle is the largest board, in which the main
data will be displayed.

A widget is added as the child of the large Board is called the
`controlled widget' (CW), The widget that is put in the top Board is
called the `column controlled widget' (CCW), the widget that is put in
the left Board is the `row controlled widget' (RCW).

Usually, the controlled widgets are larger than the Boards, and their
origins will have negative x and y coordinates. They are moved about
inside the Boards by the ScrolledWindow3, in reaction to the user
clicking on the scrollbars. The CCW moves left/right, the RCW moves
up/down, the CW moves in all directions. The controlled widgets are
therefore clipped by the Boards.

The first child widget attached to ScrolledWindow3 becomes the child
of the top left Board. The second child widget becomes the CCW, the
third the RCW, and the fourth the CW.

The CCW is typically a Label widget with a tablist to provide column
headers, and only scrolls horizontally. The CW is typically a
single column MultiList widget with a tablist, and scrolls boths
horizontally and vertically.

The ScrolledWindow3 provides a callback, but most application will not
need it, since the ScrolledWindow3 already moves the CW. The callback
is invoked AFTER the CW is moved.


@PUBLIC

@ By default, the Board widget, Frame widget and the Scrollbars are
placed 4 pixels from each other and from the edges of the ScrolledWindow3
itself.

	@var Dimension spacing = 4

@ The width of the scrollbars can be set with the |scrollbarWidth|
resource. The default is 22 pixels.

	@var Dimension scrollbarWidth = 22

@ The height of the Board widget to contain the CCW can be set with
the |colHdrHeight| resource. The default is 22 pixels. The width of
the Board to contain the RCW is |rowHdrWidth|, by default also 22
pixels.

	@var Dimension colHdrHeight = 22
	@var Dimension rowHdrWidth = 22

@ The scrollbars and the frame have the same shadow frame. (The board
has no border or frame.) The width of both is set with a single
resource: |shadowWidth|. The style of the shadow cannot be changed. It
will always be |XfwfSunken|.

	@var Dimension shadowWidth = 2

@ It is possible to switch off the display of the scrollbars, with the
following two resources: |hideHScrollbar| is by default |False|.

	@var Boolean hideHScrollbar = False

@ |hideVScrollbar| is also by default |False|.

	@var Boolean hideVScrollbar = False

@ When the arrows of the scrollbars are clicked, the controlled widgets
will be moved a fixed number of pixels in the indicated direction. The
distance can be set separately for horizontal and vertical movement.
E.g., if the controlled widget displays text, it is a good idea to set
the vertical scroll distance to the height of one line of text.

	@var int vScrollAmount = 20

@ The horizontal amount is by default copied from the vertical amount.

	@var int hScrollAmount = <CallProc> copy_vScrollAmount

@ The initial position of the CCW and CW can be set with the resources
|initialX| and |initialY|. Both are given in pixels and must be zero
or negative.

	@var Position initialX = 0

@ |initialY| is for the vertical position.

	@var Position initialY = 0

@ The callback is passed a pointer to an |XfwfScrollInfo| structure,
which is defined as |struct { XfwfSReason reason; XfwfSFlags flags;
float vpos, vsize, hpos, hsize; }| The |reason| can be:

* |XfwfSUp|: if the user clicked (or holds) the up arrow.

* |XfwfSLeft|: ditto left arrow.

* |XfwfSDown|: ditto down arrow.

* |XfwfSRight|: ditto right arrow.

* |XfwfSPageUp|: ditto area above thumb.

* |XfwfSPageDown|: ditto area below thumb.

* |XfwfSPageLeft|: ditto area left of thumb.

* |XfwfSPageRight|: ditto area right of thumb.

* |XfwfSDrag|: if the user drags one of the thumbs.

* |XfwfSMove|: if the user stops dragging the thumb.

* |XfwfSTop|: if the user requests to scroll all the way up.

* |XfwfSBottom|: same all the way down.

* |XfwfSLeftSide|: same to the left side.

* |XfwfSRightSide|: same to the right side.

Note that when the callback is called, the controlled window has
already been moved.

	@var <Callback> XtCallbackList scrollCallback = NULL

@ To set the scrolled window to a particular position, the
scrollResponse function must be used. The function has the type of a
callback function and it must be retrieved with a call to
|XtGetValues|.

	@var XtCallbackProc scrollResponse = scroll_response


@EXPORTS

@ To set the position of a scrolling widget from an application, one
could use |XtGetValues| to get the |scrollResponse| resource and then
call the returned function. The |XfwfScrollTo| function is a utility
function that makes this more convenient. Its arguments are a widget,
and |x| and |y| values (both must be between 0.0 and 1.0). It will
scroll the widget and all other connected scrolling widgets to the
indicated offset.

Note that this function is not tied to the ScrolledWindow3 widget: it
works with every widget that implements the FWF SWIP (Scrolling Widget
Interface Policy).

@proc XfwfScrollTo($, double x, double y)
{
    XtCallbackProc f = NULL;
    XfwfScrollInfo info;

    XtVaGetValues($, XtNscrollResponse, &f, NULL);
    if (f == NULL) {
	XtWarning("XfwfScrollTo called for non-scrolling widget.");
	return;
    }
    info.flags = XFWF_VPOS | XFWF_HPOS;
    info.reason = XfwfSMove;
    info.hpos = x;
    info.vpos = y;
    f(NULL, $, &info);
}

@PRIVATE

@ The ScrolledWindow2 widget has exactly four children, two grandchildren,
and one grandgrandchild. For convenience they are stored in private
variables as well as in the |children| field.

	@var Widget ulboard
	@var Widget CORNER
	@var Widget cboard
	@var Widget CCW
	@var Widget rboard
	@var Widget RCW
	@var Widget vscroll
	@var Widget hscroll
	@var Widget frame
	@var Widget board
	@var Widget CW

@ The variable |initializing| is set to |True| in the |initialize|
method, so that the |insert_child| method can see that the inserted
child is one of the three proper children.

	@var Boolean initializing

@ The scroll bars have scrollResponse function, that will be called if
the scrolled window itself received a call to its scrollResponse
function.

	@ disabled, 15 Jun '94
 var XtCallbackProc vscroll_resp
	@
 var XtCallbackProc hscroll_resp


@METHODS

@ The |initialize| method creates the six child widgets and the
grandchild and sets the controlled widgets to NULL. The resources of
the children are set to fixed values and some of the ScrolledWindow's
resources are copied. The board widget will automatically be set to
the correct size by the frame widget in which it is contained.

@proc initialize
{
    $initializing = True;
    if ($initialX > 0) $initialX = 0;
    if ($initialY > 0) $initialY = 0;
    $CORNER = NULL;
    $CCW = NULL;
    $RCW = NULL;
    $CW = NULL;
    $ulboard = XtVaCreateManagedWidget
	("_ulboard", xfwfBoardWidgetClass, $, XtNframeWidth, 0,
	 XtNborderWidth, 0, XtNhighlightThickness, 0, NULL);
    $cboard = XtVaCreateManagedWidget
	("_cboard", xfwfBoardWidgetClass, $, XtNframeWidth, 0,
	 XtNborderWidth, 0, XtNhighlightThickness, 0, NULL);
    $rboard = XtVaCreateManagedWidget
	("_rboard", xfwfBoardWidgetClass, $, XtNframeWidth, 0,
	 XtNborderWidth, 0, XtNhighlightThickness, 0, NULL);
    $frame = XtVaCreateManagedWidget
	("_frame", xfwfFrameWidgetClass, $, XtNframeType, XfwfSunken,
	 XtNframeWidth, $shadowWidth, XtNborderWidth, 0,
	 XtNhighlightThickness, 0, NULL);
    $board = XtVaCreateManagedWidget
	("_board", xfwfBoardWidgetClass, $frame, XtNframeWidth, 0,
	 XtNborderWidth, 0, XtNhighlightThickness, 0, NULL);
    $vscroll = XtVaCreateWidget
	("_vscroll", xfwfVScrollbarWidgetClass, $, XtNframeWidth,
	 $shadowWidth, XtNframeType, XfwfSunken, XtNborderWidth, 0, NULL);
    $hscroll = XtVaCreateWidget
	("_hscroll", xfwfHScrollbarWidgetClass, $, XtNframeWidth,
	 $shadowWidth, XtNframeType, XfwfSunken, XtNborderWidth, 0, NULL);
    if (! $hideVScrollbar) XtManageChild($vscroll);
    if (! $hideHScrollbar) XtManageChild($hscroll);
    compute_sizes($);
    XfwfConnectScrollingWidgets($, $vscroll);
    XfwfConnectScrollingWidgets($, $hscroll);
    $initializing = False;
    if ($scrollResponse != scroll_response) {
	$scrollResponse = scroll_response;
	XtWarning("scrollResponse resource may only be queried, not set");
    }
}

@ Changes in the resources cause the scrolled window to recompute the
layout of its children.

The |scrollResponse| resource may not be changed.

@proc set_values
{
    if ($old$spacing != $spacing
	|| $old$scrollbarWidth != $scrollbarWidth
	|| $old$shadowWidth != $shadowWidth
	|| $old$hideHScrollbar != $hideHScrollbar
	|| $old$hideVScrollbar != $hideVScrollbar)
	compute_sizes($);
    if ($old$hideVScrollbar && ! $hideVScrollbar)
	XtUnmanageChild($vscroll);
    else if (! $old$hideVScrollbar && $hideVScrollbar)
	XtManageChild($vscroll);
    if ($old$hideHScrollbar && ! $hideHScrollbar)
	XtUnmanageChild($hscroll);
    else if (! $old$hideHScrollbar && $hideHScrollbar)
	XtManageChild($hscroll);
    if ($scrollResponse != $old$scrollResponse) {
	$scrollResponse = $old$scrollResponse;
	XtWarning("scrollResponse resource may only be queried, not set");
    }
    return False;
}


@ When the scrolled window is resized, the scrollbars and the frame
should be repositioned. Also, the thumbs in the scrollbars will have
to change size, but for that purpose there is an event handler that
will intercept changes to the configuration of the CW or the board.
The event handler is installed when the CW is inserted (in the
|insert_child| method) and removed again when the child is destroyed
(in the |CW_killed| callback routine).

@proc resize
{
    compute_sizes($);
}


@ The |insert_child| method performs some tricks to ensure that the
four proper children are added normally, that the fifth is passed on
to the cboard, that the sixth is passed on to the board, and that any
other children are refused.

When the CW is added as a child of the board, the scrollbars are
initialized to the current position and size of the CW relative to the
board.

@proc insert_child
{
    Widget board;
    Position boardx, boardy, gx, gy;
    Dimension boardwd, boardht, gwd, ght;
    float wd, ht, x, y;
    Boolean dummy;

    if ($initializing) {
	#insert_child(child);
    } else if ($CORNER == NULL) {
	$CORNER = child;
	$child$parent = $ulboard;
	XtAddCallback(child, XtNdestroyCallback, CORNER_killed, $);
	#insert_child(child);
#if 0
	XtAddEventHandler(child, StructureNotifyMask, False, configure, $);
	XtAddEventHandler($ulboard, StructureNotifyMask, False, configure, $);
	$compute_inside($ulboard, &boardx, &boardy, &boardwd, &boardht);
	XtVaGetValues(child, XtNwidth, &gwd, XtNheight, &ght, NULL);
	gx = gwd <= boardwd ? 0 : max($initialX, boardwd - gwd);
	gy = ght <= boardht ? 0 : max($initialY, boardht - ght);
	XtMoveWidget(child, gx, gy);
	configure($, $, NULL, &dummy);
#endif
    } else if ($CCW == NULL) {
	$CCW = child;
	$child$parent = $cboard;
	XtAddCallback(child, XtNdestroyCallback, CCW_killed, $);
	#insert_child(child);
#if 0
	XtAddEventHandler(child, StructureNotifyMask, False, configure, $);
	XtAddEventHandler($cboard, StructureNotifyMask, False, configure, $);
	$compute_inside($cboard, &boardx, &boardy, &boardwd, &boardht);
	XtVaGetValues(child, XtNwidth, &gwd, XtNheight, &ght, NULL);
	gx = gwd <= boardwd ? 0 : max($initialX, boardwd - gwd);
	gy = ght <= boardht ? 0 : max($initialY, boardht - ght);
	XtMoveWidget(child, gx, gy);
	configure($, $, NULL, &dummy);
#endif
    } else if ($RCW == NULL) {
	$RCW = child;
	$child$parent = $rboard;
	XtAddCallback(child, XtNdestroyCallback, RCW_killed, $);
	#insert_child(child);
#if 0
	XtAddEventHandler(child, StructureNotifyMask, False, configure, $);
	XtAddEventHandler($rboard, StructureNotifyMask, False, configure, $);
	$compute_inside($rboard, &boardx, &boardy, &boardwd, &boardht);
	XtVaGetValues(child, XtNwidth, &gwd, XtNheight, &ght, NULL);
	gx = gwd <= boardwd ? 0 : max($initialX, boardwd - gwd);
	gy = ght <= boardht ? 0 : max($initialY, boardht - ght);
	XtMoveWidget(child, gx, gy);
	configure($, $, NULL, &dummy);
#endif
    } else if ($CW == NULL) {
	$CW = child;
	$child$parent = $board;
	XtAddCallback(child, XtNdestroyCallback, CW_killed, $);
	#insert_child(child);
	XtAddEventHandler(child, StructureNotifyMask, False, configure, $);
	XtAddEventHandler($board, StructureNotifyMask, False, configure, $);
	$compute_inside($board, &boardx, &boardy, &boardwd, &boardht);
	XtVaGetValues(child, XtNwidth, &gwd, XtNheight, &ght, NULL);
	gx = gwd <= boardwd ? 0 : max($initialX, boardwd - gwd);
	gy = ght <= boardht ? 0 : max($initialY, boardht - ght);
	if ($CCW) XtVaSetValues($CCW, XtNwidth, &gwd, NULL);
	if ($RCW) XtVaSetValues($RCW, XtNheight, &ght, NULL);
	XtMoveWidget(child, gx, gy);
	configure($, $, NULL, &dummy);
    } else {
	char s[500];
	(void) sprintf(s, "Cannot add <%s>, %s <%s> already has 4 children\n",
		       XtName(child), "ScrolledWindow", XtName($));
	XtWarning(s);
    }
}


@ When the scrolled window widget is destroyed, the event handler is
removed (if it was installed).

@proc destroy
{
    if ($CCW != NULL) {
	XtRemoveEventHandler($CCW, StructureNotifyMask, False, configure, $);
	XtRemoveEventHandler($cboard, StructureNotifyMask, False, configure,$);
    }
    if ($RCW != NULL) {
	XtRemoveEventHandler($RCW, StructureNotifyMask, False, configure, $);
	XtRemoveEventHandler($rboard, StructureNotifyMask, False, configure,$);
    }
    if ($CW != NULL) {
	XtRemoveEventHandler($CW, StructureNotifyMask, False, configure, $);
	XtRemoveEventHandler($board, StructureNotifyMask, False, configure, $);
    }
    if ($CORNER != NULL) {
	XtRemoveEventHandler($CORNER, StructureNotifyMask, False, configure,$);
	XtRemoveEventHandler($ulboard, StructureNotifyMask, False,configure,$);
    }
}

@ When the ScrolledWindow2 widget is configured from outside, this is
done through a call to the |scroll_response| method.

@proc scroll_response(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;
    XfwfScrollInfo *info = (XfwfScrollInfo *) call_data;
    XfwfScrollInfo new;
    Position boardx, boardy, gx, gy, minx, miny;
    Dimension boardwd, boardht, gwd, ght;
    float wd, ht, x, y;

    $compute_inside($board, &boardx, &boardy, &boardwd, &boardht);
    XtVaGetValues($CW, XtNx, &gx, XtNy, &gy, XtNwidth, &gwd,
		  XtNheight, &ght, NULL);
    minx = gwd <= boardwd ? 0 : ((int) boardwd) - gwd;
    miny = ght <= boardht ? 0 : ((int) boardht) - ght;

    if (info->flags & XFWF_VPOS) gy = info->vpos * miny;
    if (info->flags & XFWF_HPOS) gx = info->hpos * minx;

    XtMoveWidget($CCW, gx, 0);	/* Scrolls horizontally only */
    XtMoveWidget($RCW, 0, gy);	/* Scrolls vertically only */
    XtMoveWidget($CW, gx, gy);
#if 0
    XtMoveWidget($CORNER, 0, 0);
#endif

    if (info->reason != XfwfSNotify) {
	new.reason = XfwfSNotify;
	new.flags = info->flags & (XFWF_VPOS | XFWF_HPOS);
	new.hpos = info->hpos;
	new.vpos = info->vpos;
	XtCallCallbackList($, $scrollCallback, &new);
    }
}
    

@UTILITIES

@ |compute_sizes| is used by |initialize|, |resize| and |set_values|
to configure the children.

@proc compute_sizes($)
{
    Dimension selfw, selfh, framew, frameh, vsheight, hswidth, cwidth, rheight;
    Position selfx, selfy;
#if 0
    Dimension boardwd, boardht, gwd, ght;
    Position boardx, boardy, gx, gy;
    float wd, ht, x, y;
#endif

    $compute_inside($, &selfx, &selfy, &selfw, &selfh);

    vsheight = selfh - 3 * $spacing - $colHdrHeight;
    if (! $hideHScrollbar) vsheight -= $spacing + $scrollbarWidth;
    hswidth = selfw - 3 * $spacing - $rowHdrWidth;
    if (! $hideVScrollbar) hswidth -= $spacing + $scrollbarWidth;
    cwidth = selfw - 3 * $spacing - $rowHdrWidth - 2 * $shadowWidth;
    if (! $hideVScrollbar) cwidth -= $spacing + $scrollbarWidth;
    rheight = selfh - 3 * $spacing - $colHdrHeight - 2 * $shadowWidth;
    if (! $hideHScrollbar) rheight -= $spacing + $scrollbarWidth;

    XtConfigureWidget($ulboard,
		      selfx + $spacing,
		      selfy + $spacing,
		      $rowHdrWidth,
		      $colHdrHeight,
		      0);
    XtConfigureWidget($cboard,
		      selfx + 2*$spacing + $rowHdrWidth + $shadowWidth,
		      selfy + $spacing,
		      cwidth,
		      $colHdrHeight,
		      0);
    XtConfigureWidget($rboard,
		      selfx + $spacing,
		      selfy + 2*$spacing + $colHdrHeight + $shadowWidth,
		      $rowHdrWidth,
		      rheight,
		      0);
    XtConfigureWidget($vscroll,
		      selfx + selfw - $spacing - $scrollbarWidth,
		      selfy + $spacing + $colHdrHeight + $spacing,
		      $scrollbarWidth,
		      vsheight,
		      0);
    XtConfigureWidget($hscroll,
		      selfx + 2*$spacing + $rowHdrWidth,
		      selfy + selfh - $spacing - $scrollbarWidth,
		      hswidth,
		      $scrollbarWidth,
		      0);
    framew = selfw - 3 * $spacing - $rowHdrWidth;
    frameh = selfh - 3 * $spacing - $colHdrHeight;
    if (! $hideVScrollbar) framew -= $scrollbarWidth + $spacing;
    if (! $hideHScrollbar) frameh -= $scrollbarWidth + $spacing;
    XtConfigureWidget($frame,
		      selfx + 2*$spacing + $rowHdrWidth,
		      selfy + 2*$spacing + $colHdrHeight,
		      framew,
		      frameh,
		      0);

#if 0
    /* If its been resized, move the scrolled widgets to 0,0.
     * This is quick-n-dirty.  A more sophisticated implementation
     * would attempt to preserve the relative location.
     */
    if ($CCW != NULL) XtMoveWidget($CCW, 0, 0);
    if ($RCW != NULL) XtMoveWidget($RCW, 0, 0);
    if ($CW != NULL) XtMoveWidget($CW, 0, 0);
#else
#if 0
    if ($CW != NULL) {
	XtVaGetValues($CW, XtNx, &gx, XtNy, &gy, XtNwidth, &gwd,
		      XtNheight, &ght, NULL);
	$compute_inside($board, &boardx, &boardy, &boardwd, &boardht);
	if (gwd <= boardwd) {
	    if ($CCW != NULL) XtMoveWidget($CCW, 0, 0);
	    XtMoveWidget($CW, 0, gy);
	    gx = 0;
	}
	if (ght <= boardht) {
	    if ($RCW != NULL) XtMoveWidget($RCW, 0, 0);
	    XtMoveWidget($CW, gx, 0);
	}
    }
#endif /* 0 */
#endif /* 1 */
}


@ The |copy_vScrollAmount| routine is a resource default function. It
copies the value of the |vScrollAmount| resource to the
|hScrollAmount| resource.

@proc copy_vScrollAmount($, int offset, XrmValue* value)
{
    value->addr = (XtPointer) &$vScrollAmount;
}


@ When the CCW or CW is destroyed, the private variables |CCW| or |CW|
must be set back to |NULL|. The event handler that was installed to
intercept changes to the board and the CCW or CW must also be
removed.  The scrollbars are set to their `resting position'.

@proc CW_killed(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;

    XtRemoveEventHandler(w, StructureNotifyMask, False, configure, $);
    XtRemoveEventHandler($board, StructureNotifyMask, False, configure, $);
    $CW = NULL;
    XfwfSetScrollbar($vscroll, 0.0, 1.0);
    XfwfSetScrollbar($hscroll, 0.0, 1.0);
}

@proc CCW_killed(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;

    XtRemoveEventHandler(w, StructureNotifyMask, False, configure, $);
    XtRemoveEventHandler($cboard, StructureNotifyMask, False, configure, $);
    $CCW = NULL;
}

@proc RCW_killed(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;

    XtRemoveEventHandler(w, StructureNotifyMask, False, configure, $);
    XtRemoveEventHandler($rboard, StructureNotifyMask, False, configure, $);
    $RCW = NULL;
}
@proc CORNER_killed(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;

    XtRemoveEventHandler(w, StructureNotifyMask, False, configure, $);
    XtRemoveEventHandler($ulboard, StructureNotifyMask, False, configure, $);
    $CORNER = NULL;
}


@ The |configure| routine is called when the CW or the large board
receive ConfigureNotify events (and a few others, which are not
useful). It is also called by |insert_child|, when the CW is first
created. The changed size of these widgets results in different sizes
for the sliders in the scrollbars.

@proc configure(Widget w, XtPointer client_data, XEvent *event, Boolean *cont)
{
    Widget $ = (Widget) client_data;
    Dimension boardwd, boardht, gwd, ght;
    Position boardx, boardy, gx, gy;
    float wd, ht, x, y;

    if (event != NULL && event->type != ConfigureNotify) return;
    $compute_inside($board, &boardx, &boardy, &boardwd, &boardht);
    if ($CW == NULL)
	gwd = ght = 0;
    else
	XtVaGetValues($CW, XtNx, &gx, XtNy, &gy, XtNwidth, &gwd,
		      XtNheight, &ght, NULL);
    wd = gwd <= boardwd ? 1.0 : (float) boardwd/gwd;
    ht = ght <= boardht ? 1.0 : (float) boardht/ght;
    x = gwd <= boardwd ? 0.0 : gx/((float) boardwd - gwd);
    y = ght <= boardht ? 0.0 : gy/((float) boardht - ght);
    if (x > 1.0) x = 1.0;
    if (y > 1.0) y = 1.0;
    XfwfSetScrollbar($hscroll, x, wd);
    XfwfSetScrollbar($vscroll, y, ht);
}


@IMPORTS

@incl <stdio.h>
@incl <Xfwf/Frame.h>
@incl <Xfwf/Board.h>
@incl <Xfwf/VScrollb.h>
@incl <Xfwf/HScrollb.h>
@incl <Xfwf/scroll.h>
