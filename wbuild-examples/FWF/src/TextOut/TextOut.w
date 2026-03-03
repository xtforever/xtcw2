# XfwfTextOut - text output widget
# Paul Kendall (paul@kcbbs.gen.nz)

@class XfwfTextOut (XfwfCommon)	@file=TextOut

@ The |XfwfTextOut| widget is used to display text in multiple fonts
	and colors.


@PUBLIC

@ The offset from the text to the outside edge of the widgets window.

	@var Dimension internalOffset = 2

@ The text is drawn using the fonts given in the |font1-4| resources.

	@var <FontStruct> XFontStruct *font1 = <String> XtDefaultFont
	@var <FontStruct> XFontStruct *font2 = <String> XtDefaultFont
	@var <FontStruct> XFontStruct *font3 = <String> XtDefaultFont
	@var <FontStruct> XFontStruct *font4 = <String> XtDefaultFont

@ The |color1-8| resources specify the colors used for drawing the text.

	@var Pixel color1 = <String> XtDefaultForeground
	@var Pixel color2 = <String> XtDefaultForeground
	@var Pixel color3 = <String> XtDefaultForeground
	@var Pixel color4 = <String> XtDefaultForeground
	@var Pixel color5 = <String> XtDefaultForeground
	@var Pixel color6 = <String> XtDefaultForeground
	@var Pixel color7 = <String> XtDefaultForeground
	@var Pixel color8 = <String> XtDefaultForeground

@PRIVATE

@ The gc |ngc| gets modified each time a color changes or a font changes
	int the text.

	@var GC ngc

@ This data structure is used to keep the definition of the text that
	is to be displayed.

	@type TextChunk = struct _textChunk {
		String			text;
		int			font;
		int			color;
		int			nl;
		struct _textChunk	*next;
	}

@ The variable |text| holds the head of the chunk structures for the
	text to be displayed in the widget.

	@var TextChunk *text
	@var TextChunk *tail


@CLASSVARS
	@var compress_exposure =
		XtExposeCompressMultiple|XtExposeGraphicsExposeMerged


@UTILITIES

@ create the GC used for drawing text.

@proc make_gc($)
{
	XtGCMask mask, dynmask, dontcaremask;
	XGCValues values;

	values.foreground=$color1;
	values.background=$background_pixel;
	values.font=$font1->fid;
	dynmask = GCForeground | GCBackground | GCFont;
	dontcaremask = GCJoinStyle | GCFillRule | GCTile
		| GCTileStipXOrigin | GCTileStipYOrigin | GCDashOffset | GCDashList
		| GCStipple | GCGraphicsExposures | GCArcMode | GCLineWidth | GCClipMask;
	mask=0;
	$ngc=XtAllocateGC($,$depth,mask,&values,dynmask,dontcaremask);
}	

@ This function is used to calculate the maximum ascender and descender
	for a line of text chunks.  This could be the ascent from one font and
	the descent from a different font to give the maximum line height.

@proc compute_line_height($, TextChunk *chunk, int *ma, int *md)
{
	XFontStruct *f;

	*ma=0, *md=0;
	if(chunk==NULL) return;
	do {
		switch(chunk->font)
		{
			case 2:		f=$font2;	break;
			case 3:		f=$font3;	break;
			case 4:		f=$font4;	break;
			default:	f=$font1;	break;
		}

		*ma = *ma>f->ascent ? *ma : f->ascent;
		*md = *md>f->descent ? *md : f->descent;
		chunk = chunk->next;
	} while(chunk && !chunk->nl);
}

@ The |draw_string| utility function is used to draw a string at a set
	position, color and font in the widget.

@proc draw_string($, XFontStruct *f, int c, char *s, int l, int x, int y)
{
	XSetFont(XtDisplay($), $ngc, f->fid);
	switch(c)
	{
		case 2:	XSetForeground(XtDisplay($), $ngc, $color2);
			break;
		case 3:	XSetForeground(XtDisplay($), $ngc, $color3);
			break;
		case 4:	XSetForeground(XtDisplay($), $ngc, $color4);
			break;
		case 5:	XSetForeground(XtDisplay($), $ngc, $color5);
			break;
		case 6:	XSetForeground(XtDisplay($), $ngc, $color6);
			break;
		case 7:	XSetForeground(XtDisplay($), $ngc, $color7);
			break;
		case 8:	XSetForeground(XtDisplay($), $ngc, $color8);
			break;
		default:XSetForeground(XtDisplay($), $ngc, $color1);
			break;
	}
			
	XDrawString(XtDisplay($),XtWindow($),$ngc,
			x+$internalOffset,y,s,l);
}

@ Calculate the size of the widget based on the text that it has to display.

@proc compute_size($, Dimension *wd, Dimension *ht)
{
	TextChunk *chunk;
	int w=0;
	int maxw=0;
	int ma, md;
	XFontStruct *f;
	char *s, *e;
	
	compute_line_height($, $text, &ma, &md);
	*ht = ($internalOffset)*2 + ma + md;
		
	for(chunk=$text ; chunk ; chunk=chunk->next)
	{
		switch(chunk->font)
		{
			case 2:		f=$font2;	break;
			case 3:		f=$font3;	break;
			case 4:		f=$font4;	break;
			default:	f=$font1;	break;
		}

		s=chunk->text;
		e=strchr(s, '\t');
		while(e!=NULL)
		{
			if(e-s!=0) w+=XTextWidth(f,s,e-s);
			w+=(40-w%40)==0 ? 40 : (40-w%40);
			s=e+1;
			e=strchr(s, '\t');
		}
		w+=XTextWidth(f,s,strlen(s));

		if(chunk->nl)
		{
			maxw = maxw>w ? maxw : w;
			w=0;
			compute_line_height($, chunk->next, &ma, &md);
			*ht += ma + md;
		}
	}
	maxw = maxw>w ? maxw : w;
	*wd=maxw + ($internalOffset)*2;
}


@METHODS

@ The |initialize| method sets the private variables to default values.

@proc initialize
{
	$text=NULL;
	$tail=NULL;

	$ngc=NULL;
	make_gc($);
	
	$highlightThickness=0;
	$width=1;
	$height=1;
}


@proc destroy
{
	TextChunk *n, *c;
	
	XtReleaseGC($, $ngc);

	c=$text;
	while(c)
	{
		n=c->next;
		XtFree((char *)c->text);
		XtFree((char *)c);
		c=n;
	}
	$text=NULL;
	$tail=NULL;
}


@ The |set_values| method copies all changed string resources.

@proc set_values
{
	int do_size = False;

	$highlightThickness=0;

	if($old$internalOffset != $internalOffset)
		do_size = True;

	if($old$font1 != $font1)
		do_size = True;
		
	if($old$font2 != $font2)
		do_size = True;
		
	if($old$font3 != $font3)
		do_size = True;
		
	if($old$font4 != $font4)
		do_size = True;
		
	if(do_size)
		compute_size($, &($width), &($height));

	return do_size;
}

@ The |expose| method is called if the widget was overlapped and became
	visible again.  There are some basic checks to see if certain
	chunks need to be displayed. ie: are they above the exposed region or
	below it then they don't need to be displayed.  And if the remainder
	are partially or entirely within the region then they need to be
	displayed.
	
@proc expose
{
	TextChunk *chunk;
	int pos, w, cw;
	int ma, md;
	XFontStruct *f;
	char *s, *e;
		
	if(! XtIsRealized($))
		return ;

	if(event == NULL)
	{
		XClearWindow(XtDisplay($),XtWindow($));
		#expose($,event,region);
		return;
	}

	#expose($,event,region);

	compute_line_height($, $text, &ma, &md);
	pos=ma + $internalOffset;
	w=0;
	
	for(chunk=$text ; chunk ; chunk=chunk->next)
	{
		if(pos+md>=event->xexpose.y) break;
		if(chunk->nl)
		{
			pos+=md;
			compute_line_height($, chunk, &ma, &md);
			pos+=ma;
		}
	}
	
	for( ; chunk && (pos-ma)<(event->xexpose.y+event->xexpose.height); chunk=chunk->next)
	{
		switch(chunk->font)
		{
			case 2:		f=$font2;	break;
			case 3:		f=$font3;	break;
			case 4:		f=$font4;	break;
			default:	f=$font1;	break;
		}

		s=chunk->text;
		e=strchr(s, '\t');
		while(e!=NULL)
		{
			if(e-s!=0) {
				cw=XTextWidth(f,s,e-s);
				if(XRectInRegion(region, w, pos-ma, w+cw, pos+md) != RectangleOut)
					draw_string($, f, chunk->color, s, e-s, w, pos);
				w+=cw;
			}
			w+=(40-w%40)==0 ? 40 : (40-w%40);
			s=e+1;
			e=strchr(s, '\t');
		}
		cw=XTextWidth(f,s,strlen(s));
		if(XRectInRegion(region, w, pos-ma, w+cw, pos+md) != RectangleOut)
			draw_string($, f, chunk->color, s, strlen(s), w, pos);
		w+=cw;
		
		if(chunk->nl)
		{
			w=0;
			pos+=md;
			compute_line_height($, chunk, &ma, &md);
			pos+=ma;
		}
	}

}


@EXPORTS

@ The |XfwfAddText| function is used to add chunks of text to the end of
	the text structure.  The |data| is the text, |f| is the font number
	(1-4), |c| is the color (1-8) and |nl| means the there is a new line
	after this chunk.  If |data| is a NULL pointer then widget will resize
	itself to fit the new structure.  This is used so that when adding
	lots of chunks the widget is not re-calculating it's size all of
	the time.

@proc XfwfAddText($, char *data, int f, int c, int nl)
{
	TextChunk *chunk;
	Dimension w, h;
		
	assert(XtIsSubclass($, xfwfTextOutWidgetClass));

	if(data==NULL)
	{
		compute_size($, &w, &h);
		XtMakeResizeRequest($, w, h, NULL, NULL);
		return;
	}
	
	chunk=(TextChunk*)XtMalloc(sizeof(TextChunk));
	chunk->next=NULL;
	chunk->font=f;
	chunk->color=c;
	chunk->nl=nl;
	chunk->text=XtMalloc(1+strlen(data));
	strcpy(chunk->text,data);

	if($text==NULL)
		$text=chunk;
	else
		$tail->next=chunk;

	$tail=chunk;
}


@ The |XfwfEraseText| function is used to erase all of the chunks added
	to the text widget.  This is useful if you wish to add a new set of
	text to the widget.

@proc XfwfEraseText($)
{
	TextChunk *n, *c;
	
	c=$text;
	while(c)
	{
		n=c->next;
		XtFree((char *)c->text);
		XtFree((char *)c);
		c=n;
	}
	$text=NULL;
	$tail=NULL;
}

@ The |XfwfComputeTextOutSize| could be useful for sub-classes so we allow
	a hook into it.  There must be a better way!  Please tell me.

@proc XfwfComputeTextOutSize($, Dimension *w, Dimension *h)
{
	compute_size($, w, h);
}

@IMPORTS

@incl <assert.h>
@incl <ctype.h>
@incl <stdio.h>
@incl <string.h>
