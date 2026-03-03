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

#ifndef DEBUG
#define DEBUG(s)  
#endif
static void Update(pw,event,params,num_params)
UGrafWidget pw;
XEvent *event;
String *params;
Cardinal num_params;
{
    XtCallCallbacks((Widget) pw,XtNupdateCallback,NULL);
    XfwfUGUpdate(pw);
}

static void PointerLocation(pw,event,params,num_params)
UGrafWidget pw;
XEvent *event;
String *params;
Cardinal num_params;
{
    XfwfUGLocationCallbackReturn location;

    /* Save the point for posterity */
    UGrafX(pw)->pointer_location.x = event->xbutton.x;
    UGrafX(pw)->pointer_location.y = event->xbutton.y;
    UGrafX(pw)->pointer_valid = True;
    /* Pass the xpoint */
    location.xpoint.x = event->xbutton.x;
    location.xpoint.y = event->xbutton.y;
    /* Make it into a current window point */
    XfwfUGFromXPoint(&location.xpoint,&location.point,
		     &(UGrafCurrent(pw)->trans));
    /* If possible return the 3d point */
    location.valid3 = XfwfUG3FromPoint(&location.point,&location.point3,
				       UGrafCurrent3(pw));
    /* Now do the callback. */
    XtCallCallbacks((Widget) pw,XtNlocationCallback,&location);
}

static void check_steps(w)
     UGrafWidget w;
 {
    if (UGrafTrans3(w)->step_size<0.001) UGrafTrans3(w)->step_size = 0.001;
    if (UGrafTrans3(w)->step_dir!=1.0
	&& UGrafTrans3(w)->step_dir!=-1.0) UGrafTrans3(w)->step_dir = 1.0;
    if (UGrafTrans3(w)->step_angle<0.01) UGrafTrans3(w)->step_size = 0.01;
}

static void Wider(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w;
    float app;
    DEBUG(("Wider\n"));
    app = XfwfUG3Aperture(pw,app,False);
    app = XfwfUG3Aperture(pw,app*1.5,True);
}

static void Narrower(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w;
    float app;
    DEBUG(("Narrower\n"));
    app = XfwfUG3Aperture(pw,app,False);
    app = XfwfUG3Aperture(pw,app/1.5,True);
}

static void MOVEIT(p, l, d)
     Point3 *p;
     float l;
     Point3 *d;
 {
    (p)->x += (l)*(d)->x;
    (p)->y += (l)*(d)->y;
    (p)->z += (l)*(d)->z;
}

static void Move(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; 
    Point3 p, d;
    DEBUG(("Move %f\n",UGrafTrans3(pw)->step_size));
    XfwfUG3Reference(pw,&p,False);
    XfwfUG3ViewDirection(pw,&d,False);
    MOVEIT(&p,UGrafTrans3(pw)->step_dir*UGrafTrans3(pw)->step_size,&d);
    XfwfUG3Reference(pw,&p,True);
}
static void Reverse(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; 
    DEBUG(("Reverse\n"));
    UGrafTrans3(pw)->step_dir = - UGrafTrans3(pw)->step_dir;
}
static void IncreaseStep(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; 
    DEBUG(("IncreaseStep\n"));
    UGrafTrans3(pw)->step_size *= 1.4;
    UGrafTrans3(pw)->step_angle *= 1.4;
}
static void DecreaseStep(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; 
    DEBUG(("DecreaseStep\n"));
    UGrafTrans3(pw)->step_size /= 1.4;
    UGrafTrans3(pw)->step_angle /= 1.4;
}
static void Up(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; Point3 p, d;
    DEBUG(("Up\n"));
    XfwfUG3Reference(pw,&p,False);
    XfwfUG3UpDirection(pw,&d,False);
    MOVEIT(&p,UGrafTrans3(pw)->step_size,&d);
    XfwfUG3Reference(pw,&p,True);
}
static void Down(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; Point3 p, d;
    DEBUG(("Down\n"));
    XfwfUG3Reference(pw,&p,False);
    XfwfUG3UpDirection(pw,&d,False);
    MOVEIT(&p,-UGrafTrans3(pw)->step_size,&d);
    XfwfUG3Reference(pw,&p,True);
}
static void Left(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; Point3 p, d;
    DEBUG(("Left\n"));
    XfwfUG3Reference(pw,&p,False);
    XfwfUG3HorizontalDirection(pw,&d,False);
    MOVEIT(&p,-UGrafTrans3(pw)->step_size,&d);
    XfwfUG3Reference(pw,&p,True);
}
static void Right(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; Point3 p, d;
    DEBUG(("Right\n"));
    XfwfUG3Reference(pw,&p,False);
    XfwfUG3HorizontalDirection(pw,&d,False);
    MOVEIT(&p,UGrafTrans3(pw)->step_size,&d);
    XfwfUG3Reference(pw,&p,True);
}

static void DirUp(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; Point3 d,u;
    DEBUG(("DirUp\n"));
    XfwfUG3ViewDirection(pw,&d,False);
    XfwfUG3UpDirection(pw,&u,False);
    MOVEIT(&d,UGrafTrans3(pw)->step_angle,&u);
    XfwfUG3ViewDirection(pw,&d,True);
}
static void DirDown(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; Point3 d,u;
    DEBUG(("DirDown\n"));
    XfwfUG3ViewDirection(pw,&d,False);
    XfwfUG3UpDirection(pw,&u,False);
    MOVEIT(&d,-UGrafTrans3(pw)->step_angle,&u);
    XfwfUG3ViewDirection(pw,&d,True);
}
static void DirRight(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; Point3 d,h;
    DEBUG(("DirRight\n"));
    XfwfUG3ViewDirection(pw,&d,False);
    XfwfUG3HorizontalDirection(pw,&h,False);
    MOVEIT(&d,UGrafTrans3(pw)->step_angle,&h);
    XfwfUG3ViewDirection(pw,&d,True);
}
static void DirLeft(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; Point3 d,h;
    DEBUG(("DirLeft\n"));
    XfwfUG3ViewDirection(pw,&d,False);
    XfwfUG3HorizontalDirection(pw,&h,False);
    MOVEIT(&d,-UGrafTrans3(pw)->step_angle,&h);
    XfwfUG3ViewDirection(pw,&d,True);
}
static void Clockwise(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; Point3 u,h;
    DEBUG(("Clockwise\n"));
    XfwfUG3UpDirection(pw,&u,False);
    XfwfUG3HorizontalDirection(pw,&h,False);
    MOVEIT(&u,UGrafTrans3(pw)->step_angle,&h);
    XfwfUG3UpDirection(pw,&u,True);
}
static void CounterClockwise(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;
{
    UGrafWidget pw = (UGrafWidget) w; Point3 u,h;
    DEBUG(("CounterClockwise\n"));
    XfwfUG3UpDirection(pw,&u,False);
    XfwfUG3HorizontalDirection(pw,&h,False);
    MOVEIT(&u,-UGrafTrans3(pw)->step_angle,&h);
    XfwfUG3UpDirection(pw,&u,True);
}

