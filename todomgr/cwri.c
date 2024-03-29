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


#include "xtcw/Gauge.h"
#include "xtcw/Wlist5.h"


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
  { "-TraceLeve+l",	"*traceLevel",	XrmoptionSepArg, NULL },
  WCL_XRM_OPTIONS
};


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++***/
/* define resource struct */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++***/
#define FLD(n)  XtOffsetOf(CWRI_CONFIG,n)
#define WID(n) { NULL, NULL, XtRWidget, sizeof(Widget),FLD(n), XtRString, "*" #n }

typedef struct CWRI_CONFIG {
    int traceLevel;
    Widget ed,todo,status;    
} CWRI_CONFIG;
struct CWRI_CONFIG CWRI;

static XtResource CWRI_CONFIG_RES [] = {

  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
    FLD(traceLevel), XtRImmediate, 0 },
  WID(todo),
  WID(ed),
  WID(status)
};
#undef FLD
#undef WID



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++***/


static int TODO_LIST = 0;

void status(char *s)
{
    puts(s);
    XtVaSetValues(CWRI.status, "label", s, NULL );
}

static void normalize_line(int buf)
{
    int ch = CHAR(buf,0);
    if(!( ch=='.' || ch=='+' || ch=='-' )) {
	m_ins(buf,0,1);
	CHAR(buf,0)='+';
    }
}

void load_todo(void)
{
    m_free_strings(TODO_LIST,1);
    FILE *fp=fopen("todo.txt","r");
    if( !fp ) {
	status("error open file todo.txt"); 
	return;
    }
    int buf = m_create(1000,1);    
    while( m_fscan(buf,'\n',fp) == '\n' ) {
	normalize_line(buf);
	char *s = strdup(m_buf(buf));
	m_clear(buf);
	m_put(TODO_LIST,&s);
    }
    m_free(buf);
    wlist5_clear_selection(CWRI.todo);
    status("succ. loading todo.txt"); 
}




void save_todo(void)
{
    FILE *fp=fopen("todo.txt.1","w");
    if( !fp ) {
	status("could not open file todo.txt.1 for writing");
	return;
    }
    char **d; int p;
    m_foreach( TODO_LIST, p, d )
	fprintf(fp,"%s\n", *d);
    if( fclose(fp) ) {
	status("error writing to file todo.txt.1");
    } else {
	status("succesful saved todo.txt.1");
    }
}


/* add a copy of the string s to the todo list
   if the first char of s is not in (.+-) prepend the
   copied string with '+'
*/
void append_todo(char *s)
{
		char *str;
		int ch=s[0];
		int offs = 1;		
		if(!( ch=='.' || ch=='+' || ch=='-' )) {
				offs=0;
				ch='+';
		}
	    asprintf(&str, "%c%s", ch, s+offs );	   
		m_ins(TODO_LIST,0,1);
		m_write(TODO_LIST,0, &str,1 );
		wlist5_update_all(CWRI.todo);
}


void save_cb( Widget w, void *u, void *c )
{
    save_todo();
}


void edit_cb( Widget w, void *u, void *c )
{
    char *s;
    XtVaGetValues(w, "label", &s, NULL );
    if( is_empty(s) ) return;
    printf("Label: '%s'\n", s );
    append_todo(s);
}

void todo_cb( Widget w, void *u, void *c )
{
    int lineno = (intptr_t)c;
    int max = m_len(TODO_LIST);
    if( lineno >= max || max == 0 ) return;
    char *s = STR(TODO_LIST,lineno);
    m_del(TODO_LIST, lineno);
    XtVaSetValues(CWRI.ed, "label", s, NULL );
    printf("Line: %d %s\n", lineno,s );
    free(s);
    wlist5_update_all(CWRI.todo);

    XtSetKeyboardFocus( TopLevel, CWRI.ed );

}

void ActionCall(Widget w, void *u, void *c )
{
    char *args = strdup(u);
    char *buf = args;
    printf("ActionCall %s\n", args );
    char *split = strchr(args, ' ');
    if( !split ) {
	WARN("widget name undefined: %s", args );
	return;
    }
    *split++ = 0;
    char *widget_name = args;
    Widget target = WcFullNameToWidget( w, widget_name );
    if( !target ) {
	WARN("could not find widget: %s", widget_name );
	return;
    }

    char *action;
    char *action_args;
    args=split;
    split = strchr(args, ' ');
    if( !split ) {
	action_args = "";
	WARN("action without args" );
    } else {
	*split++ = 0; action_args=split;
    }
    action = args;
    WcInvokeNamedAction( action, target, action_args );    
    free(buf);
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
    RCP( top, wlist5 );
    
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_cb );
    RCB( top, test_cb );
    RCB( top, edit_cb );
    RCB( top, todo_cb );
    RCB( top, save_cb );
    RCB( top, ActionCall );
}

/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/
static void InitializeApplication( Widget top )
{
    trace_level = CWRI.traceLevel;

    TODO_LIST = m_create(100,sizeof(char*));
    load_todo();
    XtVaSetValues( CWRI.todo, "tableStrs", TODO_LIST, NULL );


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
