#if ( !defined(lint) && !defined(SABER) )
static char Xrcsid[] = "$XConsortium: EzME.c,v 1.6 89/12/11 15:20:07 kit Exp $";
#endif 

/*
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * EzME.c - Source code for the generic menu entry
 *
 * Date:    September 26, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/XawInit.h>
#include <X11/Xaw/Cardinals.h>

#include <Xfwf/EzMEP.h>
#include <Xfwf/EzMenuP.h>

#include <ctype.h>

#define offset(field) XtOffset(XfwfEzMEWidget, ezme.field)
static XtResource resources[] = {

  {XtNaction, XtCAction, XtRString, sizeof(caddr_t),
     offset(action), XtRString, (caddr_t)NULL},

  {XtNmenu, XtCMenu, XtRWidget, sizeof(Widget), 
     offset(toPopUp), XtRWidget, (caddr_t) NULL }
};   
#undef offset

static char haveSubMenuTransString[] =
    "<EnterWindow>:     highlight()             \n\
     <LeaveWindow>:     unhighlight() \n\
     <BtnMotion>:       highlight()             \n\
     <BtnUp>:           notify() unhighlight() "; 

static XtTranslations haveSubMenuTrans;

static char haveNoSubMenuTransString[] =
    "<EnterWindow>:     highlight()             \n\
     <LeaveWindow>:     unhighlight() MenuPopdown() \n\
     <BtnMotion>:       highlight()             \n\
     <BtnUp>:           notify() unhighlight()"; 

static XtTranslations haveNoSubMenuTrans;

#if (!NeedFunctionPrototypes)

static void ClassInitialize();
static void Initialize();
static void Destroy();
static Boolean SetValues();
static void Notify();
static void Highlight();
static void UnHighlight();

#else

static void ClassInitialize(void);
static void Initialize(Widget request, Widget new);
static void Destroy(Widget w);
static Boolean SetValues(Widget current, Widget request, Widget new);
static void Notify(Widget w);
static void Highlight(Widget w, XEvent *event, String *params,
		      Cardinal *num_params);
static void UnHighlight(Widget w, XEvent *event, String *params,
			Cardinal *num_params);

#endif

#define superclass (&smeBSBClassRec)

XfwfEzMEClassRec xfwfEzMEClassRec = {
  {
    /* superclass         */    (WidgetClass) superclass,
    /* class_name         */    "XfwfEzME",
    /* size               */    sizeof(XfwfEzMERec),
    /* class_initialize   */	ClassInitialize,
    /* class_part_initialize*/	NULL,
    /* Class init'ed      */	FALSE,
    /* initialize         */    (XtInitProc)Initialize,
    /* initialize_hook    */	NULL,
    /* realize            */    NULL,
    /* actions            */    NULL,
    /* num_actions        */    ZERO,
    /* resources          */    resources,
    /* resource_count     */	XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE, 
    /* compress_exposure  */    FALSE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    NULL,
    /* expose             */    XtInheritExpose,
    /* set_values         */    (XtSetValuesFunc)SetValues,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook    */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		  */    XtInheritTranslations,
    /* query_geometry	  */    XtInheritQueryGeometry,
    /* display_accelerator*/    NULL,
    /* extension	  */    NULL
  },{
    /* Simple Menu Entry Fields */
      
    /* highlight */             Highlight,
    /* unhighlight */           UnHighlight,
    /* notify */		Notify,		
    /* extension */             NULL				
  },
  {
    /* extension */             NULL				
  }
};

WidgetClass xfwfEzMEObjectClass = (WidgetClass) &xfwfEzMEClassRec;

/************************************************************
 *
 * Semi-Public Functions.
 *
 ************************************************************/


static void
ClassInitialize()
{
  haveSubMenuTrans = XtParseTranslationTable( haveSubMenuTransString );
  haveNoSubMenuTrans = XtParseTranslationTable( haveNoSubMenuTransString );
}

/*      Function Name: Initialize
 *      Description: Initializes the simple menu widget
 *      Arguments: request - the widget requested by the argument list.
 *                 new     - the new widget with both resource and non
 *                           resource values.
 *      Returns: none.
 * 
 * MENU ENTRIES CANNOT HAVE BORDERS.
 */

#ifndef XtObjectParent
#  define XtObjectParent(x) ( x -> object.parent )
#endif

/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
  XfwfEzMEWidget r = (XfwfEzMEWidget) request;
  XfwfEzMEWidget n = (XfwfEzMEWidget) new;

  n -> ezme.action = NULL;
  if ( r -> ezme.action ) {
    int len = strlen(r -> ezme.action);
    n -> ezme.action = XtMalloc(len+1);
    strcpy(n -> ezme.action, r -> ezme.action);
  }

}

static void
Destroy(w)
Widget w;
{
  XfwfEzMEWidget n = (XfwfEzMEWidget) w;

  if ( n -> ezme.action ) {
    XtFree( n -> ezme.action );
  }
}

static Boolean
SetValues(current, request, new)
     Widget current, request, new;
{
  XfwfEzMEWidget c = (XfwfEzMEWidget) current;
  XfwfEzMEWidget r = (XfwfEzMEWidget) request;
  XfwfEzMEWidget n = (XfwfEzMEWidget) new;

  if (( !c -> ezme.action && r -> ezme.action)
      ||
      (c ->ezme.action
       && r -> ezme.action
       && !strcmp(c->ezme.action, r -> ezme.action)))
      {
	int len = strlen(r -> ezme.action);
	XtFree(c->ezme.action);
	n -> ezme.action = XtMalloc(len+1);
	strcpy(n -> ezme.action, r -> ezme.action);
      }
}

/*	Function Name: Notify
 *	Description: calls the callback proceedures for this entry.
 *	Arguments: w - the menu entry.
 *	Returns: none.
 */

#define SKIPWHITE(cp) while (cp && *cp && isspace(*cp)) cp++

static void
Notify(w) 
Widget w;
{
  XfwfEzMEWidget widget = (XfwfEzMEWidget) w;
  Widget parent = XtObjectParent( widget );
  
  XtPopdown( parent );

  XtCallCallbacks(w, XtNcallback, NULL);
  
  if ( widget -> ezme.action ) {
    char *action = widget -> ezme.action;
    int actionLen = strlen( action );
    
    Bool doFree;
    char stringCopy[1024];
    char *allocatedStr;
    char *str;
    
    char *actionProcName;
    int num_params;
    char *params[100];	/* lets hope they don't have more than that.. */
    
    if ( actionLen > 1023 ) {
      allocatedStr = str = XtMalloc( actionLen + 2 );
      doFree = True;
    }
    else {
      str = stringCopy;
      doFree = False;
    }
    
    SKIPWHITE(str);
    strcpy(str, action);
    
    for (;;) {
      SKIPWHITE(str);
      if ( str == 0 || *str == 0 ) break;

      /* skip to action procedure name */
      SKIPWHITE(str);
      actionProcName = str;
      
      /* find end of action procedure name- terminate by space or paren */
      while (*str && !( isspace(*str) || *str == '(' )) str++;
      /* terminate action procedure name string */
      *(str++) = 0;
      
      /* skip past any openning paren, e.g. in case of 'foo (bar)' */

      SKIPWHITE(str);
      if ( *str == '(') str++;
      
      /* now, scan to build a list of params */
      num_params = 0;
      while ( *str && *str != ')' && num_params < 100) {
	SKIPWHITE(str);
	params[num_params] = str;
	while (*str && !( isspace(*str) || *str == ',' || *str==')' )) {
	  if (*str == '\\') {
	    str++;
	    if ( *str == 0 ) str--;
	  }
	  str++;
	}

	SKIPWHITE(str);

	/* terminate this parameter */
	
	num_params++;
	if ( *str == 0 ) {
	  break;
	} 
	else if (*str == ')') {
	  *(str++) = 0;
	  break;
	}
	else {
	  *(str++) = 0;
	}
	
	SKIPWHITE(str);
      }

      SKIPWHITE(str);
      if ( *str == ')') str++;
      
      XtCallActionProc(XtObjectParent(widget),
		       actionProcName, NULL, params, num_params);
    }
    
    if ( doFree ) {
      XtFree( allocatedStr );
    }
  }
}

static void
Highlight(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  Arg argList[20];
  int args = 0;
  int root_x, root_y, child_x, child_y, buttons;
  Dimension width;
  Dimension height;
  
  XfwfEzMEWidget widget = (XfwfEzMEWidget) w;

  (*superclass -> sme_class.highlight) (w, event, params, num_params);
  
  if ( widget -> ezme.toPopUp ) {
    Widget parent = XtObjectParent( widget );
    Widget toPopUp = widget -> ezme.toPopUp;
    
    if ( toPopUp ) { 
      
      Window root, child;
      Arg argList[20];
      int args = 0;
      int root_x, root_y, child_x, child_y;
      unsigned int buttons;
      Dimension width;
      Dimension height;
      
      XQueryPointer(XtDisplay(parent),
		    XtWindow(parent), &root, &child,
		    &root_x, &root_y, &child_x, &child_y, &buttons);

      root_x -= 20;
      root_y -= 20;	/* put the mouse solidly in the window */

      args = 0;
      XtSetArg(argList[args], XtNtranslations, haveSubMenuTrans); args++;
      XtSetValues(parent,argList,args);
      
      args = 0;
      XtSetArg(argList[args], XtNx, root_x); args++;
      XtSetArg(argList[args], XtNy, root_y); args++;
      XtSetArg(argList[args], XtNtranslations, haveNoSubMenuTrans); args++;
      XtSetValues(toPopUp,argList,args);
      XtPopup(toPopUp, XtGrabExclusive);
    }
  }
}

static void
UnHighlight(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  Arg argList[20];
  int args = 0;
  int root_x, root_y, child_x, child_y, buttons;
  Dimension width;
  Dimension height;
  
  XfwfEzMEWidget widget = (XfwfEzMEWidget) w;

  (*superclass -> sme_class.unhighlight) (w, event, params, num_params);
  
  if ( widget -> ezme.toPopUp ) {
    Widget parent = XtObjectParent( widget );

    args = 0;
    XtSetArg(argList[args], XtNtranslations, haveNoSubMenuTrans); args++;
    XtSetValues(parent,argList,args);

  }
}
