/* Generated by wbuild
 * (generator version 3.3)
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#line 320 "WpixBtn.widget"
#include <X11/Xft/Xft.h>
#line 321 "WpixBtn.widget"
#include <X11/Xmu/Converters.h>
#line 322 "WpixBtn.widget"
#include "converters.h"
#line 323 "WpixBtn.widget"
#include "xutil.h"
#line 324 "WpixBtn.widget"
#include "mls.h"
#include <xtcw/WpixBtnP.h>
#line 18 "WpixBtn.widget"
static void next_pixmap(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);
#line 29 "WpixBtn.widget"
static void highlight(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);
#line 36 "WpixBtn.widget"
static void reset(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);
#line 43 "WpixBtn.widget"
static void notify(
#if NeedFunctionPrototypes
Widget,XEvent*,String*,Cardinal*
#endif
);

static XtActionsRec actionsList[] = {
{"next_pixmap", next_pixmap},
{"highlight", highlight},
{"reset", reset},
{"notify", notify},
};

static char defaultTranslations[] = "\
<BtnDown>: notify() \n\
";
static void _resolve_inheritance(
#if NeedFunctionPrototypes
WidgetClass
#endif
);
#line 70 "WpixBtn.widget"
static int  exec_command(
#if NeedFunctionPrototypes
Widget,int ,int 
#endif
);
#line 81 "WpixBtn.widget"
static void class_initialize(
#if NeedFunctionPrototypes
void
#endif
);
#line 87 "WpixBtn.widget"
static void initialize(
#if NeedFunctionPrototypes
Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 110 "WpixBtn.widget"
static void realize(
#if NeedFunctionPrototypes
Widget,XtValueMask *,XSetWindowAttributes *
#endif
);
#line 119 "WpixBtn.widget"
static void resize(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 128 "WpixBtn.widget"
static void destroy(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 136 "WpixBtn.widget"
static void expose(
#if NeedFunctionPrototypes
Widget,XEvent *,Region 
#endif
);
#line 168 "WpixBtn.widget"
static Boolean  set_values(
#if NeedFunctionPrototypes
Widget ,Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 191 "WpixBtn.widget"
static void unloadPixmaps(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 205 "WpixBtn.widget"
static void parsePixmapNames(
#if NeedFunctionPrototypes
Widget,int *,int *
#endif
);
#line 227 "WpixBtn.widget"
static void compute_layout(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 243 "WpixBtn.widget"
static int  skip(
#if NeedFunctionPrototypes
char **
#endif
);
#line 263 "WpixBtn.widget"
static char * xstrsep(
#if NeedFunctionPrototypes
char **
#endif
);
#line 285 "WpixBtn.widget"
static int  load_pix(
#if NeedFunctionPrototypes
Widget,char *,pix_t *
#endif
);
#line 308 "WpixBtn.widget"
static void reshape_widget(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 191 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 191 "WpixBtn.widget"
static void unloadPixmaps(Widget self)
#else
#line 191 "WpixBtn.widget"
static void unloadPixmaps(self)Widget self;
#endif
#line 192 "WpixBtn.widget"
{
  int i; pix_t *p;
  m_foreach( ((WpixBtnWidget)self)->wpixBtn.m_pix, i, p ) {
  	     XFreePixmap( XtDisplay(self), p->map );
	     XFreePixmap( XtDisplay(self), p->mask );
  }
  m_clear( ((WpixBtnWidget)self)->wpixBtn.m_pix ); /* set array size to zero */
}
#line 205 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 205 "WpixBtn.widget"
static void parsePixmapNames(Widget self,int * wp,int * hp)
#else
#line 205 "WpixBtn.widget"
static void parsePixmapNames(self,wp,hp)Widget self;int * wp;int * hp;
#endif
#line 206 "WpixBtn.widget"
{
  pix_t		pix;
  char		*fn, *p;
  unloadPixmaps(self);
  if(! ((WpixBtnWidget)self)->wpixBtn.pixmapNames  ) return;
  if(! *((WpixBtnWidget)self)->wpixBtn.pixmapNames ) { ((WpixBtnWidget)self)->wpixBtn.pixmapNames=0; return; }

  if( ((WpixBtnWidget)self)->wpixBtn.name_cache ) free(((WpixBtnWidget)self)->wpixBtn.name_cache);
  ((WpixBtnWidget)self)->wpixBtn.name_cache = strdup(((WpixBtnWidget)self)->wpixBtn.pixmapNames); /* make sure we can modifiy memory */

  fn=p=((WpixBtnWidget)self)->wpixBtn.pixmapNames;
  *wp=*hp=0;
  while( (fn=xstrsep(&p)) ) {
      if( !load_pix( self,fn, &pix ) ) {
	if( pix.pm_rect.width >  *wp ) *wp=pix.pm_rect.width;
	if( pix.pm_rect.height > *hp ) *hp=pix.pm_rect.height;
	m_put( ((WpixBtnWidget)self)->wpixBtn.m_pix, &pix );
      }
  }
}
#line 227 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 227 "WpixBtn.widget"
static void compute_layout(Widget self)
#else
#line 227 "WpixBtn.widget"
static void compute_layout(self)Widget self;
#endif
#line 228 "WpixBtn.widget"
{
  /* center images */
  int i;
  pix_t *p;
  m_foreach( ((WpixBtnWidget)self)->wpixBtn.m_pix, i, p ) {
    p->pm_rect.x = (((WpixBtnWidget)self)->core.width  - p->pm_rect.width) / 2;
    p->pm_rect.y = (((WpixBtnWidget)self)->core.height - p->pm_rect.height) / 2;

    TRACE(2,"%s (%dx%d+%d+%d)", p->fileName, p->pm_rect.width, p->pm_rect.height,
    p->pm_rect.x, p->pm_rect.y );
  }
}
#line 243 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 243 "WpixBtn.widget"
static int  skip(char ** p)
#else
#line 243 "WpixBtn.widget"
static int  skip(p)char ** p;
#endif
#line 244 "WpixBtn.widget"
{
  char ch;
  if( p==0 || *p==0 ) return -1;
  while(1) {
    ch = **p;
    if( ch == 0 ) return -1;
    if( ! isspace(ch) && ch != ',' ) return 0;
    (*p)++;
  }
  return -1; /* never reached */
}
#line 263 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 263 "WpixBtn.widget"
static char * xstrsep(char ** p)
#else
#line 263 "WpixBtn.widget"
static char * xstrsep(p)char ** p;
#endif
#line 264 "WpixBtn.widget"
{
  char ch;
  char *start;

  if( skip(p) ) return NULL;
  start = *p;

  while(1) {
    ch = **p;
    if( ch == 0 ) break;
    if( isspace(ch) || ch == ',' ) {
      **p=0; (*p)++;
      break;
    }
    (*p)++;
  }
  return start;
}
#line 285 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 285 "WpixBtn.widget"
static int  load_pix(Widget self,char * name,pix_t * p)
#else
#line 285 "WpixBtn.widget"
static int  load_pix(self,name,p)Widget self;char * name;pix_t * p;
#endif
#line 286 "WpixBtn.widget"
{
  Display *dpy = XtDisplay(self);
  unsigned int width, height, bw, depth;
  Window win;
  int x,y;

  TRACE(2,"%s try loading %s\n",((WpixBtnWidget)self)->core.name, name );

  if( load_pixmap_from_file(self, name, & p->map, & p->mask) )
    return -1;

  if (! XGetGeometry(dpy, p->map, &win, &x, &y, &width, &height, &bw, &depth))
    return -1;

  /* printf("loading success %s\n", name ); */
  p->fileName = name;
  p->pm_rect.width  = (unsigned short) width;
  p->pm_rect.height = (unsigned short) height;
  p->pm_depth = depth;
  return 0;
}
#line 308 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 308 "WpixBtn.widget"
static void reshape_widget(Widget self)
#else
#line 308 "WpixBtn.widget"
static void reshape_widget(self)Widget self;
#endif
#line 309 "WpixBtn.widget"
{
	int w;
	if( ((WpixBtnWidget)self)->wpixBtn.cornerRoundPercent >0 && ((WpixBtnWidget)self)->wpixBtn.cornerRoundPercent < 100 ) {
	    w = Min(((WpixBtnWidget)self)->core.height,((WpixBtnWidget)self)->core.width);
	    w = w * ((WpixBtnWidget)self)->wpixBtn.cornerRoundPercent / 100;
	    XmuReshapeWidget( self, XmuShapeRoundedRectangle, w, w );
	}
}

static XtResource resources[] = {
#line 50 "WpixBtn.widget"
{XtNpixmapNames,XtCPixmapNames,XtRString,sizeof(((WpixBtnRec*)NULL)->wpixBtn.pixmapNames),XtOffsetOf(WpixBtnRec,wpixBtn.pixmapNames),XtRString,(XtPointer)NULL },
#line 51 "WpixBtn.widget"
{XtNvalue,XtCValue,XtRInt,sizeof(((WpixBtnRec*)NULL)->wpixBtn.value),XtOffsetOf(WpixBtnRec,wpixBtn.value),XtRImmediate,(XtPointer)0 },
#line 52 "WpixBtn.widget"
{XtNcornerRoundPercent,XtCCornerRoundPercent,XtRInt,sizeof(((WpixBtnRec*)NULL)->wpixBtn.cornerRoundPercent),XtOffsetOf(WpixBtnRec,wpixBtn.cornerRoundPercent),XtRImmediate,(XtPointer)0 },
#line 53 "WpixBtn.widget"
{XtNinnerBorderPercent,XtCInnerBorderPercent,XtRInt,sizeof(((WpixBtnRec*)NULL)->wpixBtn.innerBorderPercent),XtOffsetOf(WpixBtnRec,wpixBtn.innerBorderPercent),XtRImmediate,(XtPointer)0 },
#line 54 "WpixBtn.widget"
{XtNfileName,XtCFileName,XtRString,sizeof(((WpixBtnRec*)NULL)->wpixBtn.fileName),XtOffsetOf(WpixBtnRec,wpixBtn.fileName),XtRString,(XtPointer)NULL },
};

WpixBtnClassRec wpixBtnClassRec = {
{ /* core_class part */
/* superclass   	*/  (WidgetClass) &wheelClassRec,
/* class_name   	*/  "WpixBtn",
/* widget_size  	*/  sizeof(WpixBtnRec),
/* class_initialize 	*/  class_initialize,
/* class_part_initialize*/  _resolve_inheritance,
/* class_inited 	*/  FALSE,
/* initialize   	*/  initialize,
/* initialize_hook 	*/  NULL,
/* realize      	*/  realize,
/* actions      	*/  actionsList,
/* num_actions  	*/  4,
/* resources    	*/  resources,
/* num_resources 	*/  5,
/* xrm_class    	*/  NULLQUARK,
/* compres_motion 	*/  False ,
/* compress_exposure 	*/  FALSE ,
/* compress_enterleave 	*/  False ,
/* visible_interest 	*/  False ,
/* destroy      	*/  destroy,
/* resize       	*/  resize,
/* expose       	*/  expose,
/* set_values   	*/  set_values,
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
exec_command,
XtInherit_sig_recv,
},
{ /* WpixBtn_class part */
 /* dummy */  0
},
};
WidgetClass wpixBtnWidgetClass = (WidgetClass) &wpixBtnClassRec;
/*ARGSUSED*/
#line 18 "WpixBtn.widget"
static void next_pixmap(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
	int step=1;

	if( *num_params > 0 )
	    step = atoi( params[0] );
	((WpixBtnWidget)self)->wpixBtn.value += step;
	if( ((WpixBtnWidget)self)->wpixBtn.value < 0 ) ((WpixBtnWidget)self)->wpixBtn.value=0;
	((WpixBtnWidgetClass)self->core.widget_class)->core_class.expose(self,NULL,NULL);
}

/*ARGSUSED*/
#line 29 "WpixBtn.widget"
static void highlight(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
  if( ((WpixBtnWidget)self)->wheel.state == STATE_SELECTED ) return;
  ((WpixBtnWidget)self)->wheel.state = STATE_SELECTED;
  ((WpixBtnWidgetClass)self->core.widget_class)->core_class.expose(self,NULL,NULL);
}

/*ARGSUSED*/
#line 36 "WpixBtn.widget"
static void reset(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
  if( ((WpixBtnWidget)self)->wheel.state == STATE_NORMAL ) return;
  ((WpixBtnWidget)self)->wheel.state = STATE_NORMAL;
  ((WpixBtnWidgetClass)self->core.widget_class)->core_class.expose(self,NULL,NULL);
}

/*ARGSUSED*/
#line 43 "WpixBtn.widget"
static void notify(self,event,params,num_params)Widget self;XEvent*event;String*params;Cardinal*num_params;
{
  XtCallCallbackList( self, ((WpixBtnWidget)self)->wheel.callback, event );
}

static void _resolve_inheritance(class)
WidgetClass class;
{
  WpixBtnWidgetClass c __attribute__((unused)) = (WpixBtnWidgetClass) class;
  WpixBtnWidgetClass super __attribute__((unused));
  if (class == wpixBtnWidgetClass) return;
  super = (WpixBtnWidgetClass)class->core_class.superclass;
}
#line 70 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 70 "WpixBtn.widget"
static int  exec_command(Widget self,int  cmd,int  val)
#else
#line 70 "WpixBtn.widget"
static int  exec_command(self,cmd,val)Widget self;int  cmd;int  val;
#endif
#line 71 "WpixBtn.widget"
{
  Cardinal null;
  if( cmd == WHEEL_FIRE ) {
    if( ((WpixBtnWidget)self)->wheel.state == STATE_SELECTED ) { notify(self, NULL,NULL, &null ); return 1; }
  }

  return 0;
}
#line 81 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 81 "WpixBtn.widget"
static void class_initialize(void)
#else
#line 81 "WpixBtn.widget"
static void class_initialize()
#endif
#line 82 "WpixBtn.widget"
{
  m_init();
  converters_init();
}
#line 87 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 87 "WpixBtn.widget"
static void initialize(Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 87 "WpixBtn.widget"
static void initialize(request,self,args,num_args)Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 88 "WpixBtn.widget"
{

  int           w,h;

  ((WpixBtnWidget)self)->wpixBtn.name_cache = 0;
  w=10; h=10;
  ((WpixBtnWidget)self)->wpixBtn.cur_pix = -1; /* pixmap cache, undefined */
  ((WpixBtnWidget)self)->wpixBtn.pix_gc=0;
  ((WpixBtnWidget)self)->wpixBtn.m_pix = m_create( 3, sizeof( pix_t ) );

  /* load all pixmaps and compute maximum width,height */
  parsePixmapNames(self, &w, &h);
  /* set size of this widget if not set already by geometry manager */
  if( ((WpixBtnWidget)self)->core.width == 0 )  ((WpixBtnWidget)self)->core.width   = increase_by_percent( w, ((WpixBtnWidget)self)->wpixBtn.innerBorderPercent );
  if( ((WpixBtnWidget)self)->core.height == 0 ) ((WpixBtnWidget)self)->core.height =  increase_by_percent( h, ((WpixBtnWidget)self)->wpixBtn.innerBorderPercent );

  /* if there are no pixmaps set |value| to -1 to indicate that there is */
  /* nothing to draw */
  if( m_len(((WpixBtnWidget)self)->wpixBtn.m_pix)==0 ) ((WpixBtnWidget)self)->wpixBtn.value = -1;
  compute_layout(self);
}
#line 110 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 110 "WpixBtn.widget"
static void realize(Widget self,XtValueMask * mask,XSetWindowAttributes * attributes)
#else
#line 110 "WpixBtn.widget"
static void realize(self,mask,attributes)Widget self;XtValueMask * mask;XSetWindowAttributes * attributes;
#endif
#line 111 "WpixBtn.widget"
{
	XtCreateWindow(self, (unsigned int) InputOutput,
        		  (Visual *) CopyFromParent, *mask, attributes);
        /* this gc is Not shared and its writeable */
        ((WpixBtnWidget)self)->wpixBtn.pix_gc    = XCreateGC(XtDisplay(self),XtWindow(self),0,NULL );
  	reshape_widget(self);
}
#line 119 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 119 "WpixBtn.widget"
static void resize(Widget self)
#else
#line 119 "WpixBtn.widget"
static void resize(self)Widget self;
#endif
#line 120 "WpixBtn.widget"
{
    if (XtIsRealized(self)) {
            ((WpixBtnWidget)self)->wpixBtn.cur_pix = -1;
            compute_layout(self);
            reshape_widget(self);
    }
}
#line 128 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 128 "WpixBtn.widget"
static void destroy(Widget self)
#else
#line 128 "WpixBtn.widget"
static void destroy(self)Widget self;
#endif
#line 129 "WpixBtn.widget"
{
  if( ((WpixBtnWidget)self)->wpixBtn.pix_gc ) XtReleaseGC(self,((WpixBtnWidget)self)->wpixBtn.pix_gc);
  unloadPixmaps(self);
  if( ((WpixBtnWidget)self)->wpixBtn.name_cache ) free(((WpixBtnWidget)self)->wpixBtn.name_cache);
  m_free(((WpixBtnWidget)self)->wpixBtn.m_pix);
}
#line 136 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 136 "WpixBtn.widget"
static void expose(Widget self,XEvent * event,Region  region)
#else
#line 136 "WpixBtn.widget"
static void expose(self,event,region)Widget self;XEvent * event;Region  region;
#endif
#line 137 "WpixBtn.widget"
{
  Display *dpy = XtDisplay(self);
  if( !XtIsRealized(self)) return;

  XSetWindowBorder( dpy, XtWindow(self), ((WpixBtnWidget)self)->wheel.pixel[ ((WpixBtnWidget)self)->wheel.state ] );
  XFillRectangle( dpy, XtWindow(self), ((WpixBtnWidget)self)->wheel.gc[((WpixBtnWidget)self)->wheel.state], 0,0, ((WpixBtnWidget)self)->core.width, ((WpixBtnWidget)self)->core.height );

  /* check if we should display a pixmap */
  if( ((WpixBtnWidget)self)->wpixBtn.value < 0 || m_len(((WpixBtnWidget)self)->wpixBtn.m_pix) == 0 ) return;

  /* check for overflow */
  if( ((WpixBtnWidget)self)->wpixBtn.value >= m_len(((WpixBtnWidget)self)->wpixBtn.m_pix) ) ((WpixBtnWidget)self)->wpixBtn.value=0;

  /* check if we shall display a new pixmap and */
  /* need to apply a new pixmask */
  pix_t *p = mls(((WpixBtnWidget)self)->wpixBtn.m_pix, ((WpixBtnWidget)self)->wpixBtn.value);
  if( ((WpixBtnWidget)self)->wpixBtn.value != ((WpixBtnWidget)self)->wpixBtn.cur_pix ) {
    ((WpixBtnWidget)self)->wpixBtn.cur_pix = ((WpixBtnWidget)self)->wpixBtn.value;
    XSetClipMask(dpy, ((WpixBtnWidget)self)->wpixBtn.pix_gc,  p->mask);
    XSetClipOrigin(dpy, ((WpixBtnWidget)self)->wpixBtn.pix_gc, p->pm_rect.x, p->pm_rect.y );
  }

  /* display the image if it is not corrupted */
  if( p->pm_depth > 0 ) {
    XCopyArea(dpy, p->map, XtWindow(self), ((WpixBtnWidget)self)->wpixBtn.pix_gc,
	      0, 0, p->pm_rect.width, p->pm_rect.height,
   	      p->pm_rect.x, p->pm_rect.y );
     ((WpixBtnWidget)self)->wpixBtn.fileName = p->fileName;
  } else ((WpixBtnWidget)self)->wpixBtn.fileName = 0;
}
#line 168 "WpixBtn.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 168 "WpixBtn.widget"
static Boolean  set_values(Widget  old,Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 168 "WpixBtn.widget"
static Boolean  set_values(old,request,self,args,num_args)Widget  old;Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 169 "WpixBtn.widget"
{
  Boolean redisplay = True;
  int w,h;

  /* user changes current pixmap */
  if( ((WpixBtnWidget)self)->wpixBtn.value != ((WpixBtnWidget)old)->wpixBtn.value ) {
  };

  /* user changes pixmap cache */
  if( ((WpixBtnWidget)self)->wpixBtn.pixmapNames != ((WpixBtnWidget)old)->wpixBtn.pixmapNames ) {
      ((WpixBtnWidget)self)->wpixBtn.cur_pix=0; /* expose needs to set a new clipmask */
      parsePixmapNames(self,&w,&h);
      /* change widget size: only set width/height to a new value */
  }

  return redisplay;
}
