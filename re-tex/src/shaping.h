#ifndef RETEX_SHAPING_H
#define RETEX_SHAPING_H

#include <stdint.h>

/*
 * HarfBuzz/FreeType shaping for re-tex.
 *
 * re-tex lays text out in points.  A ShapingFont owns an FT_Face, a HarfBuzz
 * font and a Cairo font face for one (family, style, size) tuple.  Text runs
 * are shaped into clusters; each cluster owns the glyphs that map to one
 * source byte range.  The layout stores a cluster on the first character node
 * of the range and hides the remaining characters of a ligature/mark cluster.
 */

typedef struct ShapingFont ShapingFont;

typedef struct {
    uint32_t glyph_id;
    double x_advance;   /* points */
    double y_advance;   /* points */
    double x_offset;    /* points, HarfBuzz convention (y up) */
    double y_offset;
} ShapedGlyph;

typedef struct {
    ShapedGlyph *glyphs;
    int nglyphs;
    double width;
    double height;
    double depth;
    double x_scale;     /* microtype advance expansion, 1.0 = normal */
    uint32_t cp;        /* first code point of the cluster */
    ShapingFont *font;
    int source_start;   /* byte offset in the original UTF-8 text */
    int source_end;
} ShapedCluster;

/* Initialize fontconfig/FreeType.  Safe to call repeatedly. */
int shaping_init(void);

/* Optional language tag for HarfBuzz (e.g. "de", "en"). */
void shaping_set_language(const char *lang);

/* Optional HarfBuzz feature string, e.g. "liga=0, smcp=1". */
void shaping_set_features(const char *features);

/* Get a cached font.  Returns a borrowed pointer; do not free. */
ShapingFont *shaping_font_get(const char *family, int style, double size);

/* Shape a UTF-8 run.  base_offset is added to every cluster byte offset.
 * Returns a newly allocated cluster array; free with shaping_clusters_free().
 * Returns NULL when shaping is unavailable. */
ShapedCluster *shaping_shape_text(ShapingFont *font, const char *text, int len,
                                  int base_offset, int *out_nclusters);

/* Same, with a per-run HarfBuzz feature string (NULL = global features). */
ShapedCluster *shaping_shape_text_ex(ShapingFont *font, const char *text,
                                     int len, int base_offset,
                                     const char *features,
                                     int *out_nclusters);

void shaping_clusters_free(ShapedCluster *clusters, int nclusters);

/* Append shaped clusters as re-tex NODE_CHAR nodes to node_list.  Each cluster
 * is stored on one node; the node's source_offset/byte_len cover the cluster's
 * source range.  Returns 1 on success, 0 when shaping is unavailable. */
int shaping_append_run(int node_list, ShapingFont *font, const char *text,
                       int len, int base_offset, float size, int style,
                       int face_handle, int reverse_mode, double *out_width);

/* Same, with a per-run HarfBuzz feature string. */
int shaping_append_run_ex(int node_list, ShapingFont *font, const char *text,
                          int len, int base_offset, float size, int style,
                          int face_handle, int reverse_mode,
                          const char *features, double *out_width);

/* Free a single cluster moved out of a shaping result. */
void shaping_cluster_free(ShapedCluster *cluster);

/* Font metrics in points. */
void shaping_font_metrics(ShapingFont *font, double *ascent, double *descent);

/* Draw a shaped cluster at baseline (x, y) into a Cairo context. */
void shaping_draw_cluster(void *cairo_cr, ShapingFont *font, double x, double y,
                          const ShapedCluster *cluster);

/* Release all cached fonts (normally at process exit). */
void shaping_shutdown(void);

#endif /* RETEX_SHAPING_H */
