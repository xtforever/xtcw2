# Animator -- Animator widget
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 1.0.1 (see README for history)

@class XfwfAnimator (XfwfBoard) @file=Animator

@ The animator widget cycles through a series of images (XImage
structures). Each image is displayed for a certain number of
milliseconds. After the last image, the animation can either start
over or stop with the last image displayed.

The animation must be started with |XfwfStartAnimation| and it may be
stopped with |XfwfStopAnimation|.

The widget uses the location resources (see XfwfBoard(3)) for
positioning only; the preferred size of the widget will be the size of
the largest image, plus room for the frame.

@PUBLIC

@ The list of images can be specified in the resource file as a series
of filenames of files in XPM format. If it is set from an application,
the list must end with a |NULL|. Every element is a pointer to an
|XImage| structure.

@var <ImageList> XImage **images = NULL

@ The time that each image will be displayed can be set with
|intervals|. It is a list of cardinals, each representing a number of
milliseconds. The list must end with a zero. If there are insufficient
numbers in the list, the last number will be used as the interval for
any remaining images. If the list is empty (NULL), the interval will
be |defaultInterval| (normally 500).

@var <CardinalList> Cardinal *intervals = NULL

@ If the list of intervals is empty, the interval will be
|defaultInterval| milliseconds.

@var Cardinal defaultInterval = 500

@ The animation repeats indefinitely by default. When the |cycle|
resource is set to |False| the animation sequence is shown once only,
for every call to |XfwfStartAnimation|.

@var Boolean cycle = True

@ The default type of frame is set to `sunken'.

@var frameType = XfwfSunken

@ If there is no image to display, the widget has no obvious size. The
following defaults for the location resources create a 7 by 7 pixel
widget.

@var rel_width = <String> "0.0"
@var abs_width = 7
@var rel_height = <String> "0.0"
@var abs_height = 7

@PRIVATE

@ The images will be changed by a time-out routine. The timer for that
routine is stored in |timer|.

@var XtIntervalId timer

@ The current image.

@var int cur_image

@ The current interval, if |intervals| is not |NULL|.

@var int cur_interval

@ The GC for drawing the image.

@var GC image_gc

@EXPORTS

@ To start the animation an application should call
|XfwfStartAnimation|. If the animation is already running, it will be
restarted from the beginning.

@proc XfwfStartAnimation($)
{
    if (! XtIsSubclass($, xfwfAnimatorWidgetClass))
	XtWarning("Illegal widget type in XfwfStartAnimation");
    else
	$start_animation($);
}

@ The animation can be stopped with |XfwfStopAnimation|.

@proc XfwfStopAnimation($)
{
    if (! XtIsSubclass($, xfwfAnimatorWidgetClass))
	XtWarning("Illegal widget type in XfwfStopAnimation");
    else
	$stop_animation($);
}

@METHODS

@ |class_initialize| installs the type converters for a list of images
and a list of cardinal numbers.

@proc class_initialize
{
    XtSetTypeConverter(XtRString, XtRImageList, cvtStringToImageList,
		       NULL, 0, XtCacheNone, NULL);
    XtSetTypeConverter(XtRString, XtRCardinalList, cvtStringToCardinalList,
		       NULL, 0, XtCacheNone, NULL);
}

@ The |initialize| sets the (desired and actual) size of the widget to
the size of the largest image. The GC is initialized to all default
values.

@proc initialize
{
    Dimension wd, ht, dummy1, dummy2;
    Position x, y;
    XImage **p;
    XtGCMask value_mask = 0;
    XGCValues values;

    $timer = 0;
    $image_gc = XtGetGC($, value_mask, &values);
    if ($images != NULL) {
	$compute_inside($, &x, &y, &dummy1, &dummy2);
	wd = 1; ht = 1;
	for (p = $images; *p; p++) {
	    if ((*p)->width > wd) wd = (*p)->width;
	    if ((*p)->height > ht) ht = (*p)->height;
	}
	XtVaSetValues($, XtNwidth, wd + 2*x, XtNheight, ht + 2*y, NULL);
    }
}

@ The |start_animation| method is usually called by the
|XfwfStartAnimation| routine.  If an animation is already running, the
time-out function must be removed first. To draw the first image, the
routine calls the |expose| method with the |event| and |region|
parameters set to |NULL|.

@proc start_animation($)
{
    Cardinal delay;

    if (! $images) return;
    if ($timer) XtRemoveTimeOut($timer);
    $cur_image = 0;
    $cur_interval = 0;
    if (! $intervals || $intervals[0] == 0)
	delay = $defaultInterval;
    else
	delay = $intervals[$cur_interval];
    $expose($, NULL, NULL);
    $timer = XtAppAddTimeOut(XtWidgetToApplicationContext($), delay,
			     next_image, $);
}

@ The |stop_animation| method simply removes the time-out function. It
leaves the current image undisturbed.

@proc stop_animation($)
{
    if ($timer) {
	XtRemoveTimeOut($timer);
	$timer = 0;
    }
}

@ The |expose| method simply draws the current image (if any) against
the top left corner of the widget. It then calls the superclass's
|expose| method to draw the frame.

@proc expose
{
    Dimension wd, ht;
    Position x, y;

    if (! XtIsRealized($)) return;
    if (region != NULL)
	XSetRegion(XtDisplay($), $image_gc, region);
    $compute_inside($, &x, &y, &wd, &ht);
    if ($images) XPutImage(XtDisplay($), XtWindow($), $image_gc,
			   $images[$cur_image], 0, 0, x, y,
			   $images[$cur_image]->width,
			   $images[$cur_image]->height);
    if (region != NULL)
	XSetClipMask(XtDisplay($), $image_gc, None);
    #expose($, event, region);
}

@proc set_values
{
    Dimension wd, ht, dummy1, dummy2;
    Position x, y;
    XImage **p;
    Boolean restart = False;

    if ($old$images != $images) {
	if ($timer) restart = True;
	$stop_animation($);
	$compute_inside($, &x, &y, &dummy1, &dummy2);
	wd = 1; ht = 1;
	for (p = $images; *p; p++) {
	    if ((*p)->width > wd) wd = (*p)->width;
	    if ((*p)->height > ht) ht = (*p)->height;
	}
	XtVaSetValues($, XtNwidth, wd + 2*x, XtNheight, ht + 2*y, NULL);
    }
    if ($old$intervals != $intervals) {
	if ($timer) restart = True;
	$stop_animation($);
    }
    if (restart) $start_animation($);
    return False;
}

@UTILITIES

@ The timer callback routine checks if there is another image to
display, either the next image or the first image, if |cycle| is
|True|. It determines the correct interval for the new image, which
can be the next number in |intervals|, or the previous number, if
there is no next, or the |defaultInterval|, if there is no |interval|
at all. It then re-installs itself as a time-out routine.

@proc next_image(XtPointer client_data, XtIntervalId *timer)
{
    Widget $ = (Widget) client_data;
    Cardinal delay;

    if ($images[$cur_image+1] == NULL) {
	if (! $cycle) return;			/* Nothing more */
	$cur_image = 0;
	$cur_interval = 0;
	if ($intervals == NULL || $intervals[0] == 0)
	    delay = $defaultInterval;
	else
	    delay = $intervals[0];
    } else {
	$cur_image++;
	if ($intervals == NULL)
	    delay = $defaultInterval;
	else if ($intervals[$cur_interval+1] == 0)
	    delay = $intervals[$cur_interval];
	else {
	    $cur_interval++;
	    delay = $intervals[$cur_interval];
	}
    }
    $expose($, NULL, NULL);
    $timer = XtAppAddTimeOut(XtWidgetToApplicationContext($), delay,
			     next_image, $);
}

@ The converter from a string to a list of images splits the string in
words separated by spaces and/or commas. Each word must be the name of
a file containing a pixmap in XPM format. The pixmap is read into an
|XImage|.

@def done(type, value) =
  do {
      if (to->addr != NULL) {
	  if (to->size < sizeof(type)) {
	      to->size = sizeof(type);
	      return False;
	  }
	  *(type*)(to->addr) = (value);
      } else {
	  static type static_val;
	  static_val = (value);
	  to->addr = (XtPointer)&static_val;
      }
      to->size = sizeof(type);
      return True;
  } while (0)

@proc Boolean cvtStringToImageList(Display *display, XrmValuePtr args, Cardinal *num_args, XrmValuePtr from, XrmValuePtr to, XtPointer *converter_data)
{
    String p;
    XImage *im, *shape, **list = NULL;
    int status, n = 0;
    Cardinal one = 1;

    if (*num_args != 0)
	XtAppErrorMsg
	    (XtDisplayToApplicationContext(display),
	     "cvtStringToImageList", "wrongParameters",
	     "XtToolkitError",
	     "String to image list type conversion needs no arguments", 
	     (String*) NULL, (Cardinal*) NULL);

    p = strtok((String) from->addr, " \t\n,");
    while (p) {
	status = XpmReadFileToImage(display, p, &im, &shape, NULL);
	switch (status) {
	case XpmOpenFailed:
	case XpmFileInvalid:
	case XpmNoMemory:
	    XtAppWarningMsg
		(XtDisplayToApplicationContext(display),
		 "cvtStringToImageList", "fileError",
		 "XtToolkitError",
		 "Failed to read image from \"%s\"",
		 &p, &one);
	    break;	    
	case XpmColorError:
	case XpmColorFailed:
	    XtAppWarningMsg
		(XtDisplayToApplicationContext(display),
		 "cvtStringToImageList", "allocColor",
		 "XtToolkitError",
		 "Could not get (all) colors for image \"%s\"",
		 &p, &one);
	case XpmSuccess:
	    n++;
	    list = (XImage**) XtRealloc((char*) list, n * sizeof(im));
	    list[n-1] = im;
	}
	p = strtok(NULL, " \t\n,");
    }
    if (n > 0) {
	n++;
	list = (XImage**) XtRealloc((char*) list, n * sizeof(im));
	list[n-1] = NULL;
    }
    done(XImage**, list);
}

@proc Boolean cvtStringToCardinalList(Display *display, XrmValuePtr args, Cardinal *num_args, XrmValuePtr from, XrmValuePtr to, XtPointer *converter_data)
{
    String p;
    Cardinal *list = NULL;
    int n = 0, h, stat;

    if (*num_args != 0)
	XtAppErrorMsg
	    (XtDisplayToApplicationContext(display),
	     "cvtStringToCardinalList", "wrongParameters",
	     "XtToolkitError",
	     "String to cardinal list type conversion needs no arguments", 
	     (String*) NULL, (Cardinal*) NULL);

    p = (char *) from->addr;
    do {
	n++;
	list = (Cardinal*) XtRealloc((char*) list, n * sizeof(Cardinal));
	/* list[n-1] = strtoul(p, &p, 0); */
	stat = sscanf(p, "%u%n", &list[n-1], &h);
	p = p + h;
    } while (stat == 1 && list[n-1] != 0);
    list[n-1] = 0;
    done(Cardinal *, list);
}
	
@IMPORTS

@incl <X11/xpm.h>
@incl <stdio.h>
