/*--------------------------------------------------------------------
 |  AnsiTermT -- test program for the AnsiTerm widget
 | 
 |  The test program creates a toplevel widget with an AnsiTerm widget
 |  in it and starts `sh' as a subprocess, with I/O redirected through
 |  the widget.
 | 
 |  To achieve this, a callback is installed in the widget to capture
 |  the user's key presses and an input handler is registered with Xt
 |  to capture output from the shell.
 | 
 |  Before starting the sub-process, the environment variable TERM is
 |  set to `ansi'. If all goes well, the result should be not too
 |  dissimilar to an xterm.
 |
 |  It won't work too well with many applications, however, because the
 |  connection is via a pipe and not a pty, which induces many programs
 |  to buffer I/O. You can try it with pty's if you wish: apart from
 |  the code to open the file descriptors nothing changes.
 |
 |  If you feel adventurous, try defining USE_PTY and see what happens.
 | 
 |  Bert Bos <bert@let.rug.nl> (31 Oct 1994)
 +--------------------------------------------------------------------*/

#define USE_PTY

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>				/* Needed on some systems */
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#ifdef USE_PTY
#include <sys/ptyio.h>
#endif
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xfwf/AnsiTerm.h>
#include <Xfwf/Board.h>
#include <Xfwf/Slider2.h>
#include <Xfwf/Enforcer.h>



/*--------------------------------------------------------------------
 |  Provide rather strict fallback resources, to make the program work
 |  even if people have very strange resources in their X server.
 |  (This is not user-friendly, but then it's only a test.)
 +--------------------------------------------------------------------*/
 
static String fallback_resources[] = {
    "AnsiTermT.board.location: 0 0 600 300",
/*    "AnsiTermT*borderWidth: 1",			/* For all widgets */
    "AnsiTermT*frameWidth: 2",			/* 3D border */

    "AnsiTermT*enforcer.location: 0 0 0.75 1.0",
    "AnsiTermT*ansiterm.geometry: 100x100",
    "AnsiTermT*ansiterm.outerOffset: 6",	/* 6 pixel margin around */
    "AnsiTermT*ansiterm.cursor: xterm",		/* Just an example */
    "AnsiTermT*font: -*-courier-medium-r-*--*-120-*-*-*-*-iso8859-1",
    "AnsiTermT*ansiterm.background: black",
    "AnsiTermT*ansiterm.foreground: green",
    "AnsiTermT*ansiterm.frameType: sunken",
    "AnsiTermT*ansiterm.margin: 0",

    "AnsiTermT*slider.location: 0.75 0 0.25 1.0",
    "AnsiTermT*slider.label: locator",
    "AnsiTermT*slider.foreground: white",
    "AnsiTermT*slider.alignment: top left",
    "AnsiTermT*slider.thumbColor: #ff00aa",
    NULL,
};



/*--------------------------------------------------------------------
 |  syserr is a utility function that writes an error message and exits
 +--------------------------------------------------------------------*/

static void syserr(s)
    char *s;
{
    perror(s);
    kill(0, SIGKILL);				/* Raise signal */
    exit(1);
}



/*--------------------------------------------------------------------
 |  The key_cb callback is invoked whenever the user presses a key in
 |  the AnsiTerm widget. The call_data argument contains a pointer to
 |  the corresponding character. (Note that it is a character, not a
 |  key event or keysym.)
 |
 |  The client_data is a pointer to the file descriptor of the pipe
 |  to the child process.
 +--------------------------------------------------------------------*/

static void key_cb(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    int *fd = (int *) client_data;
    char *data = (char *) call_data;

    if (write(*fd, data, 1) == -1) syserr("write");
}



/*--------------------------------------------------------------------
 |  The out_cb function is called when there is data available from
 |  the supprocess. It reads the data and passes it to the AnsiTerm
 |  widget.
 |
 |  client_data holds the ID of the AnsiTerm widget to which the data
 |  must be sent.
 +--------------------------------------------------------------------*/

static void out_cb(client_data, fid, id)
    XtPointer client_data;
    int *fid;
    XtInputId *id;
{
    Widget term = (Widget) client_data;
    char buf[BUFSIZ];
    int nbytes;

    if ((nbytes = read(*fid, buf, sizeof(buf))) == -1) syserr("read");
    XfwfAnsiWrite(term, buf, nbytes);
    /* fprintf(stderr, " [%d]", nbytes); */
}



#ifdef USE_PTY
/*--------------------------------------------------------------------
 |  openpty tries all pty's it knows about in order, until it finds one
 |  that allows both master and slave sides to be opened. All names
 |  /dev/pty[a-z][0-9a-z] are tried.
 +--------------------------------------------------------------------*/

#define SUFFIX1 "pqrstuvwxyzabcdefghijklmno"	/* Tried in order (major) */
#define SUFFIX2 "0123456789abcdef"		/* Tried in order (minor) */
#define DEVPTY1 "/dev/pty??"			/* Name template of pty's */
#define DEVPTY2 "/dev/tty??"			/* Template of slave pty's */

static Bool openpty(master_return, slave_return)
    int *master_return, *slave_return;
{
    static char suffix1[] = SUFFIX1, suffix2[] = SUFFIX2;
    static char mastername[] = DEVPTY1, slavename[] = DEVPTY2;
    char *p, *q;

    for (p = suffix1; *p; p++) {
	mastername[sizeof(DEVPTY1)-3] = *p;
	slavename[sizeof(DEVPTY2)-3] = *p;
	for (q = suffix2; *q; q++) {
	    mastername[sizeof(DEVPTY1)-2] = *q;
	    slavename[sizeof(DEVPTY2)-2] = *q;
	    if ((*master_return = open(mastername, O_RDWR)) == -1) continue;
	    if ((*slave_return = open(slavename, O_RDWR)) != -1) return TRUE;
	    close(*master_return);
	}
    }
    return FALSE;
}
#endif /* USE_PTY */



/*--------------------------------------------------------------------
 |  When the child process dies, finish() executes a wait() to clean
 |  up the zombie and then exits the parent, with the status code that
 |  the child returned.
 +--------------------------------------------------------------------*/

static void finish(sig)
    int sig;
{
    int stat;

    printf("Child died; exiting\n");
    wait(&stat);
    exit((stat >> 8) & 0377);
    /* NOTREACHED */
}

/*--------------------------------------------------------------------
 |  When the parent receives a signal, it reraises the signal, sending
 |  it both to itself and to its child.
 +--------------------------------------------------------------------*/

static void resetandexit(sig)
    int sig;
{
    kill(0, sig);
}



/*--------------------------------------------------------------------
 |  The main body initializes the X Toolkit, creates the sub-process
 |  and installs the callback and the input handler.
 +--------------------------------------------------------------------*/

void main(argc, argv)
    int argc;
    char *argv[];
{
    XtAppContext app_context;
    Widget toplevel, ansiterm, board, slider, enforcer;
    pid_t child;
#ifndef USE_PTY
    int pfdout[2], pfdin[2];
#else
    int master, slave;
#endif

    toplevel = XtVaAppInitialize(&app_context, "AnsiTermT", NULL, 0, &argc,
				 argv, fallback_resources, NULL);
    board = XtVaCreateManagedWidget("board", xfwfBoardWidgetClass,
				    toplevel, NULL);
    enforcer = XtVaCreateManagedWidget("enforcer", xfwfEnforcerWidgetClass,
				       board, NULL);
    ansiterm = XtVaCreateManagedWidget("ansiterm", xfwfAnsiTermWidgetClass,
				       enforcer, NULL);
    slider = XtVaCreateManagedWidget("slider", xfwfSlider2WidgetClass,
				     board, NULL);
    XfwfConnectScrollingWidgets(ansiterm, slider);

    /*----------------------------------------------------------------
     |	Create two pipes: pfdout for data from child to parent
     |	and pfdin for data from parent to child. Fork off a child
     |	process and redirect its standard input and output to the pipes.
     |	Close file descriptores that are no longer needed. Set the
     |	TERM environment variable. Execute `sh'.
     +----------------------------------------------------------------*/

#ifndef USE_PTY
    if (pipe(pfdout) == -1 || pipe(pfdin) == -1) syserr("pipe");
#else
    if (! openpty(&master, &slave)) syserr("pty");
    /* Enable flow control */
#if 0
    if (ioctl(master, TIOCREMOTE, TRUE) == -1) syserr("ioctl");
#endif
#endif
    signal(SIGCHLD, finish);			/* Catch dying children */
    signal(SIGHUP, resetandexit);
    /* signal(SIGINT, resetandexit); */
    signal(SIGQUIT, resetandexit);
    signal(SIGABRT, resetandexit);
    signal(SIGTERM, resetandexit);

    switch ((child = fork())) {
    case -1:
	syserr("fork");
	/* NOTREACHED */
    case 0:
	setsid();				/* Dissociate from terminal */
#ifndef USE_PTY
	if (close(0) == -1) syserr("close");
	if (dup(pfdin[0]) != 0) syserr("dup failed\n");
	if (close(1) == -1) syserr("close");
	if (dup(pfdout[1]) != 1) syserr("dup failed\n");
	if (close(2) == -1) syserr("close");
	if (dup(pfdout[1]) != 2) syserr("dup failed\n");
#else
	if (close(0) == -1) syserr("close");
	if (dup(slave) != 0) syserr("dup failed\n");
	if (close(1) == -1) syserr("close");
	if (dup(slave) != 1) syserr("dup failed\n");
	if (close(2) == -1) syserr("close");
	if (dup(slave) != 2) syserr("dup failed\n");
#endif
	{
	    long c, tblsiz = sysconf(_SC_OPEN_MAX);
	    for (c = 3; c < tblsiz; c++) (void) close(c);
	}
	putenv("TERM=ansi");
#if 0
	execlp("telnet", "telnet", "www.cern.ch", NULL);
#else
	execlp("ksh", "-ksh", NULL);
#endif
	syserr("execlp");
	/* NOTREACHED */
    }
#ifndef USE_PTY
    if (close(pfdout[1]) == -1 || close(pfdin[0]) == -1) syserr("close");
#else
    if (close(slave) == -1) syserr("close");
#endif

    /*----------------------------------------------------------------
     |	Install input handler to read pfdout[0] pipe.
     |	Install callback to pass key presses on to pfdin[1].
     +----------------------------------------------------------------*/

#ifndef USE_PTY
    XtAppAddInput(app_context, pfdout[0], (XtPointer) XtInputReadMask,
		  out_cb, ansiterm);
    XtAddCallback(ansiterm, XtNkeyCallback, key_cb, &pfdin[1]);
#else
    XtAppAddInput(app_context, master, (XtPointer) XtInputReadMask,
		  out_cb, ansiterm);
    XtAddCallback(ansiterm, XtNkeyCallback, key_cb, &master);
#endif

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_context);
}
