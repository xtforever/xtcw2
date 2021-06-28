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
#include "mls.h"
#include "m_mysql.h"
#include "svar2.h"
#include "micro_vars.h"
#include "Wlist-common.h"

#include "xtcw/Gauge.h"
#include "xtcw/WeditMV.h"

Widget TopLevel;
int trace_main;
#define TRACE_MAIN 2



int simple_query(int res, char *query)
{


  MYSQL *sql = m_mysql_connect( NULL, "custsrv", NULL, "linux" );
  char *real_query = strdup(str_expand( res, query ));
  TRACE(1, "query: %s", real_query );
  int err = m_mysql_query(sql,real_query,res);
  m_mysql_close(sql);
  return err;
}


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
    Widget widget_term;
} CWRI_CONFIG;

#define FLD(n)  XtOffsetOf(CWRI_CONFIG,n)
static XtResource CWRI_CONFIG_RES [] = {

  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
   FLD(traceLevel), XtRImmediate, 0
  },
  { NULL, NULL, XtRWidget, sizeof(Widget),
    FLD(widget_term), XtRString, "*term"
  },
};
#undef FLD

struct CWRI_CONFIG CWRI;

int SQL_RES = 0;

void terminal_output_cb(Widget w, void *u, void *c)
{
    // given:   ent->pos
    // compute: ent->max, ent->data
    wlist_entry_t *ent = c;
    char line_buf[1024];

    if( SQL_RES == 0 ) {
	ent->max=0;
	TRACE(2, "no data" );
	return;
    }

    int fld_cnt   = (field_count( SQL_RES ) +1);
    ent->max = fld_cnt * row_count(SQL_RES);
    if( ent->max == 0 ) return;

    int fld_num   = ent->pos % fld_cnt;
    int row       = ent->pos / fld_cnt;
    if( fld_num == fld_cnt-1 ) {
	line_buf[0]=0;
    } else {
	snprintf(line_buf,sizeof(line_buf),"%s\t%s", field_name(SQL_RES,fld_num),
		 get_entry(SQL_RES,fld_num, row) );
    }
    
    ent->data = line_buf;
}



/* sql connection test */
void test_sql_connection(const char *custname)
{

    if( SQL_RES ) v_free(SQL_RES);
    SQL_RES = v_init();
        
    // char *custname = "gpe";
    char *query = "select custid, c.dbservid, a.srvid,nfsserver,nfsbasedir,adminnode,dbslave,backupsrv,backuppath from custdb c inner join dbserv_alias a on c.dbservid=a.dbservid inner join serverinfo i on a.srvid=i.srvid where custid like '$custname%'";
    
    v_set(SQL_RES, "custname", custname, -1 );
    
    // create and execute query 
    int err = simple_query(SQL_RES,query);    
    TRACE(2,"row count: %d", row_count(SQL_RES) );
}


void edit_customername(Widget w, void *u, void *c)
{
    int k = svar_lookup_str( "customername", -1 );
    if( k < 1 ) return;
    const char *s = svar_get_str(k,0);
    TRACE(2,"%s", s);
    test_sql_connection(s);
    wlist4_update_all(CWRI.widget_term);
    
}


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
    RCP( top, gauge );
    RCP( top, weditMV );
    
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_cb );
    RCB( top, test_cb );
    RCB( top, terminal_output_cb);
    RCB( top, edit_customername);
    
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
    svar_init();
    mv_init();
    
    TRACE(2,"test");



    XtSetLanguageProc (NULL, NULL, NULL);
    XawInitializeWidgetSet();


    // test_sql_connection("gpe");

    
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

    v_free(SQL_RES);
    svar_destruct();
    m_destruct();

    return EXIT_SUCCESS;
}
