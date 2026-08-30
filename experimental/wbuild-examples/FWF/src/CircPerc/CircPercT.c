#ifndef _HPUX_SOURCE
#define _HPUX_SOURCE
#endif
#include <X11/Intrinsic.h>
#include <Xfwf/CircPerc.h>

long lInterval = 125;

int iPercentage = 0;

#if NeedFunctionPrototypes > 0
void percLoop(XtPointer, XtIntervalId *);
void percLoop2(XtPointer, XtIntervalId *);
#else
void percLoop();
void percLoop2();
#endif

#if NeedFunctionPrototypes > 0
int main(int argc, char *argv[])
#else
int main(argc, argv)
int argc;
char *argv[];
#endif
{
  Widget toplevel,rc;
  Widget perc1,perc2;
  XtAppContext app;

  toplevel = XtVaAppInitialize(&app, 
				 "Tools", 
				 NULL, 0,
                                 &argc, argv, 
				 NULL, NULL);
  
  perc1 = XtVaCreateManagedWidget("perc1", 
	   			  xfwfCircularPercentageWidgetClass, 
				  toplevel,
                                  NULL,NULL);
  XtAppAddTimeOut(app,
		  lInterval,
		  percLoop,
		  (XtPointer) perc1);

  XtRealizeWidget(toplevel);
  XtAppMainLoop(app);

  return 0;

}

#if NeedFunctionPrototypes > 0
void percLoop(XtPointer w, XtIntervalId *dummy)
#else
void percLoop(w, dummy)
XtPointer w;
XtIntervalId *dummy;
#endif
{
  static int iPercentage = 0;
  iPercentage += 100;

  if (iPercentage > 10000)
    iPercentage = 0;

  XfwfCircularPercentageSetPercentage((Widget) w, iPercentage);

  XtAppAddTimeOut(XtWidgetToApplicationContext((Widget) w),
		  lInterval,
		  percLoop,
		  (XtPointer) w);
}

#if NeedFunctionPrototypes > 0
void percLoop2(XtPointer w, XtIntervalId *dummy)
#else
void percLoop2(w, dummy)
XtPointer w;
XtIntervalId *dummy;
#endif
{
  static int iPercentage = 10100;
  iPercentage -= 100;

  if (iPercentage < 0)
    iPercentage = 10000;

  XfwfCircularPercentageSetPercentage((Widget) w, iPercentage);

  XtAppAddTimeOut(XtWidgetToApplicationContext((Widget) w),
		  lInterval,
		  percLoop2,
		  (XtPointer) w);
}
