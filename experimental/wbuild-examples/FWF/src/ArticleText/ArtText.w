# XfwfArticleText - news/mail article text output widget
# Paul Kendall (paul@kcbbs.gen.nz)

@class XfwfArticleText (XfwfTextOut)	@file=ArtText

@ The |XfwfArticleText| widget is used to display news/mail articles in
	different fonts	and colors.


@PUBLIC

@ The text is drawn using the fonts given.

	@var <FontStruct> XFontStruct *bodyFont = <String> XtDefaultFont
	@var <FontStruct> XFontStruct *quoteFont = <String> XtDefaultFont
	@var <FontStruct> XFontStruct *headerItemFont = <String> XtDefaultFont
	@var <FontStruct> XFontStruct *headerValueFont = <String> XtDefaultFont

@ The color resources specify the colors used for drawing the text.

	@var Pixel bodyColor = <String> XtDefaultForeground
	@var Pixel quoteColor = <String> XtDefaultForeground
	@var Pixel headerItemColor = <String> XtDefaultForeground
	@var Pixel headerValueColor = <String> XtDefaultForeground

@ The |quoteChar| resource specifies the quote character used for changing
	color and font when parsing the article text.

	@var char quoteChar = '>'

@METHODS

@proc initialize
{
	$color1 = $bodyColor;
	$color2 = $quoteColor;
	$color3 = $headerItemColor;
	$color4 = $headerValueColor;
	$font1 = $bodyFont;
	$font2 = $quoteFont;
	$font3 = $headerItemFont;
	$font4 = $headerValueFont;
}

@proc set_values
{
	int do_size = False;

	$color1 = $bodyColor;
	$color2 = $quoteColor;
	$color3 = $headerItemColor;
	$color4 = $headerValueColor;

	if($old$quoteChar != $quoteChar)
		do_size = True;

	if($old$bodyFont != $bodyFont)
	{
		$font1 = $bodyFont;
		do_size = True;
	}
	
	if($old$quoteFont != $quoteFont)
	{
		$font2 = $quoteFont;
		do_size = True;
	}
	
	if($old$headerItemFont != $headerItemFont)
	{
		$font3 = $headerItemFont;
		do_size = True;
	}
	
	if($old$headerValueFont != $headerValueFont)
	{
		$font4 = $headerValueFont;
		do_size = True;
	}
	
	if(do_size)
		XfwfComputeTextOutSize($, &($width), &($height));

	return do_size;
}


@EXPORTS

@ The |XfwfSetArticleText| function is used to build up the text chunks to be
	displayed in the widgets window.  It parses the data for the break
	between the header and body of the message and changes font and
	color accordingly.  Also when parsing the body of the message, if the
	first character of the line is the |quoteChar| then the |quoteFont|
	and |quoteColor| is used to display the text.  If there was a previous
	set of text then this is removed first.

@proc XfwfSetArticleText($, char *data)
{
	char *header = data;
	char *ptr, t;
	char *body;
	
	assert(XtIsSubclass($, xfwfArticleTextWidgetClass));

	XfwfEraseText($);
	body = strstr(data, "\n\n");
	
	while(header < body)
	{
		if(header[0]!=' ' && (ptr = strchr(header, ':')) != NULL)
		{
			ptr+=2;
			t=*ptr;
			*ptr=0;
			XfwfAddText($, header, 3, 3, 0);
			*ptr=t;
			header = strchr(header, '\n');
			if(header!=NULL)
			{
				t=*header;
				*header=0;
			}
			XfwfAddText($, ptr, 4, 4, 1);
			if(header!=NULL)
			{
				*header=t;
				header++;
			}
			else
				header=body;
		}
		else
		{
			ptr = strchr(header, '\n');
			if(ptr!=NULL) *ptr=0;
			XfwfAddText($, header, 4, 4, 1);
			if(ptr!=NULL)
			{
				header = ptr+1;
				*ptr='\n';
			}else{
				header=body;
			}
		}
	}
	
	XfwfAddText($, "", 3, 1, 1);

	while(*body)
	{
		ptr=strchr(body, '\n');
		if(ptr!=NULL)
		{
			t=*ptr;
			*ptr=0;
		}
		if(*body == $quoteChar)
			XfwfAddText($, body, 2, 2, 1);
		else
			XfwfAddText($, body, 1, 1, 1);
		if(ptr==NULL)
			break;
		*ptr=t;
		body=ptr+1;
	}
	
	XfwfAddText($, NULL, 0, 0, 0);
}

@IMPORTS

@incl <assert.h>
@incl <ctype.h>
@incl <stdio.h>
@incl <string.h>
@incl <Xfwf/TextOut.h>
