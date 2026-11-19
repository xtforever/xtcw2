/*
 * HarfBuzz/FreeType shaping smoke test.
 * Skips cleanly when no font is available.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "shaping.h"

int main(void)
{
    ShapingFont *font;
    ShapedCluster *clusters;
    int n = 0, i;
    double width = 0.0;

    if (!shaping_init()) {
        printf("test_shaping skipped (no HarfBuzz/FreeType)\n");
        return 0;
    }
    font = shaping_font_get("Serif", 0, 20.0);
    if (!font) {
        printf("test_shaping skipped (no Serif font)\n");
        return 0;
    }

    clusters = shaping_shape_text(font, "AVfi", 4, 0, &n);
    if (!clusters || n == 0) {
        printf("test_shaping skipped (font has no glyphs)\n");
        shaping_clusters_free(clusters, n);
        return 0;
    }

    assert(n > 0);
    assert(clusters[0].nglyphs > 0);
    assert(clusters[0].source_start == 0);
    for (i = 0; i < n; i++) {
        assert(clusters[i].nglyphs > 0);
        assert(clusters[i].width > 0.0);
        width += clusters[i].width;
    }
    assert(width > 0.0);

    /* A cluster must not start before its run and the last one must end at
     * the end of the UTF-8 input. */
    assert(clusters[0].source_start == 0);
    assert(clusters[n - 1].source_end == 4);

    shaping_clusters_free(clusters, n);

    /* Feature API: disabling ligatures must not reduce the glyph count. */
    {
        int n_liga = 0, n_noliga = 0, i2;
        ShapedCluster *a = shaping_shape_text(font, "fi", 2, 0, &n_liga);
        int ga = 0, gb = 0;
        for (i2 = 0; i2 < n_liga; i2++) ga += a[i2].nglyphs;
        shaping_clusters_free(a, n_liga);

        shaping_set_features("liga=0");
        a = shaping_shape_text(font, "fi", 2, 0, &n_noliga);
        for (i2 = 0; i2 < n_noliga; i2++) gb += a[i2].nglyphs;
        shaping_clusters_free(a, n_noliga);
        shaping_set_features(NULL);
        assert(gb >= ga);
    }

    printf("test_shaping passed\n");
    return 0;
}
