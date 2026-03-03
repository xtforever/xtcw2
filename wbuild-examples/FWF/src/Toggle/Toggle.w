# Toggle widget
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 2.1 for FWF V4.0

@class XfwfToggle(XfwfButton)  @file=Toggle

@ The |XfwfToggle| is a button that switches states with every activation
(which is by default with every mouse click). The states are named `on' and
`off'. The states can be indicated with a $\surd$ before the label Two
callbacks report the changed state to the application: |onCallback| is called
when the button switches to `on', |offCallback| is called when the button
switches back to `off'.


@public

@ The |onCallback| is called by the |toggle| action or by the |switch_on|
action, if the previous state was `off'. The |call_data| argument will contain
the |XEvent| pointer that trigerred the action function.

	@var <Callback> XtCallbackList onCallback = NULL

@ The |offCallback| is called from the |toggle| or |switch_off| action, if the
previous state was `on'. The |call_data| argument will be a pointer to the
|XEvent| that caused the action.

	@var <Callback> XtCallbackList offCallback = NULL

@ The variable |on| records the state of the widget: |True| means `on' and
|False| means `off'.

	@var Boolean on = False

@ By default, the button displays a $\surd$ when on and nothing when
off. The two resources |onIcon| and |offPIcon| can replace these
two graphics with arbitrary pixmaps. Suggested size for these pixmaps
is about $16\times18$.

	@var Icon * onIcon = <String> "filledsquare"

@ The |offIcon| is displayed when the button is in `off' state. By
default, nothing is displayed in this state.

	@var Icon * offIcon = <String> "emptysquare"

@translations

@ The |toggle| action toggles the widget between `on' and `off'. By
default it is bound to a click of the left mouse button as well as to
the Return key.

	@trans <Btn1Down>: set_shadow("sunken")
	@trans <Btn1Up>: toggle() set_shadow()
	@trans <Key>Return: toggle()


@actions

@ The |toggle| action switches the state. Depending on the resources
it might change the tickmark. The |onCallback| or |offCallback|
functions are called, with the event as |call_data| argument.

@proc toggle
{
    XtVaSetValues($, "on", !$on, NULL);
    XtCallCallbackList($, $on ? $onCallback : $offCallback, event);
}

@ The |switch_on| action switches the button to `on' if it is `off',
otherwise it does nothing. By default it isn't bound to any event. If
the widget is changed, the |onCallback| is called with the event as
|call_data|.

@proc switch_on
{
    if (! $on) {
        XtVaSetValues($, "on", True, NULL);
        XtCallCallbackList($, $onCallback, event);
    }
}

@ The |switch_off| action switches the widget to `off' if the state is
`on', otherwise it does nothing. When the widget changes states, the
|offCallback| is called, with a pointer to the |XEvent| structure as
|call_data| argument.

@proc switch_off
{
    if ($on) {
        XtVaSetValues($, "on", False, NULL);
        XtCallCallbackList($, $offCallback, event);
    }
}


@private

@ The |on_gc| GC holds the picture of the tick mark for the `on'
state.

	@var GC on_gc

@ The |off_gc| holds the picture for the `off' state.

	@var GC off_gc

@ The previous value of |leftMargin| is stored in a private variable.
This value is added to the width of the widest pixmap to give the new
value of |leftMargin|.

	@var Dimension saveLeftMargin


@methods

@ The GC's are created for the first time and the left margin is
increased to make room for the on and off icons.

@proc initialize
{
    int status;
    Dimension w1, w2, w;

    $on_gc = NULL;
    $off_gc = NULL;
    create_on_gc($);
    create_off_gc($);

    w1 = $onIcon ? $onIcon->attributes.width : 0;
    w2 = $offIcon ? $offIcon->attributes.width : 0;
    w = max(w1, w2);
    if (w != 0) XtVaSetValues($, XtNleftMargin, $leftMargin + w, NULL);
}

@ Question: Does the computation of |leftMargin| have the desired
effect? Since |set_values| is downward chained, the Label widget has
already processed it; changing |leftMargin| doesn't cause Label to
recompute the preferred size\dots

@proc set_values
{
    Boolean redraw = False, compute_margin = False;
    Dimension w1, w2, w;

    if ($onIcon != $old$onIcon) {
	create_on_gc($);
	compute_margin = True;
    }
    if ($offIcon != $old$offIcon) {
	create_off_gc($);
	compute_margin = True;
    }
    if ($on != $old$on) {
        redraw = True;
    }
    if (compute_margin) {
	/* Compute w = old margin between icons and text */
	w1 = $old$onIcon ? $old$onIcon->attributes.width : 0;
	w2 = $old$offIcon ? $old$offIcon->attributes.width : 0;
	w = $old$leftMargin - max(w1, w2);
	/* Compute new left margin w = w + width of icons */
	w1 = $onIcon ? $onIcon->attributes.width : 0;
	w2 = $offIcon ? $offIcon->attributes.width : 0;
	w = w + max(w1, w2);
	if ($old$leftMargin != w) {
	    XtVaSetValues($, XtNleftMargin, w, NULL);
	    redraw = False;
	} else
	    redraw = True;
    }
    return redraw;
}

@ The |expose| method uses the |expose| method of the superclass to draw the
button and then possibly adds a tick mark.

@proc expose
{
    Position x, y;
    Dimension w, h;

    if (! XtIsRealized($)) return;
    #expose($, event, region);
    $compute_inside($, &x, &y, &w, &h);
    if ($on && $onIcon) {
	y = y + (h - $onIcon->attributes.height)/2;
	XSetTSOrigin(XtDisplay($), $on_gc, x, y);
	XSetClipOrigin(XtDisplay($), $on_gc, x, y);
	XFillRectangle(XtDisplay($), XtWindow($), $on_gc, x, y,
		       $onIcon->attributes.width,
		       $onIcon->attributes.height);
    } else if ($offIcon) {
	y = y + (h - $offIcon->attributes.height)/2;
	XSetTSOrigin(XtDisplay($), $off_gc, x, y);
	XSetClipOrigin(XtDisplay($), $off_gc, x, y);
	XFillRectangle(XtDisplay($), XtWindow($), $off_gc, x, y,
		       $offIcon->attributes.width,
		       $offIcon->attributes.height);
    }
}


@utilities

@ The |create_on_gc| function creates a GC with the |onIcon| as tile.

@proc create_on_gc($)
{
    XtGCMask mask = GCFillStyle;
    XGCValues values;

    if ($on_gc != NULL) XtReleaseGC($, $on_gc);
    if ($onIcon && $onIcon->pixmap != None) {
	values.tile = $onIcon->pixmap;
	mask |= GCTile;
    }
    if ($onIcon && $onIcon->mask != None) {
	values.clip_mask = $onIcon->mask;
	mask |= GCClipMask;
    }
    values.fill_style = FillTiled;
    $on_gc = XtGetGC($, mask, &values);
}

@ The |create_off_gc| function creates a GC with the |offIcon| as tile.

@proc create_off_gc($)
{
    XtGCMask mask = GCFillStyle;
    XGCValues values;

    if ($off_gc != NULL) XtReleaseGC($, $off_gc);
    if ($offIcon && $offIcon->pixmap != None) {
	values.tile = $offIcon->pixmap;
	mask |= GCTile;
    }
    if ($offIcon && $offIcon->mask != None) {
	values.clip_mask = $offIcon->mask;
	mask |= GCClipMask;
    }
    values.fill_style = FillTiled;
    $off_gc = XtGetGC($, mask, &values);
}

@imports

@ The Converters file is needed for the |Icon| type.

@incl <Xfwf/Converters.h>
