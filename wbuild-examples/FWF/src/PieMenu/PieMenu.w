# XfwfPieMenu widget
# Author: Bert Bos
# Copyright: see README file
# Version: 1.2 (Nov 1994)

@class XfwfPieMenu (OverrideShell)  @file = PieMenu

@ A PieMenu is normally used as a pop-up menu. It is a circular menu
with labeled pie segements. The menu pops up with the mouse pointer in
the center. The user moves the mouse into one of the segments and
releases the mouse button. The advantage of this kind of menu is that
-- after some practice -- it can be used without having to look at the
screen. The disadvantage is that there is only room for a few items.
This implementation works with up to 6 items.

@public

@ The size of the menu is determined with |width| and |height|. If
they are unequal, the widget tries to set the smallest equal to the
largest, to arrive at a circle. In the middle of the circle is a
`hole' (an inactive area), the size of which is set with |holeRadius|.
When the |width| is less than the diameter of the hole plus 10, the
width is automatically increased.

	@var Dimension holeRadius = 8

@ The labels and the corresponding actions are contained in the |menu|
String. The menu looks like this (example):

	|Open...    -> open()\n\|

	|Save       -> save()\n\|

	|Options    -> set(options)\n\|

	|Exit       -> quit(0)|

Each newline-seperated item has a label and an action. NB. the current
implementation only supports up to six items, up to 6 actions per item
and up to 6 arguments per action.

	@var String menu = NULL

@ The background of the pie segments can be set individually for each
of the segments. If some are left unspecified, the color of core
resource |background| is used. Note that there is currently no support
for stippled backgrounds or background pixmaps.

	@var Pixel background1 = <CallProc> copy_bg
	@var Pixel background2 = <CallProc> copy_bg
	@var Pixel background3 = <CallProc> copy_bg
	@var Pixel background4 = <CallProc> copy_bg
	@var Pixel background5 = <CallProc> copy_bg
	@var Pixel background6 = <CallProc> copy_bg

@ The foreground colors are set similarly. The resource |foreground|
is used for the lines between the segments and the central `hole'.
When the other colors are left unspecified, the |foreground| color is
used.

	@var Pixel foreground = <String> XtDefaultForeground
	@var Pixel foreground1 = <CallProc> copy_fg
	@var Pixel foreground2 = <CallProc> copy_fg
	@var Pixel foreground3 = <CallProc> copy_fg
	@var Pixel foreground4 = <CallProc> copy_fg
	@var Pixel foreground5 = <CallProc> copy_fg
	@var Pixel foreground6 = <CallProc> copy_fg

@ For the fonts the same technique is used, although in this case
|font| is purely used as the default for the other fonts, since
nothing is drawnd directly with it.

	@var <FontStruct> XFontStruct *font = <String> XtDefaultFont
	@var <FontStruct> XFontStruct *font1 = <CallProc> copy_font
	@var <FontStruct> XFontStruct *font2 = <CallProc> copy_font
	@var <FontStruct> XFontStruct *font3 = <CallProc> copy_font
	@var <FontStruct> XFontStruct *font4 = <CallProc> copy_font
	@var <FontStruct> XFontStruct *font5 = <CallProc> copy_font
	@var <FontStruct> XFontStruct *font6 = <CallProc> copy_font

@ The widget can be given a 3D shadow with the |frameWidth| resource.
Note that there is currently no way to influence the colors chosen for
the shadow.

	@var Dimension frameWidth = 4

@ Because the widget has its own decoration, the border width is by
default set to 0.

	@var border_width = 0

@private

@ The |menu| string is converted to an array of items and stored in
the private variable |items|. Currently there is a hardcoded limit of
6 items, with up to 6 actions, each with up to 6 parameters. The
strings in this array are pointers into the string variable
|menu_copy|.

	@def MAXITEMS = 6
	@def MAXACTIONS = 6
	@def MAXPARAMS = 6

	@type LabelAction = struct {
	    char *label;
	    int nactions;
	    char *action[MAXACTIONS];
	    int num_params[MAXACTIONS];
	    char *params[MAXACTIONS][MAXPARAMS];
	}

	@var String menu_copy
	@var LabelAction items[MAXACTIONS]

@ The GCs for each of the segments

	@var GC gc[MAXITEMS]

@ The GC for drawing the lines and the `hole'.

	@var GC linegc

@ The GC for drawing the light parts of the shadow frame:

	@var GC lightgc

@ The GC for drawing the dark parts of the shadow frame:

	@var GC darkgc

@ The number of items in the menu

	@var int nitems

@ The empty stipple that is used to fill the segments with.

	@var Pixmap emptystip

@ A 50 percent stipple that is used for the shadows if there are
insufficient colors.

	@var Pixmap stip4

@methods

@ The |initialize| method tries to set the |width| and |height| to
equal values and checks if the |width| is larger than the central
hole. It creates six GCs, one for each segment. The |menu| is
translated and stored in the |items| array with the help of the
|parse_menu| function.

@proc initialize
{
    int i;

    if ($width < $holeRadius + 10)
	$width = $holeRadius + 10;
    if ($height < $width)
	$height = $width;
    else if ($width < $height)
	$width = $height;

    $emptystip = XCreateBitmapFromData
	(XtDisplay($), RootWindowOfScreen(XtScreen($)), emptybm_bits,
	 emptybm_width, emptybm_height);
    $stip4 = XCreateBitmapFromData
	(XtDisplay($), RootWindowOfScreen(XtScreen($)), stip4_bits,
	 stip4_width, stip4_height);

    $lightgc = NULL; create_lightgc($);
    $darkgc = NULL; create_darkgc($);
    $linegc = NULL; create_linegc($);

    $gc[0] = NULL; create_gc($, &$gc[0], $font1, $background1, $foreground1);
    $gc[1] = NULL; create_gc($, &$gc[1], $font2, $background2, $foreground2);
    $gc[2] = NULL; create_gc($, &$gc[2], $font3, $background3, $foreground3);
    $gc[3] = NULL; create_gc($, &$gc[3], $font4, $background4, $foreground4);
    $gc[4] = NULL; create_gc($, &$gc[4], $font5, $background5, $foreground5);
    $gc[5] = NULL; create_gc($, &$gc[5], $font6, $background6, $foreground6);

    $menu = XtNewString($menu);
    $menu_copy = XtNewString($menu);
    parse_menu($menu_copy, $items, &$nitems);
}

@ The |realize| methods first calls the |realize| method of the
superclass to create a window and then uses |XmuReshapeWidget| to
change the shape of the widget to a circle.

@proc realize
{
    #realize($, mask, attributes);
    (void) XmuReshapeWidget($, XmuShapeEllipse, -1, -1);
}

@ The |expose| methods draws each of the pie segments. It counts the
number of items and then draws each of the segments.  |angle| is the
angle of each of the segments in 64th of a degree. |angle1| is the
starting angle of the segment. Segments are drawn starting at 45
degrees and drawn counterclockwise. 0 Degrees is 3 o'clock.
The position of the label is in the middle of its segment.

@proc expose
{
    int i, x, y, len, angle, angle1;
    XFontStruct *fn;
    double a;

    if (! XtIsRealized($)) return;

    /*
     * Draw the segments and the labels
     */
    angle = 360 * 64/$nitems;
    for (i = 0, angle1 = 45 * 64; i < $nitems; i++, angle1 += angle) {
	XFillArc(XtDisplay($), XtWindow($), $gc[i], 0, 0, $width, $height,
		 angle1, angle);
	fn = (i == 0 ? $font1
	      : (i == 1 ? $font2
		 : (i == 2 ? $font3
		    : (i == 3 ? $font4
		       : (i == 4 ? $font5
			  : $font6)))));
	len = strlen($items[i].label);
	a = (angle1 + angle/2) * 2 * M_PI/360/64;
	x = $width/2 - XTextWidth(fn, $items[i].label, len)/2
	    + (int) (cos(a) * $width * 0.3);
	y = $height/2 - (fn->ascent + fn->descent)/2 + fn->ascent
	    - (int) (sin(a) * $height * 0.3);
	XDrawImageString(XtDisplay($), XtWindow($), $gc[i], x, y,
		    $items[i].label, len);
    }
    /*
     * Draw central `hole'
     */
    XFillArc(XtDisplay($), XtWindow($), $linegc, $width/2 - $holeRadius,
	     $height/2 - $holeRadius, 2 * $holeRadius, 2 * $holeRadius,
	     0, 360 * 64);

    /*
     * Draw lines separating the segments
     */
    for (i = 0, angle1 = 45 * 64; i < $nitems; i++, angle1 += angle) {
	a = angle1 * 2 * M_PI/360/64;
	x = (int) (cos(a) * $width/2) + $width/2;
	y = $height/2 - (int) (sin(a) * $height/2);
	XDrawLine(XtDisplay($), XtWindow($), $linegc, $width/2, $height/2,
		  x, y);
    }
    /*
     * Draw shadow border
     */
    XDrawArc(XtDisplay($), XtWindow($), $lightgc, 0, 0, $width, $height,
	     45 * 64, 180 * 64);
    XDrawArc(XtDisplay($), XtWindow($), $darkgc, 0, 0, $width, $height,
	     225 * 64, 180 * 64);
}

@ There should be a |set_values| method here...

@ The |destroy| method frees memory allocated by the widget.

@proc destroy
{
    int i;

    for (i = 0; i < MAXITEMS; i++)
	XtReleaseGC($, $gc[i]);
    XFreePixmap(XtDisplay($), $emptystip);
    XtFree($menu_copy);
    XtFree($menu);
}

@translations

@ The only translation is for a mouse button release. The mouse press
would have popped up the widget. The release calls the action
associated with the segment and then pops the widget down again.

@trans <BtnUp>: select_item() XtMenuPopdown()

@actions

@ The |select_item| action is normally called when the mouse button is
released. It checks where the mouse is released and if it is inside
the widget and not over the central hole, it calls the action function
that is associated with that segment of the menu.

The |action| string is not yet parsed. It is copied to a temporary
string and split into action name and parameter list. The
implementation currently assumes that there are no more than 100
arguments and that the length of the original |action| string does not
exceed 500 characters.

@proc select_item
{
    int n, i, dx, dy, dist, nparm;
    double angle;

    dx = event->xbutton.x - $width/2;
    dy = $height/2 - event->xbutton.y;
    dist = dx * dx + dy * dy;
    if (dist > $width * $width/4) return;	/* Outside */
    if (dist <= $holeRadius * $holeRadius) return; /* In hole */
    angle = atan2((double) dy, (double) dx) - M_PI/4;
    if (angle < 0.0) angle += 2 * M_PI;
    n = (int) (angle / (2 * M_PI/$nitems));
    for (i = 0; i < $items[n].nactions; i++)
	XtCallActionProc($, $items[n].action[i], event, $items[n].params[i],
			 $items[n].num_params[i]);
}

@utilities

@ The resource default proc |copy_bg| copies the value of the
|background_pixel| resource to the specified resource (|background2|
to |background6|).

@proc copy_bg($, int offset, XrmValue *value)
{
    value->addr = (XtPointer) &$background_pixel;
}

@ The |copy_fg| function is similar, but it copies the |foreground|
resource.

@proc copy_fg($, int offset, XrmValue *value)
{
    value->addr = (XtPointer) &$foreground;
}

@ The |copy_font| function is similar, but it copies the |font|
resource.

@proc copy_font($, int offset, XrmValue *value)
{
    value->addr = (XtPointer) &$font;
}

@ The empty bitmap that is used a a stipple is defined here. A Pixmap
is created from it in the |initialize| method. The stip4 bitmap is a
50 percent bitmaps used for the shadows when there are insufficient
colors.

	@def emptybm_width = 1
	@def emptybm_height = 1
	@var char emptybm_bits[] = { 0x00 }

	@def stip4_width = 2
	@def stip4_height = 2
	@var char stip4_bits[] = { 0x01, 0x02 }

@ Create a new GC for one of the segments, release the previous one.
The fill style is set to |FillOpaqueStippled| with a stipple without
any foreground. This is done so that the same GC can be used both to
draw the text with the foreground color and to fill the segment with
the background color.

@proc create_gc($, GC *gc, XFontStruct *fn, Pixel bg, Pixel fg)
{
    XGCValues values;

    if (*gc != NULL) XtReleaseGC($, *gc);
    values.font = fn->fid;
    values.background = bg;
    values.foreground = fg;
    values.fill_style = FillOpaqueStippled;
    values.stipple = $emptystip;
    *gc = XtGetGC($, GCFont | GCBackground | GCForeground | GCFillStyle
		  | GCStipple, &values);
}

@ The |linegc| private variable holds the GC that is used for drawing
the lines between the segments and the central `hole'.

@proc create_linegc($)
{
    XGCValues values;

    if ($linegc != NULL) XtReleaseGC($, $linegc);
    values.background = $background_pixel;
    values.foreground = $foreground;
    values.line_width = 2;
    $linegc = XtGetGC($, GCBackground | GCForeground | GCLineWidth, &values);
}

@ The |create_darkgc| function creates the GC for the dark parts of
the shadow frame.

@proc create_darkgc($)
{
    XtGCMask mask;
    XGCValues values;

    if ($darkgc != NULL) XtReleaseGC($, $darkgc);
    if (DefaultDepthOfScreen(XtScreen($)) > 4
	&& choose_color($, 0.5, $background_pixel, &values.foreground)) {
	mask = GCForeground;
    } else {
	mask = GCFillStyle | GCBackground | GCForeground | GCStipple;
	values.fill_style = FillOpaqueStippled;
	values.background = $background_pixel;
	values.foreground = WhitePixelOfScreen(XtScreen($));
	values.stipple = $stip4;
    }
    mask |= GCLineWidth;
    values.line_width = $frameWidth;
    $darkgc = XtGetGC($, mask, &values);
}

@ |create_lightgc| does the same for the light parts of the shadow frame.

@proc create_lightgc($)
{
    XtGCMask mask;
    XGCValues values;

    if ($lightgc != NULL) XtReleaseGC($, $lightgc);
    if (DefaultDepthOfScreen(XtScreen($)) > 4
	&& choose_color($, 1.35, $background_pixel, &values.foreground)) {
	mask = GCForeground;
    } else {
	mask = GCFillStyle | GCBackground | GCForeground | GCStipple;
	values.fill_style = FillOpaqueStippled;
	values.background = $background_pixel;
	values.foreground = WhitePixelOfScreen(XtScreen($));
	values.stipple = $stip4;
    }
    mask |= GCLineWidth;
    values.line_width = $frameWidth;
    $lightgc = XtGetGC($, mask, &values);
}

@ The function |trim| is called by the type converter to remove
leading and trailing blanks from a string.

@proc Boolean trim(String s)
{
    int i, j;

    for (i = strlen(s); i > 0 && isspace(s[i-1]); i--) ;
    s[i] = '\0';
    for (i = 0; isspace(s[i]); i++) ;
    if (i != 0) {
	for (j = 0; s[i]; i++, j++) s[j] = s[i];
	s[j] = '\0';
    }
    return s[0] != '\0';
}

@ The function |parse_menu| splits the string |menu| into items by
inserting NUL bytes and then splits each of the items into a label
part and an action part. The action is further subdivided into a list
of action functions, and each action function can have a list of
arguments.

When the function is done, the contents of the string |menu| have been
changed.

In the algorithm below, |menu| = start of current item; |p| = start of
the next item (i.e., immediately after the next newline); |q| = start
of current action; |r| = start of next action; |s| = start of current
parameter; |t| = start of next parameter.

@proc parse_menu(String menu, LabelAction *items, int *nitems)
{
    int n, act, parm;
    String p, q, r, s, t;

    if (! menu) {
	*nitems = 0;
	return;
    }

    /* Loop over all items */

    for (n = 0; menu && n < MAXITEMS; n++) {
	if ((p = strchr(menu, '\n'))) *(p++) = '\0'; /* Mark end of item */
	if ((q = strstr(menu, "->"))) *q = '\0', q += 2; /* Start of actions */

	/* Loop over all actions for this item */

	for (act = 0; q && act < MAXACTIONS; act++) {
	    if ((r = strchr(q, ')'))) *(r++) = '\0'; /* End of action */
	    if ((s = strchr(q, '('))) *(s++) = '\0'; /* Start of params */

	    /* Loop over all parameters for this action */

	    for (parm = 0; s && parm < MAXPARAMS; parm++) {
		if ((t = strchr(s, ','))) *(t++) = '\0'; /* End of param */
		if (! trim(s)) parm--;		/* Remove empty param */
		else items[n].params[act][parm] = s;
		s = t;
	    }
	    items[n].num_params[act] = parm;

	    if (! trim(q)) act--;		/* Remove empty action */
	    else items[n].action[act] = q;
	    q = r;				/* Start next action loop */
	}
	items[n].nactions = act;

	if (! trim(menu)) n--;			/* Remove empty item */
	else items[n].label = menu;
	menu = p;				/* Start next item loop */
    }
    *nitems = n;
}

@imports

@incl <stdio.h>
@incl <ctype.h>
@incl <string.h>
@incl <math.h>
@incl <X11/Xmu/Converters.h>
