# FoldTree -- FoldingTree widget
# Ray Bellis <rpb@psy.ox.ac.uk>
# Version 0.9 for FWF

@class XfwfFoldingTree (XfwfBoard) @file=FoldTree

@ The FoldingTree class is designed to show subtrees of widgets which
can be expanded and collapsed by the user.

The first child of each FoldingTree widget is used as a label, and is
shown immediately to the right of the control icon.  Only subsequent
children are [un]mapped when the control icon is activated.

@PUBLIC

@ The state of the tree

	@var Boolean expanded = False

@ Icons used to indicate the current state of the tree

	@var Icon* iconExpand = NULL
	@var Icon* iconCollapse = NULL
	@var Icon* iconNoExpand = NULL

@ Whether to shrink when the subtree is collapsed.

	@var Boolean shrinkToFit = True

@ The line width to use for the tree diagram

	@var Dimension lineWidth = 2

@ The line color to use for the tree diagram

	@var Pixel lineColor = <String> "#000"

@ The amount of vertical space to leave between child widgets

	@var Dimension childSpace = 4

@ The frame width to use for the whole tree widget

	@var frameWidth = 0

	@var <Callback> XtCallbackList activate = NULL

@PRIVATE

@ Child widget automatically created during initialization used as the
control for expanded and collapsing the widget tree

	@var Widget tree_icon

@ GC used to draw the tree diagram

	@var GC line_gc

@ Position arrays used for storing coordinates for use by the |redraw_lines|
function

	@var Position* xpos
	@var Position* ypos

@METHODS

@ Registers type convertor for icons

@proc class_initialize
{
	static XtConvertArgRec args[] = {
		{ XtWidgetBaseOffset, 0, sizeof(Widget) }
	};

	XtSetTypeConverter(XtRString, "Icon", cvtStringToIcon,
		args, XtNumber(args), XtCacheNone, NULL);
}

@ Creates |iconExpand|, |iconCollapse| and |iconExpand|.
Creates |tree_icon| control widget and calls |create_line_gc|
Allocates the |xpos| and |ypos| arrays for storing line segments

@proc initialize
{
	static char	trans[] = "<Btn1Down>,<Btn1Up>: activate()";

	/* Create default icons if neccesary */
	if (!icons_created) {
		create_icons(XtDisplay($));
		icons_created = True;
	}

	/* Create a GC */
	$line_gc = NULL;
	create_line_gc($);

	/* Allocate a couple of arrays for point storage */
	$xpos = (Position *)XtMalloc(2 * sizeof(Position));
	$ypos = (Position *)XtMalloc(2 * sizeof(Position));

	/* Copy over pointers to the default icons */
	if (!$iconExpand) $iconExpand = &iconPlus;
	if (!$iconCollapse) $iconCollapse = &iconMinus;
	if (!$iconNoExpand) $iconNoExpand = &iconDot;

	/* Create the control widget */
	$tree_icon = XtVaCreateManagedWidget(
		"_icon", xfwfIconWidgetClass, $,
		XtNframeWidth, (XtArgVal)2,
		XtVaTypedArg, XtNtranslations,
			XtRString, trans, strlen(trans)+1,
		NULL);

	/* Add the control widget's callback */
	XtAddCallback($tree_icon, XtNactivate, toggle_CB, $);
}

@ Deallocates the dynamically allocated objects |line_gc|, |xpos| and |ypos|

@proc destroy
{
	XtReleaseGC($, $line_gc);
	XtFree((String)$xpos);
	XtFree((String)$ypos);
}

@ Inserts the child and then reallocates the |ypos| array if there
are now more than two children

@proc insert_child
{
	#insert_child(child);
	if ($num_children > 2) {
		$ypos = (Position*)XtRealloc((void*)$ypos,
			$num_children * sizeof(Position));
	}
}

@ Repositions all of the children if one of them changes its management
state

@proc change_managed
{
	$layout($, $shrinkToFit);
}

@ Calls the layout method, but doesn't bother shrinking the widget
 
@proc resize
{
	$layout($, False);
} 

@ Monitors changes to public class variables and calls the layout method
if neccesary

@proc set_values
{
	Boolean redraw = False;
	Boolean dolayout = False;

	/* Recreate the line_gc if the width or color needs changing */
	if ($old$lineWidth != $lineWidth ||
	    $old$lineColor != $lineColor) {
		create_line_gc($);
		redraw = True;
	}

	/* Check whether any of the icons have changed */
	if ($old$iconExpand != $iconExpand ||
	    $old$iconCollapse != $iconCollapse ||
	    $old$iconNoExpand != $iconNoExpand) {
		dolayout = True;
		redraw = True;
	}

	/* Check whether any of the layout parameters have changed */
	if ($old$expanded != $expanded ||
	    $old$shrinkToFit != $shrinkToFit ||
	    $old$childSpace != $childSpace) {
		dolayout = True;
		redraw = True;
	}

	if (dolayout) {
		$layout($, $shrinkToFit);
	}

	return redraw;
}

@ Prohibits children from asking for a position change.

@proc geometry_manager
{
	Dimension	nw, nh, nb;

	if (request->request_mode & (CWX | CWY)) return XtGeometryNo;
	if (request->request_mode & XtCWQueryOnly) return XtGeometryYes;

	nw = request->request_mode & CWWidth ? request->width : $child$width;
	nh = request->request_mode & CWHeight ? request->height : $child$height;
	nb = request->request_mode & CWBorderWidth ?
		request->border_width : $child$border_width;

	if (nw == $child$width &&
            nh == $child$height &&
	    nb == $child$border_width) return XtGeometryNo;

	XtResizeWidget(child, nw, nh, nb);
	$layout($, $shrinkToFit);
	return XtGeometryDone;
}

@ Repositions all child windows depending on the requested state of
the tree and then calls the redraw_lines function to draw the actual
tree

@proc layout($, int shrink)
{
	Widget		icon = $tree_icon, child;
	Boolean		can_expand;
	Position	left, top;
	Dimension	width, height;
	Dimension	max_width = 0;	/* Largest child widget width */
	Position	y;		/* Current top of next widget */
	Dimension	dy;		/* Increment to next widget's position*/
	int		i;

	/* Find out what sort of icon should be in use */
	can_expand = ($num_children > 2);
	$expanded &= can_expand;
	if (can_expand) {
		if ($expanded) {
			XtVaSetValues(icon, XtNimage, $iconCollapse,
				NULL);
		} else {
			XtVaSetValues(icon, XtNimage, $iconExpand,
				NULL);
		}
	} else {
		XtVaSetValues(icon, XtNimage, $iconNoExpand, NULL);
	}

	/* Find the widgets dimensions */
	$compute_inside($, &left, &top, &width, &height);

	/* xpos[0] = middle of the icon */
	$xpos[0] = left + $icon$border_width + $icon$width / 2;

	/* ypos[0] = bottom of the icon */
	$ypos[0] = top + 2 * $icon$border_width + $icon$height;
	
	/* xpos[1] = new left hand side of child icons */
	$xpos[1] = left + 2 * $icon$border_width +
			$icon$width + $childSpace;

	/* Current widget position is the top of the window */
	y = top;

	/* Loop over all children except the first (the control widget) */
	for (i = 1; i < $num_children; ++i) {
		child = $children[i];
		if (!XtIsManaged(child)) continue;

		/* Calculate first component if position step */
		dy = $child$height + 2 * $child$border_width;

		/* Check whether the control icon is taller than the
		   label widget */
		if (i == 1) {
			dy = max(dy, $icon$height + 
					2 * $icon$border_width);
			max_width = $child$width + 2 * $child$border_width;
		}

		/* Set visual stage of child widgets (unless it's the
		   label widget */
		if (i > 1) {
			XtSetMappedWhenManaged(child, $expanded);
		}

		/* Move the widget to it's place */
		XtMoveWidget(child, $xpos[1], y);

		/* Calculate position of tree line into this child widget
		   NB: If the child widget is itself a subtree then the
		   tree line points at the midpoint of the subtree's control
		   widget instead of the midpoint of the entire widget */

		if (XtIsSubclass(child, xfwfFoldingTreeWidgetClass)) {
			XfwfFoldingTreeWidget tmp =
				(XfwfFoldingTreeWidget)child;
			XfwfIconWidget icon =
				(XfwfIconWidget)$tmp$tree_icon;
			$ypos[i] = y + $child$border_width +
				$icon$height / 2;
		} else {
			$ypos[i] = y + $child$border_width + $child$height / 2;
		}

		/* Calculate the position of the next widget if the current
		   widget is visible */
		if (i == 1 || $expanded) {
			y += dy + $childSpace;
			max_width = max(max_width,
				$child$width + 2 * $child$border_width);
		}

	}

	/* Clear the old lines if neccesary */
	if (XtIsRealized($)) XClearWindow(XtDisplay($), XtWindow($));

	/* Redraw the tree lines */
	redraw_lines($);

	/* Shrink this widget if neccesary */
	if (shrink) {
		width = $xpos[1] + max_width;
		height = y - $childSpace;
		if (width != $width ||
		    height != $height) {
			XtVaSetValues($,
				XtNwidth, width,
				XtNheight, height,
				NULL);
		}
	}
}

@ Call the superclasses expose method and then draw the tree lines

@proc expose
{
	#expose($, event, region);
	if (XtIsRealized($)) {
		redraw_lines($);
	}
}

@TRANSLATIONS

@trans <Key>plus: expand()
@trans <Key>minus: collapse()

@ACTIONS

@proc expand
{
	expand_CB($, $, NULL);
}

@proc collapse
{
	collapse_CB($, $, NULL);
}

@proc toggle
{
	toggle_CB($, $, NULL);
}

@UTILITIES

@ Global variables for the three default icons

@var Boolean icons_created = False
@var Icon iconPlus
@var Icon iconMinus
@var Icon iconDot

@ Callbacks to set the state of the tree

@proc expand_CB(Widget w, XtPointer client_data, XtPointer call_data)
{
	Widget $ = (Widget) client_data;

	XtCallCallbackList($, $activate, (XtPointer)True);
	XtVaSetValues($, XtNexpanded, True, NULL);
	$layout($, $shrinkToFit);
}

@proc collapse_CB(Widget w, XtPointer client_data, XtPointer call_data)
{
	Widget $ = (Widget) client_data;

	XtCallCallbackList($, $activate, False);
	XtVaSetValues($, XtNexpanded, False, NULL);
	$layout($, $shrinkToFit);
}

@proc toggle_CB(Widget w, XtPointer client_data, XtPointer call_data)
{
	Widget $ = (Widget) client_data;

	XtCallCallbackList($, $activate, (XtPointer)!$expanded);
	XtVaSetValues($, XtNexpanded, !$expanded, NULL);
	$layout($, $shrinkToFit);
}

@ Creates the GC for drawing the tree lines of the correct width

@proc create_line_gc($)
{
	XtGCMask	mask = 0;
	XGCValues	values;

	if ($line_gc != NULL) XtReleaseGC($, $line_gc);
	values.line_width = $lineWidth;
	mask |= GCLineWidth;
	values.foreground = $lineColor;
	mask |= GCForeground;
	$line_gc = XtGetGC($, mask, &values);
}

@ Redraws the tree lines

@proc redraw_lines($)
{
	Widget		child;
	int		i;
	Window		w = XtWindow($);

	if (!XtIsRealized($) || !$expanded || !$xpos || !$ypos) return;

	for (i = 2; i < $num_children; ++i) {
		child = $children[i];
		if (!XtIsManaged(child)) continue;
		XDrawLine(XtDisplay($), w, $line_gc,
			$xpos[0], $ypos[i], $xpos[1], $ypos[i]);
	}
	XDrawLine(XtDisplay($), w, $line_gc,
		$xpos[0], $ypos[0], $xpos[0], $ypos[i - 1]);
}

@ Converts data bits into an Icon

@proc data_to_icon(Display *display, char *data[], Icon *icon)
{
	int		 status;

	icon->attributes.valuemask = XpmSize;
	status = XpmCreatePixmapFromData(
		display, DefaultRootWindow(display), data,
		&icon->pixmap, &icon->mask,
		&icon->attributes);
	switch (status) {
		case XpmNoMemory:
			XtError("Out of memory");
		case XpmColorFailed:
			XtError("Failed to allocate color for pixmap");
		case XpmColorError:
			XtWarning("Not all pixmap colors found");
		default: ; /* skip */
	}

}

@ Creates default state icons

@proc create_icons(Display *display)
{
	data_to_icon(display, plus, &iconPlus);
	data_to_icon(display, minus, &iconMinus);
	data_to_icon(display, dot, &iconDot);
}

@IMPORTS

@incl "plus.xpm"
@incl "minus.xpm"
@incl "dot.xpm"
@incl <Xfwf/Board.h>
@incl <Xfwf/Icon.h>
@incl <X11/Xmu/Converters.h>
@incl <Xfwf/Converters.h>
