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
#include <X11/IntrinsicP.h>
#include <X11/CreateI.h>
#include <X11/IntrinsicI.h>
#include <X11/ThreadsI.h>
#include <X11/VarargsI.h>
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
    // lua_pushstring( LUA_CTX, s );
    // lua_setglobal(LUA_CTX, "class_data");
    /* can we expand vars inside the lua code? e.g. player=$radio.selection1 */
    /* we need to store additional data besides the lua code e.g. class_data */
    luaxt_pushcallback( client_data, s );
}


#define luastring(L,i) (char*)lua_tostring(L,i)


/** XtCreate(WidgetName, WidgetClass, ParentWidgetPath,  Resource, Value, ... )
 **/
static int
xtcreate_lua( lua_State *L )
{
    int num_args = lua_gettop(L);
    if( num_args < 3 ) return 0;

    /* check all arguments */
    for(int i=1; i<= num_args; i++ ) {
	char *s = (char *)luastring(L,i);
	if( s == NULL ) {
	    WARN("argument %d must be a string", i );
	    goto fail;
	}
    }

    char *name = luastring(L,1);
    char *class_name = luastring(L,2);
    char *path = luastring(L,3);

    Widget parent = luaxt_nametowidget( path );
    if( parent == NULL ) {
	WARN("widget with the name '%s' not found", path );
	goto fail;
    }

    XrmQuark class_quark = WcStringToQuark(class_name);
    XtAppContext app = XtWidgetToApplicationContext( parent );
    WidgetClass class = WcMapClassFind( app, (intptr_t)class_quark );
    if( class == NULL ) {
	WARN("widget class '%s' not found", class_name );
	goto fail;
    }

    int pairs = (num_args-3) / 2; /* number of resource=value pairs */
    int arg_list = m_create( pairs, sizeof(XtTypedArg) );
    XtTypedArg *arg;

     int i=4;
     while( pairs-- ) {
	 arg=m_add(arg_list);
	 arg->name  = luastring(L, i++ );
	 char *s = luastring(L, i++ );
	 arg->type  = XtRString;
	 arg->value = (XtArgVal)s;
	 arg->size  = strlen(s)+1;
     }

    /* Widget _XtCreateWidget(
       String      name,
       WidgetClass widget_class,
       Widget      parent,
       ArgList     args,
       Cardinal    num_args,
       XtTypedArgList typed_args,
       Cardinal	num_typed_args)
    */

     Widget w = _XtCreateWidget(name,class,parent,NULL,0, m_buf(arg_list), m_len(arg_list) );
     m_free(arg_list);
     XtManageChild(w);
     lua_pushlightuserdata( L, w );
     return 1;

 fail:
     luaL_pushfail(L);
     return 0;
}

static String XtNxtConvertVarToArgList = "xtConvertVarToArgList";
static void GetResources(Widget widget, XtResourceList *res_list,
			 Cardinal *number);


/*
 *    TypedArgToArg() invokes a resource converter to convert the
 *    passed typed arg into a name/value pair and stores the name/value
 *    pair in the passed Arg structure.  If memory is allocated for the
 *    converted value, the address is returned in the value field of
 *    memory_return; otherwise that field is NULL.  The function returns
 *    1 if the conversion succeeded and 0 if the conversion failed.
 */
static int
TypedArgToArg(
    Widget              widget,
    XtTypedArgList      typed_arg,
    ArgList             arg_return,
    XtResourceList      resources,
    Cardinal            num_resources,
    ArgList		memory_return)
{
    String              to_type = NULL;
    XrmValue            from_val, to_val;

    if (widget == NULL) {
        XtAppWarningMsg(XtWidgetToApplicationContext(widget),
            "nullWidget", XtNxtConvertVarToArgList, XtCXtToolkitError,
	    "XtVaTypedArg conversion needs non-NULL widget handle",
            (String *)NULL, (Cardinal *)NULL);
        return(0);
    }

    /* again we assume that the XtResourceList is un-compiled */

    for (; num_resources--; resources++)
	if (strcmp(typed_arg->name, resources->resource_name) == 0) {
	    to_type = resources->resource_type;
	    break;
	}

    if (to_type == NULL) {
	WARN("Unknow resource specified: %s", typed_arg->name );
        XtAppWarningMsg(XtWidgetToApplicationContext(widget),
            "unknownType", XtNxtConvertVarToArgList, XtCXtToolkitError,
            "Unable to find type of resource for conversion",
            (String *)NULL, (Cardinal *)NULL);
        return(0);
    }

    to_val.addr = NULL;
    from_val.size = typed_arg->size;
    if ((strcmp(typed_arg->type, XtRString) == 0) ||
            ((unsigned) typed_arg->size > sizeof(XtArgVal))) {
        from_val.addr = (XPointer)typed_arg->value;
    } else {
            from_val.addr = (XPointer)&typed_arg->value;
    }

    LOCK_PROCESS;
    XtConvertAndStore(widget, typed_arg->type, &from_val, to_type, &to_val);

    if (to_val.addr == NULL) {
	UNLOCK_PROCESS;
        XtAppWarningMsg(XtWidgetToApplicationContext(widget),
            "conversionFailed", XtNxtConvertVarToArgList, XtCXtToolkitError,
            "Type conversion failed", (String *)NULL, (Cardinal *)NULL);
        return(0);
    }

    arg_return->name = typed_arg->name;
    memory_return->value = (XtArgVal) NULL;

    if (strcmp(to_type, XtRString) == 0) {
	arg_return->value = (XtArgVal) to_val.addr;
    }
    else {
	if (to_val.size == sizeof(long))
	    arg_return->value = (XtArgVal) *(long *)to_val.addr;
	else if (to_val.size == sizeof(int))
	    arg_return->value = (XtArgVal) *(int *)to_val.addr;
	else if (to_val.size == sizeof(short))
	    arg_return->value = (XtArgVal) *(short *)to_val.addr;
	else if (to_val.size == sizeof(char))
	    arg_return->value = (XtArgVal) *(char *)to_val.addr;
	else if (to_val.size == sizeof(XtArgVal))
	    arg_return->value = *(XtArgVal *)to_val.addr;
	else if (to_val.size > sizeof(XtArgVal)) {
	    arg_return->value = (XtArgVal) __XtMalloc(to_val.size);
	    memory_return->value = (XtArgVal)
		memcpy((void *)arg_return->value, to_val.addr, to_val.size);
	}
    }
    UNLOCK_PROCESS;

    return(1);
}


/*
 *    Given a typed argument list, TypedArgListToArgList() returns the
 *    equivalent ArgList and count. TypedArgListToArgList() handles
 *    typed arguments.  The
 *    ArgList should be freed with _XtFreeArgList.
 */
void TypedArgListToArgList(
    Widget		widget,
    XtTypedArgList	typed_arg_list,
    int			max_count,
    ArgList		*args_return,
    Cardinal		*num_args_return )
{

    int			count;
    ArgList		args = (ArgList)NULL;
    XtResourceList	resources = (XtResourceList)NULL;
    Cardinal		num_resources;


    if( (max_count  == 0) || (widget == NULL) ) {
	*num_args_return = 0;
	*args_return = (ArgList)NULL;
	return;
    }

    args = (ArgList)__XtMalloc((unsigned)(max_count * 2 * sizeof(Arg)));
    for (count = max_count *2; --count >= 0; )
	args[count].value = (XtArgVal) NULL;

    GetResources(widget, &resources, &num_resources);

    for(int i=0; i<max_count; i++ ) {
	TypedArgToArg(widget, &typed_arg_list[i], &args[i],
		      resources, num_resources,
		      &args[max_count + i]);
    }

    XtFree((XtPointer)resources);

    *num_args_return = (Cardinal)max_count;
    *args_return = (ArgList)args;
}

void FreeArgList(
		 ArgList	args,	     /* as returned by TypedArgListToArgList */
		 int		max_count)
{
    _XtFreeArgList(args,max_count,max_count);
}

/*	Function Name: GetResources
 *	Description: Retreives the normal and constraint resources
 *                   for this widget.
 *	Arguments: widget - the widget.
 * RETURNED        res_list - the list of resource for this widget
 * RETURNED        number - the number of resources in the above list.
 *	Returns: none
 */
static void
GetResources(
    Widget widget,
    XtResourceList * res_list,
    Cardinal * number)
{
    Widget parent = XtParent(widget);

    XtInitializeWidgetClass(XtClass(widget));
    XtGetResourceList(XtClass(widget), res_list, number);

    if (!XtIsShell(widget) && parent && XtIsConstraint(parent)) {
	XtResourceList res, constraint, cons_top;
	Cardinal num_constraint, temp;

	XtGetConstraintResourceList(XtClass(parent), &constraint,
				    &num_constraint);

	cons_top = constraint;
	*res_list = (XtResourceList) XtRealloc((char*)*res_list,
					       ((*number + num_constraint) *
						sizeof(XtResource)));

	for (temp= num_constraint, res= *res_list + *number; temp != 0; temp--)
	    *res++ = *constraint++;

	*number += num_constraint;
	XtFree( (XtPointer) cons_top);
    }
}


static void app_arg( int args, const char *key, const char *val )
{
    XtTypedArg *arg=m_add(args);
    arg->name  = (char*)key; /* trust me, i know what i'm doing */
    arg->type  = XtRString;
    arg->value = (XtArgVal)val;
    arg->size  = strlen(val)+1;

    }

static void luatable_to_args( int args, lua_State *L, int table_index, int max )
{

    lua_pushnil(L);  /* first key */
    while (lua_next(L, table_index) != 0) {
	/* uses 'key' (at index -2) and 'value' (at index -1) */
	app_arg(args, lua_tostring(L, -2), lua_tostring(L, -1) );
	/* removes 'value'; keeps 'key' for next iteration */
	lua_pop(L, 1);
    }
}

static int luaarg_to_args( int args,  lua_State *L, int index, int max )
{
	if( lua_istable(L,index) ) {
	    luatable_to_args( args, L, index, max );
	    return 1;
	}

	if( index < max  && lua_isstring(L,index) && lua_isstring(L,index+1) ) {
	    app_arg( args, lua_tostring(L, index ), lua_tostring(L, index+1  ));
	    return 2;
	}

	return 0;
}


static Widget luaarg_to_widget( lua_State *L, int index )
{
    Widget w;
    char *s;

    /* convert lua arg to widget */
    if( lua_isuserdata( L, index ) ) {
	w = lua_touserdata( L, index );
	if( w == NULL ) {
	    WARN("first arg is empty userdata, not a widget" );
	    luaL_pushfail(L);
	}
	return w;
    }

    /* try parsing an xrm path */
    s=luastring(L,1);
    if( s==NULL) {
	WARN("first arg must be string or userdata");
	luaL_pushfail(L);
	return NULL;
    }

    w = luaxt_nametowidget( s );
    if( w == NULL ) {
	WARN("widget '%s' not found", s );
	luaL_pushfail(L);
    }
    return w;
}


/** xtsetvalue( widget, resource-str, value-str )
 */
static int
xtsetvalue_lua( lua_State *L )
{
    Widget w =  NULL;
    int nargs = lua_gettop(L);
    int argi = 0;

    w = luaarg_to_widget( L, 1 );
    if(!w) return 0; /* error no widget defined */

    int pairs = (nargs-1)/2;
    if( pairs < 1 ) return 0; /* error no args given */

    int targ_list = m_create( pairs, sizeof(XtTypedArg) );

    argi=2;
    while( argi <= nargs ) {
	int cnt = luaarg_to_args( targ_list, L, argi, nargs );
	if( cnt == 0 ) {
	    WARN("can not parse argument %d", argi );
	    luaL_pushfail(L);
	    goto cleanup;
	}
	argi += cnt;
    }

    ArgList args; uint max_args;
    TypedArgListToArgList( w, m_buf(targ_list), m_len(targ_list), &args, &max_args );
    XtSetValues( w, args, max_args );
    FreeArgList( args, max_args );

 cleanup:
    m_free(targ_list);
    return 0;
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
    luaopen_luaxt(L); /* functions generated by swig from luaxt.i */
    luaopen_var5( L );
    lua_register(L, "xtcreate", xtcreate_lua );
    lua_register(L, "xtsetvalue", xtsetvalue_lua );

    char *fn = "ex.lua";
    int result,status = luaL_loadfile(L, fn );
    if (status) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        fprintf(stderr, "Couldn't load file: %s\n", luastring(L, -1));
        exit(1);
    }

    /* Ask Lua to run our little script */
    result = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (result) {
        fprintf(stderr, "Failed to run script: %s\n", luastring(L, -1));
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
