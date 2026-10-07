/*
 * theme_parse.c - parser for the hdf theme s-expression format.
 *
 *   (theme NAME (name value name value ...))
 *
 * value := 0xRRGGBB | #RRGGBB  -> colour
 *        | [-]<digits>        -> integer
 */
#include "theme_parse.h"
#include "mls.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define TOK_MAX 256

static const char *skip_ws(const char *p)
{
    for (;;) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == ';') { while (*p && *p != '\n') p++; continue; }
        break;
    }
    return p;
}

/* read the next token (up to whitespace or a paren); returns length */
static int read_token(const char **pp, char *out, int max)
{
    const char *p = skip_ws(*pp);
    int n = 0;

    while (*p && !isspace((unsigned char)*p) && *p != '(' && *p != ')') {
        if (n < max - 1) out[n++] = *p;
        p++;
    }
    out[n] = '\0';
    *pp = p;
    return n;
}

static int parse_color(const char *s, uint32_t *out)
{
    char *end;
    unsigned long v;

    if (s[0] == '#') s++;
    else if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;
    else return 0;

    if (!*s) return 0;
    v = strtoul(s, &end, 16);
    if (end == s || *end != '\0') return 0;
    *out = (uint32_t)v;
    return 1;
}

int theme_parse_string(const char *buf)
{
    const char *p = buf;
    char tok[TOK_MAX], name[TOK_MAX];
    int count = 0;

    for (;;) {
        p = skip_ws(p);
        if (!*p) break;

        if (*p != '(') { p++; continue; }   /* skip stray characters */
        p++;

        if (!read_token(&p, tok, sizeof tok)) break;
        if (strcmp(tok, "theme") != 0) {
            /* not a theme block: skip to its closing paren (best effort) */
            int depth = 0;
            while (*p) {
                if (*p == '(') depth++;
                else if (*p == ')') { if (depth == 0) { p++; break; } depth--; }
                p++;
            }
            continue;
        }

        if (!read_token(&p, name, sizeof name)) break;

        p = skip_ws(p);
        if (*p != '(') continue;            /* malformed, skip */
        p++;

        theme_begin(name);

        for (;;) {
            char cname[TOK_MAX], cval[TOK_MAX];
            uint32_t rgb;

            p = skip_ws(p);
            if (*p == ')') { p++; break; }
            if (!*p) break;

            if (!read_token(&p, cname, sizeof cname)) break;
            p = skip_ws(p);
            if (*p == ')') { p++; break; }  /* dangling name, ignore */
            if (!read_token(&p, cval, sizeof cval)) break;

            if (parse_color(cval, &rgb)) {
                theme_color(cname, rgb);
            } else {
                char *end;
                long v = strtol(cval, &end, 10);
                if (end != cval && *end == '\0') theme_int(cname, (int)v);
            }
        }

        theme_end();

        p = skip_ws(p);
        if (*p == ')') p++;                 /* close theme block */
        count++;
    }

    return count;
}

int theme_load(const char *file)
{
    FILE *f;
    long sz;
    char *buf;
    int n;

    if (!file) return 0;
    f = fopen(file, "r");
    if (!f) { WARN("theme file not found: %s", file); return 0; }

    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return 0; }

    buf = (char*) malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return 0; }

    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        /* short read is tolerated */
    }
    buf[sz] = '\0';
    fclose(f);

    n = theme_parse_string(buf);
    free(buf);
    return n;
}
