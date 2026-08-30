/*

    Copyright (C) 1994 Clark McGrew

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */

#include	<signal.h>
#include	<stdio.h>
#include	<fcntl.h>
#include        <math.h>
#include	<X11/Intrinsic.h>
#include	<X11/StringDefs.h>
#include	<X11/Shell.h>
#include        <Xfwf/Board.h>
#include        <Xfwf/Button.h>
#include	"UGraf.h"

XtAppContext	app_con;
UGrafWidget UGraf;

Boolean In_3D_Mode = False;

void UGraf3Test(Widget w, XEvent *event, String *argv, Cardinal *argc);
void UGrafTest(Widget w, XEvent *event, String *argv, Cardinal *argc);
void Quit_Application();

void UpdateCallback(Widget w, XtPointer a, XtPointer b) {
    if (In_3D_Mode) {
	UGraf3Test(w,NULL,NULL,NULL);
    }
    else {
	UGrafTest(w,NULL,NULL,NULL);
    }
}

void View0(Widget w, XEvent *event, String *argv, Cardinal *argc){
    Point3 refp, vdir, udir, hdir;
    
    refp.x =  0.0; refp.y =  0.0; refp.z =  0.0;
    vdir.x =  1.0; vdir.y =  0.0; vdir.z =  0.0;
    udir.x =  0.0; udir.y =  0.0; udir.z =  1.0;
    hdir.x =  0.0; hdir.y =  1.0; hdir.z =  0.0;
    XfwfUG3Reference(UGraf,&refp,True);
    XfwfUG3ViewDirection(UGraf,&vdir,True);
    XfwfUG3UpDirection(UGraf,&udir,True);
    XfwfUG3HorizontalDirection(UGraf,&hdir,True);
    XfwfUG3Aperture(UGraf,1.0,True);
}

void View1(Widget w, XEvent *event, String *argv, Cardinal *argc){
    Point3 refp, vdir, udir, hdir;
    
    refp.x =  -3.0; refp.y =  0.0; refp.z =  0.0;
    vdir.x =  1.0; vdir.y =  0.0; vdir.z =  0.0;
    udir.x =  0.0; udir.y =  0.0; udir.z =  1.0;
    hdir.x =  0.0; hdir.y =  1.0; hdir.z =  0.0;
    XfwfUG3Reference(UGraf,&refp,True);
    XfwfUG3ViewDirection(UGraf,&vdir,True);
    XfwfUG3UpDirection(UGraf,&udir,True);
    XfwfUG3HorizontalDirection(UGraf,&hdir,True);
    XfwfUG3Aperture(UGraf,1.0,True);
}

void View2(Widget w, XEvent *event, String *argv, Cardinal *argc){
    Point3 refp, vdir, udir, hdir;
    
    refp.x = -4.0; refp.y =  3.0; refp.z =  5.0;
    vdir.x =  4.0; vdir.y = -3.0; vdir.z = -5.0;
    udir.x =  0.0; udir.y =  0.0; udir.z =  1.0;
    hdir.x =  0.0; hdir.y =  1.0; hdir.z =  0.0;
    XfwfUG3Reference(UGraf,&refp,True);
    XfwfUG3ViewDirection(UGraf,&vdir,True);
    XfwfUG3UpDirection(UGraf,&udir,True);
    XfwfUG3HorizontalDirection(UGraf,&hdir,True);
    XfwfUG3Aperture(UGraf,1.0,True);
}

int DrawStar(Point3 *o, Point3 *dx, Point3 *dy, int points, float size, 
	     Point3 *p, int *d, int max) {
    int n = 0;
    int i;
    double x, y;
    int pts = 2*points+1;
    double da = 2.0*M_PI*points/pts;
    double a = 0.0;

    /* Close the star */
    ++pts;

    /* No more than max points */
    if (pts>max) pts = max;

    /* Draw the star */
    for (i=0; i<pts; ++i) {
	x = cos(a);
	y = sin(a);
	p->x = dx->x * size * x + dy->x * size * y + o->x;
	p->y = dx->y * size * x + dy->y * size * y + o->y;
	p->z = dx->z * size * x + dy->z * size * y + o->z;
	if (i) d[i] = 1;
	else d[i] = 0;
	++p;
	a += da;
    }

    /* return the number of points in the star */
    return pts;
}

void UGraf3Test(Widget w, XEvent *event, String *argv, Cardinal *argc){

    In_3D_Mode = True;
    XfwfUGClear(UGraf);

    XfwfUGGray(UGraf,0.5);

    {				/* Set the draw space. */
	Rectangle space = {1.0, 1.0, 19.0, 19.0};
	Rectangle window = {-1.0, -1.0, 1.0, 1.0};
	XfwfUGWindow(UGraf,2,&space,&window);
    }

    /* Draw a cube */
    XfwfUG3Line(UGraf, 1.0, 1.0, 1.0,0);    XfwfUG3Line(UGraf,-1.0,-1.0, 1.0,1);
    XfwfUG3Line(UGraf, 1.0,-1.0, 1.0,1);    XfwfUG3Line(UGraf, 1.0, 1.0, 1.0,1);
    XfwfUG3Line(UGraf,-1.0, 1.0, 1.0,1);    XfwfUG3Line(UGraf,-1.0,-1.0,-1.0,1);
    XfwfUG3Line(UGraf,-1.0,-1.0, 1.0,1);    XfwfUG3Line(UGraf,-1.0, 1.0, 1.0,1);
    XfwfUG3Line(UGraf,-1.0, 1.0,-1.0,1);    XfwfUG3Line(UGraf, 1.0,-1.0,-1.0,1);
    XfwfUG3Line(UGraf,-1.0,-1.0,-1.0,1);    XfwfUG3Line(UGraf,-1.0, 1.0,-1.0,1);
    XfwfUG3Line(UGraf, 1.0, 1.0,-1.0,1);    XfwfUG3Line(UGraf, 1.0,-1.0, 1.0,1);
    XfwfUG3Line(UGraf, 1.0,-1.0,-1.0,1);    XfwfUG3Line(UGraf, 1.0, 1.0,-1.0,1);
    XfwfUG3Line(UGraf, 1.0, 1.0, 1.0,1);
    XfwfUGUpdate(UGraf);

    {				/* Draw a star. */
	Point3 o =  { 0.0, 0.0, 2.0};
	Point3 dx = { 1.0, 0.0, 0.0};
	Point3 dy = { 0.0, 1.0, 0.0};
	Point3 p[1000]; int d[1000];
	int i = DrawStar(&o,&dx,&dy,2,1.0,p,d,1000);
	XfwfUG3Lines(UGraf,p,d,i);
    }
    {				/* Draw a star. */
	Point3 o =  { 0.0, 0.0, -2.0};
	Point3 dx = { 1.0, 0.0, 0.0};
	Point3 dy = { 0.0, 1.0, 0.0};
	Point3 p[1000]; int d[1000];
	int i = DrawStar(&o,&dx,&dy,3,1.0,p,d,1000);
	XfwfUG3Lines(UGraf,p,d,i);
    }
    {				/* Draw a star. */
	Point3 o =  { 0.0, 2.0, 0.0};
	Point3 dx = { 1.0, 0.0, 0.0};
	Point3 dy = { 0.0, 0.0, 1.0};
	Point3 p[1000]; int d[1000];
	int i = DrawStar(&o,&dx,&dy,4,1.0,p,d,1000);
	XfwfUG3Lines(UGraf,p,d,i);
    }
    {				/* Draw a star. */
	Point3 o =  { 0.0, -2.0, 0.0};
	Point3 dx = { 1.0, 0.0, 0.0};
	Point3 dy = { 0.0, 0.0, 1.0};
	Point3 p[1000]; int d[1000];
	int i = DrawStar(&o,&dx,&dy,5,1.0,p,d,1000);
	XfwfUG3Lines(UGraf,p,d,i);
    }
    {				/* Draw a star. */
	Point3 o =  { 2.0, 0.0, 0.0};
	Point3 dx = { 0.0, 1.0, 0.0};
	Point3 dy = { 0.0, 0.0, 1.0};
	Point3 p[1000]; int d[1000];
	int i = DrawStar(&o,&dx,&dy,6,1.0,p,d,1000);
	XfwfUG3Lines(UGraf,p,d,i);
    }
    {				/* Draw a star. */
	Point3 o =  { -2.0, 0.0, 0.0};
	Point3 dx = { 0.0, 1.0, 0.0};
	Point3 dy = { 0.0, 0.0, 1.0};
	Point3 p[1000]; int d[1000];
	int i = DrawStar(&o,&dx,&dy,7,1.0,p,d,1000);
	XfwfUG3Lines(UGraf,p,d,i);
    }
    XfwfUGUpdate(UGraf);
}

void UGrafTest(Widget w, XEvent *event, String *argv, Cardinal *argc) {
    int i;
    Point3 p;

    In_3D_Mode = False;
    XfwfUGClear(UGraf);

    XfwfUGGray(UGraf,0.5);

    /* Test XfwfUGLine */
    XfwfUGCurrent(UGraf,1);
    XfwfUGLine(UGraf,0.0,0.0,0);
    XfwfUGLine(UGraf,1.0,1.0,1);
    XfwfUGLine(UGraf,0.0,1.0,1);
    XfwfUGLine(UGraf,1.0,0.0,1);

    XfwfUGUpdate(UGraf);

    {				/* Now Test a new transform. */
	Rectangle space = {1.0, 4.0, 6.0, 6.0};
	Rectangle window = {0.0, 0.0, 1.0, 1.0};
	XfwfUGWindow(UGraf,2,&space,&window);
	XfwfUGLine(UGraf,0.0,0.0,0);
	XfwfUGLine(UGraf,1.0,1.0,1);
	XfwfUGLine(UGraf,0.0,1.0,1);
	XfwfUGLine(UGraf,1.0,0.0,1);
    }

    XfwfUGUpdate(UGraf);

    {				/* Test UGLines */
	Rectangle space = {10.0, 10.0, 14.0, 16.0};
	Rectangle window = {0.0, 0.0, 1.0, 1.0};
	Point p[4] = {{0.0,0.0}, {1.0,1.0}, {0.0,1.0}, {1.0,0.0}};
	int d[4] = {0,1,1,1};
	XfwfUGWindow(UGraf,3,&space,&window);
	XfwfUGLines(UGraf,p,d,4);
    }

    XfwfUGUpdate(UGraf);

    {				/* Test XfwfUGFill */
	Point p[3] = {{0.0,1.0}, {1.0,1.0}, {0.5,0.5}};
	XfwfUGFill(UGraf,p,3);
    }

    XfwfUGUpdate(UGraf);

    {				/* Test the grays */
	float r,s=0.01;
	Rectangle space = {17.0, 1.0, 19.0, 19.0};
	Rectangle window = {0.0, 0.0, 1.0, 1.0};
	Point p[4];

	XfwfUGWindow(UGraf,3,&space,&window);

	for (r=0.0; r<1.0+0.5*s; r += s) {
	    p[0].x = p[3].x = 0.0;
	    p[1].x = p[2].x = 1.0;
	    p[0].y = p[1].y = r;
	    p[2].y = p[3].y = r+s;
	    XfwfUGGray(UGraf,r);
	    XfwfUGFill(UGraf,p,4);
	}
    }

    XfwfUGUpdate(UGraf);

    {				/* Test the colors */
	int i;
	Rectangle space = {15.0, 1.0, 17.0, 19.0};
	Rectangle window = {0.0, 0.0, 1.0, 16.0};
	Point p[4];

	XfwfUGWindow(UGraf,3,&space,&window);

	for (i=0; i<16; ++i) {
	    p[0].x = p[3].x = 0.0;
	    p[1].x = p[2].x = 1.0;
	    p[0].y = p[1].y = i;
	    p[2].y = p[3].y = i+1;
	    XfwfUGColor(UGraf,i);
	    XfwfUGFill(UGraf,p,4);
	}
    }

    XfwfUGUpdate(UGraf);

    {				/* Test the shades */
	int i;
	double h, hs = 0.02, s, ss=0.062;
	float H,S;
	Rectangle space = {7.5, 12.0, 14.5, 19.0};
	Rectangle window = {-1.0, -1.0, 1.0, 1.0};
	Point p[3];
	
	XfwfUGWindow(UGraf,3,&space,&window);
	
	for (h=0.0; h<1.0; h+=hs) {
	    for (s=0.0; s<1.0; s+=ss) {
		H = h;
		S = 1.0-s;
		S = sqrt(S);
		p[0].x = p[0].y = 0.0;
		p[1].x = S*sin(2.0*3.14159*H);
		p[1].y = S*cos(2.0*3.14159*H);
		p[2].x = S*sin(2.0*3.14159*(H+hs));
		p[2].y = S*cos(2.0*3.14159*(H+hs));
		XfwfUGShade(UGraf,h,s);
		XfwfUGFill(UGraf,p,3);
	    }
	}
    }

    XfwfUGUpdate(UGraf);

    {				/* Test XfwfUGMark. */
	Rectangle space = {6.5, 1.0, 15.5, 10.0};
	Rectangle window = {0.0, 0.0, 13.0, 13.0};
	Point o = {0.0, 0.0};
	Point p[1000];
	int d[1000], npts=0;
	int i, j;
	XfwfUGWindow(UGraf,2,&space,&window);
	XfwfUGCurrent(UGraf,2);
	XfwfUGGray(UGraf,0.1);
	for (j = 0; j<13; ++j) {
	    for (i=0; i<13; ++i) {
		o.x = i;
		o.y = j;
		XfwfUGMark(UGraf,o.x,o.y,i+j*13,0.5);
	    }
	}
    }

    {				/* Test XfwfUGText */
	Rectangle space = {1.0, 1.0, 19.0, 19.0};
	Rectangle window = {0.0, 0.0, 15.0, 32.0};
	int fam, fnt;
	XfwfUGWindow(UGraf,2,&space,&window);
	XfwfUGColor(UGraf,4);
	XfwfUGFontDefault(UGraf);
	XfwfUGFontSize(UGraf,0.6);
	for (fam=0; fam<8; ++fam) {
	    for(fnt=0; fnt<4; ++fnt) {
		float x, y;
		x = 0.0;
		y = 4.0*fam + fnt;
		XfwfUGFontFamily(UGraf,fam);
		XfwfUGFontType(UGraf,fnt);
		XfwfUGText(UGraf,x,y,"abcdefg ABCDEFG");
	    }
	}
    }

    XfwfUGCurrent(UGraf,0);
    XfwfUGColor(UGraf,2);
    XfwfUGFontDefault(UGraf);
    XfwfUGFontSize(UGraf,2.0);
    XfwfUGFontAngle(UGraf,-45.0);
    XfwfUGFontType(UGraf,ROMAN);
    XfwfUGFontFamily(UGraf,TRIPLEX);
    XfwfUGFontJustification(UGraf,CENTER_JUSTIFICATION);
    XfwfUGText(UGraf,10.0,10.0,"Angle Test");

    XfwfUGUpdate(UGraf);
}

XtActionsRec Actions[] = {
  {"quit-application",	Quit_Application},
  {"test-ugraf",	UGrafTest},
  {"test-ugraf3",	UGraf3Test},
  {"view0",		View0},
  {"view1",		View1},
  {"view2",		View2},
};

static char *fallback_resources[] = {
  "XDraw*translations: #override \\\n\
<Key>q: quit-application()\\n\\\n\
<Key>0:  view0()\\n\\\n\
<Key>1:  view1()\\n\\\n\
<Key>2:  view2()\\n\\\n\
<Key>3:  test-ugraf3()\\n\\\n\
<Key>t:  test-ugraf()\n",
  NULL
};

main (argc, argv)
int argc;
char **argv;
{
  Widget top_level;
  Widget top_shell;
  Widget board;
  Widget stop;

  top_level = XtAppInitialize(&app_con,"XDraw",NULL,0,
			  (Cardinal*)&argc,argv,fallback_resources,NULL,0);

  XtAppAddActions(app_con,Actions,XtNumber(Actions));

  board = XtVaCreateManagedWidget((String)"board",xfwfBoardWidgetClass,
				  top_level,
				  (String)XtNheight,(XtArgVal)100,
				  (String)XtNwidth,(XtArgVal)100,
				  NULL);

  stop =  XtVaCreateManagedWidget((String)"stop",xfwfButtonWidgetClass,
				  board,
				  XtNlocation,"0.1 0.1 0.8 0.8",
				  XtNlabel,"stop",
				  NULL);

  XtAddCallback(stop,XtNactivate,Quit_Application,NULL);
  
  top_shell = XtVaAppCreateShell((String)"UDraw",(String)"udraw",
				 applicationShellWidgetClass,
				 XtDisplay(top_level),
				 NULL);
  
  UGraf = (UGrafWidget) XtVaCreateManagedWidget((String)"UGraf", 
					   xfwfUGrafWidgetClass,
					   top_shell,
					   XtNheight,400,
					   XtNwidth,400,
					   NULL);

  XtAddCallback((Widget) UGraf,XtNupdateCallback,UpdateCallback,NULL);
  
  XtRealizeWidget(top_level);
  XtRealizeWidget(top_shell);

  XtAppMainLoop(app_con);
}

void Quit_Application(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
  XtDestroyApplicationContext(app_con);
  printf("Goodbye!\n");
  exit(0);
}
