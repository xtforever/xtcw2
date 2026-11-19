#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <dlfcn.h>

#include "hyphen.h"

typedef void HyphenDict;

typedef HyphenDict *(*hyphen_load_fn)(const char *);
typedef void (*hyphen_free_fn)(HyphenDict *);
typedef int (*hyphenate_fn)(HyphenDict *, const char *, int, char *, char *,
                            char ***, int **, int **);

static void *hyphen_lib;
static HyphenDict *hyphen_dict;
static hyphen_load_fn hyphen_load;
static hyphen_free_fn hyphen_free;
static hyphenate_fn hyphenate;
static char current_language[64];
static int init_tried;
static int init_ok;

static int file_exists(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return 0;
    fclose(f);
    return 1;
}

static void strip_language(const char *in, char *out, size_t out_size)
{
    size_t n = 0;
    if (!in)
        in = "";
    while (*in && *in != '.' && *in != '@' && n + 1 < out_size) {
        out[n++] = (*in == '-') ? '_' : *in;
        in++;
    }
    out[n] = 0;
}

static HyphenDict *load_dictionary(const char *language)
{
    char lang[64], base[64], path[512];
    char *dot;

    strip_language(language, lang, sizeof(lang));
    if (!lang[0])
        return NULL;

    snprintf(base, sizeof(base), "%s", lang);
    dot = strchr(base, '_');
    if (dot)
        *dot = 0;

    const char *dirs[] = { "/usr/share/hyphen", "/usr/local/share/hyphen" };
    size_t d;

    for (d = 0; d < sizeof(dirs) / sizeof(dirs[0]); d++) {
        snprintf(path, sizeof(path), "%s/hyph_%s.dic", dirs[d], lang);
        if (file_exists(path))
            return hyphen_load(path);
        if (strcmp(base, lang) != 0) {
            snprintf(path, sizeof(path), "%s/hyph_%s.dic", dirs[d], base);
            if (file_exists(path))
                return hyphen_load(path);
        }
    }

    /* Sensible fallback for English UI text. */
    for (d = 0; d < sizeof(dirs) / sizeof(dirs[0]); d++) {
        snprintf(path, sizeof(path), "%s/hyph_en_US.dic", dirs[d]);
        if (file_exists(path))
            return hyphen_load(path);
    }
    return NULL;
}

int hyphen_init(void)
{
    if (hyphen_dict)
        return 1;
    if (init_tried)
        return init_ok;
    init_tried = 1;

    hyphen_lib = dlopen("libhyphen.so.0", RTLD_LAZY | RTLD_LOCAL);
    if (!hyphen_lib)
        hyphen_lib = dlopen("libhyphen.so", RTLD_LAZY | RTLD_LOCAL);
    if (!hyphen_lib)
        return 0;

    hyphen_load = (hyphen_load_fn)dlsym(hyphen_lib, "hnj_hyphen_load");
    hyphen_free = (hyphen_free_fn)dlsym(hyphen_lib, "hnj_hyphen_free");
    hyphenate = (hyphenate_fn)dlsym(hyphen_lib, "hnj_hyphen_hyphenate2");
    if (!hyphen_load || !hyphen_free || !hyphenate)
        return 0;

    const char *lang = current_language[0] ? current_language : NULL;
    if (!lang) {
        lang = getenv("LC_ALL");
        if (!lang || !*lang) lang = getenv("LC_CTYPE");
        if (!lang || !*lang) lang = getenv("LANG");
    }
    hyphen_dict = load_dictionary(lang ? lang : "en_US");
    init_ok = hyphen_dict != NULL;
    return init_ok;
}

void hyphen_set_language(const char *lang)
{
    strip_language(lang, current_language, sizeof(current_language));
    init_tried = 0;
    init_ok = 0;
    if (hyphen_dict && hyphen_free) {
        hyphen_free(hyphen_dict);
        hyphen_dict = NULL;
    }
    if (current_language[0])
        hyphen_init();
}

void hyphen_shutdown(void)
{
    if (hyphen_dict && hyphen_free)
        hyphen_free(hyphen_dict);
    hyphen_dict = NULL;
    if (hyphen_lib)
        dlclose(hyphen_lib);
    hyphen_lib = NULL;
    hyphen_load = NULL;
    hyphen_free = NULL;
    hyphenate = NULL;
}

int *hyphen_breaks(const char *word, int len, int *out_count)
{
    char *hyphens, *hyphenated;
    char **rep = NULL;
    int *pos = NULL, *cut = NULL;
    int *breaks = NULL;
    int n = 0, cap = 0, i, src = 0;

    if (out_count) *out_count = 0;
    if (!hyphen_dict && !hyphen_init())
        return NULL;
    if (!hyphen_dict || !hyphenate || !word || len <= 1)
        return NULL;

    hyphens = (char *)calloc((size_t)len + 32, 1);
    hyphenated = (char *)calloc((size_t)len + 32, 1);
    if (!hyphens || !hyphenated) {
        free(hyphens);
        free(hyphenated);
        return NULL;
    }

    hyphenated[len + 31] = 0;
    hyphens[len + 31] = 0;
    if (hyphenate(hyphen_dict, word, len, hyphens, hyphenated,
                  &rep, &pos, &cut) == 0) {
        for (i = 0; i < len + 31 && hyphenated[i]; i++) {
            if (hyphenated[i] == '=') {
                if (src > 0 && src < len) {
                    if (n == cap) {
                        int nc = cap ? cap * 2 : 4;
                        int *nb = (int *)realloc(breaks,
                                                 (size_t)nc * sizeof(int));
                        if (!nb)
                            break;
                        breaks = nb;
                        cap = nc;
                    }
                    breaks[n++] = src;
                }
            } else {
                src++;
            }
        }
    }

    free(hyphens);
    free(hyphenated);
    if (out_count) *out_count = n;
    return breaks;
}
