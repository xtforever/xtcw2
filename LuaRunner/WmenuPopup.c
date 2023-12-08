/* Generated by wbuild
 * (generator version 3.3)
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#line 142 "WmenuPopup.widget"
#include "mls.h"
#line 143 "WmenuPopup.widget"
#include "converters.h"
#line 144 "WmenuPopup.widget"
#include <X11/Shell.h>
#line 145 "WmenuPopup.widget"
#include <X11/Xatom.h>
#line 146 "WmenuPopup.widget"
#include <X11/Xft/Xft.h>
#line 147 "WmenuPopup.widget"
#include "converters-xft.h"
#line 148 "WmenuPopup.widget"
#include <X11/Xmu/Converters.h>
#line 149 "WmenuPopup.widget"
#include <X11/Xaw/XawImP.h>
#line 150 "WmenuPopup.widget"
#include <X11/Xaw/XawImP.h>
#line 151 "WmenuPopup.widget"
#include <X11/Xft/Xft.h>
#line 152 "WmenuPopup.widget"
#include <X11/Xmu/Converters.h>
#line 153 "WmenuPopup.widget"
#include "xutil.h"
#line 154 "WmenuPopup.widget"
#include "xtcw/Wls.h"
#include <xtcw/WmenuPopupP.h>
#line 96 "WmenuPopup.widget"
static void SetKeyboardFocus(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);
#line 110 "WmenuPopup.widget"
static void print_info(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);
#line 115 "WmenuPopup.widget"
static void menu_popdown(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);
#line 127 "WmenuPopup.widget"
static void menu_open(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);
#line 134 "WmenuPopup.widget"
static void notify(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);

static XtActionsRec actionsList[] = {
{"SetKeyboardFocus", SetKeyboardFocus},
{"print_info", print_info},
{"menu_popdown", menu_popdown},
{"menu_open", menu_open},
{"notify", notify},
};

static char defaultTranslations[] = "\
<Btn1Down>: SetKeyboardFocus() menu_open() \n\
";
static void _resolve_inheritance(
#if NeedFunctionPrototypes
WidgetClass
#endif
);
#line 33 "WmenuPopup.widget"
static void initialize(
#if NeedFunctionPrototypes
Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 45 "WmenuPopup.widget"
static void destroy(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 54 "WmenuPopup.widget"
static void open_popup(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 85 "WmenuPopup.widget"
static void popdown_shell_notify_client(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 54 "WmenuPopup.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 54 "WmenuPopup.widget"
static void open_popup(Widget self)
#else
#line 54 "WmenuPopup.widget"
static void open_popup(self)Widget self;
#endif
#line 55 "WmenuPopup.widget"
{
    if( ((WmenuPopupWidget)self)->wmenuPopup.entry_mode ) return;

    if( ! ((WmenuPopupWidget)self)->wmenuPopup.shell ) {
	    ((WmenuPopupWidget)self)->wmenuPopup.shell = XtCreatePopupShell ( "combo_shell", overrideShellWidgetClass,
                                  self, NULL, 0 );
    	    ((WmenuPopupWidget)self)->wmenuPopup.listbox = XtVaCreateManagedWidget ( "combo-scrollbox", wlsWidgetClass,
				       ((WmenuPopupWidget)self)->wmenuPopup.shell, NULL );
	    /* XtAddCallback($listbox, "callback", (XtCallbackProc)item_selected_cb, $ ); */

	    XtRealizeWidget(((WmenuPopupWidget)self)->wmenuPopup.shell);
	    static Atom wm_delete_window;
    	    wm_delete_window = XInternAtom(XtDisplay(self), "WM_DELETE_WINDOW",
                                   False);
    	    (void) XSetWMProtocols (XtDisplay(self), XtWindow(((WmenuPopupWidget)self)->wmenuPopup.shell),
                            &wm_delete_window, 1);

        XtAugmentTranslations( ((WmenuPopupWidget)self)->wmenuPopup.shell, XtParseTranslationTable( "<LeaveWindow>: XtMenuPopdown() menu_popdown()"));
	}

	Position root_x, root_y;
	XtTranslateCoords(self, 0, ((WmenuPopupWidget)self)->core.height + ((WmenuPopupWidget)self)->core.border_width, &root_x, &root_y);
	XtVaSetValues(((WmenuPopupWidget)self)->wmenuPopup.shell, "x", (int) root_x, "y", (int) root_y, NULL );
	XtResizeWidget(((WmenuPopupWidget)self)->wmenuPopup.shell, ((WmenuPopupWidget)self)->core.width, ((WmenuPopupWidget)self)->wmenuPopup.lines_height * ((WmenuPopupWidget)self)->wheel.xftFont->height, 0 );

	XtPopup(((WmenuPopupWidget)self)->wmenuPopup.shell, XtGrabNone);
	((WmenuPopupWidget)self)->wmenuPopup.entry_mode=1;
}
#line 85 "WmenuPopup.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 85 "WmenuPopup.widget"
static void popdown_shell_notify_client(Widget self)
#else
#line 85 "WmenuPopup.widget"
static void popdown_shell_notify_client(self)Widget self;
#endif
#line 86 "WmenuPopup.widget"
{
	XtPopdown(((WmenuPopupWidget)self)->wmenuPopup.shell);
	((WmenuPopupWidget)self)->wmenuPopup.entry_mode=0;
	XtCallCallbackList( self, ((WmenuPopupWidget)self)->wmenuPopup.combo_cb, (XtPointer) (intptr_t) ((WmenuPopupWidget)self)->wmenuPopup.selection );
}

static XtResource resources[] = {
#line 12 "WmenuPopup.widget"
{XtNcombo_cb,XtCCombo_cb,XtRCallback,sizeof(((WmenuPopupRec*)NULL)->wmenuPopup.combo_cb),XtOffsetOf(WmenuPopupRec,wmenuPopup.combo_cb),XtRImmediate,(XtPointer)NULL },
#line 13 "WmenuPopup.widget"
{XtNlst,XtCLst,XtRStringMArray,sizeof(((WmenuPopupRec*)NULL)->wmenuPopup.lst),XtOffsetOf(WmenuPopupRec,wmenuPopup.lst),XtRString,(XtPointer)"A,B,C"},
};

WmenuPopupClassRec wmenuPopupClassRec = {
{ /* core_class part */
/* superclass   	*/  (WidgetClass) &wlabelClassRec,
/* class_name   	*/  "WmenuPopup",
/* widget_size  	*/  sizeof(WmenuPopupRec),
/* class_initialize 	*/  NULL,
/* class_part_initialize*/  _resolve_inheritance,
/* class_inited 	*/  FALSE,
/* initialize   	*/  initialize,
/* initialize_hook 	*/  NULL,
/* realize      	*/  XtInheritRealize,
/* actions      	*/  actionsList,
/* num_actions  	*/  5,
/* resources    	*/  resources,
/* num_resources 	*/  2,
/* xrm_class    	*/  NULLQUARK,
/* compres_motion 	*/  False ,
/* compress_exposure 	*/  TRUE ,
/* compress_enterleave 	*/  False ,
/* visible_interest 	*/  False ,
/* destroy      	*/  destroy,
/* resize       	*/  XtInheritResize,
/* expose       	*/  XtInheritExpose,
/* set_values   	*/  NULL,
/* set_values_hook 	*/  NULL,
/* set_values_almost 	*/  XtInheritSetValuesAlmost,
/* get_values+hook 	*/  NULL,
/* accept_focus 	*/  XtInheritAcceptFocus,
/* version      	*/  XtVersion,
/* callback_private 	*/  NULL,
/* tm_table      	*/  defaultTranslations,
/* query_geometry 	*/  XtInheritQueryGeometry,
/* display_acceleator 	*/  XtInheritDisplayAccelerator,
/* extension    	*/  NULL 
},
{ /* Wheel_class part */
XtInherit_exec_command,
XtInherit_sig_recv,
},
{ /* Wlabel_class part */
XtInherit_cache_hit,
},
{ /* WmenuPopup_class part */
 /* dummy */  0
},
};
WidgetClass wmenuPopupWidgetClass = (WidgetClass) &wmenuPopupClassRec;
/*ARGSUSED*/
#line 96 "WmenuPopup.widget"
static void SetKeyboardFocus(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
    TRACE(2,"try to set keyboard focus");
    Widget shell, parent;

    shell = parent = self;
    while (parent) {
        if (XtIsShell(shell = parent))
            break;
        parent = XtParent(parent);
    }

    XtSetKeyboardFocus(shell, self);
}

/*ARGSUSED*/
#line 110 "WmenuPopup.widget"
static void print_info(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
	TRACE(2, "info %s", XtName(self));
}

/*ARGSUSED*/
#line 115 "WmenuPopup.widget"
static void menu_popdown(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
	TRACE(2, "WmenuPopup %s menu_popdown", XtName(self));
     ((WmenuPopupWidget)self)->wmenuPopup.entry_mode=0;
    if( ((WmenuPopupWidget)self)->wmenuPopup.entry_mode ) {
	    // XtPopdown(((WmenuPopupWidget)self)->wmenuPopup.shell);

       // XtMenuPopdown(((WmenuPopupWidget)self)->wmenuPopup.shell);
	    ((WmenuPopupWidget)self)->wmenuPopup.entry_mode=0;
	}
}

/*ARGSUSED*/
#line 127 "WmenuPopup.widget"
static void menu_open(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
	TRACE(2, "WmenuPopup %s menu_open", XtName(self));
    open_popup(self);

}

/*ARGSUSED*/
#line 134 "WmenuPopup.widget"
static void notify(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
	TRACE(2, "WmenuPopup  %s notify: entry_mode %d", XtName(self), ((WmenuPopupWidget)self)->wmenuPopup.entry_mode );
	/* item_selected_cb(NULL, $, (XtPointer)0 ); */
}

static void _resolve_inheritance(class)
WidgetClass class;
{
  WmenuPopupWidgetClass c __attribute__((unused)) = (WmenuPopupWidgetClass) class;
  WmenuPopupWidgetClass super __attribute__((unused));
  if (class == wmenuPopupWidgetClass) return;
  super = (WmenuPopupWidgetClass)class->core_class.superclass;
}
#line 33 "WmenuPopup.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 33 "WmenuPopup.widget"
static void initialize(Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 33 "WmenuPopup.widget"
static void initialize(request,self,args,num_args)Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 34 "WmenuPopup.widget"
{

    TRACE(2, "init %s", XtName(self) );
	((WmenuPopupWidget)self)->wmenuPopup.shell = 0;
	((WmenuPopupWidget)self)->wmenuPopup.entry_mode=0;
	((WmenuPopupWidget)self)->wmenuPopup.filter = m_create(200,1);
	((WmenuPopupWidget)self)->wmenuPopup.f_lst = m_create(100,sizeof(char*) );
	((WmenuPopupWidget)self)->wmenuPopup.k_lst = m_create(100,sizeof(int) );
    ((WmenuPopupWidget)self)->wmenuPopup.lines_height = 20;
}
#line 45 "WmenuPopup.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 45 "WmenuPopup.widget"
static void destroy(Widget self)
#else
#line 45 "WmenuPopup.widget"
static void destroy(self)Widget self;
#endif
#line 46 "WmenuPopup.widget"
{
	m_free( ((WmenuPopupWidget)self)->wmenuPopup.filter );
	m_free( ((WmenuPopupWidget)self)->wmenuPopup.f_lst  );
	m_free( ((WmenuPopupWidget)self)->wmenuPopup.k_lst  );
}
