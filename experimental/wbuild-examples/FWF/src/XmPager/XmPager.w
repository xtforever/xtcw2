# XmPager.w -- Pager widget class (Motif version)
# Author: Bert Bos <bert@let.rug.nl>
# Copyright: see README file
# 13 Feb 1995
# Version: 1.0

@class XmPager (XmManager)  @file=XmPager

@ The Pager widget displays one page from a long text. A dog's ear in
the corner allows flipping pages. This is an alternative to scrolled
texts, meant for material that is preferrably read a page at a time.

The Pager can operate in two modes, depending on the setting of the
|lines| resource. If |lines| is 0, as many lines are displayed as will
fit in the widget. If |lines| is positive, that many lines will be
displayed. In the latter case, the widget will also try to scale the
font (provided the font is scalable).

@PUBLIC

@ The text that is displayed can either be given directly as a
resource, or it can be input from a file or a pipe. The text can
include newlines. Form-feed characters can be used to force the start
of a new page.

If the text starts with an atsign (@@) the rest of the text is
interpreted as a file name. The file will only be read only and the
contents are held in memory. The file is searched in the current
directory first and, if that fails, with a call to
|XtResolvePathname()|.

If the text starts with an inverted quote (`), the rest of the text is
considered a command that will be passed to |popen()|. The output of
the command is stored in memory and used as the text to display.

	@var String text = NULL

@ The |fontFamily| is used to set a generic name for the font, instead
of a specific font. If |fontFamily| is set, but |roman| is left
unspecified, the widget will try to find a font of the appropriate
size itself.

	@var String fontFamily = NULL

@ The font that is used for the text can be inferred from the
|fontFamily| resource or it can be set explicitly.

Note that, if the font is scalable and the |lines| resources is
positive, the actual point size used will depend on the size of the
widget.

	@var <FontStruct> XFontStruct *roman = <CallProc> guess_roman

@ The |lines| resource can be set to a positive value, to force the
widget to display a fixed number of lines, whatever its size. This can
give strange results when the font is not scalable. If |lines <= 0|,
as many lines are displayed as will fit.

	@var int lines = 0

@ Someday, the widget might support another mode of display, namely
wrapping text. In this mode, lines are broken between words if they
don't fit on one line. Newlines are ignored, unless there are two in a
row.

	@var Boolean wrap = False

@ The interline spacing depends on the font. The |baseline| resource
specifies how much the normal baseline is stretched (or shrunk). A
value of 1 corresponds to a baseline equal to the font's ascent plus
the font's descent.

Note that setting |lines| to a positive value without also using a
scalable font, will mess up the baselines, because the widget will
still try to draw the requested number of lines.

	@var float baseline = <String> "1.2"

@ The |margin| resource gives the number of pixels that must be left
blank above and to the left of the text.

        @var int margin = 5

@ A tablist can be provided for tabbing to particular columns
within the label.

	@var String tablist =
	"8 16 24 32 40 48 56 64 72 80 88 96 104 112 120 128 136 144 152 160"



@PRIVATE

@ The text to display may have been passed in the |text| resource, or
it may have been read from file. In the latter case the |text| resource
will hold the file name instead of the text. The private variable
|fulltext| will hold the real text, whether stored in the |text|
resource or read from file.

	@var String fulltext

@ |page| is a list of indexes into |fulltext| that points to the start
of each page. It is updated whenever the size of a page changes.
|page[0]| is unused, since the first page will be numbered `1'.  The
end of the text is marked with |page[i] == -1|

	@var int * page

@ The current page number is held in |pageno|. Asnoted above, pages
are numbered from 1.

	@var int pageno

@ The dog's ear in the lower right corner actually consists of two
icons.

	@var Widget flip_back
	@var Widget flip_forward

@ The full name of the font is stored in a private variable, so
it can be changed by the |scale_font| function, in search of scaled
versions of the font.

        @var String roman_name

@ The GC is used to draw the text.

	@var GC romangc

@ The baseline distance in pixels is computed from the font's height
times the value of |baseline|.

	@var int baselineskip

@ The two icons that make up the dog's ear.

	@var Icon f_back
	@var Icon f_forward

@ The tablist is converted from string format to a list of int's for speed.

	@var int *tabs


@CLASSVARS

@ Set a few class variables.

	@var compress_motion = True
	@var compress_exposure = XtExposeCompressMultiple
	@var compress_enterleave = True


@METHODS

@ The |initialize| method uses a utility function |init_text| to set
the private variables. The same function is also used by |set_values|.
Another utility function is used to try to change the font size when
|lines| is set to a positive value.

The dog's ear is implemented as a pair of icons. Callbacks will be
attached to the icons.

The event translations for the icons would normally include an action
for the Return key, but in this case the icons are made to react only
to button events. Key events are passed on to the Pager itself.

@proc initialize
{
    int status;
    static char trans[] = "<Btn1Down>,<Btn1Up>: activate()";

    $tabs = XfwfTablist2Tabs($tablist);

    $f_back.attributes.valuemask = XpmSize;
    status = XpmCreatePixmapFromData
	(XtDisplay($), DefaultRootWindow(XtDisplay($)), flip_back,
	 &$f_back.pixmap, &$f_back.mask, &$f_back.attributes);
    switch (status) {
    case XpmNoMemory: XtAppError(XtWidgetToApplicationContext($),
				 "Out of memory");
    case XpmColorFailed: XtAppError(XtWidgetToApplicationContext($),
				    "Failed to allocate color for pixmap");
    case XpmColorError: XtAppWarning(XtWidgetToApplicationContext($),
				     "Not all pixmap colors found");
    default: ; /* skip */
    }
    $flip_back = XtVaCreateManagedWidget
	("_flip_back", xmIconWidgetClass, $,
	 XtNimage, &$f_back,
	 XmNhighlightThickness, 0, XmNtraversalOn, False,
	 XtVaTypedArg, XtNtranslations, XtRString, trans,
	 strlen(trans)+1, NULL);
    XtMoveWidget($flip_back, $width - $f_back.attributes.width,
		 $height - $f_back.attributes.height);
    XtAddCallback($flip_back, XtNactivate, backwards_CB, $);

    $f_forward.attributes.valuemask = XpmSize;
    status = XpmCreatePixmapFromData
	(XtDisplay($), DefaultRootWindow(XtDisplay($)), flip_forward,
	 &$f_forward.pixmap, &$f_forward.mask, &$f_forward.attributes);
    switch (status) {
    case XpmNoMemory: XtAppError(XtWidgetToApplicationContext($),
				 "Out of memory");
    case XpmColorFailed: XtAppError(XtWidgetToApplicationContext($),
				    "Failed to allocate color for pixmap");
    case XpmColorError: XtAppWarning(XtWidgetToApplicationContext($),
				     "Not all pixmap colors found");
    default: ; /* skip */
    }
    $flip_forward = XtVaCreateManagedWidget
	("_flip_forward", xmIconWidgetClass, $,
	 XtNimage, &$f_forward,
	 XmNhighlightThickness, 0, XmNtraversalOn, False,
	 XtVaTypedArg, XtNtranslations, XtRString, trans,
	 strlen(trans)+1, NULL);
    XtMoveWidget($flip_forward, $width - $f_forward.attributes.width,
		 $height - $f_forward.attributes.height);
    XtAddCallback($flip_forward, XtNactivate, forward_CB, $);

    create_romangc($);
    $baselineskip = 0.5 + $baseline * ($roman->ascent + $roman->descent);
    $text = XtNewString($text);
    $fontFamily = XtNewString($fontFamily);
    $tablist = XtNewString($tablist);
    $fulltext = NULL;
    $page = NULL;
    init_text($);
    split_text($);
    if ($lines > 0) scale_font($);
}


@ The |set_values| method calls |init_text| for most changes. After
that, the widget will have to be drawn again.

All string resources are copied to newly allocated space on the heap,
so the caller can use local variables as arguments to |XtVaSetValues|.

@proc set_values
{
    Boolean need_redisplay = False;

    if ($tablist != $old$tablist) {
	XtFree($old$tablist);
	XtFree((String) $old$tabs);
	$tablist = XtNewString($tablist);
	$tabs = XfwfTablist2Tabs($tablist);
	need_redisplay = True;
    }
    if ($old$fontFamily != $fontFamily) {
	XtFree($old$fontFamily);
	$fontFamily = XtNewString($fontFamily);
	XFreeFont(XtDisplay($), $roman);
	(void) infer_roman($);
    }
    if ($old$text != $text) {
	XtFree($old$text);
	$text = XtNewString($text);
	init_text($);
	split_text($);
	need_redisplay = True;
    }
    if ($old$roman != $roman
	|| $old$lines != $lines
	|| $old$wrap != $wrap
	|| $old$baseline != $baseline
	|| ($old$width != $width && $lines == 0)
	|| ($old$height != $height && $lines == 0)) {
	split_text($);
	need_redisplay = True;
    }
    if ($old$height != $height && $lines > 0) {
	scale_font($);
	need_redisplay = True;
    }
    return need_redisplay;
}


@ When the widget changes size, the font may change, in the case that
|lines| is positive.

@proc resize
{
    if (#resize) #resize($);
    if ($lines > 0) scale_font($); else split_text($);
    XtMoveWidget($flip_back, $width - $f_back.attributes.width,
		 $height - $f_back.attributes.height);
    XtMoveWidget($flip_forward, $width - $f_forward.attributes.width,
		 $height - $f_forward.attributes.height);
    if (XtIsRealized($))
	XClearArea(XtDisplay($), XtWindow($), 0, 0, 0, 0, TRUE);
}


@ Resizing the window results in a change of font, so the old contents
of the window should always be discarded.

(This doesn't seem to work...)

@proc realize
{
    *mask |= CWBitGravity;
    attributes->bit_gravity = ForgetGravity;
    #realize($, mask, attributes);
}

@ The |expose| method draws the text of the current page into the widget.
It can operate in two modes, depending on the setting of the |wrap|
resource. If |wrap| is |False|, the lines are broken only at newline
characters, even if that would mean that a part of the line disappears
off the right edge of the window. On the other hand, if |wrap| is |True|,
single newline characters are treated as spaces and only double newlines
will force a line break. In this mode, the function will try to find line
breaks at spaces in the text.

The |expose| method also adds the current page number at the bottom of
the window.

Note that the |expose| method does not have to look at the |lines|
resource to limit the number of lines on the page, since presumably the
|baseline| and the font size have already been set to such values that
exactly the right amount of lines will fit on the page. Drawing text will
stop at 50 pixels from the bottom, to leave room for the dog's ear and
the page number.
 
@proc expose
{
    Region reg;
    XRectangle rect;
    Position y;
    char *p, *q, h[20];
    Display *dpy = XtDisplay($);
    Window win = XtWindow($);

    if (! XtIsRealized($)) return;
    if (! $fulltext) {
	if (#expose) #expose($, event, region);
	return;
    }
    rect.x = $shadow_thickness;
    rect.y = $shadow_thickness;
    rect.width = $width - 2 * rect.x;
    rect.height = $height - 2 * rect.y;
    reg = XCreateRegion();
    XUnionRectWithRegion(&rect, reg, reg);
    if (region) XIntersectRegion(reg, region, reg);
    XSetRegion(dpy, $romangc, reg);
    y = $margin + $baselineskip - $roman->descent;
    if ($wrap) {
        /* wrapping mode not implemented yet */
    } else {
        p = $fulltext + $page[$pageno];
        for (;;) {
            if (y + $roman->descent >= rect.y + rect.height - 50) break;
            for (q = p; *q != '\0' && *q != '\n' && *q != '\f'; q++) ;
            XfwfDrawImageString(dpy, win, $romangc, rect.x+$margin, y,
				p, q - p, $tabs);
            if (*q == '\0' || *q == '\f') break;
            y += $baselineskip;
            p = q + 1;
        }
    }
    /* Draw page number */
    sprintf(h, "%d", $pageno);
    XfwfDrawImageString(dpy, win, $romangc, rect.x + rect.width/2, rect.y +
		     rect.height - $roman->descent, h, strlen(h), $tabs);

    XSetClipMask(dpy, $romangc, None);
    if (#expose) #expose($, event, region);
}

@proc query_geometry
{
    return XtGeometryYes;
}


@TRANSLATIONS

@trans <Key>Prior: previous_page()
@trans <Key>BackSpace: previous_page()
@trans <Key>minus: previous_page()
@trans <Key>Next: next_page()
@trans <Key>space: next_page()
@trans <Key>plus: next_page()


@ACTIONS

@proc previous_page
{
    backwards_CB($, $, NULL);
}

@proc next_page
{
    forward_CB($, $, NULL);
}


@UTILITIES

@ |read_from_file| opens a file and reads its contents into
|$fulltext|, which is dynamically allocated.

@def INCREMENT = 2048
@def mrealloc(ptr, size) = (void*) XtRealloc((char*) ptr, size)

@proc read_from_file($, String filename)
{
    int fd, len = 0, nbytes;

    if ((fd = open(filename, O_RDONLY)) < 0) {
	perror(filename);
	return;
    }
    do {
	$fulltext = mrealloc($fulltext, len + INCREMENT + 1);
	nbytes = read(fd, $fulltext + len, INCREMENT);
	if (nbytes < 0) perror(filename); else len += nbytes;
    } while (nbytes > 0);
    $fulltext[len] = '\0';
    (void) close(fd);
}


@ |read_from_pipe| works almost like |read_from_file|, except that it
read from a pipe instead of a file.

@proc read_from_pipe($, String cmd)
{
    int len = 0, nbytes;
    FILE *stream;

    if ((stream = popen(cmd, "r")) == NULL) {
	perror(cmd);
	return;
    }
    do {
	$fulltext = mrealloc($fulltext, sizeof(char) * (len + INCREMENT + 1));
	nbytes = fread($fulltext + len, sizeof(char), INCREMENT, stream);
	if (nbytes < 0) perror(cmd); else len += nbytes;
    } while (nbytes > 0);
    $fulltext[len] = '\0';
    (void) pclose(stream);
}


@ |init_text| initializes the private variables after a change to one
of the resources. It starts by freeing previously allocated memory.
Then it initializes |fulltext|.

@proc init_text($)
{
    XtFree($fulltext); $fulltext = NULL;
    XtFree((char*) $page); $page = NULL;
    if ($text == NULL) return;
    /*
     * Initialize $fulltext
     */
    switch ($text[0]) {
    case '@': read_from_file($, $text + 1); break;
    case '`': read_from_pipe($, $text + 1); break;
    default: $fulltext = XtNewString($text);
    }
}


@ |split_text| splits |fulltext| into pages, by setting the indices of
each page in |page|. The way in which that is done, depends on the
setting of the |wrap| and |lines| resources. Currently, wrapping is
not supported.

@proc split_text($)
{
    int i, j, n, h;
    Position x, y;
    Dimension wd, ht;
    char prev = '\0';

    $page = mrealloc($page, 3 * sizeof($page[0]));
    $page[1] = 0;
    j = 2;
    /* Wrapping is not implemented yet */
    if ($lines > 0 && $fulltext != NULL) {
	/* Count lines until n == $lines */
	n = 0;
	for (i = 0; $fulltext[i]; prev = $fulltext[i], i++) {
	    if (prev == '\n') {
		n++;
		if (n == $lines) {
		    $page = mrealloc($page, (j+2) * sizeof($page[0]));
		    $page[j++] = i;
		    n = 0;
		}
	    }
	}
    } else if ($fulltext != NULL) {
	/* Determine page length with actual height of the text */
	x = $shadow_thickness; y = $shadow_thickness;
	wd = $width - 2 * x; ht = $height - 2 * y;
	h = y + $margin + $baselineskip;
	ht = y + ht - 50;
	for (i = 0; $fulltext[i]; prev = $fulltext[i], i++) {
	    if (prev == '\n') {
		h += $baselineskip;
		if (h + $roman->descent >= ht) {
		    $page[j++] = i;
		    h = y + $margin + $baselineskip;
		}
	    }
	}
    }
    $page[j] = -1;
    $pageno = 1;
}

@ When the widget's size changes while |lines| is set, or when |lines|
itself changes, the widget tries to shrink or enlarge the fonts to still
be able to display the same number of lines. If the fonts cannot change
(because they are not scalable), the |baseline| will be changed instead.
This may produce strange results, even to the extent of overlapping lines
of text.

@def F_FOUNDRY = 1
@def F_FAMILY = 2
@def F_WEIGHT = 3
@def F_SLANT = 4
@def F_SET_WIDTH = 5
@def F_SANS = 6
@def F_PIXELS = 7
@def F_POINTS = 8
@def F_HRESOLUTION = 9
@def F_VRESOLUTION = 10
@def F_SPACING = 11
@def F_AVG_WITH = 12
@def F_CHARSET = 13

@proc scale_font($)
{
    Position x, y;
    Dimension w, h;
    int pixels;
    char s[250];
    XFontStruct *fs;
    String info[15];

    x = $shadow_thickness; y = $shadow_thickness;
    w = $width - 2 * x; h = $height - 2 * y;
    $baselineskip = (h - 50 - $margin) / $lines;
    if ($roman_name == NULL			/* Font name is unknown */
	|| ! parse_font_name($roman_name, info)
	|| ! is_scalable_font($roman_name)) {
	$baseline = $baselineskip / (float) ($roman->ascent + $roman->descent);
        return;
    }

    pixels = $baselineskip/$baseline;

    (void) sprintf(s, "-%s-%s-%s-%s-%s-%s-%d-*-%s-%s-%s-*-%s",
		   info[F_FOUNDRY], info[F_FAMILY], info[F_WEIGHT],
		   info[F_SLANT], info[F_SET_WIDTH], info[F_SANS],
		   pixels, info[F_HRESOLUTION], info[F_VRESOLUTION],
		   info[F_SPACING], info[F_CHARSET]);
    /* (void) fprintf(stderr, "scaled roman = %s\n", s); */
    if ((fs = XLoadQueryFont(XtDisplay($), s)) != NULL) {
        XFreeFont(XtDisplay($), $roman);
        $roman = fs;
        create_romangc($);
    }

    XtFree(info[0]);
}

@proc create_romangc($)
{
    XGCValues val;
    XtGCMask mask = GCForeground | GCBackground | GCFont;

    if ($romangc != NULL) XtReleaseGC($, $romangc);
    val.foreground = $foreground;
    val.background = $background_pixel;
    val.font = $roman->fid;
    $romangc = XtGetGC($, mask, &val);
}

@ To construct the roman font from the |fontFamily| string, the string is
passed to |XListFonts|. The returned list is scanned for a font that is
`medium' or `normal' and `r' (i.e., roman). If none is found, the list is
scanned again for one that is `r' of any weight. If that still doesn't
produce a font, the first font from the list is taken. If there is no
|fontFamily| resource, or |XListFonts| returns no matching fonts, the
function returns |False|.

@proc Boolean infer_roman($)
{
    char **fonts;
    int n, i;

    XtFree($roman_name); $roman_name = NULL;
    if ($fontFamily == NULL) return False;
    fonts = XListFonts(XtDisplay($), $fontFamily, 1000, &n);
    if (n == 0) return False;
    for (i = 0; i < n; i++)
	if (strstr(fonts[i], "normal-r") || strstr(fonts[i], "medium-r")) {
	    $roman_name = XtNewString(fonts[i]);
	    break;
	}
    if ($roman_name == NULL)
	for (i = 0; i < n; i++)
	    if (strstr(fonts[i], "-r-")) {
		$roman_name = XtNewString(fonts[i]);
		break;
	    }
    if ($roman_name == NULL)
	$roman_name = XtNewString(fonts[0]);
    $roman = XLoadQueryFont(XtDisplay($), $roman_name);
    XFreeFontNames(fonts);
    /* (void) fprintf(stderr, "roman name = %s\n", $roman_name); */
    return True;
}


@ The |guess_roman| routine first tries to construct the font from
the |fontFamily| resource and if that fails it loads the `fixed' font,
which should always be available.

@proc guess_roman($, int offset, XrmValue* value)
{
    static XFontStruct *fixed;

    $roman_name = NULL;
    if (infer_roman($))
	value->addr = (XtPointer) &$roman;
    else {
	fixed = XLoadQueryFont(XtDisplay($), "fixed");
	value->addr = (XtPointer) &fixed;
    }
}

@ This routine (which is copied from Adrian Nye's `Xlib programming
manual,' (O'Reilly 1992), page 569), returns |True| if the passed name
is a well-formed XLFD style font name with a pixel size,point size,
and average width (fields 7, 8, and 12) of |-0-|.

@proc Boolean is_scalable_font(String name)
{
    int i, field;

    if (name == NULL || name[0] != '-') return False;
    for (i = field = 0; name[i] != '\0'; i++) {
	if (name[i] == '-') {
	    field++;
	    if (field == 7 || field == 8 || field == 12)
		if (name[i+1] != '0' || name[i+2] != '-')
		    return False;
	}
    }
    return (field == 14);
}


@ The function |parse_font_name| makes a copy of the font name into
|table[0]| (room is allocated on the heap). All hyphens are then
replaced by |\0| and the other 14 entries in |table| are made to point
to the start of each part of the name.

@proc Boolean parse_font_name(String name, String *table)
{
    int i, field;

    if (name == NULL || name[0] != '-') return False;
    table[0] = XtNewString(name);
    for (i = field = 0; name[i] != '\0'; i++) {
	if (name[i] == '-') {
	    field++;
	    table[0][i] = '\0';
	    table[field] = &table[0][i+1];
	    if (field == 13) break;		/* No more fields */
	}
    }
    return True;
}

@ The |forward_CB| callback is attached to the `flip_forward' icon. The
callback increments the page number and causes the corresponding page to
be displayed, provided the current page isn't already the last one.

@proc forward_CB(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;

    if ($page[$pageno+1] >= 0) {
	$pageno++;
	XClearWindow(XtDisplay($), XtWindow($));
	$expose($, NULL, NULL);
    }
}

@ The `flip_back' icon has gotten the |backwards_CB| callback. The
callback decrements the page number and causes the corresponding page to
be displayed, provided the page number isn't already 1.

@proc backwards_CB(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget $ = (Widget) client_data;

    if ($pageno > 1) {
	$pageno--;
	XClearWindow(XtDisplay($), XtWindow($));
	$expose($, NULL, NULL);
    }
}



@IMPORTS

@incl "flip_back.xpm"
@incl "flip_forward.xpm"
@incl <Xfwf/XmIcon.h>
@incl <stdlib.h>
@incl <fcntl.h>
@incl <stdio.h>
@incl <Xfwf/TabString.h>
@incl <X11/Xmu/Converters.h>
