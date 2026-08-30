#include <stdio.h>
#include <Xm/Xm.h>
#include <Xfwf/HTML2.h>
#include <fcntl.h>

static Widget toplevel, test;
static XtAppContext app_context;

static String fallback_resources[] = {
    "HTML2T.geometry: 375x400",
    "HTML2T*background:	#ff92aa",

    /* Style sheet */
    /* "*textbackground: transparent", */
    "*style:\
*H1.size: +2\\n\
*H1.bold: True\\n\
*H1.prebreak: 2.0\\n\
*H1.postbreak: 1.5\\n\
*H1.justify: center\\n\
*H1.leftindent: 2.0\\n\
*H1.rightindent: 2.0\\n\
*P.prebreak: 1.0\\n\
*P.postbreak: 1.0\\n\
*P.leftindent: 2.0\\n\
*P.rightindent: 2.0\\n\
*EM.textcolor: red\\n\
*EM.slant: True\\n\
*EM.textbackground: #ffff55\\n\
*B.bold: True\\n\
*TT.family: tt",
    
    NULL,
};

static String txt = "\
<H1>Test program for the HTML widget</H1>\n\
\n\
<P>Call this program with a <B>file name</B> as argument\n\
or with `-' for standard input.\n\
\n\
<P>If the file contains HTML-like tags, such as\n\
<TT>&lt;H1&gt</TT>, <TT>&lt;P&gt;</TT> and <TT>&lt;EM&gt;</TT>, they will\n\
be formatted according to the style sheet resource.\n\
";


/* ARGSUSED */
static void get_file_input(client_data, fid, id)
    XtPointer client_data;
    int *fid;
    XtInputId *id;
{
    char buf[BUFSIZ];
    int nbytes, i;

    if ((nbytes = read(*fid, buf, BUFSIZ)) == -1)
	perror("get_file_input");
    else
	XfwfAddText((Widget) client_data, buf, nbytes);
}


/* ARGSUSED */
static void activate(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
}


void main(argc, argv)
    int argc;
    String *argv;
{
    int fd;

    toplevel = XtVaAppInitialize(&app_context, "HTML2T", NULL, 0,
				 &argc, argv, fallback_resources, NULL);
    test = XtVaCreateManagedWidget
	("test", xfwfHTML2WidgetClass, toplevel, NULL);
    /* XtAddCallback(test, XtNactivate, activate, NULL); */
    /* XtAddCallback(test, XtNresolveURL, resolve, NULL); */
    XtRealizeWidget(toplevel);
    if (argc < 2) {
	XfwfAddText(test, txt, strlen(txt));
	XfwfAddText(test, NULL, 0);
    } else if (strcmp(argv[1], "-") == 0) {
	fd = fileno(stdin);
    } else if ((fd = open(argv[1], O_RDONLY)) == -1) {
	perror(argv[1]);
	exit(1);
    } else {
	XtAppAddInput(app_context, fd, (XtPointer) XtInputReadMask,
		      get_file_input, test);
    }
    XtAppMainLoop(app_context);
}
