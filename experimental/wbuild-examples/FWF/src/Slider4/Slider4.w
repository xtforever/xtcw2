# Slider4 widget
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 2.2

@class XfwfSlider4 (XfwfSlider2) @file=Slider4

@ The Slider4 class is equal to the Slider2 class, except for the
addition of a sash in the lower right corner of the thumb, with which
the thumb can be resized. Since this adds two more dgrees of freedom
to the widget (viz., width and height in addition to horizontal and
vertical position), the name becomes ``Slider4.'' A callback function
is called when the thumb is being resized and another one when the
resizing has finished.

The control area in the lower right corner of the thumb looks like a
triangular button. The actions that already existed in the Slider2
class now also check if the mouse is on that button and resize instead
of drag the button in response.

@public

@ The existence of another area in the widget means that that there
are additional resources needed for determining how it will look. The
first of these resources is |triangleColor|, which gives the color of
the triangle, unless |trianglePixmap| is also defined. In that case
the pixmap will be used to tile the triangle with.

	@var Color sashColor = <String> XtDefaultBackground

@ When the |sashPixmap| resource is set, it overrides the |sashColor|
resource and causes the triangle to be tiled with a pixmap.

	@var Pixmap sashPixmap = NULL

@ The triangle also has a frame in one of the four types |XfwfRaised|,
|XfwfSunken|, |XfwfLedged| or |XfwfChiseled|. This frame is also influenced
by the setting of the |shadowScheme|, |topShadowColor|, etc., resources
(see the Frame class). The type of frame is set with |sashFrameType|.

	@var FrameType sashFrameType = XfwfRaised

@ The width of the frame around the sash:

	@var Dimension sashFrameWidth = 2


@private

@ Three more GC's are needed to draw the sash and the frame around it.

	@var GC sashgc
	@var GC sashlightgc
	@var GC sashdarkgc

@ The boolean |resize_in_progress| is set to |True| when a resizing
action is initiated (by pressing a mouse button on the sash).

	@var Boolean resize_in_progress


@translations

@ The default translations are the same as in the Slider2 widget, but
the actions have added functionality.

	@trans <Btn1Down>: start()
	@trans <Btn1Motion>: drag()
	@trans <Btn1Up>: finish()


@actions

@ The |start| action checks where the mouse is and does one of three
things depending on the mouse position. If the mouse is outside the
thumb, the |scrollCallback| list is called with as |call_data| argument
the direction in which the mouse click occurred. If the mouse is on
the thumb, but not in the triangle, a drag operation is started and
|start| sets some private variables for use by the next action. If the
mouse is on the triangle, a resize operation is started and the
private variables are set accordingly.

@proc start
{
    Dimension w, h;
    Position x, y;
    float b;
    Boolean outside = False;
    XfwfScrollInfo info;

    if (event->type != ButtonPress && event->type != ButtonRelease
        && event->type != MotionNotify)
        XtError("The start action must be bound to a mouse event");

    $compute_thumb($, &x, &y, &w, &h);

    if (event->xbutton.x < x) {			/* Left of thumb */
	info.reason = XfwfSPageLeft;
	info.flags = XFWF_HPOS;			/* Suggest a value: */
	info.hpos = max(0.0, $thumb_x - $thumb_wd);
	outside = True;
	XtCallCallbackList($, $scrollCallback, &info);
    }
    if (event->xbutton.x >= x + w) {		/* Right of thumb */
	info.reason = XfwfSPageRight;
	info.flags = XFWF_HPOS;			/* Suggest a value: */
	info.hpos = min(1.0, $thumb_x + $thumb_wd);
	outside = True;
	XtCallCallbackList($, $scrollCallback, &info);
    }
    if (event->xbutton.y < y) {			/* Above thumb */
	info.reason = XfwfSPageUp;
	info.flags = XFWF_VPOS;			/* Suggest a value: */
	info.vpos = max(0.0, $thumb_y - $thumb_ht);
	outside = True;
	XtCallCallbackList($, $scrollCallback, &info);
    }
    if (event->xbutton.y >= y + h) {		/* Below thumb */
	info.reason = XfwfSPageDown;
	info.flags = XFWF_VPOS;			/* Suggest a value: */
	info.vpos = min(1.0, $thumb_y + $thumb_ht);
	outside = True;
	XtCallCallbackList($, $scrollCallback, &info);
    }

    if (! outside) {				/* Mouse inside the thumb */
        b = y + x + h + w - 2*$thumbFrameWidth
	    - (min(h, w) - 2 * $thumbFrameWidth)/2;
        if (event->xbutton.y >= -event->xbutton.x + b) {
            $resize_in_progress = True;		/* Inside the triangle */
            $m_delta_x = w - event->xbutton.x;
            $m_delta_y = h - event->xbutton.y;
        } else {
            $drag_in_progress = True;		/* Outside the triangle */
            $m_delta_x = x - event->xbutton.x;
            $m_delta_y = y - event->xbutton.y;
        }
    }
}

@ The |finish| action checks what type of action just finished and
either does nothing (if the action was a click outside the thumb),
invokes the drop callbacks (if the mouse was dragging the thumb) or
invokes the resize callbacks (if the mouse was pulling the sash). The
callbacks get a pointer to a |XfwfScrollInfo| structure as |call_data|
argument.

@proc finish
{
    XfwfScrollInfo info;

    if ($drag_in_progress) {
        $drag_in_progress = False;
	info.reason = XfwfSMove;
	info.flags = XFWF_VPOS | XFWF_HPOS;
	info.hpos = $thumb_x;
	info.vpos = $thumb_y;
	XtCallCallbackList($, $scrollCallback, &info);
    } else if ($resize_in_progress) {
        $resize_in_progress = False;
	info.reason = XfwfSZoom;
	info.flags = XFWF_VSIZE | XFWF_HSIZE;
	info.vsize = $thumb_ht;
	info.hsize = $thumb_wd;
	XtCallCallbackList($, $scrollCallback, &info);
    }
}

@ The |drag| action moves the thumb if a drag is in progress or
resizes the thumb if a resize operation is in progress; otherwise, it
does nothing. It also calls the |scrollCallback|s.  Moving the thumb
is done by calling the |move_thumb| method, resizing is done by
calling |expose|.

@proc drag
{
    Dimension oldw, oldh, neww, newh, fw, fh, wd, ht;
    Position newx, newy, x, y, oldx, oldy, fx, fy;
    float dum1, dum2;
    XfwfScrollInfo info;
    XEvent pseudoevent;
    XRectangle rect;
    Region reg;

    if (event->type != ButtonPress && event->type != ButtonRelease
        && event->type != MotionNotify)
        XtError("The drag action must be bound to a mouse event");
    if ($drag_in_progress) {
        $compute_thumb($, &oldx, &oldy, &wd, &ht);
        newx = event->xbutton.x + $m_delta_x;
        newy = event->xbutton.y + $m_delta_y;
        $compute_info($, &newx, &newy,&wd,&ht,&$thumb_x,&$thumb_y,&dum1,&dum2);
        $move_thumb($, oldx, oldy, wd, ht, newx, newy);
	info.reason = XfwfSDrag;
	info.flags = XFWF_VPOS | XFWF_HPOS;
        info.hpos = $thumb_x;
        info.vpos = $thumb_y;
        XtCallCallbackList($, $scrollCallback, &info);
    } else if ($resize_in_progress) {
        $compute_thumb($, &x, &y, &oldw, &oldh);
        neww = event->xbutton.x + $m_delta_x;
        newh = event->xbutton.y + $m_delta_y;
        $compute_info($, &x, &y,&neww,&newh,&dum1,&dum2,&$thumb_wd,&$thumb_ht);
        rect.x = pseudoevent.xexpose.x = x;
        rect.y = pseudoevent.xexpose.y = y;
	rect.height = pseudoevent.xexpose.height = max(oldh, newh);
	rect.width = pseudoevent.xexpose.width = max(oldw, neww);
	XClearArea(XtDisplay($), XtWindow($), x, y, rect.width,
		   rect.height, False);
        reg = XCreateRegion();
        XUnionRectWithRegion(&rect, reg, reg);
        $expose($, &pseudoevent, reg);
        XDestroyRegion(reg);
	info.reason= XfwfSStretch;
	info.flags = XFWF_VSIZE | XFWF_HSIZE;
        info.hsize = $thumb_wd;
        info.vsize = $thumb_ht;
        XtCallCallbackList($, $scrollCallback, &info);
    }
}

@methods

@ The |initialize| method sets the private variables and calls three
routines to set the GC's for drawing the triangle.

@proc initialize
{
    $sashgc = NULL; create_sashgc($);
    $sashlightgc = NULL; create_sashlightgc($);
    $sashdarkgc = NULL; create_sashdarkgc($);
    $resize_in_progress = False;
}

@ The |expose| method uses the |expose| method of the superclass to
draw the widget and then adds the triangle in the lower right corner
of the thumb.  Drawing the triangle is so simple (for the X server,
not for the programmer :-) that it isn't useful to set the clipping
rectangle.

@proc expose
{
    Position x, y;
    Dimension w, h, b;
    XPoint p[9];

    if (! XtIsRealized($)) return;
    #expose($, event, region);
    $compute_inside($, &x, &y, &w, &h);
    b = min(h, w)/2;
    p[0].x = x + w - b;
    p[0].y = y + h;
    p[1].x = x + w;
    p[1].y = p[0].y;
    p[2].x = p[1].x;
    p[2].y = y + h - b;
    XFillPolygon(XtDisplay($), XtWindow($), $sashgc, p, 3, Convex,
		 CoordModeOrigin);
    if ($sashFrameType == XfwfRaised || $sashFrameType == XfwfSunken) {
	p[0].x = x + w - b + 2 * $sashFrameWidth;
	p[0].y = y + h - $sashFrameWidth;
	p[1].x = x + w - $sashFrameWidth;
	p[1].y = p[0].y;
	p[2].x = p[1].x;
	p[2].y = y + h - b + 2 * $sashFrameWidth;
        p[3].x = x + w;
        p[3].y = y + h - b;
        p[4].x = p[3].x;
        p[4].y = y + h;
        p[5].x = x + w - b;
        p[5].y = p[4].y;
        XFillPolygon(XtDisplay($), XtWindow($),
		     $sashFrameType == XfwfRaised ? $sashdarkgc : $sashlightgc,
		     p, 6, Nonconvex, CoordModeOrigin);
	p[1].x = p[2].x;
	p[1].y = p[2].y;
	p[2].x = p[3].x;
	p[2].y = p[3].y;
	p[3].x = p[5].x;
	p[3].y = p[5].y;
        XFillPolygon(XtDisplay($), XtWindow($),
		     $sashFrameType == XfwfSunken ? $sashdarkgc : $sashlightgc,
		     p, 4, Nonconvex, CoordModeOrigin);
    } else {
	p[0].x = x + w - b + 2 * $sashFrameWidth;
	p[0].y = y + h - $sashFrameWidth;
	p[1].x = x + w - $sashFrameWidth;
	p[1].y = p[0].y;
	p[2].x = p[1].x;
	p[2].y = y + h - b + 2 * $sashFrameWidth;
        p[3].x = x + w - $sashFrameWidth/2;
        p[3].y = y + h - b + $sashFrameWidth;
        p[4].x = p[3].x;
        p[4].y = y + h - $sashFrameWidth/2;
        p[5].x = x + w - b + $sashFrameWidth;
        p[5].y = p[4].y;
        XFillPolygon(XtDisplay($), XtWindow($),
		     $sashFrameType == XfwfChiseled ? $sashdarkgc : $sashlightgc,
		     p, 6, Nonconvex, CoordModeOrigin);
        p[2].x = x + w;
        p[2].y = y + h - b;
        p[1].x = p[2].x;
        p[1].y = y + h;
        p[0].x = x + w - b;
        p[0].y = p[1].y;
        XFillPolygon(XtDisplay($), XtWindow($),
		     $sashFrameType == XfwfChiseled ? $sashlightgc : $sashdarkgc,
		     p, 6, Nonconvex, CoordModeOrigin);
        p[0].x = x + w - b + 2 * $sashFrameWidth;
        p[0].y = y + h - $sashFrameWidth;
        p[1].x = x + w - $sashFrameWidth;
        p[1].y = p[0].y;
        p[2].x = x + w - $sashFrameWidth/2;
        p[2].y = y + h - b + $sashFrameWidth;
        p[3].x = x + w - b + $sashFrameWidth;
        p[3].y = y + h - $sashFrameWidth/2;
        XFillPolygon(XtDisplay($), XtWindow($),
		     $sashFrameType == XfwfChiseled ? $sashlightgc : $sashdarkgc,
		     p, 4, Convex, CoordModeOrigin);
        p[0].x = x + w - b;
        p[0].y = y + h;
        p[1].x = x + w;
        p[1].y = y + h - b;
        XFillPolygon(XtDisplay($), XtWindow($),
		     $sashFrameType == XfwfChiseled ? $sashdarkgc : $sashlightgc,
		     p, 4, Convex, CoordModeOrigin);
    }
}

@ The |set_values| method is responsible for creating new GC's when
resources change. It also makes sure the |sashFrameWidth| is even when
the frame type is chiseled or ledged.

@proc set_values
{
    Boolean need_redisplay = False;

    if ($sashPixmap != $old$sashPixmap) {
        XFreePixmap(XtDisplay($), $old$sashPixmap);
        create_sashgc($);
        need_redisplay = True;
    } else if ($sashColor != $old$sashColor) {
        XFreePixmap(XtDisplay($), $sashPixmap);
        $sashPixmap = None;
        create_sashgc($);
        need_redisplay = True;
    }
    if ($sashFrameType != $old$sashFrameType)
        need_redisplay = True;
    if ($sashFrameType == XfwfChiseled || $sashFrameType == XfwfLedged)
        $sashFrameWidth = 2 * ($sashFrameWidth/2);
    if ($sashFrameWidth != $old$sashFrameWidth)
        need_redisplay = True;
    if ($shadowScheme != $old$shadowScheme
        || ($shadowScheme == XfwfAuto && $sashColor != $old$sashColor)
        || ($shadowScheme == XfwfStipple
	    && $topShadowStipple != $old$topShadowStipple)
        || ($shadowScheme == XfwfColor
	    && $topShadowColor != $old$topShadowColor)) {
        create_sashlightgc($);
        need_redisplay = True;
    }
    if ($shadowScheme != $old$shadowScheme
        || ($shadowScheme == XfwfAuto && $sashColor != $old$sashColor)
        || ($shadowScheme == XfwfStipple
	    && $bottomShadowStipple != $old$bottomShadowStipple)
        || ($shadowScheme == XfwfColor
	    && $bottomShadowColor != $old$bottomShadowColor)) {
        create_sashdarkgc($);
        need_redisplay = True;
    }
    return need_redisplay = True;
}

@utilities

@ The |create_sashgc| function creates a GC for drawing the sash.

@proc create_sashgc($)
{
    XtGCMask mask;
    XGCValues values;

    if ($sashPixmap != None) {
        mask = GCTile | GCFillStyle;
        values.tile = $sashPixmap;
        values.fill_style = FillTiled;
    } else {
        mask = GCForeground;
        values.foreground = $sashColor;
    }
    $sashgc = XtGetGC($, mask, &values);
}

@ The GC for the light parts of the frame around the triangle depends
on the setting of the |shadowScheme| resource.

@proc create_sashlightgc($)
{
    XtGCMask mask;
    XGCValues values;

    /* fprintf(stderr, "topShadowStipple = %ld\n", $topShadowStipple); */
    switch ($shadowScheme) {
    case XfwfColor:
	mask = GCForeground;
	values.foreground = $topShadowColor;
	break;
    case XfwfStipple:
	if (DefaultDepthOfScreen(XtScreen($)) > 4) {
            mask = GCForeground | GCFillStyle | GCStipple;
            values.foreground = WhitePixelOfScreen(XtScreen($));
            values.fill_style = FillStippled;
            values.stipple = $topShadowStipple;
        } else {
            mask = GCFillStyle | GCStipple | GCForeground | GCBackground;
            values.fill_style = FillOpaqueStippled;
            values.stipple = $topShadowStipple;
            values.foreground = BlackPixelOfScreen(XtScreen($));
            values.background = WhitePixelOfScreen(XtScreen($));
        }
	break;
    case XfwfAuto:
	if (DefaultDepthOfScreen(XtScreen($)) > 4
	    && $lighter_color($, $background, &values.foreground)) {
	    mask = GCForeground;
	} else {
	    mask = GCFillStyle | GCBackground | GCForeground | GCStipple;
	    values.fill_style = FillOpaqueStippled;
	    values.background = WhitePixelOfScreen(XtScreen($));
	    values.foreground = BlackPixelOfScreen(XtScreen($));
	    values.stipple = $lightstipple;
	}
	break;
    }
    /* fprintf(stderr, "values.stipple = %ld\n", values.stipple); */
    $sashlightgc = XtGetGC($, mask, &values);
}

@ The GC for the dark parts of the frame around the triangle.

@proc create_sashdarkgc($)
{
    XtGCMask mask;
    XGCValues values;

    switch ($shadowScheme) {
    case XfwfColor:
	mask = GCForeground;
	values.foreground = $bottomShadowColor;
	break;
    case XfwfStipple:
	if (DefaultDepthOfScreen(XtScreen($)) > 4) {
            mask = GCForeground | GCFillStyle | GCStipple;
            values.foreground = BlackPixelOfScreen(XtScreen($));
            values.fill_style = FillStippled;
            values.stipple = $topShadowStipple;
        } else {
            mask = GCFillStyle | GCStipple | GCForeground | GCBackground;
            values.fill_style = FillOpaqueStippled;
            values.stipple = $bottomShadowStipple;
            values.foreground = BlackPixelOfScreen(XtScreen($));
            values.background = WhitePixelOfScreen(XtScreen($));
        }
	break;
    case XfwfAuto:
	if (DefaultDepthOfScreen(XtScreen($)) > 4
	    && $darker_color($, $background, &values.foreground)) {
	    mask = GCForeground;
	} else {
	    mask = GCFillStyle | GCBackground | GCForeground | GCStipple;
	    values.fill_style = FillOpaqueStippled;
	    values.background = WhitePixelOfScreen(XtScreen($));
	    values.foreground = BlackPixelOfScreen(XtScreen($));
	    values.stipple = $darkstipple;
	}
	break;
    }
    $sashdarkgc = XtGetGC($, mask, &values);
}

