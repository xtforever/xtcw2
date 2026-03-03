# HTML2 -- HTML widget
# v0.0 (3 Feb 1995)
# Bert Bos <bert@let.rug.nl>

@class XfwfHTML2 (XfwfSimpleSGML) @file=HTML2

@ The HTML2 widget is a subclass of the SimpleSGML widget. It
overrides the methods for handling start and end tags, in order to
insert the tags that may be omitted in HTML. It also recognizes more
entities, in particular the WWW icon entities.

TO DO: forms.


@PUBLIC

@ To be able to process forms without an ACTION attribute, the widget
must know the URL of the document itself.

	@var String base = NULL

@ The |resolveURL| callback is called when the formatter encounters an
inline image. It calls the callback function with a pointer to a
|XfwfResolveRec| as |call_data|. The |resolveRec| contains two fields:
a string |url| and a widget. The callback should set up things so that
the referent of |url| is drawn into the widget.

	@var <Callback> XtCallbackList resolveURL = NULL

@ The |activateForm| callback is called when the user clicks the mouse on
a submit button in a form. The |call_data| field is set to a pointer
to an |XfwfActivateFormInfo| struct.

	@var <Callback> XtCallbackList activateForm = NULL



@EXPORTS

	@type XfwfResolveRec = struct {
	    String url;
	    Widget widget;
	}

	@type XfwfActivateFormInfo = struct {
	    String url;
	    String data;
	    int length;				/* # bytes in data */
	    int method;				/* GET = 0, POST = 2 */
	}


@PRIVATE

	@type FormFieldType = enum {
	    Text, Submit, Reset, Password, Checkbox, Radio, Hidden,
	    Textarea, Select, Isindex,
	}
	@type FormField = struct _FormField {
	    struct _Form *form;
	    FormFieldType type;
	    char name[NAMELEN];
	    char value[LITLEN];
	    Bool checked;
	    String *options;			/* OPTIONs in a SELECT */
	    Bool *selected;			/* Selected options */
	    int nroptions;
	    Widget w;
	    struct _FormField *next;
	} *

	@type Form = struct _Form {
	    char action[LITLEN];
	    int method;
	    FormField fields;
	    struct _Form *next;
	} *

	@var Form formlist


@METHODS

@proc class_initialize
{
    init_iconentities();
    init_tagnames();
}

@proc initialize
{
    XrmDatabase db;

    if ($base) $base = XtNewString($base);
    $formlist = NULL;
    db = XrmGetStringDatabase("*TEXTAREA.savecontent:TRUE\n\
*TEXTAREA.hide:TRUE\n\
*OPTION.savecontent:TRUE\n\
*OPTION.hide:TRUE\n");
    XrmMergeDatabases(db, &$styledb);
}

@proc set_values
{
    if ($old$base != $base) {
	if ($old$base) XtFree($old$base);
	if ($base) $base = XtNewString($base);
    }
    return FALSE;
}

@ The |get_entity| method is overridden. It checks for a WWW-icon
entity and if that fails it calls the inherited method, which will
check for ISO standard character entities.

@proc get_entity
{
    char s[NAMELEN+1];
    XrmQuark q;
    int i;

    s[0] = '\0';
    q = XrmStringToQuark(strncat(s, name, min(NAMELEN, namelen)));
    for (i = 0; i < XtNumber(iconentities); i++)
	if (iconentities[i].q == q) {
	    val[0] = '\0';
	    strncat(val, iconentities[i].value, maxlen);
	    *charset = iconentities[i].charset;
	    return TRUE;
	}
    /* Not found, try inherited method */
    return #get_entity($, name, namelen, val, maxlen, charset);
}

@ The |add_starttag| method is overriden, in order to be able to
insert omitted tags. The current implementation uses a small set of
heuristic rules, when HTML becomes more complex, a parser-like state
will probably have to be maintained.

@proc add_starttag
{
    char t[NAMELEN+1] = "";
    XrmQuark q;
    FormatInfo sty;

    q = XrmStringToQuark(upper(strncat(t, name, min(NAMELEN, namelen))));

    /* #PCDATA ends at next start tag */
    assert(q != PCDATA_T || $stystack->tag != PCDATA_T);
    if ($stystack->tag == PCDATA_T)
	$pop_style($);

    if (q != HEAD_T && q != PCDATA_T && q != TITLE_T && q != META_T
	&& q != LINK_T && q != STYLE_T && q != ISINDEX_T) {
	/* Check that we're not in HEAD */
	for (sty = $stystack; sty && sty->tag != HEAD_T; sty = sty->next) ;
	if (sty) {				/* Yes, we're in HEAD */
	    while ($stystack->next && $stystack->tag != HTML_T)
		$pop_style($);
	    if (q != BODY_T) $add_starttag($, "BODY", 4, NULL);
	}
    }

    if (q == H1_T || q == H2_T || q == H3_T || q == H4_T
	|| q == H5_T || q == H6_T) {
	/* H? must be a child of DIV, BODY or FORM, pop other elements */
	/* May also be child of A, although deprecated */
	while ($stystack->next
	       && $stystack->tag != A_T
	       && $stystack->tag != DIV_T
	       && $stystack->tag != BODY_T
	       && $stystack->tag != FORM_T
	       && $stystack->tag != HTML_T) $pop_style($);
	if ($stystack->tag != DIV_T && $stystack->tag != BODY_T
	    && $stystack->tag != FORM_T) $add_starttag($, "BODY", 4, NULL);
    } else if (q == BODY_T) {
	/* BODY must be a child of HTML */
	while ($stystack->next && $stystack->tag != HTML_T) $pop_style($);
	if ($stystack->tag != HTML_T) $add_starttag($, "HTML", 4, NULL);
    } else if (q == FORM_T) {
	/* FORM must be a child of HTML */
	while ($stystack->next && $stystack->tag != HTML_T) $pop_style($);
	if ($stystack->tag != HTML_T) $add_starttag($, "HTML", 4, NULL);
    } else if (q == LI_T) {
	/* LI must be a child of OL, UL, DIR, or MENU */
	while ($stystack->next
	       && $stystack->tag != OL_T
	       && $stystack->tag != UL_T
	       && $stystack->tag != DIR_T
	       && $stystack->tag != MENU_T) $pop_style($);
	if (! $stystack->next) $add_starttag($, "UL", 2, NULL);
    } else if (q == DT_T) {
	/* DT must be a child of DL */
	while ($stystack->next
	       && $stystack->tag != DL_T) $pop_style($);
	if ($stystack->tag != DL_T) $add_starttag($, "DL", 2, NULL);
    } else if (q == DD_T) {
	/* DD must be a child of DL */
	while ($stystack->next
	       && $stystack->tag != DL_T) $pop_style($);
	if ($stystack->tag != DL_T) $add_starttag($, "DL", 2, NULL);
    } else if (q == P_T || q == NOTE_T || q == ABSTRACT_T  || q == DIR_T 
	       || q == ADDRESS_T || q == UL_T || q == OL_T || q == DL_T
	       || q == PRE_T || q == QUOTE_T || q == MENU_T || q == DIV_T
	       || q == TABLE_T || q == DISPLAY_T || q == FIG_T) {
	/* Block elements only inside block containers */
	while ($stystack->tag == P_T || $stystack->tag == PRE_T
	       || $stystack->tag == PCDATA_T || $stystack->tag == SUB_T
	       || $stystack->tag == SUP_T || $stystack->tag == B_T
	       || $stystack->tag == U_T || $stystack->tag == S_T
	       || $stystack->tag == TT_T || $stystack->tag == I_T
	       || $stystack->tag == BIG_T || $stystack->tag == SMALL_T
	       || $stystack->tag == EM_T || $stystack->tag == STRONG_T
	       || $stystack->tag == CODE_T || $stystack->tag == SAMP_T
	       || $stystack->tag == KBD_T || $stystack->tag == VAR_T
	       || $stystack->tag == CITE_T || $stystack->tag == TAB_T
	       || $stystack->tag == MATH_T || $stystack->tag == A_T
	       || $stystack->tag == IMG_T || $stystack->tag == BR_T
	       || $stystack->tag == Q_T || $stystack->tag == LANG_T
	       || $stystack->tag == AU_T || $stystack->tag == DFN_T
	       || $stystack->tag == PERSON_T || $stystack->tag == ACRONYM_T
	       || $stystack->tag == ABBREV_T || $stystack->tag == INS_T
	       || $stystack->tag == DEL_T) $pop_style($);
	if (! $stystack->next) $add_starttag($, "BODY", 4, NULL);
    } else if (q == TITLE_T || q == LINK_T || q == STYLE_T
	       || q == META_T || q == BASE_T) {
	/* Head elements must be child of HEAD */
	while ($stystack->next
	       && $stystack->tag != HEAD_T && $stystack->tag != HTML_T)
	    $pop_style($);
	if ($stystack->tag != HEAD_T) $add_starttag($, "HEAD", 4, NULL);
    } else if (q == HTML_T) {
	/* HTML may not be a descendant of anything */
	while ($stystack->next) $pop_style($);
    } else if (q == TD_T || q == TH_T) {
	/* TD and TH must be child of TR */
	while ($stystack->next
 	       && $stystack->tag != TR_T
	       && $stystack->tag != TABLE_T) $pop_style($);
	if ($stystack->tag != TR_T) $add_starttag($, "TR", 2, NULL);
    } else if (q == TR_T) {
	/* TR must be child of TABLE */
	while ($stystack->next
	       && $stystack->tag != TABLE_T) $pop_style($);
	if ($stystack->tag != TABLE_T) $add_starttag($, "TABLE", 5, NULL);
    } else if (q == OPTION_T) {
	/* OPTION must be child of SELECT */
	while ($stystack->next && $stystack->tag != SELECT_T) $pop_style($);
	if ($stystack->tag != SELECT_T) $add_starttag($, "SELECT", 6, NULL);
    } else if (q == PCDATA_T || q == SUB_T || q == SUP_T || q == B_T
	       || q == U_T || q == S_T || q == TT_T || q == I_T
	       || q == BIG_T || q == SMALL_T || q == EM_T
	       || q == STRONG_T  || q == CODE_T || q == SAMP_T
	       || q == KBD_T || q == VAR_T || q == CITE_T
	       || q == TAB_T || q == MATH_T || q == A_T || q == IMG_T
	       || q == BR_T || q == Q_T || q == LANG_T || q == AU_T
	       || q == DFN_T || q == PERSON_T || q == ACRONYM_T
	       || q == ABBREV_T || q == INS_T || q == DEL_T
	       || q == A_T) {
	/* Text must be inside a text container */
	for (;;) {
	    if ($stystack->tag == DL_T
		|| $stystack->tag == OL_T
		|| $stystack->tag == UL_T
		|| $stystack->tag == DIR_T
		|| $stystack->tag == MENU_T
		|| $stystack->tag == SELECT_T
		|| $stystack->tag == HEAD_T)
		$pop_style($);
	    else if ($stystack->tag == TABLE_T)
		$add_starttag($, "TR", 2, NULL);
	    else if ($stystack->tag == TR_T)
		$add_starttag($, "TD", 2, NULL);
	    else if ($stystack->tag == DD_T
		|| $stystack->tag == BODY_T
		|| $stystack->tag == BLOCKQUOTE_T
		|| $stystack->tag == BQ_T
		|| $stystack->tag == ADDRESS_T
		|| $stystack->tag == DIV_T
		|| $stystack->tag == FORM_T
		|| $stystack->tag == TD_T
		|| $stystack->tag == TH_T
		|| $stystack->tag == FIG_T
		|| $stystack->tag == NOTE_T
		|| $stystack->tag == HTML_T)
		$add_starttag($, "P", 1, NULL);
	    else
		break;
	}
    }
#if 0
    if (q == A_T) {
	/* A may not be a descendant of A */
	for (sty = $stystack; sty->next; sty = sty->next)
	    if (sty->tag == A_T) {
		while ($stystack->tag != A_T) $pop_style($);
		$pop_style($);
		break;
	    }
	if ($stystack->tag == PCDATA_T) $pop_style($);
    }
#endif

    /* Check for form element */
    if (q == FORM_T) start_form($, attr);
    else if (q == INPUT_T) start_form_input($, attr);
    else if (q == TEXTAREA_T) start_form_textarea($, attr);
    else if (q == SELECT_T) start_form_select($, attr);
    else if (q == OPTION_T) start_form_option($, attr);
    else if (q == ISINDEX_T) start_isindex($, attr);

    /* Finally, push the element itself */
    $push_style($, q, attr);
}

@ |pop_style| is overridden in order to catch the end of a FORM or
TEXTAREA element.

@proc pop_style
{
    FormatInfo sty;

    assert($stystack->next != NULL);
    sty = $stystack;
    if (sty->tag == FORM_T) end_form($);
    else if (sty->tag == INPUT_T) end_form_input($);
    else if (sty->tag == TEXTAREA_T) end_form_textarea($);
    else if (sty->tag == SELECT_T) end_form_select($);
    else if (sty->tag == OPTION_T) end_form_option($);
    else if (sty->tag == ISINDEX_T) end_isindex($);

    #pop_style($);
}

@ |make_child_from_source| has an URL as argument (presumably
referring to an image). It uses a callback |resolve_image| to
translate the URL to a widget. The |call_data| argument of the
callback function is a pointer to a widget (initialized to |NULL|). If
the callback succeeds, it will have set |*call_data| to a valid
widget. The |push_style| method, which calls |make_child_from_source|,
will then add it to the layout.

@proc make_child_from_source
{
    XfwfResolveRec resolve;

    resolve.url = src;
    resolve.widget = w;
    XtCallCallbackList($, $resolveURL, &resolve);
}

@ |destroy| must free the memory occupied by forms

@proc destroy
{
    Form f, f1;
    FormField h, h1;
    int i;

    f = $formlist;
    while (f) {
	h = f->fields;
	while (h) {
	    if (h->type == Select) {
		for (i = 0; i < h->nroptions; i++)
		    dispose(h->options[i]);
		dispose(h->options);
	    }		    
	    h1 = h;
	    h = h->next;
	    dispose(h1);
	}
	f1 = f;
	f = f->next;
	dispose(f1);
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
@def set_max(a, b) = if ((a) >= (b)) ; else (a) = (b)
@def set_min(a, b) = if ((a) <= (b)) ; else (a) = (b)

@ The |iconentities| array holds the replacement texts for WWW icon
entities. The first column |q| is initialized by |init_iconentities|,
which is called by |class_initialize|.

	@type EntityRec = struct {
	    XrmQuark q;				/* The name as a quark */
	    String name;			/* The name, without "&;" */
	    String value;			/* Replacement text */
	    int charset;			/* Charset or SYSTEM_DATA */
	}
	@var EntityRec iconentities[] = {
	    {0, "folder",	"A", WWW_ICONS},
	    {0, "text.document",	"B", WWW_ICONS},
	    /* ... */
	    {0, "sbsp",		"sbsp",	SYSTEM_DATA},/* Soft break space */
	}

@proc init_iconentities()
{
    int i;
    for (i = 0; i < XtNumber(iconentities); i++)
	iconentities[i].q = XrmStringToQuark(iconentities[i].name);
}

@ The tag names are used by |add_starttag|. All names are converted to
quarks for quicker comparison.

	@var XrmQuark ABBREV_T
	@var XrmQuark ABSTRACT_T
	@var XrmQuark ACRONYM_T
	@var XrmQuark ADDRESS_T
	@var XrmQuark AU_T
	@var XrmQuark A_T
	@var XrmQuark BASE_T
	@var XrmQuark BIG_T
	@var XrmQuark BLOCKQUOTE_T
	@var XrmQuark BODY_T
	@var XrmQuark BQ_T
	@var XrmQuark BR_T
	@var XrmQuark B_T
	@var XrmQuark CAPTION_T
	@var XrmQuark CHANGED_T
	@var XrmQuark CITE_T
	@var XrmQuark CODE_T
	@var XrmQuark DD_T
	@var XrmQuark DEL_T
	@var XrmQuark DFN_T
	@var XrmQuark DIR_T
	@var XrmQuark DISPLAY_T
	@var XrmQuark DIV_T
	@var XrmQuark DL_T
	@var XrmQuark DT_T
	@var XrmQuark EM_T
	@var XrmQuark FIG_T
	@var XrmQuark FOOTNOTE_T
	@var XrmQuark FORM_T
	@var XrmQuark H1_T
	@var XrmQuark H2_T
	@var XrmQuark H3_T
	@var XrmQuark H4_T
	@var XrmQuark H5_T
	@var XrmQuark H6_T
	@var XrmQuark HEAD_T
	@var XrmQuark HR_T
	@var XrmQuark HTML_T
	@var XrmQuark IMG_T
	@var XrmQuark INPUT_T
	@var XrmQuark INS_T
	@var XrmQuark ISINDEX_T
	@var XrmQuark I_T
	@var XrmQuark KBD_T
	@var XrmQuark KEY_T
	@var XrmQuark LANG_T
	@var XrmQuark LINK_T
	@var XrmQuark LI_T
	@var XrmQuark MARGIN_T
	@var XrmQuark MATH_T
	@var XrmQuark MENU_T
	@var XrmQuark META_T
	@var XrmQuark NOTE_T
	@var XrmQuark OL_T
	@var XrmQuark OPTION_T
	@var XrmQuark PCDATA_T
	@var XrmQuark PERSON_T
	@var XrmQuark PRE_T
	@var XrmQuark P_T
	@var XrmQuark QUOTE_T
	@var XrmQuark Q_T
	@var XrmQuark SAMP_T
	@var XrmQuark SELECT_T
	@var XrmQuark SMALL_T
	@var XrmQuark STRONG_T
	@var XrmQuark STYLE_T
	@var XrmQuark SUB_T
	@var XrmQuark SUP_T
	@var XrmQuark S_T
	@var XrmQuark TABLE_T
	@var XrmQuark TAB_T
	@var XrmQuark TD_T
	@var XrmQuark TEXTAREA_T
	@var XrmQuark TH_T
	@var XrmQuark TITLE_T
	@var XrmQuark TR_T
	@var XrmQuark TT_T
	@var XrmQuark UL_T
	@var XrmQuark U_T
	@var XrmQuark VAR_T

@proc init_tagnames()
{
    ABBREV_T = XrmStringToQuark("ABBREV");
    ABSTRACT_T = XrmStringToQuark("ABSTRACT");
    ACRONYM_T = XrmStringToQuark("ACRONYM");
    ADDRESS_T = XrmStringToQuark("ADDRESS");
    AU_T = XrmStringToQuark("AU");
    A_T = XrmStringToQuark("A");
    BASE_T = XrmStringToQuark("BASE");
    BIG_T = XrmStringToQuark("BIG");
    BLOCKQUOTE_T = XrmStringToQuark("BLOCKQUOTE");
    BODY_T = XrmStringToQuark("BODY");
    BQ_T = XrmStringToQuark("BQ");
    BR_T = XrmStringToQuark("BR");
    B_T = XrmStringToQuark("B");
    CAPTION_T = XrmStringToQuark("CAPTION");
    CHANGED_T = XrmStringToQuark("CHANGED");
    CITE_T = XrmStringToQuark("CITE");
    CODE_T = XrmStringToQuark("CODE");
    DD_T = XrmStringToQuark("DD");
    DEL_T = XrmStringToQuark("DEL");
    DFN_T = XrmStringToQuark("DFN");
    DIR_T = XrmStringToQuark("DIR");
    DISPLAY_T = XrmStringToQuark("DISPLAY");
    DIV_T = XrmStringToQuark("DIV");
    DL_T = XrmStringToQuark("DL");
    DT_T = XrmStringToQuark("DT");
    EM_T = XrmStringToQuark("EM");
    FIG_T = XrmStringToQuark("FIG");
    FOOTNOTE_T = XrmStringToQuark("FOOTNOTE");
    FORM_T = XrmStringToQuark("FORM");
    H1_T = XrmStringToQuark("H1");
    H2_T = XrmStringToQuark("H2");
    H3_T = XrmStringToQuark("H3");
    H4_T = XrmStringToQuark("H4");
    H5_T = XrmStringToQuark("H5");
    H6_T = XrmStringToQuark("H6");
    HEAD_T = XrmStringToQuark("HEAD");
    HR_T = XrmStringToQuark("HR");
    HTML_T = XrmStringToQuark("HTML");
    IMG_T = XrmStringToQuark("IMG");
    INPUT_T = XrmStringToQuark("INPUT");
    INS_T = XrmStringToQuark("INS");
    ISINDEX_T = XrmStringToQuark("ISINDEX");
    I_T = XrmStringToQuark("I");
    KBD_T = XrmStringToQuark("KBD");
    KEY_T = XrmStringToQuark("KEY");
    LANG_T = XrmStringToQuark("LANG");
    LINK_T = XrmStringToQuark("LINK");
    LI_T = XrmStringToQuark("LI");
    MARGIN_T = XrmStringToQuark("MARGIN");
    MATH_T = XrmStringToQuark("MATH");
    MENU_T = XrmStringToQuark("MENU");
    META_T = XrmStringToQuark("META");
    NOTE_T = XrmStringToQuark("NOTE");
    OL_T = XrmStringToQuark("OL");
    OPTION_T = XrmStringToQuark("OPTION");
    PCDATA_T = XrmStringToQuark("#PCDATA");
    PERSON_T = XrmStringToQuark("PERSON");
    PRE_T = XrmStringToQuark("PRE");
    P_T = XrmStringToQuark("P");
    QUOTE_T = XrmStringToQuark("QUOTE");
    Q_T = XrmStringToQuark("Q");
    SAMP_T = XrmStringToQuark("SAMP");
    SELECT_T = XrmStringToQuark("SELECT");
    SMALL_T = XrmStringToQuark("SMALL");
    STRONG_T = XrmStringToQuark("STRONG");
    STYLE_T = XrmStringToQuark("STYLE");
    SUB_T = XrmStringToQuark("SUB");
    SUP_T = XrmStringToQuark("SUP");
    S_T = XrmStringToQuark("S");
    TABLE_T = XrmStringToQuark("TABLE");
    TAB_T = XrmStringToQuark("TAB");
    TD_T = XrmStringToQuark("TD");
    TEXTAREA_T = XrmStringToQuark("TEXTAREA");
    TH_T = XrmStringToQuark("TH");
    TITLE_T = XrmStringToQuark("TITLE");
    TR_T = XrmStringToQuark("TR");
    TT_T = XrmStringToQuark("TT");
    UL_T = XrmStringToQuark("UL");
    U_T = XrmStringToQuark("U");
    VAR_T = XrmStringToQuark("VAR");
}



@ Forms consist of inline widgets, some of which have a callback. The
widgets are collected in a list inside a record that also contains
other information about the form (action URL and method).

@ The function |strdup_escaped| allocates a new string and copies |s|
to it, with URL-escapes where needed: characters |/?=#%| and space are
coded as |%XX|, where |XX| is the hexadeciaml code for the character.

@def hex(n) = ((n) < 10 ? (n) + '0' : (n) + 'A' - 10)

@proc char *strdup_escaped(char *s)
{
    int n;
    char *t, *t1, *s2;

    for (n = 0, s2 = s; *s2; s2++, n++)
	if (*s2 == '/' || *s2 == '+' || *s2 == '?' || *s2 == '#' ||
	    *s2 == '%' || *s2 <= ' ' || *s2 == '\177' || *s2 == ':' ||
	    *s2 == '@') n += 2;
    t1 = t = XtMalloc((n + 1) * sizeof(*t));
    for (s2 = s; *s2; s2++)
	if (*s2 == '/' || *s2 == '+' || *s2 == '?' || *s2 == '#' ||
	    *s2 == '%' || *s2 <= ' ' || *s2 == '\177' || *s2 == ':' ||
	    *s2 == '@') {
	    *t++ = '%'; *t++ = hex(*s2/16); *t++ = hex(*s2 % 16);
	} else
	    *t++ = *s2;
    *t = '\0';
    return t1;
}

@ The |submit_cb| function is a callback that is attached to a
submit-button. It will collect the values of all the input fields in
the form of which it is part and create a URL. It then calls the
|activate| callback with this URL.

@proc submit_cb($, XtPointer client_data, XtPointer call_data)
{
    FormField h, field = (FormField) client_data;
    Form f = field->form;
    Widget html = XtParent($);
    XfwfActivateFormInfo info;
    char *data, *value, *value2;
    Boolean set;
    int n, *positions, nrpositions, i;

    /* Estimate size of data */
    n = f->method == 2 ? 0 : strlen(f->action) + 1;
    if (field->name[0])				/* Submit-button itself */
	n += strlen(field->name) + 3 * strlen(field->value) + 2;
    for (h = f->fields; h; h = h->next) {
	switch (h->type) {
	case Text: case Password: case Textarea:
	    XtVaGetValues(h->w, XmNvalue, &value, NULL);
	    n += strlen(h->name) + 3 * strlen(value) + 2;
	    break;
	case Isindex:
	    XtVaGetValues(h->w, XmNvalue, &value, NULL);
	    n += 3 * strlen(value) + 2;
	    break;
	case Hidden:
	    n += strlen(h->name) + strlen(h->value) + 2;
	    break;
	case Checkbox: case Radio:
	    n += strlen(h->name) + 4;
	    break;
	case Select:
	    XmListGetSelectedPos(h->w, &positions, &nrpositions);
	    for (i = 0; i < nrpositions; i++)
		n += strlen(h->name) + 3 * strlen(h->options[i]) + 2;
	    break;
	case Submit: case Reset:
	    break;
	default:
	    assert(! "Cannot happen");
	}
    }

    /* Now construct the data */
    data = XtMalloc(n * sizeof(*data));
    if (f->method == 2) {			/* POST */
	data[0] = '\0';
    } else {					/* GET */
	strcpy(data, f->action);
	strcat(data, "?");
    }
    if (field->name[0]) {			/* Submit-button itself */
	strcat(data, field->name);
	strcat(data, "=");
	value2 = strdup_escaped(field->value);
	strcat(data, value2);
	XtFree(value2);
	strcat(data, "&");
    }
    for (h = f->fields; h; h = h->next) {
	switch (h->type) {
	case Text: case Password: case Textarea:
	    XtVaGetValues(h->w, XmNvalue, &value, NULL);
	    strcat(data, h->name);
	    strcat(data, "=");
	    value2 = strdup_escaped(value);
	    strcat(data, value2);
	    XtFree(value2);
	    if (h->next) strcat(data, "&");
	    break;
	case Isindex:
	    XtVaGetValues(h->w, XmNvalue, &value, NULL);
	    value2 = strdup_escaped(value);
	    strcat(data, value2);
	    XtFree(value2);
	    if (h->next) strcat(data, "&");
	    break;
	case Hidden:
	    strcat(data, h->name);
	    strcat(data, "=");
	    value2 = strdup_escaped(h->value);
	    strcat(data, value2);
	    XtFree(value2);
	    if (h->next) strcat(data, "&");
	    break;
	case Checkbox:
	    XtVaGetValues(h->w, XmNset, &set, NULL);
	    if (set) {
		strcat(data, h->name);
		strcat(data, "=on");
		if (h->next) strcat(data, "&");
	    }
	    break;
	case Radio:
	    XtVaGetValues(h->w, XmNset, &set, NULL);
	    if (set) {
		strcat(data, h->name);
		strcat(data, "=");
		strcat(data, h->value);
		if (h->next) strcat(data, "&");
	    }
	    break;
	case Select:
	    XmListGetSelectedPos(h->w, &positions, &nrpositions);
	    for (i = 0; i < nrpositions; i++) {
		strcat(data, h->name);
		strcat(data, "=");
		value2 = strdup_escaped(h->options[i]);
		strcat(data, value2);
		XtFree(value2);
		strcat(data, "&");
	    }
	    break;
	case Submit: case Reset:
	    break;
	default:
	    assert(! "Cannot happen");
	}
    }
    if (f->method == 2) {			/* POST */
	info.url = f->action;
	info.data = data;
	info.length = strlen(data);
	info.method = f->method;
    } else {					/* GET */
	info.url = data;
	info.data = NULL;
	info.length = 0;
	info.method = f->method;
    }
    XtCallCallbackList($, $html$activateForm, &info);
    dispose(data);
}

@proc reset_cb($, XtPointer client_data, XtPointer call_data)
{
    FormField h, field = (FormField) client_data;
    Form f = field->form;
    int i;

    for (h = f->fields; h; h = h->next) {
	switch (h->type) {
	case Text:
	    XtVaSetValues(h->w, XmNvalue, h->value, NULL);
	    break;
	case Isindex:
	    XtVaSetValues(h->w, XmNvalue, "", NULL);
	    break;
	case Password:
	    XtVaSetValues(h->w, XmNvalue, NULL, NULL);
	    break;
	case Checkbox:
	    XtVaSetValues(h->w, XmNset, h->checked, NULL);
	    break;
	case Radio:
	    XtVaSetValues(h->w, XmNset, h->checked, NULL);
	    break;
	case Textarea:
	    XtVaSetValues(h->w, XmNvalue, h->value, NULL);
	    break;
	case Select:
	    for (i = 0; i < h->nroptions; i++)
		if (h->options[i])
		    XmListSelectPos(h->w, i, FALSE);
		else
		    XmListDeselectPos(h->w, i);
	    break;
	case Hidden: case Submit: case Reset:
	    break;
	default:
	    assert(! "Cannot happen");
	}
    }
}

@ |radio_cb| is called when the user clicks on a radio button. Its
task is to check that exactly one of the radio buttons with the same
name is on. A radio button cannot be switched off, it can only be
switched off by switching another button on.

@proc radio_cb($, XtPointer client_data, XtPointer call_data)
{
    FormField h, field = (FormField) client_data;
    Form f = field->form;
    Widget html = XtParent($);
    Boolean set;

    XtVaGetValues($, XmNset, &set, NULL);
    if (! set)
	XtVaSetValues($, XmNset, TRUE, NULL);	/* Switch on again */
    else
	for (h = f->fields; h; h = h->next)
	    if (h != field && h->type == Radio
		&& XmuCompareISOLatin1(h->name, field->name) == 0)
		XtVaSetValues(h->w, XmNset, FALSE, NULL);
}

@ |start_form| creates a new form record and adds it to the list of
forms. The list must be maintained, so that it can be deallocated when
the widget is destroyed.

@proc start_form($, Attrib attr)
{
    Form f;
    char method[15];

    new(f);
    f->next = $formlist; $formlist = f;
    if (! $get_attrib($, attr, "ACTION", f->action, sizeof(f->action))) {
	f->action[0] = '\0';
	strncat(f->action, $base, sizeof(f->action) - 1);
    }
    if ($get_attrib($, attr, "METHOD", method, sizeof(method))
	&& XmuCompareISOLatin1(method, "POST") == 0)
	f->method = 2;
    else
	f->method = 0;
    f->fields = NULL;
}

@proc end_form($)
{
    /* Nothing to do */
}

@ |start_form_input| creates a button or text field and adds it to the
current form. If the button is a submit or reset button, a callback is
added.

@proc start_form_input($, Attrib attr)
{
    FormField f;
    char type[15], checked[1], size[15], maxlength[15];
    int width;
    TextStyle styleflags;

    if (! $formlist) return;			/* Not inside FORM */

    new(f);
    f->next = $formlist->fields; $formlist->fields = f;
    f->form = $formlist;			/* Back pointer */

    if (! $get_attrib($, attr, "TYPE", type, sizeof(type))) f->type = Text;
    else if (! XmuCompareISOLatin1(type, "TEXT")) f->type = Text;
    else if (! XmuCompareISOLatin1(type, "SUBMIT")) f->type = Submit;
    else if (! XmuCompareISOLatin1(type, "RESEt")) f->type = Reset;
    else if (! XmuCompareISOLatin1(type, "PASSWORD")) f->type = Password;
    else if (! XmuCompareISOLatin1(type, "CHECKBOX")) f->type = Checkbox;
    else if (! XmuCompareISOLatin1(type, "RADIO")) f->type = Radio;
    else if (! XmuCompareISOLatin1(type, "HIDDEN")) f->type = Hidden;
    else f->type = Text;

    if (! $get_attrib($, attr, "NAME", f->name, sizeof(f->name)))
	f->name[0] = '\0';

    if (! $get_attrib($, attr, "VALUE", f->value, sizeof(f->value)))
	f->value[0] = '\0';

    f->checked = $get_attrib($, attr, "CHECKED", checked, sizeof(checked));

    if ($get_attrib($, attr, "SIZE", size, sizeof(size))
	&& sscanf(size, "%u", &width) == 1)
	width *= 10;
    else
	width = 200;

    if (! $get_attrib($, attr, "MAXLENGTH", maxlength, sizeof(maxlength)))
	strcpy(maxlength, "1024");

    switch (f->type) {
    case Text:
	f->w = XtVaCreateManagedWidget
	    (f->name, xmTextFieldWidgetClass, $,
	     XmNwidth, width, XmNvalue, f->value, NULL);
	break;
    case Password:
	f->w = XtVaCreateManagedWidget
	    (f->name, xmTextFieldWidgetClass, $,
	     XmNwidth, width, NULL);
	/* This needs work: text must be masked */
	break;
    case Checkbox:
	f->w = XtVaCreateManagedWidget
	    (f->name, xmToggleButtonWidgetClass, $,
	     XmNindicatorOn, TRUE,
	     XmNvisibleWhenOff, TRUE,
	     XmNset, f->checked,
	     XtVaTypedArg, XmNlabelString, XtRString, "", 1,
	     XmNindicatorType, XmN_OF_MANY, NULL);
	break;
    case Radio:
	f->w = XtVaCreateManagedWidget
	    (f->name, xmToggleButtonWidgetClass, $,
	     XmNindicatorOn, TRUE,
	     XmNvisibleWhenOff, TRUE,
	     XmNset, f->checked,
	     XtVaTypedArg, XmNlabelString, XtRString, "", 1,
	     XmNindicatorType, XmONE_OF_MANY, NULL);
	XtAddCallback(f->w, XmNvalueChangedCallback, radio_cb, f);
	break;
    case Submit:
	if (!f->value[0]) strcpy(f->value, "Submit");
	f->w = XtVaCreateManagedWidget
	    (f->name, xmPushButtonWidgetClass, $,
	     XtVaTypedArg, XmNlabelString, XtRString,
	     f->value, strlen(f->value) + 1, NULL);
	XtAddCallback(f->w, XmNactivateCallback, submit_cb, f);
	break;
    case Reset:
	if (!f->value[0]) strcpy(f->value, "Reset");
	f->w = XtVaCreateManagedWidget
	    (f->name, xmPushButtonWidgetClass, $,
	     XtVaTypedArg, XmNlabelString, XtRString,
	     f->value, strlen(f->value) + 1, NULL);
	XtAddCallback(f->w, XmNactivateCallback, reset_cb, f);
	break;
    case Hidden:
	f->w = NULL;
	break;
    default:
	assert(! "Cannot happen");
    }
    if (f->type != Hidden) {
	styleflags = $stystack->textstyle;
	styleflags &= ~(FtALIGNFIXEDWIDTH|FtALIGNFIXEDHEIGHT);
	styleflags |= FtALIGNMIDDLE;
	$add_inline($, f->w, styleflags, 0, 0, 0,
		    $stystack->inlinehmargin, $stystack->inlinevmargin,
		    $extradata[$stystack->data]);
    }
}

@proc end_form_input($)
{
    /* Nothing to do */
}

@proc start_form_textarea($, Attrib attr)
{
    FormField f;
    char h[15];
    int rows, cols;
    TextStyle styleflags;

    if (! $formlist) return;			/* Not inside FORM */

    new(f);
    f->next = $formlist->fields; $formlist->fields = f;
    f->form = $formlist;			/* Back pointer */
    f->type = Textarea;
    f->value[0] = '\0';
    if (! $get_attrib($, attr, "NAME", f->name, sizeof(f->name)))
	f->name[0] = '\0';
    if ($get_attrib($, attr, "ROWS", h, sizeof(h))
	&& sscanf(h, "%u", &rows) == 1)
	rows *= 14;
    else
	rows = 100;
    if (! $get_attrib($, attr, "COLS", h, sizeof(h))
	&& sscanf(h, "%u", &cols) == 1)
	cols *= 10;
    else
	cols = 200;

    f->w = XtVaCreateManagedWidget
	    (f->name, xmTextWidgetClass, $,
	     XmNwidth, cols, XmNheight, rows,
	     XmNeditMode, XmMULTI_LINE_EDIT, NULL);
    styleflags = $stystack->textstyle;
    styleflags &= ~(FtALIGNFIXEDWIDTH|FtALIGNFIXEDHEIGHT);
    styleflags |= FtALIGNMIDDLE;
    $add_inline($, f->w, styleflags, 0, 0, 0,
		$stystack->inlinehmargin, $stystack->inlinevmargin,
		$extradata[$stystack->data]);
}

@proc end_form_textarea($)
{
    FormField f;

    if (! $formlist || ! $formlist->fields
	|| $formlist->fields->type != Textarea) return;

    f = $formlist->fields;
    f->value[0] = '\0';
    strncat(f->value, $contentbuffer, sizeof(f->value) - 1);
}

@proc start_form_select($, Attrib attr)
{
    FormField f;
    char h[15];
    int width = 250, height = 100;
    TextStyle styleflags;
    Bool multiple;

    if (! $formlist) return;			/* Not inside FORM */

    new(f);
    f->next = $formlist->fields; $formlist->fields = f;
    f->form = $formlist;			/* Back pointer */
    f->type = Select;
    f->nroptions = 0;
    f->options = NULL;
    f->selected = NULL;
    if (! $get_attrib($, attr, "NAME", f->name, sizeof(f->name)))
	f->name[0] = '\0';
    if ($get_attrib($, attr, "WIDTH", h, sizeof(h))
	&& sscanf(h, "%u", &width) == 1)
	width *= 10;
    if (! $get_attrib($, attr, "HEIGHT", h, sizeof(h))
	&& sscanf(h, "%u", &height) == 1)
	height *= 14;
    multiple = $get_attrib($, attr, "MULTIPLE", h, sizeof(h));

    f->w = XmCreateScrolledList($, f->name, NULL, 0);
    XtVaSetValues
	(XtParent(f->w), XmNwidth, width, XmNheight, height,
	 NULL);
    XtVaSetValues
	(f->w,
	 XmNselectionPolicy, multiple ? XmMULTIPLE_SELECT : XmSINGLE_SELECT,
	 NULL);
    XtManageChild(f->w);
    styleflags = $stystack->textstyle;
    styleflags &= ~(FtALIGNFIXEDWIDTH|FtALIGNFIXEDHEIGHT);
    styleflags |= FtALIGNMIDDLE;
    $add_inline($, XtParent(f->w), styleflags, 0, 0, 0,
		$stystack->inlinehmargin, $stystack->inlinevmargin,
		$extradata[$stystack->data]);
    /* Not yet implemented */
}

@proc end_form_select($)
{
    /* Nothing to do */
}

@ At the start of an OPTION tag, the attributes are checked for the
occurrence of VALUE. If such an attribute exists, it is added to the
list of options of the form field. If the attribute doesn't exist, a
NULL pointer is added instead. At the end of the OPTION element the
contents of the element are added as an item to the List widget and
the option value is checked again. If it is NULL, it is replaced by
the contents of the element.

@proc start_form_option($, Attrib attr)
{
    char h[LITLEN];
    FormField f;

    if (! $formlist || ! $formlist->fields
	|| $formlist->fields->type != Select) return;

    f = $formlist->fields;
    f->nroptions++;
    renewarray(f->options, f->nroptions);
    renewarray(f->selected, f->nroptions);
    if ($get_attrib($, attr, "VALUE", h, sizeof(h)))
	f->options[f->nroptions-1] = XtNewString(h);
    else
	f->options[f->nroptions-1] = NULL;
    f->selected[f->nroptions-1] =
	$get_attrib($, attr, "SELECTED", h, sizeof(h));

}

@proc end_form_option($)
{
    XmString value2;
    FormField f;

    if (! $formlist || ! $formlist->fields
	|| $formlist->fields->type != Select) return;

    f = $formlist->fields;
    if (! f->options[f->nroptions-1]) {
	f->options[f->nroptions-1] = XtNewString($contentbuffer);
	trim(f->options[f->nroptions-1]);
    }
    value2 = XmStringCreate($contentbuffer, XmFONTLIST_DEFAULT_TAG);
    XmListAddItem(f->w, value2, 0);
    XmStringFree(value2);
    if (f->selected[f->nroptions-1])
	XmListSelectPos(f->w, f->nroptions-1, FALSE);
}

@ |start_isindex| creates a form with a single text field and a submit
callback attached directly to that field.

@proc start_isindex($, Attrib attr)
{
    Form form;
    FormField f;
    char prompt[LITLEN];
    int width;
    TextStyle styleflags;

    /* Create a form */
    new(form);
    form->next = $formlist; $formlist = form;
    form->method = 0;
    form->fields = NULL;
    if (! $get_attrib($, attr, "HREF", form->action, sizeof(form->action))) {
	form->action[0] = '\0';
	strncat(form->action, $base, sizeof(form->action) - 1);
    }

    /* Create a text field */
    new(f);
    f->next = form->fields; form->fields = f;
    f->form = form;				/* Back pointer */
    f->type = Isindex;
    f->w = XtVaCreateManagedWidget
	(f->name, xmTextFieldWidgetClass, $,
	 XmNwidth, $width/2, XmNvalue, f->value, NULL);
    XtAddCallback(f->w, XmNactivateCallback, submit_cb, f);

    styleflags = $stystack->textstyle;
    styleflags &= ~(FtALIGNFIXEDWIDTH|FtALIGNFIXEDHEIGHT);
    styleflags |= FtALIGNMIDDLE;
    $add_vspace($, 0);
    if (! $get_attrib($, attr, "PROMPT", prompt, sizeof(prompt)))
	$add_word($, prompt, strlen(prompt), $stystack->fg, $stystack->bg,
		  styleflags, $extradata[$stystack->data]);
    $add_inline($, f->w, styleflags, 0, 0, 0,
		$stystack->inlinehmargin, $stystack->inlinevmargin,
		$extradata[$stystack->data]);
    $add_vspace($, 0);
}

@proc end_isindex($)
{
    /* Nothing to do */
}


@ |upper| converts the passed string |s| to uppercase (replacing
the contents).

@proc String upper(String s)
{
    String t;
    if (s != NULL) for (t = s; *t; t++) *t = _toupper(*t);
    return s;
}

@ |trim| removes whitespace from the start and the end of the string
|s|.

@proc trim(char *s)
{
    int i, j;

    if (!s) return;
    for (j = 0; isspace(s[j]); j++) ;
    for (i = strlen(s); i > j && isspace(s[i-1]); i--) s[i-1] = '\0';
    if (j != 0) memmove(s, s + j, i - j + 1);
}



@IMPORTS

@incl <stdio.h>
@incl <assert.h>
@incl <ctype.h>
@incl <X11/Xmu/CharSet.h>
@incl <Xm/ToggleB.h>
@incl <Xm/TextF.h>
@incl <Xm/PushB.h>
@incl <Xm/Text.h>
@incl <Xm/List.h>
