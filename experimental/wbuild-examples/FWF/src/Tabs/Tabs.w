# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# Version: 2.2 (12 Feb '96)

@class XfwfTabs (XfwfBoard)  @file=Tabs

@ The |XfwfTabs| widget displays a series of tabs, similar to the
alphabet tabs along the top of index cards. One tab, the front one, is
completely visible, the others are partially hidden behind it. Each of
the tabs can be clicked on with the mouse.

Although |XfwfTabs| is a descendant of |XfwfBoard|, |XfwfFrame| and
|XfwfCommon|, it does not currently use many of the resources defined
by those classes. In particular, this implementation does not take
part in keyboard traversal, and it has no 3D frame. The only resources
that it uses are the location resources defined in |XfwfBoard|.

@PUBLIC

@ The foreground color is the color used to draw the
text. |hlForeground| is the foreground for highlighted text.

	@var Color foreground = <String> XtDefaultForeground

@ The orientation of the tabs can be |XfwfUpTabs|, |XfwfDownTabs|,
|XfwfLeftTabs| or |XfwfRightTabs|. In the case of |XfwfLeftTabs| the
text is rotated 90 degress counterclockwise before being drawn, in
the case of |XfwfRightTabs| the labels are rotated clockwise.

There are converters that convert to and from strings |"up"|,
|"uptabs"|, |"left"|, |"lefttabs"|, etc, upper and lower case.

	@var TabsOrientation orientation = XfwfUpTabs

@ |lefttabs| is the number of partially visible tabs to the left of
the main one. (or above the main one, if the orientation is left or
right.)

	@var int lefttabs = 0

@ The number of tabs to the right of the main one.

	@var int righttabs = 0

@ The labels on each of the tabs must be simple strings. There is no
support for multiline labels or different fonts in this version of
|XfwfTabs|. The array |labels| should contain no less than |lefttabs +
righttabs + 1| strings. The widget makes private copies of the array
and of the strings.

The leftmost tab displays |labels[0]|. Note that the labels may be too
large to fit in the tab. In that case they are clipped. The left tabs
show only the initial part of the labels, the right tabs show the last
part of their labels and the main (front) tab shows the middle part of
its label.

	@var StringArray labels = NULL

@ The width of the tabs is given as a percentage of the total width
(or height, in case of left or right oriented tabs). Default is 50
percent. To make sure that the tabs don't overlap, use a
|tabWidthPercentage| less than 100 divided by the number of tabs
(i.e., |lefttabs + righttabs + 1|).

	@var int tabWidthPercentage = 50

@ The tabs have diagonal corners, the size of which is given with the
|cornerwidth| and |cornerheight| resources. |cornerwidth| is the size
of the corner in the direction parallel to the text.

	@var int cornerwidth = 3

@ The |cornerheight| resource is the size of the corner in the
direction perpendicular to the text.

	@var int cornerheight = 3

@ In case the |tabWidthPercentage| is 0, some extra space can be added
between the edge of each tab and the label.

	@var int textmargin = 3

@ The color of the left and right tabs. The main (front) tab is drawn
in the |background| color. Unless the |tabcolor| has been specified
explicitly, the left and right tabs have the same color as the main
tab.

	@var Color tabcolor = <CallProc> copy_bg

@ The text is drawn in the font which is given as the |font| resource.

	@var <FontStruct> XFontStruct *font = <String> XtDefaultFont

@ When the user clicks on a tab, a callback function is called. The
|call_data| argument to the callback will be a relative number: 0 if
the user clicked on the main tab, -1 if he clicked on the tab
immediately to the left, +1 if he clicked on the first right tab, and
higher numbers for tabs that are further removed from the main one.

	@var <Callback> XtCallbackList activate = NULL

@ Currently no keyboard traversal.

	@var traversalOn = False

@EXPORTS

	@type TabsOrientation = enum {
	    XfwfUpTabs, XfwfDownTabs, XfwfLeftTabs, XfwfRightTabs,
	}

@PRIVATE

@ A GC for drawing the labels.

	@var GC textgc

@ A GC for drawing the top shadow lines.

	@var GC topgc

@ A GC for drawing the top shadow lines.

	@var GC bottomgc

@ A GC for filling the main tab

	@var GC backgc

@ A GC for filling the other tabs.

	@var GC fillgc

@ |tabwidths| contains the widths of the tabs at the base. Unless
|tabWidthPercentage| is 0, all elements will have the same value.

	@var int *tabwidths

@ |offsets| contains the left (or top) coordinates of the tabs.

	@var int *offsets

@CLASSVARS

@ Set a few class variables.

	@var compress_motion = True
	@var compress_exposure = XtExposeCompressMultiple
	@var compress_enterleave = True

@METHODS

@proc class_initialize
{
    XtSetTypeConverter(XtRString, "StringArray",
		       cvtStringToStringArray,
		       NULL, 0, XtCacheNone, NULL);
    XtSetTypeConverter(XtRString, "TabsOrientation",
		       cvtStringToTabsOrientation,
		       NULL, 0, XtCacheNone, NULL);
    XtSetTypeConverter("TabsOrientation", XtRString,
		       cvtTabsOrientationToString,
		       NULL, 0, XtCacheNone, NULL);
}

@ The |initialize| method checks the resources and creates the local
variables. If the |tabWidthPercentage| is out of range, it is set to
50.

For the time being, there is no keyboard traversal.

@proc initialize
{
    String *h;
    int i;

    $traversalOn = FALSE;
    $topgc = NULL;
    create_topgc($);
    $bottomgc = NULL;
    create_bottomgc($);
    $textgc = NULL;
    create_textgc($);
    $fillgc = NULL;
    create_fillgc($);
    $backgc = NULL;
    create_backgc($);
    if ($labels) {
	h = (String*) XtMalloc(($lefttabs + $righttabs + 1) * sizeof(*h));
	for (i = $lefttabs + $righttabs; i >= 0; i--)
	    h[i] = XtNewString($labels[i]);
	$labels = h;
    }
    if ($tabWidthPercentage < 0 || $tabWidthPercentage > 100) {
	XtAppWarning(XtWidgetToApplicationContext($),
		     "tabWidthPercentage out of range; reset to 50");
	$tabWidthPercentage = 50;
    }
    $offsets = NULL;
    $tabwidths = NULL;
    compute_tabsizes($);
}

@ The |set_values| method checks changed resources and recomputes some
local variables. In this case the GC's may have to be changed.

@proc set_values
{
    Bool redraw = FALSE, resize_labels = FALSE;
    String *h;
    int i;

    if ($background != $old$background
	|| $background_pixmap != $old$background_pixmap
	|| $frameWidth != $old$frameWidth) {
	create_topgc($);
	create_bottomgc($);
	create_backgc($);
    }
    if ($foreground != $old$foreground || $font != $old$font) {
	create_textgc($);
	redraw = TRUE;
    }
    if ($tabcolor != $old$tabcolor) {
	create_fillgc($);
	redraw = TRUE;
    }
    if (($textmargin != $old$textmargin && $tabWidthPercentage == 0)
	|| $cornerwidth != $old$cornerwidth
	|| $cornerheight != $old$cornerheight) {
	resize_labels = TRUE;
    }
    if ($labels != $old$labels) {
	if ($labels) {
	    h = (String*) XtMalloc(($lefttabs + $righttabs + 1) * sizeof(*h));
	    for (i = $lefttabs + $righttabs; i >= 0; i--)
		h[i] = XtNewString($labels[i]);
	    $labels = h;
	}
	if ($old$labels) {
	    for (i = $old$lefttabs + $old$righttabs; i >= 0; i--)
		XtFree($old$labels[i]);
	    XtFree((XtPointer) $old$labels);
	}
	resize_labels = TRUE;
    }
    if ($tabWidthPercentage < 0 || $tabWidthPercentage > 100) {
	XtAppWarning(XtWidgetToApplicationContext($),
		     "tabWidthPercentage out of range; reset to 50");
	$tabWidthPercentage = 50;
    }
    if ($old$tabWidthPercentage != $tabWidthPercentage)
	resize_labels = TRUE;
    if ($lefttabs != $old$lefttabs || $righttabs != $old$righttabs)
	redraw = TRUE;
    if (resize_labels) {
	compute_tabsizes($);
	redraw = TRUE;
    }
    return redraw;
}

@ When the widget is realized or resized, a new shape mask is
installed. The shape mask makes the corners transparent.

@proc realize
{
    *mask |= CWBitGravity;
    attributes->bit_gravity = ForgetGravity;
    #realize($, mask, attributes);
    set_shape($);
}

@proc resize
{
    if (XtIsRealized($))
	XClearArea(XtDisplay($), XtWindow($), 0, 0, 0, 0, True);
    compute_tabsizes($);
    set_shape($);
    #resize($);
}

@ The |expose| method draws the tabs. First the tabs to the left, then
the tabs to the right, and finally the main one. Three private
routines do most of the work. The inherited |expose| method is called
to draw the frame, if any, although the tabs should look best without
a frame.

@proc expose
{
    int i;

    if (! XtIsRealized($)) return;

    switch ($orientation) {
    case XfwfUpTabs:
    case XfwfDownTabs:
	for (i = 0; i < $lefttabs; i++)
	    draw_hor_tab($, region, i);
	for (i = $lefttabs + $righttabs; i > $lefttabs; i--)
	    draw_hor_tab($, region, i);
	draw_hor_tab($, region, $lefttabs);
	break;
    case XfwfLeftTabs:
    case XfwfRightTabs:
	for (i = 0; i < $lefttabs; i++)
	    draw_ver_tab($, region, i);
	for (i = $lefttabs + $righttabs; i > $lefttabs; i--)
	    draw_ver_tab($, region, i);
	draw_ver_tab($, region, $lefttabs);
	break;
    }
    /* Focus highlight? */
}

@ TO DO: There should be some internal keyboard traversal, and also
new |border_highlight| and |border_unhighlight| methods.

@proc border_highlight
{
}

@proc border_unhighlight
{
}

@ |destroy| frees the memory that has been allocated by the widget.

@proc destroy
{
    int i;

    if ($labels) {
	for (i = $lefttabs + $righttabs; i >= 0; i--)
	    XtFree($labels[i]);
	XtFree((XtPointer) $labels);
    }
    if ($offsets)
	XtFree((XtPointer) $offsets);
    if ($tabwidths)
	XtFree((XtPointer) $tabwidths);
}

@UTILITIES

@def min(a, b) = ((a) < (b) ? (a) : (b))

@ |compute_tabsizes| computes the widths and offsets of each of the
tabs. If the |tabWidthPercentage| is 0, the widths will all be
different (namely the size of the label), otherwise they will all be
the same (|tabWidthPercentage * width|).

@proc compute_tabsizes($)
{
    int maxwd, basewidth, delta, i, n = $lefttabs + $righttabs + 1;
    int sum, len, h, length, breadth, shad = $frameWidth;

    if ($offsets) XtFree((XtPointer) $offsets);
    if ($tabwidths) XtFree((XtPointer) $tabwidths);
    $offsets = (XtPointer) XtMalloc(n * sizeof(*$offsets));
    $tabwidths = (XtPointer) XtMalloc(n * sizeof(*$tabwidths));

    if ($orientation == XfwfUpTabs || $orientation == XfwfDownTabs) {
	length = $width;
	breadth = $height;
    } else {
	length = $height;
	breadth = $width;
    }
    if ($tabWidthPercentage != 0) {		/* Fixed width tabs */
	basewidth = $tabWidthPercentage * length/100;
	if (n > 1) delta = (length - basewidth)/($lefttabs + $righttabs);
	for (i = 0; i < n; i++) {
	    $tabwidths[i] = basewidth;
	    $offsets[i] = i * delta;
	}
    } else if ($labels == NULL) {		/* Empty tabs */
	basewidth = length/n;
	delta = (length - basewidth)/($lefttabs + $righttabs);
	for (i = 0; i < n; i++) {
	    $tabwidths[i] = basewidth;
	    $offsets[i] = i * delta;
	}
    } else {					/* Variable width tabs */
	sum = 0;
	h = 2 * ($cornerwidth + shad + $textmargin);
	maxwd = length - n * (shad + $textmargin);
	for (i = 0; i < n; i++) {
	    len = strlen($labels[i]);
	    $tabwidths[i] = min(maxwd, XTextWidth($font,$labels[i],len) + h);
	    sum += $tabwidths[i];
	}
	$offsets[0] = 0;
	if (length >= sum)
	    delta = (length - sum)/(n - 1);	/* Between tabs */
	else
	    delta = -((sum - length + n - 2)/(n - 1)); /* Round down! */
	for (i = 1; i < n; i++)
	    $offsets[i] = $offsets[i-1] + $tabwidths[i-1] + delta;
    }
}

@proc comp_hor_tab_shape($, int i, XPoint p[12], int *x0, int *x1, int *midy)
{
    int shad = $frameWidth;
    int k = min($cornerheight, ($height - shad)/2);
    /*
     *                4 o-------------o 5
     *                 /               \
     *              3 o                 o 6
     *                |                 |
     *              2 o                 o 7
     *             1 /                   \ 8
     *   0 o--------o                     o--------o 9
     *  11 o---------------------------------------o 10
     *
     *  11 o---------------------------------------o 10
     *   0 o--------o                     o--------o 9
     *             1 \                   / 8
     *              2 o                 o 7
     *                |                 |
     *              3 o                 o 6
     *                 \               /
     *                4 o-------------o 5
     */
    p[0].x = 0;
    p[1].x = $offsets[i];
    p[2].x = $offsets[i] + $cornerwidth;
    p[3].x = $offsets[i] + $cornerwidth;
    p[4].x = $offsets[i] + 2 * $cornerwidth;
    p[5].x = $offsets[i] + $tabwidths[i] - 2 * $cornerwidth;
    p[6].x = $offsets[i] + $tabwidths[i] - $cornerwidth;
    p[7].x = $offsets[i] + $tabwidths[i] - $cornerwidth;
    p[8].x = $offsets[i] + $tabwidths[i];
    p[9].x = $width;
    p[10].x = $width;
    p[11].x = 0;

    if ($orientation == XfwfUpTabs) {
	p[0].y = $height - shad;
	p[1].y = $height - shad;
	p[2].y = $height - shad - k;
	p[3].y = k;
	p[4].y = 0;
	p[5].y = 0;
	p[6].y = k;
	p[7].y = $height - shad - k;
	p[8].y = $height - shad;
	p[9].y = $height - shad;
	p[10].y = $height;
	p[11].y = $height;
    } else {
	p[0].y = shad;
	p[1].y = shad;
	p[2].y = shad + k;
	p[3].y = $height - k;
	p[4].y = $height;
	p[5].y = $height;
	p[6].y = $height - k;
	p[7].y = shad + k;
	p[8].y = shad;
	p[9].y = shad;
	p[10].y = 0;
	p[11].y = 0;
    }
    *x0 = p[4].x;
    *x1 = p[5].x;
    *midy = (p[1].y + p[4].y)/2;
}

@proc comp_ver_tab_shape($, int i, XPoint p[12], int *y0, int *y1, int *midx)
{
    int shad = $frameWidth;
    int k = min($cornerheight, ($width - shad)/2);
    /*
     *       0 o_o 11  11 o_o 0
     *         | |        | |
     *       1 o |        | o 1
     *     3 2/  |        |  \2 3
     *     o-o   |        |   o-o
     *    /      |        |      \
     * 4 o       |        |       o 4
     *   |       |        |       |
     * 5 o       |        |       o 5
     *    \      |        |      /
     *     o-o   |        |   o-o
     *     6 7\  |        |  /7 6  
     *       8 o |        | o 8
     *         | |        | |
     *       9 o_o 10  10 o_o 9
     */
    if ($orientation == XfwfLeftTabs) {
	p[0].x = $width - shad;
	p[1].x = $width - shad;
	p[2].x = $width - shad - k;
	p[3].x = k;
	p[4].x = 0;
	p[5].x = 0;
	p[6].x = k;
	p[7].x = $width - shad - k;
	p[8].x = $width - shad;
	p[9].x = $width - shad;
	p[10].x = $width;
	p[11].x = $width;
    } else {
	p[0].x = shad;
	p[1].x = shad;
	p[2].x = shad + k;
	p[3].x = $width - k;
	p[4].x = $width;
	p[5].x = $width;
	p[6].x = $width - k;
	p[7].x = shad + k;
	p[8].x = shad;
	p[9].x = shad;
	p[10].x = 0;
	p[11].x = 0;
    }
    p[0].y = 0;
    p[1].y = $offsets[i];
    p[2].y = $offsets[i] + $cornerwidth;
    p[3].y = $offsets[i] + $cornerwidth;
    p[4].y = $offsets[i] + 2 * $cornerwidth;
    p[5].y = $offsets[i] + $tabwidths[i] - 2 * $cornerwidth;
    p[6].y = $offsets[i] + $tabwidths[i] - $cornerwidth;
    p[7].y = $offsets[i] + $tabwidths[i] - $cornerwidth;
    p[8].y = $offsets[i] + $tabwidths[i];
    p[9].y = $height;
    p[10].y = $height;
    p[11].y = 0;
    *y0 = p[4].y;
    *y1 = p[5].y;
    *midx = (p[1].x + p[4].x)/2;
}

@proc draw_border($, XPoint poly[12])
{
    Display *dpy = XtDisplay($);
    Window win = XtWindow($);

    switch ($orientation) {
    case XfwfUpTabs:
	XDrawLines(dpy, win, $topgc, poly, 6, CoordModeOrigin);
	XDrawLines(dpy, win, $bottomgc, poly + 5, 4, CoordModeOrigin);
	XDrawLines(dpy, win, $topgc, poly + 8, 2, CoordModeOrigin);
	break;
    case XfwfLeftTabs:
	XDrawLines(dpy, win, $topgc, poly, 7, CoordModeOrigin);
	XDrawLines(dpy, win, $bottomgc, poly + 6, 2, CoordModeOrigin);
	XDrawLines(dpy, win, $topgc, poly + 7, 3, CoordModeOrigin);
	break;
    case XfwfRightTabs:
    case XfwfDownTabs:
	XDrawLines(dpy, win, $bottomgc, poly, 2, CoordModeOrigin);
	XDrawLines(dpy, win, $topgc, poly + 1, 4, CoordModeOrigin);
	XDrawLines(dpy, win, $bottomgc, poly + 4, 6, CoordModeOrigin);
    }
}

@ The |draw_hor_tab| routine draws a horizontal tab (not the main
one). The |poly| variable holds the corner points of the area occupied
by this tab. The label is centered inside this area.

@proc draw_hor_tab($, Region region, int i)
{
    XPoint p[12];
    Display *dpy = XtDisplay($);
    Window win = XtWindow($);
    Region clip;
    int x0, x1, midy;

    comp_hor_tab_shape($, i, p, &x0, &x1, &midy);
    clip = XPolygonRegion(p, XtNumber(p), WindingRule);
    if (region) XIntersectRegion(clip, region, clip);
    if (XEmptyRegion(clip)) return;

    XSetRegion(dpy, $textgc, clip);
    XSetRegion(dpy, $topgc, clip);
    XSetRegion(dpy, $bottomgc, clip);
    if (i == $lefttabs) {
	XSetRegion(dpy, $backgc, clip);
	XFillPolygon(dpy, win, $backgc,
		     p, XtNumber(p), Convex, CoordModeOrigin);
    } else {
	XSetRegion(dpy, $fillgc, clip);
	XFillPolygon(dpy, win, $fillgc,
		     p, XtNumber(p), Convex, CoordModeOrigin);
    }
    if ($labels) {
	int w, y, x, len = strlen($labels[i]);
	y = midy - ($font->ascent + $font->descent)/2 + $font->ascent;
	w = XTextWidth($font, $labels[i], len);
	if (i == $lefttabs
	    || $tabWidthPercentage <= 100/($lefttabs + $righttabs + 1))
	    x = (x0 + x1 - w)/2;		/* Centered text */
	else if (i < $lefttabs)
	    x = x0 + $textmargin;		/* Left aligned text */
	else
	    x = x1 - $textmargin - w;		/* Right aligned text */
	XDrawString(dpy, win, $textgc, x, y, $labels[i], len);
    }
    draw_border($, p);
    XDestroyRegion(clip);
}

@proc draw_ver_tab($, Region region, int i)
{
    Display *dpy = XtDisplay($);
    Window win = XtWindow($);
    XPoint p[12];
    Region clip;
    int y0, y1, midx;

    comp_ver_tab_shape($, i, p, &y0, &y1, &midx);
    clip = XPolygonRegion(p, XtNumber(p), WindingRule);
    if (region) XIntersectRegion(clip, region, clip);
    if (XEmptyRegion(clip)) return;

    XSetRegion(dpy, $textgc, clip);
    XSetRegion(dpy, $topgc, clip);
    XSetRegion(dpy, $bottomgc, clip);
    if (i == $lefttabs) {
	XSetRegion(dpy, $backgc, clip);
	XFillPolygon(dpy, win, $backgc,
		     p, XtNumber(p), Convex, CoordModeOrigin);
    } else {
	XSetRegion(dpy, $fillgc, clip);
	XFillPolygon(dpy, win, $fillgc,
		     p, XtNumber(p), Convex, CoordModeOrigin);
    }
    if ($labels) {
	int y, align;
	float angle = $orientation == XfwfLeftTabs ? 90.0 : -90.0;
	if (i == $lefttabs
	    || $tabWidthPercentage <= 100/($lefttabs + $righttabs + 1)) {
	    y = (y0 + y1)/2;
	    align = MCENTRE;
	} else if (i < $lefttabs) {
	    y = y0 + $textmargin;
	    align = $orientation == XfwfLeftTabs ? MRIGHT : MLEFT;
	} else {
	    y = y1 - $textmargin;
	    align = $orientation == XfwfLeftTabs ? MLEFT : MRIGHT;
	}
	XRotDrawAlignedString
	    (dpy, $font, angle, win, $textgc, midx, y, $labels[i], align);
    }
    draw_border($, p);
    XDestroyRegion(clip);
}

@ The GCs are not shared, because the clip masks are changed
sometimes.

The current shadows are only drawn with colors, not with pixmaps. This
has to be changed some day. The line width is twice the
|frameWidth|, because half of the line is clipped when the tab
is drawn.

@proc create_topgc($)
{
    XtGCMask mask = GCForeground | GCLineWidth;
    XGCValues values;

    if ($topgc != NULL) XFreeGC(XtDisplay($), $topgc);
    values.foreground = $topShadowColor;
    values.line_width = 2 * $frameWidth;
    $topgc = XCreateGC(XtDisplay($), RootWindowOfScreen(XtScreen($)),
		       mask, &values);
}

@proc create_bottomgc($)
{
    XtGCMask mask = GCForeground | GCLineWidth;
    XGCValues values;

    if ($bottomgc != NULL) XFreeGC(XtDisplay($), $bottomgc);
    values.foreground = $bottomShadowColor;
    values.line_width = 2 * $frameWidth;
    $bottomgc = XCreateGC(XtDisplay($), RootWindowOfScreen(XtScreen($)),
			  mask, &values);
}

@proc create_textgc($)
{
    XtGCMask mask = GCForeground | GCFont;
    XGCValues values;

    if ($textgc != NULL) XFreeGC(XtDisplay($), $textgc);
    values.foreground = $foreground;
    values.font = $font->fid;
    $textgc = XCreateGC(XtDisplay($), RootWindowOfScreen(XtScreen($)),
			mask, &values);
}

@proc create_fillgc($)
{
    XtGCMask mask = GCForeground;
    XGCValues values;

    if ($fillgc != NULL) XFreeGC(XtDisplay($), $fillgc);
    values.foreground = $tabcolor;
    $fillgc = XCreateGC(XtDisplay($), RootWindowOfScreen(XtScreen($)),
			mask, &values);
}

@proc create_backgc($)
{
    XtGCMask mask = GCForeground;
    XGCValues values;

    if ($backgc != NULL) XFreeGC(XtDisplay($), $backgc);
    values.foreground = $background;
    $backgc = XCreateGC(XtDisplay($), RootWindowOfScreen(XtScreen($)),
			mask, &values);
}

@ |copy_bg| copies the |background| color to the |main_color|.

@proc copy_bg($, int offset, XrmValue* value)
{
    value->addr = (XtPointer) &$background;
}

@ |set_shape| is called from |realize| and from |resize|, to set the
shape of the window.

TO DO: shape gaps between tabs.

@proc set_shape($)
{
    int x0, x1, midy, y0, y1, midx, i;
    Region region, clip;
    XPoint poly[12];

    if (! XtIsRealized($)) return;

    region = XCreateRegion();

    switch ($orientation) {
    case XfwfUpTabs:
    case XfwfDownTabs:
	for (i = 0; i <= $lefttabs + $righttabs; i++) {
	    comp_hor_tab_shape($, i, poly, &x0, &x1, &midy);
	    clip = XPolygonRegion(poly, XtNumber(poly), WindingRule);
	    XUnionRegion(region, clip, region);
	    XDestroyRegion(clip);
	}
	break;
    case XfwfLeftTabs:
    case XfwfRightTabs:
	for (i = 0; i <= $lefttabs + $righttabs; i++) {
	    comp_ver_tab_shape($, i, poly, &y0, &y1, &midx);
	    clip = XPolygonRegion(poly, XtNumber(poly), WindingRule);
	    XUnionRegion(region, clip, region);
	    XDestroyRegion(clip);
	}
	break;
    }
    XShapeCombineRegion(XtDisplay($), XtWindow($), ShapeBounding,
			0, 0, region, ShapeSet);
    XDestroyRegion(region);
}

@ The converter |cvtStringToTabsOrientation| converts strings like
`right', `up' and `uptabs' to values of type |TabsOrientation|. Case
is unimportant.

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

@proc Boolean cvtStringToTabsOrientation(Display *display, XrmValuePtr args, Cardinal *num_args, XrmValuePtr from, XrmValuePtr to, XtPointer *converter_data)
{
    TabsOrientation a = XfwfUpTabs;
    char *s = (char*) from->addr;

    if (*num_args != 0)
	XtAppErrorMsg
	    (XtDisplayToApplicationContext(display),
	     "cvtStringToTabsOrientation", "wrongParameters", "XtToolkitError",
	     "String to TabsOrientation conversion needs no arguments",
	     (String*) NULL, (Cardinal*) NULL);

    if (XmuCompareISOLatin1(s, "up") == 0) a = XfwfUpTabs;
    else if (XmuCompareISOLatin1(s, "uptabs") == 0) a |= XfwfUpTabs;
    else if (XmuCompareISOLatin1(s, "down") == 0) a |= XfwfDownTabs;
    else if (XmuCompareISOLatin1(s, "downtabs") == 0) a |= XfwfDownTabs;
    else if (XmuCompareISOLatin1(s, "left") == 0) a |= XfwfLeftTabs;
    else if (XmuCompareISOLatin1(s, "lefttabs") == 0) a |= XfwfLeftTabs;
    else if (XmuCompareISOLatin1(s, "right") == 0) a |= XfwfRightTabs;
    else if (XmuCompareISOLatin1(s, "righttabs") == 0) a |= XfwfRightTabs;
    else XtDisplayStringConversionWarning(display, s, "TabsOrientation");
    done(TabsOrientation, a);
}

@ The converter |cvtTabsOrientationToString| does the reverse: it
convertes values of type |TabsOrientation| to strings.

@proc Boolean cvtTabsOrientationToString(Display *display, XrmValuePtr args, Cardinal *num_args, XrmValuePtr from, XrmValuePtr to, XtPointer *converter_data)
{
    TabsOrientation *a = (TabsOrientation*) from->addr;

    if (*num_args != 0)
	XtAppErrorMsg
	    (XtDisplayToApplicationContext(display),
	     "cvtTabsOrientationToString", "wrongParameters", "XtToolkitError",
	     "TabsOrientation to String conversion needs no arguments",
	     (String*) NULL, (Cardinal*) NULL);
    switch (*a) {
    case XfwfUpTabs: done(String, "up");
    case XfwfDownTabs: done(String, "down");
    case XfwfLeftTabs: done(String, "left");
    case XfwfRightTabs: done(String, "right");
    }
    XtAppErrorMsg
	(XtDisplayToApplicationContext(display),
	 "cvtTabsOrientationToString", "illParameters", "XtToolkitError",
	     "TabsOrientation to String conversion got illegal argument",
	     (String*) NULL, (Cardinal*) NULL);
    return TRUE;
}

@TRANSLATIONS

@trans <Btn1Down>,<Btn1Up>: activate()

@ACTIONS

@proc activate
{
    int x0, x1, dummy, i, x, y;
    XPoint poly[12];

    switch ($orientation) {
    case XfwfUpTabs:
    case XfwfDownTabs:
	x = event->xbutton.x;
	comp_hor_tab_shape($, $lefttabs, poly, &x0, &x1, &dummy);
	if (x0 <= x && x < x1) {
	    XtCallCallbackList($, $activate, (XtPointer) 0);
	    return;
	}
	for (i = -1; i >= -$lefttabs; i--) {
	    comp_hor_tab_shape($, i + $lefttabs, poly, &x0, &x1, &dummy);
	    if (x0 <= x && x < x1) {
		XtCallCallbackList($, $activate, (XtPointer) i);
		return;
	    }
	}
	for (i = 1; i <= $righttabs; i++) {
	    comp_hor_tab_shape($, i + $lefttabs, poly, &x0, &x1, &dummy);
	    if (x0 <= x && x < x1) {
		XtCallCallbackList($, $activate, (XtPointer) i);
		return;
	    }
	}
	break;
    case XfwfLeftTabs:
    case XfwfRightTabs:
	y = event->xbutton.y;
	comp_ver_tab_shape($, $lefttabs, poly, &x0, &x1, &dummy);
	if (x0 <= y && y < x1) {
	    XtCallCallbackList($, $activate, (XtPointer) 0);
	    return;
	}
	for (i = -1; i >= -$lefttabs; i--) {
	    comp_ver_tab_shape($, i + $lefttabs, poly, &x0, &x1, &dummy);
	    if (x0 <= y && y < x1) {
		XtCallCallbackList($, $activate, (XtPointer) i);
		return;
	    }
	}
	for (i = 1; i <= $righttabs; i++) {
	    comp_ver_tab_shape($, i + $lefttabs, poly, &x0, &x1, &dummy);
	    if (x0 <= y && y < x1) {
		XtCallCallbackList($, $activate, (XtPointer) i);
		return;
	    }
	}
	break;
    }
}

@IMPORTS

@incl <stdio.h>
@incl <X11/extensions/shape.h>
@incl <Xfwf/Converters.h>
@incl <X11/Xmu/CharSet.h>
@incl "rotated.h"
