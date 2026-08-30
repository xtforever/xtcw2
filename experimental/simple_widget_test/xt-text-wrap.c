#include "xt-text-wrap.h"
#include "text-wrap.h"
#include "mls.h"


void m_free_mlist(void *ctx, void *d)
{
    m_free( *(int*) d );
}

void m_free_user(int m, void (*free_h)(void *,void*), void *ctx)
{
    void *d; int p;
    m_foreach(m,p,d) free_h(ctx,d);
}

void m_free_list_of_list(int m)
{
    m_free_user( m, m_free_mlist, 0 );
    m_free(m);
}

void m_clear_list_of_list(int m)
{
    m_free_user( m, m_free_mlist, 0 );
    m_clear(m);
}


int t2_write_centered( XftDraw *draw, XftColor *foreground, XftFont *font, int x0, int y0, int width, int height, int line )
{
    XGlyphInfo extents;
    Display *dpy = XftDrawDisplay(draw);
    int len = s_strlen( line );
    char *s = m_str(line);
    XftTextExtentsUtf8(dpy, font, (FcChar8*)  s, len, &extents);
    x0 += ( width - x0 - extents.xOff ) / 2;
    XftDrawStringUtf8( draw, foreground, font, x0, y0, s, len  );   
}



int t2_write_to_pixmap( Display *dpy, Pixmap pix,  XftColor *foreground, XftFont *font, int width, int height, char *txt, int char_width, int x0, int y0 )
{
    int p,*d;
    int lines = t2_wrap_string( 0, txt, char_width );
    Visual *vis = DefaultVisual(dpy,DefaultScreen(dpy));
    XftDraw *draw = XftDrawCreate(dpy, pix, vis, None );

    // centered vertical + horizontal
    int x=x0;
    int y=y0 + ( height - m_len(lines) * font->height ) / 2 + font->ascent;   
    m_foreach(lines,p,d) {
	t2_write_centered( draw, foreground, font, x, y, width, height, *d );
	y+= font->height;
    }
    goto leave;

    // top-left left-justified
    x=x0;
    y=y0 + font->ascent;
    m_foreach(lines,p,d) { 
	int len = s_strlen( *d );
	XftDrawStringUtf8( draw, foreground, font, x, y, m_str(*d), len  );
	y+= font->height;
    }


 leave:
    XftDrawDestroy( draw );
    m_free_list_of_list(lines);
    return 0;
}
