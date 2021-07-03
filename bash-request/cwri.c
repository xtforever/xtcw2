/* most important define,
   check APPNAME.ad !
*/
#define APP_NAME "cwri"

/*
 */

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

#include "xtcw/Gauge.h"

char *m_str(int m) { return m_buf(m); }

static struct fork2_info *SPROC = NULL;
static XtInputId sprocid[2];
static Widget popShell = NULL;


Widget TopLevel;
int trace_main;
#define TRACE_MAIN 2

char *fallback_resources[] = {
    APP_NAME ".allowShellResize: False",
    "*WclResFiles:" APP_NAME ".ad\n",
    "*traceLevel: 1",
    NULL };

/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
  { "-TraceLevel",	"*traceLevel",	XrmoptionSepArg, NULL },
  WCL_XRM_OPTIONS
};

typedef struct CWRI_CONFIG {
    int traceLevel;
} CWRI_CONFIG;

#define FLD(n)  XtOffsetOf(CWRI_CONFIG,n)
static XtResource CWRI_CONFIG_RES [] = {

  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
   FLD(traceLevel), XtRImmediate, 0
  }

};
#undef FLD

struct CWRI_CONFIG CWRI;

void test_cb( Widget w, void *u, void *c )
{
  printf("cb\n");

  if(! XtIsSubclass(w, weditWidgetClass ) ) return;
  char *s;
  XtVaGetValues(w, "label", &s, NULL );
  if( is_empty(s) ) return;
  printf("Label: %s\n", s );

}

void quit_cb( Widget w, void *u, void *c )
{
    TRACE(1,"QUIT");
    XtAppSetExitFlag( XtWidgetToApplicationContext(w) );
}

void popdown_cb( Widget w, void *u, void *c )
{
    int letter = (intptr_t) u;
    TRACE(1,"popdown %c", letter );
    char msg[3] = { 0, 10, 0 };
    msg[0]=letter;
    
    fork2_write(SPROC, msg  );
    XtPopdown(popShell);
}


int do_sproc(int n, int buf)
{
    int err = fork2_read(SPROC, n);

    if( err ) {
	TRACE(1,"error pipe %d", n);
	XtRemoveInput(sprocid[n]); sprocid[n]=0;
	if( sprocid[0] == 0 &&
	    sprocid[1] == 0  ) {
	    fork2_close(SPROC);
	    SPROC=0;
	}
	return err;
    }
    
    err =  fork2_getline(SPROC,n,buf);
    if( !err  ) TRACE(1, "%s", m_str(buf) );
    return err;
}



int matchone( char *s, char *charset )
{
    int ch;
    while( ch=*charset++ ) {
	if( *s == ch ) break;
    }
    return ch;
}

int matchinside(char *s, char *charset )
{
    int ch;
    while(*s) {
	ch = matchone(s,charset);
	if( ch ) return ch;
	s++;
    }
    return 0;
}


void make_button(Widget gb, int gx, char *label, int ret )
{
    Widget bt = XtVaCreateManagedWidget( label, wbuttonWidgetClass, gb,
				  "label", label,
				  "gridy", 1,
				  "gridx", gx,
				  NULL );
	
    XtAddCallback(bt, "callback", popdown_cb,(XtPointer)(intptr_t) ret );	
}


void make_request(char *q, char *letters )
{
    static int question = 0;
    if( ! question ) question = m_create(20,1);

    int button_yes    = 0;
    int button_no     = 0;
    int button_abort  = 0;
    int button_cancel = 0;
    
    if( matchinside(letters, "jJyY") )
	button_yes = 1;
    if( matchinside(letters, "nN") )
	button_no = 1;
    if( matchinside(letters, "aAqQ") )
	button_abort = 1;
    if( matchinside(letters, "cC") )
	button_cancel = 1;

    if( popShell ) {
	XtDestroyWidget(popShell);
    }

    m_clear(question);
    m_write( question, 0, q, strlen(q) );
    m_putc(question,32); m_putc(question,'?');

    Widget shell2;
    popShell = shell2 = XtCreatePopupShell ( "shell2", topLevelShellWidgetClass,
                                  TopLevel, NULL, 0 );

    
    Widget gb = XtCreateManagedWidget ( "Widget", gridboxWidgetClass,
                            shell2, NULL, 0 );


    Widget bt; int gx=0;

    if( button_yes ) make_button(gb,gx++,"yes",'y');
    if( button_no ) make_button(gb,gx++,"no",'n');
    if( button_abort ) make_button(gb,gx++,"abort",'a');
    if( button_cancel ) make_button(gb,gx++,"cancel",'c');

    Widget lb = XtVaCreateManagedWidget( "header", wlabelWidgetClass, gb,
					 "label", m_str(question),
					 "gridy", 0,
					 "gridx", 0,
					 "gridWidth", gx,
					 NULL );
    
    XtPopup ( popShell, XtGrabNone );	
}


// find (YNCA) ? 
void parse_req(int buf)
{
    char *regex = "(.*)\\( *([YNCAynca/]*) *\\) *\\?";


    int m = m_regex( 0, regex, m_str(buf) );
    if( m_len(m) < 2 ) goto leave;

    TRACE(1,"Question: %s, Answers: %s", STR(m,1), STR(m,2) );

    make_request( STR(m,1), STR(m,2) );
    
 leave:
    m_free_strings(m,0);
}


static void sproc_stdout_cb( XtPointer p, int *n, XtInputId *id )
{
    int buf = m_create(100,1);
    if( do_sproc(0,buf) ) goto leave;
    parse_req(buf);
 leave:
    m_free(buf);
    return;
}

static void sproc_stderr_cb( XtPointer p, int *n, XtInputId *id )
{
    int buf = m_create(100,1);
    if( do_sproc(1,buf) == 0 ) TRACE(1,"%s", m_str(buf) );    
    m_free(buf);
}



void popup_cb( Widget w, void *u, void *c )
{
    Widget shell2;
    TRACE(1,"popup");

    /*
    if( ! popShell ) {
    popShell = shell2 = XtCreatePopupShell ( "shell2", topLevelShellWidgetClass,
                                  TopLevel, NULL, 0 );

    
    Widget gb = XtCreateManagedWidget ( "Widget", gridboxWidgetClass,
                            shell2, NULL, 0 );


    Widget lb = XtVaCreateManagedWidget( "header", wlabelWidgetClass, gb,
					 "label", "would you like to exit",
					 "gridy", 0,
					 "gridx", 0,
					 "gridWidth", 2,
					 NULL );
    
    Widget bt = XtVaCreateManagedWidget( "yes", wbuttonWidgetClass, gb,
					 "label", "yes",
					 "gridy", 1,
					 "gridx", 0,
			     NULL );
    XtAddCallback(bt, "callback", popdown_cb, 0 );

    bt = XtVaCreateManagedWidget( "no", wbuttonWidgetClass, gb,
					 "label", "no",
					 "gridy", 1,
					 "gridx", 1,
			     NULL );
    XtAddCallback(bt, "callback", popdown_cb, (XtPointer)1 );

    }
    
    XtPopup ( popShell, XtGrabNone );
    */
    
    if( ! SPROC ) {	
	SPROC = fork2_open( "./testshell.sh", "1", "2", "3", NULL );
	sprocid[0]=
	XtAppAddInput(XtWidgetToApplicationContext(TopLevel),
		      SPROC->fd[ CHILD_STDOUT_RD ], (XtPointer)  (XtInputReadMask),
		     sproc_stdout_cb, NULL );
	sprocid[1] =
    	XtAppAddInput(XtWidgetToApplicationContext(TopLevel),
		      SPROC->fd[ CHILD_STDERR_RD ],(XtPointer)  (XtInputReadMask),
		      sproc_stderr_cb, NULL );
    }
    
}



/* --------------------------------------------------------------------------------------------------------------------

                        IMPLEMENTATION


                        Must Provide
                        - RegisterApplication
                        - InitializeApplication

  -------------------------------------------------------------------------------------------------------------------- */

static void RegisterApplication ( Widget top )
{
    /* -- Register widget classes and constructors */
    // RCP( top, wbatt );
    RCP( top, gauge );
    
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_cb );
    RCB( top, test_cb );
    RCB( top, popup_cb );
}


/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/
static void InitializeApplication( Widget top )
{
    trace_level = CWRI.traceLevel;
}

/******************************************************************************
**  Private Functions
******************************************************************************/

static void syntax(void)
{
  puts( syntax_wcl );
  puts( "-TraceLevel <num>\n"
	"-ListenPort <num>\n" );
}







/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    trace_main = TRACE_MAIN;
    trace_level=1;
    XtAppContext app;
    m_init();
    TRACE(2,"test");
    XtSetLanguageProc (NULL, NULL, NULL);
    XawInitializeWidgetSet();

    /*  --  Intialize Toolkit creating the application shell
     */
    Widget appShell = XtOpenApplication (&app, APP_NAME,
             /* resources: can be set from argv */
             options, XtNumber(options),
	     &argc, argv,
	     fallback_resources,
	     sessionShellWidgetClass,
	     NULL, 0
	   );


    /*  --  Enable Editres support
     */
    XtAddEventHandler(appShell, (EventMask) 0, True, _XEditResCheckMessages, NULL);

    XtAddCallback( appShell, XtNdieCallback, quit_cb, NULL );

    /*  --  not parsed options are removed by XtOpenApplication
            the only entry left should be the program name
    */
    if (argc != 1) { m_destruct(); syntax(); exit(1); }
    TopLevel = appShell;


    /*  --  Register all application specific
            callbacks and widget classes
    */
    RegisterApplication ( appShell );

    /*  --  Register all Athena and Public
            widget classes, CBs, ACTs
    */
    XpRegisterAll ( app );

    /*  --  Create widget tree below toplevel shell
            using Xrm database
    */
    WcWidgetCreation ( appShell );


    /*  -- Get application resources and widget ptrs
     */
    XtGetApplicationResources(	appShell, (XtPointer)&CWRI,
				CWRI_CONFIG_RES,
				XtNumber(CWRI_CONFIG_RES),
				(ArgList)0, 0 );

    trace_level = CWRI.traceLevel;
    InitializeApplication(appShell);

    /*  --  Realize the widget tree and enter
            the main application loop  */
    XtRealizeWidget ( appShell );

    grab_window_quit( appShell );

    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);
    m_destruct();

    return EXIT_SUCCESS;
}
