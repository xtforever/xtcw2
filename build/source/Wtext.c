/* Generated by wbuild
 * (generator version 3.3)
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#line 302 "Wtext.widget"
#include <X11/Xft/Xft.h>
#line 303 "Wtext.widget"
#include "converters-xft.h"
#line 304 "Wtext.widget"
#include <X11/Xmu/Converters.h>
#line 305 "Wtext.widget"
#include "mls.h"
#line 306 "Wtext.widget"
#include "util.h"
#line 307 "Wtext.widget"
#include "xutil.h"
#include <xtcw/WtextP.h>
static void _resolve_inheritance(
#if NeedFunctionPrototypes
WidgetClass
#endif
);
#line 25 "Wtext.widget"
static void initialize(
#if NeedFunctionPrototypes
Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 38 "Wtext.widget"
static void destroy(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 46 "Wtext.widget"
static void resize(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 52 "Wtext.widget"
static Boolean  set_values(
#if NeedFunctionPrototypes
Widget ,Widget ,Widget,ArgList ,Cardinal *
#endif
);
#line 61 "Wtext.widget"
static int  word_wrap(
#if NeedFunctionPrototypes
int ,int ,XRectangle *
#endif
);
#line 67 "Wtext.widget"
static int  get_height(
#if NeedFunctionPrototypes
Widget,int 
#endif
);
#line 73 "Wtext.widget"
static int  get_width(
#if NeedFunctionPrototypes
Widget,int ,int ,int ,int 
#endif
);
#line 85 "Wtext.widget"
static void measure_segments(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 99 "Wtext.widget"
static void  write_int(
#if NeedFunctionPrototypes
int ,int ,int 
#endif
);
#line 118 "Wtext.widget"
static void init_text(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 141 "Wtext.widget"
static void add_attrib(
#if NeedFunctionPrototypes
Widget,char ,char *
#endif
);
#line 155 "Wtext.widget"
static void set_text(
#if NeedFunctionPrototypes
Widget,char *
#endif
);
#line 162 "Wtext.widget"
static void layout_segments(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 185 "Wtext.widget"
static int  attribute_space(
#if NeedFunctionPrototypes
Widget,int 
#endif
);
#line 191 "Wtext.widget"
static void relayout(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 197 "Wtext.widget"
static void print_str_west(
#if NeedFunctionPrototypes
Widget,XftFont *,XRectangle *,char *
#endif
);
#line 215 "Wtext.widget"
static void print_str_center(
#if NeedFunctionPrototypes
Widget,XftFont *,XRectangle *,char *
#endif
);
#line 236 "Wtext.widget"
static void print_int(
#if NeedFunctionPrototypes
Widget,XftFont *,XRectangle *,int 
#endif
);
#line 255 "Wtext.widget"
static void sensor_draw(
#if NeedFunctionPrototypes
Widget,void *,void *
#endif
);
#line 270 "Wtext.widget"
static void print_segment(
#if NeedFunctionPrototypes
Widget,text_segment_t *
#endif
);
#line 288 "Wtext.widget"
static void redraw(
#if NeedFunctionPrototypes
Widget
#endif
);
#line 61 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 61 "Wtext.widget"
static int  word_wrap(int  seg_list,int  txt,XRectangle * r)
#else
#line 61 "Wtext.widget"
static int  word_wrap(seg_list,txt,r)int  seg_list;int  txt;XRectangle * r;
#endif
#line 62 "Wtext.widget"
{
    return 0;
}
#line 67 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 67 "Wtext.widget"
static int  get_height(Widget self,int  attribute)
#else
#line 67 "Wtext.widget"
static int  get_height(self,attribute)Widget self;int  attribute;
#endif
#line 68 "Wtext.widget"
{
    text_attribute_t *attr = mls( ((WtextWidget)self)->wtext.tseg.m_attr, attribute ); 
    return attr->font->height;
}
#line 73 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 73 "Wtext.widget"
static int  get_width(Widget self,int  attribute,int  ms,int  start,int  end)
#else
#line 73 "Wtext.widget"
static int  get_width(self,attribute,ms,start,end)Widget self;int  attribute;int  ms;int  start;int  end;
#endif
#line 74 "Wtext.widget"
{
    char *s = mls(ms,start);
    char *p = mls(ms,end);
    XGlyphInfo	    extents;
    text_attribute_t *attr = mls( ((WtextWidget)self)->wtext.tseg.m_attr, attribute ); 
    
    XftTextExtentsUtf8(XtDisplay(self), attr->font,  (FcChar8*)  s,
		       (int)(p-s)+1, &extents );
    return extents.xOff;
}
#line 85 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 85 "Wtext.widget"
static void measure_segments(Widget self)
#else
#line 85 "Wtext.widget"
static void measure_segments(self)Widget self;
#endif
#line 86 "Wtext.widget"
{
    text_segment_t *seg;
    int i;
    int w=0, h=0;
    m_foreach(((WtextWidget)self)->wtext.tseg.m_seg,i,seg) {
	w+=seg->r.width = get_width(self, seg->attribute, 
				 ((WtextWidget)self)->wtext.tseg.text, seg->start, seg->end );
	h+=seg->r.height = get_height(self, seg->attribute );
    }
    ((WtextWidget)self)->wtext.seg_w = w; ((WtextWidget)self)->wtext.seg_h = h;
}
#line 99 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 99 "Wtext.widget"
static void  write_int(int  m,int  p,int  val)
#else
#line 99 "Wtext.widget"
static void  write_int(m,p,val)int  m;int  p;int  val;
#endif
#line 100 "Wtext.widget"
{
    int c = val, i=0;
    
    while( c > 9 ) {
	c /= 10;
	i++;
    }
    
    m_setlen( m, p + i + 2 );
    CHAR(m, p+i+1 ) = 0;
    while( i>=0 ) {
	CHAR( m, p+i ) = '0' + (val % 10);
	val /=  10;
	i--;
    }
}
#line 118 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 118 "Wtext.widget"
static void init_text(Widget self)
#else
#line 118 "Wtext.widget"
static void init_text(self)Widget self;
#endif
#line 119 "Wtext.widget"
{
    tseg_init(& ((WtextWidget)self)->wtext.tseg );

    int len, m = m_create(50,1);
    m_write( m,0, ((WtextWidget)self)->wtext.family, strlen(((WtextWidget)self)->wtext.family));
    m_putc(  m, '-' ); len = m_len(m);

    write_int( m, len, ((WtextWidget)self)->wtext.sizeSmall );
    add_attrib(self, 's', mls(m,0) );

    write_int( m, len, ((WtextWidget)self)->wtext.sizeMedium );
    add_attrib(self, 'm', mls(m,0) );

    write_int( m, len, ((WtextWidget)self)->wtext.sizeLarge );
    add_attrib(self, 'l', mls(m,0) );

    write_int( m, len, ((WtextWidget)self)->wtext.sizeHuge );
    add_attrib(self, 'h', mls(m,0) );

    m_free(m);
}
#line 141 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 141 "Wtext.widget"
static void add_attrib(Widget self,char  name,char * fnt)
#else
#line 141 "Wtext.widget"
static void add_attrib(self,name,fnt)Widget self;char  name;char * fnt;
#endif
#line 142 "Wtext.widget"
{
    Display *dpy = XtDisplay(self);
    XGlyphInfo extents;
    text_attribute_t attr;
    memset( &attr,0,sizeof(attr));
    attr.font = xft_fontopen(dpy, DefaultScreen(dpy), fnt, False, 0 );
    memcpy( & attr.color, ((WtextWidget)self)->wheel.xft_col, sizeof( attr.color ));
    XftTextExtentsUtf8(dpy, attr.font, (FcChar8*)" ",1, &extents);
    attr.space_width = extents.xOff;
    attr.name = name;
    m_put( ((WtextWidget)self)->wtext.tseg.m_attr, &attr );
}
#line 155 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 155 "Wtext.widget"
static void set_text(Widget self,char * s)
#else
#line 155 "Wtext.widget"
static void set_text(self,s)Widget self;char * s;
#endif
#line 156 "Wtext.widget"
{
    tseg_set_text(& ((WtextWidget)self)->wtext.tseg,s );
    measure_segments(self);
}
#line 162 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 162 "Wtext.widget"
static void layout_segments(Widget self)
#else
#line 162 "Wtext.widget"
static void layout_segments(self)Widget self;
#endif
#line 163 "Wtext.widget"
{
    int m_seg = ((WtextWidget)self)->wtext.tseg.m_seg;
    text_segment_t *seg;
    int i;
    int x,y;
    int max_h;
    int line_break = 0;
    ((WtextWidget)self)->wtext.total_height = 0;

    x=0; y=0; max_h=0;
    m_foreach(m_seg,i,seg) {
	if( x+seg->r.width > ((WtextWidget)self)->core.width || line_break ) {
	    y+=max_h; x=0; max_h=0; line_break = 0;
	}
	max_h = Max(seg->r.height, max_h );
	seg->r.x = x;
	seg->r.y = y; ((WtextWidget)self)->wtext.total_height = y + max_h;
	x += seg->r.width + attribute_space( self, seg->attribute );
	if( seg->format & T_HARD_BREAK) line_break=1;
    }       
}
#line 185 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 185 "Wtext.widget"
static int  attribute_space(Widget self,int  attr_id)
#else
#line 185 "Wtext.widget"
static int  attribute_space(self,attr_id)Widget self;int  attr_id;
#endif
#line 186 "Wtext.widget"
{
    text_attribute_t *attr = mls(((WtextWidget)self)->wtext.tseg.m_attr, attr_id);
    return attr->space_width;
}
#line 191 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 191 "Wtext.widget"
static void relayout(Widget self)
#else
#line 191 "Wtext.widget"
static void relayout(self)Widget self;
#endif
#line 192 "Wtext.widget"
{
	layout_segments(self);
}
#line 197 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 197 "Wtext.widget"
static void print_str_west(Widget self,XftFont * fnt,XRectangle * r,char * s)
#else
#line 197 "Wtext.widget"
static void print_str_west(self,fnt,r,s)Widget self;XftFont * fnt;XRectangle * r;char * s;
#endif
#line 198 "Wtext.widget"
{
    int x,y;
    if( !s ) return; /* disabled */
    XFillRectangle( XtDisplay(self), XtWindow(self), ((WtextWidget)self)->wheel.gc[0], 
		    r->x, r->y, r->width, r->height );
    /* XftDrawSetClipRectangles( $draw,0,0,r, 1); */
    /*XftTextExtentsUtf8(XtDisplay($), $fnt, (FcChar8*)s,
		       strlen(s), &extents);
    w = extents.xOff;
    */

    x = 1;
    y = (r->height - fnt->height) / 2 + fnt->ascent;
    XftDrawStringUtf8(((WtextWidget)self)->wlabel.draw, ((WtextWidget)self)->wheel.xft_col, fnt, 
		      x+r->x,y+r->y, (FcChar8*)s, strlen(s) );
}
#line 215 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 215 "Wtext.widget"
static void print_str_center(Widget self,XftFont * fnt,XRectangle * r,char * s)
#else
#line 215 "Wtext.widget"
static void print_str_center(self,fnt,r,s)Widget self;XftFont * fnt;XRectangle * r;char * s;
#endif
#line 216 "Wtext.widget"
{
    XGlyphInfo extents;
    int w,x,y;

    if( !s ) return; /* disabled */

    XFillRectangle( XtDisplay(self), XtWindow(self), ((WtextWidget)self)->wheel.gc[0], 
		    r->x, r->y, r->width, r->height );
   /* XftDrawSetClipRectangles( $draw,0,0,r, 1); */
   XftTextExtentsUtf8(XtDisplay(self), fnt, (FcChar8*)s,
		       strlen(s), &extents);
    w = extents.xOff;
   
    x = (r->width - w) / 2;
    y = (r->height - fnt->height) / 2 + fnt->ascent;
    XftDrawStringUtf8(((WtextWidget)self)->wlabel.draw, ((WtextWidget)self)->wheel.xft_col, fnt, 
		      x+r->x,y+r->y, (FcChar8*)s, strlen(s) );
}
#line 236 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 236 "Wtext.widget"
static void print_int(Widget self,XftFont * fnt,XRectangle * r,int  val)
#else
#line 236 "Wtext.widget"
static void print_int(self,fnt,r,val)Widget self;XftFont * fnt;XRectangle * r;int  val;
#endif
#line 237 "Wtext.widget"
{
    char buf[100];
    snprintf( buf,100,"%d", val );
    int x,y;
    char *s;

    if( val < 0 ) return; /* disabled */

    XFillRectangle( XtDisplay(self), XtWindow(self), ((WtextWidget)self)->wheel.gc[0], 
		    r->x, r->y, r->width, r->height );
    /* XftDrawSetClipRectangles( $draw,0,0,r, 1); */
    x = 1;
    y = (r->height - fnt->height) / 2 + fnt->ascent;
    s = buf;
    XftDrawStringUtf8(((WtextWidget)self)->wlabel.draw, ((WtextWidget)self)->wheel.xft_col, fnt, 
		      x+r->x,y+r->y, (FcChar8*)s, strlen(s) );
}
#line 255 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 255 "Wtext.widget"
static void sensor_draw(Widget self,void * a,void * b)
#else
#line 255 "Wtext.widget"
static void sensor_draw(self,a,b)Widget self;void * a;void * b;
#endif
#line 256 "Wtext.widget"
{
	
    	XFillRectangle( XtDisplay(self), XtWindow(self), ((WtextWidget)self)->wheel.gc[ ((WtextWidget)self)->wheel.state ], 
		    0,0, ((WtextWidget)self)->core.width, ((WtextWidget)self)->core.height );

	if( ((WtextWidget)self)->wtext.relayout ) {
	    relayout(self);
	    ((WtextWidget)self)->wtext.relayout = False;
	}    	

	((WtextWidget)self)->wlabel.update = -1;
	redraw(self);
}
#line 270 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 270 "Wtext.widget"
static void print_segment(Widget self,text_segment_t * s)
#else
#line 270 "Wtext.widget"
static void print_segment(self,s)Widget self;text_segment_t * s;
#endif
#line 271 "Wtext.widget"
{
    FcChar8* str;
    int len;
    int x,y;
    text_attribute_t *attr = mls(((WtextWidget)self)->wtext.tseg.m_attr, s->attribute);

    x = s->r.x;
    y = s->r.y + attr->font->ascent;

    str = mls( ((WtextWidget)self)->wtext.tseg.text, s->start);
    len = s->end - s->start + 1;
    
    XftDrawStringUtf8(((WtextWidget)self)->wlabel.draw, & attr->color, attr->font, 
		      x,y,str,len );
    
}
#line 288 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 288 "Wtext.widget"
static void redraw(Widget self)
#else
#line 288 "Wtext.widget"
static void redraw(self)Widget self;
#endif
#line 289 "Wtext.widget"
{
    text_segment_t *seg;
    int i;

    m_foreach( ((WtextWidget)self)->wtext.tseg.m_seg, i, seg )
	{
	    print_segment( self, seg );
	}

    ((WtextWidget)self)->wlabel.update = 0;
}

static XtResource resources[] = {
#line 8 "Wtext.widget"
{XtNtext,XtCText,XtRString,sizeof(((WtextRec*)NULL)->wtext.text),XtOffsetOf(WtextRec,wtext.text),XtRImmediate,(XtPointer)NULL },
#line 9 "Wtext.widget"
{XtNfamily,XtCFamily,XtRString,sizeof(((WtextRec*)NULL)->wtext.family),XtOffsetOf(WtextRec,wtext.family),XtRString,(XtPointer)"Sans"},
#line 10 "Wtext.widget"
{XtNsizeSmall,XtCSizeSmall,XtRInt,sizeof(((WtextRec*)NULL)->wtext.sizeSmall),XtOffsetOf(WtextRec,wtext.sizeSmall),XtRImmediate,(XtPointer)8 },
#line 11 "Wtext.widget"
{XtNsizeMedium,XtCSizeMedium,XtRInt,sizeof(((WtextRec*)NULL)->wtext.sizeMedium),XtOffsetOf(WtextRec,wtext.sizeMedium),XtRImmediate,(XtPointer)14 },
#line 12 "Wtext.widget"
{XtNsizeLarge,XtCSizeLarge,XtRInt,sizeof(((WtextRec*)NULL)->wtext.sizeLarge),XtOffsetOf(WtextRec,wtext.sizeLarge),XtRImmediate,(XtPointer)20 },
#line 13 "Wtext.widget"
{XtNsizeHuge,XtCSizeHuge,XtRInt,sizeof(((WtextRec*)NULL)->wtext.sizeHuge),XtOffsetOf(WtextRec,wtext.sizeHuge),XtRImmediate,(XtPointer)38 },
};

WtextClassRec wtextClassRec = {
{ /* core_class part */
/* superclass   	*/  (WidgetClass) &wlabelClassRec,
/* class_name   	*/  "Wtext",
/* widget_size  	*/  sizeof(WtextRec),
/* class_initialize 	*/  NULL,
/* class_part_initialize*/  _resolve_inheritance,
/* class_inited 	*/  FALSE,
/* initialize   	*/  initialize,
/* initialize_hook 	*/  NULL,
/* realize      	*/  XtInheritRealize,
/* actions      	*/  NULL,
/* num_actions  	*/  0,
/* resources    	*/  resources,
/* num_resources 	*/  6,
/* xrm_class    	*/  NULLQUARK,
/* compres_motion 	*/  False ,
/* compress_exposure 	*/  TRUE ,
/* compress_enterleave 	*/  False ,
/* visible_interest 	*/  False ,
/* destroy      	*/  destroy,
/* resize       	*/  resize,
/* expose       	*/  XtInheritExpose,
/* set_values   	*/  set_values,
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
XtInherit_cache_hit,
},
{ /* Wtext_class part */
 /* dummy */  0
},
};
WidgetClass wtextWidgetClass = (WidgetClass) &wtextClassRec;
static void _resolve_inheritance(class)
WidgetClass class;
{
  WtextWidgetClass c __attribute__((unused)) = (WtextWidgetClass) class;
  WtextWidgetClass super __attribute__((unused));
  if (class == wtextWidgetClass) return;
  super = (WtextWidgetClass)class->core_class.superclass;
}
#line 25 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 25 "Wtext.widget"
static void initialize(Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 25 "Wtext.widget"
static void initialize(request,self,args,num_args)Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 26 "Wtext.widget"
{
    ((WtextWidget)self)->wlabel.draw_override = sensor_draw;	
    ((WtextWidget)self)->wtext.relayout = True;      
    ((WtextWidget)self)->wlabel.update = -1; 
    if( ((WtextWidget)self)->wtext.text == NULL ) ((WtextWidget)self)->wtext.text = ((WtextWidget)self)->core.name;
    init_text(self);
    set_text(self, ((WtextWidget)self)->wtext.text);
    /* measure_segments */
    ((WtextWidget)self)->core.width = ((WtextWidget)self)->wtext.seg_w; ((WtextWidget)self)->core.height = ((WtextWidget)self)->wtext.seg_h; 
    TRACE(2,"Wtext: %s Height: %d", ((WtextWidget)self)->core.name, ((WtextWidget)self)->core.height );
}
#line 38 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 38 "Wtext.widget"
static void destroy(Widget self)
#else
#line 38 "Wtext.widget"
static void destroy(self)Widget self;
#endif
#line 39 "Wtext.widget"
{
    text_attribute_t *a; int i;
    m_foreach( ((WtextWidget)self)->wtext.tseg.m_attr, i, a )
	XftFontClose( XtDisplay(self), a->font );
    tseg_free( & ((WtextWidget)self)->wtext.tseg );
}
#line 46 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 46 "Wtext.widget"
static void resize(Widget self)
#else
#line 46 "Wtext.widget"
static void resize(self)Widget self;
#endif
#line 47 "Wtext.widget"
{
    
    ((WtextWidget)self)->wtext.relayout = True; 
}
#line 52 "Wtext.widget"
/*ARGSUSED*/
#if NeedFunctionPrototypes
#line 52 "Wtext.widget"
static Boolean  set_values(Widget  old,Widget  request,Widget self,ArgList  args,Cardinal * num_args)
#else
#line 52 "Wtext.widget"
static Boolean  set_values(old,request,self,args,num_args)Widget  old;Widget  request;Widget self;ArgList  args;Cardinal * num_args;
#endif
#line 53 "Wtext.widget"
{  
    return True;
}
#line 5 "Wtext.widget"
