/*
 * shaping.c - HarfBuzz + FreeType shaping for re-tex.
 *
 * The rest of the engine works in points and keeps a source-byte mapping for
 * every glyph cluster, so selection, hit-testing and line breaking keep
 * working while rendering uses real shaped glyph runs.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <fontconfig/fontconfig.h>

#include <hb.h>
#include <hb-ft.h>
#include <hb-ot.h>

#include <cairo.h>
#include <cairo-ft.h>

#include "shaping.h"
#include "utf8.h"
#include "node.h"
#include "scaled.h"
#include "mls.h"

struct ShapingFont {
    FT_Face ft;
    hb_font_t *hb;
    cairo_font_face_t *cairo_face;
    double size;
    int style;
    char *family;
    char *path;
    int index;
    ShapingFont *next;
};

static FT_Library ft_lib;
static int ft_ready;
static int fc_ready;
static ShapingFont *font_cache;
static hb_language_t shape_language;
static hb_feature_t *shape_features;
static unsigned int shape_nfeatures;

typedef struct FeatureCache {
    char *key;
    hb_feature_t *features;
    unsigned int nfeatures;
    struct FeatureCache *next;
} FeatureCache;

static FeatureCache *feature_cache;

static int shaping_ensure_init(void)
{
    if (!fc_ready) {
        if (!FcInit())
            return 0;
        fc_ready = 1;
    }
    if (!ft_ready) {
        if (FT_Init_FreeType(&ft_lib) != 0)
            return 0;
        ft_ready = 1;
    }
    return 1;
}

int shaping_init(void)
{
    return shaping_ensure_init();
}

void shaping_set_language(const char *lang)
{
    if (lang && *lang)
        shape_language = hb_language_from_string(lang, -1);
    else
        shape_language = NULL;
}

static hb_feature_t *parse_features(const char *features, unsigned int *out_n)
{
    FeatureCache *fc;
    char *copy, *tok, *save = NULL;
    hb_feature_t *list = NULL;
    unsigned int n = 0, cap = 0;

    *out_n = 0;
    if (!features || !*features)
        return NULL;

    for (fc = feature_cache; fc; fc = fc->next) {
        if (strcmp(fc->key, features) == 0) {
            *out_n = fc->nfeatures;
            return fc->features;
        }
    }

    copy = strdup(features);
    if (!copy)
        return NULL;
    for (tok = strtok_r(copy, ", ", &save); tok;
         tok = strtok_r(NULL, ", ", &save)) {
        hb_feature_t f;
        if (!hb_feature_from_string(tok, -1, &f))
            continue;
        if (n == cap) {
            unsigned int nc = cap ? cap * 2 : 4;
            hb_feature_t *nl = (hb_feature_t *)realloc(list,
                                                       nc * sizeof(*nl));
            if (!nl)
                break;
            list = nl;
            cap = nc;
        }
        list[n++] = f;
    }
    free(copy);

    fc = (FeatureCache *)calloc(1, sizeof(*fc));
    if (fc) {
        fc->key = strdup(features);
        fc->features = list;
        fc->nfeatures = n;
        fc->next = feature_cache;
        feature_cache = fc;
    }
    *out_n = n;
    return list;
}

void shaping_set_features(const char *features)
{
    char *copy, *tok, *save = NULL;
    hb_feature_t *list = NULL;
    unsigned int n = 0, cap = 0;

    free(shape_features);
    shape_features = NULL;
    shape_nfeatures = 0;
    if (!features || !*features)
        return;

    copy = strdup(features);
    if (!copy)
        return;
    for (tok = strtok_r(copy, ", ", &save); tok;
         tok = strtok_r(NULL, ", ", &save)) {
        hb_feature_t f;
        if (!hb_feature_from_string(tok, -1, &f))
            continue;
        if (n == cap) {
            unsigned int nc = cap ? cap * 2 : 4;
            hb_feature_t *nl = (hb_feature_t *)realloc(list,
                                                       nc * sizeof(*nl));
            if (!nl)
                break;
            list = nl;
            cap = nc;
        }
        list[n++] = f;
    }
    free(copy);
    shape_features = list;
    shape_nfeatures = n;
}

static int same_size(double a, double b)
{
    return fabs(a - b) < 0.05;
}

static ShapingFont *font_lookup(const char *family, int style, double size)
{
    ShapingFont *f;
    for (f = font_cache; f; f = f->next) {
        if (f->style == style && same_size(f->size, size) &&
            strcmp(f->family, family) == 0)
            return f;
    }
    return NULL;
}

static char *fc_family_for(const char *family, int style, double size,
                           char **path_out, int *index_out)
{
    FcPattern *pat, *match;
    FcResult result;
    FcChar8 *file = NULL, *matched_family = NULL;
    int index = 0;
    char *out = NULL;

    pat = FcPatternCreate();
    if (!pat)
        return NULL;
    FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)family);
    FcPatternAddInteger(pat, FC_WEIGHT,
                        (style & 1) ? FC_WEIGHT_BOLD : FC_WEIGHT_REGULAR);
    FcPatternAddInteger(pat, FC_SLANT,
                        (style & 2) ? FC_SLANT_ITALIC : FC_SLANT_ROMAN);
    FcPatternAddDouble(pat, FC_SIZE, size);
    FcPatternAddBool(pat, FC_SCALABLE, FcTrue);
    FcConfigSubstitute(NULL, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    match = FcFontMatch(NULL, pat, &result);
    FcPatternDestroy(pat);
    if (!match)
        return NULL;

    FcPatternGetString(match, FC_FILE, 0, &file);
    FcPatternGetInteger(match, FC_INDEX, 0, &index);
    FcPatternGetString(match, FC_FAMILY, 0, &matched_family);
    if (file)
        *path_out = strdup((const char *)file);
    if (index_out)
        *index_out = index;
    if (matched_family)
        out = strdup((const char *)matched_family);
    FcPatternDestroy(match);
    return out;
}

ShapingFont *shaping_font_get(const char *family, int style, double size)
{
    ShapingFont *f;
    char *path = NULL, *matched = NULL;
    int index = 0;
    FT_Face ft = NULL;
    hb_font_t *hb = NULL;
    cairo_font_face_t *cairo_face = NULL;

    if (!family || !*family)
        family = "Sans";
    if (size <= 0.0)
        size = 12.0;
    if (!shaping_ensure_init())
        return NULL;

    f = font_lookup(family, style, size);
    if (f)
        return f;

    matched = fc_family_for(family, style, size, &path, &index);
    if (!path)
        goto fail;
    if (FT_New_Face(ft_lib, path, index, &ft) != 0)
        goto fail;
    FT_Set_Char_Size(ft, 0, (FT_F26Dot6)(size * 64.0), 0, 0);

    /* Use the OpenType tables directly instead of hb-ft's font funcs.
     * The FT_Face is shared with the cairo font face, and cairo resets the
     * FT char size for the target DPI while drawing.  hb-ft reads advances
     * through the FT face, so after a draw at e.g. 120 dpi the next shaping
     * pass returned advances 120/72 times too wide and words broke apart. */
    {
        hb_face_t *hb_face = hb_ft_face_create_referenced(ft);
        if (!hb_face)
            goto fail;
        hb = hb_font_create(hb_face);
        hb_face_destroy(hb_face);
        if (!hb)
            goto fail;
        hb_ot_font_set_funcs(hb);
        hb_font_set_scale(hb, (int)(size * 64.0), (int)(size * 64.0));
        /* Prefer real OpenType kerning/ligatures when the font has them. */
        hb_font_set_ppem(hb, 0, 0);
    }

    cairo_face = cairo_ft_font_face_create_for_ft_face(ft, 0);
    if (!cairo_face || cairo_font_face_status(cairo_face) != CAIRO_STATUS_SUCCESS)
        goto fail;

    f = (ShapingFont *)calloc(1, sizeof(*f));
    if (!f)
        goto fail;
    f->ft = ft;
    f->hb = hb;
    f->cairo_face = cairo_face;
    f->size = size;
    f->style = style;
    /* Cache key must be the requested family; fontconfig's matched family
     * ("DejaVu Sans" for "Sans", "EB Garamond 12" for "EB Garamond") would
     * never match the lookup key and would create a new FT face per word. */
    f->family = strdup(family);
    f->path = path;
    f->index = index;
    f->next = font_cache;
    font_cache = f;
    free(matched);
    return f;

fail:
    if (cairo_face)
        cairo_font_face_destroy(cairo_face);
    if (hb)
        hb_font_destroy(hb);
    if (ft)
        FT_Done_Face(ft);
    free(path);
    free(matched);
    return NULL;
}

void shaping_font_metrics(ShapingFont *font, double *ascent, double *descent)
{
    double a = 0.0, d = 0.0;
    if (font && font->ft) {
        double upem = font->ft->units_per_EM;
        if (upem <= 0.0)
            upem = 1000.0;
        a = (double)font->ft->ascender * font->size / upem;
        d = -(double)font->ft->descender * font->size / upem;
    }
    if (a < 0.0) a = 0.0;
    if (d < 0.0) d = 0.0;
    if (ascent) *ascent = a;
    if (descent) *descent = d;
}

static void cluster_init_metrics(ShapedCluster *c, ShapingFont *font)
{
    double a, d;
    shaping_font_metrics(font, &a, &d);
    c->height = a;
    c->depth = d;
}

ShapedCluster *shaping_shape_text_ex(ShapingFont *font, const char *text,
                                     int len, int base_offset,
                                     const char *features,
                                     int *out_nclusters)
{
    hb_feature_t *feats = NULL;
    unsigned int nfeats = 0;
    hb_buffer_t *buf;
    hb_glyph_info_t *info;
    hb_glyph_position_t *pos;
    unsigned int count, i, start;
    ShapedCluster *clusters = NULL;
    int n = 0, cap = 0;

    if (out_nclusters) *out_nclusters = 0;
    if (!font || !font->hb || !text || len <= 0)
        return NULL;

    buf = hb_buffer_create();
    if (!buf)
        return NULL;
    hb_buffer_add_utf8(buf, text, len, 0, len);
    hb_buffer_guess_segment_properties(buf);
    if (shape_language)
        hb_buffer_set_language(buf, shape_language);
    if (features && *features)
        feats = parse_features(features, &nfeats);
    else {
        feats = shape_features;
        nfeats = shape_nfeatures;
    }
    hb_shape(font->hb, buf, feats, nfeats);
    info = hb_buffer_get_glyph_infos(buf, &count);
    pos = hb_buffer_get_glyph_positions(buf, &count);
    if (!info || !pos || count == 0) {
        hb_buffer_destroy(buf);
        return NULL;
    }

    for (start = 0; start < count; start = i) {
        int cluster = (int)info[start].cluster;
        ShapedCluster c;
        unsigned int k;
        double width = 0.0;

        i = start + 1;
        while (i < count && (int)info[i].cluster == cluster)
            i++;

        memset(&c, 0, sizeof(c));
        c.nglyphs = (int)(i - start);
        c.glyphs = (ShapedGlyph *)calloc((size_t)c.nglyphs, sizeof(ShapedGlyph));
        if (!c.glyphs)
            break;
        c.font = font;
        c.x_scale = 1.0;
        c.cp = 0;
        {
            const char *cp = text + cluster;
            if (cluster >= 0 && cluster < len)
                utf8_decode_cp(&cp, &c.cp);
        }
        c.source_start = base_offset + cluster;
        c.source_end = (i < count) ? base_offset + (int)info[i].cluster
                                   : base_offset + len;
        for (k = start; k < i; k++) {
            ShapedGlyph *g = &c.glyphs[k - start];
            g->glyph_id = info[k].codepoint;
            g->x_advance = pos[k].x_advance / 64.0;
            g->y_advance = pos[k].y_advance / 64.0;
            g->x_offset = pos[k].x_offset / 64.0;
            g->y_offset = pos[k].y_offset / 64.0;
            width += g->x_advance;
        }
        c.width = width;
        cluster_init_metrics(&c, font);

        if (n == cap) {
            int nc = cap ? cap * 2 : 8;
            ShapedCluster *nm = (ShapedCluster *)realloc(clusters,
                                                         (size_t)nc * sizeof(*nm));
            if (!nm) {
                free(c.glyphs);
                break;
            }
            clusters = nm;
            cap = nc;
        }
        clusters[n++] = c;
    }

    hb_buffer_destroy(buf);
    if (out_nclusters) *out_nclusters = n;
    return clusters;
}

void shaping_cluster_free(ShapedCluster *cluster)
{
    if (!cluster)
        return;
    free(cluster->glyphs);
    cluster->glyphs = NULL;
    cluster->nglyphs = 0;
    free(cluster);
}

ShapedCluster *shaping_shape_text(ShapingFont *font, const char *text, int len,
                                  int base_offset, int *out_nclusters)
{
    return shaping_shape_text_ex(font, text, len, base_offset, NULL,
                                 out_nclusters);
}

int shaping_append_run_ex(int node_list, ShapingFont *font, const char *text,
                          int len, int base_offset, float size, int style,
                          int face_handle, int reverse_mode,
                          const char *features, double *out_width)
{
    ShapedCluster *clusters;
    int nclusters = 0, i;

    if (!font || !text || len <= 0)
        return 0;
    clusters = shaping_shape_text_ex(font, text, len, base_offset, features,
                                     &nclusters);
    if (!clusters || nclusters <= 0) {
        shaping_clusters_free(clusters, nclusters);
        return 0;
    }
    for (i = 0; i < nclusters; i++) {
        ShapedCluster *c = (ShapedCluster *)malloc(sizeof(*c));
        Node *cn;
        if (!c)
            break;
        *c = clusters[i];
        clusters[i].glyphs = NULL;
        node_create_char(node_list, (int)c->cp, FROM_DOUBLE(c->width),
                         FROM_DOUBLE(c->height), FROM_DOUBLE(c->depth),
                         size, style, face_handle, reverse_mode);
        cn = (Node *)mls(node_list, m_len(node_list) - 1);
        cn->shaped = c;
        cn->source_offset = c->source_start;
        cn->byte_len = c->source_end - c->source_start;
        if (out_width)
            *out_width += c->width;
    }
    shaping_clusters_free(clusters, nclusters);
    return 1;
}

int shaping_append_run(int node_list, ShapingFont *font, const char *text,
                       int len, int base_offset, float size, int style,
                       int face_handle, int reverse_mode, double *out_width)
{
    return shaping_append_run_ex(node_list, font, text, len, base_offset,
                                 size, style, face_handle, reverse_mode,
                                 NULL, out_width);
}

void shaping_clusters_free(ShapedCluster *clusters, int nclusters)
{
    int i;
    if (!clusters)
        return;
    for (i = 0; i < nclusters; i++)
        free(clusters[i].glyphs);
    free(clusters);
}

void shaping_draw_cluster(void *cairo_cr, ShapingFont *font, double x, double y,
                          const ShapedCluster *cluster)
{
    cairo_t *cr = (cairo_t *)cairo_cr;
    cairo_glyph_t *glyphs;
    double cx;
    int i;

    if (!cr || !font || !cluster || cluster->nglyphs <= 0)
        return;

    glyphs = (cairo_glyph_t *)malloc((size_t)cluster->nglyphs *
                                     sizeof(cairo_glyph_t));
    if (!glyphs)
        return;
    cx = 0.0;
    for (i = 0; i < cluster->nglyphs; i++) {
        const ShapedGlyph *g = &cluster->glyphs[i];
        glyphs[i].index = (unsigned long)g->glyph_id;
        glyphs[i].x = cx + g->x_offset;
        glyphs[i].y = y - g->y_offset;
        cx += g->x_advance;
    }

    cairo_save(cr);
    cairo_set_font_face(cr, font->cairo_face);
    cairo_set_font_size(cr, font->size);
    cairo_translate(cr, x, 0.0);
    if (cluster->x_scale != 1.0)
        cairo_scale(cr, cluster->x_scale, 1.0);
    cairo_show_glyphs(cr, glyphs, cluster->nglyphs);
    cairo_restore(cr);
    free(glyphs);
}

void shaping_shutdown(void)
{
    ShapingFont *f = font_cache;
    while (f) {
        ShapingFont *next = f->next;
        if (f->cairo_face)
            cairo_font_face_destroy(f->cairo_face);
        if (f->hb)
            hb_font_destroy(f->hb);
        if (f->ft)
            FT_Done_Face(f->ft);
        free(f->family);
        free(f->path);
        free(f);
        f = next;
    }
    font_cache = NULL;
    free(shape_features);
    shape_features = NULL;
    shape_nfeatures = 0;
    {
        FeatureCache *fc = feature_cache;
        while (fc) {
            FeatureCache *next = fc->next;
            free(fc->key);
            free(fc->features);
            free(fc);
            fc = next;
        }
        feature_cache = NULL;
    }
    if (ft_ready) {
        FT_Done_FreeType(ft_lib);
        ft_ready = 0;
    }
}
