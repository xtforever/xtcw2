#include "xl2.h"
#include "ctx.h"
#include "xutil.h"

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
    XftFont *fnt;
    XftColor *fg;
    XftColor *bg;
    int width,height,x,y;
    char *label;
    int space_dx;
    int bol;
};

static int xl2_ctx = 0;
static int xl2_pre_init(void)
{
    return ctx_init(&xl2_ctx,100,sizeof(struct xl2_ctx));
}

static struct xl2_ctx* xl2_ctx_get(int num)
{
    return mls(xl2_ctx,num);
}


static void add_word(int wl, XftFont *fnt, XftColor *fg, XftColor *bg, char *label )
{
    struct xl2_word w;
    w.fnt=fnt;
    w.fg = fg;
    w.bg = bg;
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


int xl2_init( Widget w, XftFont *fnt, XftColor *fg, XftColor *bg, char *label )
{
    int ctx = xl2_pre_init();
    struct xl2_ctx* xl2 = xl2_ctx_get(ctx);
    xl2->widget = w;
    xl2->fnt = fnt;
    xl2->bg = bg;
    xl2->fg = fg;
    xl2->label = strdup(label);
    xl2->wl = m_create(100,sizeof(struct xl2_word));
    word_split(xl2->wl,fnt,fg,bg,label);
    word_list_measure(xl2->widget, xl2->wl);
}

static void set_ascent(int xl, int asc, int from, int to)
{
    for(int i=from; i<to; i++) {
	struct xl2_word *wd = mls(xl,i);
	wd->y += asc; 
    }
}

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
    int max_ascent=0,i,line_start = 0;
    m_foreach(xl2->wl, p, word ) {
	word->bol = bol; bol=0;
	word->x = cur_x;
	word->y = cur_y;
	if( max_ascent < word->fnt->ascent )
	    max_ascent = word->fnt->ascent;
	if( word->height > line_max_height )
	    line_max_height = word->height;
	
	next_x = word->width + cur_x + word->space_dx;

	if( p < m_len(xl2->wl)-1 ) {
	    word = mls(xl2->wl, p+1 );
	    if( next_x + word->width < max_x ) {
		cur_x = next_x;
		TRACE(1,"%s %d %d %d",
		      word->label, cur_x, word->width, max_x );
	    } else {
		set_ascent(xl2->wl, max_ascent, line_start, p);
		line_start = p;
		bol = 1;
		cur_x = rect->x;
		cur_y += line_max_height;
	    }
	}
    }
    set_ascent(xl2->wl, max_ascent, line_start, p);
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
