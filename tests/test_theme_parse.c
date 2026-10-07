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

    theme_destroy();
    m_destruct();

    if (failures) {
        fprintf(stderr, "test_theme_parse: %d check(s) FAILED\n", failures);
        return 1;
    }
    printf("test_theme_parse: PASSED\n");
    return 0;
}
