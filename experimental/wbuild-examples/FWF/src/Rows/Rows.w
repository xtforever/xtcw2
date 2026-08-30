# XfwfRows -- lays out children in rows
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 1.0
# 26 Apr '94


@CLASS XfwfRows (XfwfBoard)  @file=Rows

@ The RowCol widget forces all its children into tightly packed
rows. The children keep their preferred size, but the preferred
position is ignored.

@PUBLIC

@ The children are packed together on as many rows as are needed, but
within the row they can either be aligned on the top or on the bottom.

	@var Boolean alignTop = True

@ The inherited resource |frameType| is given a default value of
|XfwfSunken|, instead of the inherited default |XfwfRaised|. The frame
width is set to a default of 2 pixels, instead of 0.

	@var frameType = XfwfSunken
	@var frameWidth = 2

@METHODS

@ If a child becomes managed or unmanaged, the RowCol widget
recomputes the positions of all managed children. That is done by a
method |layout|.

@proc change_managed
{
    $layout($);
}

@ The |layout| function is responsible for moving the children to their
positions in the grid. It is called from |change_managed|,
|geometry_manager| and |resize|.

@proc layout($)
{
    Position top, left, x, y;
    Dimension width, height, w, maxht;
    int i, j;
    Widget child;

    $compute_inside($, &left, &top, &width, &height);
    y = top;
    i = 0;
    while (i < $num_children) {
	child = $children[i];
	w = $child$width;			/* Accumulated width */
	maxht = $child$height;			/* Max. height in this row */
	j = i + 1;
	do {
	    if (j >= $num_children) break;
	    child = $children[j];
	    if (w + $child$width > width) break;
	    w += $child$width;
	    if ($child$height > maxht) maxht = $child$height;
	    j++;
	} while (1);
	for (x = left; i < j; i++) {
	    child = $children[i];
	    if ($alignTop)
		XtMoveWidget(child, x, y);
	    else
		XtMoveWidget(child, x, y + maxht - $child$height);
	    x += $child$width;
	}
	y += maxht;
    }
}

@ When a child wants to change its size or border width, it calls its
parent's |geometry_manager| method (through a call to
|XtMakeGeometryRequest| or |XtMakeResizeRequest|.) The RowCol widget
always grants size changes to its children. The size change is carried
out immediately and a new layout is computed. If a child requests a
change of position, the request is denied. A request for a change in
stacking order is ignored.

@proc geometry_manager
{
    Dimension newwd, newht, newbd;

    if (request->request_mode & (CWX | CWY)) return XtGeometryNo;
    if (request->request_mode & XtCWQueryOnly) return XtGeometryYes;

    newwd = request->request_mode & CWWidth ? request->width : $child$width;
    newht = request->request_mode & CWHeight ? request->height : $child$height;
    newbd = request->request_mode & CWBorderWidth
	? request->border_width : $child$border_width;

    if (newwd == $child$width && newht == $child$height
	&& newbd == $child$border_width) return XtGeometryNo;

    XtResizeWidget(child, newwd, newht, newbd);
    $layout($);
    return XtGeometryDone;
}

@ The |resize| method is called when the widget is resized. If the
|rows| and |columns| resources are both zero, the children will have
to be be re-aligned. In this case, there is no sense in asking the
parent for a new size, so |layout| is passed a value of |False|.

@proc resize
{
    $layout($);
}

@ The RowCol widget needs to recompute the positions of the children
when one of the resources changes. When the layout changes, the widget
also needs to be redrawn, of course.  The private variables are not
dependent on the resources, so they don't need recomputing.

@proc set_values
{
    Boolean need_layout = False;
    Boolean need_redisplay = False;

    if ($old$alignTop != $alignTop) {
	need_layout = True;
    }
    if (need_layout) {
	$layout($);
	need_redisplay = True;
    }
    return need_redisplay;
}
