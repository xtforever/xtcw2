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
  
  PopShell = XtCreatePopupShell ( "shell2", topLevelShellWidgetClass,
                                  TopLevel, NULL, 0 );

  Widget gb = XtCreateManagedWidget ( "mgr", gridboxWidgetClass,
                            PopShell, NULL, 0 );

  Widget g1 = XtCreateManagedWidget ( "msg", messageBoxWidgetClass,
				      gb, NULL, 0 );
  
  XtAddCallback(g1, "callback", popdown_cb, NULL );
  
  XtPopup ( PopShell, XtGrabNone );
}



void msgbox_init(Widget top)
{
  RCB(top, MessageBox );
}
