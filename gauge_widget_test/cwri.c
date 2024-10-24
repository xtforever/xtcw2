/* most important define,
   check APPNAME.ad !
*/
#define APP_NAME "cwri"

/* speed of gauge update */
#define GAUGE_TIMER_MS 2000
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


#include <WcCreate.h>
#include <Xp.h>
#include <xutil.h>
#include "wcreg2.h"
#include <xtcw/register_wb.h>
#include "nbus.h"
#include "subshell.h"
#include "m_tool.h"

#include "xtcw/Gauge.h"
#include "xtcw/Gauge2.h"

Widget TopLevel;
int trace_main;
#define TRACE_MAIN 4
static XtAppContext APPCTX;

char *fallback_resources[] = {
    APP_NAME ".allowShellResize: False",
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


static int SPROC;
static XtInputId sprocid[2];

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

static void gauge_timer(XtPointer data, XtIntervalId *id )
{
    /* straight from the resource file: g1-percent */
    static int qvar = 0;
    if( !qvar) qvar = XrmStringToQuark("g1-percent");

    /* XT timer stuff */
    XtAppContext app = data;
    XtAppAddTimeOut(app,GAUGE_TIMER_MS, gauge_timer, APPCTX ); 

    /* update gauge widget with: mv_onwrite(...) */
    int p = rand() % 100;
    mv_write(qvar, p );
    TRACE(4,"update values");

    struct sensor_reg *d;
    m_foreach( SENSOR_LIST, p, d ) {
	    if( d->gather ) {
		    d->gather(d);
	    }	    
    }
}


/* extract path, index and value */
int mvar_assign2( int buf )
{
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

static void sproc_parse( int sensorid )
{
    int n=0; // only stdout supported
    TRACE(1, "" );
    int err;
    int buf = m_create(100,1);
    
	    
    // new line of data available if err==1 
    while( (err=shell_getline( SPROC, n, buf)) == 1 ) {
	    int sens_var = s_printf(0,0, "diskstats.%s", m_str(buf) );
	    TRACE(TRACE_MAIN,"%s", m_str(sens_var));
	    mvar_assign2(sens_var);
	    m_free(sens_var);
	    m_clear(buf);
    }

    
    /* error handling - could loose data on stderr, but i dont care */
    if( err < 0 ) {
	    TRACE(TRACE_MAIN,"error reading stdout, closing subshell" );
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
	int sensor_num = (intptr_t) n;
	TRACE(TRACE_MAIN,"");
	sproc_parse(sensor_num);
}

int run_script(Widget top, int args )
{
    TopLevel=top;
    XtAppContext app = XtWidgetToApplicationContext(top);

    if( SPROC ) {
	WARN("script already running");
	return -1;
    }
    
    SPROC = shell_create( args );
    if( SPROC < 0 ) {
	return -1;
    }
    signal(SIGCHLD, shell_signal_cb);
    
    sprocid[0]=
	XtAppAddInput(app,
		      shell_fd(SPROC,  CHILD_STDOUT_RD), (XtPointer)  (XtInputReadMask),
		      sproc_stdout_cb, (void*) (intptr_t) SPROC );
    /* sprocid[1] = */
    /* 	XtAppAddInput(app, */
    /* 		      shell_fd(SPROC, CHILD_STDERR_RD), (XtPointer)  (XtInputReadMask), */
    /* 		      sproc_stderr_cb, NULL ); */
    

    return 0;
}



static int TASK_CNT = 0,
	TASK_MAX = 3,
	TASK_LAST = 0;

struct task_reg {
	int sensor_name; /* must be the first entry */
	int shell;
	int run;
};

int TASK_REG = 0;

static void task_cb(XtPointer p, int *n, XtInputId *id )
{	
	int task_num = (intptr_t) p;
	struct task_reg *r = mls(TASK_REG,task_num);
	int shell = r->shell;
	// cp sensor name w/o trailing zero and append a dot  
	int prefix = m_slice(0,0, r->sensor_name, 0, -2 );
	m_putc(prefix,'.');
	int prefix_len = m_len(prefix);	       	    

	int stream = 0; // only stdout supported
	int err;
	int buf = m_create(100,1);
	// new line of data available if err==1 
	while( (err=shell_getline( shell, stream, buf)) == 1 ) {
		m_slice( prefix, prefix_len, buf, 0, -1 );
		TRACE(TRACE_MAIN,"%s", m_str(prefix) );
		mvar_assign2(prefix);
		m_clear(buf);
	}
	m_free(prefix);
	m_free(buf);
	
	/* error handling - could loose data on stderr, but i dont care */
	if( err < 0 ) {
	    TRACE(TRACE_MAIN,"error reading stdout, closing subshell" );
	    XtRemoveInput(*id);
	    shell_close(shell);
	    r->run=0; r->shell=0;
	    TASK_CNT--;
	}
}

static void sensor_timer(XtPointer data, XtIntervalId *id )
{
    /* XT timer stuff */
    XtAppContext app = data;
    XtAppAddTimeOut(app,SENSOR_TIMER_MS, sensor_timer, APPCTX );
    TRACE(TRACE_MAIN,"");

    // find not running task and start it
    int len =  m_len( TASK_REG );
    for( int i=0; i<len && TASK_CNT < TASK_MAX;i++ ) {
	    if( ++TASK_LAST  >= len ) TASK_LAST = 0;
	    struct task_reg *r = mls(TASK_REG,TASK_LAST);
	    if( r->run ) continue;
	    int filename = s_printf(0,0, "./%s", m_str(r->sensor_name) ); 
	    r->shell=shell_create1( filename  );
	    m_free(filename);
	    if( r->shell < 0 ) continue;
	    r->run=1;
	    TASK_CNT++;
	    XtAppAddInput(app,
		      shell_fd(r->shell,  CHILD_STDOUT_RD), (XtPointer)  (XtInputReadMask),
			  task_cb, (void*) (intptr_t) TASK_LAST );
    }
}

    
static void RegisterApplication ( Widget top )
{
    /* -- Register widget classes and constructors */
    // RCP( top, wbatt );
    RCP( top, gauge );
    RCP( top, gauge2 );
    
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

void handle_msg(char *msg)
{
    int mm = m_split(0,msg,32,1);
    if( m_len(mm) != 2 ) goto leave;

    if( strcmp(STR(mm,0), "add" ) == 0 ) {
	add_widget(STR(mm,1));
	goto leave;
    }
    
    gauge_set_value( STR(mm,0), STR(mm,1) );

 leave:
    m_free_strings(mm,0);
}

void gauge_server( XtPointer p, int *n, XtInputId *id );
void gauge_server_new_connection(void);
void gauge_server_parse_messge(XtInputId *id, int client_id);

static int NBUS;

void gauge_server_new_connection(void)
{
    int client_id = nbus_server_message(NBUS);
    if( client_id < 0 ) return;
    int fd = nbus_client_fd(NBUS, client_id);
    XtAppAddInput(APPCTX,fd,(XtPointer) XtInputReadMask,
		  gauge_server, (void*)(intptr_t)client_id);
}

void gauge_server_parse_messge(XtInputId *id, int client_id)
{
    int req = nbus_client_message(NBUS,client_id);
    if( req == CLIENT_EXIT ) {
	TRACE(1,"remove client:%d", client_id );
	XtRemoveInput(*id);
	return;
    }

    if( req == CLIENT_REQ ) {
	char *msg = nbus_client_get_msg(NBUS,client_id);
	TRACE(1,"client sends: %s", msg );
	handle_msg(msg);
    }    
}


void gauge_server( XtPointer p, int *n, XtInputId *id )
{
    int client_id = (intptr_t) p;

    if( client_id < 0 ) {
	gauge_server_new_connection();
	return;
    }

    gauge_server_parse_messge(id, client_id);	
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

    /* init sensors */
    /* well known identifier to connect data-source and gauge2 */
    int p;

    struct task_reg *task;
    TASK_REG = m_create(10, sizeof(*task));
    struct sensor_reg *d;
    m_foreach( SENSOR_LIST, p, d ) {
	    TRACE(TRACE_MAIN, "check sensor %s", CHARP(d->name) );
	    lookup_int(TASK_REG, d->name );	    
    }


    gauge_timer(APPCTX,NULL);
    sensor_timer(APPCTX,NULL);

    // NBUS = nbus_create( CWRI.listenPort );
    // int fd = nbus_get_socket(NBUS);
    // if( fd < 0 ) ERR("could not open port 7788");
    // XtAppAddInput(APPCTX,fd, (XtPointer) XtInputReadMask,
    // gauge_server, (void*)(intptr_t)-1);

    int args = m_create(10,sizeof(char*));
    char *prog = "./diskstats";
    m_put( args, &prog );
    run_script( top, args );
    m_free(args);
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
    mv_init();
    conststr_init();
    mvar_init();

    srand(time(NULL));
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

    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);

    mv_destroy();
    m_destruct();

    return EXIT_SUCCESS;
}
