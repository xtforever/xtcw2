# Icon.w -- XfwfIcon widget
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 1.3 (see README for history)

@class XfwfIcon (XfwfBoard) @file=Icon

@ The XfwfIcon widget displays an image. The preferred width and
height of the widget will be set to the width and height of the loaded
image, plus space for the frame. The widget has a callback
|activateCallback|, that is by default bound to a click of the left
mouse button.

The image is a |Pixmap|, optionally accompanied by a mask, to make
parts of the image transparent. When the image has a mask, the frame
around the widget will not be shown.

@PUBLIC

@ The image must be in |Pixmap| format. The width and height of the
image will be used to set the width and height of the widget.  There
is a converter for strings (defined in the Common widget), which will
try to interpret a string as a file name and load the file. The file
must be in XPM format. The converter also knows about some built-in
images, called |"FATAL"|, |"ERROR"|, |"WARNING"|, |"QUESTION"|,
|"INFO"| and |"NONE"|.

	@var Icon* image = NULL

@ The callback is called by the |activate| action, which is by default
bound to a click of mouse button 1.

	@var <Callback> XtCallbackList activate = NULL

@ By default, icons do not have a frame.

	@var frameWidth = 0

@EXPORTS

@ The type |Icon| is defined in |Converters.h|

@incl <Xfwf/Converters.h>

@PRIVATE

@ The GC for drawing the image.

	@var GC image_gc

@METHODS

@ The |initialize| sets the (desired and actual) size of the widget to
the size of the image. The GC is initialized. The width and height
of the icon must be set in the icon's attributes.

@proc initialize
{
    Dimension dummy1, dummy2;
    Position x, y;

    $image_gc = NULL;
    create_image_gc($);
    if ($image && $image->pixmap != None) {
	$compute_inside($, &x, &y, &dummy1, &dummy2);
	$width = $image->attributes.width + 2*x;
	$height = $image->attributes.height + 2*y;
	/* fprintf(stderr, "Icon.initialize: width=%d height=%d\n",
		$width, $height); */
    } else {
	$compute_inside($, &x, &y, &dummy1, &dummy2);
	$width = 10 + 2*x;
	$height = 10 + 2*y;
	/* fprintf(stderr, "Icon.initialize: width=%d height=%d (no icon)\n",
		$width, $height); */
    }
}

@ The |expose| method simply draws the image (if any) against the top
left corner of the widget. The window is already masked with the
image's mask. After that the image is drawn with a call to
|XFillRectangle|. Finally, |expose| calls the superclass's |expose|
method to draw the frame.

@proc expose
{
    Dimension wd, ht;
    Position x, y;

    if (! XtIsRealized($)) return;
    if ($image && $image->pixmap != None) {
	$compute_inside($, &x, &y, &wd, &ht);
	XFillRectangle(XtDisplay($), XtWindow($), $image_gc, x, y, wd, ht);
    }
    #expose($, event, region);
}

@ A change of image also causes a change in size and a change in the
shape mask.

@proc set_values
{
    Dimension dummy1, dummy2;
    Position x, y;
    Boolean need_redraw = False;

    if ($old$frameWidth != $frameWidth
	|| $old$outerOffset != $outerOffset
	|| $old$image != $image) {
	create_image_gc($);
	need_redraw = True;
    }
    if ($old$image != $image) {
	if (! $image) {				/* Remove shape mask */
	    XShapeCombineMask(XtDisplay($), XtWindow($), ShapeBounding,
			      0, 0, None, ShapeSet);
	} else {				/* New size and shape */
	    $compute_inside($, &x, &y, &dummy1, &dummy2);
	    if ($image->pixmap != None
		&& ($image->attributes.valuemask & XpmSize) != 0)
		$set_abs_location($, CWWidth | CWHeight, 0, 0,
				  $image->attributes.width + 2*x,
				  $image->attributes.height + 2*y);
	    if (XtIsRealized($))
		XShapeCombineMask(XtDisplay($), XtWindow($), ShapeBounding,
				  x, y, $image->mask, ShapeSet);
	}
	need_redraw = True;
    }
    return need_redraw;
}

@ When the Widget is realized, the window is immediately combined with
the icon's mask.

@proc realize
{
    Dimension wd, ht;
    Position x, y;

    #realize($, mask, attributes);
    $compute_inside($, &x, &y, &wd, &ht);
    XShapeCombineMask(XtDisplay($), XtWindow($), ShapeBounding, x, y,
		      $image ? $image->mask : None, ShapeSet);
}    

@TRANSLATIONS

@ By default, the |activate| action is bound to a mouse click and to
the Return key.

	@trans <Btn1Down>,<Btn1Up>: activate()
	@trans <Key>Return: activate()


@ACTIONS

@ The |activate| action just calls the |activate| callback functions,
passing the |XEvent| pointer in the |call_data| argument.

@proc activate
{
    XtCallCallbackList($, $activate, event);
}

@UTILITIES

@ The GC is created by a utility function. It sets the fill style to
|FillTiled| and the origin to the coordinates just inside the widget's
frame

@proc create_image_gc($)
{
    Dimension wd, ht;
    Position x, y;
    XtGCMask mask = GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin;
    XGCValues values;

    if ($image_gc != NULL) XtReleaseGC($, $image_gc);
    $compute_inside($, &x, &y, &wd, &ht);
    if ($image && $image->pixmap != None) {
	values.tile = $image->pixmap;
	mask |= GCTile;
    }
    values.fill_style = FillTiled;
    values.ts_x_origin = x;
    values.ts_y_origin = y;
    $image_gc = XtGetGC($, mask, &values);
}

@IMPORTS

@incl <X11/extensions/shape.h>
@incl <stdio.h>
