#include "script_call.h"


#include "mls.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>              /* Obtain O_* constant definitions */


#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>
#include <X11/Vendor.h>
#include <X11/Xaw/XawInit.h>

#include <WcCreate.h>
#include <Xp.h>
#include <xutil.h>
#include "wcreg2.h"
#include <xtcw/register_wb.h>
#include "subshell.h"
#include "m_tool.h"
#include "mrb.h"

#include "SelectReq.h"


int trace_script_call = 2;
static Widget TopLevel = 0;
static int SPROC;
static XtInputId sprocid[2];
static Widget popShell = NULL;
static int TERM_FH=-1;

static void term_open()
{
    if( TERM_FH != -1 ) close(TERM_FH);
    char *s = "script.fifo";
    mkfifo( s, 0600 );
    int cmd = s_printf(0,0,"xterm -e 'cat < %s' &", s);
    system( m_str(cmd) );
    TRACE(1, "exec: %s", m_str(cmd) );
    TERM_FH = open( s, O_WRONLY  );
    ASSERT(TERM_FH != -1 );   
    m_free(cmd);
}


/* called by yes/no requester */
static void popdown_cb( Widget w, void *u, void *c )
{
    int letter = (intptr_t) u;
    TRACE(1,"popdown %c", letter );
    char msg[3] = { 0, 10, 0 };
    msg[0]=letter;
    
    shell_write(SPROC, msg  );
    // XtPopdown(popShell);
    XtDestroyWidget(popShell);
    popShell=0;
}



static void make_button(Widget gb, int gx, char *label, int ret )
{
    Widget bt = XtVaCreateManagedWidget( label, wbuttonWidgetClass, gb,
				  "label", label,
				  "gridy", 1,
				  "gridx", gx,
				  NULL );
	
    XtAddCallback(bt, "callback", popdown_cb,(XtPointer)(intptr_t) ret );	
}


#define B_YES      1
#define B_NO       2
#define B_ABORT    4
#define B_CANCEL   8
#define B_OK       16

static void make_yn_request(char *q, char *letters )
{
    static int question = 0;
    if( ! question ) question = m_create(20,1);

    int button = 0;    
    static char btab[256] = { ['y']=B_YES, ['n']=B_NO, ['a']=B_ABORT, ['c']=B_CANCEL, ['o']=B_OK };
    for( char *s=letters; *s; s++) { button |= btab[ tolower(*s) ]; }
    
    if( popShell ) {
	XtDestroyWidget(popShell);
    }

    s_printf(question,0,"%s ?", q );

    Widget shell2;
    popShell = shell2 = XtCreatePopupShell ( "shell2", topLevelShellWidgetClass,
                                  TopLevel, NULL, 0 );
    
    Widget gb = XtCreateManagedWidget ( "Widget", gridboxWidgetClass,
                            shell2, NULL, 0 );

    int gx=0;
    if( button & B_YES ) make_button(gb,gx++,"yes",'y');
    if( button & B_NO ) make_button(gb,gx++,"no",'n');
    if( button & B_ABORT ) make_button(gb,gx++,"abort",'a');
    if( button & B_CANCEL ) make_button(gb,gx++,"cancel",'c');
    if( button & B_OK ) make_button(gb,gx++,"ok",'o');

    XtVaCreateManagedWidget( "header", wlabelWidgetClass, gb,
					 "label", m_str(question),
					 "gridy", 0,
					 "gridx", 0,
					 "gridWidth", gx,
					 NULL );
    
    XtPopup ( popShell, XtGrabNone );
    TRACE(3, "POPUP STARTET");
}

/*
item list:
1. hello
question
 */


static struct list_box_st {
    int lines;
    int state;
    int question;
    int answer;
    Widget shell;
} PARSE_LIST;

static void parse_selection_cleanup(void);

static void select_req_cb(Widget w, void *u, void *d )
{
    struct list_box_st *rq = u;
    int sel = (intptr_t) d; 
    char *msg;
    asprintf(&msg,"%d\n", sel );
    shell_write(SPROC, msg  );
    XtDestroyWidget(rq->shell);
    rq->shell=0;
    free(msg);
    parse_selection_cleanup();
}

static void  make_list_request( struct list_box_st *rq )
{
    char **d;
    int p,m = rq->lines;
    if( !m || !m_len(m) ) return;
    m_foreach(m,p,d) {
	TRACE(2,"%d. %s", p, *d);
    }
    TRACE(2, "Frage: %s", m_str( rq->question) );

    if( rq->shell ) 
	XtDestroyWidget(rq->shell);
    
    rq->shell = XtCreatePopupShell ( "shell_list_req", topLevelShellWidgetClass,
                                  TopLevel, NULL, 0 );
        
    Widget w = XtVaCreateManagedWidget ( "requester", selectReqWidgetClass,
				       rq->shell,
				       "lines", rq->lines,
				       NULL );

    XtAddCallback(w, "callback", select_req_cb, (XtPointer) rq );
     
    XtPopup ( rq->shell, XtGrabNone );
    TRACE(3, "POPUP STARTET");

}


static void parse_selection_cleanup(void)
{
    PARSE_LIST.state=0;
    if( PARSE_LIST.lines ) { m_free_strings( PARSE_LIST.lines,0 ); PARSE_LIST.lines=0; }
    if( PARSE_LIST.question ) { m_free( PARSE_LIST.question); PARSE_LIST.question=0; }
}


static int parse_list_header(int buf)
{        
    // find select box
    if( mstrcmp( buf,0, "item list:" ) == 0 ) {
	TRACE(3,"header found");
	parse_selection_cleanup();
	PARSE_LIST.state=1;
    }

    return PARSE_LIST.state;
}


// scan buf for '00. item'
// returns 1 - item found
static int parse_list_item(int buf)
{
    int res_ok = 0;
    
    // match with NUMBER DOT SPACE ITEM-TEXT
    char *regex = "^[[:space:]]*([0-9]+)\\.[[:space:]]*(.*)$";
    int m = m_regex( 0, regex, m_str(buf) );
    if( m_len(m) > 1 ) { 
	TRACE(3,"found: '%s' '%s'", STR(m,1), STR(m,2) );
	PARSE_LIST.lines = m_str_app( PARSE_LIST.lines, STR(m,2) , NULL );


#if 0	/* check list-lines array */
	printf("\n\nDEBUG------------------\n")
	int ln = PARSE_LIST.lines;
	int p = m_len(ln);
	for(int i = 0; i<p; i++ )
	    {
		printf( "%d. '%s'\n", i, STR(ln,i) );
	    }
#endif	
	res_ok = 1;
    }
    
    m_free_strings(m,0);
    return res_ok;
}


// returns 1 - still scanning
static int parse_list(int buf)
{
    int ret;
    
    if( PARSE_LIST.state == 0 ) { // not scanning
	ret = parse_list_header(buf);
	return ret; // 1 - header found
    }

    
    if( parse_list_item(buf) ) return 1; // 1 - item found
    TRACE(3,"item not found, guessing question");
    // no item check if we have a question or empty string

    PARSE_LIST.question = m_dub( buf );

    make_list_request( &PARSE_LIST );
    PARSE_LIST.state = 0; 
    return 1; // ignore this line   
}


// find some marker in the output:
// ( YNCA ) ?   - yes/no selection
// item list:
// 00. item
// ?
//
static void parse_req(int buf)
{
    int n=m_len(buf);
    for( int i=0 ; i<n; i++ ) {
	int ch = CHAR(buf,i);
	if( isprint(ch) ) fprintf(stderr,"%c", ch );
	else fprintf(stderr,":%.2x:", ch );
    }
    

    if( n==1 &&  CHAR(buf,0) == 0 )
	printf("breakpoint\n");
    
    
    TRACE(3, "parsing" );
    
    if( parse_list(buf)  ) return;

    TRACE(3,"parsing other request");
    // check if buf contains a yes/no question 
    char *regex = "(.*)\\( *([YNCAyncaoO/]*) *\\) *\\?";
    int m = m_regex( 0, regex, m_str(buf) );
    if( m_len(m) > 1 ) { 
	TRACE(2,"Question: %s, Answers: %s", STR(m,1), STR(m,2) );
	make_yn_request( STR(m,1), STR(m,2) );
    }  
  
    m_free_strings(m,0);
}



static void term_output(int buf )
{
    write( TERM_FH, m_buf(buf), m_len(buf) );
    write( TERM_FH, "\n", 1 );

    printf("%s\n", m_str(buf) );
}


static void sproc_parse( int n )
{
    n=0; // only stdout supported
    TRACE(1, "" );
    int err;
    int buf = m_create(100,1);

    // new line of data available if err==1 
    while( (err=shell_getline( SPROC, n, buf)) == 1 ) {
	term_output( buf );
	parse_req(buf);
	m_clear(buf);
    }

    /* special case - we have a line ending with '?' this could be
       a question without a newline  
    */
    struct mrb *mb = shell_queue(SPROC,n);
    if( ! mb ) goto leave; /* the child process is dead */
    int ch = mrb_last( mb );
    if( ch != '?' ) goto leave;

    // fetch line from input queue and handle it 
    m_clear(buf);
    mrb_get_line(mb,buf);
    m_putc(buf,0);
    term_output( buf );
    parse_req(buf);	
    
 leave:

    /* error handling - could loose data on stderr, but i dont care */
    if( err < 0 ) {
	TRACE(1,"error reading pipe %d, closing subshell", n);
	XtRemoveInput(sprocid[n]);
	sprocid[n]=0; 
	shell_close(SPROC);
	SPROC=0;
    }

    m_free(buf);
    TRACE(1, "leave" );
    return;
}


static void sproc_stdout_cb( XtPointer p, int *n, XtInputId *id )
{
    TRACE(1,"");
    sproc_parse(0);
}

#if 0
static void sproc_stderr_cb( XtPointer p, int *n, XtInputId *id )
{
    return;
    if( shell_check(SPROC) ) {
	char buf[1024];
	puts("SIGCHLD in STDERR CB");
	read( shell_fd(SPROC, CHILD_STDERR_RD), buf, 1024 );
	puts("read returns");
	return; /* prevent blocking read */
    }
    // char buf[1024];
    // read(shell_fd(SPROC, CHILD_STDERR_RD),buf,sizeof(buf) );
    int err,buf = m_create(100,1);
    // new line of data available if err==1 
    while( (err=shell_getline( SPROC, 1, buf)) == 1 ) {
	term_output( buf );
	m_clear(buf);
    }
    m_free(buf);
}
#endif


int run_script(Widget top, int args )
{
    TopLevel=top;
    XtAppContext app = XtWidgetToApplicationContext(top);


    if( SPROC ) {
	WARN("script already running");
	return -1;
    }
    
    term_open(); /* log output to xterm via fifo */
    SPROC = shell_create( args );
    if( SPROC < 0 ) {
	return -1;
    }
    signal(SIGCHLD, shell_signal_cb);
    
    sprocid[0]=
	XtAppAddInput(app,
		      shell_fd(SPROC,  CHILD_STDOUT_RD), (XtPointer)  (XtInputReadMask),
		      sproc_stdout_cb, NULL );
    /* sprocid[1] = */
    /* 	XtAppAddInput(app, */
    /* 		      shell_fd(SPROC, CHILD_STDERR_RD), (XtPointer)  (XtInputReadMask), */
    /* 		      sproc_stderr_cb, NULL ); */
    

    return 0;
}
