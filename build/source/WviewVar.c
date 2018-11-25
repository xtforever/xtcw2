/* Generated by wbuild
 * (generator version 3.3)
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#line 63 "WviewVar.widget"
#include "xutil.h"
#line 64 "WviewVar.widget"
#include "micro_vars.h"
#include <xtcw/WviewVarP.h>
static void _resolve_inheritance(
#if NeedFunctionPrototypes
WidgetClass
#endif
);
#line 11 "WviewVar.widget"
static void initialize(
#if NeedFunctionPrototypes
Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 19 "WviewVar.widget"
static Bool  cache_hit(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 31 "WviewVar.widget"
static void do_update(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 37 "WviewVar.widget"
static void show_data(
#if NeedFunctionPrototypes
Widget,void *,void *
#endif
);
#line 31 "WviewVar.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 31 "WviewVar.widget"
static void do_update(Widget self)
#else
#line 31 "WviewVar.widget"
static void do_update(self)Widget self;
#endif
#line 32 "WviewVar.widget"
{
        XtVaSetValues( self, "update", ((WviewVarWidget)self)->wlabel.update+1, NULL );
}
#line 37 "WviewVar.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 37 "WviewVar.widget"
static void show_data(Widget self,void * a,void * b)
#else
#line 37 "WviewVar.widget"
static void show_data(self,a,b)Widget self;void * a;void * b;
#endif
#line 38 "WviewVar.widget"
{

        int wh,val,h,x,y;
        char *s, str[30];
        val = *mv_var( ((WviewVarWidget)self)->wviewVar.qvar );
        sprintf(str,"%d", val );
        h = ((WviewVarWidget)self)->wheel.xftFont->height;
        wh = ((WviewVarWidget)self)->core.height / 2;
        x = 0;
        y = (wh-h) / 2 + ((WviewVarWidget)self)->wheel.xftFont->ascent;
        s = ((WviewVarWidget)self)->wlabel.label;
        XftDrawStringUtf8(((WviewVarWidget)self)->wlabel.draw,((WviewVarWidget)self)->wheel.xft_col + ((WviewVarWidget)self)->wheel.state, ((WviewVarWidget)self)->wheel.xftFont,
                                         x,y,(FcChar8*)s, strlen(s) );


        y += wh;
        s = str;
        x = ((WviewVarWidget)self)->core.width - 2 - xft_textwidth(XtDisplay(self),((WviewVarWidget)self)->wheel.xftFont, s );
        XftDrawStringUtf8(((WviewVarWidget)self)->wlabel.draw,((WviewVarWidget)self)->wheel.xft_col + ((WviewVarWidget)self)->wheel.state, ((WviewVarWidget)self)->wheel.xftFont,
                                         x,y,(FcChar8*)s, strlen(s) );
        ((WviewVarWidget)self)->wviewVar.drawn = val;

}

static XtResource resources[] = {
#line 4 "WviewVar.widget"
{XtNvarname,XtCVarname,XtRString,sizeof(((WviewVarRec*)NULL)->wviewVar.varname),XtOffsetOf(WviewVarRec,wviewVar.varname),XtRString,(XtPointer)""},
};

WviewVarClassRec wviewVarClassRec = {
{ /* core_class part */
/* superclass   	*/  (WidgetClass) &wlabelClassRec,
/* class_name   	*/  "WviewVar",
/* widget_size  	*/  sizeof(WviewVarRec),
/* class_initialize 	*/  NULL,
/* class_part_initialize*/  _resolve_inheritance,
/* class_inited 	*/  FALSE,
/* initialize   	*/  initialize,
/* initialize_hook 	*/  NULL,
/* realize      	*/  XtInheritRealize,
/* actions      	*/  NULL,
/* num_actions  	*/  0,
/* resources    	*/  resources,
/* num_resources 	*/  1,
/* xrm_class    	*/  NULLQUARK,
/* compres_motion 	*/  False ,
/* compress_exposure 	*/  TRUE ,
/* compress_enterleave 	*/  False ,
/* visible_interest 	*/  False ,
/* destroy      	*/  NULL,
/* resize       	*/  XtInheritResize,
/* expose       	*/  XtInheritExpose,
/* set_values   	*/  NULL,
/* set_values_hook 	*/  NULL,
/* set_values_almost 	*/  XtInheritSetValuesAlmost,
/* get_values+hook 	*/  NULL,
/* accept_focus 	*/  XtInheritAcceptFocus,
/* version      	*/  XtVersion,
/* callback_private 	*/  NULL,
/* tm_table      	*/  NULL,
/* query_geometry 	*/  XtInheritQueryGeometry,
/* display_acceleator 	*/  XtInheritDisplayAccelerator,
/* extension    	*/  NULL 
},
{ /* Wheel_class part */
XtInherit_exec_command,
XtInherit_sig_recv,
},
{ /* Wlabel_class part */
cache_hit,
},
{ /* WviewVar_class part */
 /* dummy */  0
},
};
WidgetClass wviewVarWidgetClass = (WidgetClass) &wviewVarClassRec;
static void _resolve_inheritance(class)
WidgetClass class;
{
  WviewVarWidgetClass c __attribute__((unused)) = (WviewVarWidgetClass) class;
  WviewVarWidgetClass super __attribute__((unused));
  if (class == wviewVarWidgetClass) return;
  super = (WviewVarWidgetClass)class->core_class.superclass;
}
#line 11 "WviewVar.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 11 "WviewVar.widget"
static void initialize(Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 11 "WviewVar.widget"
static void initialize(request,self,args,num_args)Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 12 "WviewVar.widget"
{
        ((WviewVarWidget)self)->wviewVar.qvar = XrmStringToQuark( ((WviewVarWidget)self)->wviewVar.varname );
        ((WviewVarWidget)self)->wlabel.draw_override = show_data;
        mv_onwrite( ((WviewVarWidget)self)->wviewVar.qvar, do_update, self, 0 );
        ((WviewVarWidget)self)->core.height = (((WviewVarWidget)self)->wheel.xftFont->height) * 2.5;
}
#line 19 "WviewVar.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 19 "WviewVar.widget"
static Bool  cache_hit(Widget self)
#else
#line 19 "WviewVar.widget"
static Bool  cache_hit(self)Widget self;
#endif
#line 20 "WviewVar.widget"
{
        int val = *mv_var( ((WviewVarWidget)self)->wviewVar.qvar );
        Bool hit = (((WviewVarWidget)self)->wviewVar.drawn == val);
        hit = hit && wlabelClassRec.wlabel_class.cache_hit(self);
        return hit;
}
