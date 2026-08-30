#include "msg_box.h"


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
#include "widreg.h"
#include "mls.h"



static void SetWMProtocolTranslations(Widget w);


extern Widget TopLevel;
Widget PopShell = 0;

static void popdown_cb( Widget w, void *u, void *c )
{
    int letter = (intptr_t) u;
    TRACE(1,"popdown %c", letter );

    XtPopdown(PopShell);
	      // XtDestroyWidget(popShell);
	      // popShell=0;
}


void
MessageBox(Widget w, XtPointer client_data, XtPointer call_data)
{
  WARN("MessageBox");

  if( PopShell ) {
     XtPopup ( PopShell, XtGrabNone );
     return;
  }
  
  PopShell = XtVaCreatePopupShell ( "shell2", transientShellWidgetClass,
				    TopLevel,
				    XtNtransientFor, TopLevel,
				    XtNallowShellResize, True,
				    NULL );
  
  Widget gb = XtCreateManagedWidget ( "mgr", gridboxWidgetClass,
                            PopShell, NULL, 0 );
  // window close will call XtcwWMProtocols which calls the "cancel" widget callback
  Widget g1 = XtCreateManagedWidget ( "cancel", messageBoxWidgetClass,
				      gb, NULL, 0 );
  XtAddCallback(g1, "callback", popdown_cb, NULL );
  XtPopup ( PopShell, XtGrabNone );
  
  
  /* XtAugumentTranslations */
  /* 	(PopShell, XtParseTranslationTable ("<Message>WM_PROTOCOLS: PopDown()")); */
  /*  static Atom wm_delete_window; */
  /*  wm_delete_window = XInternAtom(XtDisplay(PopShell), "WM_DELETE_WINDOW", */
  /* 				   False); */
  /*  (void) XSetWMProtocols (XtDisplay(PopShell), XtWindow(PopShell), */
  /* 			   &wm_delete_window, 1); */
  



  SetWMProtocolTranslations(PopShell);
}


/*
 * Function
 *	GetShell
 *	nearest shell widget.
 *
 * Parameters:
 *	w - widget whose parent shell should be returned
 *
 * Returns:
 *	  The shell widget among the ancestors of w that is the
 *	fewest levels up in the widget hierarchy.
 *
 * Description:
 *	Walks up the widget hierarchy to find the topmost shell widget.
 */
static Widget
GetShell(Widget w)
{
    while (w != NULL && !XtIsShell(w))
	w = XtParent(w);

    return (w);
}

static Bool
InParams(String str, String *p, unsigned int n)
{
    unsigned int i;

    for (i = 0; i < n; p++, i++)
	if (! strcmp(*p, str))
	    return (True);
    return (False);
}

static char *WM_DELETE_WINDOW = "WM_DELETE_WINDOW";
static char* DISMISS_NAME = "cancel";
#define DISMISS_NAME_LEN 6

static void
XtcwWMProtocols(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    Atom wm_delete_window;
    Atom wm_protocols;

    wm_delete_window = XInternAtom(XtDisplay(w), WM_DELETE_WINDOW, True);
    wm_protocols = XInternAtom(XtDisplay(w), "WM_PROTOCOLS", True);

    /* Respond to a recognized WM protocol request if
     * event type is ClientMessage and no parameters are passed, or
     * event type is ClientMessage and event data is matched to parameters, or
     * event type isn't ClientMessage and parameters make a request
     */
#define DO_DELETE_WINDOW InParams(WM_DELETE_WINDOW, params, *num_params)

    if ((event->type == ClientMessage
	 && event->xclient.message_type == wm_protocols
	 && event->xclient.data.l[0] == wm_delete_window
	 && (*num_params == 0 || DO_DELETE_WINDOW))
	|| (event->type != ClientMessage && DO_DELETE_WINDOW)) {
#undef DO_DELETE_WINDOW
	Widget cancel;
	char descendant[DISMISS_NAME_LEN + 2];

	snprintf(descendant, sizeof(descendant), "*%s", DISMISS_NAME);
	cancel = XtNameToWidget(w, descendant);
	if (cancel)
	    XtCallCallbacks(cancel, XtNcallback, NULL);
    }
}

static void
SetWMProtocolTranslations(Widget w)
{
    static XtTranslations compiled_table;
    static XtAppContext *app_context_list;
    static Cardinal list_size;

    unsigned int i;
    XtAppContext app_context;
    Atom wm_delete_window;

    app_context = XtWidgetToApplicationContext(w);

    /* parse translation table once */
    if (!compiled_table)
	compiled_table =
	    XtParseTranslationTable("<Message>WM_PROTOCOLS:XtcwWMProtocols()\n");

    /* add actions once per application context */
    for (i = 0; i < list_size && app_context_list[i] != app_context; i++)
	;
    if (i == list_size) {
	XtActionsRec actions[1];

	actions[0].string = "XtcwWMProtocols";
	actions[0].proc = XtcwWMProtocols;
	list_size++;
	app_context_list = (XtAppContext *)XtRealloc
	    ((char *)app_context_list, list_size * sizeof(XtAppContext));
	XtAppAddActions(app_context, actions, 1);
	app_context_list[i] = app_context;
    }

    /* establish communication between the window manager and each shell */
    XtAugmentTranslations(w, compiled_table);
    wm_delete_window = XInternAtom(XtDisplay(w), WM_DELETE_WINDOW, False);
    (void)XSetWMProtocols(XtDisplay(w), XtWindow(w), &wm_delete_window, 1);
}



void msgbox_init(Widget top)
{
  RCB(top, MessageBox );
}
