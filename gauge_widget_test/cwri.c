/* most important define,
   check APPNAME.ad !
*/
#define APP_NAME "cwri"

/* speed of gauge update */
#define GAUGE_TIMER_MS 2000
/*
 */

#include "mls.h"
#include "micro_vars.h"

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

#include "xtcw/Gauge.h"

Widget TopLevel;
int trace_main;
#define TRACE_MAIN 2
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
}




static void RegisterApplication ( Widget top )
{
    /* -- Register widget classes and constructors */
    // RCP( top, wbatt );
    RCP( top, gauge );
    
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


/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/
static void InitializeApplication( Widget top )
{
    trace_level = CWRI.traceLevel;

    gauge_timer(APPCTX,NULL);

    NBUS = nbus_create( CWRI.listenPort );
    int fd = nbus_get_socket(NBUS);
    if( fd < 0 ) ERR("could not open port 7788");
    XtAppAddInput(APPCTX,fd, (XtPointer) XtInputReadMask,
		  gauge_server, (void*)(intptr_t)-1);
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
