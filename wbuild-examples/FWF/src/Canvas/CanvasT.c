#include	<signal.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<X11/Intrinsic.h>
#include	<X11/StringDefs.h>
#include	<X11/Shell.h>
#include	<Xfwf/Canvas.h>

#define MAXSEGMENTS 128

Display	*disp;

Widget	outer,
  quitbutton,canvas;


struct XDrawresources{
  char		*servername;
  Pixel		color[3];
  Dimension	linewidth;
} xdrawresources;

XtAppContext	app_con;

struct LineData{
  unsigned	x1,y1,x2,y2;
  int		color;
} listoflines[MAXSEGMENTS];
int	listlength;

Canvas_Expose(w, event, exposed, client_data)
     Widget w;
     XEvent *event;
     Region exposed;
     void *client_data;

{
  Window	win;
  GC	tgc;
  int	i;
  XSegment lines[3][MAXSEGMENTS];
  int	count[3];

  win = XtWindow(w);
  tgc = XCreateGC(disp,win, 0,NULL);
  XSetRegion(disp,tgc,exposed);
  XSetLineAttributes(disp,tgc,xdrawresources.linewidth,
		     LineSolid,CapRound,JoinBevel);

  for (i=0; i<3; i++)
    count[i] = 0;

  for (i=0; i<listlength; i++)
    {
      register int	tc=listoflines[i].color;
			/* ^^^ the color of this line */
      XSegment		*xsp;

      xsp = &lines[tc][count[tc]];
      xsp->x1 = listoflines[i].x1;
      xsp->y1 = listoflines[i].y1;
      xsp->x2 = listoflines[i].x2;
      xsp->y2 = listoflines[i].y2;
      count[tc]++;
    }
  for (i=0; i<3; i++)
    {
      XSetForeground(disp,tgc,xdrawresources.color[i]);
      XDrawSegments(disp,win,tgc,lines[i],count[i]);
    }
}

setup_widgets()
{
  canvas = XtVaCreateManagedWidget
    ((String)"canvas",xfwfcanvasWidgetClass,outer,
     (String)"exposeProc",(XtArgVal)Canvas_Expose,
     (String)"exposeProcData",(XtArgVal)(int)NULL,
     (String)XtNheight,(XtArgVal)250,
     (String)XtNwidth,(XtArgVal)210,
     0);
}

void Quit_Application();
void StartDraw();
void StopDraw();

static XrmOptionDescRec Options[] = {
  {"-c1",	"color1",	XrmoptionSepArg,	NULL},
  {"-c2",	"color2",	XrmoptionSepArg,	NULL},
  {"-c3",	"color3",	XrmoptionSepArg,	NULL},
  {"-lw",	"linewidth",	XrmoptionStickyArg,	NULL},
};

#define offset(field) XtOffsetOf(struct XDrawresources,field)
static XtResource Resources[] = {
  {"color1", "Color", XtRPixel, sizeof(Pixel), offset(color[0]),
     XtRString, (XtPointer)"blue"},
  {"color2", "Color", XtRPixel, sizeof(Pixel), offset(color[1]),
     XtRString, (XtPointer)"green"},
  {"color3", "Color", XtRPixel, sizeof(Pixel), offset(color[2]),
     XtRString, (XtPointer)"forestgreen"},
  {"linewidth","Width",XtRDimension,sizeof(Dimension),offset(linewidth),
     XtRString, (String)"5"},
};

XtActionsRec Actions[] = {
  {"start-draw",	StartDraw},
  {"stop-draw",		StopDraw},
  {"quit-application",	Quit_Application},
};

static char *fallback_resources[] = {
  "XDraw*translations: #override \\\n\
<BtnDown>:start-draw()\\n\\\n\
<BtnUp>:stop-draw()\\n\\\n\
<Key>q:quit-application()\n",
/*  "XDraw*color1:violet",*/
  NULL
};

main (argc, argv)
     int argc;
     char **argv;

{
  xdrawresources.color[0] = 1;
  outer = XtAppInitialize(&app_con,"XDraw",Options,XtNumber(Options),
			  (Cardinal*)&argc,argv,fallback_resources,NULL,0);
  XtGetApplicationResources(outer,&xdrawresources,Resources,
			    XtNumber(Resources),NULL,0);
  XtAppAddActions(app_con,Actions,XtNumber(Actions));

  setup_widgets();

  XtRealizeWidget(outer);
  disp = XtDisplay(outer);

  XtAppMainLoop(app_con);
}


int	xstart,ystart,button;
int	drawing=0;

void StartDraw(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;

{
  register XButtonEvent *ev;
  fflush(stdout);
  ev = &event->xbutton;
  xstart = ev->x;
  ystart = ev->y;
  button = ev->button;
  drawing=1;
}


void StopDraw(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;

{
  if (drawing)
    {
      Window	win;
      GC	tgc;
      struct LineData *ld;
      register XButtonEvent *ev;

      win = XtWindow(w);
      tgc = XCreateGC(disp,win, 0,NULL);
      ev = &event->xbutton;
      ld = listoflines + listlength++;
      ld->x1 = xstart;
      ld->y1 = ystart;
      ld->x2 = ev->x;
      ld->y2 = ev->y;
      ld->color = button-1;
      drawing = 0;
      XSetForeground(disp,tgc,xdrawresources.color[ld->color]);
      XSetLineAttributes(disp,tgc,xdrawresources.linewidth,
			 LineSolid,CapRound,JoinBevel);
      XDrawLine(disp,win,tgc,ld->x1,ld->y1,ld->x2,ld->y2);
    }
}


void Quit_Application(w, event, argv, argc)
     Widget w;
     XEvent *event;
     String *argv;
     Cardinal *argc;

{
  printf("Goodbye!\n");
  XtDestroyApplicationContext(app_con);
  exit(0);
}
