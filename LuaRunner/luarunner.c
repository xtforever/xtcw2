/* most important define,
   check APPNAME.ad !
*/
#define APP_NAME "luarunner"

#include "mls.h"
#include "svar2.h"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <sys/time.h>
#include <sys/resource.h>


#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "luaxt.h"
extern int luaopen_luaxt(lua_State* L);
#include "lua-var5.h"


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
#include "widreg.h"

#include "socket_log.h"
#include "communication.h"
#include "ini_read2.h"

#include "Wlabel.h"
#include "Wlist4.h"
#include "Wlist-common.h"
#include "Gridbox.h"

#include "m_tool.h"
#include "var5.h"
#include "font.h"



// static inline const char *m_str(const int m) { return m_buf(m); }

static lua_State *LUA_CTX;
XtAppContext LUAXT_APP;
Widget TopLevel;
int trace_main;
extern int trace_slog;

#define TRACE_MAIN 2
#define TRACE_SLOG TRACE_MAIN



extern struct RC_DB RC;


static int GridY, GridX;

int PAGES;
int LABELS;
int MAX_LABELS = 0;


char *fallback_resources[] = {
    APP_NAME ".allowShellResize: False",
    "*WclResFiles:" APP_NAME ".ad\n",
    "*traceLevel: 2",
    "*socket: 18000",
    "*inifile: ap.ini",
    NULL };

/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
    { "-Socket",	"*socket",	XrmoptionSepArg, NULL },
    { "-TraceLevel",	"*traceLevel",	XrmoptionSepArg, NULL },
    { "-Inifile",	"*inifile",	XrmoptionSepArg, NULL },
    
    WCL_XRM_OPTIONS
};

typedef struct SIGNAGE_CONFIG {
    int traceLevel;
    int socket;
    char *inifile;
    Widget widget_grid;
} SIGNAGE_CONFIG ; 

#define FLD(n)  XtOffsetOf(SIGNAGE_CONFIG,n)
static XtResource SIGNAGE_CONFIG_RES [] = {

  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
   FLD(traceLevel), XtRImmediate, 0
  },

  { "socket", "Socket", XtRInt, sizeof(int),
   FLD(socket), XtRImmediate, 0
  },

  { "inifile", "Inifile", XtRString, sizeof(String),
    FLD(inifile), XtRString, NULL
  },
  
  /*
    { NULL, NULL, XtRWidget, sizeof(Widget),
    FLD(widget_grid), XtRString, "*gb"
  },

  { NULL, NULL, XtRWidget, sizeof(Widget),
    FLD(widget_grid), XtRString, "*gb"
  },
  */
  
};
#undef FLD

struct SIGNAGE_CONFIG SIGNAGE = { 0 };



/** set the keyboard focus to a named widget and call the focus_in function
    of that widget
*/
void SetFocus( Widget w, void *u, void *c )
{
    char buffer[MAX_XRMSTRING];
    Widget focus_widget;
	
    TRACE(8, "Widget called the setfocus callback");
    if(! u ) {
	WARN("Callback SetFocus invoked without argument i.e. without a widget name to set the focus to");
	return;
    }

    /* need my own implementation here i.e. snprintf */
    WcCleanName( u, buffer );
    focus_widget=WcFullNameToWidget( w, buffer );

    /* the warning message should be printed by WcFullNameToWidget */
    if( ! focus_widget ) {
	WARN("the widget %s was not found in this widget tree");
	return;
    }

    if( w != focus_widget ) {
	XtCallActionProc( w, "focus_out", NULL, NULL, 0 );
	XtCallActionProc( focus_widget, "focus_in", NULL, NULL, 0 );
    }
    
    XtCallActionProc( focus_widget, "SetKeyboardFocus", NULL, NULL, 0 );
}

void CallAction( Widget w, void *u, void *c ) 
{
    char buffer[MAX_XRMSTRING];
    Widget widget;
	
    TRACE(8, "Widget called the CallAction callback");
    if(! u ) {
	WARN("Callback CallAction invoked without argument i.e. without a widget name to set the focus to");
	return;
    }

    /* need my own implementation here i.e. snprintf */
    char *args = WcCleanName( u, buffer );
    TRACE(8, "Widget Name: %s", buffer );
    widget=WcFullNameToWidget( w, buffer );    
    if(! widget ) return;
    WcCleanName( args, buffer );
    TRACE(8, "Action Name: %s", buffer );
    XtCallActionProc( widget, buffer, NULL, NULL, 0 );
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

void vl_dump(int opts); // ini_read2.c

void SetKeyboardFocus(Widget w)
{
    Widget shell, parent;

    shell = parent = w;
    while (parent) {
        if (XtIsShell(shell = parent))
            break;
        parent = XtParent(parent);
    }
    XtSetKeyboardFocus(shell, w);
}


int CUR_PAGE = 0;


static void
LUA(Widget w, XtPointer client_data, XtPointer call_data)
{    
    if( ! client_data ) {
	WARN("Called without argument");
	return;
    }

    TRACE(1,"hallo");
    
    char *s = call_data ? call_data : "";
    lua_pushstring( LUA_CTX, s );
    lua_setglobal(LUA_CTX, "class_data");
    
    /* can we expand vars inside the lua code? e.g. player=$radio.selection1 */
    /* we need to store additional data besides the lua code e.g. class_data */
    luaxt_pushcallback( client_data );
}


/* --------------------------------------------------------------------------------------------------------------------

                        IMPLEMENTATION


                        Must Provide
                        - RegisterApplication
                        - InitializeApplication

  -------------------------------------------------------------------------------------------------------------------- */


/** all widgets callbacks and actions must be registered at this point
 */
static void RegisterApplication ( Widget top )
{
    
    /* -- Register widget classes and constructors */


    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_cb );
    RCB( top, test_cb );
    RCB( top, SetFocus );
    RCB( top, CallAction );
    RCB( top, LUA );
}

/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/
static void InitializeApplication( Widget top )
{

    trace_level = SIGNAGE.traceLevel;

}



static void DestroyApplication(void)
{
    // rc_free();
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

/*
static int m_split_list( const char *s, const char *delm )
{
    int ls =  m_create(2,sizeof(int));
    int w  =  0;
    int p  =  0;
    int ch;
    
    do {
	ch = s[p];
	if(!w) {
	    w =  m_create( 10,1 );
	    m_put( ls, &w );
	}

	if( ch == *delm || ch == 0 ) {
	    m_putc(w,0); w=0;    
	} else {
	    m_putc(w,s[p]);
	}
	p++;
    } while( ch );
    
    return ls;
}
*/

/*
static void asgn(char *s)
{
    int ls =  m_split_list(s, "=" );
    if( m_len(ls) != 2 ) goto leave;
    int v = mvar_parse( INT(ls,0), VAR_STRING );
    mvar_put_string(v, m_str(INT(ls,1)), 0);

 leave:
    m_free_list(ls);
}
*/



void proc_exit()
{

		int wstat;
		pid_t	pid;

		while (TRUE) {
			pid = wait3 (&wstat, WNOHANG, (struct rusage *)NULL );
			if (pid == 0)
				return;
			else if (pid == -1)
				return;
			else
			    printf ("Return code: %d\n",WEXITSTATUS( wstat ));
		}
}

/******************************************************************************
*   MAIN function
******************************************************************************/
int main ( int argc, char **argv )
{
    lua_State *L;
    signal (SIGCHLD, proc_exit);
    trace_main = TRACE_MAIN;
    trace_slog = TRACE_SLOG;
    
    XtAppContext app;
    m_init(); trace_level=1;
    mvar_init();
    svar_create();
    
    /*
    asgn("task1.t1=hello");
    asgn("task1.t2=world");
    const char *s = mvar_str_string( "task1", "form: $t1 und $t2");
    fprintf(stderr,"%s\n", s );
    */
    


    
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
    LUAXT_APP = app;

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
    widreg(app);
    
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
    

    luaxt_init();
    LUA_CTX = L = luaL_newstate();
    luaL_openlibs(L); /* Load Lua libraries */
    luaopen_luaxt(L); /* load luaxt table i.e. functions generated by swig */
    luaopen_var5( L );

    char *fn = "ex.lua"; 
    int result,status = luaL_loadfile(L, fn );
    if (status) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        exit(1);
    }

    /* Ask Lua to run our little script */
    result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result) {
        fprintf(stderr, "Failed to run script: %s\n", lua_tostring(L, -1));
        exit(1);
    }
    
    // XtAppMainLoop ( app ); /* use XtAppSetExitFlag */

    XtDestroyWidget(appShell);
    DestroyApplication();
    font_cache_flush();
    svar_destruct();
    mvar_destruct();
    luaxt_destroy();
    m_destruct();

    
    return EXIT_SUCCESS;
}
