# FText.w -- Formatted text widget
# Bert Bos <bert@let.rug.nl>
# Version 1.0, 17 Jan 1995

@class XfwfFormattedText (XmManager) @file=FText

@PUBLIC

@ The widget maintains 5 sets of fonts. Each font is available in 6
sizes and in styles normal, bold, italic and both (not every one of
them may be available). The fonts are selected with resources.

|sizes1| contains 7 numbers, giving the sizes of the
fonts from family 1, from smallest to largest, in decipoints.

	@var String fontFamily1 = "times"
	@var String fontCharset1 = "iso8859-1"
	@var String fontSlant1 = "i"
	@var String fontBold1 = "bold"
	@var String fontSizes1 = "72 80 100 120 140 180 240"

	@var String fontFamily2 = "helvetica"
	@var String fontCharset2 = "iso8859-1"
	@var String fontSlant2 = "o"
	@var String fontBold2 = "bold"
	@var String fontSizes2 = "72 80 100 120 140 180 240"

	@var String fontFamily3 = "courier"
	@var String fontCharset3 = "iso8859-1"
	@var String fontSlant3 = "o"
	@var String fontBold3 = "bold"
	@var String fontSizes3 = "72 80 100 120 140 180 240"

	@var String fontFamily4 = "wwwicons"
	@var String fontCharset4 = "*"
	@var String fontSlant4 = "*"
	@var String fontBold4 = "*"
	@var String fontSizes4 = "72 80 100 120 140 180 240"

	@var String fontFamily5 = "symbol"
	@var String fontCharset5 = "*"
	@var String fontSlant5 = "*"
	@var String fontBold5 = "*"
	@var String fontSizes5 = "72 80 100 120 140 180 240"


@ The |activate| callback is called when the user clicks the mouse on
something (any area occupied by a chunk of text, or something else)
with a non-null |data| field. The |call_data| argument contains a
pointer to an |XfwfFTextCallbackStruct|, with the event and the |data|
pointer of that chunk. The |reason| passed in that struct is
|XfwfActivate|.

	@var <Callback> XtCallbackList activate = NULL

@ The |enter| callback is called when the mouse pointer enters the
area occupied by a chunk with a non-null |data| field. It is passed a
pointer to an |XfwfFTextCallbackStruct|, containing the event and the
|data| of that chunk. The |reason| passed in that struct is
|XfwfEnter|.

	@var <Callback> XtCallbackList enter = NULL

@ The |leave| callback is called when the mouse leaves a chunk with a
non-null |data| field. It has the same arguments as |activate|, but
the reason is |XfwfLeave|.

	@var <Callback> XtCallbackList leave = NULL

@ The cursor to use over text that has |data| associated with
it. Default is the same cursor as the rest of the widget.

	@var Cursor activeCursor = NULL

@ Provide defaults for the size of the widget.

	@var width = 150
	@var height = 150


@EXPORTS

@ The |XfwfFTextCallbackStruct| is passed as the |call_data| argument
in callbacks. It contains a pointer to the event that caused the
callback and the |data| field of the chunk over which the event
occurred. The |reason| is |XfwfActivate|, |XfwfEnter| or |XfwfLeave|.
If the |event| field is |NULL|, this was a synthetic event, coming
from |pass_click| (or |XfwfPassClick|). |x| and |y| are the same
values as in |event|, but possibly corrected for scrolling. If |event|
is |NULL| they are the arguments passed to |pass_click|.


	@def XfwfActivate = 1
	@def XfwfEnter = 2
	@def XfwfLeave = 3

	@type XfwfFTextCallbackStruct = struct {
	    int reason;
	    XEvent *event;
	    int x, y;
	    XtPointer data;
	}

@ |TextStyle| is a set of flags, which determine the style of text.
Bits 0-2 give the size (0-6, size 7 is not used).

Bits 2-4 select the font family (0-4). Bits 4-6 select 4 levels of
superscript, from 0 (no superscript) to 3 (raise to 3rd position above
baseline). Bits 7-8 do the same for subscript.

|FtUNDERSCORE| puts a line under the text. |FtUNDERSCORE2| puts 2
underscores under the text. |FtSTRIKE| puts a line through the text.

|FtHIDEWIDTH| causes the text to be written as if it didn't take up
horizontal space. The next word will start at the same x position (and
thereby overwrite the first word, unless some horizontal space is
inserted).

	@type TextStyle = long

	@def FtSMALLEST = 0
	@def FtSMALLER = 1
	@def FtSMALL = 2
	@def FtNORMALSIZE = 3
	@def FtLARGE = 4
	@def FtLARGER = 5
	@def FtLARGEST = 6
	@def FtSIZESHIFT = 0
	@def FtSIZEMASK = 07

	@def FtFAMILYSHIFT = 3
	@def FtFAMILYMASK = (7 << FtFAMILYSHIFT)
	@def FtFAMILY1 = (0 << FtFAMILYSHIFT)
	@def FtFAMILY2 = (1 << FtFAMILYSHIFT)
	@def FtFAMILY3 = (2 << FtFAMILYSHIFT)
	@def FtFAMILY4 = (3 << FtFAMILYSHIFT)
	@def FtFAMILY5 = (4 << FtFAMILYSHIFT)

	@def FtSUPERSHIFT = 6
	@def FtSUPERMASK = (3 << FtSUPERSHIFT)

	@def FtSUBSHIFT = 8
	@def FtSUBMASK = (3 << FtSUBSHIFT)

	@def FtBOLD = (1 << 10)
	@def FtITALIC = (1 << 11)
	@def FtSTYLESHIFT = 10
	@def FtSTYLEMASK = (3 << FtSTYLESHIFT)

	@def FtSTRIKE = (1 << 12)
	@def FtHIDEWIDTH = (1 << 13)
	@def FtUNDERSCORE = (1 << 14)
	@def FtUNDERSCORE2 = (1 << 15)

@ Additional flags for an FtInline chunk.

	@def FtALIGNSHIFT = 16
	@def FtALIGNTOP = 00
	@def FtALIGNBOTTOM = (1 << FtALIGNSHIFT)
	@def FtALIGNMIDDLE = (2 << FtALIGNSHIFT)
	@def FtALIGNDEPTH = (3 << FtALIGNSHIFT)
	@def FtALIGNMASK = (3 << FtALIGNSHIFT)
	@def FtALIGNFIXEDHEIGHT = (1 << 18)
	@def FtALIGNFIXEDWIDTH = (1 << 19)


@ The next flags are used in the |mode| field of an FtParShape chunk.

	@def FtJUSTIFYSHIFT = 0
	@def FtJUSTIFYMASK = 3
	@def FtJUSTIFYBOTH = 0
	@def FtJUSTIFYLEFT = (1 << FtJUSTIFYSHIFT)
	@def FtJUSTIFYRIGHT = (2 << FtJUSTIFYSHIFT)
	@def FtJUSTIFYCENTER = (3 << FtJUSTIFYSHIFT)

	@def FtPOSITIONSHIFT = 3
	@def FtPOSITIONMASK = (3 << FtPOSITIONSHIFT)
	@def FtNORMALPARA = 0
	@def FtLEFTFLOAT = (1 << FtPOSITIONSHIFT)
	@def FtRIGHTFLOAT = (2 << FtPOSITIONSHIFT)

	@def FtNOCLEAR = 0
	@def FtCLEARLEFT = (1 << 6)
	@def FtCLEARRIGHT = (1 << 7)

	@def FtKEEPWIDTH = (1 << 8)

@ The `color' |TRANSPARENT| is not a real color, but a special code to
signal that nothing should be drawn. It is often used as the
background color for text.

	@def TRANSPARENT = ((Pixel) -1)



@ CONVENIENCE FUNCTIONS

@ The following functions are convenience functions to access the
methods of the widget. Note that none of these functions actually draw
the text on the screen. To achieve that, the calling application must
cause an expose event to be generated (or call |expose| directly).

This is for efficiency reasons. Subclasses or application, that draw
larger amounts of text at once can better decide when to make the
output appear.


@ |XfwfAddWord| adds a text string to the end of the existing text. The
string is considered a single word and won't be broken across
lines. The word is copied to private storage.

|fg| and |bg| are the colors of the foreground and background. The
special value |TRANSPARENT| is allowed for the background. |sty|
contains the relative font size, style, and underlining.

|data| is a pointer that will be passed in the |call_data| argument of
the |activate| or |enter| callback, when the user clicks on this word
or moves the mouse pointer over it.

@proc XfwfAddWord($, const String word, Cardinal len, Pixel fg, Pixel bg, TextStyle style, XtPointer data)
{
    assert(XtIsSubclass($, xfwfFormattedTextWidgetClass));
    $add_word($, word, len, fg, bg, style, data);
}

@ |XfwfAddHSpace| adds a horizontal space, which implies a possible break
point. The width of the space is |spacefactor/1000| times the normal
width of a space. The space can stretch or shrink if |stretch| is
|TRUE|.

|spacefactor| can be negative, the effect is to move the next word to
the left (possibly overwriting anything already there). Negative
spaces do not stretch or shrink.

|data| is a pointer that will be passed in the |call_data| argument of
the |activate| or |enter| callback, when the user clicks on this space
or moves the mouse pointer over it.

@proc XfwfAddHSpace($, int spacefactor, Bool stretchable, Bool breakable, Pixel fg, Pixel bg, TextStyle style, XtPointer data)
{
    assert(XtIsSubclass($, xfwfFormattedTextWidgetClass));
    $add_hspace($, spacefactor, stretchable, breakable, fg, bg, style, data);
}

@ |XfwfAddVSpace| ends the current paragraph and adds vertical space. If
|add_vspace| is called more than once without other intervening calls,
the resulting vertical space is not the sum of the spaces, but the
maximum of them. The amount of space is |linefactor/1000| times the
normal line height of the

@proc XfwfAddVSpace($, int pixels)
{
    assert(XtIsSubclass($, xfwfFormattedTextWidgetClass));
    $add_vspace($, pixels);
}

@ |XfwfAddInline| adds a widget at the end of the text. It is inserted as
if it was a word. The size of the widget is taken from its |width| and
|height| fields.

|mode| determines how much the widget extends below the baseline. If
|mode| is |FtALIGNDEPTH|, the |depth| gives the number of pixels to
lower it. If |mode| is |FtALIGNTOP|, |ftALIGNMIDDLE| or
|FtALIGNBOTTOM|, the widget is aligned with the current line's top,
bottom or middle.

|style| is used to check if the widget should raised to superscript
position.

Note: there is a shortcoming in the current implementation, in that
the height and depth of the chunk are estimated based on the height
and depth of only a part of the current line. This may mean that too
much space is left above or below the widget, especially if the widget
is the first chunk in the current line.

@proc XfwfAddInline($, Widget w, TextStyle style, int wd, int ht, int dp, int hmargin, int vmargin, XtPointer data)
{
    assert(XtIsSubclass($, xfwfFormattedTextWidgetClass));
    $add_inline($, w, style, wd, ht, dp, hmargin, vmargin, data);
}

@ |XfwfAddParShape| ends the current paragraph and then sets the margins
and the justification mode for the all subsequent ones, until the next
call.

|left| and |right| are the new margins, |mode| contains the justification
style (|FtJUSTIFYLEFT|, |FtJUSTIFYRIGHT|, |FtJUSTIFYBOTH|, |FtJUSTIFYCENTER|),
and other flags: |FtCLEARLEFT| and |FtCLEARRIGHT|
mean that the paragraph should start below any floats in the left
and/or right margin, |FtLEFTFLOAT| and |FtRIGHTFLOAT| mean that the
paragraph is itself a floating paragraph and should go in the left
resp. right margin. |FtKEEPWIDTH| means that the following text is a
caption and should have the same width as the preceeding line (which
presumably consisted of an in-line image).

@proc XfwfAddParShape($, int left, int right, int width, int mode, double leading)
{
    assert(XtIsSubclass($, xfwfFormattedTextWidgetClass));
    $add_parshape($, left, right, width, mode, leading);
}

@ |XfwfAddHRrule| ends the current paragraph and draws a horizontal
rule. The rule extends to the current left and right margins.

@proc XfwfAddHRule($)
{
    assert(XtIsSubclass($, xfwfFormattedTextWidgetClass));
    $add_hrule($);
}

@ When no more data is forthcoming, |XfwfAddEOD| can be used to finish
the current paragraph and any pending operations. Without it, some
data may be kept undisplayed, waiting for the end of the paragraph to
come.

@proc XfwfAddEOD($)
{
    assert(XtIsSubclass($, xfwfFormattedTextWidgetClass));
    $add_eod($);
}

@ The function |XfwfPassClick| is used to pass a click on one of the
child windows on to the FormattedText widget. The function actually
calls the |pass_click| method, which looks through the list of
children to see if this one has a |data| field and if so, calls the
|activate| callback, just as if the user clicked on a chunk without an
inline widget.

@proc XfwfPassClick($, Widget child, int x, int y)
{
    assert(XtIsSubclass($, xfwfFormattedTextWidgetClass));
    $pass_click($, child, x, y);
}


@PRIVATE

@ The |x| and |y| fields include the |raise| field. They
point to the baseline where the chunk should be drawn. |raise| is
kept, because the coordinates must be computed again when
some of the widget's resources change.

	@type FtType = enum {
	    FtWord, FtHSpace, FtInline, FtVSpace, FtParShape, FtHRule
	}
	@type FtWordChunk = struct {
	    TextStyle style;			/* Size & style flags */
	    Pixel fg, bg;			/* Colors */
	    String text;			/* NUL-terminated */
	    int len;				/* # of characters */
	    Font fid;				/* Font ID */
	}
	@type FtHSpaceChunk = struct {
	    TextStyle style;			/* Size & style flags */
	    Pixel fg, bg;			/* Colors */
	    int normwidth;			/* Unstretched width */
	    Bool stretchable;			/* Stretchable? */
	    Bool breakable;			/* Valid breakpoint? */
	}
	@type FtInlineChunk = struct {
	    TextStyle style;			/* Size & style flags */
	    Widget widget;			/* The embedded widget */
	    Bool mapped;			/* Widget is mapped? */
	    int hmargin, vmargin;		/* Extra pixels around obj. */
	    struct _FtChunk *prev_inlined;	/* List of inlines only */
	}
	@type FtParShapeChunk = struct {
	    int left, right, width;		/* Margins */
	    int mode;				/* Type of paragraph */
	    float leading;			/* 1.0 is normal line height */
	}
	@type FtChunk = struct _FtChunk {
	    FtType tp;				/* Type of chunk */
	    struct _FtChunk *next, *prev;	/* Doubly linked list */
	    int x, y, w, h, d, raise;		/* Position & size */
	    XtPointer data;			/* Callback data */
	    union {
		FtWordChunk word;
		FtHSpaceChunk hspace;
		FtInlineChunk inlined;
		FtParShapeChunk parshape;
	    } u;
	} *
	@type FtLine = struct {
	    int x, y, w, h;
	    FtChunk start, stop;
	}

@ The list of all chunks from |first| to |last|. |inlined_chunks| is a
list of only the inline chunks.

	@var FtChunk chunks
	@var FtChunk last
	@var FtChunk inlined_chunks


@ Arrays of lines of chunks. Each line is made up of chunks with the
same y-coordinate (except for raised and lowered text). Lines are
arranged in three arrays: the main body, the floats in the left
margin, and the floats in the right margin.

	@var FtLine *lines
	@var int nrlines
	@var int nrlines_alloc
	@var FtLine *leftlines
	@var int nrleftlines
	@var int nrleftlines_alloc
	@var FtLine *rightlines
	@var int nrrightlines
	@var int nrrightlines_alloc

@ Font sybsystem. The font array holds XFontStructs for every style
and every size of font. When an entry is |NULL|, the font has not yet
been used. There are 5 families, 4 styles, and 7 sizes.

The |sizes| array holds the 6 sizes for the four families as numbers,
instead of strings (translated from the |fontSizes1-4| resources).

	@type FontInfo = struct {
	    XFontStruct *fs;
	    int sup, sub, ulpos;		/* Positions */
	    int ulthick;			/* Underline thickness */
	    int space;				/* Normal space */
	    int em;				/* One quad */
	}
	@def NR_FAMILIES = 5
	@def NR_STYLES = 4
	@def NR_SIZES = 7
	@var FontInfo fonts[NR_FAMILIES][NR_STYLES][NR_SIZES]
	@var int fsizes[NR_FAMILIES][NR_SIZES]

@ The GCs are used for drawing the horizontal rules, non-transparent
whitespace and text.

	@var GC linegc
	@var GC gc
	@var GC spacegc

@ A vertical scrollbar, and the offset for the scrolled text

	@var Widget vscrollbar
	@var int voffset

@ The last chunk that was found in the |motion| action function.

	@var FtChunk last_motion_chunk

@ |need_reformat| is set to |NEED_REFORMAT| or
|NEED_REFORMAT_AND_REDRAW| when a child resizes itself. When the event
queue is empty, the widget will call |do_layout| to reformat the text.
With |NEED_REFORMAT|, the |do_layout| function assumes that the widget
that was resized is off-screen, and therefore no redraw is needed.

	@ def DONT_NEED_REFORMAT = 0
	@ def NEED_REFORMAT = 1
	@ def NEED_REFORMAT_AND_REDRAW = 2
	@ var int need_reformat

@ The ID of the work procedure.

	@var XtWorkProcId workid



@CLASSVARS

@ |compress_motion| must be |FALSE|, because motion events are needed
to generate |enterleave| callbacks.

	@var compress_motion = FALSE

	@var compress_exposure =
	    XtExposeCompressMultiple|XtExposeGraphicsExposeMerged

	@var compress_enterleave = True



@METHODS

@proc initialize
{
    int i, j, k, n;
    XtGCMask mask, dynmask, dontcaremask;
    XGCValues values;
    Widget vscroll;

    vscroll = XtVaCreateManagedWidget
	("VertScrollBar", xmScrollBarWidgetClass, $,
	 XmNpageIncrement, max(10, $height - 10),
	 XmNheight, $height,
	 NULL);
    XtMoveWidget(vscroll, $width - $vscroll$width, 0);
    XtAddCallback(vscroll, XmNvalueChangedCallback, scroll_cb, $);
    XtAddCallback(vscroll, XmNdragCallback, scroll_cb, $);
    $vscrollbar = vscroll;
    $voffset = 0;

    $last_motion_chunk = NULL;

    mask = 0;
    dynmask = GCForeground | GCBackground | GCFont | GCLineWidth | GCClipMask;
    dontcaremask = GCJoinStyle | GCFillRule | GCTile
	| GCTileStipXOrigin | GCTileStipYOrigin | GCDashOffset | GCDashList
	| GCStipple | GCGraphicsExposures | GCArcMode;
    $gc = XtAllocateGC($, $depth, mask, &values, dynmask, dontcaremask);

    mask = 0;
    dynmask = GCForeground | GCClipMask;
    dontcaremask = GCBackground | GCFont
	| GCJoinStyle | GCFillRule | GCTile | GCTileStipXOrigin
	| GCTileStipYOrigin | GCDashOffset | GCDashList | GCLineWidth
	| GCGraphicsExposures | GCArcMode | GCLineStyle | GCCapStyle
	| GCStipple;
    $spacegc = XtAllocateGC($, $depth, mask, &values, dynmask, dontcaremask);

    values.stipple = XCreateBitmapFromData
	(XtDisplay($), RootWindowOfScreen(XtScreen($)), line3d_bits,
	 line3d_width, line3d_height);
    values.fill_style = FillOpaqueStippled;
    values.foreground = $bottom_shadow_color;
    values.background = $top_shadow_color;
    mask = GCFillStyle | GCStipple | GCFillStyle | GCForeground | GCBackground;
    dynmask = GCTileStipXOrigin | GCTileStipYOrigin | GCClipMask;
    dontcaremask = GCLineStyle | GCCapStyle | GCJoinStyle | GCFillRule
	| GCTile | GCDashOffset | GCDashList | GCFont | GCLineWidth
	| GCArcMode | GCGraphicsExposures;
    $linegc = XtAllocateGC($, $depth, mask, &values, dynmask, dontcaremask); 


    $last = $chunks = NULL;
    $inlined_chunks = NULL;
    $lines = NULL; $nrlines_alloc = 0;
    $leftlines = NULL; $nrleftlines_alloc = 0;
    $rightlines = NULL; $nrrightlines_alloc = 0;
    for (i = 0; i < NR_FAMILIES; i++)
	for (j = 0; j < NR_STYLES; j++)
	    for (k = 0; k < NR_SIZES; k++)
		$fonts[i][j][k].fs = NULL;

    if ((n = sscanf($fontSizes1, "%d%d%d%d%d%d%d", &$fsizes[0][0],
		    &$fsizes[0][1], &$fsizes[0][2], &$fsizes[0][3],
		    &$fsizes[0][4], &$fsizes[0][5], &$fsizes[0][6])) == 0)
	$fsizes[0][n++] = 120;		/* Default */
    for (; n < NR_SIZES; n++) $fsizes[0][n] = $fsizes[0][n-1];

    if ((n = sscanf($fontSizes2, "%d%d%d%d%d%d%d", &$fsizes[1][0],
		    &$fsizes[1][1], &$fsizes[1][2], &$fsizes[1][3],
		    &$fsizes[1][4], &$fsizes[1][5], &$fsizes[1][6])) == 0)
	$fsizes[1][n++] = 120;		/* Default */
    for (; n < NR_SIZES; n++) $fsizes[1][n] = $fsizes[1][n-1];

    if ((n = sscanf($fontSizes3, "%d%d%d%d%d%d%d", &$fsizes[2][0],
		    &$fsizes[2][1], &$fsizes[2][2], &$fsizes[2][3],
		    &$fsizes[2][4], &$fsizes[2][5], &$fsizes[2][6])) == 0)
	$fsizes[2][n++] = 120;		/* Default */
    for (; n < NR_SIZES; n++) $fsizes[2][n] = $fsizes[2][n-1];

    if ((n = sscanf($fontSizes4, "%d%d%d%d%d%d%d", &$fsizes[3][0],
		    &$fsizes[3][1], &$fsizes[3][2], &$fsizes[3][3],
		    &$fsizes[3][4], &$fsizes[3][5], &$fsizes[3][6])) == 0)
	$fsizes[3][n++] = 120;		/* Default */
    for (; n < NR_SIZES; n++) $fsizes[3][n] = $fsizes[3][n-1];

    if ((n = sscanf($fontSizes5, "%d%d%d%d%d%d%d", &$fsizes[4][0],
		    &$fsizes[4][1], &$fsizes[4][2], &$fsizes[4][3],
		    &$fsizes[4][4], &$fsizes[4][5], &$fsizes[4][6])) == 0)
	$fsizes[4][n++] = 120;		/* Default */
    for (; n < NR_SIZES; n++) $fsizes[4][n] = $fsizes[4][n-1];

    init_par_builder($);

    /* $need_reformat = DONT_NEED_REFORMAT; */
    $workid = 0;
}

@proc destroy
{
    FtChunk h;
    int i, j, k;

    if ($workid) XtRemoveWorkProc($workid);
    while ($chunks) {
	h = $chunks;
	$chunks = h->next;
	if (h->tp == FtWord) XtFree(h->u.word.text);
	XtFree((String)h);
    }
    for (i = 0; i < NR_FAMILIES; i++)
	for (j = 0; j < NR_STYLES; j++)
	    for (k = 0; k < NR_SIZES; k++)
		if ($fonts[i][j][k].fs)
		    XFreeFont(XtDisplay($), $fonts[i][j][k].fs);
    XtFree((String)$lines);
    XtFree((String)$leftlines);
    XtFree((String)$rightlines);
}


@ When the widget is resized, the paragraphs have to be formatted
again. This is done by looping over the list of chunks and formatting
them again.

@proc resize
{
    Widget vscroll = $vscrollbar;

    XtConfigureWidget(vscroll, $width - $vscroll$width, 0,
		      $vscroll$width, $height, $vscroll$border_width);
    if ($lines || $leftlines || $rightlines) {
	/* $need_reformat = NEED_REFORMAT_AND_REDRAW; */
	install_do_layout($);
    } else
	init_par_builder($);
}

@ The |expose| methods looks for the set of lines that fall within the
bounding box given by the |event|. It does this by looking for the
first line that starts below the exposed region and then iterating
backwards.

Finer control over what needs to be redrawn is possible, but drawing a
few extra chunks is probably not that much slower. A clip mask is set
to save some unnecessary drawing.

@proc expose
{
    int x, y, w, h, i;

    if (! XtIsRealized($)) return;

    if (region) {
	XSetRegion(XtDisplay($), $gc, region);
	if ($linegc != $gc) XSetRegion(XtDisplay($), $linegc, region);
	if ($spacegc != $gc) XSetRegion(XtDisplay($), $spacegc, region);
    } else {
	XSetClipMask(XtDisplay($), $gc, None);
	if ($linegc != $gc) XSetClipMask(XtDisplay($), $linegc, None);
	if ($spacegc != $gc) XSetClipMask(XtDisplay($), $spacegc, None);
    }
    x = event ? event->xexpose.x : 0;
    y = (event ? event->xexpose.y : 0) + $voffset;
    w = event ? event->xexpose.width : $width;
    h = event ? event->xexpose.height : $height;

    /* fprintf(stderr, "expose FText (%d,%d,%d,%d)\n", x, y, w, h); */

    i = find_line_below_y($lines, $nrlines, y + h) - 1;
    while (i >= 0 && $lines[i].y + $lines[i].h > y) {
	draw_chunks($, $lines[i].start, $lines[i].stop);
	i--;
    }
    i = find_line_below_y($leftlines, $nrleftlines, y + h) - 1;
    while (i >= 0 && $leftlines[i].y + $leftlines[i].h > y) {
	draw_chunks($, $leftlines[i].start, $leftlines[i].stop);
	i--;
    }
    i = find_line_below_y($rightlines, $nrrightlines, y + h) - 1;
    while (i >= 0 && $rightlines[i].y + $rightlines[i].h > y) {
	draw_chunks($, $rightlines[i].start, $rightlines[i].stop);
	i--;
    }
}

@ If a child requests to be resized, the request is granted, unless
the chunk in which the child is contained has fixed width or height.
We ignore stacking order.

Somehow it seems necessary to do the |XtResizeWidget| ourselves,
because otherwise the size change will not have been done the next
time the child is (unmapped and) mapped by |do_layout|, causing a new
resize-request.

@proc geometry_manager
{
    /* Widget $ = XtParent(child); */
    FtChunk p;
    int newwd, newht;

    /* Find the chunk that contains the widget */
    for (p = $inlined_chunks; p && p->u.inlined.widget != child;
	p = p->u.inlined.prev_inlined)
	assert(p->tp == FtInline);

    if (! p) return XtGeometryYes;		/* Not a chunk */

    if ((request->request_mode & CWWidth)
	&& (p->u.inlined.style & FtALIGNFIXEDWIDTH) == 0) {
	newwd = request->width;			/* Allow change */
    	p->w = newwd + 2 * p->u.inlined.hmargin;
	/* if (p->y >= $voffset + $height || p->y + p->h <= $voffset)
	    $need_reformat = NEED_REFORMAT;
	else
	    $need_reformat = NEED_REFORMAT_AND_REDRAW; */
	install_do_layout($);
    } else {
	newwd = $child$width;			/* Old width */
    }
    if ((request->request_mode & CWHeight)
	&& (p->u.inlined.style & FtALIGNFIXEDHEIGHT) == 0) {
	newht = request->height;		/* Allow change */
	p->h = newht - p->d + 2 * p->u.inlined.vmargin;
	/* if (p->y >= $voffset + $height || p->y + p->h <= $voffset)
	    $need_reformat = NEED_REFORMAT;
	else
	    $need_reformat = NEED_REFORMAT_AND_REDRAW; */
	install_do_layout($);
    } else {
	newht = $child$height;			/* Old heigth */
    }
    XtResizeWidget(child, newwd, newht, $child$border_width);
    format_inline($, p);
    return XtGeometryDone;
}

@ When the parent asks for the preferred size, the FormattedText
widget returns the larger of 100x100 and the requested size.

@proc query_geometry
{
    reply->request_mode = CWWidth | CWHeight;
    reply->width = PREF_WD;
    reply->height = PREF_HT;
    if ((request->request_mode & CWWidth) && request->width > PREF_WD)
	reply->width = request->width;
    if ((request->request_mode & CWHeight) && request->height > PREF_HT)
	reply->height = request->height;
    /* fprintf(stderr, "%s: query_geometry %dx%d -> %dx%d\n",
	    XtName($), request->width, request->height,
	    reply->width, reply->height); */
    if (reply->width == $width && reply->height == $height)
	return XtGeometryNo;
    if ((request->request_mode & (CWWidth|CWHeight)) != (CWWidth|CWHeight))
	return XtGeometryAlmost;
    if (request->width != reply->width || request->height != reply->height)
	return XtGeometryAlmost;
    return XtGeometryYes;
}


@ TEXT CHUNK METHODS

@ The methods |add_word|, |add_hspace|, |add_vspace|, |add_inline|,
|add_parshape|, |add_hrule| and |add_eod| add chunks of information
to the widget. This is the only way to add data, there is no resource
that can be set. For documentation, see the |XfwfAdd*| functions in
the `EXPORTED' section.

|add_word| creates a word chunk and appends it to the master
chunk list. No formatting is done yet. A non-null |data| means
that the word is (part of) a hyperlink and will react to mouse
clicks (see the |activate| action procedure).

@proc add_word($, const String word, Cardinal len, Pixel fg, Pixel bg, TextStyle style, XtPointer data)
{
    FtChunk p;
    new(p);
    p->tp = FtWord;
#ifndef NDEBUG
    p->x = -9999;
    p->y = -9999;
#endif
    p->u.word.text = strndup(word, len);
    p->u.word.len = len;
    p->u.word.style = style;
    p->u.word.fg = fg;
    p->u.word.bg = bg;
    p->data = data;
    font_extents($, style, word, len, &p->h, &p->d, &p->w, &p->raise);
    if (style & FtHIDEWIDTH) p->w = 0;
    p->u.word.fid = font_id($, style);
    append($, p);
}

@ |add_hspace| adds a horizontal space to the master chunk list.
No formatting is done yet. Spaces have a spacefactor (1000 is a
normal interword space), and two flags: stretchable and
breakable.

@proc add_hspace($, int spacefactor, Bool stretchable, Bool breakable, Pixel fg, Pixel bg, TextStyle style, XtPointer data)
{
    FtChunk p;
    new(p);
    p->tp = FtHSpace;
#ifndef NDEBUG
    p->x = -9999;
    p->y = -9999;
    p->w = -9999;				/* Set by render_one_line */
#endif
    font_space($, style, &p->h, &p->d, &p->u.hspace.normwidth, &p->raise);
    p->u.hspace.normwidth = round(p->u.hspace.normwidth * spacefactor/1000);
    p->u.hspace.stretchable = stretchable && (spacefactor > 0);
    p->u.hspace.breakable = breakable && (spacefactor > 0);
    if (! p->u.hspace.stretchable) p->w = p->u.hspace.normwidth;
    p->u.hspace.fg = fg;
    p->u.hspace.bg = bg;
    p->u.hspace.style = style;
    p->data = data;
    append($, p);
}

@ |add_vspace| adds a fixed vertical skip to the master chunk
list and calls |format_vspace| to format the paragraph that just
ended (if any).

@proc add_vspace($, int pixels)
{
    FtChunk p;
    new(p);
    p->tp = FtVSpace;
#ifndef NDEBUG
    p->x = -9999;
    p->y = -9999;
    p->w = -9999;
    p->d = -9999;
    p->raise = -9999;
    p->data = NULL;
#endif
    p->h = pixels;
    append($, p);
    format_vspace($, p);
}

@ |add_inline| adds a chunk containing a widget. No formatting
is done yet. |depth| is only meaningful if |style| includes
|FtALIGNDPETH|, |width| and |height| are meaningful if |style|
includes |FtFIXEDWIDTh| resp. |FtFIXEDHEIGHT|. The width, height
and depth are set as if |style| included all three flags.
|format_inline| will change them if needed.

If the chunk has non-null |data|, the cursor of the widget is set to
the |activeCursor| (if not NULL).

@proc add_inline($, Widget w, TextStyle style, int wd, int ht, int dp, int hmargin, int vmargin, XtPointer data)
{
    FtChunk p;
    int dum1, dum2, dum3;

    new(p);
    p->tp = FtInline;
#ifndef NDEBUG
    p->x = -9999;
    p->y = -9999;
    p->raise = -9999;				/* Set by format_inline */
#endif
    p->w = wd + 2 * hmargin;
    p->h = ht - dp + vmargin;
    p->d = dp + vmargin;
    p->u.inlined.hmargin = hmargin;
    p->u.inlined.vmargin = vmargin;
    p->u.inlined.widget = w;
    p->u.inlined.mapped = FALSE;
    if (XtIsRealized(w)) {
	XtUnmapWidget(w);			/* Mapped by draw_inline() */
	XLowerWindow(XtDisplay($), XtWindow(w));
    }
    p->u.inlined.style = style;
    p->u.inlined.prev_inlined = $inlined_chunks;	/* List of inlines */
    $inlined_chunks = p;
    p->data = data;
    if ((style & (FtALIGNFIXEDWIDTH|FtALIGNFIXEDHEIGHT))) {
	if ((style & FtALIGNFIXEDWIDTH) == 0) wd = $w$width;
	if ((style & FtALIGNFIXEDHEIGHT) == 0) ht = $w$height;
	XtResizeWidget(w, wd, ht, $w$border_width);
    }
    if (style & FtHIDEWIDTH) p->w = 0;
    if (data && $activeCursor && XtIsRealized(w)) {
	XDefineCursor(XtDisplay(w), XtWindow(w), $activeCursor);
	/* XtAugmentTranslations(w, enterleavetrans); */
    }
    append($, p);
    format_inline($, p);
}

@ |add_parshape| adds a chunk that sets the margins and the position
for the following paragraph(s). It calls |format_parshape| to format
the paragraphs before it. A negative value for |width| means that the
width is not known and should be computed by the formatter itself,
based on the window width and the margins. A positive value for
|width|, in combination with a floating paragraph (|mode &
FtPOSITIONLEFT| or |FtPOSITIONRIGHT|) means that |rightmargin| is
ignored. A positive width for a normal paragraph is ignored.

@proc add_parshape($, int left, int right, int width, int mode, double leading)
{
    FtChunk p;
    new(p);
    p->tp = FtParShape;
#ifndef NDEBUG
    p->x = -9999;
    p->y = -9999;
    p->w = -9999;
    p->h = -9999;
    p->d = -9999;
    p->raise = -9999;
    p->data = NULL;
#endif
    p->u.parshape.width = width;
    p->u.parshape.left = left;
    p->u.parshape.right = right;
    p->u.parshape.mode = mode;
    p->u.parshape.leading = leading;
    append($, p);
    format_parshape($, p);
}

@ |add_hrule| adds a chunk for a horizontal rule and calls
|format_hrule| to format the paragraph that just ended (if any)
and determine the width of the rule.

@proc add_hrule($)
{
    FtChunk p;
    new(p);
    p->tp = FtHRule;
#ifndef NDEBUG
    p->x = -9999;
    p->y = -9999;
    p->w = -9999;
    p->raise = -9999;
    p->h = -9999;
    p->data = NULL;
#endif
    p->d = line3d_height;			/* Height of bitmap */
    append($, p);
    format_hrule($, p);
}

@ |add_eod| adds a final chunk and causes the last chunks to
be formatted. No calls to |add_*| should occur after |add_eod|
and |add_eod| itself may only be called once.

@proc add_eod($)
{
    $add_vspace($, 0);
    /* dump($); */
}


@ The |pass_click| method is called by the |XfwfPassClick| convenience
routine, when an application wants to simulate a click on a child
widget of the Formatted text widget.

@proc pass_click($, Widget child, int x, int y)
{
    XfwfFTextCallbackStruct info;
    FtChunk p;

    for (p = $inlined_chunks; p; p = p->u.inlined.prev_inlined)
	if (p->u.inlined.widget == child) {
	    if (p->data) {
		info.reason = XfwfActivate;
		info.event = NULL;
		info.x = x;
		info.y = y;
		info.data = p->data;
		XtCallCallbackList($, $activate, &info);
	    }
	    break;
	}
}

@ |reformat_scrollbar| repositions the scrollbar slider, based on the
current height of the formatted document.

@proc reformat_scrollbar($)
{
    int value, slider, maximum, pageinc;

    XtVaGetValues($vscrollbar, XmNvalue, &value, NULL);
    maximum = max($ypos, max($leftypos, $rightypos));
    set_max(maximum, 1);
    slider = min($height, maximum);
    set_min(value, maximum - slider);
    $voffset = value;
    pageinc = max(1, $height - 10);
    XtVaSetValues($vscrollbar, XmNmaximum, maximum, XmNsliderSize, slider,
		  XmNvalue, value, XmNincrement, 10,
		  XmNpageIncrement, pageinc, NULL);
}

@ The |get_em_of_textstyle| method is used by subclasses to get the
width of an em in the font corresponding to |style|.

@proc int get_em_of_textstyle($, TextStyle style)
{
    int fam, sty, siz, raise;
    interpret_style($, style, &fam, &sty, &siz, &raise);
    return $fonts[fam][sty][siz].em;
}

@ The |get_space_of_textstyle| method is used by subclasses to get the
width of a normal interword space in the font corresponding to |style|.

@proc int get_space_of_textstyle($, TextStyle style)
{
    int fam, sty, siz, raise;
    interpret_style($, style, &fam, &sty, &siz, &raise);
    return $fonts[fam][sty][siz].space;
}

@ The |get_lineheight_of_textstyle| method is used by subclasses to
get the vertical size (height + depth) of a normal line in the font
corresponding to |style|.

@proc get_lineheight_of_textstyle($, TextStyle style, int *h, int *d)
{
    int fam, sty, siz, raise;
    interpret_style($, style, &fam, &sty, &siz, &raise);
    *h = $fonts[fam][sty][siz].fs->ascent;
    *d = $fonts[fam][sty][siz].fs->descent;
}

@ The |get_word_extent| method is another function provided solely for
the benefit of subclasses. It computes the width, height and depth of
the word in the given style. The |raise| parameter is set to the
amount the word would be raised above the baseline. This value is
already included in the returned |h| and |d|.

@proc get_word_extent($, const String word, Cardinal len, TextStyle textstyle, int *h, int *d, int *w, int *raise)
{
    font_extents($, textstyle, word, len, h, d, w, raise);
}



@TRANSLATIONS

@trans <Btn1Down>,<Btn1Up>: activate()
@trans <Motion>: motion()
@trans <Enter>: motion()
@trans <Leave>: motion()



@ACTIONS

@ |activate| finds the chunk over which the event (must be a pointer
event) occurred and if it has a non-null |data| field, calls the
|activate| callback.

@proc activate
{
    XfwfFTextCallbackStruct info;
    int x, y;
    FtChunk p;

    switch (event->type) {
    case ButtonPress: case ButtonRelease:
	x = event->xbutton.x; y = event->xbutton.y; break;
    case MotionNotify:
	x = event->xmotion.x; y = event->xmotion.y; break;
    case EnterNotify: case LeaveNotify:
	x = event->xcrossing.x; y = event->xcrossing.y; break;
    default:
	XtAppError(XtWidgetToApplicationContext($),
		   "activate() action called with incorrect event");
    }
    y += $voffset;
    /* fprintf(stderr, "activate at (virtual) (%d,%d)\n", x, y); */
    if ((p = find_chunk_at_xy($, x, y))) {
	if (p->data) {
	    info.reason = XfwfActivate;
	    info.event = event;
	    info.x = x;
	    info.y = y - $voffset;
	    info.data = p->data;
	    XtCallCallbackList($, $activate, &info);
	}
    }
}

@ |motion| finds the chunk over which the event (must be a pointer
event) occurred and if this is a different chunk from the last time,
it calls the |leave| callback for the chunk just left and the |enter|
callback for the chunk entered. The callbacks are not called if the
chunks have a |NULL| |data| field.

@proc motion
{
    XfwfFTextCallbackStruct info;
    int x, y;
    FtChunk found;

    switch (event->type) {
    case ButtonPress: case ButtonRelease:
	x = event->xbutton.x; y = event->xbutton.y; break;
    case MotionNotify:
	x = event->xmotion.x; y = event->xmotion.y; break;
    case EnterNotify: case LeaveNotify:
	x = event->xcrossing.x; y = event->xcrossing.y; break;
    default:
	XtAppError(XtWidgetToApplicationContext($),
		   "motion() action called with incorrect event");
    }
    y += $voffset;
    found = find_chunk_at_xy($, x, y);
    if (found != $last_motion_chunk) {
	if ($last_motion_chunk != NULL && $last_motion_chunk->data != NULL) {
	    info.reason = XfwfLeave;
	    info.event = event;
	    info.x = x;
	    info.y = y - $voffset;
	    info.data = $last_motion_chunk->data;
	    XtCallCallbackList($, $leave, &info);
	}
	if (found != NULL && found->data != NULL) {
	    XDefineCursor(XtDisplay($), XtWindow($), $activeCursor);
	    info.reason = XfwfEnter;
	    info.event = event;
	    info.x = x;
	    info.y = y - $voffset;
	    info.data = found->data;
	    XtCallCallbackList($, $enter, &info);
	} else {
	    XDefineCursor(XtDisplay($), XtWindow($), None);
	}
	$last_motion_chunk = found;
    }
}



@UTILITIES

@ The preferred size of the widget. Arbitrary, but not too small.

@def PREF_WD = 150
@def PREF_HT = 150

@ |new| allocates space and points |p| to it (Pascal-like allocation).

@def new(p) = ((p) = (XtPointer) XtMalloc(sizeof(*(p))))
@def renewarray(p, n) = ((p)=(XtPointer)XtRealloc((String)p,(n)*sizeof(*(p))))

@ The |draw_chunks| routine is called by |expose| and several other
routines to draw the chunks between |first| and |last|. It uses a
single GC, which is changed when needed. 

@proc draw_word($, FtChunk p)
{
    int fam, sty, siz, raise, ulpos, ulthick, w;

    assert(p->tp == FtWord);
    XSetFont(XtDisplay($), $gc, p->u.word.fid);
    XSetForeground(XtDisplay($), $gc, p->u.word.fg);
    if (p->u.word.bg != TRANSPARENT)
	XSetBackground(XtDisplay($), $gc, p->u.word.bg);

    if (p->u.word.bg == TRANSPARENT)
	XDrawString(XtDisplay($), XtWindow($), $gc,
		    p->x, p->y - $voffset, p->u.word.text, p->u.word.len);
    else
	XDrawImageString(XtDisplay($), XtWindow($), $gc,
			 p->x, p->y - $voffset, p->u.word.text, p->u.word.len);

    if (p->u.word.style & (FtUNDERSCORE | FtUNDERSCORE2)) {
	interpret_style($, p->u.word.style, &fam, &sty, &siz, &raise);
	ulpos = $fonts[fam][sty][siz].ulpos;
	ulthick = $fonts[fam][sty][siz].ulthick;
	w = XTextWidth($fonts[fam][sty][siz].fs, p->u.word.text, p->u.word.len);
	if (w != 0 && ulthick != 0 && (p->u.word.style & FtUNDERSCORE))
	    XFillRectangle(XtDisplay($), XtWindow($), $gc,
			   p->x, p->y - $voffset + ulpos, w, ulthick);
	if (w != 0 && ulthick != 0 && (p->u.word.style & FtUNDERSCORE2))
	    XFillRectangle(XtDisplay($), XtWindow($), $gc,
			   p->x, p->y - $voffset + ulpos + 2, w, ulthick);
    }
}

@proc draw_hspace($, FtChunk p)
{
    int fam, sty, siz, raise, ulpos, ulthick;

    /* assert(p->tp == FtHSpace); */
    if (p->w > 0 && p->u.hspace.bg != TRANSPARENT) {
	XSetForeground(XtDisplay($), $spacegc, p->u.hspace.bg);
	XFillRectangle(XtDisplay($), XtWindow($), $spacegc, p->x,
		       p->y - $voffset - p->h, p->w, p->h + p->d);
    }

    if (p->w > 0 && (p->u.hspace.style & (FtUNDERSCORE | FtUNDERSCORE2))) {
	interpret_style($, p->u.hspace.style, &fam, &sty, &siz, &raise);
	ulpos = $fonts[fam][sty][siz].ulpos;
	ulthick = $fonts[fam][sty][siz].ulthick;
	if (ulthick != 0 && (p->u.hspace.style & FtUNDERSCORE))
	    XFillRectangle(XtDisplay($), XtWindow($), $gc,
			   p->x, p->y - $voffset + ulpos, p->w, ulthick);
	if (ulthick != 0 && (p->u.hspace.style & FtUNDERSCORE2))
	    XFillRectangle(XtDisplay($), XtWindow($), $gc,
			   p->x, p->y - $voffset + ulpos + 2, p->w, ulthick);
    }
}

@ |draw_inline| moves the widget to its place, based on the
information in the chunk, then maps it. Whenever the layout changes or
the widget scrolls, all child widgets are unmapped again.

@proc draw_inline($, FtChunk p)
{
    Widget w = p->u.inlined.widget;
    if ($w$x != p->x + p->u.inlined.hmargin
	|| $w$y != p->y - p->h + p->u.inlined.vmargin - $voffset) {
	XtMoveWidget(w, p->x + p->u.inlined.hmargin,
		     p->y - p->h + p->u.inlined.vmargin - $voffset);
    }
    if (! p->u.inlined.mapped) {
	XtMapWidget(w);
	p->u.inlined.mapped = TRUE;
    }
}

@proc draw_hrule($, FtChunk p)
{
    XSetTSOrigin(XtDisplay($), $linegc, p->x, p->y - $voffset);
    XFillRectangle(XtDisplay($), XtWindow($), $linegc, p->x,
		   p->y - $voffset, p->w, p->d);
}

@proc draw_chunks($, FtChunk first, FtChunk stop)
{
    for (; first != stop; first = first->next)
	switch (first->tp) {
	case FtWord: draw_word($, first); break;
	case FtHSpace: draw_hspace($, first); break;
	case FtVSpace: break;
	case FtInline: draw_inline($, first); break;
	case FtParShape: break;
	case FtHRule: draw_hrule($, first); break;
	default: assert(! "Cannot happen");
	}
}


@ PARAGRAPH BUILDER

@ The paragraph builder consists of the routines |init_par_builder|,
|fini_par_builder|, and |format_*|, which are called from the
methods. The paragraph builder maintains a number of variables that
determine how the current paragraph should be formatted.

@PRIVATE

@ |fullwidth| = width of the widget; |leftmargin| = left indent;
|rightmargin| = right indent; |targetwidth| computed whenever a new
paragraph shape starts: the width of the next line. |$next_to_render|
is the first chunk that has not been formatted. It is used by
|format_paragraphs|.

	@type ParaType = enum {NormalPara, LeftFloat, RightFloat}

	@var int fullwidth
	@var int ypos
	@var int vspace
	@var int leftypos
	@var int leftvspace
	@var int rightypos
	@var int rightvspace
	@var FtChunk next_to_render

@ Spaces can shrink to 0.6 times their normal size.

	@def SHRINK = 0.6

@UTILITIES

@def set_max(a, b) = if ((a) >= (b)) ; else (a) = (b)
@def set_min(a, b) = if ((a) <= (b)) ; else (a) = (b)
@def min(a, b) = ((a) < (b) ? (a) : (b))
@def max(a, b) = ((a) > (b) ? (a) : (b))
@def round(f) = ((int) ((f) + 0.5))

@ |init_par_builder| initializes the private variables used by the
paragraph builder.

@proc init_par_builder($)
{
    Widget vscroll = $vscrollbar;
    $fullwidth = $width - $vscroll$width;
    $ypos = 0;					/* Bottom of previous line */
    $vspace = 0;
    $leftypos = 0;				/* Bottom of left floats */
    $leftvspace = 0;
    $rightypos = 0;				/* Bottom of right floats */
    $rightvspace = 0;
    $next_to_render = NULL;
    $nrlines = 0;
    $nrleftlines = 0;
    $nrrightlines = 0;
}

@ The |append| function adds a chunk to the global list of chunks.

@proc append($, FtChunk p)
{
    p->next = NULL;
    p->prev = $last;
    if (! $chunks) $chunks = p;
    else $last->next = p;
    $last = p;
}

@ |strndup| allocates space for |n+1| characters and copies the text
to it.

@proc String strndup(const String s, Cardinal n)
{
    String t = XtMalloc((n + 1) * sizeof(*t));
    *t = '\0';
    strncat(t, s, n);
    return t;
}

@ |find_line_below_y| find the first line that starts below
y-coordinate |y|. Returns |nrlines| if no line is found.

@proc int find_line_below_y(FtLine *lines, int nrlines, int y)
{
    int l = 0, h = nrlines, m;

    while (l != h) {
        m = (l + h)/2;
        if (lines[m].y <= y) l = m + 1; else h = m;
    }
    assert(l == 0 || lines[l-1].y <= y);
    return l;
}


@ |find_chunk_at_xy| finds the chunk that includes coordinates
|(x,y)|, if it exists. It uses |local_find_chunk_at_xy| to do most of
the work.

@proc FtChunk local_find_chunk_at_xy(FtLine *lines, int nrlines, int x, int y)
{
    int n;
    FtChunk p;

    n = find_line_below_y(lines, nrlines, y) - 1;
    if (n >= 0 && lines[n].y + lines[n].h > y
        && lines[n].x <= x && lines[n].x + lines[n].w > x) {
	assert(lines[n].y <= y);
        for (p = lines[n].start; p && p != lines[n].stop; p = p->next)
            if (p->x <= x && x < p->x + p->w
                && p->y - p->h <= y && y < p->y + p->d)
                return p;
    }
    return NULL;
}

@proc FtChunk find_chunk_at_xy($, int x, int y)
{
    FtChunk p;

    p = local_find_chunk_at_xy($lines, $nrlines, x, y);
    if (! p) p = local_find_chunk_at_xy($leftlines, $nrleftlines, x, y);
    if (! p) p = local_find_chunk_at_xy($rightlines, $nrrightlines, x, y);
    return p;
}

@ |resort_lines_vertically| inserts the last added line at its proper
place in the array of lines. The line with number |nrlines-1| is the
line that must be inserted. The other lines have to shift one place.

@proc resort_lines_vertically(FtLine *lines, int nrlines)
{
    int i, j;
    FtLine h;

    j = nrlines - 1;				/* The line to insert */
    i = nrlines - 2;
    while (i >= 0 && lines[i].y > lines[j].y) i--;
    if (i < nrlines - 2) {
	i++;
	h = lines[j];
	for (; j > i; j--) lines[j] = lines[j-1]; /* Shift lines */
	lines[i] = h;
    }
}

@ |append_line| adds a new structure for a completed line to one
of the three lists of lines. These lists (arrays) are used in
|expose| and |activate| to more quickly find the chunk that
contains a certain pair of coordinates. It is assumed that lines
do not overlap.

@def INCR = 50

@proc append_line($, int mode, int x, int y, int w, int h, FtChunk start, FtChunk stop)
{
    if ((mode & FtLEFTFLOAT)) {
	assert($nrleftlines <= $nrleftlines_alloc);
	if ($nrleftlines == $nrleftlines_alloc) {
	    $nrleftlines_alloc = ($nrleftlines_alloc + INCR)/INCR * INCR;
	    renewarray($leftlines, $nrleftlines_alloc);
	}
        $leftlines[$nrleftlines].x = x;
        $leftlines[$nrleftlines].y = y;
        $leftlines[$nrleftlines].w = w;
        $leftlines[$nrleftlines].h = h;
        $leftlines[$nrleftlines].start = start;
        $leftlines[$nrleftlines].stop = stop;
        $nrleftlines++;
        resort_lines_vertically($leftlines, $nrleftlines);
    } else if ((mode & FtRIGHTFLOAT)) {
	assert($nrrightlines <= $nrrightlines_alloc);
	if ($nrrightlines == $nrrightlines_alloc) {
	    $nrrightlines_alloc = ($nrrightlines_alloc + INCR)/INCR * INCR;
	    renewarray($rightlines, $nrrightlines_alloc);
	}
        $rightlines[$nrrightlines].x = x;
        $rightlines[$nrrightlines].y = y;
        $rightlines[$nrrightlines].w = w;
        $rightlines[$nrrightlines].h = h;
        $rightlines[$nrrightlines].start = start;
        $rightlines[$nrrightlines].stop = stop;
        $nrrightlines++;
        resort_lines_vertically($rightlines, $nrrightlines);
    } else {
	assert($nrlines <= $nrlines_alloc);
	if ($nrlines == $nrlines_alloc) {
	    $nrlines_alloc = ($nrlines_alloc + INCR)/INCR * INCR;
	    renewarray($lines, $nrlines_alloc);
	}
        $lines[$nrlines].x = x;
        $lines[$nrlines].y = y;
        $lines[$nrlines].w = w;
        $lines[$nrlines].h = h;
        $lines[$nrlines].start = start;
        $lines[$nrlines].stop = stop;
        $nrlines++;
        resort_lines_vertically($lines, $nrlines);
    }
}

@ |find_parshape_before| searches for the last |FtParShape|
chunk before (and including) |h| and returns it. If there is
none, it returns |NULL|.

@proc FtChunk find_parshape_before(FtChunk h)
{
#if 0
    if (h == NULL) return NULL;
    else if (h->tp == FtParShape) return h;
    else return find_parshape_before(h->prev);
#else
    while (h && h->tp != FtParShape) h = h->prev;
    return h;
#endif
}

@def is_breakable(h) =
    ((h->tp == FtHSpace && h->u.hspace.breakable)
    || h->tp == FtVSpace || h->tp == FtParShape || h->tp == FtHRule)

@ |render_one_line| sets the coordinates of the chunks between
|first| and |stop|. If |may_fill| is |FALSE|, a mode of
|FtJUSTIFYFULL| is reset to |FtJUSTIFYLEFT|. |sumwd| doesn't
include the width of the stretchable spaces.

@proc render_one_line($, FtChunk first, FtChunk stop, int mode, int x, int top, int maxht, int maxdp, int targetwd, int sumwd, int sumstretch, Bool may_fill)
{
    FtChunk p;
    int desiredwhite, minwhite, startx, base;

    assert(! is_breakable(first));
    while (stop->prev != first
	   && stop->prev->tp != FtWord && stop->prev->tp != FtInline)
        stop = stop->prev;

    base = top + maxht;
    minwhite = sumstretch * SHRINK;
    if (sumwd + minwhite > targetwd)		/* Doesn't fit any way */
        desiredwhite = minwhite;
    else if (sumwd + sumstretch > targetwd)	/* Needs some shrink */
        desiredwhite = targetwd - sumwd;
    else if ((mode & FtJUSTIFYMASK) == FtJUSTIFYBOTH && may_fill)
        desiredwhite = targetwd - sumwd;
    else					/* Fits with normal spaces */
        desiredwhite = sumstretch;

    if ((mode & FtJUSTIFYMASK) == FtJUSTIFYCENTER)
	x += (targetwd - sumwd - desiredwhite)/2;
    else if ((mode & FtJUSTIFYMASK) == FtJUSTIFYRIGHT)
	x += targetwd - sumwd - desiredwhite;
    startx = x;

    for (p = first; p != stop; p = p->next) {	/* Set all coordinates */
	p->x = x;
	p->y = base;
	if (p->tp != FtHSpace || ! p->u.hspace.stretchable) {
	    x += p->w;
	} else {
	    assert(sumstretch != 0);
	    p->w = p->u.hspace.normwidth * desiredwhite/sumstretch;
	    x += p->w;
	    desiredwhite -= p->w;
	    sumstretch -= p->u.hspace.normwidth;
	}
    }
#if 0
    append_line($, mode, startx, top, x - startx, maxht + maxdp, first, stop);
#endif
}

@ |find_break| finds the next chunk after |h| before which a
line can be broken. This must be a vspace, parshape, or an
hspace with |breakable| set. It returns the width of the
unstrechable chunks, the normal width of the stretchable spaces
and the maximum height and depth of the chunks between |h|
(inclusive) and the returned breakpoint (exclusive).

@proc FtChunk find_break(FtChunk h, FtChunk last, int * wd, int * stretch, int * ht, int * dp)
{
    assert(h != last && is_breakable(last));
    *stretch = 0;  *wd = 0;  *ht = 0;  *dp = 0;
    do {
        if (h->tp != FtHSpace || ! h->u.hspace.stretchable) *wd += h->w;
        else *stretch += h->u.hspace.normwidth;
        set_max(*ht, h->h);
        set_max(*dp, h->d);
        h = h->next;
    } while (! is_breakable(h));
    return h;
}

@ |left_float_width| finds the maximum width of the floats in the left
margin between y coordinates |topy| and |boty|. (Could use binary
search here, via |find_line_below_y|.)

@proc int left_float_width($, int topy, int boty)
{
    int i, w = 0;
    for (i = 0; i < $nrleftlines; i++) {
	if ($leftlines[i].y >= boty) break;
	else if ($leftlines[i].y + $leftlines[i].h < topy) continue;
	else set_max(w, $leftlines[i].x + $leftlines[i].w);
    }
    return w;
}

@ |right_float_width| finds the maximum width of the floats in the
right margin between y coordinates |topy| and |boty|.

@proc int right_float_width($, int topy, int boty)
{
    int i, w = 0;
    for (i = 0; i < $nrrightlines; i++) {
	if ($rightlines[i].y >= boty) break;
	else if ($rightlines[i].y + $rightlines[i].h < topy) continue;
	else set_max(w, $fullwidth - $rightlines[i].x);
    }
    return w;
}

@ |compute_width| computes the width that is available to a paragraph
starting with parshape |p| at vertical position between |ytop| and
|ybottom|. This is the target width for the text at that position.
Note that the space taken up by floats is subtracted from the margins.

@proc int compute_width($, FtChunk p, int ytop, int ybottom)
{
    int lft, rgt;

    assert(!p || p->tp == FtParShape);
    if (p == NULL) {
	/* No parshape, use full width minus floats */
	lft = left_float_width($, ytop, ybottom);
	rgt = right_float_width($, ytop, ybottom);
	return $fullwidth - lft - rgt;
    } else if ((p->u.parshape.mode & (FtLEFTFLOAT | FtRIGHTFLOAT)) == 0) {
	/* This is normal paragraph */
	lft = left_float_width($, ytop, ybottom);
	set_max(lft, p->u.parshape.left);
	rgt = right_float_width($, ytop, ybottom);
	set_max(rgt, p->u.parshape.right);
        return $fullwidth - lft - rgt;
    } else if (p->u.parshape.width > 0) {
	/* This is a floating par. with explicit width */
        return p->u.parshape.width;
    } else if (p->next && ! is_breakable(p->next)) {
	/* This is a floating par. without explicit width */
#if 0
	fprintf(stderr, "float width taken from next chunk (tp=%d): %d\n",
		p->next->tp, p->next->w);
#endif
	return p->next->w;
    } else {
	/* This is floating par. with nothing fixed following it */
#if 0
	fprintf(stderr, "float width estimated: %d\n",
		$fullwidth/3 - p->u.parshape.left -
		p->u.parshape.right);
#endif
	return $fullwidth/3 - p->u.parshape.left - p->u.parshape.right;
    }
}

@ |render_paragraphs| formats all lines from where it left off
(|$next_to_render|) up to |last|. |last| itself is assumed to be
a valid breakpoint (i.e., not a word or unbreakable space).

@proc FtChunk render_paragraphs($, FtChunk last)
{
    FtChunk p, p2, q, breakpoint;
    int mode, sumwd, maxht, maxdp, sumstretch;
    int wd, ht, dp, stretchwd, targetwd, targetwd1;
    int maxht1, maxdp1, sumwd1, sumstretch1, y, x, x1, x2;
    float leading;

    assert(last->tp != FtWord && last->tp != FtHSpace && last->tp != FtInline);

    if (! $next_to_render) $next_to_render = $chunks;
    while ($next_to_render != last && is_breakable($next_to_render))
        $next_to_render = $next_to_render->next;

    /* Find first parshape to use */
    p = find_parshape_before($next_to_render);

    while ($next_to_render != last) {
        breakpoint = find_break($next_to_render, last,
				&sumwd, &sumstretch, &maxht, &maxdp);
        /* sumwd doesn't include the stretchable space */

	assert(!p || p->tp == FtParShape);        
        mode = p ? p->u.parshape.mode : 0;
	leading = p ? p->u.parshape.leading : 1.0;

        if (mode & FtRIGHTFLOAT) y = $rightypos + $rightvspace;
        else if (mode & FtLEFTFLOAT) y = $leftypos + $leftvspace;
        else y = $ypos + $vspace;

        targetwd = compute_width($, p, y, y + maxht + maxdp);
	if (targetwd < $width/10) {		/* Try to flush floats */
	    set_max(y, min($leftypos + $leftvspace,
			   $rightypos + $rightvspace));
	    targetwd = compute_width($, p, y, y + maxht + maxdp);
	}
	if (targetwd < $width/10) {		/* Flush all floats */
	    set_max(y, max($leftypos + $leftvspace,
			   $rightypos + $rightvspace));
	    targetwd = compute_width($, p, y, y + maxht + maxdp);
	}

        while (breakpoint != last) {
            q = find_break(breakpoint, last, &wd, &stretchwd, &ht, &dp);
            sumwd1 = sumwd + wd;
            sumstretch1 = sumstretch + stretchwd;
            if (sumwd1 + sumstretch1 * SHRINK > targetwd)
                break;                  /* Break at last breakpoint */
            if (ht > maxht || dp > maxdp) { /* Recompute targetwd */
                maxht1 = max(maxht, ht);
                maxdp1 = max(maxdp, dp);
                targetwd1 = compute_width($, p, y, y + maxht1 + maxdp1);
                if (sumwd1 + sumstretch1 * SHRINK > targetwd1)
                    break;              /* Doesn't fit, due to ht/dp */
                /* Still fits, despite larger height or depth */
                maxht = maxht1;
                maxdp = maxdp1;
                targetwd = targetwd1;
            }
            sumwd = sumwd1;
            sumstretch = sumstretch1;
            breakpoint = q;
        }

        /* breakpoint is best breakpoint, render one line */
        assert(is_breakable(breakpoint));
        if (mode & FtLEFTFLOAT) {
	    x1 = 0;
            x = p->u.parshape.left;
	    x2 = x + targetwd + p->u.parshape.right;
        } else if (mode & FtRIGHTFLOAT) {
            x = $fullwidth - targetwd - p->u.parshape.right;
	    x1 = x - p->u.parshape.left;
	    x2 = $fullwidth;
        } else if (! p) {
	    x = left_float_width($, y, y + maxht + maxdp);
	    x1 = x;
	    x2 = x + targetwd;
        } else {
	    x1 = left_float_width($, y, y + maxht + maxdp);
	    x = max(x1, p->u.parshape.left);
	    x2 = x + targetwd + p->u.parshape.right;
	    /* x2 may be too large, should we compare it to right_float? */
	}
        render_one_line($, $next_to_render, breakpoint, mode, x, y, maxht,
			maxdp, targetwd, sumwd, sumstretch,
			breakpoint != last);
#if 1
	append_line($, mode, x1, y, x2 - x1, maxht + maxdp,
		    $next_to_render, breakpoint);
#endif

        /* Set variables for next cycle */
        if (mode & FtLEFTFLOAT) {
            $leftypos = y + round((maxht + maxdp) * leading);
            $leftvspace = 0;
        } else if (mode & FtRIGHTFLOAT) {
            $rightypos = y + round((maxht + maxdp) * leading);
            $rightvspace = 0;
        } else {
            $ypos = y + round((maxht + maxdp) * leading);
            $vspace = 0;
        }
	/* Small optimization: if we encounter a parshape,
	 * we don't have to call find_parshape_before() below
	 */
	p2 = NULL;
        while (breakpoint != last && is_breakable(breakpoint)) {
	    if (breakpoint->tp == FtParShape) p2 = breakpoint;
            breakpoint = breakpoint->next;
	}
        $next_to_render = breakpoint;

	/* Find parshape to use for next line */
	p = p2 ? p2 : find_parshape_before($next_to_render);
    }
    return p;
}

@ |format_vspace| calls |render_paragraphs| to render any
unformatted paragraphs before |h|. It then stores the vertical
space to offset the next paragraph.

@proc format_vspace($, FtChunk h)
{
    FtChunk p;
    int mode;
    p = render_paragraphs($, h);
    /* p = find_parshape_before(h); */
    mode = p ? p->u.parshape.mode : FtNORMALPARA;
    switch (mode & FtPOSITIONMASK) {
    case FtNORMALPARA: set_max($vspace, h->h); break;
    case FtLEFTFLOAT: set_max($leftvspace, h->h); break;
    case FtRIGHTFLOAT: set_max($rightvspace, h->h); break;
    default: assert(! "Cannot happen");
    }
}

@ |format_inline| adjusts the size of the inline chunk to the
size of the widget inside it, unless it has a fixed height or
width. If it has, the |w| field is left unchanged and the sum |h
+ d| is kept fixed.

@proc format_inline($, FtChunk h)
{
    Widget w = h->u.inlined.widget;
    int wd, ht, dp, fixedht;
    int vm = h->u.inlined.vmargin, hm = h->u.inlined.hmargin;

    assert(! (h->u.inlined.style & FtHIDEWIDTH) || h->w == 0);
    if ((h->u.inlined.style & (FtHIDEWIDTH|FtALIGNFIXEDWIDTH)) == 0)
	h->w = $w$width + 2 * hm;
    font_space($, h->u.inlined.style, &ht, &dp, &wd, &h->raise);
    if (h->u.inlined.style & FtALIGNFIXEDHEIGHT) {
        fixedht = h->h + h->d;
        switch (h->u.inlined.style & FtALIGNMASK) {
        case FtALIGNTOP:
            h->h = ht; h->d = fixedht - ht; break;
        case FtALIGNBOTTOM:
            h->d = dp; h->h = fixedht - dp; break;
        case FtALIGNMIDDLE:
            h->h = (fixedht + ht - dp)/2; h->d = fixedht - h->h; break;
	case FtALIGNDEPTH:
	    h->h = fixedht - h->d; break;
        default:
            assert(! "Cannot happen");
        }
	assert(h->h + h->d == fixedht);
    } else {					/* No fixed height */
	switch (h->u.inlined.style & FtALIGNMASK) {
	case FtALIGNTOP:
	    h->h = ht + vm; h->d = $w$height - ht + vm; break;
	case FtALIGNBOTTOM:
	    h->d = dp + vm; h->h = $w$height - dp + vm; break;
	case FtALIGNMIDDLE:
	    h->h = ($w$height + ht - dp)/2 + vm;
	    h->d = $w$height - h->h + 2 * vm;
	    break;
	case FtALIGNDEPTH:
	    h->h = $w$height + 2 * vm - h->d; break;
	default:
	    assert(! "Cannot happen");
	}
    }
}

@ |find_width_before| finds the last parshape or inline before
|h| and returns its width. It is used in |format_parshape| to
compute the width of a floating paragraph with |FtKEEPWIDTH|
set. If no suitable chunk is found, it returns a negative
number.

@proc int find_width_before($, FtChunk h)
{
    h = h->prev;
    for (; h && h->tp != FtParShape && h->tp != FtInline; h = h->prev) ;
    if (! h) return -1;
    else if (h->tp == FtInline) return h->w;
    else if (h->u.parshape.mode & (FtLEFTFLOAT | FtRIGHTFLOAT)) {
        assert(h->u.parshape.width >= 0);
        return h->u.parshape.width;
    } else return -1;
}

@ |format_parshape| calls |render_paragraphs| to format the
paragraphs before |h| and flushes floating figures if the |mode|
field calls for it. For floating paragraphs, the |width| field
is computed if it is not set already (negative value means `not
set'). The default width of floating paragraphs is one third the
full width of the window.

@proc format_parshape($, FtChunk p)
{
    int width;

    (void) render_paragraphs($, p);
    switch (p->u.parshape.mode & FtPOSITIONMASK) {
    case FtNORMALPARA:
        if (p->u.parshape.mode & FtCLEARLEFT)
            set_max($ypos, $leftypos + $leftvspace - $vspace);
        if (p->u.parshape.mode & FtCLEARRIGHT)
            set_max($ypos, $rightypos +$rightvspace - $vspace);
#if 0
        assert(left_float_width($, $ypos + $vspace, $ypos + $vspace)
            + right_float_width($, $ypos + $vspace, $ypos + $vspace)
            <= $fullwidth);
#endif
        break;
    case FtLEFTFLOAT:
	set_max($leftypos, $ypos + $vspace - $leftvspace);
        if (p->u.parshape.mode & FtCLEARRIGHT)
            set_max($leftypos, $rightypos + $rightvspace - $leftvspace);
        if (p->u.parshape.width >= 0)
            ;
        else if ((p->u.parshape.mode & FtKEEPWIDTH)
            && (width = find_width_before($, p)) >= 0)
            p->u.parshape.width = width;
#if 0
        else /* if (p->u.parshape.width < 0) */
            p->u.parshape.width = $fullwidth/3
                - p->u.parshape.left - p->u.parshape.right;
#endif
        /* check if there is no collision of floats */
	if (left_float_width($, $leftypos + $leftvspace,
			     $leftypos + $leftvspace)
            + right_float_width($, $leftypos + $leftvspace,
				$leftypos + $leftvspace) >= $fullwidth)
            $leftypos = $rightypos - $leftvspace;
        break;
    case FtRIGHTFLOAT:
	set_max($rightypos, $ypos + $vspace - $rightvspace);
        if (p->u.parshape.mode & FtCLEARLEFT)
            set_max($rightypos, $leftypos + $leftvspace - $rightvspace);
        if (p->u.parshape.width >= 0)
            ;
        else if ((p->u.parshape.mode & FtKEEPWIDTH)
            && (width = find_width_before($, p)) >= 0)
            p->u.parshape.width = width;
#if 0
        else /* if (p->u.parshape.width < 0) */
            p->u.parshape.width = $fullwidth/3
                - p->u.parshape.left - p->u.parshape.right;
#endif
        /* check if there is no collision of floats */
	if (left_float_width($, $rightypos + $rightvspace,
			     $rightypos + $rightvspace)
            + right_float_width($, $rightypos + $rightvspace,
				$rightypos + $rightvspace) >= $fullwidth)
            $rightypos = $leftypos - $rightvspace;
        break;
    default:
        assert(! "Cannot happen");
    }
}

@ |format_hrule| calls |render_paragraph| to render any
unformatted paragraphs before |h|. It then sets the position and
width of the rule itself. |h->d| contains the height of the
rule, |h->h| is not used.

@proc format_hrule($, FtChunk h)
{
    FtChunk p;
    int mode, left, right;

    assert(h->d >= 0);
    p = render_paragraphs($, h);
    /* p = find_parshape_before(h); */
    mode = p ? p->u.parshape.mode : FtNORMALPARA;
    left = p ? p->u.parshape.left : 0;
    right = p ? p->u.parshape.right : 0;
    switch (mode & FtPOSITIONMASK) {
    case FtNORMALPARA:
        h->y = $ypos + $vspace;
        h->x = left_float_width($, h->y, h->y + h->d) + left;
        h->w = $fullwidth - h->x - right
            - right_float_width($, h->y, h->y + h->d);
        $vspace = 0;
        $ypos = h->y + h->d;
        break;
    case FtLEFTFLOAT:
        h->y = $leftypos + $leftvspace;
        h->x = left;
        assert(p && p->u.parshape.width >= 0);
        h->w = p->u.parshape.width;
        $leftvspace = 0;
        $leftypos = h->y + h->d;
        break;
    case FtRIGHTFLOAT:
        h->y = $rightypos + $rightvspace;
        assert(p && p->u.parshape.width >= 0);
        h->w = p->u.parshape.width;
        h->x = $fullwidth - p->u.parshape.width - right;
        $rightvspace = 0;
        $rightypos = h->y + h->d;
        break;
    default:
	assert(! "Cannot happen");
    }
    append_line($, mode, h->x, h->y, h->w, h->d, h, h->next);
}

@ |unmap_children| is called from |scroll_cb| to hide
the child widgets. The |draw_inline| functions maps the widgets that
are in view.

@proc unmap_children($)
{
    FtChunk p;

    for (p = $inlined_chunks; p; p = p->u.inlined.prev_inlined) {
	assert(p->tp == FtInline);
#if 0
	if (p->y + p->h - $voffset > 0 && p->y - $voffset <= $height)
	    XtMoveWidget(p->u.inlined.widget, p->x + p->u.inlined.hmargin,
			 p->y - p->h + p->u.inlined.vmargin - $voffset);
	else if (p->u.inlined.mapped) {
	    XtUnmapWidget(p->u.inlined.widget);
	    p->u.inlined.mapped = FALSE;
	}
#else
	XtMoveWidget(p->u.inlined.widget, p->x + p->u.inlined.hmargin,
		     p->y - p->h + p->u.inlined.vmargin - $voffset);
#endif
    }
}

@ The |scroll_cb| function is a callback that is attached to the
valueChanged callback of the vertical scrollbar. It adjusts the
vertical offset of the text in the widget and redisplays the text.

TO DO: use |XCopyArea| and selectively re-map the children.

@proc scroll_cb(Widget vscroll, XtPointer client_data, XtPointer call_data)
{
    XmScrollBarCallbackStruct *info = (XmScrollBarCallbackStruct *) call_data;
    Widget $ = (Widget) client_data;
    int delta;

    delta = info->value - $voffset;
    $voffset = info->value;
    unmap_children($);
#if 1
    XCopyArea(XtDisplay($), XtWindow($), XtWindow($),
	      DefaultGCOfScreen(XtScreen($)), 0, delta,
	      $width - $vscroll$width, $height, 0, 0);
#else
    XClearArea(XtDisplay($), XtWindow($), 0, 0, 0, 0, TRUE);
#endif
}

@ |do_layout| is a work procedure, called whenever the event queue is
empty. It checks if the widget needs to be reformatted. It always
returns |TRUE|, to indicate that it should be removed from the list of
work procs after it returns.

@proc Boolean do_layout(XtPointer client_data)
{
    Widget $ = (Widget) client_data;
    FtChunk p;

    init_par_builder($);
    for (p = $chunks; p; p = p->next) {
	switch (p->tp) {
	case FtWord: break;
	case FtHSpace: break;
	case FtInline: format_inline($, p); break;
	case FtVSpace: format_vspace($, p); break;
	case FtParShape: format_parshape($, p); break;
	case FtHRule: format_hrule($, p); break;
	default: assert(! "Cannot happen");
	}
    }
    unmap_children($);
    if (XtIsRealized($)) {
	XClearArea(XtDisplay($), XtWindow($), 0, 0, 0, 0, TRUE);
	/* $expose($, NULL, NULL); */
    }
    $reformat_scrollbar($);
        $workid = 0;
    return TRUE;				/* Un-install */
}

@ |install_do_layout| installs |do_layout| as a work procedure, if it
has not been installed before.

@proc install_do_layout($)
{
    if (! $workid)
	$workid = XtAppAddWorkProc(XtWidgetToApplicationContext($),
				   do_layout, $);
}


@ FONT SYSTEM

@ The font family names and the sizes are combined with other
information to select the proper fonts. Initially, no fonts are
allocated (to speed up starting), only when a font is needed is it
actually allocated. When a font cannot be found at once, the
characteristics are relaxed one by one, until a matching font is
found.

@ |load_font| finds the font that best matches the style and stores
information about it.

@proc load_font($, int family, int style, int size)
{
    unsigned long val;
    char s[256], *fam, *charset, *slant, *bold;
    XFontStruct *fs;
    int siz, hres, vres;

    assert(0 <= family && family < NR_FAMILIES);
    switch (family) {
    case 4:
	fam = $fontFamily5;
	charset = $fontCharset5;
	slant = $fontSlant5;
	bold = $fontBold5;
	break;
    case 3:
	fam = $fontFamily4;
	charset = $fontCharset4;
	slant = $fontSlant4;
	bold = $fontBold4;
	break;
    case 2:
	fam = $fontFamily3;
	charset = $fontCharset3;
	slant = $fontSlant3;
	bold = $fontBold3;
	break;
    case 1:
	fam = $fontFamily2;
	charset = $fontCharset2;
	slant = $fontSlant2;
	bold = $fontBold2;
	break;
    default:
	fam = $fontFamily1;
	charset = $fontCharset1;
	slant = $fontSlant1;
	bold = $fontBold1;
	break;
    }
    assert(0 <= size && size < NR_SIZES);
    siz = $fsizes[family][size];
    hres = 25.4 * WidthOfScreen(XtScreen($))/WidthMMOfScreen(XtScreen($));
    vres = 25.4 * HeightOfScreen(XtScreen($))/HeightMMOfScreen(XtScreen($));
    if (abs(hres - 75) > abs(hres - 100)) hres = 100; else hres = 75;
    if (abs(vres - 75) > abs(vres - 100)) vres = 100; else vres = 75;

    switch (style) {
    case 0:					/* Normal style */
	sprintf(s, "-*-%s-medium-r-normal--*-%d-%d-%d-*-*-%s",
		fam, siz, hres, vres, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "-*-%s-medium-r-*--*-%d-*-*-*-*-%s", fam, siz, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "-*-*-medium-r-*--*-%d-*-*-*-*-%s", siz, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "fixed");
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	XtAppError(XtWidgetToApplicationContext($), "Failed to find Xfonts");
	break;
    case 1:					/* Bold style */
	sprintf(s, "-*-%s-%s-r-normal--*-%d-%d-%d-*-*-%s",
		fam, bold, siz, hres, vres, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "-*-%s-%s-r-*--*-%d-*-*-*-*-%s",
		fam, bold, siz, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "-*-%s-*-r-*--*-%d-*-*-*-*-%s",
		fam, siz, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "fixed");
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	XtAppError(XtWidgetToApplicationContext($), "Failed to find Xfonts");
	break;
    case 2:					/* Italic style */
	sprintf(s, "-*-%s-medium-%s-normal--*-%d-%d-%d-*-*-%s",
		fam, slant, siz, hres, vres, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "-*-%s-medium-%s-*--*-%d-*-*-*-*-%s",
		fam, slant, siz, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "-*-%s-*-%s-*--*-%d-*-*-*-*-%s",
		fam, slant, siz, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "fixed");
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	XtAppError(XtWidgetToApplicationContext($), "Failed to find Xfonts");
	break;
    case 3:					/* Bold-italic style */
	sprintf(s, "-*-%s-%s-%s-normal--*-%d-%d-%d-*-*-%s",
		fam, bold, slant, siz, hres, vres, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "-*-%s-%s-%s-*--*-%d-*-*-*-*-%s",
		fam, bold, slant, siz, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "-*-%s-*-%s-*--*-%d-*-*-*-*-%s",
		fam, slant, siz, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "-*-%s-%s-*-*--*-%d-*-*-*-*-%s",
		fam, bold, siz, charset);
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	sprintf(s, "fixed");
	fs = XLoadQueryFont(XtDisplay($), s);
	if (fs) break;
	XtAppError(XtWidgetToApplicationContext($), "Failed to find Xfonts");
	break;
    default:
	assert(! "Cannot happen");
    }
    if (XGetFontProperty(fs, XA_SUPERSCRIPT_Y, &val))
	$fonts[family][style][size].sup = val;
    else
	$fonts[family][style][size].sup = fs->ascent/2;
    if (XGetFontProperty(fs, XA_SUBSCRIPT_Y, &val))
	$fonts[family][style][size].sub = val;
    else
	$fonts[family][style][size].sub = -fs->descent/2;
    if (XGetFontProperty(fs, XA_UNDERLINE_POSITION, &val))
	$fonts[family][style][size].ulpos = val;
    else
	$fonts[family][style][size].ulpos = 2;
    if (XGetFontProperty(fs, XA_UNDERLINE_THICKNESS, &val))
	$fonts[family][style][size].ulthick = val;
    else
	$fonts[family][style][size].ulthick = 1;
    if (XGetFontProperty(fs, XA_NORM_SPACE, &val))
	$fonts[family][style][size].space = val;
    else
	$fonts[family][style][size].space = XTextWidth(fs, " ", 1);
    if (XGetFontProperty(fs, XA_QUAD_WIDTH, &val))
	$fonts[family][style][size].em = val;
    else
	$fonts[family][style][size].em = $fonts[family][style][size].space;
    $fonts[family][style][size].fs = fs;
}

@proc interpret_style($, TextStyle style, int *fam, int *sty, int *siz, int *raise)
{
    int sup, sub;

    *fam = (style & FtFAMILYMASK) >> FtFAMILYSHIFT;
    *sty = (style & FtSTYLEMASK) >> FtSTYLESHIFT;
    *siz = (style & FtSIZEMASK) >> FtSIZESHIFT;
    sup = (style & FtSUPERMASK) >> FtSUPERSHIFT;
    sub = (style & FtSUBMASK) >> FtSUBSHIFT;
    if (! $fonts[*fam][*sty][*siz].fs) load_font($, *fam, *sty, *siz);
    *raise = 0;
    for (; sup != 0; sup--) {
	*raise += $fonts[*fam][*sty][*siz].sup;
	if (*siz != 0) *siz--;
	if (! $fonts[*fam][*sty][*siz].fs) load_font($, *fam, *sty, *siz);
    }
    for (; sub != 0; sub--) {
	*raise += $fonts[*fam][*sty][*siz].sup;
	if (*siz != 0) *siz--;
	if (! $fonts[*fam][*sty][*siz].fs) load_font($, *fam, *sty, *siz);
    }
}

@ |font_extents| selects the font, based on the flags in |style|,
loads it when it hasn't been loaded before, and computes the extents.

@proc font_extents($, TextStyle style, const String word, int len, int *h,
		  int *d, int *w, int *raise)
{
    int fam, sty, siz;
    interpret_style($, style, &fam, &sty, &siz, raise);
    *w = XTextWidth($fonts[fam][sty][siz].fs, word, len);
    *h = $fonts[fam][sty][siz].fs->ascent + *raise;
    *d = $fonts[fam][sty][siz].fs->descent - *raise;
}

@ |font_space| returns the normal size of a space and the normal line
height and depth for the font selected by |style|.

@proc font_space($, TextStyle style, int *h, int *d, int *w, int *raise)
{
    int fam, sty, siz;
    interpret_style($, style, &fam, &sty, &siz, raise);
    *w = $fonts[fam][sty][siz].space;
    *h = $fonts[fam][sty][siz].fs->ascent + *raise;
    *d = $fonts[fam][sty][siz].fs->descent - *raise;
}

@ |font_id| returns the font identifier of the font selected by
|style|.

@proc Font font_id($, TextStyle style)
{
    int fam, sty, siz, raise;
    interpret_style($, style, &fam, &sty, &siz, &raise);
    return $fonts[fam][sty][siz].fs->fid;
}

@ |dump| is for debugging only. It writes the whole chunk list to
|stderr|.

@proc dump_chunk(FtChunk f)
{
    switch (f->tp) {
    case FtWord:
	fprintf(stderr, "[FtWord, %d, %d, %d, %d + %d, \"%s\" [%s, %o]]\n",
		f->x, f->y, f->w, f->h, f->d,
		f->data ? f->data : "<none>",
		f->u.word.text, f->u.word.style);
	break;
    case FtHSpace:
	fprintf(stderr, "[FtHSpace, %d, %d, %d, %d + %d, \"%s\" [%d, %o]]\n",
		f->x, f->y, f->w, f->h, f->d,
		f->data ? f->data : "<none>",
		f->u.hspace.normwidth, f->u.hspace.style);
	break;
    case  FtInline:
	fprintf(stderr, "[FtInline, %d, %d, %d, %d + %d, \"%s\" [%ld, %o]]\n",
		f->x, f->y, f->w, f->h, f->d,
		f->data ? f->data : "<none>",
		f->u.inlined.widget, f->u.inlined.style);
	break;
    case FtVSpace:
	fprintf(stderr, "[FtVSpace, %d, %d, %d, %d + %d, \"%s\"]\n",
		f->x, f->y, f->w, f->h, f->d,
		f->data ? f->data : "<none>");
	break;
    case FtParShape:
	fprintf(stderr, "[FtParShape [%o, %d, %d, %d]]\n",
		f->u.parshape.mode, f->u.parshape.left,
		f->u.parshape.right, f->u.parshape.width);
	break;
    case FtHRule:
	fprintf(stderr, "[FtHRule, %d, %d, %d, %d + %d, \"%s\"]\n",
		f->x, f->y, f->w, f->h, f->d,
		f->data ? f->data : "<none>");
	break;
    default:
	fprintf(stderr, "???\n");
    }
}

@proc dump($)
{
    FtChunk f;

    for (f = $chunks; f; f = f->next) dump_chunk(f);
}


@IMPORTS

@incl <assert.h>
@incl <stdio.h>
@incl <Xm/ScrollBar.h>
@incl "line3d.bm"
@ incl "background.bm"
