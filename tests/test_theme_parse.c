/*
 * test_theme_parse.c - headless test for the theme registry + hdf parser.
 *
 * Pixel resolution (get_color) needs a Display/XCC and is exercised by the
 * demo under Xvfb; here we test the store, parser and integer/colour values.
 */
#include "mls.h"
#include "theme.h"
#include "theme_parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int failures = 0;

#define CHECK(cond) do { \
    if (!(cond)) { fprintf(stderr, "FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } \
} while (0)

int main(void)
{
    int bn = 0;
    char **names = NULL;

    m_init();

    /* parse a colour + integer theme */
    int n = theme_parse_string(
        "(theme highcontrast (bg_norm 0xffffff bg_hi 0x00eeee borderwidth 0))");
    CHECK(n == 1);

    theme_select("highcontrast");
    CHECK(strcmp(theme_active(), "highcontrast") == 0);
    CHECK(theme_get_rgb("bg_norm") == 0xffffff);
    CHECK(theme_get_rgb("bg_hi") == 0x00eeee);
    CHECK(theme_color_defined("bg_norm"));
    CHECK(!theme_color_defined("borderwidth"));
    CHECK(theme_int_defined("borderwidth"));
    CHECK(!theme_int_defined("bg_norm"));
    CHECK(theme_get_int("borderwidth", &bn) && bn == 0);
    CHECK(!theme_color_defined("does_not_exist"));
    CHECK(theme_get_rgb("does_not_exist") == 0);

    /* unknown theme name is a no-op */
    theme_select("nope");
    CHECK(strcmp(theme_active(), "highcontrast") == 0);
    CHECK(theme_get_rgb("bg_norm") == 0xffffff);

    /* a second theme, plus a redefinition of the first */
    theme_parse_string(
        "(theme test (bg_norm 0x010203 borderwidth 6 bordercolor 0xff0000))");
    CHECK(theme_list(&names) == 2);
    CHECK(names != NULL);
    if (names) { free(names); names = NULL; }

    theme_select("test");
    CHECK(theme_get_rgb("bg_norm") == 0x010203);
    CHECK(theme_get_int("borderwidth", &bn) && bn == 6);
    CHECK(theme_get_rgb("bordercolor") == 0xff0000);

    /* redefinition replaces in place (no duplicate) */
    theme_parse_string("(theme test (bg_norm 0x0a0b0c borderwidth 2))");
    CHECK(theme_list(&names) == 2);
    if (names) { free(names); names = NULL; }

    /* a ';' comment may follow a value without whitespace */
    theme_parse_string(
        "(theme comment (bg_norm 0xff0000;red\n"
        " borderwidth 2;two\n))");
    theme_select("comment");
    CHECK(theme_color_defined("bg_norm"));
    CHECK(theme_get_rgb("bg_norm") == 0xff0000);
    CHECK(theme_int_defined("borderwidth"));
    CHECK(theme_get_int("borderwidth", &bn) && bn == 2);

    /* duplicate keys: the last assignment wins */
    theme_parse_string("(theme dup (bg_norm 0x111111 bg_norm 0x222222))");
    theme_select("dup");
    CHECK(theme_get_rgb("bg_norm") == 0x222222);

    /* --- string values -------------------------------------------------- */
    /* bare token -> string; quoted token is ALWAYS a string; negative int
     * stays an int even after we gained string support. */
    theme_parse_string(
        "(theme strings (font_small \"DejaVu Sans 11\" font_bare Sans-11 "
        "num \"5\" hexlook \"0x10\" semi \";\" neg -5 pos 7))");
    theme_select("strings");

    CHECK(theme_str_defined("font_small"));
    CHECK(strcmp(theme_get_str("font_small"), "DejaVu Sans 11") == 0);
    CHECK(!theme_int_defined("font_small"));
    CHECK(!theme_color_defined("font_small"));
    CHECK(theme_str_defined("font_bare") && strcmp(theme_get_str("font_bare"), "Sans-11") == 0);
    CHECK(!theme_int_defined("font_bare"));
    /* a quoted numeric-looking value is a string, not an int */
    CHECK(theme_str_defined("num") && strcmp(theme_get_str("num"), "5") == 0);
    CHECK(!theme_int_defined("num"));
    /* a quoted 0x-looking value is a string, not a colour */
    CHECK(theme_str_defined("hexlook") && strcmp(theme_get_str("hexlook"), "0x10") == 0);
    CHECK(!theme_color_defined("hexlook"));
    /* a string may contain a comment character when quoted */
    CHECK(theme_str_defined("semi") && strcmp(theme_get_str("semi"), ";") == 0);
    /* integers still work, including negative ones */
    CHECK(!theme_str_defined("neg"));
    CHECK(theme_int_defined("neg") && theme_get_int("neg", &bn) && bn == -5);
    CHECK(theme_int_defined("pos") && theme_get_int("pos", &bn) && bn == 7);
    CHECK(theme_get_str("does_not_exist") == NULL);
    CHECK(!theme_str_defined("does_not_exist"));

    /* --- style-guide helpers -------------------------------------------- */
    theme_parse_string(
        "(theme style (font_ui \"Serif-11\" control_height 30 "
        "button_bg 0x112233 input_bg 0x445566))");
    theme_select("style");
    {
        char face[64];
        int px = 0;
        CHECK(strcmp(theme_font_spec(), "Serif-11") == 0);
        CHECK(theme_font_parts(theme_font_spec(), face, sizeof(face), &px) == 1);
        CHECK(strcmp(face, "Serif") == 0);
        CHECK(px == 11);
        CHECK(theme_metric("control_height", 0) == 30);
        CHECK(theme_metric("does_not_exist", 24) == 24);
        CHECK(theme_get_rgb("button_bg") == 0x112233);
        CHECK(theme_get_rgb("input_bg") == 0x445566);

        /* font spec without a size: whole string is the face, px == 0 */
        {
            char nosize[16];
            CHECK(theme_font_parts("NoSize", nosize, sizeof(nosize), &px) == 0);
            CHECK(strcmp(nosize, "NoSize") == 0);
            CHECK(px == 0);
        }
        /* a short face buffer is truncated, but the size still parses */
        {
            char small[5];
            CHECK(theme_font_parts("Sans-13", small, sizeof(small), &px) == 1);
            CHECK(strcmp(small, "Sans") == 0);
            CHECK(px == 13);
        }
    }

    /* theme_save -> reparse round-trip preserves strings (incl. spaces) */
    {
        const char *tmp = "/tmp/xtcw_theme_roundtrip.hdf";
        CHECK(theme_save(tmp) == 1);
        theme_destroy();                 /* drop the store, then reload from disk */
        CHECK(theme_load(tmp) >= 1);
        theme_select("strings");
        CHECK(theme_str_defined("font_small"));
        CHECK(strcmp(theme_get_str("font_small"), "DejaVu Sans 11") == 0);
        CHECK(strcmp(theme_get_str("num"), "5") == 0);
        CHECK(strcmp(theme_get_str("hexlook"), "0x10") == 0);
        CHECK(strcmp(theme_get_str("semi"), ";") == 0);
        CHECK(theme_int_defined("neg") && theme_get_int("neg", &bn) && bn == -5);
        remove(tmp);
    }

    /* theme_save escapes quotes, backslashes, newlines and tabs */
    {
        const char *tmp = "/tmp/xtcw_theme_escape.hdf";

        theme_begin("esc");
        theme_str("quote", "a\"b");
        theme_str("slash", "a\\b");
        theme_str("nl", "l1\nl2");
        theme_str("tab", "a\tb");
        theme_str("semi", ";");
        theme_end();
        theme_select("esc");

        CHECK(strcmp(theme_get_str("quote"), "a\"b") == 0);
        CHECK(strcmp(theme_get_str("slash"), "a\\b") == 0);
        CHECK(strcmp(theme_get_str("nl"), "l1\nl2") == 0);
        CHECK(strcmp(theme_get_str("tab"), "a\tb") == 0);
        CHECK(strcmp(theme_get_str("semi"), ";") == 0);

        CHECK(theme_save(tmp) == 1);
        theme_destroy();                 /* drop the store, then reload from disk */
        CHECK(theme_load(tmp) >= 1);
        theme_select("esc");
        CHECK(strcmp(theme_get_str("quote"), "a\"b") == 0);
        CHECK(strcmp(theme_get_str("slash"), "a\\b") == 0);
        CHECK(strcmp(theme_get_str("nl"), "l1\nl2") == 0);
        CHECK(strcmp(theme_get_str("tab"), "a\tb") == 0);
        CHECK(strcmp(theme_get_str("semi"), ";") == 0);
        remove(tmp);
    }

    /* saving an empty registry must not crash (and reloads as zero themes) */
    theme_destroy();
    {
        const char *tmp = "/tmp/xtcw_theme_empty.hdf";
        CHECK(theme_save(tmp) == 1);
        CHECK(theme_load(tmp) == 0);
        remove(tmp);
    }

    theme_destroy();
    m_destruct();

    if (failures) {
        fprintf(stderr, "test_theme_parse: %d check(s) FAILED\n", failures);
        return 1;
    }
    printf("test_theme_parse: PASSED\n");
    return 0;
}
