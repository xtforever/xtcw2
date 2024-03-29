/*
* SCCS_data: %Z% %M% %I% %E% %U%
*
* Widget Creation Library - WcSetValue.c
*
* WcSetValue allows resource specifications to be used to set values on
* widgets.  The resource specifications are in this syntax:
*
*	<client_data>	::=	<res_spec>
*			|	<res_spec_list>
*
*	<res_spec>	::=	<targetName>.<resName>: <resValue>
*			|	<targetName>.<resName>(<resType>): <resValue>
*
*	<res_spec_list>	::=	( <res_spec> )
*			|	<res_spec_list> ( <res_spec> )
*
*	<targetName>	::=	<empty>
*			|	this
*			|	<relative_opt><widget_path>
*			
*
* If there are multiple resource specs, then each must be enclosed
* in parens.
*
* In other words, a resource spec looks just like a line in a resource file,
* except that it may include an optional resource type specification in
* parens, and the name of the targetWidget may be 'this' or a relative path
* as well as the standard wildcarded paths.  An empty targetName is the
* same as 'this'.  
*
* The special resource value of "this" means "this widget."  Typically,
* using "this" as the resource value is used to set the "XmNdefaultButton"
* resource on a XmbulletinBoard, "menuBar", "workArea", etc on XmMainWindows,
* the subMenuId resource on menuBar cascade buttons, and so on.
*
* The targetName must resolve to a specific widget: therefore the last
* component of the targetName must be the name of a Widget instance, and
* the targetName MUST be followed by a '.' and NOT a '*'.
*
* Caution: only use resource_type when you REALLY must, as this is not safe.
* With sub-resources you will need it.
*
*******************************************************************************
*/

#include <X11/IntrinsicP.h>

#ifdef sun
#include <X11/ObjectP.h>	/* why don't they just use X from mit!?! */
#include <X11/RectObjP.h>
#endif

#include <X11/StringDefs.h>
#include "WcCreateP.h"

/*  -- Usage message for WcSetValue and all functions it calls
*******************************************************************************
*/
static char* svUsage = "\n\
    Usage: WcSetValue( target_widget_name.res_name: res_value )\n\
       Or: WcSetValue( target_widget_name.res_name(res_type): res_value ) \n\
       Or: WcSetValue( ( resSpec ) (resSpec) ... )";

/*
*******************************************************************************
**  Private Routines for parsing Resource Specification into its Components
*******************************************************************************
*/

/*  -- Error Messages and Error Return from WcxResSpecFindSegments()
*******************************************************************************
    The BARF macro invokes the error message print function, cleans up
    return arguments, and returns from WcxResSpecFindSegments().  Note 
    that the invokation of the BARF macro should NOT be followed by a
    semi-colon.

    The BARF macro and the WcxBarf() function just make it easier to read
    WcxResSpecFindSegments().
*/

#define BARFED 1
#define NO_PROBLEMS 2

#define BARF(barfId) { \
    WcxBarf(widget, barfId, resSpec);					\
    *nameStart = *nameEnd = *resStart = *resEnd = NULL;			\
    *typeStart = *typeEnd = *valueStart = *valueEnd = *next = NULL;	\
    return BARFED;							\
}

static void WcxBarf( w, id, resSpec )
    Widget w;
    int	   id;
    String resSpec;
{
    switch (id) {
    
#define FOUND_NUL 1
    case FOUND_NUL:
	WcWARN2( w, "WcSetValue", "foundNul",
		"Wcl Warning: WcSetValue(%s) - Empty resource spec.%s",
		resSpec, svUsage );
	break;

#define MISSING_NAME 2
    case MISSING_NAME:
	WcWARN2( w, "WcSetValue", "missingName",
		"Wcl Warning: WcSetValue(%s) - Missing target widget name.%s",
		resSpec, svUsage );
	break;

#define MISSING_RES_END 3
    case MISSING_RES_END:
	WcWARN2( w, "WcSetValue", "noResEnd",
		"Wcl Warning: WcSetValue(%s) - Could not find ':' or '('.%s",
		resSpec, svUsage );
	break;

#define NO_WS_IN_TYPE 4
    case NO_WS_IN_TYPE:
	WcWARN2( w, "WcSetValue", "noWsInType",
		"Wcl Warning: WcSetValue(%s) - Type specifier cannot have whitespace.%s",
		resSpec, svUsage );
	break;

#define EMPTY_TYPE 5
    case EMPTY_TYPE:
	WcWARN2( w, "WcSetValue", "emptyType",
		"Wcl Warning: WcSetValue(%s) - Type specifier is empty.%s",
		resSpec, svUsage );
	break;

#define MISSING_COLON 6
    case MISSING_COLON:
	WcWARN2( w, "WcSetValue", "missingColon",
		"Wcl Warning: WcSetValue(%s) - Expected ':' immediately following ')'.%s",
		resSpec, svUsage );
	break;

#define UNBAL_PARENS 7
    case UNBAL_PARENS:
	WcWARN2( w, "WcSetValue", "unbalParens",
		"Wcl Warning: WcSetValue(%s) - End-of-resSpec before outermost ')'.%s", 
		resSpec, svUsage );
	break;

#define NEEDS_PARENS 8
    case NEEDS_PARENS:
	WcWARN2( w, "WcSetValue", "needsParens",
		"Wcl Warning: WcSetValue(%s) - Multiple resSpecs must be in parens.%s", 
		resSpec, svUsage );
	break;

#define TARGET_NOT_FOUND 9
    case TARGET_NOT_FOUND:
	WcWARN2( w, "WcSetValue", "targetNotFound",
		"Wcl Warning: WcSetValue(%s) - Target widget not found.%s",
		resSpec, svUsage );
	break;

#define NO_SUCH_RESOURCE 10
    case NO_SUCH_RESOURCE:
	WcWARN2( w, "WcSetValue", "noSuchResource",
		"Wcl Warning: WcSetValue(%s) - Cannot determine type of this resource.%s",
		resSpec, svUsage );
	break;

    default:
	WcWARN1( w, "WcSetValue", "unknownBarf",
		"Wcl Warning: WcSetValue(%s) - Barfed with unknown id", resSpec );
	break;
    }
}


/*  -- Find the fields within a resource spec
*******************************************************************************
    This function does all the messy character-by-character parsing
    required by WcxResSpecParse().

    The start and end arguments point into the resSpec argument.  The
    next argument is either NULL (if nothing more to parse) or points to
    the beginning '(' of the next resourceSpec.

    The return value is BARFED if error, NO_PROBLEMS otherwise.
*/

static int WcxResSpecFindSegments( widget, resSpec, 
		   nameStart, nameEnd, resStart, resEnd, 
		   typeStart, typeEnd, valueStart, valueEnd, next )
Widget widget;			/* needed for warning messages */
char *resSpec;			/* input resource specification */
char **nameStart,  **nameEnd;	/* output target widget name */
char **resStart,   **resEnd;	/* output resource name */
char **typeStart,  **typeEnd;	/* output optional type name */
char **valueStart, **valueEnd;	/* output lead/trailing whitespace stripped */
char **next;			/* output if (next) *next=='(' */
{
    register char* cp;
    int            inParens = 0;

    *nameStart = *nameEnd = *resStart   = *resEnd   = NULL;
    *typeStart = *typeEnd = *valueStart = *valueEnd = NULL;

    /* Skip initial whitespace and optional leading left paren.
    ** If we see a leading paren, then we are doing a res_spec_list
    ** so there may be more parenthesized resSpec's following this one.
    */
    for ( cp = resSpec ; *cp && (*cp <= ' ' || *cp == '(') ; cp++ )
	if (*cp == '(')
	    inParens++;	/* skip initial whitespace */

    if ( *cp == NUL )
	BARF(FOUND_NUL)

    *nameStart = cp;

    /* WE KNOW nameStart
    ** Find the '(' or ':' following the resource.  Every time we see
    ** a '.' or a relative naming operator set nameEnd and resStart
    */
    for ( ; *cp > ' ' && *cp != '(' && *cp != ':' ; cp++ )
    {
	if ( *cp == '.' )
	{
	    *nameEnd  = cp-1;
	    *resStart = cp+1;
	}
	else if ( *cp == '~' || *cp == '^' )
	{
	    *nameEnd  = cp;
	    *resStart = cp+1;
	}
    }
    if (*nameEnd == NULL)
	BARF(MISSING_NAME)
    if ( *cp <= ' ' )
	BARF(MISSING_RES_END)
    *resEnd = cp-1;

    /* WE KNOW nameStart nameEnd resStart resEnd
    ** ASSERT ( *cp == ':' || *cp == '(' )
    */
    if ( *cp == ':' )
    {
	*typeStart = *typeEnd = NULL;
    }
    else
    {
	for ( *typeStart = ++cp ; ' ' < *cp && *cp != ')' ; cp++ )
	    /*EMPTY*/;
	*typeEnd = cp-1;

	if ( *cp <= ' ')
	    BARF(NO_WS_IN_TYPE)
	if ( *typeStart > *typeEnd )
	    BARF(EMPTY_TYPE)
	if ( *++cp != ':' )
	    BARF(MISSING_COLON)
    }

    /* WE KNOW nameStart nameEnd resStart resEnd typeStart typeEnd
    ** ASSERT ( *cp == ':' )
    */

    for ( cp++ ; *cp && *cp <= ' ' ; cp++ )
	/*EMPTY*/;
    *valueStart = cp;

    if (inParens)
    {
	for ( ; *cp && inParens ; cp++ )
	{
	    if ( *cp == '(')
		inParens++;
	    else if ( *cp == ')')
		inParens--;
	}
	if (inParens)
	    BARF(UNBAL_PARENS)

	*next = cp;			/* char AFTER  outer ')' */

	/* strip trailing whitespace
	*/
	for ( cp -= 2 ; *cp <= ' ' ; cp-- )
	    /*EMPTY*/;
	*valueEnd = cp;

	/* next will be NULL or the 1st paren of the next resSpec
	*/
	for ( ; **next && **next <= ' ' ; ++*next )
	    /*EMPTY*/;
	if (**next == NUL)
	    *next = NULL;
	else if (**next != '(')
	    BARF(NEEDS_PARENS)
    }
    else
    {
	for ( ; *cp ; cp++ )
	    /*EMPTY*/;
	for ( cp-- ; *cp <= ' ' ; cp-- )
	    /*EMPTY*/;
	*valueEnd = cp;
	*next = NULL;		/* nothing can come after */
    }
    return NO_PROBLEMS;
}
 
/*  -- Find and alloc space for copies of the fields within a resSpec
*******************************************************************************
    The return value is NO_PROBLEMS or BARFED.

    The allocated return arguments must be free'd with XtFree().  Any NULL
    arguments should not be free'd of course.  If the next argument is not
    NULL, use it as the next resSpec.
*/

static int WcxResSpecParse( widget, resSpec, nam, res, typ, val, next )
    Widget widget;		/* needed for warning messages		*/
    char* resSpec;		/* input  name.res(resType): resVal	*/
    char** nam;			/* output target widget name		*/
    char** res;			/* output resource name			*/
    char** typ;			/* output resource type			*/
    char** val;			/* output resource value		*/
    char** next;		/* output **next=='(' or *next==NULL	*/
{
    int   status;
    char *namStart, *namEnd, *resStart, *resEnd,
	 *typStart, *typEnd, *valStart, *valEnd;

    *nam = *res = *typ = *val = *next = (char*)0;

    status = WcxResSpecFindSegments( widget, resSpec, 
				    &namStart, &namEnd, &resStart, &resEnd, 
				    &typStart, &typEnd, &valStart, &valEnd,
				    next );
    if (status == BARFED)
	return BARFED;

#define WCResSpecField( tla, tlaStart, tlaEnd ) \
    *tla = (char*)XtCalloc( tlaEnd - tlaStart + 2,  sizeof(char) ); \
    strncpy( *tla, tlaStart, (tlaEnd - tlaStart + 1) * sizeof(char) );

    WCResSpecField(nam,namStart,namEnd)

    WCResSpecField(res,resStart,resEnd)

    if (typStart)
    {
	WCResSpecField(typ,typStart,typEnd)
    }
    else
	*typ = NULL;

    if (valStart <= valEnd)
    {
	WCResSpecField(val,valStart,valEnd)
    }
    else
	*val = XtNewString("");
#undef WCResSpecField

    return status;
}

/*  -- Set all resources using ParseResSpecs
*******************************************************************************
    The function is derived from WcSetValueFromStringAndType() but it
    sets all values together when multiple values are specified for the
    same widget.  This helps when setting values on constraint widgets,
    for example, the attachWidget and attachment resources on XmForm widgets.
*/

typedef struct _ParsedResSpec {
    Widget	targetWidget;	/* WcFullNameToWidget(target)		*/
    char*	target;		/* name of widget to change		*/
    char*	res;		/* name of widget resource to change	*/
    char*	type;		/* safer to get res type from widget	*/
    char*	value;		/* can be NULL				*/
} ParsedResSpecRec, *ParsedResSpec;
#define MAX_RES_SPECS 128

static void WcxSetValuesFromParseResSpecs( spec, numSpecs )
    ParsedResSpec	spec;		/* array of ParsedResSpecRecs	*/
    int			numSpecs;	/* the number of res specs	*/
{
    XrmValue    fr_val;
    XrmValue    to_val;
    Arg		arg[MAX_RES_SPECS];
    int		argc;
    char*	mallocList[MAX_RES_SPECS];
    int		malc;
    int		inx;
    int		thisFirstInx = 0;
    Widget	this;

    while ( 1 )
    {
	/* Find the first widget we have not yet processed (We assign NULL to
	** targetWidget after we've converted its values).
	*/
	while ( NULL == (this = spec[thisFirstInx].targetWidget) )
	    if ( numSpecs == ++thisFirstInx )
		return;			/* no more, all done. */

	inx = thisFirstInx;

	/* reset which Arg[] we write converted values into,
	** and the mallocList we save malloc'd pointers in.
	*/
	argc = malc = 0;

	/* Now go through list of specs which apply to this widget,
	** and convert values from strings into appropriate types, if
	** the required type is not String.
	*/
	do
	{
	    if ( WcStrEq( spec[inx].type, XtRString ) )
	    {
		XtSetArg( arg[argc], spec[inx].res, (caddr_t)spec[inx].value );
		argc++;
	    }
	    else
	    {
		fr_val.size = WcStrLen(spec[inx].value) + 1;
		fr_val.addr = (caddr_t)spec[inx].value;

		/* Force the converter to supply the storage
		*/
		to_val.size = 0;
		to_val.addr = NULL;

		XtConvert(
			this,		/* the widget		*/
			XtRString,	/* from type		*/
			&fr_val,	/* from value		*/
			spec[inx].type,	/* to type		*/
			&to_val		/* the converted value	*/
		);

		if (to_val.addr)
		{
		    /* Conversion worked.  Copy value byte by byte.
		     * Cannot trust that to_val.addr is pointer to char,
		     * it may be an opaque pointer (i.e., to_val.addr[1]
		     * may increment ptr by address size, not byte size).
		     */
		    int byte;
		    char* buf;
		    char* converted_bytes = (char*)to_val.addr;
		    buf = mallocList[malc++] = XtMalloc( to_val.size );
		    for ( byte = 0 ; byte < to_val.size ; byte++ )
			buf[byte] = converted_bytes[byte];

		    switch(to_val.size)
		    {
		    case sizeof(char):
			XtSetArg( arg[argc], spec[inx].res, *(char*)buf );
			break;
#ifndef CRAY
		    case sizeof(short):
			XtSetArg( arg[argc], spec[inx].res, *(short*)buf );
			break;
#endif
		    case sizeof(int):
			XtSetArg( arg[argc], spec[inx].res, *(int*)buf );
			break;
#ifdef __alpha
		    case sizeof(long):
			XtSetArg( arg[argc], spec[inx].res, *(long*)buf );
			break;
#endif
		    default:
			XtSetArg( arg[argc], spec[inx].res, buf );
		    }
		    argc++;
		}
		/* If the conversion fails, the converter issues a warning.
		*/
	    }
	    /* Remember we have already dealt with this spec.
	    */
	    spec[inx].targetWidget = NULL;

	    /* Look for next spec which applies to this widget
	    */
	    for ( inx++ ; inx < numSpecs ; inx++ )
		if ( this == spec[inx].targetWidget )
		    break;

	    /* Keep converting all res specs for this widget.
	    */
	} while ( inx < numSpecs );

	/* Set all the values on this widget.
	*/
        XtSetValues( this, arg, argc );

	/* Free copies of converted data in mallocList[]
	*/
	for ( malc-- ; -1 < malc ; malc-- )
	    XtFree( mallocList[malc] );

	/* Now do the next widget in the list of resource specs
	*/
    }
}

/*
*******************************************************************************
** Public Functions
*******************************************************************************
*/

/*  -- Set Resource Value on Widget
*******************************************************************************
    This function sets a resource value on the named widget.  The named widget
    can be relative to the reference widget.  A type may optionally be provided.
*/

void WcSetValue( w, resSpec )
    Widget  w;  
    char*   resSpec;
{
    ParsedResSpecRec spec[MAX_RES_SPECS];
    int		inx = 0;

    /* dynamically alloc'd so must be XtFree'd if not NULL */
    char*	target;		/* name of widget to change		*/
    char*	res;		/* name of widget resource to change	*/
    char*	type;		/* safer to get res type from widget	*/
    char*	value;		/* can be NULL				*/

    Widget	targetWidget;	/* the widget to be changed		*/

    char*	next;		/* there may be multiple resource specs */
    int		stat;		/* NO_PROBLEMS or BARFED		*/

    while (resSpec && inx < MAX_RES_SPECS-1 )
    {
	stat = WcxResSpecParse(w, resSpec, &target, &res, &type, &value, &next);

	/* Error messages given by WcxResSpecFindSegments() via WcxBarf().
	** We cannot continue if the parser barfs, we CAN continue if
	** there are semantic problems detected here.
        */
        if ( stat == BARFED )
	{
	    XtFree(target);
	    XtFree(res);
	    XtFree(type);
	    XtFree(value);
	    goto free_all_specs;
	}

        /* See if we can find the target widget 
	*/
        if ( NULL == (targetWidget = WcFullNameToWidget( w, target )) )
        {
	    WcxBarf( w, TARGET_NOT_FOUND, resSpec );
	    XtFree(target);
	    XtFree(res);
	    XtFree(type);
	    XtFree(value);
	    goto do_next_res_spec;
        }

        /* If resType not specified, see if widget has this resource type
	*/
        if ( type == NULL )
	{
	    if (! (type = WcGetResourceType( targetWidget, res )) )
	    {
		WcxBarf( w, NO_SUCH_RESOURCE, resSpec );
		XtFree(target);
		XtFree(res);
		XtFree(value);
		goto do_next_res_spec;
	    }
        }

	/* We have the type, so we can convert value and set value on target.
	*/

	spec[inx].targetWidget	= targetWidget;
	spec[inx].target	= target;
	spec[inx].res		= res;
	spec[inx].type		= type;
	spec[inx].value		= value;
	inx++;

do_next_res_spec:
	/* NB: inx is NOT incremented if resSpec could not be processed
	*/
	resSpec = next;
    }

    /* All the resource specs have now been parsed.  Do a single
    ** XtSetValues for each widget which has been named.
    ** NB: if inx is still 0, then no resource specs could be parsed,
    ** so do not try to set anything.
    */

    if ( 0 < inx )
	WcxSetValuesFromParseResSpecs( spec, inx );

free_all_specs:

    /* All the strings have been XtMallocd and so must be XtFreed
    */
    for ( inx-- ; -1 < inx ; inx-- )
    {
	XtFree(spec[inx].target);
	XtFree(spec[inx].res);
	XtFree(spec[inx].type);
	XtFree(spec[inx].value);
    }
}

/*  -- Search widget's resource list for resource_type
*******************************************************************************
    Gets the XtResourceList from the widget, searches the list for
    the resource name to determine the type required, which is then
    returned to the caller.
*/

char* WcGetResourceType( w, res_name )
    Widget w;
    char*  res_name;
{
    XtResource* res_list;
    int         i;
    Cardinal	num;
    char*	retstr;

    XtGetResourceList( w->core.widget_class, &res_list, &num );

    for ( i = 0 ; i < num ; i++ )
    {
        if ( WcStrEq( res_name, res_list[i].resource_name) 
	 ||  WcStrEq( res_name, res_list[i].resource_class) )
	{
            retstr = XtNewString(res_list[i].resource_type);
	    XtFree( (char*)res_list );
	    return retstr;
	}
    }

    /* Did not find resource in regular resources
    */
    XtFree( (char*)res_list );

    w = XtParent( w );
    if ( (Widget)0 != w  &&  XtIsConstraint(w) )
    {
	XtGetConstraintResourceList( w->core.widget_class, &res_list, &num );

	for ( i = 0 ; i < num ; i++ )
	{
	    if ( WcStrEq( res_name, res_list[i].resource_name)
	     ||  WcStrEq( res_name, res_list[i].resource_class) )
            {
                retstr = XtNewString(res_list[i].resource_type);
                XtFree( (char*)res_list );
                return retstr;
            }
	}

	/* Did not find resource in constraint resources
	*/
	XtFree( (char*)res_list );
    }

    return NULL;
}

/*  -- Convert resource value from string to whatever the widget needs
*******************************************************************************
    Gets the XtResourceList from the widget, searches the list for
    the resource name to determine the type required, then uses the
    resource manager to convert from string to the required type.
    Calls XtSetValue with converted type.

    Note that if the widget does not have the specified resource
    type, it is not set.  WcGetResourceType() checks for both
    widget resources and constraint resources.

    Note also that no converter-failed behavior is necessary,
    because converters generally give their own error messages.
*/

void WcSetValueFromString( w, res_name, res_val )
    Widget w;		 /* MUST already be init'd */
    char*  res_name;
    char*  res_val;	/* NUL terminated, should NOT have whitespace */
{
    char*	res_type;	/* must be XtFree'd */

    /*ASSIGN_IN_IF*/
    if ( res_type = WcGetResourceType( w, res_name ) )
    {
	/* This widget does know about this resource type */
	WcSetValueFromStringAndType( w, res_name, res_val, res_type );
    }
    XtFree( res_type );
}

void WcSetValueFromStringAndType( w, res_name, res_val, res_type )
    Widget w;
    char*  res_name;
    char*  res_val;
    char*  res_type;
{
    ParsedResSpecRec spec[1];

    spec[0].targetWidget = w;
    spec[0].target = XtName(w);
    spec[0].res = res_name;
    spec[0].type = res_type;
    spec[0].value = res_val;

    WcxSetValuesFromParseResSpecs( spec, 1 );
}

/*  -- Augment or override translations
*******************************************************************************
    Makes it slightly more convenient to change translations on widgets.
    Xt itself provides a string-to-translation converter which looks at
    the #augment, #override, or #replace directives which may exist in
    a translations string, but requires the client to specifically do
    what this routine does.  Wcl needs this (WcTranslationsCB()), so I
    guess clients will too.
*/

void WcSetTranslations( w, translations )
    Widget w;
    char* translations;
{
    XtTranslations compiledTrans = XtParseTranslationTable( translations );

    if ( 0 == WcStrCmpN( translations, "#augment", 8 ) )

	XtAugmentTranslations( w, compiledTrans );

    else if ( 0 == WcStrCmpN( translations, "#override", 9 ) )

	XtOverrideTranslations( w, compiledTrans );

    else
    {
	Arg args[1];
	XtSetArg( args[0], XtNtranslations, compiledTrans );
	XtSetValues( w, args, 1 );
    }
}
