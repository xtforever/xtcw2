#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <SDL.h>
#include <SDL_image.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftadvanc.h>
#include <freetype/ftsnames.h>
#include <freetype/tttables.h>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb-icu.h> /* Alternatively you can use hb-glib.h */

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>

#define NUM_EXAMPLES 3

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


enum {
    ENGLISH=0, ARABIC, CHINESE
};

