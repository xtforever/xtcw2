/* most important define,
   check APPNAME.ad !
*/
#define APP_NAME "cwri"

/* speed of gauge update */
#define UPDATE_TIMER_MS 2000
#define SENSOR_TIMER_MS 2000
/*
 */



/*
  gui - x resource


  *p.wcClass: gauge3
  *p.sensor:  diskstats
  *p.label:   device[0]
  *p.value:   read[0] write[0]
  *p.graph:   

  

 */




#include "mls.h"
#include "micro_vars.h"
#include "sensorreg.h"
#include "conststr.h"
#include "var5.h"


#include <signal.h>
#include <stdlib.h>
#include <time.h>

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


#include "WcCreate.h"
#include "Xp.h"
#include "xutil.h"
#include "wcreg2.h"
#include "xtcw/register_wb.h"
#include "nbus.h"
#include "subshell.h"
#include "m_tool.h"

#include "xtcw/Gauge.h"
#include "xtcw/Gauge2.h"
#include "xtcw/Repeatgb.h"
#include "xtcw/WlabelV5.h"

Widget TopLevel;
int trace_main;
#define TRACE_MAIN 7
static XtAppContext APPCTX;

char *fallback_resources[] = {
	APP_NAME ".allowShellResize: True",
    "*WclResFiles:" APP_NAME ".ad\n",
    "*traceLevel: 1",
    NULL };

/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
  { "-TraceLevel",	"*traceLevel",	XrmoptionSepArg, NULL },
  { "-ListenPort",      "*listenPort",  XrmoptionSepArg, NULL },  WCL_XRM_OPTIONS
};

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++***/
/* define resource struct */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++***/
#define FLD(n)  XtOffsetOf(CWRI_CONFIG,n)
#define WID(n) { NULL, NULL, XtRWidget, sizeof(Widget),FLD(n), XtRString, "*" #n }

typedef struct CWRI_CONFIG {
    int traceLevel;
    char *listenPort;
    Widget vb;
} CWRI_CONFIG;
struct CWRI_CONFIG CWRI;

static XtResource CWRI_CONFIG_RES [] = {

  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
    FLD(traceLevel), XtRImmediate, 0
  },
  { "listenPort", "ListenPort", XtRString, sizeof(String),
    FLD(listenPort), XtRString, "7788"
  },
  WID(vb)

};
#undef FLD
#undef WID



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++***/

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



/* --------------------------------------------------------------------------------------------------------------------

                        IMPLEMENTATION


                        Must Provide
                        - RegisterApplication
                        - InitializeApplication

  -------------------------------------------------------------------------------------------------------------------- */



/* extract path, index and value */
int mvar_assign2( int buf )
{
	TRACE(TRACE_MAIN,"parse %s", m_str(buf) );
	
	int id = -1;
	int typ = VAR_STRING;
	int ls = m_split_list( (const char*) mls(buf,0), "=" );
	if( m_len(ls) != 2 ) goto cleanup;

	int index = 0;
	int var = INT(ls,0);
	int len = m_len(var)-1;
	int ch;
	int mult=1;
	if( len < 1 ) goto cleanup;
	if( CHAR(var,len-1) == ']' ) {
		while( 1 ) {
			len--;
			if( len <= 2 )  goto cleanup;
			ch =  CHAR(var, len-1 );
			if( ch == '[' ) break;
			if( index < 0 ) goto cleanup;
			if( isdigit(ch) ) {
				index += (ch-'0') * mult;
				mult *= 10;
				continue;
			}
			if( ch == '-' ) {
				index=-index;
				continue;
			}
			goto cleanup;
		}
		CHAR(var,len-1)=0;
		m_setlen(var,len);
	}
	id = mvar_parse( var, typ );
	if( id >= 0 ) mvar_put_string(id, m_str(INT(ls,1)), index );
cleanup:
	m_free_list(ls);
	return id;
}


int mvar_set2(char *mvar, ...) {
    va_list ap;
    va_start(ap,mvar);
    int m = vas_printf( 0,0, mvar, ap );
    va_end(ap);
    int v = mvar_assign2( m );
    m_free(m);
    return v;
}

int mvar_assign_c(const char *s)
{
	int buf = s_printf(0,0, "%s", s );
	int x = mvar_assign2( buf );
	m_free(buf);
	return x;
}


static int TASK_CNT = 0;
static int TASK_MAX = 3;
static int TASK_LAST = 0;


static void close_task(int task_num)
{
	struct sensor_reg *r = mls(SENSOR_LIST, task_num);
	TRACE(TRACE_MAIN,"closing subshell: (%d), %s", r->shell, m_str(r->name) );
	XtRemoveInput(r->inputid);
	if( shell_exitcode(r->shell) == 0 )
		r->run = 0;
	else
		r->run = -1;

	shell_close(r->shell);
	TASK_CNT--;



}

static void task_cb(XtPointer p, int *n, XtInputId *id )
{	
	int task_num = (intptr_t) p;
	struct sensor_reg *r = mls(SENSOR_LIST, task_num);
	int shell = r->shell;
	// cp sensor name w/o trailing zero and append a dot  
	int prefix = m_slice(0,0, r->name, 0, -2 );
	m_putc(prefix,'.');
	int prefix_len = m_len(prefix);	       	    
	int stream = 0; // only stdout supported
	int err;
	int buf = m_create(100,1);


	TRACE(TRACE_MAIN,"subshell: %s", m_str(r->name));
	// new line of data available if err==1 
	while( (err=shell_getline( shell, stream, buf)) == 1 ) {
		if(! isalpha(CHAR(buf,0))) continue; 
		m_slice( prefix, prefix_len, buf, 0, -1 ); /* app buf to prefix */
		mvar_assign2(prefix);		
		m_clear(buf);
	}
	m_free(prefix);
	m_free(buf);
	
	/* error handling - could loose data on stderr, but i dont care */
	if( err < 0 ) {		
		if( r->inputid != *id )
			ERR("inputid does not match, task_num:%d, inputid:%ld",  task_num, *id);
		close_task( task_num );	
	}
}

static void sensor_timer(XtPointer data, XtIntervalId *id )
{
	int filename = 0;
	XtAppContext app = data;
	XtAppAddTimeOut(app,SENSOR_TIMER_MS, sensor_timer, APPCTX );
	TRACE(TRACE_MAIN,"");
	struct sensor_reg *r;
	int i;

	/* check for dead sub-processes, killed before task_cb was called */
	m_foreach(SENSOR_LIST,i,r) {
		// if( r->run > 0 && (! shell_running(r->shell))) {
		//	close_task( i );
		// }
	}		
	
	// find not running task and start it
	int len =  m_len(  SENSOR_LIST );
	for( i=0; i<len && TASK_CNT < TASK_MAX;i++ ) {
		if( ++TASK_LAST  >= len ) TASK_LAST = 0;
		r = mls(SENSOR_LIST, TASK_LAST);
		if( r->run ) {
			TRACE(TRACE_MAIN,"Task ignored:%s %d", m_str(r->name), r->run );
			continue;
		}
	    
		filename = s_printf(filename,0, "./%s", m_str(r->name) ); 
		r->shell=shell_create1( filename  );
		if( r->shell < 0 ) {
			r->run=-1; /* disable command */
			TRACE(TRACE_MAIN,"ERROR cmd: %s", m_str(filename) );
			continue;
		}
		TRACE(TRACE_MAIN,"run shell cmd: %s", m_str(filename) );
		r->run=1;
		TASK_CNT++;
		r->inputid = XtAppAddInput(app,
		      shell_fd(r->shell,  CHILD_STDOUT_RD), (XtPointer)  (XtInputReadMask),
			  task_cb, (void*) (intptr_t) TASK_LAST );
		// TRACE(TRACE_MAIN, "task:%d has inputid:%ld", TASK_LAST );
	}
	m_free(filename);
}


static void gui_update(XtPointer data, XtIntervalId *id )
{
	XtAppContext app = data;
	XtAppAddTimeOut(app,UPDATE_TIMER_MS, gui_update, APPCTX );	
	int q=mvar_parse_string("gui_update",0);
        var_call_callbacks( q, 0 );

}


#include "WcCreateP.h"

static void RegisterApplication ( Widget top )
{
    /* -- Register widget classes and constructors */
    // RCP( top, wbatt );
    RCP( top, gauge );
    RCP( top, gauge2 );
    RCP( top, repeatgb );
    RCP( top, wlabelV5 );
    
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_cb );
    RCB( top, test_cb );
}


void gauge_set_value( char *var_name, char *value )
{
    TRACE(1,"%s=%s", var_name, value );
    int qvar = XrmStringToQuark( var_name  );
    int val = atoi(value);
    mv_write( qvar, val );
}

void add_widget( char *s )
{
    TRACE(1,"%s", s );
    XtVaCreateManagedWidget( s, gaugeWidgetClass,CWRI.vb,
			     XtVaTypedArg, /* use resource type conv */
			     XtNqpercent,  /* our resource */
			     XtRString,	   /* value type */
			     s,		   /* value */
			     strlen(s)+1,  /* value size */
			     NULL );

}



/* Exported variables:
   
     NAME                       |  TYPE        | RANGE
     ---------------------------+--------------+-------
      sensor.wlanstat.interface |   string[]   |
      sensor.wlanstat.quality   |   integer[]  |  0-100

*/

/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/
static void InitializeApplication( Widget top )
{
    trace_level = CWRI.traceLevel;

    struct sigaction sa;
    sa.sa_handler = shell_signal_cb;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP; // Automatically restart system calls; don't notify for stopped children
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
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
    trace_child=6;
    XtAppContext app;
    m_init();
    mv_init();
    conststr_init();
    mvar_init();
    srand(time(NULL));
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
    APPCTX = app;

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

    /* start sensors */
    sensor_timer(APPCTX,NULL);
    gui_update(APPCTX,NULL);
    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);

    mv_destroy();
    mvar_destruct();
    conststr_free();
    m_destruct();

    return EXIT_SUCCESS;
}
