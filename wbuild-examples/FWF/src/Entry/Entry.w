# XfwfEntry - single line text input widget
# Andre Fornacon (afo@pub.th-zwickau.de)
# 9501 v1.0 - initial release
# 9502 v1.1 - now child of Board (i need too less from Label)
#
# TODO: - selection 
#       - scrolling (right,left) for text longer than the size of the field

@class XfwfEntry (XfwfBoard)	@file=Entry	

@ The |XfwfEntry| widget is used to read a single line of text.
You can restrict the total length of the input string using the 
variable |max| . The default value is 0 (characters).
Using the |valid| resource it is possible to control the users input.
The user can only enter the specified characters.
Setting this ressource to NULL (default) let the users imput all
characters between 0x20 (SPACE) and 0xff.
The |echo| resource enables/disables displaying of the entered text.
You can use this for things like password boxes.


@PUBLIC

@ The variable |text| holds the string modified by the user.

	@var String text = NULL
	
	
@ Using the |valid| ressource it is possible to limit the characters the user 
is allowed to enter. With a value of NULL the allowed characters ranges
from 0x20 (SPACE) to 0xff.

	@var String valid = NULL


@ The |max| ressource holds the number of maximal allowed characters.
It defaults to 0.

	@var int max	= 0


@ The |echo| flag enables/disables displaying of the entered text.

	@var Boolean echo = True


@ The |activate| callback is called if the activate action is executed,
bound by default to the <Return> key. entered text is passed in the 
|call_data| argument.

	@var <Callback> XtCallbackList activate = NULL

@ The text is drawn using the font given in the |font| resource.

	@var <FontStruct> XFontStruct *font = <String> XtDefaultFont

@ The |foreground| resource specifies the color used for drawing the text.

	@var Pixel foreground = <String> XtDefaultForeground


@ The |cursor| ressource is by default a vertical bar.

	@var cursor = <String> "xterm"

@ this widget receives keyboard events, |traversalOn| is set to |True|.

	@var traversalOn = True

@ The default frame width is set to 2

	@var frameWidth = 2

@ the |frameType| resource defaults to |"sunken"|.

	@var frameType = XfwfSunken


@PRIVATE

@ The current length of the text

	@var int text_len
	
@ current cursor position	

	@var int text_pos

@ The |text_start| ressource indicates the first displayed character , 
if the whole exceeds the width . needed for horizontal scrolling.

	@var int text_start

@ The maximum number of chars displayed at once is stored in |text_width|

	@var int text_width

@ Between keypresses, the |XComposeStatus| is retained.

	@var XComposeStatus compose_stat

@ For normal text, ngc is used

	@var GC ngc

@ For drawing text in reverse color (like the cursor) rgc is used

	@var GC rgc

@ should i use a third gc for drawing highlighted text (selected ?)
a grayed one or a stippled ?


@EXPORTS

@proc void XfwfEntryClear($)
{
	Position x,y;
	Dimension h,w;
	
	if($text != NULL)
	{
		get_pos($,&x,&y);
		w=XTextWidth($font,$text,$text_len);
		h=$font->ascent+$font->descent;
#ifdef DEBUG		
		fprintf(stderr,"clear(%s,%d) %dx%d @%d,%d,[%dx%d]\n",$text,$text_len,\
				w,h,x,y,$width,$height);
#endif		
		$text_pos=$text_len=0;
		$text[0]='\0';
		XClearArea(XtDisplay($),XtWindow($),x,y-$font->ascent,w,h,False);
		if($traversal_focus == True)
			set_cursor($,True);
	}
}

@METHODS

@ The |initialize| method sets the private variables to default values

@proc initialize
{
	String tmp;
	int i;

#ifdef DEBUG	
	fprintf(stderr,"init(%s) [%s,%d,%s]\n",XtName($),$valid,$max,\
			$echo ? "true" : "false");
#endif
	
	$text_pos=$text_len=0;

	$ngc=$rgc=NULL;
	make_gc($);
	
	if($valid != NULL)
		$valid=XtNewString($valid);
	tmp=XtMalloc((1+$max)*sizeof(char));
	tmp[0]='\0';

	if($text != NULL)
	{
		for(i=0;$text[i] && i < $max;i++)
			tmp[i]=$text[i];
		tmp[i]='\0';
		$text_pos=$text_len=i;
	}
	$text=tmp;
#ifdef DEBUG	
	fprintf(stderr,"entry.init(%s)\n",$text);
#endif	
}


@ The |set_values| method copies all changed string ressources 

@proc set_values
{
	String tmp;
	int i;

#ifdef DEBUG	
	fprintf(stderr,"set_values(%s) '%s'\n",XtName($),$text);
#endif
	
	if($old$text != $text)
	{
		XtFree($old$text);
		tmp=XtMalloc((1+$max)*sizeof(char));
		for(i=0;$text[i] && i < $max;i++)
			tmp[i]=$text[i];
		tmp[i]='\0';
		$text_pos=$text_len=i;
		$text=tmp;
	}

	return True;	
}

@ The |expose| method is called if the widget was overlapped and became
	visible again
	
@proc expose
{
	if(! XtIsRealized($))
		return ;
	
	#expose($,event,region);
	set_text($,0);
	if($traversal_focus == True)
		set_cursor($,True);
	
#ifdef DEBUG	
	fprintf(stderr,"expose(%s)\t,%s,len %d, max %d, pos %d\n",XtName($),\
				$text,$text_len,$max,$text_pos);
#endif	
}


@TRANSLATIONS

@trans <FocusIn>:			focusIn() mouse_in()
@trans <FocusOut>:			mouse_out() focusOut()

@trans <Btn1Down>:			traverseCurrent() mclick()
@trans <Key>Up:				traverseUp()
@trans <Key>Down:			traverseDown()

@trans	Shift<Key>Tab:		traversePrev()
@trans	~Shift<Key>Tab:		traverseNext()

@trans	<Key>BackSpace:		backspace()
@trans	<Key>Right:			right()
@trans	<Key>Left:			left()

@trans	<Key>Home:			bol()
@trans	<Key>End:			eol()

@trans	Ctrl<Key>a:			bol()
@trans	Ctrl<Key>d:			del_char()
@trans	Ctrl<Key>e:			eol()
@trans	Ctrl<Key>b:			left()
@trans	Ctrl<Key>f:			right()
@trans	Ctrl<Key>h:			backspace()
@trans	Ctrl<Key>k:			kill()
@trans	Ctrl<Key>l:			redraw()
@trans	<Key>Return:		activate()
@trans	~Ctrl<Key>:			self_insert()


@ACTIONS

@ The |activate| action calls the |activate| callback with the current
|text| as |call_data|.

@proc activate
{
    XtCallCallbackList($, $activate, $text);
}

@proc self_insert
{
    KeySym keysym;
    int n,k,i;
    char buf[100];

    if (event->type != KeyPress && event->type != KeyRelease) 
	{
		XtWarning("action <self_insert> can only handle keyboard events");
		return;
    }

    n = XLookupString(&event->xkey, buf, sizeof(buf), &keysym, &$compose_stat);
	set_cursor($,False);
	for(k=i=0;i < n;i++)
		k+=insert_char($,buf[i]);
	set_text($,$text_pos-k);
	set_cursor($,True);

#ifdef DEBUG	
	fprintf(stderr,"self_insert()\t%d ->%s<\n",keysym,$text);
#endif	
}



@proc del_char
{
    int i;

	if($text_pos < $text_len)
	{
		set_cursor($,False);
		$text_len--;
		for(i=$text_pos;i < $text_len;)
			$text[i]=$text[++i];
		$text[$text_len]='\0';
		if($text_pos > $text_len)
			$text_pos=$text_len;
		set_text($,$text_pos);
		set_cursor($,True);
	}	
}


@proc backspace
{
    int i;

    if($text_len > 0 && $text_pos > 0)
	{
		set_cursor($,False);
		for(i=$text_pos-1;i < $text_len;)
			$text[i]=$text[++i];
		$text_len--;
		$text_pos--;
		set_text($,$text_pos);
		set_cursor($,True);
	}
}


@proc kill
{
	Position x,y;
	Dimension w,h;
	
	if($text_pos < $text_len)
	{
		get_pos($,&x,&y);
		x+=XTextWidth($font,$text,$text_pos);
		w=XTextWidth($font,$text+$text_pos,(1+$text_len-$text_pos));
		h=$font->ascent+$font->descent;
		
#ifdef DBEUG		
		fprintf(stderr,"kill(%s,%d,%d) %dx%d @%d,%d\n",$text,$text_pos,\
				$text_len,w,h,x,y);
#endif
		
		XClearArea(XtDisplay($),XtWindow($),x,y-$font->ascent,w,h,False);
		$text[$text_pos]='\0';
		$text_len=$text_pos;
		set_text($,$text_pos);
		set_cursor($,True);
	}
}


@proc redraw
{
	set_text($,0);
	set_cursor($,True);
}


@proc mclick
{
	Position x,y;
	XButtonEvent xbutton;
	
	if(event->type != ButtonPress)
	{
		fprintf(stderr,"mclick called with invalid event type\n");
		XBell(XtDisplay($),100);
		return ;
	}
	xbutton= event->xbutton;
	set_cursor($,False);
	get_pos($,&x,&y);

	for($text_pos=0;$text_pos<=$text_len && XTextWidth($font,$text,$text_pos)<(xbutton.x-x);$text_pos++);
	$text_pos--;
	if($text_pos > $text_len)
		$text_pos=$text_len;
	set_cursor($,True);

#ifdef DEBUG
	fprintf(stderr,"mclick @%d,%d -> %d\n",xbutton.x,xbutton.y,$text_pos);
#endif	
}


@proc mouse_in
{
	set_cursor($,True);	
}


@proc mouse_out
{
	set_cursor($,False);	
}

	
@proc bol
{
	if ($text_pos != 0)
	{
		set_cursor($,False);		
		$text_pos = 0;
		set_cursor($,True);		
	}
}

@proc eol
{
	if ($text_pos < $text_len)
	{
		set_cursor($,False);
		$text_pos = $text_len;
		set_cursor($,True);
	}
}

@proc left
{
	if($text_pos > 0)
	{
		set_cursor($,False);
		$text_pos--;
		set_cursor($,True);
	}
}

@proc right
{
	if($text_pos < $text_len)
	{
		set_cursor($,False);
		$text_pos++;
		set_cursor($,True);
	}
}


@UTILITIES

@ create the GC's used for drawing text

@proc make_gc($)
{
	XtGCMask mask;
	XGCValues values;

	if($ngc != NULL)				/* GC for normal text */
		XtReleaseGC($,$ngc);

	values.foreground=$foreground;
	values.background=$background_pixel;
	values.font=$font->fid;
	mask=GCFont|GCBackground|GCForeground;
	$ngc=XtGetGC($,mask,&values);

	if($rgc != NULL)				/* GC for reverse text */
		XtReleaseGC($,$rgc);

	values.foreground=$background_pixel;	
	values.background=$foreground;			
	values.font=$font->fid;
	mask=GCFont|GCBackground|GCForeground;
	$rgc=XtGetGC($,mask,&values);

}	

@ Get the Position of the first char

@proc get_pos($,Position *x, Position *y)
{
	Dimension baseline;
	XRectangle rect;
	
	baseline = $font->ascent + $font->descent;
	$compute_inside($, &rect.x, &rect.y, &rect.width, &rect.height);

	rect.x += 2;	rect.width -= 4;
	rect.y += 2;	rect.height -= 4;

	*y = rect.y + (rect.height - baseline)/2 + $font->ascent;
	*x = rect.x;
}


@proc set_cursor($,Bool flag)
{
	Position x,y;
	Dimension pos,w;
	GC gc;

	/* cursor position is always 0 if echo disabled */
	pos = ($echo == False ? 0 : $text_pos);
	
	gc = (flag == True ? $rgc : $ngc);
	get_pos($,&x,&y);
	w=XTextWidth($font, $text, pos);
					  
	if($text_pos == $text_len || $echo == False)
		XDrawImageString(XtDisplay($),XtWindow($),gc,x+w,y," ",1);
	else
		XDrawImageString(XtDisplay($),XtWindow($),gc,x+w,y,$text+$text_pos,1);
}

@proc set_text($,int pos)
{
	Position x,y;
	Dimension w, h;
	
	if($echo == True)
	{
		get_pos($,&x,&y);
		x+=XTextWidth($font,$text,$text_len);
		w=$font->max_bounds.width;
		h=$font->ascent+$font->descent;
		
		XClearArea(XtDisplay($),XtWindow($),x,y-$font->ascent,w,h,False);

		get_pos($,&x,&y);
		w=XTextWidth($font, $text, pos);
	
#ifdef DEBUG 	
		fprintf(stderr,"set_text(%s,%d) @%d,%d\n",$text+pos,$text_len-pos,x+w,y);
#endif
	
		XDrawImageString(XtDisplay($),XtWindow($),$ngc,x+w,y,$text+pos,
						 $text_len-pos);
	}
}



@proc int insert_char($,int c)
{
	int i,n=0;
	Boolean flag=False;

	if($text_len == $max)
	{
		XBell(XtDisplay($),100);
		return n;
	}
	
	if($valid == NULL)			/* all printable chars are allowed */
	{
		if(' ' <= c && c <= '~')
			flag=True;
	}
	else					/* check if c is a valid char */
	{
		for(i=0;$valid[i];i++)
			if(c == $valid[i])
			{
				flag=True;
				break ;
			}
	}

	if(flag == True)
	{
		$text_len++;
		for(i=$text_len;i > $text_pos;)
			$text[i]=$text[--i];
		$text[$text_pos++]=c;
		n++;
	}
	return n;
}


@IMPORTS

@incl <ctype.h>
@incl <stdio.h>
@incl <string.h>
@incl <X11/Xatom.h>
@incl <X11/Xmu/Atoms.h>
