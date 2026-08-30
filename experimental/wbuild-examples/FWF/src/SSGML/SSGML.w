# SSGML -- Simple SGML widget
# Bert Bos <bert@let.rug.nl>
# 24 Jan 1995

@class XfwfSimpleSGML (XfwfFormattedText) @file=SSGML

@ The SimpleSGML widget accepts text and parses it, using a
simplified SGML syntax. The resulting tokens are combined with
information from a style sheet and transformed into `chunks', which
are rendered (by the superclass).


@PUBLIC

@ The |style| resource contains a stylesheet. Stylesheet syntax is the
same as resource database syntax, except that the components of a
resource are not widgets, but SGML elements. The string pointed to by
|style| is not copied to private storage.

	@var String style = NULL

@ The document title. This resource is set by the widget itself, when
it parses SGML text that, according to the style sheet, belongs in the
title.

	@var String title = NULL



@EXPORTS

@ The |XfwfSSGMLData| struct is passed in the |data| field of the
|XfwfFTextCallbackStruct| that is passed as |call_data| argument in
the |activate| callback. It contains a URL and a flag to indicate if
the URL is of type `ismap'.

	@type XfwfSSGMLData = struct {
	    String url;
	    Bool ismap;
	}

@ The |XfwfAddText| function is a convenience function that calls the
|add_text| method of the widget. The text is parsed and rendered in
the widget. |XfwfAddText| may be called several times, each time
adding some text at the end. Because of this, the last part of the
text may not be displayed until the parser can recognize a complete
token. To signal the end of the data, call |XfwfAddText| with |nchars
= 0|.

@proc XfwfAddText($, const char *text, Cardinal nchars)
{
    if (! XtIsSubclass($, xfwfSimpleSGMLWidgetClass))
	XtAppError(XtWidgetToApplicationContext($),
		   "XfwfAddText called with incorrect widget");
    $add_text($, text, nchars);
}



@PRIVATE

@ |previous_is_blank| is used to prevent multiple spaces in the input
from producing too much whitespace (unless |obeyspaces| is selected).

	@var Bool previous_is_blank
	@var Bool start_of_par

@ |suppress_indent| is set by |pop_style| when the popped element
specified that the next paragraph should not begin with a paragraph
indent. |add_word|, which inserts indentation, resets this variable.

	@var Bool suppress_indent

@ The resource database containing the style rules.

	@var XrmDatabase styledb

@ The accumulated source text, and the length in characters. |nparsed|
is used by |get_token| and contains the length of the part of |$text|
that has been parsed.

	@var String text
	@var Cardinal nchars
	@var Cardinal nparsed

@ Attributes are stored in a linked list of name/value pairs. The
names are of length |NAMELEN|, the values of length |LITLEN|. These
are implementation dependent constants. Note that values may include
character entities, but both before and after expansion, the value may
not exceed |LITLEN|. |TAGLVL| is the maximum allowed nesting. It is
used when constructing nested resources.

	@def NAMELEN = 72
	@def LITLEN = 1024
	@def TAGLVL = 100
	@type Attrib = struct _Attrib {
	    struct _Attrib *next;
	    char name[NAMELEN+1];
	    char value[LITLEN+1];
	} *

@ The style stack is a linked list of |FormatInfo| records. The
|parmode| contains the justification and the track, but not the
`flush' (|CLEAR*|) flags, which are set in |flush|. Before calling
|$add_parshape| they have to be or'ed.

	@type StyType = enum {
	    StyleNo,				/* Not a stylesheet */
	    StyleMerge,				/* Add to stylesheet */
	    StyleReplace,			/* Completely replace */
	    StyleOverride			/* Add and override */
	}
	@type FormatInfo = struct _FormatInfo {
	    struct _FormatInfo *next;
	    XrmQuark tag;			/* Current element */
	    Bool verbatim;			/* Every space/nl counts? */
	    Bool nowrap;			/* Lines not broken? */
	    TextStyle textstyle;		/* Font selection */
	    Pixel fg, bg;			/* Colors */
	    Bool hide;				/* Don't display? */
	    Bool title;				/* Put contents in title? */
	    StyType stylesheettype;		/* Put cont. in stylesheet? */
	    String stylesheet;			/* Stylesheet, if any */
	    int data;				/* Ptr to extra info (URL) */
	    Bool ismap;				/* Pass (x,y) after URL? */
	    int inlinevmargin, inlinehmargin;	/* Extra pxls around inline */
	    int children;			/* # of children */
	    Bool break_after;			/* Vertical skip after elt? */
	    /* float postbreak;			/* how many lines to skip */
	    int postbreak;			/* Pixels to skip */
	    int parmode;			/* Float or normal par */
	    int leftmargin, rightmargin;	/* Indents */
	    int width;				/* Width, or -1 */
	    Bool hrule_after;			/* Add rule below element? */
	    /* float hrulebreak;		/* Between text and hrule */
	    int hrulebreak;			/* Pixels betw. text & rule */
	    float leading;			/* 1.0 is normal line height */
	    int flush;				/* Flush floating images */
	    char insafter[LITLEN+1];		/* Text to insert after elt. */
	    int parindent;			/* Indent first line of par */
	    Bool suppress_indent;		/* No indent for *next* elt. */
	    Bool save_content;			/* Store content in buffer */
	} *

@ The stack of elements and formatting info.

	@var FormatInfo stystack

@ The em and line height of the initial font.

	@var int default_em
	@var int default_baseline

@ Extra data that is passed in the |data| field of a text chunk is
allocated and kept in a separate array, since it must survive the
format stack. |extradata[0]| is always |NULL|. The array is lenghtened
25 entries at a time.

	@var XfwfSSGMLData **extradata
	@var int nrextradata
	@var int extradata_allocated

@ The lexical scanner returns tokens.

	@type Token = enum {
	    T_WORD, T_BLANK, T_NL, T_ENTITY, T_STARTTAG,
	    T_ENDTAG, T_NONE, T_EOD
	}

@ The |get_entity| method resolves entities and returns the character
set to be used for the replacement text.

	@def SYSTEM_DATA = 0
	@def ISO8859_1 = 1
	@def GENERAL_ENTITY = -1
	@def WWW_ICONS = -2
	@def SYMBOLS = -3

@ Some elements do not display their contents, but use them in other
ways. (TEXTAREA, OPTION). Such contents are stored in
|$contentbuffer|.

	@var String contentbuffer



@METHODS

@proc class_initialize
{
    XtAddConverter(XtRString, XtRJustify, XmuCvtStringToJustify, NULL, 0);
    init_style_quarks();
    init_entity_quarks();
}

@proc initialize
{
    newarray($extradata, 1);			/* Allocate 1 entry */
    $extradata[0] = NULL;			/* Entry 0 is always NULL */
    $nrextradata = 1;				/* One entry used so far */
    $extradata_allocated = 1;
    $title = NULL;
    $text = 0;
    $nchars = 0;
    $nparsed = 0;
    $contentbuffer = NULL;
    $styledb = XrmGetStringDatabase($style ? $style : "");
    init_stystack($);
}

@ |set_values| accepts a change in style sheet, but currently we don
not reformat the text. Maybe in a future version...

@proc set_values
{
    if ($style != $old$style) {
	XrmDestroyDatabase($styledb);
	$styledb = XrmGetStringDatabase($style ? $style : "");
    }
    return FALSE;
}

@proc destroy
{
    FormatInfo sty;
    int i;

    XtFree($title);
    XrmDestroyDatabase($styledb);
    while ($stystack) {
	sty = $stystack;
	$stystack = sty->next;
	dispose(sty);
    }
    XrmDestroyDatabase($styledb);
    for (i = 1; i < $nrextradata; i++) {
	XtFree($extradata[i]->url);
	XtFree((String) $extradata[i]);
    }
    XtFree((String) $extradata);
}

@ |pop_style| executes the actions required at the end of the current
element (i.e, inserting vertical space, resetting paragraph shapes)
and pops that element off the format stack.

@proc pop_style($)
{
    Cardinal save_nchars, save_nparsed;
    String save_text;
    FormatInfo sty;

    assert($stystack->next != NULL);
#if 0
#ifndef NDEBUG
    {
	XrmQuark nest[TAGLVL];
	int h;
	tag_nesting_to_quarklist($, nest);
	fprintf(stderr, "<<< ");
	for (h = 0; nest[h]; h++)
	    fprintf(stderr, ".%s", XrmQuarkToString(nest[h]));
	fprintf(stderr, "\n");
    }
#endif
#endif
    sty = $stystack;

    if (! sty->next->hide) {
	if (sty->insafter[0]) {			/* Insert text */
	    /*
	     *  Warning! This may cause infinite recursion, e.g:
	     *  *WBR.insertafter: <WBR>
	     */
	    get_parser_state($, &save_text, &save_nchars, &save_nparsed);
	    set_parser_state($, sty->insafter, strlen(sty->insafter), 0);
	    parse_text($, True);
	    set_parser_state($, save_text, save_nchars, save_nparsed);
	}
	if (sty->hrule_after) {			/* Insert rule */
	    $add_vspace($, sty->hrulebreak);
	    $add_hrule($);
	    $previous_is_blank = TRUE;
	    $start_of_par = TRUE;
	}
	if (sty->break_after) {			/* Insert vspace */
	    $add_vspace($, sty->postbreak);
	    $previous_is_blank = TRUE;
	    $start_of_par = TRUE;
	}
	if (sty->leftmargin != sty->next->leftmargin
	    || sty->rightmargin != sty->next->rightmargin
	    || sty->width != sty->next->width
	    || sty->parmode != sty->next->parmode) {
	    $add_parshape($, sty->next->leftmargin, sty->next->rightmargin,
			  sty->next->width, sty->next->parmode, sty->leading);
	    $previous_is_blank = TRUE;
	    $start_of_par = TRUE;
	}
	if (sty->suppress_indent) {
	    $suppress_indent = TRUE;
	}
    }
    if (sty->stylesheet != NULL) {
	/* At the end of a stylesheet specification */
	XrmDatabase db = XrmGetStringDatabase(sty->stylesheet);
	switch (sty->stylesheettype) {
	case StyleMerge:
	    XrmCombineDatabase(db, &$styledb, FALSE);
	    XrmDestroyDatabase(db);
	    break;
	case StyleOverride:
	    XrmMergeDatabases(db, &$styledb);
	    break;
	case StyleReplace:
	    XrmDestroyDatabase($styledb);
	    $styledb = NULL;
	    XrmMergeDatabases(db, &$styledb);
	    break;
	case StyleNo:
	default:
	    assert(! "Cannot happen");
	}
	XtFree(sty->stylesheet);
    }
    if ($contentbuffer != NULL && ! sty->next->save_content) {
	/* At the end of a save-content area */
	dispose($contentbuffer);
    }

    /* Now pop the style record */
    $stystack = sty->next;

    /* Free the occupied memory */
    dispose(sty);
}

@ |make_child_from_source| can be used for inline material that is
specified in an SGML attribute, such as an URL. In this widget, the
method does nothing, but subclasses may wish to override it.

The widget is where the contents of the document should appear. It is
a Composite widget, so it may get children. |src| is the address (URL)
of a document.

@proc make_child_from_source($, Widget w, const String src)
{
    /* Left to subclasses */
}

@ The |push_style| function reads the style resources for the given
element name and interprets them, creating a new style record on the
format stack and executing commands that are needed at the start of
the new element. The resources are in a resource database |$styledb|,
which is initialized when the widget is initialized. The style
resources are of the form |...*TAG.feature: value|, where |TAG| is an
element name in uppercase, feature is something like |size|,
|textcolor|, or |bold|. Context specific resources can also be
created, e.g., |*TAG1*TAG2.feature| for a feature of |TAG2| if it
occurs inside |TAG1|.

|MAXSTYLEATTR| is the maximum length of the search list that is
extracted from the resource database.

@def MAXSTYLEATTR = 100

@proc Bool scan_num(char *s, char *cp, int *np)
{
    if (sscanf(s, "%1s%u", cp, np) == 2 && (*cp == '-' || *cp == '+'))
	return TRUE;
    else {
	*cp = '\0';
	return sscanf(s, "%u", np) == 1;
    }
}

@proc push_style($, XrmQuark tag, Attrib attr)
{
    XrmHashTable list[MAXSTYLEATTR], idlist[MAXSTYLEATTR];
    XrmQuark nest[TAGLVL], id[2], repr;
    char s[LITLEN+1], buf[LITLEN+2];
    XrmValue v1, val;
    FormatInfo sty;
    Bool ok, has_id;
    float f;
    int h, n;
    char prefix;

    /* Push new style record */
    new(sty);
    sty->next = $stystack;
    $stystack = sty;
    sty->tag = tag;
    sty->children = 0;				/* Count sub-elements */
    sty->next->children++;			/* For counting labels */

    /* Get subset of style resource for current element */
    tag_nesting_to_quarklist($, nest);
#if 0
#ifndef NDEBUG
    fprintf(stderr, ">>> ");
    for (h = 0; nest[h]; h++)
	fprintf(stderr, ".%s", XrmQuarkToString(nest[h]));
    fprintf(stderr, "\n");
#endif
#endif
    ok = XrmQGetSearchList($styledb, nest, nest, list, MAXSTYLEATTR);

    /* If the element has an ID, construct another resource list */
    has_id = ok && get_property($, list, attr, QID, buf + 1);
    if (has_id) {
	buf[0] = '@';			/* Insert '@' in front */
	id[0] = XrmStringToQuark(buf);
	id[1] = 0;
	has_id = XrmQGetSearchList($styledb, id, id, idlist, MAXSTYLEATTR);
    }
    
    /* "*BODY.tile: ..." tile the window with a pixmap */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QTILE, buf))
	    || get_property($, list, attr, QTILE, buf))) {
	Pixmap tile;
	int status = XpmCreatePixmapFromBuffer
	    (XtDisplay($), RootWindowOfScreen(XtScreen($)), buf,
	     &tile, NULL, NULL);
	if (status == XpmColorError || status == XpmSuccess) {
	    XtVaSetValues($, XmNbackgroundPixmap, tile, NULL);
	    XFreePixmap(XtDisplay($), tile);
	}
    }

    /* "*A.target: !NAME" attribute that contains hyper target name */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QTARGET, buf))
	    || get_property($, list, attr, QTARGET, buf))) {
	/*
	 * s is an ID, for use as hyper target... not implemented
	 */
    }

    sty->textstyle = sty->next->textstyle;

    /* "*H1.size: +3" font size in <H1> increased by 3 */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QSIZE, buf))
	    || get_property($, list, attr, QSIZE, buf))
	&& scan_num(buf, &prefix, &n)) {
	int h;
	h = (sty->textstyle & FtSIZEMASK) >> FtSIZESHIFT;
	if (prefix == '+') h = max(0, min(6, h + n));
	else if (prefix == '-') h = max(0, min(6, h - n));
	else h = max(0, min(6, n));
	sty->textstyle &= ~FtSIZEMASK;
	sty->textstyle |= h << FtSIZESHIFT;
    }

    /* "*H1.family: alt" alternative font family in <H1> */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QFAMILY, buf))
	    || get_property($, list, attr, QFAMILY, buf))) {
	int h;
	if (XmuCompareISOLatin1(buf, "normal") == 0) h = FtFAMILY1;
	else if (XmuCompareISOLatin1(buf, "alt") == 0) h = FtFAMILY2;
	else if (XmuCompareISOLatin1(buf, "tt") == 0) h = FtFAMILY3;
	else if (XmuCompareISOLatin1(buf, "sym") == 0) h = FtFAMILY4;
	else h = sty->next->textstyle &= FtFAMILYMASK;
	sty->textstyle &= ~FtFAMILYMASK;
	sty->textstyle |= h;
    }

    /* familyname not supported */

    /* "*EM.emphasis: 1" use simple emphasis in <EM> */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QEMPHASIS, buf))
	    || get_property($, list, attr, QEMPHASIS, buf))
	&& sscanf(buf, "%u", &n) == 1) {
	sty->textstyle &= ~(FtBOLD | FtITALIC);
	if (n >= 3) sty->textstyle |= FtBOLD | FtITALIC;
	else if (n == 2) sty->textstyle |= FtBOLD;
	else if (n == 1) sty->textstyle |= FtITALIC;
    }

    /* "*I.slant: true" use italic font for <I> */
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QSLANT, buf))
	    || get_property($, list, attr, QSLANT, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRBool, &v1)) {
	if (*((Bool *) v1.addr)) sty->textstyle |= FtITALIC;
	else sty->textstyle &= ~FtITALIC;
    }

    /* "*B.bold: true" use bold font for <B> */
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QBOLD, buf))
	    || get_property($, list, attr, QBOLD, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRBool, &v1)) {
	if (*((Bool *) v1.addr)) sty->textstyle |= FtBOLD;
	else sty->textstyle &= ~FtBOLD;
    }

    /* "*U.underscore: 1" underline <U> with a single line */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QUSCORE, buf))
	    || get_property($, list, attr, QUSCORE, buf))
	&& sscanf(buf, "%u", &n) == 1) {
	if (n >= 3) sty->textstyle |= FtUNDERSCORE2 | FtUNDERSCORE;
	else if (n == 2) sty->textstyle |= FtUNDERSCORE2;
	else if (n == 1) sty->textstyle |= FtUNDERSCORE;
	else sty->textstyle &= ~(FtUNDERSCORE2 | FtUNDERSCORE);
    }

    /* "*S.strikeout: true" put a line through <S> */
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QSTRIKE, buf))
	    || get_property($, list, attr, QSTRIKE, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRBool, &v1)) {
	if (*((Bool *) v1.addr)) sty->textstyle |= FtSTRIKE;
	else sty->textstyle &= ~FtSTRIKE;
    }

    /* "*SUB*SUB.raise: -2" lower text in <SUP>..<SUB> 2 level2 */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QRAISE, buf))
	    || get_property($, list, attr, QRAISE, buf))
	&& sscanf(buf, "%i", &n) == 1) {
	sty->textstyle &= ~FtSUPERMASK;
	sty->textstyle &= ~FtSUBMASK;
	if (n < -3) sty->textstyle |= 3 << FtSUBSHIFT;
	else if (n < 0) sty->textstyle |= (-n) << FtSUBSHIFT;
	else if (n == 0) ;
	else if (n <= 3) sty->textstyle |= n << FtSUPERSHIFT;
	else sty->textstyle |= 3 << FtSUPERSHIFT;
    }

    /* "*UL.postbreak: 1.0" skip one line after a <UL> */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QPOSTBREAK, buf))
	    || get_property($, list, attr, QPOSTBREAK, buf))
	&& sscanf(buf, "%f", &f) == 1) {
	sty->break_after = TRUE;
	sty->postbreak = round(f * $default_baseline);
    } else {
	sty->break_after = FALSE;
    }

    /* "*UL.ruleafter: 0.5" a hor. rule after <UL>, separated by 0.5 line */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QRULEAFTER, buf))
	    || get_property($, list, attr, QRULEAFTER, buf))
	&& sscanf(buf, "%f", &f) == 1) {
	sty->hrule_after = TRUE;
	sty->hrulebreak = round(f * $default_baseline);
    } else {
	sty->hrule_after = FALSE;
    }

    /* "*IMG.ismap: true" a hyperlink should be followed by x,y */
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QISMAP, buf))
	    || get_property($, list, attr, QISMAP, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRBool, &v1)) {
	sty->ismap = *((Bool *) v1.addr);
    } else {
	sty->ismap = sty->next->ismap;
    }

    /* "*A.anchor: !HREF" HREF-attrib in <A> contains hyperlink */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QANCHOR, buf))
	    || get_property($, list, attr, QANCHOR, buf))) {
	sty->data = alloc_extradata($, buf, sty->ismap);
    } else {
	sty->data = sty->next->data;
    }

    /* Check and update "ismap" if it changed */
    if (sty->data && $extradata[sty->data]->ismap != sty->ismap) {
	sty->data = alloc_extradata($, $extradata[sty->data]->url, sty->ismap);
    }

    /* "*A.textcolor: blue" foreground is blue inside <A> */
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QTEXTCOLOR, buf))
	    || get_property($, list, attr, QTEXTCOLOR, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRPixel, &v1)) {
	sty->fg = *((Pixel *) v1.addr);
    } else {
	sty->fg = sty->next->fg;
    }

    /* "*A.textbackground: yellow" background is yellow inside <A> */
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    sty->bg = sty->next->bg;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QTEXTBG, buf))
	    || get_property($, list, attr, QTEXTBG, buf))) {
	if (XmuCompareISOLatin1(buf, "transparent") == 0)
	    sty->bg = TRANSPARENT;
	else if (XtConvertAndStore($, XtRString, &val, XtRPixel, &v1))
	    sty->bg = *((Pixel *) v1.addr);
    }

    /* "*TITLE.title: True" contents of <TITLE> constitute the title */
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QTITLE, buf))
	    || get_property($, list, attr, QTITLE, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRBool, &v1)) {
	sty->title = *((Bool *) v1.addr);
    } else {
	sty->title = sty->next->title;
    }

    /* "*STYLE.stylesheet: merge" contents of <STYLE> are a stylesheet */
    sty->stylesheet = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QSTYLESHEET, buf))
	    || get_property($, list, attr, QSTYLESHEET, buf))) {
	if (XmuCompareISOLatin1(buf, "merge") == 0)
	    sty->stylesheettype = StyleMerge;
	else if (XmuCompareISOLatin1(buf, "replace") == 0)
	    sty->stylesheettype = StyleReplace;
	else if (XmuCompareISOLatin1(buf, "override") == 0)
	    sty->stylesheettype = StyleOverride;
	else
	    sty->stylesheettype = StyleNo;
    } else {
	sty->stylesheettype = sty->next->stylesheettype;
    }

    /* "*PRE.obeyspaces: True" every space counts in <PRE> */
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QOBEYSP, buf))
	    || get_property($, list, attr, QOBEYSP, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRBool, &v1)) {
	sty->verbatim = *((Bool *) v1.addr);
    } else {
	sty->verbatim = sty->next->verbatim;
    }

    /* "*NOBR.nowrap: True" don't break lines inside <NOBR> */
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QNOWRAP, buf))
	    || get_property($, list, attr, QNOWRAP, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRBool, &v1)) {
	sty->nowrap = *((Bool *) v1.addr);
    } else {
	sty->nowrap = sty->next->nowrap;
    }

    /* "*HEAD.hide: True" contents of <HEAD> are not displayed */
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QHIDE, buf))
	    || get_property($, list, attr, QHIDE, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRBool, &v1)) {
	sty->hide = *((Bool *) v1.addr);
    } else {
	sty->hide = sty->next->hide;
    }

    /* "*P.parindent: 2.0" indent first line of paragraph 2 ems */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QPARINDENT, buf))
	    || get_property($, list, attr, QPARINDENT, buf))
	&& sscanf(buf, "%f", &f) == 1) {
	sty->parindent = round(f * $default_em);
    } else {
	sty->parindent = sty->next->parindent;
    }

    /* "*H1.noindent: true" don't indent 1st para *after* H1 */
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QNOINDENT, buf))
	    || get_property($, list, attr, QNOINDENT, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRBool, &v1)) {
	sty->suppress_indent = *((Bool *) v1.addr);
    } else {
	sty->suppress_indent = sty->next->suppress_indent;
    }

    /*
     * Paragraph modes
     */
    sty->parmode = sty->next->parmode;

    /* "*H2.justify: left" left-align text in <H1> */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QJUSTIFY, buf))
	    || get_property($, list, attr, QJUSTIFY, buf))) {
	if (XmuCompareISOLatin1(buf, "left") == 0)
	    sty->parmode = (sty->parmode & ~FtJUSTIFYMASK) | FtJUSTIFYLEFT;
	else if (XmuCompareISOLatin1(buf, "right") == 0)
	    sty->parmode = (sty->parmode & ~FtJUSTIFYMASK) | FtJUSTIFYRIGHT;
	else if (XmuCompareISOLatin1(buf, "center") == 0)
	    sty->parmode = (sty->parmode & ~FtJUSTIFYMASK) | FtJUSTIFYCENTER;
	else if (XmuCompareISOLatin1(buf, "full") == 0)
	    sty->parmode = (sty->parmode & ~FtJUSTIFYMASK) | FtJUSTIFYBOTH;
	/* else unrecognized keyword */
    }

    /* "*FIG.track: !ALIGN" attribute that contains position */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QTRACK, buf))
	    || get_property($, list, attr, QTRACK, buf))) {
	if (XmuCompareISOLatin1(buf, "left") == 0)
	    sty->parmode = (sty->parmode & ~FtPOSITIONMASK) | FtLEFTFLOAT;
	else if (XmuCompareISOLatin1(buf, "right") == 0)
	    sty->parmode = (sty->parmode & ~FtPOSITIONMASK) | FtRIGHTFLOAT;
	else if (XmuCompareISOLatin1(buf, "right") == 0)
	    sty->parmode = (sty->parmode & ~FtPOSITIONMASK) | FtNORMALPARA;
	/* else unrecognized keyword */
    }

    /* "*LI.leftindent: 3.0" indent text in <LI> by 3em */
    sty->leftmargin = sty->next->leftmargin;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QLINDENT, buf))
	    || get_property($, list, attr, QLINDENT, buf))
	&& sscanf(buf, "%f", &f) == 1) {
	sty->leftmargin += round(f * $default_em);
    }

    /* "*QUOTE.rightindent: 3.0" right-indent text in <QUOTE> by 3em */
    sty->rightmargin = sty->next->rightmargin;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QRINDENT, buf))
	    || get_property($, list, attr, QRINDENT, buf))
	&& sscanf(buf, "%f", &f) == 1) {
	sty->rightmargin += round(f * $default_em);
    }

    /* "*NOTE.textwidth: 30.0" NOTE paragraph is 30 em wide */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QTEXTWIDTH, buf))
	    || get_property($, list, attr, QTEXTWIDTH, buf))
	&& sscanf(buf, "%f", &f) == 1) {
	sty->width = round(f * $default_em);
    } else {
	sty->width = sty->next->width;
    }

    /* "*QUOTE.leading: 2.0" double spacing <QUOTE> */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QLEADING, buf))
	    || get_property($, list, attr, QLEADING, buf))
	&& sscanf(buf, "%f", &f) == 1) {
	sty->leading = f;
    } else {
	sty->leading = sty->next->leading;
    }

    /* "*BR.flush: !CLEAR" move to below floats if CLEAR attr. says so */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QFLUSH, buf))
	    || get_property($, list, attr, QFLUSH, buf))) {
	if (XmuCompareISOLatin1(buf, "left") == 0)
	    sty->flush = FtCLEARLEFT;
	else if (XmuCompareISOLatin1(buf, "right") == 0)
	    sty->flush = FtCLEARRIGHT;
	else if (XmuCompareISOLatin1(buf, "full") == 0)
	    sty->flush = FtCLEARLEFT | FtCLEARRIGHT;
	else
	    sty->flush = FtNOCLEAR;
    } else {
	sty->flush = FtNOCLEAR;			/* Not inherited */
    }

    /*
     * Execute parshape, if anything changed.
     * We could check sty->hide, instead of sty->next->hide,
     * but then we would need to add the parshape after all when
     * there is an `inline'
     */
    if (! sty->next->hide
	&& (sty->leftmargin != sty->next->leftmargin
	    || sty->rightmargin != sty->next->rightmargin
	    || sty->width != sty->next->width
	    || sty->parmode != sty->next->parmode
	    || sty->flush != FtNOCLEAR
	    || sty->leading != sty->next->leading)) {
	$add_parshape($, sty->leftmargin, sty->rightmargin, sty->width,
		      sty->parmode | sty->flush, sty->leading);
	$start_of_par = TRUE;
	$previous_is_blank = TRUE;
    }

    /* "*IMG.vmargin: 0.5" half a line above and below the object */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QVMARGIN, buf))
	    || get_property($, list, attr, QVMARGIN, buf))
	&& sscanf(buf, "%f", &f) == 1) {
	sty->inlinevmargin = round(f * $default_baseline);
    } else {
	sty->inlinevmargin = sty->next->inlinevmargin;
    }

    /* "*IMG.hmargin: 0.5" half an em left and right of the object */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QHMARGIN, buf))
	    || get_property($, list, attr, QHMARGIN, buf))
	&& sscanf(buf, "%f", &f) == 1) {
	sty->inlinehmargin = round(f * $default_em);
    } else {
	sty->inlinehmargin = sty->next->inlinehmargin;
    }

    /* "*IMG.inline: !SRC" <IMG> has an image in attribute "SRC" */
    if (ok && ! sty->next->hide
	&& ((has_id && get_property($, idlist, attr, QINLINE, buf))
	    || get_property($, list, attr, QINLINE, buf))) {
	Widget w;
	char buf1[LITLEN+2];
	int ht, wd, dp = 0, align = FtALIGNTOP;

	if ($stystack->data)
	    w = XtVaCreateManagedWidget
		("inline", xmManagerWidgetClass, $, 
		 XtNborderWidth, 2, XtNborderColor, sty->fg, NULL);
	else
	    w = XtVaCreateManagedWidget
		("inline", xmManagerWidgetClass, $,
		 XtNborderWidth, 0, NULL);

	/* "*IMG.valign: !ALIGN" <IMG> has a vert. alignment attrib. */
	if (ok
	    && ((has_id && get_property($, idlist, attr, QVALIGN, buf1))
		|| get_property($, list, attr, QVALIGN, buf1))) {
	    if (XmuCompareISOLatin1(buf1, "bottom") == 0)
		align = FtALIGNBOTTOM;
	    else if (XmuCompareISOLatin1(buf1, "middle") == 0)
		align = FtALIGNMIDDLE;
	}

	/* "*IMG.depth: !DEPTH" <IMG> has a depth attrib. */
	if (ok
	    && ((has_id && get_property($, idlist, attr, QDEPTH, buf1))
		|| get_property($, list, attr, QDEPTH, buf1))
	    && sscanf(buf1, "%i", &n) == 1) {
	    align = FtALIGNDEPTH;
	    dp = n;
	}

	/* "*IMG.width: 20" <IMG> has fixed width */
	if (ok
	    && ((has_id && get_property($, idlist, attr, QWIDTH, buf1))
		|| get_property($, list, attr, QWIDTH, buf1))
	    && sscanf(buf1, "%i", &n) == 1) {
	    align |= FtALIGNFIXEDWIDTH;
	    wd = n;
	}

	/* "*IMG.height: 20" <IMG> has fixed height */
	if (ok
	    && ((has_id && get_property($, idlist, attr, QHEIGHT, buf1))
		|| get_property($, list, attr, QHEIGHT, buf1))
	    && sscanf(buf1, "%i", &n) == 1) {
	    align |= FtALIGNFIXEDHEIGHT;
	    ht = n - dp;
	}

	$add_inline($, w, sty->textstyle | align, wd, ht, dp,
		    sty->inlinehmargin, sty->inlinevmargin,
		    $extradata[sty->data]);
	$start_of_par = FALSE;
	$previous_is_blank = FALSE;
	$make_child_from_source($, w, buf);
    }

    /* "*OL.prebreak: 0.5" skip half a line before an <OL> */
    if (ok && ! sty->next->hide
	&& ((has_id && get_property($, idlist, attr, QPREBREAK, buf))
	    || get_property($, list, attr, QPREBREAK, buf))
	&& sscanf(buf, "%f", &f) == 1) {
        /* `prebreak' is excecuted immediately */
	$add_vspace($, round(f * $default_baseline));
	$start_of_par = TRUE;
	$previous_is_blank = TRUE;
    }

    /* "*OL.rulebefore: 0.5" hor. rule before <OL>, followed by 0.5 line */
    if (ok && ! sty->next->hide
	&& ((has_id && get_property($, idlist, attr, QRULEBEFORE, buf))
	    || get_property($, list, attr, QRULEBEFORE, buf))
	&& sscanf(buf, "%f", &f) == 1) {
        /* `rulebefore' is excecuted immediately */
	$add_hrule($);
	$add_vspace($, round(f * $default_baseline));
	$start_of_par = TRUE;
	$previous_is_blank = TRUE;
    }

    /* "*UL.LI.label: bullet" put a label in front of <LI> */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QLABEL, buf))
	    || get_property($, list, attr, QLABEL, buf))) {
	char t[10];
	Cardinal len = 1; int w, h, d, raise, sp;
	TextStyle style = sty->textstyle | FtBOLD;
	if (strcmp(buf, "a") == 0)
	    t[0] = 'a' - 1 + sty->next->children;
	else if (strcmp(buf, "A") == 0)
	    t[0] = 'A' - 1 + sty->next->children;
	else if (strcmp(buf, "1") == 0)
	    len = sprintf(t, "%i", sty->next->children);
	else {
	    t[0] = '\267';			/* Bullet */
	    style = (style & ~(FtFAMILYMASK|FtSTYLEMASK)) | FtFAMILY5;
	}
	/* Others not implemented yet... */
	sp = $get_space_of_textstyle($, style);
	$get_word_extent($, t, len, style, &h, &d, &w, &raise);
	w = 1000 * w/sp + 2000;			/* Label + 2 spaces */
	$add_hspace($, -w, FALSE, FALSE, sty->fg, sty->bg, style,
		    $extradata[sty->data]);
	$add_word($, t, len, sty->fg, sty->bg, style | FtHIDEWIDTH,
		  $extradata[sty->data]);
	$add_hspace($, w, FALSE, FALSE, sty->fg, sty->bg, style,
		    $extradata[sty->data]);
	$start_of_par = FALSE;
	$previous_is_blank = TRUE;
    }

    /* "*WBR.insertbefore: &sbsp;" insert text at start of <WBR> */
    if (ok
	&& (( has_id && get_property($, idlist, attr, QINSBEFORE, buf))
	    || get_property($, list, attr, QINSBEFORE, buf))) {
	String save_text;
	Cardinal save_nchars, save_nparsed;
	/*
	 *  Warning! This may cause infinite recursion, e.g:
	 *  *WBR.insertbefore: <WBR>
	 */
	get_parser_state($, &save_text, &save_nchars, &save_nparsed);
	set_parser_state($, buf, strlen(buf), 0);
	parse_text($, True);
	set_parser_state($, save_text, save_nchars, save_nparsed);
    }

    /* "*Q.insertafter: '" insert text after </Q> */
    if (ok
	&& (( has_id && get_property($, idlist, attr, QINSAFTER, buf))
	    || get_property($, list, attr, QINSAFTER, buf))) {
	strcpy(sty->insafter, buf);
    } else {
	sty->insafter[0] = '\0';		/* Not inherited */
    }

    /* "*OPTION.savecontent: TRUE" store content in buffer */
    if (ok
	&& ((has_id && get_property($, idlist, attr, QSAVECONT, buf))
	    || get_property($, list, attr, QSAVECONT, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRBool, &v1)) {
	sty->save_content = *((Bool *) v1.addr);
    } else {
	sty->save_content = sty->next->save_content;
    }

    /* "*BR.empty: True" <BR> is an empty tag */
    assert(sizeof(buf) > LITLEN);
    val.size = LITLEN + 1; val.addr = buf; v1.addr = NULL;
    if (ok
	&& ((has_id && get_property($, idlist, attr, QEMPTY, buf))
	    || get_property($, list, attr, QEMPTY, buf))
	&& XtConvertAndStore($, XtRString, &val, XtRBool, &v1)) {
	if (*((Bool *) v1.addr)) $pop_style($);
    }
}

@ |get_entity| translates an entity name to a replacement text. In
this widget, it only translates some of the ISO standard character
entities. The returned values are: |val| = replacement text, and
|charset| = character set (currently only ISO8859_1, SYMBOLS and
WWW_ICONS). The function returns |FALSE| if no replacement could be
found, in that case |val| contains the entity name itself and
|charset| is |ISO8859_1|.

@proc Bool get_entity($, const String name, Cardinal namelen, char val[], Cardinal maxlen, int *charset)
{
    char s[NAMELEN+1];
    XrmQuark q;
    int i;

    s[0] = '\0';
    strncat(s, name, min(NAMELEN, namelen));
    q = XrmStringToQuark(s);
    for (i = 0; i < XtNumber(entities); i++)
	if (entities[i].q == q) {
	    val[0] = '\0';
	    strncat(val, entities[i].value, maxlen);
	    *charset = entities[i].charset;
	    return TRUE;
	}
    /* Not found, return name itself and FALSE */
    val[0] = '\0';
    strncat(val, "&", maxlen);
    strncat(val, name, min(namelen, maxlen - 1));
    strncat(val, ";", maxlen - namelen - 1);
    *charset = ISO8859_1;
    return FALSE;
}

@ |process_entity| is a method that interprets the (character) entity
in |data| and inserts the appropriate character or symbol. It calls
the |get_entity| method for resolving the entity.

Entities with |charset = SYSTEM_DATA| are treated specially.

@proc process_entity($, const String data, Cardinal len)
{
    char s[LITLEN+1];
    int charset, style;
    int n;

    if ($stystack->tag != PCDATA_TAG)		/* Start #PCDATA */
	$add_starttag($, "#PCDATA", 7, NULL);

    if (data[0] != '#') {
	(void) $get_entity($, data, len, s, sizeof(s), &charset);
    } else {					/* Numerical entity */
	s[0] = '\0'; strncat(s, data + 1, len - 1);
	if (sscanf(strncat(s, data + 1, len - 1), "%d", &n) != 1) {
	    strcpy(s, "&");			/* Number too large? */
	    strncat(s, data, min(sizeof(s) - 3, len));
	    strcat(s, ";");
	} else {
	    s[0] = (n ? n : 0xff) & 0xff;	/* Number -> char */
	    s[1] = '\0';
	}
	charset = ISO8859_1;
    }
    style = $stystack->textstyle;
    if (charset == SYSTEM_DATA) {
	if (strcmp(s, "sbsp") == 0) {		/* Soft break space */
	    if (! $stystack->title && $stystack->stylesheettype == StyleNo
		&& ! $stystack->hide)
		$add_hspace($, 0, FALSE, TRUE, $stystack->fg, $stystack->bg,
			    style, $extradata[$stystack->data]);
	} else if (strcmp(s, "nbsp") == 0) {	/* No break (required) space */
	    if (! $stystack->title && $stystack->stylesheettype == StyleNo
		&& ! $stystack->hide) {
		$add_hspace($, 1000, TRUE, FALSE, $stystack->fg, $stystack->bg,
			    style, $extradata[$stystack->data]);
		CHK_SAVE_CONTENT($stystack, " ", 1);
	    } else
		add_spaces($, " ", 1);
	} /* else: not handled yet */
	$start_of_par = FALSE;
    } else {
	if (charset == WWW_ICONS)
	    style = (style & ~FtFAMILYMASK) | FtFAMILY4;
	else if (charset == SYMBOLS)
	    style = (style & ~FtFAMILYMASK) | FtFAMILY5;
	add_word($, s, strlen(s), $stystack->fg, $stystack->bg, style,
		 $stystack->parindent, $extradata[$stystack->data]);
    }
}

@ |add_endtag| pops the style stack until the tag in |name| comes up.
It processes the formatting actions required at the end of that
element and then pops it off the stack also. If |namelen| is 0, just
the topmost element is popped.

@proc add_endtag($, const String name, Cardinal namelen)
{
    char t[NAMELEN+1] = "";
    XrmQuark tag;
    FormatInfo sty;

    if (namelen == 0) {				/* Null endtag */
	if ($stystack->next) $pop_style($);	/* Just pop top element */
	return;
    }
    tag = XrmStringToQuark(upper(strncat(t, name, min(NAMELEN, namelen))));
    /* Check if the named tag is indeed open */
    for (sty = $stystack; sty->next; sty = sty->next)
	if (sty->tag == tag) {
	    /* It is open, pop elements up to and including tag */
	    while ($stystack->tag != tag) $pop_style($);
	    $pop_style($);
	    break;
	}
}

@ |add_starttag| looks up the tag in the style sheet and pushes the
new style on the style stack, while executing the commands required at
the start of this style, such as paragraph shapes and vertical
spaces. Subclasses may wish to extend this method by providing
(heuristic) rules for inserting omitted tags.

@proc add_starttag($, const String name, Cardinal namelen, Attrib attr)
{
    char t[NAMELEN+1] = "";

    upper(strncat(t, name, min(NAMELEN, namelen)));
    $push_style($, XrmStringToQuark(t), attr);
}

@proc add_text($, const char *text, Cardinal nchars)
{
    if (nchars != 0) {				/* Add to buffered text */
	renewarray($text, $nchars + nchars + 1);
	memcpy($text + $nchars, text, nchars);
	$nchars += nchars;
	$text[$nchars] = '\0';
    }
    parse_text($, (nchars == 0));		/* Parse tokens */
    if (nchars == 0)
	add_eod($);				/* End of input */
    else {
	$expose($, NULL, NULL);			/* Make sure output appears */
	$reformat_scrollbar($);
    }
}

@ |get_attrib| iterates over the list of attributes until it finds one
with the given name (case-insensitive). It expands character entities
(by calling the |get_entity| method) and returns the result in
|value|. When no attribute is found, it returns |FALSE|.

@proc Bool get_attrib($, Attrib attr, const String name, char value[], Cardinal maxlen)
{
    int i, j, k, dum;
    char s[LITLEN+1];

    if (attr == NULL)
	return FALSE;				/* Not found */
    else if (XmuCompareISOLatin1(attr->name, name) != 0)
	return get_attrib($, attr->next, name, value, maxlen);
    else {
	i = j = 0;
	while (j < maxlen - 1 && attr->value[i]) {
	    if (attr->value[i] != '&')
		value[j++] = attr->value[i++];
	    else {
		i++;				/* Skip '&' */
		for (k = 0; isnmchar(attr->value[i+k]); k++) ;
		if ($get_entity($, attr->value + i, k, s, sizeof(s), &dum)) {
		    i += k;
		    if (attr->value[i] == ';') i++;
		    for (k = 0; s[k] != '\0' && j < maxlen - 1; k++)
			value[j++] = s[k];
		} else {			/* Unknown entity, copy it */
		    value[j++] = '&';
		    for (; k > 0 && j < maxlen - 1; k--)
			value[j++] = attr->value[i++];
		}
	    }
	}
	value[j] = '\0';
	return TRUE;
    }
}



@TRANSLATIONS

@ The same translations as in the superclass.

@trans <Btn1Down>,<Btn1Up>: activate()
@trans <Motion>: motion()



@UTILITIES

@ Useful macros

@def new(p) = ((p) = (XtPointer) XtMalloc(sizeof(*(p))))
@def dispose(p) = (XtFree((String)p), (p) = NULL)
@def newarray(p, n) = ((p) = (XtPointer) XtMalloc((n)*sizeof(*(p))))
@def renewarray(p, n) = ((p)=(XtPointer)XtRealloc((String)p,(n)*sizeof(*(p))))
@def min(a, b) = ((a) < (b) ? (a) : (b))
@def max(a, b) = ((a) > (b) ? (a) : (b))
@def round(f) = ((int) ((f) + 0.5))
@def STRLEN(s) = ((s) ? strlen(s) : 0)

@ The following quarks contain the names of style resources. They are
initialized by |init_style_quarks|, which is called by
|class_initialize|.

	@var XrmQuark QANCHOR		@var XrmQuark QBOLD
	@var XrmQuark QDEPTH		@var XrmQuark QEMPHASIS
	@var XrmQuark QEMPTY		@var XrmQuark QFAMILY
	@var XrmQuark QFLUSH		@var XrmQuark QHEIGHT
	@var XrmQuark QHIDE		@var XrmQuark QHMARGIN
	@var XrmQuark QID		@var XrmQuark QINLINE
	@var XrmQuark QINSAFTER		@var XrmQuark QINSBEFORE
	@var XrmQuark QISMAP		@var XrmQuark QJUSTIFY
	@var XrmQuark QLABEL		@var XrmQuark QLEADING
	@var XrmQuark QLINDENT		@var XrmQuark QNOINDENT
	@var XrmQuark QNOWRAP		@var XrmQuark QOBEYSP
	@var XrmQuark QPARINDENT	@var XrmQuark QPOSTBREAK
	@var XrmQuark QPREBREAK		@var XrmQuark QRAISE
	@var XrmQuark QRINDENT		@var XrmQuark QRULEAFTER
	@var XrmQuark QRULEBEFORE	@var XrmQuark QSAVECONT
	@var XrmQuark QSIZE
	@var XrmQuark QSLANT		@var XrmQuark QSTRIKE
	@var XrmQuark QSTYLESHEET	@var XrmQuark QTARGET
	@var XrmQuark QTEXTBG		@var XrmQuark QTEXTCOLOR
	@var XrmQuark QTEXTWIDTH	@var XrmQuark QTILE
	@var XrmQuark QTITLE		@var XrmQuark QTRACK
	@var XrmQuark QUSCORE		@var XrmQuark QVALIGN
	@var XrmQuark QVMARGIN		@var XrmQuark QWIDTH

@ The value of |PCDATA_TAG| is |"#PCDATA"|.

	@var XrmQuark PCDATA_TAG



@proc init_style_quarks()
{
    QANCHOR = XrmStringToQuark("anchor");
    QBOLD = XrmStringToQuark("bold");
    QDEPTH = XrmStringToQuark("depth");
    QEMPHASIS = XrmStringToQuark("emphasis");
    QEMPTY = XrmStringToQuark("empty");
    QFAMILY = XrmStringToQuark("family");
    QFLUSH = XrmStringToQuark("flush");
    QHEIGHT = XrmStringToQuark("height");
    QHIDE = XrmStringToQuark("hide");
    QHMARGIN = XrmStringToQuark("hmargin");
    QID = XrmStringToQuark("id");
    QINLINE = XrmStringToQuark("inline");
    QINSAFTER = XrmStringToQuark("insertafter");
    QINSBEFORE = XrmStringToQuark("insertbefore");
    QISMAP = XrmStringToQuark("ismap");
    QJUSTIFY = XrmStringToQuark("justify");
    QLABEL = XrmStringToQuark("label");
    QLEADING = XrmStringToQuark("leading");
    QLINDENT = XrmStringToQuark("leftindent");
    QNOINDENT = XrmStringToQuark("noindent");
    QNOWRAP = XrmStringToQuark("nowrap");
    QOBEYSP = XrmStringToQuark("obeyspaces");
    QPARINDENT = XrmStringToQuark("parindent");
    QPOSTBREAK = XrmStringToQuark("postbreak");
    QPREBREAK = XrmStringToQuark("prebreak");
    QRAISE = XrmStringToQuark("raise");
    QRINDENT = XrmStringToQuark("rightindent");
    QRULEAFTER = XrmStringToQuark("ruleafter");
    QRULEBEFORE = XrmStringToQuark("rulebefore");
    QSAVECONT = XrmStringToQuark("savecontent");
    QSIZE = XrmStringToQuark("size");
    QSLANT = XrmStringToQuark("slant");
    QSTRIKE = XrmStringToQuark("strikeout");
    QSTYLESHEET = XrmStringToQuark("stylesheet");
    QTARGET = XrmStringToQuark("target");
    QTEXTBG = XrmStringToQuark("textbackground");
    QTEXTCOLOR = XrmStringToQuark("textcolor");
    QTEXTWIDTH = XrmStringToQuark("textwidth");
    QTILE = XrmStringToQuark("tile");
    QTITLE = XrmStringToQuark("title");
    QTRACK = XrmStringToQuark("track");
    QUSCORE = XrmStringToQuark("underscore");
    QVALIGN = XrmStringToQuark("valign");
    QVMARGIN = XrmStringToQuark("vmargin");
    QWIDTH = XrmStringToQuark("width");

    PCDATA_TAG = XrmStringToQuark("#PCDATA");
}

@ The |get_entity| method uses the following table to find the
replacement text for (character) entities. The quarks are initialized
by |init_entity_quarks|, which is called by |class_initialize|.

	@type EntityRec = struct {
	    XrmQuark q;				/* The name as a quark */
	    String name;			/* The name, without "&;" */
	    String value;			/* Replacement text */
	    int charset;			/* Charset or SYSTEM_DATA */
	}
	
	@var EntityRec entities[] = {
	    /* ISOlat1 */
	    {0, "aacute",	"\341",	ISO8859_1},
	    {0, "Aacute",	"\301",	ISO8859_1},
	    {0, "acirc",	"\342",	ISO8859_1},
	    {0, "Acirc",	"\302",	ISO8859_1},
	    {0, "agrave",	"\340",	ISO8859_1},
	    {0, "Agrave",	"\300",	ISO8859_1},
	    {0, "aring",	"\345",	ISO8859_1},
	    {0, "Aring",	"\300",	ISO8859_1},
	    {0, "atilde",	"\343",	ISO8859_1},
	    {0, "Atilde",	"\303",	ISO8859_1},
	    {0, "auml",	 	"\344",	ISO8859_1},
	    {0, "Auml",	 	"\304",	ISO8859_1},
	    {0, "aelig",	"\346",	ISO8859_1},
	    {0, "AElig",	"\306",	ISO8859_1},
	    {0, "ccedil",	"\347",	ISO8859_1},
	    {0, "Ccedil",	"\307",	ISO8859_1},
	    {0, "eth",		"\360",	ISO8859_1},
	    {0, "ETH",		"\320",	ISO8859_1},
	    {0, "eacute",	"\351",	ISO8859_1},
	    {0, "Eacute",	"\311",	ISO8859_1},
	    {0, "ecirc",	"\352",	ISO8859_1},
	    {0, "Ecirc",	"\312",	ISO8859_1},
	    {0, "egrave",	"\350",	ISO8859_1},
	    {0, "Egrave",	"\310",	ISO8859_1},
	    {0, "euml",		"\353",	ISO8859_1},
	    {0, "Euml",		"\313",	ISO8859_1},
	    {0, "iacute",	"\355",	ISO8859_1},
	    {0, "Iacute",	"\315",	ISO8859_1},
	    {0, "icirc",	"\356",	ISO8859_1},
	    {0, "Icirc",	"\316",	ISO8859_1},
	    {0, "igrave",	"\354",	ISO8859_1},
	    {0, "Igrave",	"\314",	ISO8859_1},
	    {0, "iuml",		"\357",	ISO8859_1},
	    {0, "Iuml",		"\317",	ISO8859_1},
	    {0, "ntilde",	"\361",	ISO8859_1},
	    {0, "Ntilde",	"\321",	ISO8859_1},
	    {0, "oacute",	"\363",	ISO8859_1},
	    {0, "Oacute",	"\323",	ISO8859_1},
	    {0, "ocirc",	"\364",	ISO8859_1},
	    {0, "Ocirc",	"\324",	ISO8859_1},
	    {0, "ograve",	"\362",	ISO8859_1},
	    {0, "Ograve",	"\322",	ISO8859_1},
	    {0, "oslash",	"\370",	ISO8859_1},
	    {0, "Oslash",	"\330",	ISO8859_1},
	    {0, "otilde",	"\365",	ISO8859_1},
	    {0, "Otilde",	"\325",	ISO8859_1},
	    {0, "ouml",		"\366",	ISO8859_1},
	    {0, "Ouml",		"\326",	ISO8859_1},
	    {0, "szlig",	"\337",	ISO8859_1},
	    {0, "thorn",	"\376",	ISO8859_1},
	    {0, "THORN",	"\336",	ISO8859_1},
	    {0, "uacute",	"\372",	ISO8859_1},
	    {0, "Uacute",	"\332",	ISO8859_1},
	    {0, "ucirc",	"\373",	ISO8859_1},
	    {0, "Ucirc",	"\333",	ISO8859_1},
	    {0, "ugrave",	"\371",	ISO8859_1},
	    {0, "Ugrave",	"\331",	ISO8859_1},
	    {0, "uuml",		"\374",	ISO8859_1},
	    {0, "Uuml",		"\334",	ISO8859_1},
	    {0, "yacute",	"\375",	ISO8859_1},
	    {0, "Yacute",	"\335",	ISO8859_1},
	    {0, "yuml",		"\377",	ISO8859_1},
	    /* ISOnum */
	    {0, "half",		"\275",	ISO8859_1},
	    {0, "frac12",	"\275",	ISO8859_1},
	    {0, "frac14",	"\274",	ISO8859_1},
	    {0, "frac34",	"\276",	ISO8859_1},
	    {0, "sup1",		"\271",	ISO8859_1},
	    {0, "sup2",		"\262",	ISO8859_1},
	    {0, "sup3",		"\263",	ISO8859_1},
	    {0, "plus",		"+",	ISO8859_1},
	    {0, "plusmn",	"\261",	ISO8859_1},
	    {0, "lt",		"<",	ISO8859_1},
	    {0, "equals",	"=",	ISO8859_1},
	    {0, "gt",		">",	ISO8859_1},
	    {0, "divide",	"\270",	SYMBOLS},
	    {0, "times",	"\264",	SYMBOLS},
	    {0, "curren",	"\244",	ISO8859_1},
	    {0, "pound",	"\243",	ISO8859_1},
	    {0, "dollar",	"$",	ISO8859_1},
	    {0, "cent",		"\242",	ISO8859_1},
	    {0, "yen",		"\245",	ISO8859_1},
	    {0, "num",		"#",	ISO8859_1},
	    {0, "percnt",	"%",	ISO8859_1},
	    {0, "amp",		"&",	ISO8859_1},
	    {0, "ast",		"*",	ISO8859_1},
	    {0, "commat",	"@",	ISO8859_1},
	    {0, "lsqb",		"[",	ISO8859_1},
	    {0, "bsol",		"\\",	ISO8859_1},
	    {0, "rsqb",		"]",	ISO8859_1},
	    {0, "lcub",		"{",	ISO8859_1},
	    {0, "horbar",	"\276",	SYMBOLS},
	    {0, "verbar",	"|",	ISO8859_1},
	    {0, "rcub",		"}",	ISO8859_1},
	    {0, "micro",	"\265",	ISO8859_1},
	    {0, "ohm",		"\127", SYMBOLS},
	    {0, "deg",		"\260",	ISO8859_1},
	    /*  "ordm",		"???",	ISO8859_1 */
	    /*  "ordf",		"???",	ISO8859_1 */
	    {0, "sect",		"\246",	ISO8859_1},
	    {0, "para",		"\266",	ISO8859_1},
	    {0, "middot",	"\267",	ISO8859_1},
	    {0, "larr",		"\254",	SYMBOLS},
	    {0, "rarr",		"\256",	SYMBOLS},
	    {0, "uarr",		"\255",	SYMBOLS},
	    {0, "darr",		"\257",	SYMBOLS},
	    {0, "copy",		"\251",	ISO8859_1},
	    {0, "reg",		"\256",	ISO8859_1},
	    {0, "trade",	"\324",	SYMBOLS},
	    {0, "brvbar",	"\245",	ISO8859_1},
	    {0, "not",		"\254",	ISO8859_1},
	    /*  "sung",		"???",	ISO8859_1 */
	    {0, "excl",		"!",	ISO8859_1},
	    {0, "iexcl",	"\240",	ISO8859_1},
	    {0, "quot",		"\"",	ISO8859_1},
	    {0, "apos",		"\264",	ISO8859_1},
	    {0, "lpar",		"(",	ISO8859_1},
	    {0, "rpar",		")",	ISO8859_1},
	    {0, "comma",	",",	ISO8859_1},
	    {0, "lowbar",	"_",	ISO8859_1},
	    {0, "hyphen",	"-",	ISO8859_1},
	    {0, "period",	".",	ISO8859_1},
	    {0, "sol",		"/",	ISO8859_1},
	    {0, "colon",	":",	ISO8859_1},
	    {0, "semi",		";",	ISO8859_1},
	    {0, "quest",	"?",	ISO8859_1},
	    {0, "iquest",	"\277",	ISO8859_1},
	    {0, "laquo",	"\341",	SYMBOLS},
	    {0, "raquo",	"\361",	SYMBOLS},
	    {0, "lsquo",	"`",	ISO8859_1},
	    {0, "rsquo",	"'",	ISO8859_1},
	    {0, "ldquo",	"``",	ISO8859_1},
	    {0, "rdquo",	"''",	ISO8859_1},
	    {0, "nbsp",		"nbsp",	SYSTEM_DATA}, /* No break space */
	    {0, "shy",		"",	SYSTEM_DATA}, /* Soft hyphen */
	    /* ISOgrk3 */
	    {0, "alpha",	"a",	SYMBOLS},
	    {0, "beta",		"b",	SYMBOLS},
	    {0, "gamma",	"g",	SYMBOLS},
	    {0, "Gamma",	"G",	SYMBOLS},
	    /*  "gammad",	"",	SYMBOLS */
	    {0, "delta",	"d",	SYMBOLS},
	    {0, "Delta",	"D",	SYMBOLS},
	    {0, "epsi",		"e",	SYMBOLS},
	    {0, "epsiv",	"\316",	SYMBOLS},
	    /*  "epsis",	"",	SYMBOLS */
	    {0, "zeta",		"z",	SYMBOLS},
	    {0, "eta",		"h",	SYMBOLS},
	    {0, "thetas",	"q",	SYMBOLS},
	    {0, "Theta",	"Q",	SYMBOLS},
	    {0, "thetav",	"J",	SYMBOLS},
	    {0, "iota",		"i",	SYMBOLS},
	    {0, "kappa",	"k",	SYMBOLS},
	    /*  "kappav",	"",	SYMBOLS */
	    {0, "lambda",	"l",	SYMBOLS},
	    {0, "Lambda",	"L",	SYMBOLS},
	    {0, "mu",		"m",	SYMBOLS},
	    {0, "nu",		"n",	SYMBOLS},
	    {0, "xi",		"x",	SYMBOLS},
	    {0, "Xi",		"X",	SYMBOLS},
	    {0, "pi",		"p",	SYMBOLS},
	    /*  "piv",		"",	SYMBOLS */
	    {0, "Pi",		"P",	SYMBOLS},
	    {0, "rho",		"r",	SYMBOLS},
	    /*  "rhov",		"",	SYMBOLS */
	    {0, "sigma",	"s",	SYMBOLS},
	    {0, "Sigma",	"S",	SYMBOLS},
	    {0, "sigmav",	"V",	SYMBOLS},
	    {0, "tau",		"t",	SYMBOLS},
	    {0, "upsi",		"u",	SYMBOLS},
	    {0, "Upsi",		"U",	SYMBOLS},
	    {0, "phis",		"f",	SYMBOLS},
	    {0, "Phi",		"F",	SYMBOLS},
	    {0, "phiv",		"j",	SYMBOLS},
	    {0, "chi",		"c",	SYMBOLS},
	    {0, "psi",		"y",	SYMBOLS},
	    {0, "Psi",		"Y",	SYMBOLS},
	    {0, "omega",	"w",	SYMBOLS},
	    {0, "Omega",	"W",	SYMBOLS},
	    /* ISOtech */
	    {0, "aleph",	"\300", SYMBOLS},
	    {0, "and",		"\331", SYMBOLS},
	    /*  "ang90",	"", SYMBOLS */
	    /*  "angsph",	"", SYMBOLS */
	    {0, "ap",		"\273", SYMBOLS},
	    {0, "becaus",	"\334", SYMBOLS},
	    {0, "bottom",	"\136", SYMBOLS},
	    {0, "cap",		"\307", SYMBOLS},
	    /*  "cong",		"", SYMBOLS */
	    /*  "conint",	"", SYMBOLS */
	    {0, "cup",		"\254", SYMBOLS},
	    {0, "equiv",	"\272", SYMBOLS},
	    {0, "exist",	"\44", SYMBOLS},
	    {0, "forall",	"\42", SYMBOLS},
	    {0, "fnof",		"\246", SYMBOLS},
	    {0, "ge",		"\263", SYMBOLS},
	    {0, "iff",		"\333", SYMBOLS},
	    {0, "infin",	"\245", SYMBOLS},
	    {0, "int",		"\362", SYMBOLS},
	    {0, "isin",		"\316", SYMBOLS},
	    {0, "lang",		"\341", SYMBOLS},
	    {0, "lArr",		"\334", SYMBOLS},
	    {0, "le",		"\243", SYMBOLS},
	    {0, "minus",	"-", SYMBOLS},
	    /*  "mnplus",	"", SYMBOLS */
	    {0, "nabla",	"\321", SYMBOLS},
	    {0, "ne",		"\271", SYMBOLS},
	    {0, "ni",		"\314", SYMBOLS},
	    {0, "or",		"\332", SYMBOLS},
	    /*  "par",		"", SYMBOLS */
	    {0, "part",		"\266", SYMBOLS},
	    /*  "permil",	"", SYMBOLS */
	    {0, "perp",		"\136", SYMBOLS},
	    {0, "prime",	"\242", SYMBOLS},
	    {0, "Prime",	"\262", SYMBOLS},
	    {0, "prop",		"\265", SYMBOLS},
	    /*  "radic",	"", SYMBOLS */
	    {0, "rang",		"\361", SYMBOLS},
	    {0, "rArr",		"\336", SYMBOLS},
	    {0, "sim",		"\176", SYMBOLS},
	    /*  "sime",		"", SYMBOLS */
	    /*  "square",	"", SYMBOLS */
	    {0, "sub",		"\314", SYMBOLS},
	    {0, "sube",		"\315", SYMBOLS},
	    {0, "sup",		"\311", SYMBOLS},
	    {0, "supe",		"\312", SYMBOLS},
	    /*  "there4",	"", SYMBOLS */
	    {0, "Verbar",	"||", SYMBOLS},
	    {0, "angst",	"\305", ISO8859_1},
	    /*  "bernou",	"", SYMBOLS */
	    /*  "compfn",	"", SYMBOLS */
	    /*  "Dot",		"", SYMBOLS */
	    /*  "DotDot",	"", SYMBOLS */
	    /*  "hamilt",	"", SYMBOLS */
	    /*  "lagran",	"", SYMBOLS */
	    {0, "lowast",	"*", SYMBOLS},
	    {0, "notin",	"\317", SYMBOLS},
	    /*  "order",	"", SYMBOLS */
	    /*  "phmmat",	"", SYMBOLS */
	    /*  "tdot",		"", SYMBOLS */
	    /*  "tprime",	"", SYMBOLS */
	    /*  "wedgeq",	"", SYMBOLS */
	    /* ... */
	    {0, "bull",		"\267",	SYMBOLS},
	    {0, "mdash",	"\276",	SYMBOLS},
	    {0, "ndash",	"\276",	SYMBOLS},
	    {0, "harr",		"\253",	SYMBOLS},
	    {0, "uArr",		"\335",	SYMBOLS},
	    {0, "dArr",		"\337",	SYMBOLS},
	    /* ... */
	}

@proc init_entity_quarks()
{
    int i;
    for (i = 0; i < XtNumber(entities); i++)
	entities[i].q = XrmStringToQuark(entities[i].name);
}

@ |init_stystack| creates the bottommost style entry, which may not be
popped. It also computes the em and lineheight in the default font,
relative to which most of the dimensions in the style sheet are
specified.

@proc init_stystack($)
{
    FormatInfo sty;
    int h, d;

    new(sty);
    sty->next = NULL;
    sty->tag = XrmStringToQuark("<default style>");
    sty->verbatim = FALSE;
    sty->nowrap = FALSE;
    sty->textstyle = FtNORMALSIZE | FtFAMILY1;
    sty->fg = $foreground;
    sty->bg = TRANSPARENT;
    sty->hide = FALSE;
    sty->title = FALSE;
    sty->stylesheettype = StyleNo;
    sty->stylesheet = NULL;
    sty->data = 0;				/* --> $extradata[0] = NULL */
    sty->ismap = FALSE;
    sty->inlinehmargin = 0;
    sty->inlinevmargin = 0;
    sty->children = 0;
    sty->break_after = FALSE;
    sty->parmode = FtJUSTIFYBOTH | FtNORMALPARA | FtNOCLEAR;
    sty->leftmargin = 0;
    sty->rightmargin = 0;
    sty->width = -1;
    sty->leading = 1.0;
    sty->flush = FtNOCLEAR;
    sty->parindent = 0;
    sty->suppress_indent = FALSE;
    sty->save_content = FALSE;

    $stystack = sty;

    $default_em = $get_em_of_textstyle($, sty->textstyle);
    $get_lineheight_of_textstyle($, sty->textstyle, &h, &d);
    $default_baseline = h + d;
    $previous_is_blank = TRUE;
    $start_of_par = TRUE;
    $suppress_indent = FALSE;
}

@def CHK_SAVE_CONTENT(sty, data, len) =
    do {
        if ((sty)->save_content) {
	    if (! $contentbuffer) {
		newarray($contentbuffer, (len) + 1);
		$contentbuffer[0] = '\0';
	    } else
		renewarray($contentbuffer, strlen($contentbuffer) + (len) + 1);
	    strncat($contentbuffer, data, len);
	}
    } while (0)

@ |add_word| adds the passed character as a word chunk, by calling the
|add_word| method. It passes the current style and color.

@proc add_word($, const String data, Cardinal len, Pixel fg, Pixel bg, TextStyle style, int parindent, XtPointer extra)
{
    int w, sp;
    FormatInfo sty = $stystack;

    if (sty->title) {				/* Current elt is a title */
	if (! $title) {
	    newarray($title, len + 1);
	    $title[0] = '\0';
	} else
	    renewarray($title, strlen($title) + len + 1);
	strncat($title, data, len);
	CHK_SAVE_CONTENT(sty, data, len);
    } else if (sty->stylesheettype != StyleNo) { /* Current is stylesheet */
	if (! sty->stylesheet) {
	    newarray(sty->stylesheet, len + 1);
	    sty->stylesheet[0] = '\0';
	} else
	    renewarray(sty->stylesheet, strlen(sty->stylesheet) + len + 1);
	strncat(sty->stylesheet, data, len);
	CHK_SAVE_CONTENT(sty, data, len);
    } else if (sty->hide) {			/* Current element is hidden */
	CHK_SAVE_CONTENT(sty, data, len);
	/* Do nothing */
    } else if (sty->tag != PCDATA_TAG) {	/* Start PCDATA first */
	$add_starttag($, "#PCDATA", 7, NULL);	/* Then recurse */
	add_word($, data, len, fg, bg, style, parindent, extra);
    } else {					/* Normal word, create chunk */
	if ($start_of_par) {			/* New paragraph */
	    if ($suppress_indent) {		/* No indent */
		$suppress_indent = FALSE;	/* Reset for next par */
	    } else if (parindent != 0) {	/* Insert parindent */
		sp = $get_space_of_textstyle($, style);
		w = 1000 * parindent/sp;
		$add_hspace($, w, FALSE, FALSE, fg, bg, style, extra);
	    }
	    $start_of_par = FALSE;
	}
	$add_word($, data, len, fg, bg, style, extra);
	$previous_is_blank = FALSE;
	CHK_SAVE_CONTENT(sty, data, len);
    }
}

@ |add_spaces| adds the whitespace either as stretchable space, or as
`word', depending on the current style.

@proc add_spaces($, const String data, Cardinal len)
{
    FormatInfo sty = $stystack;

    if (sty->title) {				/* Store in title */
	if ($title) {				/* Don't add spaces at start */
	    renewarray($title, strlen($title) + len + 2);
	    strncat($title, data, len);
	}
	CHK_SAVE_CONTENT(sty, data, len);
    } else if (sty->stylesheettype != StyleNo) { /* Store in stylesheet */
	if (! sty->stylesheet) {
	    newarray(sty->stylesheet, len + 1);
	    sty->stylesheet[0] = '\0';
	} else
	    renewarray(sty->stylesheet, strlen(sty->stylesheet) + len + 2);
	strncat(sty->stylesheet, data, len);
	CHK_SAVE_CONTENT(sty, data, len);
    } else if (sty->hide) {			/* Don't display */
	CHK_SAVE_CONTENT(sty, data, len);
    } else if (sty->verbatim) {			/* As many spaces as in data */
	if ($stystack->tag != PCDATA_TAG) {	/* Start PCDATA */
	    $add_starttag($, "#PCDATA", 7, NULL);
	    add_spaces($, data, len);		/* Recursively */
	} else {
	    add_word($, data, len, sty->fg, sty->bg, sty->textstyle,
		     sty->parindent, $extradata[sty->data]);
	}
    } else if ($start_of_par) {			/* Ignore space at start */
	CHK_SAVE_CONTENT(sty, data, len);
    } else if ($previous_is_blank) {		/* Ignore multiple spaces */
	CHK_SAVE_CONTENT(sty, data, len);
    } else if (sty->tag != PCDATA_TAG) {	/* Start PCDATA first */
	add_starttag($, "#PCDATA", 7, NULL);
	add_spaces($, data, len);		/* Recursively */
    } else if (sty->nowrap) {			/* No breakable space */
	$add_hspace($, 1000, TRUE, FALSE, sty->fg, sty->bg,
		    sty->textstyle, $extradata[sty->data]);
	$previous_is_blank = TRUE;
	CHK_SAVE_CONTENT(sty, data, len);
    } else {
	$add_hspace($, 1000, TRUE, TRUE, sty->fg, sty->bg,
		    sty->textstyle, $extradata[sty->data]);
	$previous_is_blank = TRUE;
	CHK_SAVE_CONTENT(sty, data, len);
    }
}

@ |add_newline| checks if newlines are significant in the current text
and inserts a line break or a space. If |obeyspaces| is set, the
newline results in a line break (|$add_vspace|). The extra space
before the line break is there so that the formatter cannot remove the
vertical space if there was already some vertical space above it.

@proc add_newline($)
{
    FormatInfo sty = $stystack;

    if (sty->title) add_spaces($, " ", 1);
    else if (sty->stylesheettype != StyleNo) add_spaces($, "\n", 1);
    else if (sty->hide) {
	CHK_SAVE_CONTENT(sty, "\n", 1);
    } else if (! sty->verbatim) add_spaces($, " ", 1);
    else {
	if (sty->tag != PCDATA_TAG) {		/* Start PCDATA first */
	    $add_starttag($, "#PCDATA", 7, NULL);
	    add_newline($);			/* Recursively */
	} else {
	    /* add_spaces($, " ", 1); */
	    $add_vspace($, 0);
	    $start_of_par = FALSE;
	    CHK_SAVE_CONTENT(sty, "\n", 1);
	}
    }
}

@ |add_eod| is called by |add_text| when the end of the text is
reached (as indicated by the |nchars| argument of |add_text| being
0). It pops the remaining elements from the stack and flushes the
chunk buffer in the superclass (FormattedText).

@proc add_eod($)
{
    while ($stystack->next != NULL) $pop_style($);
    $add_eod($);
}

@ |interp_ifmatch| parses a string |"@@ifmatch(attr, pat, then,
else)"|, matches the value of the attribute with the pattern and
returns either the `then' string or the `else' string. |t| is assumed
to be at least |LITLEN+1| characters long.

@proc Bool interp_ifmatch($, char *s, Attrib attr, char t[])
{
    char name[LITLEN+1], re[LITLEN+1], val[LITLEN+1];
    Bool in_set, match, is_attr;
    regex_t regex;
    int i, k;

    assert(strncmp(s, "@ifmatch(", 9) == 0);

    /* Get attribute or property name */
    for (i = 9; isspace(s[i]); i++) ;
    if (s[i] == '!') is_attr = TRUE;
    else if (s[i] == '$') is_attr = FALSE;	/* property name */
    else return FALSE;				/* Syntax error */
    for (i++, k = 0; isnmchar(s[i]); i++, k++) name[k] = s[i];
    name[k] = '\0';

    /* Get regular expression */
    for (; isspace(s[i]); i++) ;
    if (s[i] != ',') return FALSE;		/* Syntax error */
    for (i++; isspace(s[i]); i++) ;
    if (s[i] != '"') return FALSE;		/* Syntax error */
    i++;
    in_set = FALSE;
    for (k = 0; s[i] && (s[i] != '"' || in_set); i++, k++) {
        re[k] = s[i];
        if (s[i] == '\\' && s[i+1]) re[++k] = s[++i];
        else if (s[i] == '[') in_set = TRUE;
        else if (s[i] == ']' && s[i-1] != '[') in_set = FALSE;
    }
    re[k] = '\0';
    if (s[i] != '"') return FALSE;		/* Syntax error */

    /* Get attribute/property value and execute regular expression */
    if (! is_attr) {
	XtAppWarning(XtWidgetToApplicationContext($),
		   "`$property' in stylesheet not implemented yet.");
	match = FALSE;
    } else if (! get_attrib($, attr, name, val, sizeof(val))) {
	match = FALSE;
    } else if (regcomp(&regex, re, REG_EXTENDED|REG_NOSUB) != 0) {
#if 0
	XtAppWarning(XtWidgetToApplicationContext($),
		   "Regular expression in stylesheet is incorrect.");
#endif
	return FALSE;
    } else {
	match = regexec(&regex, val, 0, NULL, 0) == 0;
	regfree(&regex);
    }

    /* Skip to "then" part */
    for (i++; isspace(s[i]); i++) ;
    if (s[i] != ',') return FALSE;		/* Syntax error */
    for (i++; isspace(s[i]); i++) ;

    if (match) {
	/* Copy "then" part */
        for (k = 0; s[i] && s[i] != ',' && s[i] != ')'; i++, k++) t[k] = s[i];
        t[k] = '\0';
    } else {
        for (; s[i] && s[i] != ',' && s[i] != ')'; i++) ; /* Skip then-part */
	/* Get "else" part */
	if (!s[i]) return FALSE;		/* Syntax error */
	if (s[i] == ')') return FALSE;		/* Empty else-part */
	for (i++; isspace(s[i]); i++) ;
	for (k = 0; s[i] && s[i] != ')'; i++, k++) t[k] = s[i];
#if 0
	if (!s[i]) return FALSE;		/* Syntax error */
#endif
	t[k] = '\0';
    }
    return TRUE;
}

@ |get_property| looks for the property |property| in the resource
database and recursively interprets the result in case it is a
function. If the value starts with an `!', it is replaced by the value
of the attribute after |!|, if it starts with |$|, it is replaced by
the value of the property after |$|. That property is the value of the
property in the parent element (not implemented yet).

|val| is a string of length at least |LITLEN + 1|. |list| is the
result of |XrmQGetSearchList|.

@proc Bool get_property($, XrmSearchList list, Attrib attr, XrmQuark property, char val[])
{
    XrmQuark repr;
    XrmValue v2, v1;

    v1.size = LITLEN + 1;
    v1.addr = val;
    v2.addr = NULL;
    if (! XrmQGetSearchResource(list, property, property, &repr, &v2)
	|| ! XtConvertAndStore($, XrmNameToString(repr), &v2, XtRString, &v1))
	return FALSE;

    /* Evaluate @-functions */
    while (strncmp(val, "@ifmatch(", 9) == 0)
	if (!interp_ifmatch($, val, attr, val)) return FALSE;

    if (val[0] == '!')
	return get_attrib($, attr, val + 1, val, LITLEN + 1);
    /* else if (val[0] == '$')
	return get_parent_property($, val + 1, val); */
    else
	return TRUE;
}

@ |tag_nesting_to_quarklist| creates an array of quarks, corresponding
to the current nesting of elements, with the oldest first and the
current element at the end.

@proc tag_nesting_to_quarklist($, XrmQuark nest[TAGLVL])
{
    FormatInfo s;
    Cardinal n;

    for (n = 0, s = $stystack; s->next; s = s->next) n++;
    if (n >= TAGLVL) n = TAGLVL - 1;
    nest[n] = 0;
    for (s = $stystack; n > 0 && s->next; s = s->next) nest[--n] = s->tag;
    assert(n == 0);

}

@ |upper| converts the passed string |s| to uppercase (replacing
the contents).

@proc String upper(String s)
{
    String t;
    if (s != NULL) for (t = s; *t; t++) *t = _toupper(*t);
    return s;
}

@ |parse_starttag| is called by |get_token| when it has found a start
tag. When the function is called, |$nparsed| points to the |<| and
|get_token| has already checked that the is a |>| at he end.
|parse_starttag| will set |name| and |namelen| to the name of the
element and collect the attributes in |attr|, which will be allocated
on the heap.

The attribute names are converted to uppercase. An attribute value is
converted to uppercase only when it wasn't in quoted.

@proc parse_starttag($, String *name, Cardinal *namelen, Attrib *attr)
{
    char *s = $text + $nparsed;
    int i, j, k;
    Attrib h;

    assert(s[0] == '<');

    /* Get element name */
    i = 1;					/* Skip '<' */
    while (s[i] != '>' && ! isspace(s[i])) i++;
    *namelen = min(i - 1, NAMELEN);		/* Element name */
    *name = s + 1;

    /* Get attributes */
    *attr = NULL;
    while (s[i] != '>') {
	j = i;
	while (isspace(s[j])) j++;
	i = j;
	if (s[i] == '>') break;

	/* Get next attribute name */
	while (isnmchar(s[j])) j++;
	if (j == i) break;			/* Error, ill. char. */
	new(h); h->next = *attr; *attr = h;
	h->name[0] = '\0'; upper(strncat(h->name, s + i, min(j - i, NAMELEN)));

	/* Look for attribute's value */
	k = j;
	while (isspace(s[j])) j++;
	if (s[j] != '=') {			/* No value, use name */
	    strcpy(h->value, h->name);
	    i = j;
	} else {
	    j++;
	    while (isspace(s[j])) j++;
	    i = j;
	    if (s[i] == '>') break;		/* Error, ignore */
	    h->value[0] = '\0';
	    if (s[i] == '"') {			/* Quoted literal */
		j = i + 1;
		while (s[j] != '"' && s[j] != '\n') j++;
		/* \n is incorrect, but we catch unclosed strings... */
		strncat(h->value, s + i + 1, min(j - (i + 1), LITLEN));
		i = j + 1;
	    } else if (s[i] == '\'') {		/* Quoted (') literal */
		j = i + 1;
		while (s[j] != '\'' && s[j] != '\n') j++;
		/* \n is incorrect, but we catch unclosed strings... */
		strncat(h->value, s + i + 1, min(j - (i + 1), LITLEN));
		i = j + 1;
	    } else {				/* Name token */
		while (s[j] != '>' && ! isspace(s[j])) j++;
#if 0
		upper(strncat(h->value, s + i, min(j - i, LITLEN)));
#else
		strncat(h->value, s + i, min(j - i, LITLEN));
#endif
		i = j;
	    }
	}
    }
}

@ |get_token| is the lexical analyser. It recognizes SGML tags,
entities, words, and whitespace. It returns the next token in the
|$text| variable, returning |T_NONE| is the token is incomplete,
unless |is_eof| is |TRUE|, in which case no more text is forthcoming.

|isnmchar| tests for an SGML name character (assumed to be a letter,
'-', '.', or '_').

@def isnmchar(c) = (isalpha(c) || (c) == '-' || (c) == '.' || (c) == '_')

@proc Bool get_token($, Bool is_eof, Token *tok, String *data, Cardinal *len, Attrib *attr)
{
    char prev, *s = $text;
    Bool com, sq, dq;
    Cardinal i, nbrack;

    assert($nparsed <= $nchars);
    if ($nparsed == $nchars) return FALSE;	/* Insufficient data */
    i = $nparsed;
    *data = s + i;

    switch (s[i]) {

    case '\r':
	*tok = T_NONE;
	i++;
	*len = i - $nparsed;
	$nparsed = i;
	return TRUE;

    case ' ':
    case '\t':
	while (i < $nchars && (s[i] == ' ' || s[i] == '\t')) i++;
	if (i == $nchars && ! is_eof) return FALSE;
	*tok = T_BLANK;
	*len = i - $nparsed;
	$nparsed = i;
	return TRUE;

    case '\n':
    case '\f':
	if (i + 1 == $nchars && is_eof) *tok = T_NL; /* Nothing follows */
	else if (i + 1 == $nchars) return FALSE; /* Next char unknown */
	else if (s[i+1] != '<') *tok = T_NL;	/* Next is not a tag */
	else if (i + 2 == $nchars && is_eof) *tok = T_NL; /* Next not tag */
	else if (i + 2 == $nchars) return FALSE; /* Unknown < or </ */
	else if (s[i+2] == '/') *tok = T_NONE; /* Next is endtag */
	else *tok = T_NL;
	i++;
	*len = i - $nparsed;
	$nparsed = i;
	return TRUE;

    case '&':
	i++;
	if (s[i] == '#') {
	    if (i == $nchars) {*tok = T_NONE; return is_eof;}
	    i++;
	    while (i < $nchars && isdigit(s[i])) i++;
	} else {
	    while (i < $nchars && isnmchar(s[i])) i++;
	}
	if (i == $nchars && ! is_eof) return FALSE;
	(*data)++;				/* Skip '&' */
	*len = i - $nparsed - 1;
	$nparsed = (i < $nchars && s[i] == ';') ? i + 1 : i;
	*tok = T_ENTITY;
	return TRUE;

    case '<':					/* Markup? */
	if (++i == $nparsed) return FALSE;

	switch (s[i]) {

	case '/':				/* Endtag? */
	    while (i < $nchars && s[i] != '>' && ! isspace(s[i])) i++;
	    if (i == $nchars && ! is_eof) {
		return FALSE;			/* Next char unknown */
	    } else if (i == $nchars || s[i] != '>') {
		*tok = T_WORD;			/* Incomplete tag */
		*len = i - $nparsed;
		$nparsed = i;
		return TRUE;
	    } else {
		assert(s[i] == '>');
		*tok = T_ENDTAG;
		*data += 2;			/* Skip "</" */
		*len = i - $nparsed - 2;	/* Length of tag */
		$nparsed = i + 1;		/* Skip ">" */
		return TRUE;
	    }

	case '!':
	    prev = '\0';
	    com = dq = sq = FALSE;
	    nbrack = 0;
	    while (i < $nchars && (s[i] != '>' || com || dq || sq || nbrack)) {
		if (s[i] == '"' && !sq && !com) dq = !dq;
		else if (s[i] == '\'' && !dq && !com) sq = !sq;
		else if (s[i] == '-' && !dq && !sq && prev == '-') com = !com;
		else if (s[i] == '<' && !dq && !sq && !com) nbrack++;
		else if (s[i] == '>' && !dq && !sq && !com) nbrack--;
		else if (s[i] == '\n') dq = sq = FALSE;
		/* \n is incorrect, but we catch unclosed strings... */
		prev = prev != '-' ? s[i] : '\0';
		i++;
	    }
	    if (i == $nchars) return FALSE;
	    *tok = T_NONE;
	    i++;
	    *len = i - $nparsed;
	    $nparsed = i;
	    return TRUE;

	default:				/* Start tag */
	    dq = sq = FALSE;
	    while (i < $nchars && (s[i] != '>' || dq || sq)) {
		if (s[i] == '"' && !sq) dq = !dq;
		else if (s[i] == '\'' && !dq) sq = !sq;
		else if (s[i] == '\n') dq = sq = FALSE;
		/* \n is incorrect, but we catch unclosed strings... */
		i++;
	    }
	    if (i == $nchars) return FALSE;	/* No > found yet */
	    else if (i + 1 == $nchars && is_eof) ; /* Nothing after > */
	    else if (i + 1 == $nchars) return FALSE; /* Wait for \n */
	    else if (s[i+1] == '\n' || s[i+1] == '\f') i++; /* Skip \n */
	    else if (s[1+1] != '\r') ;		/* Nothing to skip */
	    else if (i + 2 == $nchars && is_eof) i++;	/* Skip \r */
	    else if (i + 2 == $nchars) return FALSE; /* Wait for \n */
	    else if (s[i+2] == '\n' || s[i+2] == '\f') i += 2; /* Skip \r\n */
	    i++;
	    *tok = T_STARTTAG;
	    parse_starttag($, data, len, attr);
	    $nparsed = i;
	    return TRUE;
	}

    default:
	while (i < $nchars && s[i] != ' ' && s[i] != '\t'
	       && s[i] != '\r' && s[i] != '\n' && s[i] != '\f'
	       && s[i] != '<' && s[i] != '&') i++;
	if (i == $nchars && ! is_eof) return FALSE;
	*tok = T_WORD;
	*len = i - $nparsed;
	$nparsed = i;
	return TRUE;
    }
}

@proc dispose_attr(Attrib *attr)
{
    if (*attr) {
	dispose_attr(&(*attr)->next);
	dispose(*attr);
    }
}

@ |parse_text| splits the text in |$text| in tokens and processes
each of them. It changes |$nparsed|.

@proc parse_text($, Bool is_eof)
{
    Token tok;
    String data;
    Cardinal len;
    Attrib attr;

    while (get_token($, is_eof, &tok, &data, &len, &attr)) {
	switch (tok) {
	case T_WORD:
	    add_word($, data, len, $stystack->fg, $stystack->bg,
		     $stystack->textstyle, $stystack->parindent,
		     $extradata[$stystack->data]);
	    break;
	case T_BLANK: add_spaces($, data, len); break;
	case T_NL: add_newline($); break;
	case T_ENTITY: $process_entity($, data, len); break;
	case T_ENDTAG: $add_endtag($, data, len); break;
	case T_STARTTAG:
	    $add_starttag($, data, len, attr);
	    dispose_attr(&attr);
	    break;
	case T_NONE: break;
	default: assert(! "Cannot happen");
	}
    }
}

@ |get_parser_state| returns the three variables that determine the
state of the tokenizer: |$text|, |$nchars| and |$nparsed|.

@proc get_parser_state($, String *text, Cardinal *nchars, Cardinal *nparsed)
{*text = $text; *nchars = $nchars; *nparsed = $nparsed;}

@ |set_parser_state| sets the three variables that determine the state
of the tokenizer to new values.

@proc set_parser_state($, String text, Cardinal nchars, Cardinal nparsed)
{$text = text; $nchars = nchars; $nparsed = nparsed;}


@ |alloc_extradata| extends the |$extradata| array and puts the url
and ismap flag in the new cell. It returns the index of the new cell.

For efficiency, the array is not re-allocated each time, but only once
every 25 calls.

@def ED_INCR = 25

@proc int alloc_extradata($, char *url, Bool ismap)
{
    if ($extradata_allocated == $nrextradata) {
	$extradata_allocated += ED_INCR;
	renewarray($extradata, $extradata_allocated);
    }
    new($extradata[$nrextradata]);
    $extradata[$nrextradata]->url = XtNewString(url);
    $extradata[$nrextradata]->ismap = ismap;
    return $nrextradata++;
}


@IMPORTS

@incl <X11/Xmu/Converters.h>
@incl <ctype.h>
@incl <assert.h>
@incl <stdio.h>
@incl <X11/Xmu/CharSet.h>
@incl <regex.h>
@incl <X11/xpm.h>
