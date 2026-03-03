#include <stdio.h>
#include <Xm/Xm.h>
#include <Xfwf/SSGML.h>

static Widget toplevel, test;
static XtAppContext app_context;

static String fallback_resources[] = {
    "SSGMLT.geometry: 250x300",
    "SSGMLT*background:	#ff92aa",

    "*test.fontFamily1: new century schoolbook",
    
    /*
     * Style sheet, including a background pixmap.
     * The background is coupled to the <BG> element,
     * but it could have been attached to any element.
     */

    "*test.style: \
*BG.empty: true\\n\
*BG.tile: \
/* XPM */\\\\n\\\
static char * tmp [] = {\\\\n\\\
/* width height ncolors cpp [x_hot y_hot] */\\\\n\\\
\"10 10 5 1 0 0\",\\\\n\\\
/* colors */\\\\n\\\
\" 	s iconGray1	m white	c #e1e1e1e1e1e1\",\\\\n\\\
\".	s iconGray2	m white	c #c8c8c8c8c8c8\",\\\\n\\\
\"X	s iconColor2	m white	c white\",\\\\n\\\
\"o	s iconGray4	m white	c #969696969696\",\\\\n\\\
\"O	s iconGray3	m white	c #afafafafafaf\",\\\\n\\\
/* pixels */\\\\n\\\
\"     .X   \",\\\\n\\\
\" .   .X o \",\\\\n\\\
\"  . . X o \",\\\\n\\\
\"   ..XXXo \",\\\\n\\\
\" XXXXX  o \",\\\\n\\\
\"Oooo.X   o\",\\\\n\\\
\"O . oooo o\",\\\\n\\\
\"X . X . oo\",\\\\n\\\
\"O   X  .  \",\\\\n\\\
\"    X   . \"};\\n\
*H1.size: +2\\n\
*H1.bold: True\\n\
*H1.prebreak: 2.0\\n\
*H1.ruleafter: 0.5\\n\
*H1.postbreak: 2.0\\n\
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
*B.bold: True",

    NULL,
};

static String txt = "\
<BG>\n\
<H1>A header</H1>\n\
\n\
<P>A paragraph, with some <EM>italicized</EM> and <B>bold</B>\n\
words in it.</P>\n\
\n\
<P>Another paragraph. Nothing special, except that it is\n\
much longer than the previous one. That is caused by\n\
the fact that there is so much nonsense in this paragraph.\n\
It only serves to demonstrate the justification.\n\
Well, just to make it interesting, there is a word\n\
at the end that is both\n\
bold and italicized. This is the <EM><B>word.</B></EM></P>\n\
";

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
    toplevel = XtVaAppInitialize(&app_context, "SSGMLT", NULL, 0,
				 &argc, argv, fallback_resources, NULL);
    test = XtVaCreateManagedWidget
	("test", xfwfSimpleSGMLWidgetClass, toplevel, NULL);
    /* XtAddCallback(ssgml, XtNactivate, activate, NULL); */
    XtRealizeWidget(toplevel);
    XfwfAddText(test, txt, strlen(txt));
    XfwfAddText(test, NULL, 0);
    XtAppMainLoop(app_context);
}
