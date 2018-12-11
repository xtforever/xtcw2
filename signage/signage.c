/* most important define,
   check APPNAME.ad !
*/
#define APP_NAME "signage"

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

#include "socket_log.h"
#include "communication.h"

#include "Wlabel.h"
#include "Gridbox.h"


Widget TopLevel;
int trace_main;

#define TRACE_MAIN 2
#define TRACE_SLOG TRACE_MAIN

static void incomming_msg(int fd, char *buf, int size);


char *fallback_resources[] = {
    APP_NAME ".allowShellResize: False",
    "*WclResFiles:" APP_NAME ".ad\n",
    "*traceLevel: 2",
    "*socket: 18000",
    NULL };

/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
    { "-Socket",	"*socket",	XrmoptionSepArg, NULL },
    { "-TraceLevel",	"*traceLevel",	XrmoptionSepArg, NULL },
    WCL_XRM_OPTIONS
};

typedef struct SIGNAGE_CONFIG {
    int traceLevel;
    int socket;
    Widget widget_grid;
} SIGNAGE_CONFIG;

#define FLD(n)  XtOffsetOf(SIGNAGE_CONFIG,n)
static XtResource SIGNAGE_CONFIG_RES [] = {

  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
   FLD(traceLevel), XtRImmediate, 0
  },

  { "socket", "Socket", XtRInt, sizeof(int),
   FLD(socket), XtRImmediate, 0
  },
  
  { NULL, NULL, XtRWidget, sizeof(Widget),
    FLD(widget_grid), XtRString, "*gb"
  },
  
};
#undef FLD

struct SIGNAGE_CONFIG SIGNAGE;

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

static void RegisterApplication ( Widget top )
{
    /* -- Register widget classes and constructors */
    // RCP( top, wbatt );
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_cb );
    RCB( top, test_cb );
}

/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/


Widget LABELS[100];
int MAX_LABELS = 0;

static void InitializeApplication( Widget top )
{
    trace_level = SIGNAGE.traceLevel;
    slog_init( top, SIGNAGE.socket );
    slog_cb = incomming_msg;

    if( SIGNAGE.widget_grid == 0 ) ERR("gridwidget not defined in signage.ad");

    int max = sizeof(LABELS)/sizeof(LABELS[0]);
    Widget mgr = SIGNAGE.widget_grid;
    char *label;

    Widget w;
    Arg wargs[10];
    int n;

    for(int y=0;y<10;y++) {
	for(int x=0;x<10;x++) {
	    asprintf(&label,"%dx%d", x,y ); 
	    n=0;
	    XtSetArg(wargs[n], XtNgridx, x); n++;
	    XtSetArg(wargs[n], XtNgridy, y); n++;
	    XtSetArg(wargs[n], XtNlabel, label); n++;
	    // w = XtCreateManagedWidget(label,wlabelWidgetClass, mgr, wargs, n );
	    w = XtCreateManagedWidget(label,wtextWidgetClass, mgr, wargs, n );
	    if( MAX_LABELS < max ) LABELS[MAX_LABELS++] = w;
	}
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


static void start_listening(void)
{
    TRACE(2,"listen on socket: %d", SIGNAGE.socket );
}

static void socket_connect_cb(void)
{
}

static void incomming_msg(int fd, char *buf, int size)
{
  buf[size-1]=0;
  TRACE(2,"msg: %s",buf);

  int num = 0;
  int state=0;
  int i=0;
  while( buf[i++] != 32 ) if( i>=size) return;
  sscanf(buf, "%u:%u", &num,&state );
  if( num > MAX_LABELS ) return;

  char *label = buf+i;
  if( state <0 || state >2 ) state=0;
  
  XtVaSetValues( LABELS[num], "text", label,
		 XtNstate, state, NULL ); 

  slog_close(fd);
    
}

static void update_widget(void)
{
}


extern int trace_slog;


/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    trace_main = TRACE_MAIN;
    trace_slog = TRACE_SLOG;
    
    XtAppContext app;
    m_init();
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
    XtGetApplicationResources(	appShell, (XtPointer)&SIGNAGE,
				SIGNAGE_CONFIG_RES,
				XtNumber(SIGNAGE_CONFIG_RES),
				(ArgList)0, 0 );

    InitializeApplication(appShell);

    /*  --  Realize the widget tree and enter
            the main application loop  */
    XtRealizeWidget ( appShell );

    grab_window_quit( appShell );
    start_listening();

    XtAppMainLoop ( app ); /* use XtAppSetExitFlag */
    XtDestroyWidget(appShell);
    m_destruct();

    return EXIT_SUCCESS;
}
