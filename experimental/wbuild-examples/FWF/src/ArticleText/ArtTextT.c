#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <Xfwf/ScrollWin.h>
#include <Xfwf/ArtText.h>

static XtAppContext app;

static String fallback_resources[] = {
	"*.background: grey",
	"*.bodyFont: -adobe-times-medium-r-normal--12-*-*-*-p-*-iso8859-1",
	"*.quoteFont: -adobe-times-medium-r-normal--12-*-*-*-p-*-iso8859-1",
	"*.headerItemFont: -adobe-helvetica-bold-r-normal--12-*-*-*-p-*-iso8859-1",
	"*.headerValueFont: -adobe-helvetica-medium-r-normal--12-*-*-*-p-*-iso8859-1",
	"*.headerItemColor: darkolivegreen",
	"*.headerValueColor: darkolivegreen",
	"*.bodyColor: black",
	"*.quoteColor: blue",
	NULL
};

/*---------------------------------------------------------------------------*

	main(argc,argv)

	This is the main entry point of the scrolledList
	widget application program.

 *---------------------------------------------------------------------------*/

int main(argc,argv)
int argc;
char **argv;
{
	char *buf;
	FILE *f;
	int s;
	struct stat stats;
	Widget top,textOut,scr;

	if(argc<2)
	{
		fprintf(stderr, "Usage: %s <news_article_file or mail_message>\n", argv[0]);
		exit(1);
	}
	
	top = XtVaAppInitialize(&app,"TextOutT",NULL,0,&argc,argv,fallback_resources, NULL);

	scr = XtVaCreateManagedWidget("scroll", xfwfScrolledWindowWidgetClass,
					top, XtNwidth, 500, XtNheight, 700, NULL);
	textOut = XtVaCreateManagedWidget("textout", xfwfArticleTextWidgetClass,
					scr, NULL);

	XtRealizeWidget(top);

	f=fopen(argv[1], "r");
	fstat(fileno(f), &stats);
	s=stats.st_size;
	buf=XtMalloc(s+1);
	fread(buf, 1, s, f);
	XfwfSetArticleText(textOut, buf);

	XtAppMainLoop(app);
	return(1);
} /* End main */
