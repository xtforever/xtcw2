# XmIcon.w -- XmIcon widget
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 1.0 for FWF V4.0 (see README for history)

@class XmIcon (XmManager) @file=XmIcon

@ The XmIcon widget displays an image. The preferred width and height
of the widget will be set to the width and height of the loaded
image. The widget has a callback |activateCallback|, that is by
default bound to a click of the left mouse button.

The image is a |Pixmap|, optionally accompanied by a mask, to make
parts of the image transparent.

@PUBLIC

@ The image must be in |Pixmap| format. The width and height of the
image will be used to set the width and height of the widget.  There
is a converter for strings, which will
try to interpret a string as a file name and load the file. The file
must be in XPM format. The converter also knows about some built-in
images, called |"FATAL"|, |"ERROR"|, |"WARNING"|, |"QUESTION"|,
|"INFO"| and |"NONE"|.

	@var Icon* image = NULL

@ The callback is called by the |activate| action, which is by default
bound to a click of mouse button 1.

	@var <Callback> XtCallbackList activate = NULL

@EXPORTS

@ The type |Icon| is defined in |Converters.h|

@incl <Xfwf/Converters.h>

@CLASSVARS

@ Set a few class variables.

	@var compress_motion = True
	@var compress_exposure = XtExposeCompressMultiple
	@var compress_enterleave = True

@METHODS

@ The converter from String to Icon needs one extra argument, viz.,
the widget for which the icon is loaded. An offset of 0 should give a
pointer to the widget itself.

@proc class_initialize
{
    static XtConvertArgRec args[] = {
    { XtWidgetBaseOffset, 0, sizeof(Widget) } };

    XtSetTypeConverter(XtRString, "Icon", cvtStringToIcon,
		       args, XtNumber(args), XtCacheNone, NULL);
}

@ The |initialize| method sets the (desired and actual) size of the
widget to the size of the image. The width and height of the icon must
be set in the icon's attributes.

@proc initialize
{
    if ($image)
	XtVaSetValues
	    ($,
	     XtNwidth, $image->attributes.width,
	     XtNheight, $image->attributes.height,
	     XtNbackgroundPixmap, $image->pixmap,
	     NULL);
}

@ A change of image also causes a change in size.

@proc set_values
{
    if ($old$image != $image) {
	if (! $image) {
	    XtVaSetValues($, XtNbackgroundPixmap, None, NULL);
	} else {
	    XtVaSetValues
		($,
		 XtNwidth, $image->attributes.width,
		 XtNheight, $image->attributes.height,
		 XtNbackgroundPixmap, $image->pixmap,
		 NULL);
	    if (XtIsRealized($))
		XShapeCombineMask(XtDisplay($), XtWindow($), ShapeBounding,
				  0, 0, $image->mask, ShapeSet);
	}
    }
    return FALSE;
}

@ When the Widget is realized, the window is immediately combined with
the icon's mask.

@proc realize
{
    #realize($, mask, attributes);
    XShapeCombineMask(XtDisplay($), XtWindow($), ShapeBounding, 0, 0,
		      $image ? $image->mask : None, ShapeSet);
}

@ When the widget is destroyed, the memory for the Pixmaps and the
attributes can be freed.

@proc destroy
{
    if ($image) {
	XFreePixmap(XtDisplay($), $image->pixmap);
	XFreePixmap(XtDisplay($), $image->mask);
	XpmFreeAttributes(&$image->attributes);
    }
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

@IMPORTS

@incl <X11/extensions/shape.h>
@incl <stdio.h>
@incl <X11/xpm.h>
