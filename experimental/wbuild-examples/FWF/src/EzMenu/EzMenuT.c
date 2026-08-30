#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/Xatom.h>
#include <X11/Object.h>
#include <X11/ObjectP.h>

#include <assert.h>

#include "X11/Xaw/Label.h"
#include <Xfwf/EzMenu.h>

#ifdef SVR4                                     /* BB, 27 Mar '96 */
#define index(s, c) strchr(s, c)
#endif


int Argc;
char **Argv;

Widget TopLevelWidget;
static Widget labelMenu;
static Widget menu;

char *ProgName = "Eztest";
char *ClassName = "Eztest";

int 
main(argc,argv)
char ** argv;
int argc;
{

  Argc = argc;
  Argv = argv;

  printf("\nThis is EzMenuT.  Be sure to load the menu resource\n");
  printf("description first with:\n");
  printf("\txrdb -display unix:0 -merge EzMenuT.res\n\n");

  if( (ProgName = index(argv[0], '/')) == NULL )
	  ProgName = argv[0];

  TopLevelWidget = XtInitialize(ProgName, ClassName,
				NULL, 0,
				(Cardinal*)(&Argc), Argv);

  XtGetApplicationResources(TopLevelWidget,
			    NULL,
			    NULL, 0,
			    NULL, (Cardinal) 0);


  labelMenu =
    XtCreateManagedWidget("label",
			  labelWidgetClass, TopLevelWidget, NULL, 0);

  menu = XtCreatePopupShell("menu",
				 xfwfEzMenuWidgetClass,
				 TopLevelWidget, NULL, 0);

  XtRealizeWidget(TopLevelWidget);

  XtMainLoop();
  exit(0);
}
