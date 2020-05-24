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


#include "xtcw/Wdemo.h"
#include "xtcw/Wdemo2.h"
#include "xtcw/Wdemo3.h"

Widget TopLevel;
int trace_main;
#define TRACE_MAIN 2
static XtAppContext APPCTX;
#define CHECK_TIMER_MS 1000

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

#define FLD(n)  XtOffsetOf(CWRI_CONFIG,n)
#define WID(n) { NULL, NULL, XtRWidget, sizeof(Widget),FLD(n), XtRString, "*" #n }

typedef struct CWRI_CONFIG {
    int mountbuttons;
    int mountpoints;
    int traceLevel;
    Widget mountframe;
} CWRI_CONFIG;
struct CWRI_CONFIG CWRI = { 0 };

static XtResource CWRI_CONFIG_RES [] = {

  { "traceLevel", "TraceLevel", XtRInt, sizeof(int),
    FLD(traceLevel), XtRImmediate, 0
  },

  WID(mountframe)

};
#undef FLD
#undef WID






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

static int m_file_get_contents(int buf, char *fn)
{
    int pos,n,max;
    FILE *fp;

    max=2048;
    assert( fp=fopen(fn,"r") );
    if( !buf ) buf=m_create(max+1,1); else m_clear(buf);
    m_putc(buf,10); pos=1;
    while(1) {
	m_setlen(buf, pos+max );
	n=fread(mls(buf,pos), 1, max, fp);
	if( n != max ) break;
	pos += max; max*=2;
    }
    fclose(fp);
    m_setlen(buf, pos+n );
    m_putc(buf,0);
    return buf;
}


/*
  compatible assignements
  struct nb {
      int number;
      char buf[32];
  }
  nb = 100   <-->  nb.var[0] = 100

  compatible interfaces
  struct n2 {
       int number;
       char buf[32];
       int data_Ext;
  }
  
  matching - names of elements for assignments of structs
  typeless variables for intermediate transport allowed
   - type is implicit contained 
*/

static int  msl_from_file( int buf, char *fn )
{
    int max = 1024;
    FILE *fp;
    assert( fp=fopen(fn,"r") );
    if( !buf ) buf=m_create(max+1,sizeof (int)); else m_clear(buf);

    while(1) {
	int linebuf = m_create(25,1);
	int ret = m_fscan2(linebuf,'\n',fp);
	if( ret == -1 ) {
	    m_free(linebuf);
	    break;
	}
	m_puti(buf,linebuf);
    }

    fclose(fp);
    return buf;
}

static void msl_dump(int msl)
{
    int p,*d;
    m_foreach( msl,p,d ) {
	puts( m_buf(*d) );
    }    
}

static void msl_free(int msl)
{
    if(msl<=0) return;
    int p,*d;
    m_foreach( msl,p,d ) {
	m_free( *d );
    }    
    m_free(msl);
}

static int mstring_is_prefix(int ms, int buf, int cur)
{
    int i,ch1,ch2;
    /* ms has to be a prefix of buf+cur */
    i=0; 
    do {
	ch1 = CHAR(ms,i); 
	if( cur+i >= m_len(buf) ) return -i;
	ch2 = CHAR(buf,cur+i);
	if( ch1 == 0 ) { /* ms equals buf */
	    if( ch2 == 32 ) return i; 
	    break;
	}
	i=i+1;
    } while( ch1 == ch2 );
    return -i;
}


/* dev is first on each line */
static int search_line_prefix(int ms, int buf)
{
    int cur,i;
     
    /* compare ms with buf until 0,32,10,overflow */
    cur=0; 
    while(1) {
	i = mstring_is_prefix( ms, buf, cur );
	if( i > 0 ) return cur;
       
	/* skip to end of line */
	cur -= i;
	do {
	    if( cur >= m_len(buf) ) return -1;
	} while( CHAR(buf,cur++) != 10 );	
    }	
}


static int rec_from_file( int buf, char *fn )
{
    int max = 1024;
    FILE *fp;
    assert( fp=fopen(fn,"r") );
    if( !buf ) buf=m_create(max+1,sizeof (int)); else m_clear(buf);

    int linebuf = m_create(250,1);
    while(1) {
	m_clear(linebuf);
	if( m_fscan2(linebuf,'\n',fp) < 0 ) break;
	int rec = m_split(0,m_buf(linebuf),32,1);
	m_puti(buf,rec);
    }
    m_free(linebuf);
    fclose(fp);
    return buf;
}

static void sl_dump(int sl)
{
    int p; char **d;
    m_foreach( sl,p,d ) {
	puts( *d );
    }
}


static void rec_dump(int m)
{
    int p,*d;
    m_foreach( m,p,d ) {
	printf("--- %d ---", p );
	sl_dump(*d);
    }
}

static void rec_free(int m)
{
    int p,*d;
    m_foreach( m,p,d ) {
	m_free_strings(*d,0);
    }
    m_free(m);
}

static int rec_find(char *s1, int rec_list )
{
    int p,*d;
    m_foreach(rec_list,p,d) {
	char *s = STR(*d,0);
	if( strcmp(s1, s) == 0 )
	    return p;
    }
    return -1;
}


static void all_test_functions()
{
    return;
    
    int b = m_file_get_contents( 0, "/proc/self/mounts" );
    TRACE(1,"len=%d", m_len(b) );

    int msl = msl_from_file(0, "mountpath.ini" );
    msl_dump(msl);
    msl_free(msl);

    /* read records from file, a record is a list of space
       seperated strings 
    */
    int rec = rec_from_file(0, "mountpath.ini" );
    rec_dump(rec);
    int rec2 = rec_from_file(0, "/proc/self/mounts");
    rec_dump(rec2);

    int s1 = INT(rec,0);
    char *str = STR(s1,0);
    rec_find(str, rec2 );

    #if 0
      char *args[] = { "mount", "192.168.178.185:/mnt/back2019a", "/mnt/back2019a", "-t", "nfs", NULL };
      char *path   = "/bin/mount";
      execv( path, args );
      perror("");
    #endif
    
    rec_free(rec);
    rec_free(rec2);

    
    
    m_destruct(); ERR("exit");
}


static void load_mountpoints(void)
{
    CWRI.mountpoints =
	rec_from_file(0, "mountpath.ini" );    
}


static void blocking_execvp(char *cmd, char **argv)
{
    pid_t parent = getpid();
    pid_t pid = fork();

    if (pid == -1)
	{
	    ERR("failed to fork()");
	} 
    else if (pid > 0)
	{
	    int status;
	    waitpid(pid, &status, 0);
	}
    else 
	{
	    // we are the child
	    if( execvp(cmd,argv) ) perror("");	
	    _exit(EXIT_FAILURE);   // exec never returns
	}
}

static int exec_command( char *cmd, int params )
{
    int p;
    char **str;
    int sl = m_create(m_len(params)+2, sizeof(char*));
    m_put(sl, &cmd );
    m_foreach( params,p,str ) m_put( sl, str );
    char *null = NULL; m_put( sl, &null );
    blocking_execvp(cmd,m_buf(sl));
    m_free(sl);
}

static void toggle_mount_cb(Widget w, void *u, void *c )
{
    int state;
    char *name = u;
    XtVaGetValues(w, XtNstate, &state, NULL );

    /* find name in mountpoints */
    int p = rec_find(name, CWRI.mountpoints );
    if( p < 0 ) { WARN("mount:%s not found", name ); return; }
    int params = INT(CWRI.mountpoints,p);

    if( state == 0 ) {
	exec_command("mount", params);
    } else {
	/* Array sl = { name } */
	int sl = m_create(1,sizeof(name)); 
	m_put(sl,&name );
	exec_command("umount",sl);
	m_free(sl); /* delete sl */
    }
}


/* override translations, mountpoints is defined */
static void create_mount_buttons(void)
{
    Widget w;
    char *s = "<BtnDown>: notify()";
    
    CWRI.mountbuttons =
	m_create( m_len(CWRI.mountpoints), sizeof(Widget));

    int p,*d;
    m_foreach( CWRI.mountpoints, p, d ) {
	char *name = STR(*d,0);	 
	w = XtVaCreateManagedWidget(
	      name, wbuttonWidgetClass,
	      CWRI.mountframe,
	      XtNlabel, name,
	      XtVaTypedArg,
	      XtNtranslations,
	      XtRString,s, strlen(s)+1,
	      NULL );
	XtAddCallback( w,"callback",
		       toggle_mount_cb,
		       name );
	m_put(CWRI.mountbuttons,&w);
    }
}

static void check_active_mounts(void)
{
    int rec2 = rec_from_file(0, "/proc/self/mounts");

    int p,*d;
    m_foreach(CWRI.mountpoints, p, d ) {
	char *str = STR(*d,0);
	int found = rec_find(str, rec2 );
	/* w = CWRI.mountbuttons[p] */
	Widget w = *(Widget*) mls(CWRI.mountbuttons,p);
	XtVaSetValues( w, XtNstate, found >= 0, NULL );
    }
    rec_free(rec2);
}
static void check_timer(XtPointer data, XtIntervalId *id )
{
    /* XT timer stuff */
    XtAppAddTimeOut(APPCTX,CHECK_TIMER_MS, check_timer, NULL ); 

    /* update widget */
    check_active_mounts();
}


static void RegisterApplication ( Widget top )
{
    /* -- Register widget classes and constructors */
    RCP( top, wdemo );
    RCP( top, wdemo2 );
    RCP( top, wdemo3 );
    
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
    RCB( top, quit_cb );
    RCB( top, toggle_mount_cb );
}

/*  init application functions and structures and widgets
    All widgets are created, but not visible.
    functions can now communicate with widgets
*/
static void InitializeApplication( Widget top )
{
    trace_level = CWRI.traceLevel;
    all_test_functions();
   
    load_mountpoints();
    create_mount_buttons();
    check_timer(NULL,NULL);
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
    m_destruct();

    return EXIT_SUCCESS;
}
