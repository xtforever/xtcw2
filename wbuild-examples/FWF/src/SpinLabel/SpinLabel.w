# SpinLabel.w - scroll trough a list using 2 arrows
# afo@pub.th-zwickau.de
# 9502

@class XfwfSpinLabel(XfwfLabel)		@file = SpinLabel

@ The |XfwfSpinLabel| is derived from the |XfwfLabel| widget.
It provides nearly the same features as its superclass but
additionally it is possible to manipulate the labels using
two arrow buttons. With these the user is able to increase/decrease a 
numerical value or scroll trough a list of strings. every time one 
of the arrows is pressed the |activate| callback is invoked.
The |SpinLabel| widget provides a keyboard interface too.
Using a callback the programmer is free to decide trough what 
he let the user scroll.
It is possible to choose various styles using different
combinations of orientation and side.


@PUBLIC

@ The foreground color for the arrows. if not specified the background
color is used.

	@var Pixel arrowForeground = <CallProc> copy_background

@ The |activate| action is invoked every time the user clicks
on one of the arrows using the mouse or uses the keyboard interface.
See Translations for the default keybindings.
The concrete action (XfwfFirst,XfwfPrev,XfwfNext,XfwfLast) is passed 
using the |call_data| argument.

	@var <Callback> XtCallbackList callback = NULL


@ In contrast to its superclass |XfwfLabel| the SpinLabel widget
provides a keyboard interface. this means traversalOn is set to True.

	@var traversalOn = True

@ The default |frameWidth| of the SpinLabel widget is 0.

	@var frameWidth	= 0

@ The value of the |frameType| resource is set to XfwfChiseled.

	@var frameType = XfwfChiseled

@ The |labelframeWidth| controls the frame width of the label.
Defaults to 0.

	@var Dimension labelframeWidth = 0

@ The |labelframeType| controls the frame type of the label.
Default values is XfwfSunken.

	@var FrameType labelframeType = XfwfSunken

@ The |horizontal| resource specifies if the arrows should be
horizontal (|True|) or vertical (|False|).
Default value is |True|.

	@var Boolean horizontal = True

@ The |side| is used to specify where the label should live.
Possible values are |"center"| (|XfwfCenter|), |"left"| (|XfwfLeft|)
and |"right"| (|XfwfRight|).
The default value is |XfwfCenter|.

	@var Alignment alignment = XfwfCenter



@EXPORTS

@ SpinType is used for returning the action.

	@type SpinType = enum {XfwfFirst,XfwfPrev,XfwfNext,XfwfLast}


@PRIVATE

@ The 3 children are stored in the following private variables

	@var Widget _child[3]


@METHODS

@ The |initialize| method creates the Widgets:
Two arrows and one label Widget.

@proc initialize
{
	Bool vert;

	vert=($alignment == XfwfCenter && $horizontal ? True : False);
			
	$_child[0]=XtVaCreateWidget("_arrow1",xfwfArrowWidgetClass,$,
			XtNframeWidth,0,XtNforeground,$arrowForeground,
			XtNtraversalOn, False, XtNhighlightThickness, 0,  
			XtNdirection, vert ? XfwfLeft : XfwfBottom,NULL);
	XtAddCallback($_child[0],XtNcallback,arrow_prev,$);
		
	$_child[1]=XtVaCreateWidget("_label",xfwfLabelWidgetClass,$,
			XtNframeWidth,$labelframeWidth,XtNframeType,$labelframeType,
			XtNlabel,$label,NULL);
	
	$_child[2]=XtVaCreateWidget("_arrow2",xfwfArrowWidgetClass,$,
			XtNframeWidth,0,XtNforeground,$arrowForeground,
			XtNtraversalOn, False, XtNhighlightThickness, 0,
			XtNdirection, vert ? XfwfRight : XfwfTop,NULL);
	XtAddCallback($_child[2],XtNcallback,arrow_next,$);
	set_geometry($);
	XtManageChildren($_child,3);
}

@proc resize
{
	set_geometry($);
}


@proc set_values
{
	Bool redisplay=False;

#ifdef DEBUG	
	fprintf(stderr,"set_values(%s)\n",XtName($));
#endif
	
	$set_label($_child[1],$label);

	if($old$alignment != $alignment)
		redisplay=True;

	if($old$horizontal != $horizontal)
		redisplay=True;

	if(redisplay)
		set_geometry($);
	
	if($old$arrowForeground != $arrowForeground)
	{
		XtVaSetValues($_child[0],XtNforeground,$arrowForeground,NULL);
		XtVaSetValues($_child[2],XtNforeground,$arrowForeground,NULL);
	}

	 if($old$labelframeType != $labelframeType)
		XtVaSetValues($_child[1],XtNframeType,$labelframeType,NULL);

	if($old$labelframeWidth != $labelframeWidth)
		XtVaSetValues($_child[1],XtNframeWidth,$labelframeWidth,NULL);

	return redisplay;
}


@TRANSLATIONS

@trans	<Btn1Down>:			traverseCurrent()
@trans	~Shift<Key>Tab:		traverseNext()
@trans	Shift<Key>Tab:		traversePrev()

@trans	<Key>Home:			first()
@trans	Ctrl<Key>a:			first()

@trans	<Key>Left:			prev()
@trans	<Key>Down:			prev() 
@trans	<Key>KP_Subtract:	prev()
@trans	<Key>minus:			prev()
@trans	Ctrl<Key>p:			prev()
@trans	Ctrl<Key>b:			prev()

@trans	<Key>Up:			next()
@trans	<Key>Right:			next()
@trans	Ctrl<Key>n:			next()
@trans	Ctrl<Key>f:			next()
@trans	<Key>plus:			next()
@trans	<Key>KP_Add:		next()

@trans	<Key>End:			last()
@trans	Ctrl<Key>e:			last()


@UTILITIES

@proc arrow_prev(Widget w,XtPointer client_data,XtPointer call_data)
{
	Widget $ = (Widget) client_data;
	
	XtCallCallbackList(w,$callback,(XtPointer) XfwfPrev);
}

@proc arrow_next(Widget w,XtPointer client_data,XtPointer call_data)
{
	Widget $ = (Widget) client_data;
	
	XtCallCallbackList(w,$callback,(XtPointer) XfwfNext);
}

@ The |copy_background| routine is resource default procedure. It is
called to initialize the default value of the |foreground| resource.

@proc copy_background($, int offset, XrmValue* value)
{
    value->addr = (XtPointer) &$background_pixel;
}


@ The |set_geometry| calculates the size of the children

@proc set_geometry($)
{
	Position x,y,xa1,xa2,xl,yl,ya1,ya2;
	Dimension w,h,wa,ha,wl,hl;
	
	$compute_inside($,&x,&y,&w,&h);
	
	if($horizontal)
	{
		hl=h;
		ha=wa=h-4;	/* 2*$frameWidth; */
		wl=w-(2*ha)-2;	/* $frameWidth; */
		yl=y;
		ya1=ya2=y+2;	/* $frameWidth; */
		
		if($alignment == XfwfLeft)
		{
			 xl=x;	xa1=wl+4; /* 2*$frameWidth;*/	xa2=xa1+wa;
		}
		else if($alignment == XfwfRight)
		{
			xa1=x;	xa2=wa+2; /* $frameWidth; */	xl=2*xa2;
		}
		else
		{
			if($alignment != XfwfCenter)
			{
				XtWarning("SpinLabel.alignment: illegal value. using center");
				$alignment=XfwfCenter;
			}
			xa1=x+2; /*$frameWidth;*/	xl=wa+4; /* 2*($frameWidth+$labelframeWidth); */
			xa2=xl+wl;
		}
	}
	else		/* vertical */	
	{
		if($alignment == XfwfLeft)
		{
			hl=h;	ha=h/2-1;
			wa=ha+4; /* 2*$frameWidth;*/	wl=w-wa-2;
			xl=x;	xa1=xa2=wl+2; /* $frameWidth; */
			yl=ya2=y;
			ya1=ha+6;	/* 2+2*$frameWidth; */
		}
		else if($alignment == XfwfRight)
		{
			hl=h;	ha=h/2-1;
			wa=ha+4; /* 2*$frameWidth;*/	wl=w-wa-2;
			xl=wa+4; /* 2*$frameWidth;*/	xa1=xa2=x;
			yl=ya2=y;
			ya1=ha+4; /* 2+2*$frameWidth; */
		}
		else
		{
			if($alignment != XfwfCenter)
			{
				XtWarning("SpinLabel.alignment: illegal value. using center");
				$alignment=XfwfCenter;
			}
			hl=ha=h/3;	wl=wa=w;
			xl=xa1=xa2=x;
			ya2=y;	yl=ya2+ha;	ya1=yl+hl;
		}
	}

#ifdef DEBUG	
	fprintf(stderr,"spin.resize(%s %dx%d @%d,%d)\n",XtName($),w,h,x,y);
	fprintf(stderr,"\t[arrow1 %dx%d @%d,%d]\n",wa,ha,xa1,ya1);
	fprintf(stderr,"\t[label  %dx%d @%d,%d]\n",wl,hl,xl,yl);
	fprintf(stderr,"\t[arrow2 %dx%d @%d,%d]\n",wa,ha,xa2,ya2);
#endif

	XtConfigureWidget($_child[0],xa1,ya1,wa,ha,0);
	XtConfigureWidget($_child[1],xl,yl,wl,hl,0);
	XtConfigureWidget($_child[2],xa2,ya2,wa,ha,0);
}

@ACTIONS

@ The |first| action just calls the |callback| function passing
|XfwfFirst| in the |call_data| argument.

@proc first
{
	XtCallCallbackList($,$callback,(XtPointer) XfwfFirst);
}

@ The |prev| action just calls the |callback| function passing
|XfwfPrev| in the |call_data| argument.

@proc prev
{
	XtCallCallbackList($,$callback,(XtPointer) XfwfPrev);
}


@ The |next| action just calls the |callback| callback passing
|XfwfNext| in the |call_data| argument.

@proc next
{
	XtCallCallbackList($,$callback,(XtPointer) XfwfNext);
}


@ The |last| action just calls the |callback| function passing
|XfwfLast| in the |call_data| argument.


@proc last
{
	XtCallCallbackList($,$callback,(XtPointer) XfwfLast);
}


@IMPORTS

@incl <stdio.h>
@incl <Xfwf/Arrow.h>		
@incl <Xfwf/Label.h>	
