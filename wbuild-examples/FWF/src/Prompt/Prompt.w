# Prompt.w -- XfwfPrompt widget
# Author: Bert Bos <bert@let.rug.nl>
# 23 Nov '92
# Copyright: see README file
# Version: 1.1 for FWF V4.0

@class XfwfPrompt (XfwfLabel)  @file=Prompt

@ The Prompt widget is used for short (no more than a few lines) text
entry. It can be used in two modes: either with or without a template.
If there is a template, input will be checked against it and only
accepted if it matches. A template must be accompanied by a pattern,
that gives the acceptable character classes for each position in the
template. A template can include literal characters, e.g., |"Enter
date: __-__-__ (DD-MM-YY)"| with pattern |"99-99-99"| means that the
user may enter only digits on the `|_|' positions.

If the pattern and template do not provide enough validation, or if
there is no template, there is also the possibility of using a
callback after each character is entered. The callback can accept or
reject the new text.

The |rvStart| resource is used as cursor position.


@PUBLIC

@ The |text| resource is what is modified by the user. If there is a
template defined, the text will be the portion of the template from
the first `|_|' to the last.

	@var String text = NULL

@ The |template| consists of text with at least one `|_|' (or, to be
precise, the value of |placeholder|). If there is no template, input
of any length is accepted (but see the |validate| callback below).

Note that the |label| resource that is inherited from |XfwfLabel| is
used to hold the combination of |template| and |text|. Setting |label|
has no effect, and, indeed, will cause a warning message to be
printed.

	@var String template = NULL

@ The |pattern| is only used if there is a |template|. The length of
the |pattern| must be the length from the first `|_|' in |template| to
the last. Where the |template| has a `|_|', the |pattern| must have
one of:

* `9' only digits are allowed.

* `a' digits, upper- and lowercase letters and spaces.

* `A' digits, uppercase letters and spaces.

* `F' only `safe' file name characters.

* 'f' file name character plus `*' `[', `]' and `?'

* `X' any character.

* `z' digits, upper- and lowercase letters (no spaces)

* `Z' digits, uppercase letters (no spaces).

If the pattern is empty or too short, it will be extended with X's.

	@var String pattern = NULL

@ The |validate| callbacks are called when the text changes, because a
letter is added or removed. The callback get a pointer to an
|XfwfPromptInfo| structure as |call_data|. The structure includes a
field |reply|, which should be set to |XfwfReject|, |XfwfAccept| or
|XfwfComplete|. The first causes the new text to be rejected with a
`beep'; the second sets the |complete| resource to |False|, to
indicate that the text is valid, but not yet complete; the third sets
the |complete| resource to |True|.

	@var <Callback> XtCallbackList validate = NULL

@ The |leave| callback is called when the focus leaves the prompt
widget. This is usually a sign that the user is ready entering the
text. The final text is passed in the |call_data| argument.

	@var <Callback> XtCallbackList leave = NULL

@ The |activate| callback is invoked from the |activate| action, which
is normally bound to the Return key. The |call_data| argument of the
callbacks routines is filled with a pointer to the event that
triggered the action. This callback can be used in a dialog box, for
example, to activate the default button.

	@var <Callback> XtCallbackList activate = NULL

@ The resource |complete| is always |True|, unless the most recent
call to the |validate| callback returned |XfwfAccept|.

	@var Boolean complete = True

@ The character thatis used as the placeholder can be changed from the
default `|_|' by setting the |placeholder| resource. Note that this
character will also be used in the |text| string to indicate open
places.

	@var char placeholder = '_'

@ The cursor is by default a vertical bar, called the `xterm' cursor.

	@var cursor = <String> "xterm"

@ Since prompts, in contrast to labels, should receive keyboard
events, the |traversalOn| resource is by default |True|.

	@var traversalOn = True

@ The default frame width is also given a different value.

	@var frameWidth = 2

@ As is the frame type

	@var frameType = XfwfSunken

@ The default alignment of text is changed to center-left.

	@var alignment = XfwfLeft



@EXPORTS

@ The |XfwfPromptInfo| is what is passed to the |validate| callback
after the text has changed. It contains pointers to the old and the
new text and a boolean field |result| that is initially set to
|XfwfComplete|, but that may be changed by the callback routine, when
the new text is unacceptable, or acceptable but not complete.

	@type XfwfPromptInfoResult = enum {
		XfwfReject, XfwfAccept, XfwfComplete
	}
	@type XfwfPromptInfo = struct {
		String oldstring;
		String newstring;
		Boolean result;
	}


@PRIVATE

@ The start and length of the entry part of the template is stored in
two private variables.

	@var int text_start
	@var int text_len

@ Between keypresses, the |XComposeStatus| is retained.

	@var XComposeStatus compose_stat

@ If the widget is the owner of the selection, the text of the
selection is stored here.

	@var String selection_buffer



@METHODS

@ The |initialize| method initializes the private variables, with the
help of a utility routine. That routine also checks if |pattern| and
|template| agree.

All String resources are copied to new allocated space, so that the
application can change or throw away the original strings.

@proc initialize
{
    if ($label != NULL)
	XtWarning("The label resource of a Prompt should not be set");
    $text_start = 0;
    if ($text != NULL) $text = XtNewString($text);
    if ($template != NULL) $template = XtNewString($template);
    if ($pattern != NULL) $pattern = XtNewString($pattern);
    set_text_start($);
    while ($rvStart > $text_start + $text_len) $rvStart--;
    while ($rvStart < $text_start) $rvStart++;
    $selection_buffer = NULL;
}


@ The |set_values| method copies all string resources that have
changed to new heap space, and calls |set_text_start| to compute the
|text_start| and |text_end| and to set the |label| resource.

@proc set_values
{
    Boolean need_redisplay = False, reset_text = False;

    if ($old$text != $text) {
	XtFree($old$text);
	$text = XtNewString($text);
	reset_text = True;
    }
    if ($old$template !=$template) {
	XtFree($old$template);
	$template = XtNewString($template);
	reset_text = True;
    }
    if ($old$pattern != $pattern) {
	XtFree($old$pattern);
	$pattern = XtNewString($pattern);
	reset_text = True;
    }
    if (reset_text || $old$placeholder != $placeholder) {
	set_text_start($);
	need_redisplay = True;
    }
    return need_redisplay;
}


@ The |expose| method does much of the work that is also done by the
superclass, but without actually drawing anything. The only thing that
is drawn is the cursor.

@proc expose
{
    Region reg;
    XRectangle rect;
    int baseline, i, j;
    Dimension w;
    int x, y;

    if (! XtIsRealized($)) return;
    #expose($, event, region);
    if ($label == NULL) return;
    baseline = $font->ascent + $font->descent;
    $compute_inside($, &rect.x, &rect.y, &rect.width, &rect.height);
    rect.x += $leftMargin;  rect.width -= $leftMargin + $rightMargin;
    rect.y += $topMargin;  rect.height -= $topMargin + $bottomMargin;
    if ($alignment & XfwfTop)
	y = rect.y + $font->ascent;
    else if ($alignment & XfwfBottom)
	y = rect.y + rect.height - $nlines * baseline + $font->ascent;
    else
	y = rect.y + (rect.height - $nlines * baseline)/2 + $font->ascent;
    for (i = 0, j = 0; True; i++) {
	if ($label[i] == '\n' || $label[i] == '\0') {
	    if (j <= $rvStart && $rvStart <= i) {
		w = XfwfTextWidth($font, $label + j, i - j, $tabs);
		if ($alignment & XfwfLeft) x = rect.x;
		else if ($alignment & XfwfRight) x = rect.x + rect.width - w;
		else x = rect.x + (rect.width - w)/2;
		x += XfwfTextWidth($font, $label + j, $rvStart - j, $tabs);
		XDrawLine(XtDisplay($), XtWindow($), $gc, x, y -
			  $font->ascent, x, y + $font->descent);
		break;
	    }
	    j = i + 1;
	    y += baseline;
	}
    }
}



@TRANSLATIONS

@ The traversal code is added, except for |<Key>Left: traverseLeft()|
and |<Key>Right: traverseRight()|, which are used for other purposes.

@trans	<FocusIn>:		focusIn()
@trans	<FocusOut>:		leave() focusOut()

@ removed: trans	<Visible>:		visibility()
@ removed: trans	<Unmap>:		unmap()

@trans	<Key>Up:		traverseUp()
@trans	<Key>Down:		traverseDown()
@trans	<Key>Next:		traverseNext()
@trans	~Shift<Key>Tab:		traverseNext()
@trans	<Key>Prior:		traversePrev()
@trans	Shift<Key>Tab:		traversePrev()
@trans	<Key>KP_Enter:		traverseNextTop()
@trans	<Key>Home:		traverseHome()

@trans	Shift<Btn1Down>:	extend_select()
@trans	<Btn1Down>:		traverseCurrent() start_select()
@trans	<Btn1Motion>:		extend_select()
@trans	<Btn1Up>:		end_select()
@trans	<Btn2Down>,<Btn2Up>:	paste()
@trans	<Key>BackSpace:		cut() backspace()
@trans	Ctrl<Key>d:		cut() delete()
@trans	<Key>Right:		unselect() right()
@trans	<Key>Left:		unselect() left()
@trans	Ctrl<Key>a:		unselect() bol()
@trans	Ctrl<Key>e:		unselect() eol()
@trans	<Key>Return:		activate()
@trans	<Key>:			cut() self_insert()


@ACTIONS

@ The |activate| action just calls the |activate| callback functions,
passing the |XEvent| pointer in the |call_data| argument.

@proc activate
{
    XtCallCallbackList($, $activate, event);
}


@ The |leave| action calls the |leave| callbacks with the current
|text| as |call_data|.

@proc leave
{
    if (event->type == FocusOut
	&& event->xfocus.detail != NotifyAncestor
	&& event->xfocus.detail != NotifyInferior
	&& event->xfocus.detail != NotifyNonlinear) return;
    XtCallCallbackList($, $leave, $text);
}


@ The |start_select| action sets the cursor position as close as
possible to the mouse position, but only if the mouse actually points
at some text. It also removes any previous selection, preparing for a
new one.

@proc start_select
{
    Position x, y;
    Dimension w, h;
    Cardinal pos;

    if (find_cursor($, &event->xbutton, &$rvStart)) {
	$rvLength = 0;
	$compute_inside($, &x, &y, &w, &h);
	XClearArea(XtDisplay($), XtWindow($), x, y, w, h, True);
    }
}


@ The |extend_select| action extends the selection to the current
cursor position.

@proc extend_select
{
    Position x, y;
    Dimension w, h;
    int pos, len, start;

    if (find_cursor($, &event->xbutton, &pos)) {
	len = abs(pos - $rvStart);
	start = min(pos, $rvStart);
	if (len != $rvLength || start != $rvStart) {
	    $rvLength = len;
	    $rvStart = start;
	    $compute_inside($, &x, &y, &w, &h);
	    XClearArea(XtDisplay($), XtWindow($), x, y, w, h, True);
	}
    }
}


@ The |end_select| action copies the selected text to the clipboard.

@proc end_select
{
    if ($rvLength == 0) return;
    if (! XtOwnSelection($, XA_PRIMARY, event->xbutton.time,
			 convert_proc, lose_ownership_proc, NULL)) {
	XtWarning("failed attempting to become selection owner.");
	return;
    }
    XtFree($selection_buffer);
    $selection_buffer = XtMalloc(sizeof(char) * ($rvLength + 1));
    (void) strncpy($selection_buffer, $label + $rvStart, $rvLength);
    $selection_buffer[$rvLength] = '\0';
}


@ The |paste| action requests the contents of the clipboard in
string-format and tries to insert it into the text.

@proc paste
{
    XtGetSelectionValue($, XA_PRIMARY, XA_STRING, paste_callback, NULL,
			event->xbutton.time);
}


@ The |unselect| action restores the text that was shown in reverse to
normal.

@proc unselect
{
    Position x, y;
    Dimension w, h;

    if ($rvLength != 0) {
	$rvLength = 0;
	$compute_inside($, &x, &y, &w, &h);
	XClearArea(XtDisplay($), XtWindow($), x, y, w, h, True);
    }
}


@ The |cut| action deletes the highlighted portion from the text.

@proc cut
{
    int j, i;
    XfwfPromptInfo info;
    String newlabel;

    if ($rvLength == 0) return;
    info.oldstring = XtNewString($text);
    if ($template == NULL) {
	j = strlen($text);
	i = min($rvStart + $rvLength, j) - 1;
	for (; i >= $rvStart; i--, j--) delete_from_run($, i, j, '\0');
    } else {
	i = min($rvStart + $rvLength, $text_start + $text_len) - 1;
	for (; i >= $rvStart; i--) {
	    if ($template[i] != $placeholder) continue;
	    j = find_end_of_run($, i);
	    delete_from_run($, i - $text_start, j - $text_start, $placeholder);
	}
    }
    info.newstring = $text;
    info.result = XfwfComplete;
    XtCallCallbackList($, $validate, &info);
    switch (info.result) {
    case XfwfReject: XtFree($text); $text = info.oldstring; return;
    case XfwfAccept: XtFree(info.oldstring); $complete = False; break;
    case XfwfComplete: XtFree(info.oldstring); $complete = True; break;
    }
    if ($template == NULL) {
	$set_label($, $text);
    } else {
	newlabel = XtNewString($template);
	for (j = 0; j < $text_len; j++) newlabel[j+$text_start] = $text[j];
	$set_label($, newlabel);
	XtFree(newlabel);
    }
    $rvLength = 0;
}


@ The |self_insert| function inserts the key that was pressed into the
text. If the key is not a printable key, it does nothing. If the key
doesn't match the pattern, it does nothing. If the callback |validate|
returns |XfwfReject|, it does nothing.

@proc self_insert
{
    KeySym keysym;
    int n, i;
    char buf[100];
    XfwfPromptInfo info;
    String newlabel;

    if (event->type != KeyPress && event->type != KeyRelease) {
	XtWarning("action <self_insert> can only handle keyboard events");
	return;
    }
    info.oldstring = XtNewString($text);
    n = XLookupString(&event->xkey, buf, sizeof(buf), &keysym, &$compose_stat);
    for (i = 0; i < n; i++) insert_char($, buf[i]);
    info.newstring = $text;
    info.result = XfwfComplete;
    XtCallCallbackList($, $validate, &info);
    switch (info.result) {
    case XfwfReject: XtFree($text); $text = info.oldstring; return;
    case XfwfAccept: XtFree(info.oldstring); $complete = False; break;
    case XfwfComplete: XtFree(info.oldstring); $complete = True; break;
    }
    if ($template == NULL)
	$set_label($, $text);
    else {
	newlabel = XtNewString($template);
	for (i = 0; i < $text_len; i++) newlabel[i+$text_start] = $text[i];
	$set_label($, newlabel);
	XtFree(newlabel);
    }
}


@ The |bol| action moves the cursor to the leftmost placeholder.

@proc bol
{
    Position x, y;
    Dimension w, h;

    if ($rvStart == $text_start) return;
    $rvStart = $text_start;
    $compute_inside($, &x, &y, &w, &h);
    XClearArea(XtDisplay($), XtWindow($), x, y, w, h, True);
}


@ The |eol| action moves the cursor to the end of the label, if it
isn't there already.

@proc eol
{
    Position x, y;
    Dimension w, h;

    if ($template == NULL) {
	if ($label[$rvStart] == '\0') return;
	do { $rvStart++; } while ($label[$rvStart]);
    } else {
	if ($rvStart == $text_start + $text_len) return;
	$rvStart = $text_start + $text_len;
    }
    $compute_inside($, &x, &y, &w, &h);
    XClearArea(XtDisplay($), XtWindow($), x, y, w, h, True);
}


@ The |left| action moves the cursor one position to the left, unless
there are no more placeholders in that direction.

@proc left
{
    Position x, y;
    Dimension w, h;

    if ($rvStart == $text_start) return;
    if ($template == NULL)
	$rvStart--;
    else
	do { $rvStart--; } while ($template[$rvStart] != $placeholder);
    $compute_inside($, &x, &y, &w, &h);
    XClearArea(XtDisplay($), XtWindow($), x, y, w, h, True);
}


@ The |right| action moves the cursor one position to the right,
unless it is already at the end of the template.

@proc right
{
    Position x, y;
    Dimension w, h;

    if ($template == NULL) {
	if ($label[$rvStart] == '\0') return;
	$rvStart++;
    } else {
	if ($rvStart == $text_start + $text_len) return;
	do { $rvStart++; } while ($template[$rvStart] != $placeholder);
    }
    $compute_inside($, &x, &y, &w, &h);
    XClearArea(XtDisplay($), XtWindow($), x, y, w, h, True);
}


@ The |delete| action removes the character to the right of the
cursor, if there is any. The rest of the characters in the same run
will shift to the left.

@proc delete
{
    int j;
    XfwfPromptInfo info;
    String newlabel;

    if ($template == NULL) {
	if (! $text || $text[$rvStart] == '\0') return;
	info.oldstring = XtNewString($text);
	j = strlen($text);
	delete_from_run($, $rvStart, j, '\0');
    } else {
	if ($template[$rvStart] != $placeholder) return;
	info.oldstring = XtNewString($text);
	j = find_end_of_run($, $rvStart) - $text_start;
	delete_from_run($, $rvStart - $text_start, j, $placeholder);
    }
    info.newstring = $text;
    info.result = XfwfComplete;
    XtCallCallbackList($, $validate, &info);
    switch (info.result) {
    case XfwfReject: XtFree($text); $text = info.oldstring; return;
    case XfwfAccept: XtFree(info.oldstring); $complete = False; break;
    case XfwfComplete: XtFree(info.oldstring); $complete = True; break;
    }
    if ($template == NULL) {
	$set_label($, $text);
    } else {
	newlabel = XtNewString($template);
	for (j = 0; j < $text_len; j++) newlabel[j+$text_start] = $text[j];
	$set_label($, newlabel);
	XtFree(newlabel);
    }
}


@ The |backspace| action removes the character to the left of the
cursor, if there is any. The cursor moves left and all characters in
the same run move left.

@proc backspace
{
    int j;
    XfwfPromptInfo info;
    String newlabel;

    if ($rvStart == $text_start) return;
    info.oldstring = XtNewString($text);
    if ($template == NULL) {
	$rvStart--;
	j = strlen($label);
	delete_from_run($, $rvStart, j, '\0');
    } else {
	do { $rvStart--; } while ($template[$rvStart] != $placeholder);
	j = find_end_of_run($, $rvStart) - $text_start;
	delete_from_run($, $rvStart - $text_start, j, $placeholder);
    }
    info.newstring = $text;
    info.result = XfwfComplete;
    XtCallCallbackList($, $validate, &info);
    switch (info.result) {
    case XfwfReject: XtFree($text); $text = info.oldstring; return;
    case XfwfAccept: XtFree(info.oldstring); $complete = False; break;
    case XfwfComplete: XtFree(info.oldstring); $complete = True; break;
    }
    if ($template == NULL) {
	$set_label($, $text);
    } else {
	newlabel = XtNewString($template);
	for (j = 0; j < $text_len; j++) newlabel[j+$text_start] = $text[j];
	$set_label($, newlabel);
	XtFree(newlabel);
    }
}


@UTILITIES

@ The |find_cursor| function sets the cursor position |pos| to the
character closest to the mouse coordinates, it returns |True| if it
succeeded, else |False|.

@proc Boolean find_cursor($, XButtonEvent *event, int *pos)
{
    XRectangle rect;
    int baseline, i, j;
    Dimension w;
    int x, y;

    if (! XtIsRealized($)) return False;
    if ($label == NULL) return False;
    baseline = $font->ascent + $font->descent;
    $compute_inside($, &rect.x, &rect.y, &rect.width, &rect.height);
    rect.x += $leftMargin;  rect.width -= $leftMargin + $rightMargin;
    rect.y += $topMargin;  rect.height -= $topMargin + $bottomMargin;
    if ($alignment & XfwfTop)
	y = rect.y;
    else if ($alignment & XfwfBottom)
	y = rect.y + rect.height - $nlines * baseline;
    else
	y = rect.y + (rect.height - $nlines * baseline)/2;
    i = 0;
    j = 0;
    do {
	if ($label[i] == '\n' || $label[i] == '\0') {
	    if (y <= event->y && event->y < y + baseline) {
		find_cursor_in_line($, rect, event->x, event->y, j, i, pos);
		return True;
	    } else {
		j = i + 1;
		y += baseline;
	    }
	}
    } while ($label[i++]);
    return False;
}

@ |find_cursor| makes use of an auxiliary function
|find_cursor_in_line|, which is called when the mouse is on the line
between |$label[j]| and |$label[i]|.

@proc find_cursor_in_line($, XRectangle rect, int mx, int my, int j, int i, int *pos)
{
    Dimension w;
    int x;

    w = XfwfTextWidth($font, $label + j, i - j, $tabs);
    if ($alignment & XfwfLeft)
	x = rect.x;
    else if ($alignment & XfwfRight)
	x = rect.x + rect.width - w;
    else
	x = rect.x + (rect.width - w)/2;
    if (x + w <= mx) {				/* Mouse right of the text */
	if ($template == NULL)
	    *pos = i;
	else {					/* Find placeholder */
	    while (i > $text_start && $template[i] != $placeholder) i--;
	    *pos = i;
	}
    } else if (mx < x) {			/* Mouse left of the text */
	if ($template == NULL)
	    *pos = j;
	else {					/* Find placeholder */
	    while (j > $text_start && $template[j] != $placeholder) j--;
	    *pos = j;
	}
    } else {					/* Mouse points in the text */
	for (w = 0; True; j++) {
	    w += XfwfTextWidth($font, $label + j, 1, $tabs);
	    if (x + w >= mx) break;
	}
	if ($template == NULL)
	    *pos = j;
	else if (j >= $text_start + $text_len)
	    *pos = $text_start + $text_len;
	else if (j <= $text_start)
	    *pos = $text_start;
	else {					/* Find nearest placeholder */
	    while (j > $text_start && $template[j] != $placeholder) j--;
	    *pos = j;
	}
    }
}

@ The |insert_char| function tries to insert a character at the cursor
position. If it matches the pattern, it is inserted, otherwise, if it
matches the next literal character in the template, the cursor will
skip to that position. If a character could be inserted, the
|validate| callback is called. If that returns |XfwfReject|, the old
situation is restored.

If there is no template, only the callback is called.

The first few functions deal with `runs', which are defined as rows of
consecutive placeholders with the same pattern character.
|find_end_of_run| looks for the first character beyond the end of the
run that starts at |start|.

@proc int find_end_of_run($, int start)
{
    int i, j;

    i = start + 1;
    j = i - $text_start;
    while ($template[i] == $placeholder && $pattern[j] == $pattern[j-1]) {
	i++;
	j++;
    }
    return i;
}


@ |insert_in_run| inserts character |c| at position |start| in
|text|, shifting the rest of the characters in the run one position
forward.

@proc insert_in_run($, int start, int end, int c)
{
    int i;

    for (i = end - 1; i > start; i--) $text[i] = $text[i-1];
    $text[start] = c;
}


@ |delete_from_run| shift all characters in the run to the left, the
last character will become a placeholder (|filler|), the first
character will disappear.

@proc delete_from_run($, int start, int end, int filler)
{
    int i;

    for (i = start; i < end - 1; i++) $text[i] = $text[i+1];
    $text[end-1] = filler;
}


@ If the characer |c| is in the class indicated by |class|, the
function returns |True|.

@proc Boolean matches(int c, int class)
{
    if (class == '9') return isdigit(c);
    if (class == 'a') return isalnum(c) || isspace(c);
    if (class == 'A') return isdigit(c) || isupper(c) || isspace(c);
    if (class == 'F')
	return isgraph(c) && c != '*' && c != '?' && c != '[' && c != ']';
    if (class == 'f') return isgraph(c);
    if (class == 'X') return isascii(c);
    if (class == 'z') return isalnum(c);
    if (class == 'Z') return isdigit(c) || isupper(c);
    XtWarning("Cannot happen: incorrect pattern");
    return True;
}

@ The |insert_char| function. |c| is really a character, not an |int|.

@proc insert_char($, int c)
{
    int j;

    if (! $template) {
	j = strlen($text);
	$text = XtRealloc($text, j + 2);
	$text[j+1] = '\0';
	insert_in_run($, $rvStart, j + 1, c);
	$rvStart++;
    } else {
	if ($template[$rvStart] != $placeholder) {
	    XBell(XtDisplay($), 50);		/* At end of template */
	    return;
	}
	j = find_end_of_run($, $rvStart);
	if (matches(c, $pattern[$rvStart - $text_start])) {
	    insert_in_run($, $rvStart - $text_start, j - $text_start, c);
	    do {
		$rvStart++;
	    } while ($rvStart < $text_start + $text_len
		     && $template[$rvStart] != $placeholder);
	} else if (c == $template[j]) {
	    while ($template[j] && $template[j] != $placeholder) j++;
	    $rvStart = j;
	} else {
	    XBell(XtDisplay($), 50);
	}
    }
}


@ |set_text_start| checks if |pattern| and |template| agree and if so,
sets the private variables |text_start| and |text_len| appropriately.
The |label| resource is set to the value of |template| with |text|
substituted. If |text| is too short, it is extended.

@def pattern_char(c) =
    (c == '9' || c == 'a' || c == 'A' || c == 'F' || c == 'f' || c ==
     'X' || c == 'z' || c == 'Z')

@proc set_text_start($)
{
    int i, j, k;
    String newlabel;

    if ($template == NULL) {
	newlabel = XtNewString($text);
	XtVaSetValues($, XtNlabel, newlabel, NULL);
	XtFree(newlabel);
	return;
    }
    /*
     * Find start and end in template
     */
    for (i = 0; $template[i] && $template[i] != '_'; i++) ; /* skip */
    for (k = i, j = i; $template[j]; j++) if ($template[j] == '_') k = j;
    $text_start = i;
    $text_len = k + 1 - i;
    /*
     * Check the pattern, first the length, then the contents
     */
    if ($pattern == NULL) {
	$pattern = XtMalloc(($text_len + 1)*sizeof(char));
	$pattern[0] = '\0';
    } else if (strlen($pattern) < $text_len) {
	$pattern = XtRealloc($pattern, ($text_len + 1)*sizeof(char));
    }
    for (i = strlen($pattern); i < $text_len; i++)
	$pattern[i] = 'X';
    for (i = $text_start, k = 0; k < $text_len; i++, k++)
	if ($template[i] == $placeholder && ! pattern_char($pattern[k])) {
	    XtWarning("Pattern doesn't match template");
	    $pattern[k] = 'X';
	}
    $pattern[k] = '\0';
    /*
     * Check the length of the text
     */
    if ($text == NULL) {
	$text = XtMalloc(($text_len + 1)*sizeof(char));
	$text[0] = '\0';
    } else if (strlen($text) < $text_len) {
	$text = XtRealloc(XtNewString($text), ($text_len + 1)*sizeof(char));
    }
    for (j = strlen($text); j < $text_len; j++)
	$text[j] = $template[j+$text_start];
    /*
     * Initialize the label
     */
    newlabel = XtNewString($template);
    for (i = $text_start, j = 0; j < $text_len; i++, j++)
	newlabel[i] = $text[j];
    $set_label($, newlabel);
    XtFree(newlabel);
}


@ The procedure to convert the selection.

@proc Boolean convert_proc($, Atom *selection, Atom *target, Atom *type, XtPointer *value, unsigned long *length, int *format)
{
#ifdef SVR4                                     /* BB, 27 Mar '96 */
#define bcopy(b1, b2, length) memcpy(b2, b1, length)
#endif
    Display* d = XtDisplay($);
    XSelectionRequestEvent* req = XtGetSelectionRequest($, *selection, NULL);

    if (*target == XA_TARGETS(d)) {
	Atom* targetP;
	Atom* std_targets;
	unsigned long std_length;
	XmuConvertStandardSelection($, req->time, selection, target,
				    type, (caddr_t*)&std_targets,
				    &std_length, format);
	*value = XtMalloc(sizeof(Atom) * (std_length + 1));
	targetP = *(Atom**) value;
	*targetP++ = XA_STRING;
	*length = std_length + (targetP - (*(Atom **) value));
	bcopy((char*)std_targets, (char*)targetP, sizeof(Atom)*std_length);
	XtFree((char*)std_targets);
	*type = XA_ATOM;
	*format = sizeof(Atom) * 8;
	return True;
    }
    if (*target == XA_STRING) {
	*type = XA_STRING;
	*length = sizeof(char) * (strlen($selection_buffer) + 1);
    	*value = XtNewString($selection_buffer);
    	*format = 8;
    	return True;
    }
    if (XmuConvertStandardSelection($, req->time, selection, target,
				    type, (caddr_t*)value, length, format))
	return True;

    return False;
}


@ The procedure |lose_ownership_proc| is called when another window
takes over the ownership of the selection. There is nothing to do.
(Maybe the reversed text should be restored?)

@proc lose_ownership_proc($, Atom *selection)
{
}


@ The routine |paste_callback| is called by Xt after the selection is
requested in the |paste| action. It is handed the contents of the
selection, which it will insert into the text of the Prompt.

@proc paste_callback($, XtPointer client_data, Atom *selection, Atom *type, XtPointer value, unsigned long *length, int *format)
{
    String data = (String) value;
    XfwfPromptInfo info;
    int i;
    String newlabel;

    if (value == NULL && *length == 0) {
	XBell(XtDisplay($), 50);
	/* XtWarning("no selection or selection timed out, try again"); */
	return;
    }
    info.oldstring = XtNewString($text);
    for (i = 0; i < *length && data[i]; i++)
	insert_char($, data[i]);
    info.newstring = $text;
    info.result = XfwfComplete;
    XtCallCallbackList($, $validate, &info);
    switch (info.result) {
    case XfwfReject: XtFree($text); $text = info.oldstring; return;
    case XfwfAccept: XtFree(info.oldstring); $complete = False; break;
    case XfwfComplete: XtFree(info.oldstring); $complete = True; break;
    }
    if ($template == NULL)
	$set_label($, $text);
    else {
	newlabel = XtNewString($template);
	for (i = 0; i < $text_len; i++) newlabel[i+$text_start] = $text[i];
	$set_label($, newlabel);
	XtFree(newlabel);
    }
    XtFree(value);
}



@IMPORTS

@incl <ctype.h>
@incl <stdio.h>
@incl <X11/Xatom.h>
@incl <X11/Xmu/Xmu.h>
