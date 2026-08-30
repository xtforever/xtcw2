/*

    Copyright (C) 1994 Clark McGrew

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */

#ifdef DEBUG_UGRAF
#define DEBUG(msg)  printf msg
#else
#define DEBUG(msg)  
#endif

#define USE_PIXMAP

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <stdio.h>
#include <math.h>

#include <Xfwf/UGraf.h>
#include <Xfwf/UGrafP.h>

static void FillPixmap();

/*******************
  The Widget Actions and translations.
  ******************/
#include "UGrafTranslations.h"

XtActionsRec actions[] = {
  /* {name, procedure} */
  {"ug-update",         Update},
  {"location",          PointerLocation},
  {"wider",             Wider},
  {"narrower",          Narrower},
  {"move",              Move},
  {"reverse",           Reverse},
  {"increase-step",     IncreaseStep},
  {"decrease-step",     DecreaseStep},
  {"move-up",           Up},
  {"move-down",         Down},
  {"move-left",         Left},
  {"move-right",        Right},
  {"dir-left",          DirLeft},
  {"dir-right",         DirRight},
  {"dir-up",            DirUp},
  {"dir-down",          DirDown},
  {"clockwise",         Clockwise},
  {"counter-clockwise", CounterClockwise},
};

char UGrafTranslations[] =
"<Key>KP_Enter:     ug-update()\n\
<Key>KP_Divide:     narrower()\n\
<Key>KP_Multiply:   wider()\n\
<Key>KP_5:          move()\n\
<Key>KP_Add:        reverse()\n\
<Key>KP_0:          increase-step()\n\
<Key>KP_Decimal:    decrease-step()\n\
<Key>KP_9:          move-up()\n\
<Key>KP_7:          move-down()\n\
<Key>KP_1:          move-left()\n\
<Key>KP_3:          move-right()\n\
<Key>KP_4:          dir-left()\n\
<Key>KP_6:          dir-right()\n\
<Key>KP_8:          dir-up()\n\
<Key>KP_2:          dir-down()\n\
<Key>Left:          clockwise()\n\
<Key>Right:         counter-clockwise()\n\
<BtnDown>:          location()\n\
";

/********************
  The Widget Resources.
********************/
#define UGrafXOffset(field) XtOffset(UGrafWidget, UGraf.x.field)
#define UGrafTransOffset(field) XtOffset(UGrafWidget, UGraf.t2.field)
#define UGrafTrans3Offset(field) XtOffset(UGrafWidget, UGraf.t3.field)
static XtResource resources[] = {
    {XtNupdateCallback, XtCUpdateCallback, XtRCallback, sizeof(caddr_t),
	 UGrafXOffset(callbacks), XtRCallback, NULL},
    {XtNlocationCallback, XtCLocationCallback, XtRCallback, sizeof(caddr_t),
	 UGrafXOffset(callbacks), XtRCallback, NULL},
    {XtNvisual, XtCVisual, XtRVisual, sizeof(Visual*),
	 UGrafXOffset(visual), XtRImmediate, CopyFromParent},
    {XtNlowHue, XtCLowHue, XtRInt, sizeof(int),
	 UGrafXOffset(color.low_hue), XtRString, "0"},
    {XtNhighHue, XtCHighHue, XtRInt, sizeof(int),
	 UGrafXOffset(color.high_hue), XtRString, "360"},
    {XtNlowShade, XtCLowShade, XtRInt, sizeof(int),
	 UGrafXOffset(color.low_shade), XtRString, "30"},
    {XtNhighShade, XtCHighShade, XtRInt, sizeof(int),
	 UGrafXOffset(color.high_shade), XtRString, "100"},
    {XtNlowValue, XtCLowValue, XtRInt, sizeof(int),
	 UGrafXOffset(color.low_value), XtRString, "25"},
    {XtNhighValue, XtCHighValue, XtRInt, sizeof(int),
	 UGrafXOffset(color.high_value), XtRString, "100"},
    {XtNgamma, XtCGamma, XtRFloat, sizeof(float),
	 UGrafXOffset(color.gamma_int), XtRString, "1.3"},
    {XtNredGamma, XtCRedGamma, XtRFloat, sizeof(float),
	 UGrafXOffset(color.gamma_red), XtRString, "1.0"},
    {XtNgreenGamma, XtCGreenGamma, XtRFloat, sizeof(float),
	 UGrafXOffset(color.gamma_green), XtRString, "1.0"},
    {XtNblueGamma, XtCBlueGamma, XtRFloat, sizeof(float),
	 UGrafXOffset(color.gamma_blue), XtRString, "1.0"},
    {XtNtolerance, XtCTolerance, XtRFloat, sizeof(float),
	 UGrafTrans3Offset(tolerance), XtRString, "0.3"},
    {XtNminDepth, XtCMinDepth, XtRInt, sizeof(int),
	 UGrafTrans3Offset(min_depth), XtRString, "1"},
    {XtNmaxDepth, XtCMaxDepth, XtRInt, sizeof(int),
	 UGrafTrans3Offset(max_depth), XtRString, "3"},
    {XtNstepSize, XtCStepSize, XtRFloat, sizeof(float),
	 UGrafTrans3Offset(step_size), XtRString, "1.0"},
    {XtNstepAngle, XtCStepAngle, XtRFloat, sizeof(float),
	 UGrafTrans3Offset(step_angle), XtRString, "0.1"},
    {XtNstepDir, XtCStepDir, XtRFloat, sizeof(float),
	 UGrafTrans3Offset(step_dir), XtRString, "1.0"}
};

/********************************
  The Widget Methods 
  *******************************/

static void Initialize();
static void Realize();
static void Redisplay();
static void Resize();
static Boolean SetValues();
static void Destroy();

/********************************
  The Widget Class Record
  *******************************/

XfwfUGrafClassRec XfwfugrafClassRec = {
    {
    /* core_class fields	 */
    /* superclass	  	 */ (WidgetClass) &widgetClassRec,
    /* class_name	  	 */ "UGraf",
    /* widget_size	  	 */ sizeof(UGrafRec),
    /* class_initialize   	 */ NULL,
    /* class_part_initialize	 */ NULL,
    /* class_inited       	 */ False,
    /* initialize	  	 */ Initialize,
    /* initialize_hook		 */ NULL,
    /* realize		  	 */ Realize,
    /* actions		  	 */ actions,
    /* num_actions	  	 */ XtNumber(actions),
    /* resources	  	 */ resources,
    /* num_resources	  	 */ XtNumber(resources),
    /* xrm_class	  	 */ NULLQUARK,
    /* compress_motion	  	 */ True,
    /* compress_exposure  	 */ XtExposeCompressMultiple,
    /* compress_enterleave	 */ True,
    /* visible_interest	  	 */ True,
    /* destroy		  	 */ Destroy,
    /* resize		  	 */ Resize,
    /* expose		  	 */ Redisplay,
    /* set_values	  	 */ SetValues,
    /* set_values_hook		 */ NULL,
    /* set_values_almost	 */ XtInheritSetValuesAlmost,
    /* get_values_hook		 */ NULL,
    /* accept_focus	 	 */ NULL,
    /* version			 */ XtVersion,
    /* callback_private   	 */ NULL,
    /* tm_table		   	 */ UGrafTranslations,
    /* query_geometry		 */ XtInheritQueryGeometry,
    /* display_accelerator       */ XtInheritDisplayAccelerator,
    /* extension                 */ NULL
    },
    {
      0 /* some stupid compilers barf on empty structures */
    },
};

WidgetClass xfwfUGrafWidgetClass = (WidgetClass) & XfwfugrafClassRec;

/****************************************************
  Initialize --

  Initialize an instance of the UGraf widget.  This takes the width and
  height from the core and builds a first set of normalization
  transforms and window transforms.
  ***************************************************/
static void Initialize(req_widget, new_widget, args, num_args)
    Widget req_widget, new_widget;
    ArgList args;
    Cardinal *num_args;
{
    int i;
    UGrafWidget pw = (UGrafWidget) new_widget;
    float xspace = 20.0, yspace = 20.0;
    Rectangle space = {1.0, 1.0, 19.0, 19.0};
    Rectangle window = {0.0, 0.0, 1.0, 1.0};

    DEBUG(("ugInitialize\n"));
    /* Set the current transformation */
    UGrafTrans(pw)->Current_Transform = 1;

    /* Set the normalization transform */
    XfwfUGNormal(pw,xspace,yspace);

    /* Set all of the window transforms */
    for(i=0; i<MaxWindows; ++i) {
	XfwfUGWindow(pw,i,&space,&window);
    }
    XfwfUGCurrent(pw,0);

    /* Set the current 3d Transform */
    UGrafTrans3(pw)->Current_Transform = 0;
    for (i=0; i<MaxWindows; ++i) {
	XfwfUG3Initialize(UGrafWindow3(pw,i));
    }

    /* Set the current font */
    UGrafInstance(pw)->text.font = 0;
    UGrafInstance(pw)->text.family = 0;

}

/****************************************************
  Realize --

  Draw the widget on the screen for the first time.  This allocates
  the window and the pixmap, as well as sets up the graphics context.
  ***************************************************/
static void Realize(widget, value_mask, attributes)
    Widget		 widget;
    XtValueMask		 *value_mask;
    XSetWindowAttributes *attributes;
{
  UGrafWidget	pw = (UGrafWidget)widget;

  DEBUG(("ugRealize\n"));
  /* First make a window on the display for this widget. */
  XtCreateWindow(widget, (unsigned int) InputOutput,
		 (Visual *) UGrafX(pw)->visual, *value_mask, attributes);

  /* I always want a backing pixmap for double buffering and handling */
  /* exposure events.  */ 
#ifdef USE_PIXMAP  
  UGrafX(pw)->pmap = XCreatePixmap(XtDisplay(pw),XtWindow(pw),
				UGrafCore(pw)->width, UGrafCore(pw)->height,
				DefaultDepthOfScreen(XtScreen(pw)));
#else
  UGrafX(pw)->pmap = XtWindow(pw);
#endif

  /* Get the graphics context for this widget. */
  UGrafX(pw)->gc = XtGetGC((Widget) pw,0,NULL); /* Use the default for now. */

  /* Initialize the pointer location routines */
  UGrafX(pw)->pointer_valid = False;

  /* Define the colors for this widget */
  DefinePalette(pw);
  UGrafX(pw)->background = attributes->background_pixel;
  XfwfUGGray(pw,0.5);

  /* Clear out the window */
  XfwfUGClear(pw);

#ifndef NO_ADVERTISING
  /* Fill the window with an advertisement.  This is actually */
  /* debugging, since I want to make sure the widget is filling it's */
  /* space on the screen. */
  FillPixmap(pw);
#endif

} /* CoreRealize */

/********************************************
  Destroy --

  Free up all of the resources allocated by this instance of the plot
  widget.
  *******************************************/
static void Destroy(widget)
    Widget  widget;
{
    UGrafWidget pw = (UGrafWidget)widget;
    DEBUG(("ugDestroy\n"));
    if (UGrafX(pw)->pmap != XtWindow(pw)) {
	XFreePixmap(XtDisplay(pw),UGrafX(pw)->pmap);
    }
    XtReleaseGC((Widget) pw,UGrafX(pw)->gc);
}

/********************************************
  Expose --

  Redisplay the widget after an exposure event.
  *******************************************/
static void Redisplay(w, event, region)
  Widget w;
  XExposeEvent *event;
  Region region;
{
    UGrafWidget	pw = (UGrafWidget)w;

    DEBUG(("ugRedisplay...\n"));
	
    /* Before we draw make real sure the widget is realized. */
    if (!XtIsRealized((Widget) pw))
	return;

    DEBUG(("ugRedisplayed\n"));
    /* Copy the area from the Pixmap to the Window */
    if (UGrafX(pw)->pmap != XtWindow(pw)) {	
    DEBUG(("Copy pixmap\n"));
    XCopyArea(XtDisplay(pw), UGrafX(pw)->pmap, XtWindow(pw), UGrafX(pw)->gc,
		  0,0,UGrafCore(pw)->width,UGrafCore(pw)->height,0,0);
    }
}
/* User Routine:
   XfwfUGUpdate --
   Draw all the pending operations. 
   */
void XfwfUGUpdate(UGrafWidget pw) {
    Redisplay(pw,0,0);
}

/*************************************************
  Set_Values --

  This is called when the application calls XtSetValues to set the
  resources of the widget.  It recalculates the private instance
  variables based on the new public instance variable values.  It is
  similar to Initialize, but is called at different and multiple
  times.
  ************************************************/
static Boolean SetValues(current, request, new, args, nargs)
UGrafWidget current, request, new;
ArgList args;
Cardinal *nargs;
{
  int	i;
  DEBUG(("ugSetValues\n"));
  for(i=0; i<*nargs; i++) {
    if (strcmp("vvv",args[i].name)==0 ||
	strcmp("vvvv",args[i].name)==0)
      return True;
  }
  return False;
}

/*************************************************
  Resize --
  
  Resize the widget.  This calculates the new normalization
  transforms.
  ************************************************/
static void Resize(pw)
UGrafWidget pw;
{
    DEBUG(("ugResize\n"));
    XfwfUGMakeNormalizationTransform(pw);
    if (UGrafX(pw)->pmap != XtWindow(pw)) {
	XFreePixmap(XtDisplay(pw),UGrafX(pw)->pmap);
	UGrafX(pw)->pmap = 
	    XCreatePixmap(XtDisplay(pw),XtWindow(pw),
			  UGrafCore(pw)->width, UGrafCore(pw)->height,
			  DefaultDepthOfScreen(XtScreen(pw)));
	XfwfUGClear(pw);
	XtCallCallbacks((Widget) pw,XtNupdateCallback,NULL);
	XfwfUGUpdate(pw);
    }
}

/************************************************
  Fill the window with a default picture
  ************************************************/
static void FillPixmap(w) 
Widget w;
{
    UGrafWidget ug = (UGrafWidget) w;
    float cx=10.0, cy=10.0;
    float r = 5.0;

    {				/* Make a pretty color wheel */
	int i;
	double h, hs = 0.02, s, ss=0.062;
	float H,S;
	Point p[3];

	for (h=0.0; h<1.0; h+=hs) {
	    for (s=0.0; s<1.0; s+=ss) {
		H = h;
		S = 1.0-s;
		S = sqrt(S);
		p[0].x = cx; p[0].y = cy;
		p[1].x = cx+r*S*sin(2.0*3.14159*H);
		p[1].y = cy+r*S*cos(2.0*3.14159*H);
		p[2].x = cx+r*S*sin(2.0*3.14159*(H+hs));
		p[2].y = cy+r*S*cos(2.0*3.14159*(H+hs));
		XfwfUGShade(ug,h,s);
		XfwfUGFill(ug,p,3);
	    }
	}
    }

    /* Select a pretty and print the message */
    XfwfUGFontDefault(ug);
    XfwfUGFontSize(ug,2.0);
    XfwfUGFontType(ug,SCRIPT);
    XfwfUGFontFamily(ug,COMPLEX);
    XfwfUGFontJustification(ug,CENTER_JUSTIFICATION);
    XfwfUGText(ug,cx,17.5,"User");
    XfwfUGText(ug,cx,2.5,"Graphics");
    XfwfUGFontDefault(ug);
    
    XfwfUGUpdate(ug);
}


