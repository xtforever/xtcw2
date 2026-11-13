/*
 * theme.c - central theme / colour registry.
 *
 * Storage is a global set of MLS lists:
 *   themes       : list of struct theme_entry { char *name; int map; }
 *   map          : list of struct theme_value
 *   active_map   : deep copy of the selected theme's map
 *   widgets      : list of struct theme_widget { Widget w; theme_apply_fn fn; }
 */
#include "theme.h"
#include "mls.h"
#include "XCC.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* one named value inside a theme: a colour, an integer, or a string */
struct theme_value {
    char   *name;
    int     is_color;   /* 1 = colour */
    int     is_str;     /* 1 = string (sval); 0 otherwise */
    uint32_t rgb;       /* valid when is_color */
    int     ival;       /* valid when !is_color && !is_str */
    char   *sval;       /* valid when is_str; heap-owned (strdup) */
    Pixel   pixel;      /* cached Pixel (colours only) */
    int     pixel_valid;
};

struct theme_entry {
    char *name;
    int   map;          /* MLS of struct theme_value */
};

struct theme_widget {
    Widget         w;
    theme_apply_fn fn;
};

/* --- global state ------------------------------------------------------- */

static int         display_ready = 0;
static int         active_map = 0;
static int         themes = 0;
static int         building = 0;
static char       *building_name = NULL;
static char       *active_name = NULL;
static int         widgets = 0;

static Display    *dpy = NULL;
static int         screen = 0;
static Colormap    cmap = 0;
static XCC         xcc = NULL;

/* --- helpers ------------------------------------------------------------ */

static void ensure_store(void)
{
    if (!themes)  themes  = m_create(8,  sizeof(struct theme_entry));
    if (!widgets) widgets = m_create(16, sizeof(struct theme_widget));
}

static void free_map(int map)
{
    struct theme_value *v;
    int i;
    if (!map) return;
    for (i = 0; i < m_len(map); i++) {
        v = mls(map, i);
        if (v->name) { free(v->name); v->name = NULL; }
        if (v->sval) { free(v->sval); v->sval = NULL; }
    }
    m_free(map);
}

static struct theme_value *map_lookup(int map, const char *name)
{
    struct theme_value *v;
    int i;
    if (!map || !name) return NULL;
    for (i = 0; i < m_len(map); i++) {
        v = mls(map, i);
        if (v->name && strcmp(v->name, name) == 0) return v;
    }
    return NULL;
}

static struct theme_value *active_lookup(const char *name)
{
    return map_lookup(active_map, name);
}

static int find_theme_map(const char *name)
{
    struct theme_entry *e;
    int i;
    if (!themes || !name) return 0;
    for (i = 0; i < m_len(themes); i++) {
        e = mls(themes, i);
        if (e->name && strcmp(e->name, name) == 0) return e->map;
    }
    return 0;
}

static struct theme_value *append_value(const char *name)
{
    struct theme_value *v;
    if (!building) theme_begin("");

    /* Last assignment wins: replace an existing value with the same name
     * instead of appending a second entry (the old first-wins behaviour made
     * a duplicated key silently ineffective). */
    v = map_lookup(building, name ? name : "");
    if (v) {
        if (v->sval) { free(v->sval); v->sval = NULL; }
        v->is_color = 0;
        v->is_str = 0;
        v->rgb = 0;
        v->ival = 0;
        v->pixel = 0;
        v->pixel_valid = 0;
        return v;
    }

    v = mls(building, m_new(building, 1));
    v->name = strdup(name ? name : "");
    v->is_color = 0;
    v->is_str = 0;
    v->rgb = 0;
    v->ival = 0;
    v->sval = NULL;
    v->pixel = 0;
    v->pixel_valid = 0;
    return v;
}

/* --- lifecycle ---------------------------------------------------------- */

void theme_init(Widget top)
{
    Screen *scr = XtScreen(top);

    if (xcc) { XCCFree(xcc); xcc = NULL; }
    display_ready = 0;

    dpy = DisplayOfScreen(scr);
    screen = XScreenNumberOfScreen(scr);
    cmap = DefaultColormapOfScreen(scr);

    xcc = XCCCreate(dpy, DefaultVisualOfScreen(scr), False, False, None, &cmap);
    if (xcc) display_ready = 1;

    ensure_store();
}

void theme_destroy(void)
{
    struct theme_entry *e;
    int i;

    display_ready = 0;

    if (xcc) { XCCFree(xcc); xcc = NULL; }

    if (building)      { free_map(building); building = 0; }
    if (building_name) { free(building_name); building_name = NULL; }
    if (active_name)   { free(active_name); active_name = NULL; }
    if (active_map)    { free_map(active_map); active_map = 0; }

    if (themes) {
        for (i = 0; i < m_len(themes); i++) {
            e = mls(themes, i);
            if (e->map) free_map(e->map);
            if (e->name) free(e->name);
        }
        m_free(themes);
        themes = 0;
    }

    if (widgets) { m_free(widgets); widgets = 0; }
}

/* --- building ----------------------------------------------------------- */

void theme_begin(const char *name)
{
    ensure_store();

    /* discard any unfinished build */
    if (building)      { free_map(building); building = 0; }
    if (building_name) { free(building_name); building_name = NULL; }

    building = m_create(8, sizeof(struct theme_value));
    building_name = strdup(name ? name : "");
}

void theme_color(const char *name, uint32_t rgb)
{
    struct theme_value *v = append_value(name);
    v->is_color = 1;
    v->is_str = 0;
    v->rgb = rgb;
}

void theme_int(const char *name, int value)
{
    struct theme_value *v = append_value(name);
    v->is_color = 0;
    v->is_str = 0;
    v->ival = value;
}

void theme_str(const char *name, const char *value)
{
    struct theme_value *v = append_value(name);
    v->is_color = 0;
    v->is_str = 1;
    if (v->sval) free(v->sval);
    v->sval = strdup(value ? value : "");
}

void theme_end(void)
{
    struct theme_entry *e;
    int i, found = -1;

    if (!building || !building_name) return;
    ensure_store();

    for (i = 0; i < m_len(themes); i++) {
        e = mls(themes, i);
        if (e->name && strcmp(e->name, building_name) == 0) { found = i; break; }
    }

    if (found >= 0) {
        e = mls(themes, found);
        free_map(e->map);            /* drop old values */
        e->map = building;
        free(building_name);         /* keep e->name, drop the new copy */
    } else {
        e = mls(themes, m_new(themes, 1));
        e->name = building_name;     /* transfer ownership */
        e->map = building;
    }

    building = 0;
    building_name = NULL;
}

/* --- selecting / querying ---------------------------------------------- */

static void set_active_from(int map)
{
    struct theme_value *src, *dst;
    int i;

    if (active_map) { free_map(active_map); active_map = 0; }
    active_map = m_create(8, sizeof(struct theme_value));
    if (!map) return;

    for (i = 0; i < m_len(map); i++) {
        src = mls(map, i);
        dst = mls(active_map, m_new(active_map, 1));
        dst->name = strdup(src->name ? src->name : "");
        dst->is_color = src->is_color;
        dst->is_str = src->is_str;
        dst->rgb = src->rgb;
        dst->ival = src->ival;
        dst->sval = NULL;
        if (src->is_str && src->sval) dst->sval = strdup(src->sval);
        dst->pixel = 0;
        dst->pixel_valid = 0;
    }
}

void theme_select(const char *name)
{
    int map;

    if (!name) return;
    map = find_theme_map(name);
    if (!map) return;                /* unknown -> leave active theme */

    set_active_from(map);
    if (active_name) free(active_name);
    active_name = strdup(name);

    theme_apply();
}

uint32_t theme_get_rgb(const char *name)
{
    struct theme_value *v = active_lookup(name);
    return (v && v->is_color) ? v->rgb : 0;
}

int theme_color_defined(const char *name)
{
    struct theme_value *v = active_lookup(name);
    return (v && v->is_color) ? 1 : 0;
}

int theme_int_defined(const char *name)
{
    struct theme_value *v = active_lookup(name);
    return (v && !v->is_color && !v->is_str) ? 1 : 0;
}

int theme_str_defined(const char *name)
{
    struct theme_value *v = active_lookup(name);
    return (v && v->is_str) ? 1 : 0;
}

int theme_get_int(const char *name, int *out)
{
    struct theme_value *v = active_lookup(name);
    if (v && !v->is_color && !v->is_str) {
        if (out) *out = v->ival;
        return 1;
    }
    return 0;
}

const char *theme_get_str(const char *name)
{
    struct theme_value *v = active_lookup(name);
    return (v && v->is_str) ? v->sval : NULL;
}

const char *theme_active(void)
{
    return active_name;
}

int theme_list(char ***out)
{
    struct theme_entry *e;
    char **arr;
    int i, n = themes ? m_len(themes) : 0;

    arr = (char**) malloc(sizeof(char*) * (size_t)(n + 1));
    if (!arr) { if (out) *out = NULL; return 0; }

    for (i = 0; i < n; i++) {
        e = mls(themes, i);
        arr[i] = e->name;
    }
    arr[n] = NULL;

    if (out) *out = arr; else free(arr);
    return n;
}

Pixel get_color(const char *name)
{
    struct theme_value *v;

    if (!display_ready || !xcc) return 0;
    v = active_lookup(name);
    if (!v || !v->is_color) return 0;

    if (!v->pixel_valid) {
        uint32_t rgb = v->rgb;
        unsigned int r = (rgb >> 16) & 0xff;
        unsigned int g = (rgb >>  8) & 0xff;
        unsigned int b =  rgb        & 0xff;
        v->pixel = XCCGetPixel(xcc, r * 0x101, g * 0x101, b * 0x101);
        v->pixel_valid = 1;
    }
    return v->pixel;
}

/* --- style-guide helpers ------------------------------------------------ */

const char *theme_font_spec(void)
{
    const char *s = theme_get_str("font_ui");
    return (s && *s) ? s : "Sans-13";
}

int theme_font_parts(const char *spec, char *face, int face_size, int *px)
{
    const char *dash, *p;
    size_t n;

    if (!face || face_size <= 0) {
        if (px) *px = 0;
        return 0;
    }
    if (!spec || !*spec) spec = "Sans-13";

    /* find the last '-' whose suffix is all digits */
    dash = NULL;
    for (p = spec; *p; p++) {
        if (*p == '-') {
            const char *q = p + 1;
            if (*q) {
                while (*q >= '0' && *q <= '9') q++;
                if (!*q) dash = p;
            }
        }
    }
    if (!dash) {
        n = strlen(spec);
        if (n >= (size_t)face_size) n = (size_t)face_size - 1;
        memcpy(face, spec, n);
        face[n] = 0;
        if (px) *px = 0;
        return 0;
    }
    n = (size_t)(dash - spec);
    if (n >= (size_t)face_size) n = (size_t)face_size - 1;
    memcpy(face, spec, n);
    face[n] = 0;
    if (px) *px = atoi(dash + 1);
    return 1;
}

int theme_metric(const char *name, int def)
{
    int v;
    if (theme_int_defined(name) && theme_get_int(name, &v))
        return v;
    return def;
}

/* --- widget registration ------------------------------------------------ */

void theme_register(Widget w, theme_apply_fn fn)
{
    struct theme_widget *x;

    if (!w || !fn) return;
    ensure_store();

    x = mls(widgets, m_new(widgets, 1));
    x->w = w;
    x->fn = fn;
}

void theme_unregister(Widget w)
{
    struct theme_widget *x;
    int i;

    if (!widgets) return;

    for (i = 0; i < m_len(widgets); i++) {
        x = mls(widgets, i);
        if (x->w == w) { m_del(widgets, i); return; }
    }
}

void theme_apply(void)
{
    struct theme_widget *snap;
    static int applying = 0;
    int i, n;

    if (!display_ready || !widgets) return;
    if (applying) return;                 /* re-entrant theme_select */

    n = m_len(widgets);
    if (n <= 0) return;

    /* Snapshot the observer list: an apply callback may register or
     * unregister widgets, which would otherwise shift indices mid-loop. */
    snap = (struct theme_widget *)malloc(sizeof(*snap) * (size_t)n);
    if (!snap) return;
    for (i = 0; i < n; i++)
        snap[i] = *(struct theme_widget *)mls(widgets, i);

    applying = 1;
    for (i = 0; i < n; i++) {
        if (snap[i].w && snap[i].fn) snap[i].fn(snap[i].w);
    }
    applying = 0;

    free(snap);
}

/* --- persistence -------------------------------------------------------- */

/* Write |s| escaped for the hdf string syntax (the parser understands
 * \" \\ \n \r \t). */
static void save_escaped(FILE *f, const char *s)
{
    const unsigned char *p = (const unsigned char *)(s ? s : "");

    for (; *p; p++) {
        switch (*p) {
        case '"':  fputs("\\\"", f); break;
        case '\\': fputs("\\\\", f); break;
        case '\n': fputs("\\n", f);  break;
        case '\r': fputs("\\r", f);  break;
        case '\t': fputs("\\t", f);  break;
        default:   fputc(*p, f);     break;
        }
    }
}

int theme_save(const char *file)
{
    struct theme_entry *e;
    struct theme_value *v;
    FILE *f;
    int i, j, ok = 1;

    f = fopen(file, "w");
    if (!f) return 0;

    /* No themes loaded: an empty file is a valid (if useless) store.  Avoid
     * m_len(0), which aborts in the MLS debug build. */
    if (!themes) {
        ok = (fclose(f) == 0);
        return ok;
    }

    for (i = 0; i < m_len(themes); i++) {
        e = mls(themes, i);
        if (fprintf(f, "(theme %s (", e->name) < 0) ok = 0;
        for (j = 0; j < m_len(e->map); j++) {
            v = mls(e->map, j);
            if (v->is_color)
                fprintf(f, "%s 0x%06x ", v->name, (unsigned)(v->rgb & 0xffffff));
            else if (v->is_str) {
                fprintf(f, "%s \"", v->name);
                save_escaped(f, v->sval);
                fputs("\" ", f);
            } else
                fprintf(f, "%s %d ", v->name, v->ival);
        }
        fprintf(f, "))\n");
    }

    if (ferror(f)) ok = 0;
    if (fclose(f) != 0) ok = 0;
    return ok;
}
