#include "hb-example.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef unsigned int uint;
typedef unsigned long ulong;



#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftadvanc.h>
#include <freetype/ftsnames.h>
#include <freetype/tttables.h>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb-icu.h> /* Alternatively you can use hb-glib.h */

#include <X11/Intrinsic.h>
#include <X11/xpm.h>
#include <cairo/cairo-xlib-xrender.h>
#include  <cairo/cairo-xlib.h>
#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#define NUM_EXAMPLES 3




const char *texts[NUM_EXAMPLES] = {
    "This is some english text",
    "هذه بعض النصوص العربية",
    "這是一些中文",
};

const int text_directions[NUM_EXAMPLES] = {
    HB_DIRECTION_LTR,
    HB_DIRECTION_RTL,
    HB_DIRECTION_TTB,
};

/* XXX: I don't know if these are correct or not, it doesn't seem to break anything
 *      regardless of their value. */
const char *languages[NUM_EXAMPLES] = {
    "en",
    "ar",
    "ch",
};

const hb_script_t scripts[NUM_EXAMPLES] = {
    HB_SCRIPT_LATIN,
    HB_SCRIPT_ARABIC,
    HB_SCRIPT_HAN,
};

inline static uint get_component( Pixel px, ulong m )
{
    ulong p = px;
    p &= m;

    while( m && !(m&1) ) { m>>=1; p >>=1; }
    return p;
}


void cairo_set_source_rgb_from_pixel_dpy(Display *dpy, cairo_t *c, Pixel px)
{
    Visual *v = DefaultVisual(dpy,DefaultScreen(dpy));
    uint r,g,b;
    r = get_component( px, v->red_mask );
    g = get_component( px, v->green_mask );
    b = get_component( px, v->blue_mask );
    cairo_set_source_rgb(c, r / 255.0 ,g / 255.0, b / 255.0 );
}


int write_to_pixmap( Display *dpy, Pixmap p,  Pixel foreground, XftFont *font, int width, int height, int id_lang, char *txt, int x0, int y0 )
{

    fprintf(stderr, "Hello World");
    int x = 0;
    int y = 0;
    // https://lists.cairographics.org/archives/cairo/2014-June/025275.html    
    // char *v = XGetDefault(dpy, "Xft", "dpi");
    // if( v ) dpi=atoi(v); else dpi=96;
    
    /* Init freetype */
    FT_Library ft_library;
    assert(!FT_Init_FreeType(&ft_library));
    FT_Face face = XftLockFace ( font );

    
    double font_size, font_dpi;
    FcPattern *pat = font->pattern;
    // FcPatternGetDouble( pat, FC_PIXEL_SIZE, 0, &pixel_size);
    FcPatternGetDouble( pat, FC_SIZE, 0, &font_size );
    FcPatternGetDouble( pat, FC_DPI, 0, &font_dpi );


    
    FT_Set_Char_Size(face,font_size *64,
		     font_size *64,
		     font_dpi, font_dpi);

    /* Get our cairo font structs */
    cairo_font_face_t *cairo_ft_face;
    cairo_ft_face = cairo_ft_font_face_create_for_ft_face(face, 0 );

    /* Get our harfbuzz font/face structs */
    hb_font_t *hb_ft_font;
    hb_face_t *hb_ft_face;
    hb_ft_font = hb_ft_font_create(face, NULL);
    hb_ft_face = hb_ft_face_create(face, NULL);

    /* create cairo drawable */
    cairo_surface_t *cairo_surface = cairo_xlib_surface_create(dpy,p,
                                   DefaultVisual(dpy, DefaultScreen(dpy)),
                                   width,height );

    cairo_t *cr = cairo_create(cairo_surface);

    /* Create a buffer for harfbuzz to use */
    hb_buffer_t *buf = hb_buffer_create();

    //alternatively you can use hb_buffer_set_unicode_funcs(buf, hb_glib_get_unicode_funcs());
    hb_buffer_set_unicode_funcs(buf, hb_icu_get_unicode_funcs());
    hb_buffer_set_direction(buf, text_directions[id_lang]); /* or LTR */
    hb_buffer_set_script(buf, scripts[id_lang]); /* see hb-unicode.h */
    hb_buffer_set_language(buf, hb_language_from_string(languages[id_lang], -1));

    /* Layout the text */
    hb_buffer_add_utf8(buf, txt, strlen(txt), 0, strlen(txt));
    hb_shape(hb_ft_font, buf, NULL, 0);

    /* Hand the layout to cairo to render */
    unsigned int         glyph_count;
    hb_glyph_info_t     *glyph_info   = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t *glyph_pos    = hb_buffer_get_glyph_positions(buf, &glyph_count);
    cairo_glyph_t       *cairo_glyphs = malloc(sizeof(cairo_glyph_t) * glyph_count);

    unsigned int string_width_in_pixels = 0;
    for (int i=0; i < glyph_count; ++i) {
	string_width_in_pixels += glyph_pos[i].x_advance/64;
    }

    if (id_lang == ENGLISH) { x = x0; y = y0 + font->ascent; } /* left justify */
    if (id_lang == ARABIC)  {				       
	x = width - string_width_in_pixels -x0;
	y = y0 + font->ascent;
    }   /* right justify */
    if (id_lang == CHINESE) {
	x = width/2 - string_width_in_pixels/2 + x0;
	y = y0;
    }   /* center */

    for (int i=0; i < glyph_count; ++i) {
	cairo_glyphs[i].index = glyph_info[i].codepoint;
	cairo_glyphs[i].x = x + (glyph_pos[i].x_offset/64.0);
	cairo_glyphs[i].y = y - (glyph_pos[i].y_offset/64.0);
	x += glyph_pos[i].x_advance/64.0;
	y -= glyph_pos[i].y_advance/64.0;
    }
    cairo_set_source_rgb_from_pixel_dpy( dpy, cr, foreground );
    cairo_set_font_face(cr, cairo_ft_face);
    cairo_set_font_size(cr, font_size * font_dpi / 72 );
    cairo_show_glyphs(cr, cairo_glyphs, glyph_count);
    free(cairo_glyphs);
    hb_buffer_destroy(buf);
    cairo_show_page(cr);
	
    /* We're now done with our cairo surface */
    cairo_surface_destroy(cairo_surface);
    cairo_destroy(cr);
    cairo_font_face_destroy(cairo_ft_face);
    hb_font_destroy(hb_ft_font);
    hb_face_destroy(hb_ft_face);
    FT_Done_FreeType(ft_library);
    XftUnlockFace(font);    
    return 0;
}



int hb_measure( Display *dpy, XftFont *font, int id_lang, char *txt, int *w, int *h )
{
    fprintf(stderr, "Hello World");
    /* Init freetype */
    FT_Library ft_library;
    assert(!FT_Init_FreeType(&ft_library));

    double font_size, font_dpi;
    FcPattern *pat = font->pattern;
    FcPatternGetDouble( pat, FC_SIZE, 0, &font_size ); 
    FcPatternGetDouble( pat, FC_DPI, 0, &font_dpi );
    
    /* Get our cairo font structs */
    FT_Face face = XftLockFace ( font );
    FT_Set_Char_Size(face,font_size *64,
		     font_size *64,
		     font_dpi, font_dpi);

    printf( "font:%lf dpi:%lf\n", font_size, font_dpi );

    /* Get our harfbuzz font/face structs */
    hb_font_t *hb_ft_font;
    hb_face_t *hb_ft_face;
    hb_ft_font = hb_ft_font_create(face, NULL);
    hb_ft_face = hb_ft_face_create(face, NULL);

     /* Create a buffer for harfbuzz to use */
    hb_buffer_t *buf = hb_buffer_create();

    //alternatively you can use hb_buffer_set_unicode_funcs(buf, hb_glib_get_unicode_funcs());
    hb_buffer_set_unicode_funcs(buf, hb_icu_get_unicode_funcs());
    hb_buffer_set_direction(buf, text_directions[id_lang]); /* or LTR */
    hb_buffer_set_script(buf, scripts[id_lang]); /* see hb-unicode.h */
    hb_buffer_set_language(buf, hb_language_from_string(languages[id_lang], -1));

    /* Layout the text */
    hb_buffer_add_utf8(buf, txt, strlen(txt), 0, strlen(txt));
    hb_shape(hb_ft_font, buf, NULL, 0);

    /* Hand the layout to cairo to render */
    unsigned int         glyph_count;
    hb_glyph_position_t *glyph_pos    = hb_buffer_get_glyph_positions(buf, &glyph_count);

    unsigned int string_width_in_pixels = 0;
    unsigned int string_height_in_pixels = 0;
    for (int i=0; i < glyph_count; ++i) {
	string_width_in_pixels += glyph_pos[i].x_advance/64;
	string_height_in_pixels += glyph_pos[i].y_advance/64;
    }

    /*
    printf("%d width=%d, height=%d, font-height=%d\n", id_lang, string_width_in_pixels, string_height_in_pixels, font->height );
    */
    
    
    if (id_lang == ENGLISH || 
	id_lang == ARABIC)  {
	*w = string_width_in_pixels;
	*h = font->height;
    }
    else if (id_lang == CHINESE) {
	*h = -string_height_in_pixels;
	*w = font->height;
    }

    hb_buffer_destroy(buf);
    hb_font_destroy(hb_ft_font);
    hb_face_destroy(hb_ft_face);
    FT_Done_FreeType(ft_library);
    XftUnlockFace(font);   
    return 0;
}
