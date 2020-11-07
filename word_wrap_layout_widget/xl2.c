#include "xl2.h"
#include "ctx.h"
#include "xutil.h"

static XftFont *fnt_big;
struct xl2_ctx {
    int init;
    Widget widget;
    
    XftFont *fnt;
    XftColor *fg;
    XftColor *bg;

    char *label;
    int wl;
};

#define HARDBREAK (128)
#define LINESTART (64)
#define FONTSIZE  (1+2)
#define COLFG     (4+8)
#define COLBG     (16+32)

struct xl2_attr {
    XftFont *fnt;
    XftColor *fg;
    XftColor *bg;
};

struct xl2_word {
    int init;
    XftFont *fnt;
    XftColor *fg;
    XftColor *bg;
    int width,height,x,y;
    char *label;
    int space_dx;
    int flags;
};

#include <math.h>
#ifndef M_PI
# define M_PI 3.14159265359
#endif


/*

@fn.This
is
a
Text


split space and newline
space is size of prev. word
*/





static XftFont* xft_fontopen2(Display *dpy, int screen, char* name, double size, Bool core, int angle)
{
  XftFont *font; // the font we will return;
  XftPattern *fnt_pat;
  FcChar8 *picked_name;
  XftPattern *match_pat;  // the best available match on the system
  XftResult match_result; // the result of our matching attempt



  fnt_pat = XftNameParse (name);
  if( !fnt_pat ) {
    TRACE(2,"Name '%s' not parsed", name );
    return 0;
  }

  // rotate font if angle!=0
  if (angle !=0) {
    XftMatrix m;
    XftMatrixInit(&m);
    XftMatrixRotate(&m,cos(M_PI*angle/180.),sin(M_PI*angle/180.));
    XftPatternAddMatrix (fnt_pat, XFT_MATRIX,&m);
  }

  if (core) {
    XftPatternAddBool(fnt_pat, XFT_CORE, FcTrue);
    XftPatternAddBool(fnt_pat, XFT_RENDER, FcFalse);
  }


  XftPatternAddDouble (fnt_pat, XFT_SIZE, size );  
  
  // query the system to find a match for this font
  match_pat = XftFontMatch(dpy,screen, fnt_pat, &match_result);
  XftPatternDestroy( fnt_pat );
  if( !match_pat ) {
    TRACE(2,"Name '%s' not matched", name );
    return 0;
  }

  // open the matched font
  font = XftFontOpenPattern(dpy, match_pat);

  if (!font)
    XftPatternDestroy (match_pat);

  //  XftPatternDestroy(match_pat);
  //  FontConfig will destroy this resource for us. We must not!

  return font;
}

struct text_attrib {
    char name;
    char *family;
    int size;
    char *fg;
    char *bg;
} TEXT_ATTRIB[10];						\

 
static void init_attrib( int num, char name, char *family, int size, char *fg, char *bg )
{
    if( num >= ALEN(TEXT_ATTRIB) ) ERR("attrib num > array max");
    struct text_attrib *ta =  TEXT_ATTRIB + num;
    
    ta->name=name;
    ta->family=strdup(family);
    ta->size=size;
    ta->fg=strdup(fg);
    ta->bg=strdup(bg);
    
}


static void init_defaults( )
{
    char *family = "Sans";
    char *fg_color = "330033";
    char *bg_color = "808033";
    int sizes[] = { 7,8,10,12,16 };
    char *names = "tsmlh";

    char *p=names;
    int i=0;
    while( i < sizeof sizes && *p ) {
	init_attrib( i, *p, family, sizes[i], fg_color, bg_color );
    }
}
    



static int xl2_ctx = 0;

static struct xl2_ctx* xl2_ctx_get(int num)
{
    return mls(xl2_ctx,num);
}

static struct xl2_ctx* xl2_pre_init(void)
{
    int ctx_num = ctx_init(&xl2_ctx,100,sizeof(struct xl2_ctx));
    return xl2_ctx_get(ctx_num);
}



static void free_word(int wl, int index)
{
    struct xl2_word *w = mls(wl,index);
    if( w  )
	free( w->label );
    else ERR("word should be allocated here");
}




static void add_word(int wl, XftFont *fnt, XftColor *fg, XftColor *bg, char *label )
{
    struct xl2_word w;
    w.fnt=fnt;
    w.fg = fg;
    w.bg = bg;

	/* detect word color and format */
    if( strcmp(label,"World")==0 ) {
	TRACE(1,"font change");
	w.fnt = fnt_big;
    }

    w.label = strdup(label);
    m_put(wl, &w );
}

static void word_split(int wl, XftFont *fnt, XftColor *fg, XftColor *bg, char *label )
{
    char **word;
    int p;
    int w = m_split(0,label,32, 1);
    m_foreach(w,p,word) {
	add_word(wl,fnt,fg,bg, *word );
    }
    m_free_strings(w,0);
}

static void word_measure(Widget wid, struct xl2_word *w)
{
    xft_calc_string_size( wid,
		   	  w->fnt,
		   	  " ",
		   	  &(w->space_dx),
		   	  &(w->height) );
    xft_calc_string_size( wid,
		   	  w->fnt,
		   	  w->label,
		   	  &(w->width),
		   	  &(w->height) );

    TRACE(1,"%s %d %d %d", w->label, w->width, w->height, w->space_dx );
};

static void word_list_measure(Widget wid, int wl)
{
    struct xl2_word *word;
    int p;
    m_foreach(wl,p,word) {
	word_measure(wid, word);
    }
}


static void free_word_list(int wl)
{
    int i;
    for(i=0;i<m_len(wl);i++) {
	free_word(wl,i);
    }
}


int xl2_init( Widget w, XftFont *fnt, XftColor *fg, XftColor *bg, char *label )
{
    struct xl2_ctx* xl2 = xl2_pre_init();

    xl2->widget = w;
    xl2->fnt = fnt;

    Display *dpy = XtDisplay(w);
    fnt_big = xft_fontopen2( dpy,DefaultScreen(dpy), "Gentium Book Basic", 20, False, 0 ); 
    xl2->bg = bg;
    xl2->fg = fg;
    xl2->label = strdup(label);
    xl2->wl = m_create(100,sizeof(struct xl2_word));
    word_split(xl2->wl,fnt,fg,bg,label);
    word_list_measure(xl2->widget, xl2->wl);
}

static void set_ascent(int xl, int asc, int from, int to)
{
    if( to > m_len(xl) ) to=m_len(xl);
    for(int i=from; i < to; i++) {
	struct xl2_word *wd = mls(xl,i);
	wd->y += asc;
    }
}

static void set_line_center(int xl, int x0, int x1, int from, int to)
{
    if( to > m_len(xl) ) to=m_len(xl);
    struct xl2_word *wd = mls(xl,to-1);
    int ln_x1 = wd->x + wd->width;
    int word_shift = (x1 - ln_x1) / 2;
    if( word_shift < 0 ) return;
    for(int i=from; i < to; i++) {
	wd = mls(xl,i);
        wd->x += word_shift;
    }
}


/*
 */
static void layout_hvcenter(struct xl2_ctx* xl2, XRectangle *rect)
{
    int bol=1;
    int cur_x = rect->x;
    int cur_y = rect->y;
    int max_x = rect->x + rect->width;
    int max_y = rect->y + rect->height;
    int line_max_height = 0;
    int next_x;
    struct xl2_word *word;
    int p=0;

    /* layout word lines
     */
    int max_ascent=0,i,line_start = 0,
	last_word = m_len(xl2->wl)-1;


    bol=1;
    p=0;
    while( p < m_len(xl2->wl) ) {

	word = mls(xl2->wl,p);

	if( max_ascent < word->fnt->ascent )
	    max_ascent = word->fnt->ascent;
	if( word->height > line_max_height )
	    line_max_height = word->height;

	/* does this word fit (first word fits) ?*/
	next_x = cur_x + word->width; // cur_x + word->space_dx;
	if( next_x <= max_x || bol ) {
	    word->x = cur_x;
	    word->y = cur_y;
	    cur_x = next_x + word->space_dx;
	    word->flags |= bol ? LINESTART : 0;
	    bol=0;
	    p++; /* next word */
	    continue; /* ok, word placed on this line */
	}

	/* this word does not fit, advance to next line */
	set_ascent(xl2->wl, max_ascent, line_start, p);
	set_line_center(xl2->wl, rect->x, max_x, line_start, p ); 
	line_start = p;
	bol = 1;
	cur_x = rect->x;
	cur_y += line_max_height;
	max_ascent = 0;
	line_max_height = 0;
    }

    TRACE(1,"%d %d", line_start, p);
    set_ascent(xl2->wl, max_ascent, line_start, p);
    set_line_center(xl2->wl, rect->x, max_x, line_start, p ); 
}


static void draw_word_x(XftDraw *draw,XftFont*fnt,XftColor *c, int x, int y, char *s)
{
    XftDrawStringUtf8(draw, c, fnt,x,y, (FcChar8*)s, strlen(s) );
}

static void draw_word(XftDraw *draw, struct xl2_word *wd )
{
    draw_word_x(draw,wd->fnt,wd->fg,wd->x, wd->y, wd->label);
}

int xl2_draw_centered(int ctx, XftDraw *draw, XRectangle *rect)
{
    struct xl2_ctx* xl2 = xl2_ctx_get(ctx);
    TRACE(1,"should draw %s", xl2->label );
    layout_hvcenter(xl2, rect);
    struct xl2_word *wd;
    int p;
    m_foreach(xl2->wl,p,wd) {
	draw_word(draw,wd);
    }
}
