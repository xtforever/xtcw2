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

struct xl2_word {
    int init;
    XftFont *fnt;
    XftColor *fg;
    XftColor *bg;
    int width,height,x,y;
    char *label;
    int space_dx;
    int bol;
};

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
	add_word(wl, fnt,fg,bg, *word );
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
    fnt_big = XftFontOpenName( dpy,DefaultScreen(dpy), "Gentium Book Basic-50" ); 
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
	
	/* does this word fit ? */
	next_x = cur_x + word->width; // cur_x + word->space_dx;
	if( next_x <= max_x || bol ) {
	    word->x = cur_x;
	    word->y = cur_y;
	    cur_x = next_x + word->space_dx;
	    bol=0;
	    p++;
	    continue; /* ok, word placed on this line */	    
	}
	
	/* this word does not fit, advance to next line */
	set_ascent(xl2->wl, max_ascent, line_start, p);
	set_line_center(xl2->wl, rect->x, max_x, line_start, p ); 
	line_start = p;
	bol = 1;
	cur_x = rect->x;
	cur_y += line_max_height;
	max_ascent = 0; line_max_height = 0;
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
