/*
 * Unit test for the XtRDistance-style unit conversion and the theme metric
 * helper.  Uses a NULL Display, i.e. the 96-dpi fallback, so no X11 server is
 * required.
 */
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "converters.h"
#include "theme.h"
#include "theme_parse.h"
#include "mls.h"

static int px_of(const char *spec)
{
    int px = -999;
    assert(converters_distance_from_display(NULL, spec, &px));
    return px;
}

int main(void)
{
    m_init();

    /* pt is 1/72 inch; 96 dpi is the fallback. */
    assert(px_of("72pt") == 96);
    assert(px_of("10.5pt") == 14);
    assert(px_of("0pt") == 0);
    assert(px_of("-3pt") == -4);

    /* physical units */
    assert(px_of("1in") == 96);
    assert(px_of("3mm") == 11);      /* 3 * 96 * 10 / 254 = 11.34 */
    assert(px_of("2cm") == 76);      /* 2 * 96 * 100 / 254 = 75.6  */

    /* bare numbers are pixels */
    assert(px_of("14") == 14);
    assert(px_of("-1") == -1);

    /* malformed specifications are rejected */
    {
        int px = 123;
        assert(!converters_distance_from_display(NULL, "bogus", &px));
        assert(!converters_distance_from_display(NULL, "10.5pt ", &px));
        assert(!converters_distance_from_display(NULL, "", &px));
    }

    /* theme values with units; plain integers keep their pixel meaning */
    assert(theme_parse_string(
        "(theme t (bar_size \"10.5pt\" legacy 14))") > 0);
    theme_select("t");
    assert(theme_metric_distance("bar_size", "1pt") == 14);
    assert(theme_metric_distance("legacy", "1pt") == 14);
    assert(theme_metric_distance("missing", "3mm") == 11);

    printf("test_distance passed\n");
    return 0;
}
