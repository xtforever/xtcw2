# XfwfwEnforcer -- apply location resources to a child
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 1.0 (Feb 1995)

@class XfwfEnforcer (XfwfBoard) @file=Enforcer

@ The Enforcer widget can be used to apply location resources to a
widget that is not a subclass of XfwfBoard. The Widget accepts a
single child and forces that child to the same size as itself (minus
the frame).

It can also be used to put a frame around some widget.

@METHODS

@ The |resize| method passes on the resize message to its child, after
decreasing the area by the amount needed for the frame.

@proc resize
{
    Position x, y;
    Dimension w, h;
    Widget child;

    if ($num_children == 0) return;
    $compute_inside($, &x, &y, &w, &h);
    child = $children[0];
    w -= 2 * $child$border_width;
    h -= 2 * $child$border_width;
    XtConfigureWidget(child, x, y, w, h, $child$border_width);
}

@ The |change_managed| method is called when a child becomes managed
or unmanaged. The task of the routine is enforcing the layout policy,
which in this case consists of resizing the child to fit inside the
frame.

@proc change_managed
{
    Position x, y;
    Dimension w, h;
    Widget child;

    if ($num_children == 0) return;
    $compute_inside($, &x, &y, &w, &h);
    child = $children[0];
    w -= 2 * $child$border_width;
    h -= 2 * $child$border_width;
    XtConfigureWidget(child, x, y, w, h, $child$border_width);
}

@ If a child requests to be resized, the request is always ignored.

@proc geometry_manager
{
    return XtGeometryNo;
}
