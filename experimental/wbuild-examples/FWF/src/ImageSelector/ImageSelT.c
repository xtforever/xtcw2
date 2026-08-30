/*
 * Author:
 * 	Brian Totty
 * 	Department of Computer Science
 * 	University Of Illinois at Urbana-Champaign
 *	1304 West Springfield Avenue
 * 	Urbana, IL 61801
 * 
 * 	totty@cs.uiuc.edu
 * 	
 */ 

#include <stdio.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <Xfwf/ImageSel.h>

XfwfImage *		CreateImagesFromFont();
Pixmap 			CreatePixmapFromFontChar();

char *help_text[] =
{
	"",
	"\tThis program demonstrates a possible application of the",
	"\tXfwfImageSelector widget.  The widget displays a grid of",
	"\timages (Pixmaps), and allows the images to be selected.",
	"",
	"\tThis program displays a set of characters from a font which",
	"\tis resident in your X server.  The first argument of this",
	"\tprogram is a name of a resident font.  The second argument",
	"\tis the ASCII value of the first character to display.  The",
	"\tthird argument is the ASCII value of the final character to",
	"\tshow.",
	"",
	"\tThe program will create an XfwfImageSelector widget and load it",
	"\twith the images of the selected characters from the font.",
	"",
	"\tFor example, the command:",
	"",
	"\t\tImageSelT variable 65 90",
	"",
	"\twill display the uppercase characters A to Z, from the font named",
	"\t'variable'.  These images can then be selected.",
	NULL
};


int main(argc,argv)
int argc;
char **argv;
{
	char **p;
	int from,to;
	Widget top,is;
	Arg args[3];
	char *font_name;
	XFontStruct *font;
	XfwfImage *images;
	XtAppContext app;

	XtSetArg(args[0],XtNallowShellResize,True);
	top = XtAppInitialize(&app,"XImageSelector",NULL,0,
		(Cardinal*)(&argc),argv,NULL,args,1);
	
	if (argc != 4)
	{
		fprintf(stderr,"usage: %s fontname start_index end_index\n",
			argv[0]);
		for (p = help_text; *p != NULL; p++)
			fprintf(stderr,"%s\n",*p);
		exit(1);
	}
	font_name = argv[1];
	from = atoi(argv[2]);
	to = atoi(argv[3]);

	font = XLoadQueryFont(XtDisplay(top),font_name);
	if (font == NULL)
	{
		fprintf(stderr,"Can't load font '%s'\n",font_name);
		exit(1);
	}

	images = CreateImagesFromFont(top,font,from,to);
	
	XtSetArg(args[0],XtNnumImages,to - from + 1);
	XtSetArg(args[1],XtNimages,images);

	is = XtCreateManagedWidget("IS",xfwfImageSelectorWidgetClass,
		top,args,2);

	XtRealizeWidget(top);
	XtAppMainLoop(app);
	return(0);
}


XfwfImage *CreateImagesFromFont(w,font,start,end)
Widget w;
XFontStruct *font;
int start,end;
{
	int i,count;
	XfwfImage *data;

	count = end - start + 1;
	if (count <= 0)
	{
		fprintf(stderr,"Character count must be > 0\n");
		exit(1);
	}

	data = (XfwfImage *)calloc(count,sizeof(XfwfImage));
	for (i = 0; i < count; i++)
	{
		data[i].label = (char *)calloc(16,sizeof(char));
		sprintf(data[i].label,"%d (%c)",i + start,i + start);
		data[i].pixmap = CreatePixmapFromFontChar(w,font,i + start);
		data[i].selected = False;
		data[i].selectable = True;
	}

	return(data);
}


Pixmap CreatePixmapFromFontChar(w,font,c)
Widget w;
XFontStruct *font;
int c;
{
	Display *dpy;
	int scr;
	GC draw_gc,erase_gc;
	XGCValues gc_values;
	char buf[32];
	Pixmap pixmap;
	int width,height,depth;

	dpy = XtDisplay(w);
	scr = XScreenNumberOfScreen(XtScreen(w));

	gc_values.foreground = BlackPixel(dpy,scr);
	gc_values.font = font->fid;
	draw_gc = XtGetGC(w,GCForeground | GCFont,&gc_values);
	gc_values.foreground = WhitePixel(dpy,scr);
	erase_gc = XtGetGC(w,GCForeground,&gc_values);

	sprintf(buf,"%c",c);
	width = XTextWidth(font,buf,strlen(buf));
	height = font->max_bounds.ascent + font->max_bounds.descent;
	depth = XDisplayPlanes(dpy,scr);
	if (width > 0 && height > 0)
	{
		pixmap = XCreatePixmap(dpy,RootWindow(dpy,scr),
				       width,height,depth);
		XFillRectangle(dpy,pixmap,erase_gc,0,0,width,height);
		XDrawString(dpy,pixmap,draw_gc,0,(font->max_bounds.ascent),buf,
			    strlen(buf));
	}
	    else
	{
		pixmap = None;
	}

	XtReleaseGC(w,draw_gc);
	XtReleaseGC(w,erase_gc);

	return(pixmap);
}
