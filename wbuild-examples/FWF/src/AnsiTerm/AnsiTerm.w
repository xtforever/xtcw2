# XfwfAnsiTerm -- ANSI terminal emulator widget
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version 1.2 (12 Feb 1996)
#

@CLASS XfwfAnsiTerm (XfwfBoard) @file=AnsiTerm

@ The AnsiTerm widget emulates an ANSI terminal in an X Window. It
displays lines of text and scrolls them if necessary. It displays a
cursor, to indicate where the next text will appear and it interprets
the ANSI escape codes to position the cursor or highlight the text.

Text is written to the widget by calling |XfwfAnsiWrite| (a
convenience function which calls the |write| method).

A callback is provided for reacting to key presses. The widget also
supports the selection mechanism, the same way the xterm program does
(select by dragging with mouse button 1 pressed, paste by clicking
with mouse button 2)

Another callback is provided to notify the application when the number
of rows or columns changes. If the application runs a shell or some
other program in the background, it can use this callback to send a
|SIGWINCH| signal to the child process.


@PUBLIC

@ The |rows| resource is used to set the number of lines of text. The
widget will try to set its |height| to match the number of rows. Note
that the reverse is not true: if the height changes for any other
reason, the |rows| resource is not changed.

	@var int rows = 24

@ The |columns| resource sets the width of the widget to a certain
number of characters. The widget will try to set its |width| larger
enough for that many columns. The reverse is not true: if the |width|
changes, |columns| is not adjusted.

	@var int columns = 80

@ The widget uses two fonts: normal and bold. They should usually be
monospaced fonts. Note that |font| is a string, not a |XFontStruct|;
this is to enable the widget to derive the name of the bold version
from this string.

	@var String font = XtDefaultFont

@ The |boldfont| is by default computed on the basis of the
|font| resource.

	@var XFontStruct *boldfont = <CallProc> infer_bold

@ The margin between the frame and the text, on all sides.

	@var int margin = 10

@ The color of the text.

	@var Color foreground = <String> XtDefaultForeground

@ The |keyCallback| is called whenever the widget receives a key
press. The callback is called by the |key| action procedure. The
|call_data| argument contains a pointer to the character that was
pressed (type: |char *|). (If the pressed key has no representation in
the form of one or more characters, the callback is not called.)

	@var <Callback> XtCallbackList keyCallback = NULL

@ The |resizeCallback| is invoked when the widget changes
the number of rows or columns. The |call_data| holds a
pointer to a struct |XfwfResizeInfo| containing two ints.
The first is the number of rows, the second the number of
columns

	@var <Callback> XtCallbackList resizeCallback = NULL

@ The AnsiTerm widget takes part in the FWF's Scrolling Widget
Interface Policy (SWIP). The following two callbacks are needed for
connecting the widget to other scrolling widgets (such as scrollbars).

The routines on the |scrollCallback| when the widget receives a call
on its |scrollResponse| function (see below) with a reason other than
|XfwfSNotify|. This is used to pass on the information to
other widgets in the scrolling chain.

The |call_data| parameter of the callback routines is a pointer
to an |XfwfScrollInfo| structure, which looks like this: |typedef
struct _XfwfScrollInfo { XfwfSReason reason; XfwfSFlags flags; float
vpos, vsize, hpos, hsize;} XfwfScrollInfo|.

See the documentation on SWIP and in particular the function
|XfwfConnectScrollingWidgets| for more info.

	@var <Callback> XtCallbackList scrollCallback = NULL

@ When the application or another widget wants the AnsiTerm widget to
scroll, it calls a function in the AnsiTerm widget. A pointer to that
function can be retrieved with |XtGetValues| as the resource
|XtNscrollResponse|. This resource can only be queried, not set.

	@var XtCallbackProc scrollResponse = scroll_response


@CLASSVARS

@ Set a few class variables for optimizing event processing:
multiple consecutive events of the same type are added
together by Xt and delivered as one event. This applies to
motion, expose and enter/leave events.

	@var compress_motion = True
	@var compress_enterleave = True
	@var compress_exposure = XtExposeCompressMultiple
		| XtExposeGraphicsExpose | XtExposeGraphicsExposeMerged


@EXPORTS

@ The |XfwfAnsiWrite| function sends characters to the AnsiTerm
widget. It is a convenience function that simply calls the |write|
method of the widget.

@proc void XfwfAnsiWrite($, char *buf, int nbytes)
{
    if (! XtIsSubclass($, xfwfAnsiTermWidgetClass))
	XtAppError(XtWidgetToApplicationContext($),
		   "XfwfAnsiWrite may only be called for AnsiTerm widgets.");
    $write($, buf, nbytes);
}

@ The |resizeCallback| is passed a pointer to an
|XfwfResizeInfo| struct.

	@type XfwfResizeInfo = struct {
	    int rows, columns;
	}

@ The |scroll.h| header file is needed for the |XfwfScrollInfo|
structure.

@incl <Xfwf/scroll.h>


@PRIVATE

@ The size of a character cell is assumed to be constant. It is stored
in |cellwidth| and |cellheight|.

	@var int cellwidth
	@var int cellheight
	@var int uline_pos
	@var int uline_thick

@ Four GC's are used for drawing the eight possible faces:
normal/bold, normal/reverse, with/without underline.

	@var XFontStruct *fnt
	@var GC gc
	@var GC boldgc
	@var GC revgc
	@var GC revboldgc

@ The contents of the widget are held in an array of lines.
Each character also has a byte with attributes (bold,
reverse, underlined) in a separate array. |allocated_lines|
is the length of the array. It is used in the
|allocate_contents| function to know the number of lines to
deallocate.

The |ATTRIB_DIRTY| attribute is used by the |write| method and its
utility functions to mark lines that have changed, so that they can
all be redrawn at the end of the |write| function. This flag is only
ever set on the first character of a line.

	@var char **contents
	@var char **attribs
	@var int allocated_rows
	@var int allocated_columns
	@def ATTRIB_BOLD = (1 << 0)
	@def ATTRIB_REV = (1 << 1)
	@def ATTRIB_ULINE = (1 << 2)
	@def ATTRIB_INVIS = (1 << 3)
	@def ATTRIB_DIRTY = (1 << 7)

@ The cursor position is held in |cursorx| and |cursory|. Because the
cursor might move between being drawn and being removed, the timer
procedure that blinks the cursor keeps the position where the cursor
is drawn in a separate pair of coordinates.

Another pair of coordinates is used to implement the `ESC 7' or `ESC [
s'(save cursor) and `ESC 8' or `ESC [ u' (restore cursor) control
sequences.

	@var int cursorx
	@var int cursory
	@var int old_cx
	@var int old_cy
	@var int savedx
	@var int savedy
	@var Bool cursor_on

@ The next text to be added at the cursor position has attributes
|cur_attrib|. The value of this variable can only be changed through
ANSI escapes, that are sent to the |write| method. |insert_mode| is
likewise only set by escape sequences; it determines whether new
characters on a line push existing characters to the right and
newlines push lower lines down. |last_char| is the most recently
written character.

	@var char cur_attrib
	@var Bool insert_mode
	@var char last_char;

@ The |write| method interprets escape using an FSM, the state of
which is recorded in |state|. Escape sequences can have up to 5
numeric parameters, which are stored by |write| in 5 registers.

	@def Init = 0
	@def Esc = 1
	@def EscLB = 2
	@def EscLBGT = 3
	@def EscLBGT2 = 4
	@def Register0 = 5
	@def Register1 = 6
	@def Register2 = 7
	@def Register3 = 8
	@def Register4 = 9
	
	@var int state
	@var int escparm[5]
    
@ The |timer| variable holds the ID of the timer function
that handles the blinking of the cursor.

	@var XtIntervalId timer

@ The current selection is kept in a buffer.

	@var int start_x
	@var int start_y
	@var int end_x
	@var int end_y
	@var char *selection
	@var int selection_len

@ A press of Button1 without a subsequent motion is not enough to
start a selection. The |drag_started| variable is set to |TRUE| by
|extend_selection| and is inspected by |end_selection| to decide
whether a selection has taken place. If |drag_started| is |FALSE| the
event was a simple click.

        @var Bool drag_started

@ The AnsiTerm widget can `lock' a number of lines.
|locked_lines| gives the number of lines that are locked,
starting from the top.

        @var int locked_lines

@ The widget's size can be too small to view all the rows and columns.
In that case |xoffset| and |yoffset| hold the coordinates of the pixel
in the virtual window that ends up in the upper left corner of the
real window.

	@var int xoffset
	@var int yoffset


@METHODS

@ The |initialize| method initializes the private variables
and sets the geometry resources to the values indicated by
|rows| and |columns|.

An event handler for the Map and Unmap events is registered, which
will in turn install or remove a timer that makes the cursor blink.

Note that |make_gc| uses |uline_thick|, which is set by
|compute_cell_size|.

The strange call to |XmuInternAtom| is necessary to initialize the Xmu
atom caching mechanism. |XA_TARGETS| is used by the selection
procedures.

The event handler for key presses is installed, because we don't want
translations to change the meaning of key presses. Also we don't want
other widgets to steal our keyboard input, not even for keyboard
traversal.

@proc initialize
{
    int framewd = 2 * compute_framewd($);

    (void) XmuInternAtom(XtDisplay($), XmuMakeAtom("NULL"));
    $fnt = XLoadQueryFont(XtDisplay($), $font);
    if (! $fnt)
	XtAppError(XtWidgetToApplicationContext($),
		   "Failed to find font in AnsiTerm");
    compute_cell_size($);

    if ($columns < 1) $columns = 1;
    if ($rows < 1) $rows = 1;
    $width = framewd + 2 * $margin + $columns * $cellwidth;
    $height = framewd + 2 * $margin + $rows * $cellheight;

    $allocated_columns = $allocated_rows = 0;
    $contents = $attribs = NULL;
    allocate_contents($);

    XtAddEventHandler($, StructureNotifyMask, FALSE, map_handler, NULL);
    XtInsertEventHandler($, KeyPressMask, FALSE, handle_keys, NULL,
			 XtListHead);

    $gc = $boldgc = $revgc = $revboldgc = NULL;
    make_gc($);

    $savedx = $savedy = $cursorx = $cursory = 0;
    $cursor_on = FALSE;
    $cur_attrib = 0;
    $insert_mode = FALSE;
    $last_char = ' ';
    $state = Init;
    $selection = NULL;
    $locked_lines = 0;

    $xoffset = 0;
    $yoffset = 0;
    if ($scrollResponse != scroll_response) {
	$scrollResponse = scroll_response;
	XtAppWarning(XtWidgetToApplicationContext($),
		     "scrollResponse resource may only be queried, not set");
    }
}

@ If the font, the margin or the number of rows/columns
changes, the |set_values| method will resize the widget in
order to keep the requested number of rows/columns. Only if
|width| or |height| is changed explicitly (and |rows| or
|columns| is not changed at the same time), will the |rows|
or |columns| be recomputed and changed.

@proc set_values
{
    Bool redraw = FALSE, need_gc = FALSE;
    int framewd = 2 * compute_framewd($);

    if ($font != $old$font || $boldfont != $old$boldfont) {
	if ($fnt) XFreeFont(XtDisplay($), $fnt);
	$fnt = XLoadQueryFont(XtDisplay($), $font);
	need_gc = TRUE;
	compute_cell_size($);
    }
    if ($foreground != $old$foreground
	|| $background != $old$background) {
	need_gc = TRUE;
    }

    if ($columns < 1) $columns = 1;		/* Silently increase */
    if ($rows < 1) $rows = 1;

    if ($width == $old$width) {
	/* If columns changed, but width didn't, recompute width */
	if ($columns != $old$columns
	    || $margin != $old$margin || $cellwidth != $old$cellwidth)
	    $width = framewd + 2 * $margin + $columns * $cellwidth;
    }
    if ($height == $old$height) {
	/* If rows changed, but height didn't, recompute height */
	if ($rows != $old$rows
	    || $margin != $old$margin || $cellheight != $old$cellheight)
	    $height = framewd + 2 * $margin + $rows * $cellheight;
    }
    if ($rows != $old$rows || $columns != $old$columns)
	allocate_contents($);
	
    if (need_gc) {
	make_gc($);
	redraw = TRUE;
    }
    if ($width != $old$width || $height != $old$height)
	redraw = FALSE;			/* Don't redraw if resizing */

    if ($cursorx >= $columns) $cursorx = $columns - 1;
    if ($cursory >= $rows) $cursory = $rows - 1;

    if ($scrollResponse != $old$scrollResponse) {
	$scrollResponse = $old$scrollResponse;
	XtAppWarning(XtWidgetToApplicationContext($),
		     "scrollResponse resource may only be queried, not set");
    }

    return redraw;
}

@ When the parent widget asks for the preferred size, the AnsiTerm
widget replies with the size that is needed for the current rows and
columns.

@proc query_geometry
{
    int framewd = compute_framewd($);

    /* Compute our preferred geometry */
    reply->request_mode = CWWidth | CWHeight;
    reply->width = framewd + 2 * $margin + $columns * $cellwidth;
    reply->height = framewd + 2 * $margin + $rows * $cellheight;

    if ((!(request->request_mode & CWWidth)
	 || request->width == reply->width)
	&& (!(request->request_mode & CWHeight)
	    || request->height == reply->height))
	return XtGeometryYes;			/* Accept the request */

    if ($width == reply->width && $height == reply->height)
	return XtGeometryNo;			/* Ask for no change */

    return XtGeometryAlmost;			/* We supply an alternative */
}

@ When the widget is realized, it notifies all connected scrolling
widgets of its relative size and position.

@proc realize
{
    #realize($, mask, attributes);
    send_scroll_notify($);
}

@ When the widget is resized, it notifies all connected scrolling
widgets of the new relative size and position.

It also sets the new clip regions in all GCs.

@proc resize
{
    send_scroll_notify($);
    clip_gcs($);
}

@ The |destroy| method frees the memory occupied by the
|contents| and |attribs| arrays. It also frees the GCs and the
fonts.

@proc destroy
{
    int i;

    if ($allocated_rows != 0) {
	for (i = 0; i < $allocated_rows; i++) {
	    XtFree($contents[i]);
	    XtFree($attribs[i]);
	}
	XtFree((char *) $contents);
	XtFree((char *) $attribs);
    }
    XtReleaseGC($, $gc);
    XtReleaseGC($, $boldgc);
    XtReleaseGC($, $revgc);
    XtReleaseGC($, $revboldgc);
    XFreeFont(XtDisplay($), $fnt);
    XFreeFont(XtDisplay($), $boldfont);
}

@ The |expose| method draws only the characters in the exposed region.
(More precisely: in the smallest rectangle enclosing the region, as
given by the fields of the event structure.) All text is offset by
|$xoffset| and |$yoffset|.

QUESTION: is it useful to maintain an off-screen copy of the
text in a Pixmap and redraw the screen from that? It might speed
up the redrawing, but it takes up server memory and some servers
may already have their own `save-under' optimizations.

@proc expose
{
    int top, bot, lft, rgt, framewd, y, x, i, w, h;
    XExposeEvent *ev = (XExposeEvent *) event;
    Region region1;
    XRectangle rect;

    if (! XtIsRealized($)) return;

    #expose($, event, region);			/* Superclass draws frame */

    $compute_inside($, &rect.x, &rect.y, &rect.width, &rect.height);
    framewd = rect.x;
    region1 = XCreateRegion();
    XUnionRectWithRegion(&rect, region1, region1);
    if (region) XIntersectRegion(region, region1, region1);
    XSetRegion(XtDisplay($), $gc, region1);
    XSetRegion(XtDisplay($), $boldgc, region1);
    XSetRegion(XtDisplay($), $revgc, region1);
    XSetRegion(XtDisplay($), $revboldgc, region1);

    x = ev ? ev->x + $xoffset : $xoffset;
    y = ev ? ev->y + $yoffset : $yoffset;
    w = ev ? ev->width : rect.width;
    h = ev ? ev->height : rect.height;

    find_cell($, framewd, x, y, &lft, &top);
    find_cell($, framewd, x + w, y + h, &rgt, &bot);

    if (lft < $columns && top < $rows)
	for (i = top; i <= bot; i++) draw_line($, framewd, i, lft, rgt + 1);

    XDestroyRegion(region1);
    clip_gcs($);				/* Reset clipping region */
}

@ The method |scroll_response| is exported via the |scrollResponse|
resource. It has the format of a callback function, so that it can be
registered as a callback in the |scrollCallback| list of some other
widget. The |client_data| must be a pointer to the widget
itself, the |call_data| is a pointer to an |XfwfScrollInfo| structure.
The widget |wdg| is the widget from whose callback list the function
is called.

The widget can only react to changes in x and y (|XFWF_VPOS| or
|XFWF_HPOS|), other fields in the scroll info are ignored.

The reason field can be |XfwfSMove, XfwfSDrag, XfwfSUp, XfwfSLeft,
XfwfSDown, XfwfSRight, XfwfSPageUp, XfwfSPageLeft, XfwfSPageDown,
XfwfSPageRight, XfwfSTop, XfwfSBottom, XfwfSLeftSide, XfwfSRightSide|.

@proc scroll_response(Widget wdg, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;
    XfwfScrollInfo new, *info = (XfwfScrollInfo *) call_data;
    int maxx, maxy, desiredwd, desiredht, newx, newy;
    XRectangle rect;

    newx = $xoffset;				/* Will become new xoffset */
    newy = $yoffset;				/* Will become new yoffset */
    desiredwd = 2 * $margin + $columns * $cellwidth; /* Virtual width */
    desiredht = 2 * $margin + $rows * $cellheight; /* Virtual height */
    $compute_inside($, &rect.x, &rect.y, &rect.width, &rect.height);
    maxx = max(0, desiredwd - rect.width);	/* Max. for xoffset */
    maxy = max(0, desiredht - rect.height);	/* Max. for yoffset */

    switch (info->reason) {
    case XfwfSUp:
	newy = max(0, newy - $cellheight);
	info->flags |= XFWF_VPOS;
	info->vpos = maxy != 0 ? (float) newy/maxy : 0.0;
	break;
    case XfwfSPageUp:
	newy = max(0, newy - rect.height);
	info->flags |= XFWF_VPOS;
	info->vpos = maxy != 0 ? (float) newy/maxy : 0.0;
	break;
    case XfwfSDown:
	newy = min(maxy, newy + $cellheight);
	info->flags |= XFWF_VPOS;
	info->vpos = maxy != 0 ? (float) newy/maxy : 0.0;
	break;
    case XfwfSPageDown:
	newy = min(maxy, newy + rect.height);
	info->flags |= XFWF_VPOS;
	info->vpos = maxy != 0 ? (float) newy/maxy : 0.0;
	break;
    case XfwfSLeft:
	newx = max(0, newx - $cellwidth);
	info->flags |= XFWF_HPOS;
	info->hpos = maxx != 0 ? (float) newx/maxx : 0.0;
	break;
    case XfwfSPageLeft:
	newx = max(0, newx - rect.width);
	info->flags |= XFWF_HPOS;
	info->hpos = maxx != 0 ? (float) newx/maxx : 0.0;
	break;
    case XfwfSRight:
	newx = min(maxx, newx + $cellwidth);
	info->flags |= XFWF_HPOS;
	info->hpos = maxx != 0 ? (float) newx/maxx : 0.0;
	break;
    case XfwfSPageRight:
	newx = min(maxx, newx + rect.width);
	info->flags |= XFWF_HPOS;
	info->hpos = maxx != 0 ? (float) newx/maxx : 0.0;
	break;
    case XfwfSDrag: case XfwfSMove:
	if (info->flags & XFWF_HPOS) newx = info->hpos * maxx;
	if (info->flags & XFWF_VPOS) newy = info->vpos * maxy;
	break;
    default:
	; /* skip */
    }
    if (newx != $xoffset || newy != $yoffset) {
	if (XtIsRealized($)) {
#if 0
	    XCopyArea(XtDisplay($), XtWindow($), XtWindow($), $gc,
		      -$xoffset, -$yoffset, desiredwd, desiredht,
		      -newx, -newy);
#else
	    XClearArea(XtDisplay($), XtWindow($), rect.x, rect.y,
		       rect.width, rect.height, TRUE);
#endif
	}
	$xoffset = newx;
	$yoffset = newy;
    }

    /* Pass (updated) info on to others in scrolling chain */
    if (info->reason != XfwfSNotify) {
	new = *info;
	new.reason = XfwfSNotify;
	XtCallCallbackList($, $scrollCallback, &new);
    }
}

@ The |write| method is the means by which characters are added
to the display. The function adds characters to the |contents|
array, wrapping and scrolling if necessary. It also interprets
ANSI escape codes. It implements a finite state machine to keep
track of its position in an escape sequence.

Actual drawing is postponed until all |n| characters have been
processed. At that point all lines with `dirty' flags on their
first character are redrawn. This is only efficient when |n| is
greater than one, it becomes more efficient with larger |n|.

The reason lies in the expectation that programs generate large
numbers of characters at once, but one character at a time is usually
the result of a user typing. In the latter case the small performance
penalty is probably not noticable. This hypothesis has not been tested
in practice, however.

(Note that the `dirty' flag is not reset when the widget is not
realized. This shouldn't matter much.)

@proc write($, char *text, int n)
{
    int framewd, i;

    for (i = 0; i < n; i++) {
        switch ($state) {
        case Init:
            switch (text[i]) {
            case '\007': XBell(XtDisplay($), 0); break; /* Bell */
            case '\015': $cursorx = 0; break;	/* CR */
            case '\011': next_tabstop($); break; /* Tab */
            case '\012': cursor_down($); break;	/* LF */
            case '\010': if ($cursorx) $cursorx--; break; /* Backspace */
            case '\033': $state = Esc; break;	/* Esc */
            default: add_char($, text[i]);
            }
            break;
        case Esc:
            switch (text[i]) {
            case '[': $state = EscLB; break;
	    case '2': $state = Init; break;	/* Set tab stop ignored */
	    case '7':
		$savedx = $cursorx;
		$savedy = $cursory;
		$state = Init;
		break;
	    case '8': goto_xy($, $savedx, $savedy); $state = Init; break;
            default:                            /* Unrecognized sequence */
                add_char($, '\033');
                add_char($, text[i]);
                $state = Init;
            }
	    break;
	case EscLB:
	    switch (text[i]) {
	    case 'H': $cursorx = $cursory = 0; $state = Init; break;
	    case 'J': clear_eos($); $state = Init; break;
	    case 'K': clear_eol($), $state = Init; break;
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		$escparm[0] = text[i] - '0';
		$state = Register0;
		break;
	    case 'C': 
		if ($cursorx < $columns - 1) $cursorx++;
		$state = Init;
		break;
	    case 'A': if ($cursory) $cursory--; $state = Init; break;
	    case 'P': delete_char($); $state = Init; break;
	    case 'M': delete_line($); $state = Init; break;
	    case 'm': $cur_attrib = 0; $state = Init; break;
	    case '@': insert_chars($, 1); $state = Init; break;
	    case 'L': insert_line($); $state = Init; break;
	    case '>': $state = EscLBGT; break;
	    case 's':
		$savedx = $cursorx;
		$savedy = $cursory;
		$state = Init;
		break;
	    case 'u': goto_xy($, $savedx, $savedy); $state = Init; break;
	    case 'r': $state = Init; break;	/* Set scroll region ignored */
	    default:				/* Unrecognized sequence */
		add_char($, '\033');
		add_char($, '[');
		add_char($, text[i]);
		$state = Init;
	    }
		break;
	case Register0:
	    switch (text[i]) {
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		$escparm[0] = 10 * $escparm[0] + text[i] - '0';
		break;
	    case ';': $escparm[1] = 0; $state = Register1; break;
	    case 'm': set_attrib($, 1); $state = Init; break;
	    case 'h': $insert_mode = TRUE; $state = Init; break;
	    case 'l': $insert_mode = FALSE; $state = Init; break;
	    case '@': insert_chars($, $escparm[0]); $state = Init; break;
	    case 'b': repeat_char($, $escparm[0] - 1); $state = Init; break;
	    case 'H': case 'f':
                goto_xy($, 0, $escparm[0] - 1);
		$state = Init;
		break;
	    case 'n': report_cursor_pos($); $state = Init; break;
	    case 'j':
		$cursorx = $cursory = 0;
		clear_eos($);
		$state = Init;
		break;
	    case 'r': $state = Init; break;	/* Set scroll region ignored */
	    default: $state = Init;		/* Ignore unknown sequence */
	    }
	    break;
	case Register1:
	    switch (text[i]) {
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		$escparm[1] = 10 * $escparm[1] + text[i] - '0';
		break;
	    case ';': $escparm[2] = 0; $state = Register2; break;
	    case 'm': set_attrib($, 2); $state = Init; break;
	    case 'H': case 'f':
                goto_xy($, $escparm[1] - 1, $escparm[0] - 1);
		$state = Init;
		break;
	    case 'r': $state = Init; break;	/* Set scroll region ignored */
	    default: $state = Init;		/* Ignore unknown sequence */
	    }
	    break;
	case Register2:
	    switch (text[i]) {
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		$escparm[2] = 10 * $escparm[2] + text[i] - '0';
		break;
	    case ';': $escparm[3] = 0; $state = Register3; break;
	    case 'm': set_attrib($, 3); $state = Init; break;
	    default: $state = Init;		/* Ignore unknown sequence */
	    }
	    break;
	case Register3:
	    switch (text[i]) {
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		$escparm[3] = 10 * $escparm[3] + text[i] - '0';
		break;
	    case ';': $escparm[4] = 0; $state = Register4; break;
	    case 'm': set_attrib($, 4); $state = Init; break;
	    default: $state = Init;		/* Ignore unknown sequence */
	    }
	    break;
	case Register4:
	    switch (text[i]) {
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
		$escparm[4] = 10 * $escparm[4] + text[i] - '0';
		break;
	    case 'm': set_attrib($, 5); $state = Init; break;
	    default: $state = Init;		/* Ignore unknown sequence */
	    }
	    break;
	case EscLBGT:				/* Seen \E[> */
	    switch (text[i]) {
	    case '2': $state = EscLBGT2; break;
	    default:
		add_char($, '\033');
		add_char($, '[');
		add_char($, '>');
		add_char($, text[i]);
		$state = Init;
	    }
	    break;
	case EscLBGT2:				/* Seen \E[>2 */
	    switch (text[i]) {
	    case 'h': memory_lock($); $state = Init; break;
	    case 'l': memory_unlock($); $state = Init; break;
	    default:
		add_char($, '\033');
		add_char($, '[');
		add_char($, '>');
		add_char($, '2');
		add_char($, text[i]);
		$state = Init;
	    }
		break;
	default:
	    assert(! "Cannot happen");
	}
    }

    /* Finally redraw all lines that are marked as `dirty' */
    if (XtIsRealized($)) {
	framewd = compute_framewd($);
	for (i = 0; i < $rows; i++)
	    if ($attribs[i][0] & ATTRIB_DIRTY) {
		$attribs[i][0] &= ~ATTRIB_DIRTY;
		draw_line($, framewd, i, 0, $columns);
	    }
	do toggle_cursor($); while (! $cursor_on);
    }
}


@TRANSLATIONS

	@trans Shift<Btn1Down>: extend_selection()
	@trans <Btn1Down>: start_selection() traverseCurrent()
	@trans <Btn1Motion>: extend_selection()
	@trans <Btn1Up>: end_selection()
	@trans <Btn2Down>: paste_selection()
	@trans <Map>: traverseCurrent()


@ACTIONS

@ The |key| action procedure calls the |keyCallback| callback
procedure with the pressed key as |call_data|. Nothing happens
if the key doesn't have a representation as (a sequence of)
characters.

If |XLookupString| returns a character representation, that
representation is used, otherwise the keysym is checked for some
special keys (Return, Backspace, etc.). Finally, if the user
pressed Control + letter, the character returned is (letter -
64).

@proc key
{
    char buf[15];
    KeySym keysym;
    int n, i;
    
    if (event->type != KeyPress && event->type != KeyRelease)
	XtError("key() action in AnsiTerm not associated with key events");

    n = XLookupString(&event->xkey, buf, sizeof(buf), &keysym, NULL);
    if (n == 0)
	switch (keysym) {
	case XK_BackSpace:  n = 1; buf[0] = '\010';	    break;
	case XK_Home:	    n = 3; strcpy(buf, "\033[H");   break;
	case XK_Down:	    n = 3; strcpy(buf, "\033[B");   break;
	case XK_Left:	    n = 3; strcpy(buf, "\033[D");   break;
	case XK_Right:	    n = 3; strcpy(buf, "\033[C");   break;
	case XK_Up:	    n = 3; strcpy(buf, "\033[A");   break;
	case XK_Return:
	case XK_KP_Enter:
	case XK_Linefeed:   n = 1; buf[0] = '\015';	    break;
	case XK_Tab:	    n = 1; buf[0] = '\011';	    break;
	case XK_Delete:     n = 1; buf[0] = '\177';	    break;
	default:
	    if ((event->xkey.state & ControlMask)
		&& 65 <= keysym && keysym <= 95) {
		buf[n++] = keysym - 64;	/* Control + letter */
	    }
	}
    for (i = 0; i < n; i++) XtCallCallbackList($, $keyCallback, &buf[i]);
}

@ The selection mechanism consist of three action procedures for
highlighting a selection and one for pasting text into the
widget.

|start_selection| establishes the start of the highlighted
selection from the mouse position.  |extend_selection|
highlights the text between the start position and the current
mouse position. |end_selection| copies the highlighted part to a
buffer and notifies the X server that this widget wants to be
the current selection owner.

|paste_selection| requests the current selection from whatever
application has it and processes it as if the user had typed it;
i.e., it calls the |keyCallback| for every character.

@proc start_selection
{
    find_cell($, compute_framewd($), event->xbutton.x + $xoffset,
	      event->xbutton.y + $yoffset, &$start_x, &$start_y);
    $end_x = $start_x;
    $end_y = $start_y;
    $drag_started = FALSE;			/* No selection yet */
}

@ |extend_selection| is called when the mouse is dragged over
the text. It finds the cell that the mouse is on and highlights
all cells from the one where the drag started to the current one
(both inclusive).

The function only draws the lines that the mouse passed between
the previous and the current event.

@proc extend_selection
{
    int i, j, framewd, sx, sy, ex, ey, x, y;

    $drag_started = TRUE;
    framewd = compute_framewd($);
    find_cell($, framewd, event->xbutton.x + $xoffset,
	      event->xbutton.y + $yoffset, &x, &y);

    if ($start_y < y
        || ($start_y == y && $start_x < x)) {
        sy = $start_y;  ey = y;
        sx = $start_x;  ex = x;
    } else {                                   /* Swap start and end */
        sy = y;  ey = $start_y;
        sx = x;  ex = $start_x;
    }    

    /* Draw the lines that the mouse passed over since last event */
    for (i = min(y, $end_y); i <= max(y, $end_y); i++) {

        /* Toggle REV attribute on cells within selected region */
        if (i == sy && i == ey)
            for (j = sx; j <= ex; j++) $attribs[i][j] ^= ATTRIB_REV;
        else if (i == sy)
            for (j = sx; j < $columns; j++) $attribs[i][j] ^= ATTRIB_REV;
        else if (i == ey)
            for (j = 0; j < ex; j++) $attribs[i][j] ^= ATTRIB_REV;
        else if (sy < i && i < ey)
            for (j = 0; j < $columns; j++) $attribs[i][j] ^= ATTRIB_REV;

        draw_line($, framewd, i, 0, $columns);	/* Draw the line */

        /* Toggle REV attribute on cells in region again */
        if (i == sy && i == ey)
            for (j = sx; j <= ex; j++) $attribs[i][j] ^= ATTRIB_REV;
        else if (i == sy)
            for (j = sx; j < $columns; j++) $attribs[i][j] ^= ATTRIB_REV;
        else if (i == ey)
            for (j = 0; j < ex; j++) $attribs[i][j] ^= ATTRIB_REV;
        else if (sy < i && i < ey)
            for (j = 0; j < $columns; j++) $attribs[i][j] ^= ATTRIB_REV;
    }

    $end_x = x;
    $end_y = y;
}

@ When the mouse button is released, the selection is complete
and the highlighted part is copied to a buffer. The widget then
tells the X server that it wants to become the selection owner
for the PRIMARY selection.

A simple click should not be taken as a selection. If there has
been no movement between the mouse press and release, the
|end_selection| action simply returns without doing anything.

The highlight is immediately removed from the screen. This is
easy to implement but it removes a visual indicator that some
people may want to leave there while the selection is active.
Something to reconsider for the next version?

@proc end_selection
{
    int sx, sy, ex, ey, k, i, j, framewd;

    if (! $drag_started) return;		/* No movement since BtnDown */

    if ($start_y < $end_y
        || ($start_y == $end_y && $start_x < $end_x)) {
        sy = $start_y;  ey = $end_y;
        sx = $start_x;  ex = $end_x;
    } else {					/* Swap start & end */
        sy = $end_y;  ey = $start_y;
        sx = $end_x;  ex = $start_x;
    }

    /* Unhighlight the selection */
    framewd = compute_framewd($);
    if (sy == ey) {
        draw_line($, framewd, sy, sx, ex + 1);
    } else {
        draw_line($, framewd, sy, sx, $columns); /* First (partial) line */
        for (i = sy + 1; i <= ey - 1; i++)      /* Middle lines */
            draw_line($, framewd, i, 0, $columns);
        draw_line($, framewd, ey, 0, ex + 1);   /* Last (partial) line */
    }

    /* Copy selection to buffer */
    if (sy == ey) {

        $selection = XtRealloc($selection, ex - sx + 2);
        for (k = 0, j = sx; j <= ex; k++, j++)
            $selection[k] = $contents[sy][j];
        $selection[k] = '\0';
        $selection_len = k;

    } else {

        $selection = XtRealloc($selection,
            $columns - sx + 1 + (ey - sy - 1) * ($columns + 1) + ex + 3);
        k = 0;
        for (j = sx; j < $columns; j++, k++)
            $selection[k] = $contents[sy][j];
        $selection[k++] = '\n';
        for (i = sy + 1; i <= ey - 1; i++) {
            for (j = 0; j < $columns; j++, k++)
                $selection[k] = $contents[i][j];
            $selection[k++] = '\n';
        }
        for (j = 0; j <= ex; j++, k++)
            $selection[k] = $contents[ey][j];
        $selection[k] = '\0';
        $selection_len = k;
    }

    /* Now ask the X server for ownership of the selection */
    if (XtOwnSelection($, XA_PRIMARY, event->xbutton.time,
		       convert_proc, lose_ownership_proc, NULL) == FALSE)
        XtAppWarning(XtWidgetToApplicationContext($),
		     "Failed to become selection owner");
}

@proc paste_selection
{
    XtGetSelectionValue($, XA_PRIMARY, XA_STRING, paste_callback, NULL,
	event->xbutton.time);
}


@UTILITIES

@def max(a, b) = ((a) > (b) ? (a) : (b))
@def min(a, b) = ((a) < (b) ? (a) : (b))

@ The |draw_line| function is called by |expose| to draw the
characters in line |row| between columns |lft| and |rgt|. It
draws runs of characters with the same attributes.

@proc draw_line($, int framewd, int row, int lft, int rgt)
{
    Display *dpy = XtDisplay($);
    Window win = XtWindow($);
    int x, y, i, n;
    GC use_gc;

    x = framewd + $margin + lft * $cellwidth - $xoffset;
    y = framewd + $margin + row * $cellheight + $fnt->ascent - $yoffset;
    while (lft < rgt) {
	i = lft + 1;
	while (i < rgt && $attribs[row][i] == $attribs[row][lft]) i++;
        if ($attribs[row][lft] & ATTRIB_INVIS) {
            XClearArea(dpy, win, x, y - $fnt->ascent, n * $cellwidth,
		       $cellheight, FALSE);
        } else {
	    switch ($attribs[row][lft] & (ATTRIB_REV | ATTRIB_BOLD)) {
	    case 0: use_gc = $gc; break;
	    case ATTRIB_BOLD: use_gc = $boldgc; break;
	    case ATTRIB_REV: use_gc = $revgc; break;
	    case ATTRIB_REV | ATTRIB_BOLD: use_gc = $revboldgc; break;
	    }
	    n = i - lft;
	    XDrawImageString(dpy, win, use_gc, x, y, &$contents[row][lft], n);
	    if ($attribs[row][lft] & ATTRIB_ULINE)
		XDrawLine(dpy, win, use_gc, x, y + $uline_pos,
			  x + n * $cellwidth - 1, y + $uline_pos);
	}
	lft = i;
	x += n * $cellwidth;
    }
}

@ The |convert_proc| function is a callback called by Xt when
some application requests the selection. It checks if the
requested type is |XA_STRING|, since it can only give data in
that format.

@proc Boolean convert_proc($, Atom *selection, Atom *target, Atom *type_return, XtPointer *value_return, unsigned long *length_return, int *format_return)
{
    if (*target == XA_TARGETS(XtDisplay($))) {

	XSelectionRequestEvent *req;
	Atom *targetP;
	Atom *std_targets;
	unsigned long std_length;

	req = XtGetSelectionRequest($, *selection, NULL);
	XmuConvertStandardSelection($, req->time, selection, target,
	    type_return, (XtPointer *) &std_targets, &std_length,
	    format_return);
	*value_return = XtMalloc(sizeof(Atom) * (std_length + 1));
	targetP = *(Atom **) value_return;
	*length_return = std_length + 1;
	*targetP++ = XA_STRING;
	bcopy((char*)std_targets, (char*)targetP, sizeof(Atom) * std_length);
	XtFree((char *) std_targets);
	*type_return = XA_ATOM;
	*format_return = sizeof(Atom) * 8;
	return TRUE;

    } else if (*target == XA_STRING) {
	*value_return = $selection;
	*type_return = XA_STRING;
	*length_return = $selection_len;
	*format_return = 8;
	return TRUE;
	
    } else {

	if (XmuConvertStandardSelection($, CurrentTime, selection, target,
		type_return, value_return, length_return, format_return))
	    return TRUE;
	else
	    return FALSE;
    }
}

@ The |lose_ownership_proc| is a callback that is called when
the widget loses selection ownership. Since the AnsiTerm widget
already unhighlighted the selection when the mouse was released,
there is nothing for this function to do.

@proc lose_ownership_proc($, Atom *selection) {}

@ The |paste_callback| is installed by the |paste_selection|
action procedure. It receives the selection data and `pastes' it
into the AnsiTerm. In this case that means it is passed
character for character to the |keyCallback| function.

@proc paste_callback($, XtPointer client_data, Atom *selection, Atom *type, XtPointer value, unsigned long *length, int *format)
{
    char *text = (char *) value;
    int i;

    assert(*selection == XA_PRIMARY && *type == XA_STRING && *format == 8);

    if (value == NULL && *length == 0) {
	XBell(XtDisplay($), 0);
	return;
    }
    for (i = 0; i < *length; i++)
	XtCallCallbackList($, $keyCallback, (XtPointer) &text[i]);
    XtFree(value);
}

@ |compute_cell_size| compares the character sizes from the
normal and the bold font and sets |cellwidth| and |cellheight|
to the character size of the larger of the two.

@proc compute_cell_size($)
{
    unsigned long h1, h2;

    if (! XGetFontProperty($fnt, XA_QUAD_WIDTH, &h1))
	h1 = XTextWidth($fnt, "M", 1);
    if (! XGetFontProperty($boldfont, XA_QUAD_WIDTH, &h2))
	h2 = XTextWidth($boldfont, "M", 1);
    $cellwidth = max(h1, h2);

    h1 = $fnt->ascent + $fnt->descent;
    h2 = $boldfont->ascent + $boldfont->descent;
    $cellheight = max(h1, h2);

    if (! XGetFontProperty($fnt, XA_UNDERLINE_POSITION, &h1)) h1 = 2;
    if (! XGetFontProperty($boldfont, XA_UNDERLINE_POSITION, &h2)) h2 = 2;
    $uline_pos = max(h1, h2);

    if (! XGetFontProperty($fnt, XA_UNDERLINE_THICKNESS, &h1)) h1 = 1;
    if (! XGetFontProperty($boldfont, XA_UNDERLINE_THICKNESS, &h2)) h2 = 1;
    $uline_thick = max(h1, h2);
}

@proc int compute_framewd($)
{
    Position x, y;
    Dimension w, h;

    $compute_inside($, &x, &y, &w, &h);
    return x;
}

@ The |allocate_contents| function first deallocates the memory
for the lines that are no longer needed or allocates memory if
the number of lines has grown. It then lengthens or shortens
each line to the new number of columns. New lines and columns
are initialized to spaces, with default (=zero) attributes.

@def myrealloc(p, n) = ((XtPointer)XtRealloc((XtPointer)(p), (n)*sizeof(*(p))))

@proc allocate_contents($)
{
    int i, j;

    /* Remove superfluous lines */
    for (i = $rows; i < $allocated_rows; i++) {
	XtFree($contents[i]);
	XtFree($attribs[i]);
    }
    /* Allocate and initialize new lines */
    $contents = myrealloc($contents, $rows);
    $attribs = myrealloc($attribs, $rows);
    for (i = $allocated_rows; i < $rows; i++) {
	$contents[i] = XtMalloc($columns);
	$attribs[i] = XtMalloc($columns);
	for (j = 0; j < $columns; j++) {
	    $contents[i][j] = ' ';
	    $attribs[i][j] = 0;
	}
    }
    /* Lengthen or shorten existing lines */
    if ($allocated_columns != $columns) {
	for (i = 0; i < min($allocated_rows, $rows); i++) {
	    $contents[i] = myrealloc($contents[i], $columns);
	    $attribs[i] = myrealloc($attribs[i], $columns);
	    for (j = $allocated_columns; j < $columns; j++) {
		$contents[i][j] = ' ';
		$attribs[i][j] = 0;
	    }
	}
    }
    $allocated_rows = $rows;
    $allocated_columns = $columns;
}

@ The |blink_handler| function is called every 500 milliseconds
to blink the cursor. It draws the character cell under the
cursor alternately in its normal face and in reverse.
It reinstalls itself.

@proc toggle_cursor($)
{
    if ($cursor_on) {				/* Restore to normal */
	draw_line($, compute_framewd($), $old_cy, $old_cx, $old_cx + 1);
    } else if (! $traversalOn || $traversal_focus) { /* Draw in reverse */
	$attribs[$cursory][$cursorx] ^= ATTRIB_REV;
	draw_line($, compute_framewd($), $cursory, $cursorx, $cursorx + 1);
	$attribs[$cursory][$cursorx] ^= ATTRIB_REV;
    } else {					/* Draw with underline */
	$attribs[$cursory][$cursorx] ^= ATTRIB_ULINE;
	draw_line($, compute_framewd($), $cursory, $cursorx, $cursorx + 1);
	$attribs[$cursory][$cursorx] ^= ATTRIB_ULINE;
    }
    $old_cx = $cursorx;
    $old_cy = $cursory;
    $cursor_on = ! $cursor_on;
}

@proc blink_handler(XtPointer client_data, XtIntervalId *id)
{
    Widget $ = (Widget) client_data;
    toggle_cursor($);
    $timer = XtAppAddTimeOut(XtWidgetToApplicationContext($),
			     500, blink_handler, $);
}

@ The |map_handler| function is installed by |initialize| as an
event handler for Map and Unmap events. It will install (on Map)
or remove (on Unmap) the timer function that blinks the cursor.

TO DO: this can be made a little more efficient by checking the
widget's |visibilty| field (after setting the class variable
|visible_interest| to |TRUE|) and installing a timer for
blinking only when the widget is visible and a timer for
periodically checking |visible| otherwise.

@proc map_handler($, XtPointer client_data, XEvent *event, Boolean *cont)
{
    if (event->type == MapNotify)
	$timer = XtAppAddTimeOut(XtWidgetToApplicationContext($),
				 500, blink_handler, $);
    else if (event->type == UnmapNotify)
	XtRemoveTimeOut($timer);
}

@ |send_scroll_notify| is called by |realize| and |resize| to let all
widgets in the scolling chain know about the relative size and postion
of the scrolling window in the AnsiTerm.

@proc send_scroll_notify($)
{
    int framewd, width, height, desiredwd, desiredht;
    XfwfScrollInfo info;

    framewd = compute_framewd($);
    width = $width - 2 * framewd;
    height = $height - 2 * framewd;
    desiredwd = framewd + 2 * $margin + $columns * $cellwidth;
    desiredht = framewd + 2 * $margin + $rows * $cellheight;

    info.reason = XfwfSNotify;
    info.flags = XFWF_VPOS | XFWF_VSIZE | XFWF_HPOS | XFWF_HSIZE;
    info.hpos = width<desiredwd ? (float)$xoffset/(desiredwd - width) : 0.0;
    info.vpos = height<desiredht ? (float)$yoffset/(desiredht - height) : 0.0;
    info.hsize = width < desiredwd ? (float)width/desiredwd : 1.0;
    info.vsize = height < desiredht ? (float)height/desiredht : 1.0;
    XtCallCallbackList($, $scrollCallback, &info);
}

@ |clip_gcs| sets the clipping regions in all GCs. It is called
wenever the GC's change or the widget resizes.

@proc clip_gcs($)
{
    XRectangle rect;
    $compute_inside($, &rect.x, &rect.y, &rect.width, &rect.height);
    XSetClipRectangles(XtDisplay($), $gc, 0, 0, &rect, 1, YXSorted);
    XSetClipRectangles(XtDisplay($), $boldgc, 0, 0, &rect, 1, YXSorted);
    XSetClipRectangles(XtDisplay($), $revgc, 0, 0, &rect, 1, YXSorted);
    XSetClipRectangles(XtDisplay($), $revboldgc, 0, 0, &rect, 1, YXSorted);
}

@ The |make_gc| function is called by |initialize| and
|set_values| every time the fonts or the colors change.

These GCs are always clipped to the part inside the frame.

@proc make_gc($)
{
    XGCValues values;
    XtGCMask mask, dynmask, dontmask;

    mask = GCFont | GCForeground | GCBackground | GCLineWidth;
    dynmask = GCClipMask;
    dontmask = GCArcMode | GCDashOffset | GCDashList | GCFillRule |
	GCFillStyle | GCJoinStyle | GCStipple | GCFillRule | GCTile |
	GCTileStipXOrigin | GCTileStipYOrigin;

    if ($gc) XtReleaseGC($, $gc);
    values.font = $fnt->fid;
    values.foreground = $foreground;
    values.background = $background;
    values.line_width = $uline_thick;
    $gc = XtAllocateGC($, 0, mask, &values, dynmask, dontmask);

    if ($boldgc) XtReleaseGC($, $boldgc);
    values.font = $boldfont->fid;
    values.foreground = $foreground;
    values.background = $background;
    values.line_width = $uline_thick;
    $boldgc = XtAllocateGC($, 0, mask, &values, dynmask, dontmask);

    if ($revgc) XtReleaseGC($, $revgc);
    values.font = $fnt->fid;
    values.foreground = $background;
    values.background = $foreground;
    values.line_width = $uline_thick;
    $revgc = XtAllocateGC($, 0, mask, &values, dynmask, dontmask);

    if ($revboldgc) XtReleaseGC($, $revboldgc);
    values.font = $boldfont->fid;
    values.foreground = $background;
    values.background = $foreground;
    values.line_width = $uline_thick;
    $revboldgc = XtAllocateGC($, 0, mask, &values, dynmask, dontmask);

    clip_gcs($);
}

@ The |add_char| function is called by the |write| method to add
a character at the current cursor position. The cursor is then
moved to the right (unless it is already at the right margin).

When |insert_mode| is on, the characters that are on the same
line to the right of the cursor are shifted to the right.

The first character of the modified line is marked with the
|ATTRIB_DIRTY| flag, to notify the |write| that this line should
be redrawn.

@proc add_char($, int c)
{
    if ($insert_mode) {
        bcopy(&$contents[$cursory][$cursorx],
              &$contents[$cursory][$cursorx+1], $columns - $cursorx - 1);
        bcopy(&$attribs[$cursory][$cursorx],
              &$attribs[$cursory][$cursorx+1], $columns - $cursorx - 1);
        $contents[$cursory][$cursorx] = c;
        $attribs[$cursory][$cursorx] = $cur_attrib;
    } else {
        $contents[$cursory][$cursorx] = c;
        $attribs[$cursory][$cursorx] = $cur_attrib;
    }
    $attribs[$cursory][0] |= ATTRIB_DIRTY;
    if ($cursorx < $columns - 1)
        $cursorx++;
    else {					/* Margin reached */
        $cursorx = 0;				/* CR */
        cursor_down($);				/* LF */
    }
    $last_char = c;				/* Save for repeat_char() */
}

@ The |repeat_char| functions inserts |n| copies of the most recently
inserted character.

@proc repeat_char($, int n)
{
    for (; n > 0; n--) add_char($, $last_char);
}

@ |next_tabstop| moves the cursor to the next tab stop. In
overwrite mode, the cursor is moved and the line is not changed;
in insert mode up to 7 spaces are inserted and the line is
marked for redrawing.

@proc next_tabstop($)
{
    if (! $insert_mode) {
	$cursorx = (($cursorx + 8)/8) * 8;
    } else {
	int x = (($cursorx + 8)/8) * 8;
	if (x >= $columns) x = $columns - 1;
	while ($cursorx != x) add_char($, ' ');
	$attribs[$cursory][0] |= ATTRIB_DIRTY;
    }
}

@ The |delete_line| function deletes the line on which the
cursor is, shifting all lower lines up. The cursor doesn't move.
All lines that changed are marked for redrawing. The deleted
line is reinserted at the bottom and cleared. (This way no
memory is lost.)

@proc delete_line($)
{
    int i;
    char *swap_contents, *swap_attribs;

    swap_contents = $contents[$cursory];
    swap_attribs = $attribs[$cursory];
    for (i = $cursory; i < $rows - 1; i++) {
	$contents[i] = $contents[i+1];
	$attribs[i] = $attribs[i+1];
	$attribs[i][0] |= ATTRIB_DIRTY;
    }
    $contents[$rows-1] = swap_contents;
    $attribs[$rows-1] = swap_attribs;
    memset(&$contents[$rows-1][0], ' ', $columns);
    memset(&$attribs[$rows-1][0], 0, $columns);
    $attribs[$rows-1][0] |= ATTRIB_DIRTY;
}

@ The |cursor_down| function moves the cursor to the next lower
line or scrolls the text up.

@proc cursor_down($)
{
    int save_y;
    
    if ($cursory < $rows - 1)
	$cursory++;
    else {
	save_y = $cursory;
        $cursory = $locked_lines;
	delete_line($);
	$cursory = save_y;
    }
}

@ The |clear_eol| function clears the text from the cursor to
the end of the line.

@proc clear_eol($)
{
    memset(&$contents[$cursory][$cursorx], ' ', $columns - $cursorx);
    memset(&$attribs[$cursory][$cursorx], 0, $columns - $cursorx);
    $attribs[$cursory][0] |= ATTRIB_DIRTY;
}

@ The |clear_eos| function clears the text from the cursor to
the end of the screen.

@proc clear_eos($)
{
    int i, save_x, save_y;

    clear_eol($);
    save_x = $cursorx;
    save_y = $cursory;
    $cursorx = 0;
    for (i = save_y + 1; i < $rows; i++) {
	$cursory = i;
	clear_eol($);
    }
    $cursorx = save_x;
    $cursory = save_y;
}

@ |delete_char| removes the character under the cursor and
shifts the rest of the line one position to the left.

@proc delete_char($)
{
    bcopy(&$contents[$cursory][$cursorx+1], &$contents[$cursory][$cursorx],
          $columns - $cursorx - 1);
    bcopy(&$attribs[$cursory][$cursorx+1], &$attribs[$cursory][$cursorx],
          $columns - $cursorx - 1);
    $contents[$cursory][$columns-1] = ' ';
    $attribs[$cursory][$columns-1] = 0;
    $attribs[$cursory][0] |= ATTRIB_DIRTY;
}

@ |insert_chars| inserts |n| spaces, shifting the rest of the
line to the right.

@proc insert_chars($, int n)
{
    if (n > $columns - $cursorx) n = $columns - $cursorx;
    bcopy(&$contents[$cursory][$cursorx], &$contents[$cursory][$cursorx+n],
          $columns - $cursorx - n);
    bcopy(&$attribs[$cursory][$cursorx], &$attribs[$cursory][$cursorx+n],
          $columns - $cursorx - n);
    memset(&$contents[$cursory][$cursorx], ' ', n);
    memset(&$attribs[$cursory][$cursorx], 0, n);
    $attribs[$cursory][0] |= ATTRIB_DIRTY;
}

@ |insert_line| inserts a blank line at the cursor's line,
moving the existing line and all lines below it down. The line
that is pushed off the bottom is reinserted at the cursor and
cleared.

@proc insert_line($)
{
    int i;
    char *swap_contents, *swap_attribs;

    swap_contents = $contents[$rows-1];
    swap_attribs = $attribs[$rows-1];
    for (i = $rows - 1; i > $cursory; i--) {
	$contents[i] = $contents[i-1];
	$attribs[i] = $attribs[i-1];
	$attribs[i][0] |= ATTRIB_DIRTY;
    }
    $contents[$cursory] = swap_contents;
    $attribs[$cursory] = swap_attribs;
    memset($contents[$cursory], ' ', $columns);
    memset($attribs[$cursory], 0, $columns);
    $attribs[$cursory][0] |= ATTRIB_DIRTY;
}

@ |set_attrib| sets the current text attributes. The parameter
|n| tells the function how many of the registers contain useful
codes.	Each of these registers is interpreted.

@proc set_attrib($, int n)
{
    int i;

    for (i = 0; i < n; i++) {
	switch($escparm[i]) {
	case 0: $cur_attrib = 0; break;
	case 1: $cur_attrib |= ATTRIB_BOLD; break;
	case 4: $cur_attrib |= ATTRIB_ULINE; break;
	case 5: $cur_attrib |= ATTRIB_BOLD; break; /* Should be: blinking */
	case 7: $cur_attrib |= ATTRIB_REV; break;
        case 8: $cur_attrib |= ATTRIB_INVIS; break;
	}
    }
}

@ |memory_lock| and |memory_unlock| lock and unlock lines from
the top of the screen. |memory_lock| locks all lines above the
cursor.


@proc memory_lock($)
{
    $locked_lines = $cursory;                  /* Lock lines above cursor */
}

@proc memory_unlock($)
{
    $locked_lines = 0;
}

@ |goto_xy| positions the cursor at the given coordinates, making sure
they fall within the current extend.

@proc goto_xy($, int x, int y)
{
    $cursorx = max(0, min($columns - 1, x));
    $cursory = max(0, min($rows - 1, y));
}

@ |report_cursor_pos| sends back the current cursor position via
the |keyCallback| callback (as if the user had typed it). The
returned data is in the format `ESC [ r ; c R' (where r = row +
1 and c = column + 1).

@proc report_cursor_pos($)
{
    char s[25];
    int i, n;

    n = sprintf(s, "\033[%d;%dR", $cursory + 1, $cursorx + 1);
    for (i = 0; i < n; i++) XtCallCallbackList($, $keyCallback, &s[i]);
}

@ |find_cell| computes the coordinates of the cell that contains
(cx,cy) or that is closest to it. (cx,cy) are relative to the virtual
window, not the real window (i.e., |$xoffset| and |$yoffset| have
already been added).

@proc find_cell($, int framewd, int cx, int cy, int *col, int *row)
{
    int i, j, x, y;

    for (x = framewd + $margin + $cellwidth, i = 0;
	i < $columns - 1 && x < cx;
	i++, x += $cellwidth) ;

    for (y = framewd + $margin + $cellheight, j = 0;
	j < $rows - 1 && y < cy;
	j++, y += $cellheight) ;

    *col = i;
    *row = j;
}

@ The function |parse_font_name| makes a copy of the font name into
|table[0]| (room is allocated on the heap). All hyphens are then
replaced by |\0| and the other 14 entries in |table| are made to point
to the start of each part of the name.

@def F_FOUNDRY = 1
@def F_FAMILY = 2
@def F_WEIGHT = 3
@def F_SLANT = 4
@def F_SET_WIDTH = 5
@def F_SANS = 6
@def F_PIXELS = 7
@def F_POINTS = 8
@def F_HRESOLUTION = 9
@def F_VRESOLUTION = 10
@def F_SPACING = 11
@def F_AVG_WITH = 12
@def F_CHARSET = 13

@proc Boolean parse_font_name(String name, String *table)
{
    int i, field;

    if (name == NULL || name[0] != '-') return False;
    table[0] = XtNewString(name);
    for (i = field = 0; name[i] != '\0'; i++) {
	if (name[i] == '-') {
	    field++;
	    table[0][i] = '\0';
	    table[field] = &table[0][i+1];
	    if (field == 13) break;		/* No more fields */
	}
    }
    return True;
}

@ |infer_bold| tries to find a bold version of the |font|
resource. It parses the |font| name and tries to form a new name
out of it. If that doesn't result in a valid font name, it uses
the |font| resource unchanged.

@proc infer_bold($, int offset, XrmValue* value)
{
    static XFontStruct *fs = NULL;
    String info[15];
    char s[250];

    if ($font == NULL)
	XtAppError(XtWidgetToApplicationContext($),
		   "Font resource may not be NULL in AnsiTerm widget");

    if (parse_font_name($font, info)) {
	sprintf(s, "-%s-%s-%s-%s-*-%s-*-%s-%s-%s-%s-*-%s",
		info[F_FOUNDRY], info[F_FAMILY], "bold",
		info[F_SLANT], /* info[F_SET_WIDTH], */ info[F_SANS],
		/* info[F_PIXELS], */ info[F_POINTS], info[F_HRESOLUTION],
		info[F_VRESOLUTION], info[F_SPACING],
		/* info[F_AVG_WIDTH], */ info[F_CHARSET]);
	fs = XLoadQueryFont(XtDisplay($), s);
    }
    if (! fs)					/* Try font unchanged */
	fs = XLoadQueryFont(XtDisplay($), $font);
    if (! fs)
	XtAppWarning(XtWidgetToApplicationContext($),
		   "Couldn't find font for AnsiTerm widget");
    value->addr = (XtPointer) &fs;
}

@ |handle_keys| is an event handler that is
installed by the |initialize| method to handle all key
presses. When a key press could be interpreted, the |cont| parameter
is set to |FALSE|, so that the key is not used again for other
purposes (such as keyboard traversal, in the case of the Tab key).

@proc handle_keys($, XtPointer client_data, XEvent *event, Boolean *cont)
{
    char buf[15];
    KeySym keysym;
    int n, i;
    
    assert(event->type == KeyPress);

    n = XLookupString(&event->xkey, buf, sizeof(buf), &keysym, NULL);
    if (n == 0)
	switch (keysym) {
	case XK_BackSpace:  n = 1; buf[0] = '\010';	    break;
	case XK_Home:	    n = 3; strcpy(buf, "\033[H");   break;
	case XK_Down:	    n = 3; strcpy(buf, "\033[B");   break;
	case XK_Left:	    n = 3; strcpy(buf, "\033[D");   break;
	case XK_Right:	    n = 3; strcpy(buf, "\033[C");   break;
	case XK_Up:	    n = 3; strcpy(buf, "\033[A");   break;
	case XK_Return:
	case XK_KP_Enter:
	case XK_Linefeed:   n = 1; buf[0] = '\015';	    break;
	case XK_Tab:	    n = 1; buf[0] = '\011';	    break;
	case XK_Delete:     n = 1; buf[0] = '\177';	    break;
	default:
	    if ((event->xkey.state & ControlMask)
		&& 65 <= keysym && keysym <= 95)
		buf[n++] = keysym - 64;	/* Control + letter */
	}
    for (i = 0; i < n; i++) XtCallCallbackList($, $keyCallback, &buf[i]);
    if (n != 0) *cont = FALSE;
}


@IMPORTS

@incl <assert.h>
@incl <stdio.h>
@incl <X11/Xatom.h>
@incl <X11/keysym.h>
@incl <X11/Xmu/Xmu.h>
