/* most important define,
   check APPNAME.ad !
xterm -e './100lines.sh; bash' &
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
#include "ini_read2.h"
#include "micro_vars.h"

#include "Wlabel.h"
#include "Wlist4.h"
#include "Wlist-common.h"
#include "Gridbox.h"
#include "WeditMV.h"
static inline const char *m_str(const int m) { return m_buf(m); }

Widget TopLevel;
int trace_main;

#define TRACE_MAIN 2
#define TRACE_SLOG TRACE_MAIN

static void incomming_msg(int fd, char *buf, int size);

extern struct RC_DB RC;



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
  
  { NULL, NULL, XtRWidget, sizeof(Widget),
    FLD(widget_grid), XtRString, "*gb"
  },

  { NULL, NULL, XtRWidget, sizeof(Widget),
    FLD(widget_grid), XtRString, "*gb"
  },

  
};
#undef FLD

struct SIGNAGE_CONFIG SIGNAGE = { 0 };

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

static int get_section( char *section )
{
    int p =  m_lookup_str( RC.db, section, 1 );
    if( p < 0 ) ERR("section [%s] not defined in %s", section, SIGNAGE.inifile );
    int options = ((rc_ent_t*) mls( RC.db, p ))->chap; 
    return options;
}

static int get_all_tasks(void)
{
    return get_section("task");
}

static char* get_task_name(int p)
{
    int o = get_all_tasks();
    return STR( INT(o,p), 0);
}

typedef struct chapter_info_st {
    int vset;
    int opt;		     // all options from ini file 
    int callback;	     // helper struct for edit widget callback
    char *name;		     // chapter name
    str_exp_t se_script;    /* buffer for script stringexpand */
    Widget w_script;	     /* display script name using "label"  */
} chapter_info_t;
int CHAPINFO = -1;

struct chap_callback_st {
    int var;			// vset var
    int mvar;			// microvar
    int chap;			// number of the corresponding chapter
    Widget widget;
};

chapter_info_t *get_chapter_info( int p )
{
    if( CHAPINFO < 0 ) CHAPINFO = m_create(10, sizeof( chapter_info_t) );

    if( p >= m_len( CHAPINFO) ) {
	m_setlen(CHAPINFO, p+1 );
    }
    
    return mls(CHAPINFO,p);
}


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

Widget get_page_widget(int page)
{
    return *(Widget *) mls(PAGES, page);
}


void switch_page(int page)
{
    if( page == CUR_PAGE ) return;
    Widget w0 = get_page_widget(CUR_PAGE);
    Widget w1 = get_page_widget(page);    
    XtUnmanageChild( w0 );
    XtManageChild(w1);
    CUR_PAGE = page;
}


void terminal_output_cb(Widget w, void *u, void *c)
{
    wlist_entry_t *ent = c;
    TRACE(2, "wlist cb");
    ent->max=10;
    char *buf = NULL;
    asprintf( &buf, "hello %d", ent->pos );
    ent->data = buf;
}

void chapter0_leave( Widget w, void *u, void *c )
{
    switch_page(0);
}

static Widget create_script_widget( int task, int var, Widget mgr )
{
    char *type = STR( var, 1 );


    // create a label "name" 
    return XtVaCreateManagedWidget(type,wbuttonWidgetClass, mgr,
				   XtNweightx, 1,
				   XtNweighty, 0,
				   XtNlabel, type,
				   NULL);
    
}

// copy buffer to variable and terminate string with zero 
char* buffer_to_var( int buf, int len, int var )
{
    int max = m_len(buf);
    if( len > max || len < 1 ) len=max;
    char **d;
    
    if( m_len(var) > 1 ) {
	d = mls( var, 1 );
	if( *d ) free(*d);
    } else {
	d=m_add(var);
    }
    
    *d = malloc( len +1 );
    m_read( buf, 0, (void**)d, len );
    (*d)[ len ] = 0;
    return *d;
}


void input_cb( void *u )
{
    TRACE(2, "" );
    struct chap_callback_st *cb = u;
    int buf = *mv_var( cb->mvar );
    buffer_to_var( buf, m_len(buf)-1, cb->var );
    TRACE(2, "set var: %s=%s", v_kget(cb->var,0), v_kget(cb->var,1) );
    
    struct chapter_info_st *chap = get_chapter_info( cb->chap );
    se_expand( & chap->se_script, chap->vset, 0 );
    char *s = m_buf( chap->se_script.buf );
    TRACE(2, "script: %s", s );

    XtVaSetValues( chap->w_script, XtNlabel,s,NULL );
    
}

void input_widget_enter_cb(Widget w, void *u, void *c)
{
    int task = (intptr_t) u;
    TRACE(1,"Current task: %d", task );
    struct chapter_info_st *chap = get_chapter_info(task);
    struct chap_callback_st *cb;
    int i=-1;
    while( m_next(chap->callback, &i, &cb) ) {
	if( cb->widget == w ) break;
    }
    i= (i+1) % m_len(chap->callback);
    cb = mls(chap->callback, i);
    SetKeyboardFocus( cb->widget );    
}

static Widget create_input_widget( int task, int var, Widget mgr )
{

    char *section = get_task_name(task);
    char *name = STR(var,0);

    
    char *input_var;
    asprintf(&input_var, "var_%s_%s", section, name );
    TRACE(4, "defining input box for var %s", input_var );

    // our manager is a gridbox widget
    // create a label and input widget inside 
    Widget w;
    
    // create a label to display "name"
    w = XtVaCreateManagedWidget(name,wlabelWidgetClass, mgr,
				XtNweightx, 0,
				XtNweighty, 0,
				XtNlabel, name,
				NULL );

    w = XtVaCreateManagedWidget(input_var,weditMVWidgetClass, mgr,
				XtVaTypedArg, XtNtextBuffer,
				  XtRString, input_var, strlen(input_var) + 1,
				XtNgridx, 1,
				XtNweightx, 1,
				XtNweighty, 0,
				XtNlabel, name,
				NULL);

    // create a callback and an info structure to copy the data from the
    // input widget to a chaper-local input storage (vset)
    // which can be used to create variable expansion strings
    struct chapter_info_st *chap = get_chapter_info(task);
    struct chap_callback_st *cb = m_add( chap->callback );
    cb->mvar = XrmStringToQuark( input_var  ); // used by weditMVWidgetClass
    cb->var  = v_lookup( chap->vset, name ); // used by string_expand
    cb->widget = w;
    mv_onwrite( cb->mvar, input_cb, (void*) cb, 0 );
    XtAddCallback( w, XtNcallback, input_widget_enter_cb, (void*)(intptr_t) task );
    free(input_var);
    
    return w;
}

void exec_script_cb(Widget w, void *u, void *c)
{
    int task = (intptr_t) u;
    chapter_info_t *chap = get_chapter_info(task);
    TRACE(2,"script: %s", get_task_name(task));    
    int buf;
    buf = s_printf(0,0,"xterm -e './%s; bash' &",m_str(chap->se_script.buf) );
    TRACE(2,"about to execute %s", m_str(buf) );    
    system( m_str(buf) );
}


void create_widget_from_var( int task, int var, Widget mgr )
{
    char *section = get_task_name(task);
    char *name = STR(var,0);
    char *type = STR(var,1);
    TRACE(4, "section: %s, name: %s, type %s", section, name,type );

    if( strncasecmp( type, "input", 5) == 0 )
	{
	    create_input_widget( task, var, mgr );
	}
    else if( strncasecmp( type, "script:", 7) == 0 )
	{
	    chapter_info_t *chap = get_chapter_info(task);
	    if( chap->se_script.buf ) ERR("only one script allowed");
	    se_init( & chap->se_script );
	    se_parse( & chap->se_script, type+7 );
	    chap->w_script = create_script_widget( task, var, mgr );
	    XtAddCallback( chap->w_script, XtNcallback,
			   exec_script_cb, (void*) (intptr_t) task );
	}
}


static chapter_info_t *create_chapter( int num )
{
    chapter_info_t *chap = get_chapter_info( num );
    chap->vset = v_init();
    chap->callback = m_create(5, sizeof (struct chap_callback_st ));
    chap->name = get_task_name(num);
    chap->opt = get_section(chap->name); 
    return chap;
}

// 
// unmanage  SIGNAGE.widget_grid
// manage       chapter[0].widget_grid
// or create    chapter[0].widget_grid
static void create_chapter0( Widget mgr, int task )
{
    Widget w;
    Arg wargs[10];
    int n;

    chapter_info_t *chap = create_chapter( task );    
    int x, y, p; int *d;
    y=0;
    m_foreach( chap->opt, p, d ) {
	w = XtVaCreateManagedWidget("chapter0",gridboxWidgetClass, mgr,
				    XtNgridy, y,
				    XtNweightx, 1,
				    XtNweighty, 1,
				    NULL );

	create_widget_from_var( task, *d, w );
	y++; 
    }
    
    char *label = "test chapter 0 leave";
    n=0; x=0; 
    XtSetArg(wargs[n], XtNgridx, x); n++;
    XtSetArg(wargs[n], XtNgridy, y); n++;
    XtSetArg(wargs[n], XtNlabel, label); n++;
     // w = XtCreateManagedWidget(label,wlabelWidgetClass, mgr, wargs, n );
    w = XtCreateManagedWidget(label,wbuttonWidgetClass, mgr, wargs, n );
    XtAddCallback(w, XtNcallback, chapter0_leave, (XtPointer)0 );
}


// callback from menu_page`
// unmanage page[0], manage page[1]
void chapter_cb( Widget w, void *u, void *c )
{
    int chap = (intptr_t) u;
    TRACE(1,"chapter %d", chap);

    if( chap == 0 ) {
	switch_page(1);
	
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
    RCP( top, weditMV );
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_cb );
    RCB( top, test_cb );
    RCB( top, terminal_output_cb );
}

/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/




static void create_menu_page(Widget mgr)
{
    char *label;
    Widget w;
    Arg wargs[10];
    int n;
    int options = get_section("task");
    int max = m_len( options );
    int xmax,ymax;
    LABELS=m_create(max,sizeof(Widget));
    double a = sqrt(max);
    xmax = a;
    ymax = (max / a)+1;
    MAX_LABELS = 0;
    for(int y=0;y<ymax;y++) {
	for(int x=0;x<xmax;x++) {
	    if( MAX_LABELS >= max ) break;
	    label =  STR(INT(options, MAX_LABELS), 1);
	    n=0;
	    XtSetArg(wargs[n], XtNgridx, x); n++;
	    XtSetArg(wargs[n], XtNgridy, y); n++;
	    XtSetArg(wargs[n], XtNlabel, label); n++;
	    // w = XtCreateManagedWidget(label,wlabelWidgetClass, mgr, wargs, n );
	    w = XtCreateManagedWidget(label,wbuttonWidgetClass, mgr, wargs, n );
	    XtAddCallback(w, XtNcallback, chapter_cb, (XtPointer)(intptr_t)MAX_LABELS  );
	    m_put(LABELS, &w);
	    MAX_LABELS++;
	}
    }    
}


static void InitializeApplication( Widget top )
{

    trace_level = SIGNAGE.traceLevel;
    if( SIGNAGE.widget_grid == 0 ) ERR("gridwidget not defined in signage.ad");
    if(!SIGNAGE.inifile) ERR("inifile not defined");

    slog_init( top, SIGNAGE.socket );
    slog_cb = incomming_msg;
    rc_init( SIGNAGE.inifile );

    PAGES=m_create(10,sizeof(Widget));

    Widget w =  SIGNAGE.widget_grid;
    Widget mgr;

    mgr = XtCreateWidget("menu",gridboxWidgetClass, w, NULL,0 );
    m_put(PAGES, &mgr);
    create_menu_page(mgr);

    mgr = XtCreateWidget("chapter0",gridboxWidgetClass, w, NULL,0 );
    m_put(PAGES, &mgr);
    create_chapter0(mgr, 0 ); // task 0 in section task

    XtManageChild( get_page_widget(0) );
    CUR_PAGE=0;
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

  Widget *w = mls(LABELS,num);
  XtVaSetValues( *w, "text", label,
		 XtNstate, state, NULL ); 

  slog_close(fd);
    
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
    mv_init();
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
