/* most important define,
   check APPNAME.ad !
*/
#define APP_NAME "cwri"

/*
 */

#include "mls.h"
#include "svar2.h"
#include "opts.h"

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
#include <micro_vars.h>
#include "wcreg2.h"
#include <xtcw/register_wb.h>
#include "xtcw/WeditMV.h"
#include "xtcw/Woption.h"
#include "xtcw/Wswitch.h"
#include "xtcw/Radio2.h"
#include "xtcw/Wls.h"
#include "xtcw/Woptc.h"
#include "xtcw/WlistScroll.h"


Widget TopLevel;
int trace_main;
#define TRACE_MAIN 2
int EditBuffer;
#define EDITBUFFERNAME "myVariable"

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
    String option_group;
    String svar_result;
    String svar_all_results;
    int key_svar_all_results;

    Widget inpv; /* input-verify example */
    int key_result_ed1; /* input-verify svar w/ callback */
    
} CWRI_CONFIG;

#define FLD(n)  XtOffsetOf(CWRI_CONFIG,n)
static XtResource CWRI_CONFIG_RES [] = {

  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
   FLD(traceLevel), XtRImmediate, 0
  },
  { NULL, NULL, XtRWidget, sizeof(Widget),
   FLD(inpv), XtRString, "*inpv"
  },

  { "option_group", "Option_group", XtRString, sizeof(XtRString),
   FLD(option_group), XtRString, ""
  },
  { "svar_result", "Svar_result", XtRString, sizeof(XtRString),
   FLD(svar_result), XtRString, ""
  },
  { "svar_all_results", "Svar_all_results", XtRString, sizeof(XtRString),
   FLD(svar_all_results), XtRString, ""
  },

  

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


static void dump_mstring_array( int value )
{
    int p,*d;
    m_foreach(value, p, d ) {
	printf("pos: %d, value: %s\n", p, m_str(*d) );
    }
}

static void print_svar(int k)
{
  svar_t *v = svar(k);
  printf( "svar-name:%s, svar-type:%d\n",
	  svar_name(k), *svar_type(k) );
  if( v->type == SVAR_MSTRING_ARRAY ) {
      dump_mstring_array( v->value );
  }
}

static void dump_svar_array(int key)
{
    svar_t *v = svar(key);
    int p,*d;
    m_foreach(v->value,p,d) {
	printf("---- result: %d -----\n", p );
	print_svar(*d);
    }

    m_foreach(v->value,p,d) {
	const char *value = svar_get_str(*d, 0);
	const char *name = svar_name(*d);
	printf("name=value: %s=%s\n", name, value );
    }
    puts("");
}

static void set_label(Widget w, char *s)
{
    XtVaSetValues(w, "label", s, NULL );
}
static void set_state(Widget w, int e)
{
    TRACE(1,"%d", e );
    XtVaSetValues(w, "state", e == 0, NULL );
}


/** @brief fail safe - get string entry from marray of mstring
 *  easy string extract function for marray-string-lists.
 *
 *  @param   v   handle of the marray to marray structure 
 *  @param   p   if <0 return last element, else return v[p] 
 *  @return      the string buffer from v[p] or an empty string 
 */
static const char* ma_get_str(int v, int p)
{
    if( v <=0 ) return "";
    if( m_len(v) == 0 ) return "";
    if( p < 0 ) { p = m_len(v)-1; }
    else if( m_len(v) <= p ) return "";
    int ms = INT(v, p );
    if( ms <= 0 ) return "";
    if( m_len(ms) == 0 ) return "";
    return m_str(ms);
}




/* register a write callback for result.ed1 
   check values array: [ name,value ]
   check for number
   update CWRI.inpv
*/
static void ed1_changed(void *ctx, int key)
{
    TRACE(1, "result.ed1 %d", svar_lookup_str("result.ed1", -1 ));
    int e, intnum;
    const char *str = svar_get_str( key, 0 );
    e = sscanf( str , "%d", &intnum );
    set_label(CWRI.inpv, e ? "Num" : "NaN" );
    set_state(CWRI.inpv, e );


}


static void option_changed(void *ctx, int key )
{
    (void)ctx;
    TRACE(1,"svar onwrite callback: %d, val: %d, type: %d", key,
	  *svar_value(key), *svar_type(key) );

    
    dump_mstring_array( *svar_value(key) );
}

void result_dump_cb( Widget w, void *u, void *c )
{
    int key = CWRI.key_svar_all_results;
    svar_t *v = svar(key);
    if( (v->type & SVAR_MASK) == SVAR_SVAR_ARRAY ) {
	printf("my results:\n");
	dump_svar_array(key);
	printf("---------------------\n");
    }


    
}


void option_cb( Widget w, void *u, void *c )
{
  printf("cb from option\n");

  if(! XtIsSubclass(w, woptionWidgetClass ) ) return;

  char *s,*val;
  XtVaGetValues(w, "label", &s, "value", &val, NULL );
  printf("Label: %s\nValue: %s\n", s, val );

  Widget w1 = get_active_widget(CWRI.option_group);
  if( w1 != w )      TRACE(1,"Achtung Active widget not current widget" );

  int k = svar_lookup_str( CWRI.svar_result, -1 );
  svar_t *v = svar(k);
  TRACE(1, "svar-name:%s, svar-type:%d", svar_name(k), *svar_type(k) );
  if( *svar_type(k) == SVAR_MSTRING_ARRAY ) {
      int p,*d;
      m_foreach( *svar_value(k), p, d ) {
	  TRACE(1,"svar-pos: %d, svar-value: %s", p, m_str(*d) );
      }
  }
  


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
    RCP( top, weditMV );
    RCP( top, woption );
    RCP( top, wswitch );
    RCP( top, radio2 );
    RCP( top, wls );
    RCP( top, woptc );
    RCP( top, wlistScroll );
    
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_cb );
    RCB( top, test_cb );
    RCB( top, option_cb );
    RCB( top, result_dump_cb );
}




void update_edit_buffer_cb( void *ctx )
{
    int str = *mv_var( EditBuffer );
    if( m_len(str) > 0 )
      fwrite( m_buf(str), m_len(str)-1,1, stdout );
    fputc( '\n', stdout );
    
    
}




/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/
static void InitializeApplication( Widget top )
{
    trace_level = CWRI.traceLevel;
    EditBuffer = XrmStringToQuark( EDITBUFFERNAME  );
    TRACE(1, "register %d", EditBuffer );
    mv_onwrite(EditBuffer, update_edit_buffer_cb, 0, 0);
    int k = svar_lookup_str( CWRI.svar_result, -1 );
    svar_onwrite( k, option_changed, 0, 0 );

    
    CWRI.key_svar_all_results = svar_lookup_str( CWRI.svar_all_results, -1 );

    CWRI.key_result_ed1 = svar_lookup_str( "result.ed1", -1 );
    svar_onwrite(CWRI.key_result_ed1, ed1_changed, 0, 0 );
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
    svar_init();
    
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

    svar_destruct();
    m_destruct();

    return EXIT_SUCCESS;
}
