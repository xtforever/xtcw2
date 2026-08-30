#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <Xfwf/SpinLabel.h>
#include <Xfwf/Button.h>

static String res[]= {
	"*foreground:			MediumBlue",
	"*background:			Gray80",
/*	"*XfwfSpinLabel.labelframeWidth:	1", */
/*	"*XfwfSpinLabel.frameWidth:		2", */
	
	"*board.x:				100",
	"*board.y				50",
	"*board.width:			400",
	"*board.height:			300",

	"*spin0.location:		0.05	0.15	0.40	0.12",
	"*spin0.alignment:		left",
	"*spin0.frameType:		sunken",
	"*spin0.frameWidth:		2",
	
	"*spin1.location:		0.05	0.30	0.40	0.12",
	"*spin1.alignment:		right",
	"*spin1.frameWidth:		2",
	
	"*spin2.location:		0.05	0.45	0.40	0.12",
	"*spin2.labelframeWidth:	2",
	"*spin2.arrowForeground:HotPink",
	
	"*spin3.location:		0.55	0.15	0.40	0.12",
	"*spin3.alignment:		left",
	"*spin3.horizontal:		False",
	"*spin3.frameWidth:		2", 
	"*spin3.frameType:		raised",
	
	"*spin4.location:		0.55	0.30	0.40	0.12",
	"*spin4.alignment:		right",
	"*spin4.horizontal:		False",
	"*spin4.labelframeWidth:	1",
	
	"*spin5.location:		0.55	0.45	0.20	0.30",
	"*spin5.horizontal:		False",
	"*spin5.labelframeType:	raised",
	"*spin5.labelframeWidth:	2",
	
	"*quit.label:			Quit",
	"*quit.location:		0.40	0.85	0.22	0.10",
	NULL
};
	
#define MAX			12

static int ndx[6]={0,0,0,0,0,0};
static String list[MAX]= {
	"January","February","March","April","May","June","July",
	"August","September","Oktober","November","Dezember"
};
static Widget spin[6];

void SpinCB(Widget w,XtPointer dclient,XtPointer dcall)
{
	SpinType diff=(SpinType) dcall;
	int i;

	for (i=0; spin[i]!=(Widget)dclient; i++) ;
	
	fprintf(stderr,"SpinCB(%s,%d - %d) called.\n",
		XtName(w),i,(int) dcall);
	if(diff == XfwfFirst && ndx[i] > 0)
		ndx[i]=0;
	else if(diff == XfwfPrev && ndx[i] > 0)
		ndx[i]--;
	else if(diff == XfwfNext && ndx[i] < MAX-1)
		ndx[i]++;
	else if(diff == XfwfLast && ndx[i] < MAX-1)
		ndx[i]=MAX-1;
	XtVaSetValues( (Widget) dclient,XtNlabel,list[ndx[i]],NULL);
}

void QuitCB(Widget w,XtPointer dclient,XtPointer dcall)
{
	fprintf(stderr,"have a nice day.\n");
	exit(0);
}


int main(int ac,char **av)
{
	XtAppContext app_context;
	Widget top,board,quit;
	String str[]= {"spin0","spin1","spin2","spin3","spin4","spin5"};
	int i;
		
	top=XtVaAppInitialize(&app_context,"SpinLabelT",NULL,0,&ac,av,res,NULL);
	board=XtVaCreateManagedWidget("board",xfwfBoardWidgetClass,top,NULL);

	for(i=0;i < 6;i++)
	{
		spin[i]=XtVaCreateWidget(str[i],xfwfSpinLabelWidgetClass,board,\
							 XtNlabel,list[ndx[i]],NULL);
		XtAddCallback(spin[i],XtNcallback,SpinCB,spin[i]);
	}
	XtManageChildren(spin,6);
	
	quit=XtVaCreateManagedWidget("quit",xfwfButtonWidgetClass,board,NULL);
	XtAddCallback(quit,XtNactivate,QuitCB,NULL);
	
	XtRealizeWidget(top);
	XtAppMainLoop(app_context);
}
