/*
 * markdown.c - a small Markdown subset for the Wmarkdown viewer.
 *
 * See markdown.h for the supported syntax.  The parser is deliberately
 * line-oriented: it turns blocks into retex paragraphs separated by blank
 * lines and inline markers into RAtom looks.
 */

#include <stdlib.h>
#include <string.h>

#include "markdown.h"
#include "conststr.h"
#include "utf8.h"
#include "utf32.h"

struct MarkdownDoc {
    RAtom *atoms;
    int natoms;
    int acap;
    RetexParaFormat *fmts;
    int nfmts;
    int fcap;
};

typedef struct {
    RAtom *a;
    int n;
    int cap;
    RetexParaFormat fmt;
} Block;

enum {
    K_NONE = 0,
    K_PARA,
    K_LIST,
    K_QUOTE,
    K_HEADING,
    K_CODE,
    K_RULE
};

static int grow(void **ptr, int *cap, int need, size_t elem)
{
    int nc;
    void *p;

    if (need <= *cap)
        return 1;
    nc = *cap ? *cap * 2 : 32;
    while (nc < need)
        nc *= 2;
    p = realloc(*ptr, (size_t)nc * elem);
    if (!p)
        return 0;
    *ptr = p;
    *cap = nc;
    return 1;
}

static int doc_put(MarkdownDoc *d, uint32_t cp, RLook look)
{
    if (!grow((void **)&d->atoms, &d->acap, d->natoms + 1, sizeof(RAtom)))
        return 0;
    d->atoms[d->natoms].kind = RATOM_CHAR;
    d->atoms[d->natoms].cp = cp;
    d->atoms[d->natoms].look = look;
    d->atoms[d->natoms].payload = 0;
    d->natoms++;
    return 1;
}

static int block_put(Block *b, uint32_t cp, RLook look)
{
    if (!grow((void **)&b->a, &b->cap, b->n + 1, sizeof(RAtom)))
        return 0;
    b->a[b->n].kind = RATOM_CHAR;
    b->a[b->n].cp = cp;
    b->a[b->n].look = look;
    b->a[b->n].payload = 0;
    b->n++;
    return 1;
}

static void block_reset(Block *b)
{
    b->n = 0;
}

static void doc_close_block(MarkdownDoc *d, Block *b, RLook sep_look)
{
    if (b->n <= 0) {
        block_reset(b);
        return;
    }
    if (d->nfmts > 0) {
        doc_put(d, '\n', sep_look);
        doc_put(d, '\n', sep_look);
    }
    if (grow((void **)&d->atoms, &d->acap, d->natoms + b->n, sizeof(RAtom))) {
        memcpy(d->atoms + d->natoms, b->a, (size_t)b->n * sizeof(RAtom));
        d->natoms += b->n;
    }
    if (grow((void **)&d->fmts, &d->fcap, d->nfmts + 1,
             sizeof(RetexParaFormat))) {
        d->fmts[d->nfmts++] = b->fmt;
    }
    block_reset(b);
}

static int is_space(uint32_t c) { return c == ' ' || c == '\t'; }

static int is_word(uint32_t c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') || c >= 0x80;
}

static int line_blank(const uint32_t *c, int s, int e)
{
    int i;
    for (i = s; i < e; i++)
        if (!is_space(c[i]))
            return 0;
    return 1;
}

static int skip_lead(const uint32_t *c, int s, int e, int max)
{
    int p = s;
    while (p < e && p - s < max && c[p] == ' ')
        p++;
    return p;
}

static void inline_put(Block *b, const uint32_t *c, int s, int e,
                       RLook base, int mono_face, double mono_size,
                       int extra_style, int depth)
{
    int i = s;
    int bold = 0, italic = 0, code = 0;

    if (depth > 4)
        depth = 4;

    while (i < e) {
        uint32_t ch = c[i];

        if (ch == '\\' && i + 1 < e) {
            RLook lk = base;
            lk.style |= extra_style | (bold ? 1 : 0) | (italic ? 2 : 0);
            block_put(b, c[i + 1], lk);
            i += 2;
            continue;
        }

        if (ch == '`') {
            code = !code;
            i++;
            continue;
        }

        if (!code && (ch == '*' || ch == '_')) {
            int doubled = (i + 1 < e && c[i + 1] == ch);
            int active = doubled ? bold : italic;
            int boundary;
            if (ch == '*') {
                boundary = 1;
            } else if (active) {
                /* closing underscore: must follow a non-space */
                boundary = (i > s && !is_space(c[i - 1]));
            } else {
                /* opening underscore: must precede text and start a word */
                boundary = (i + 1 < e && !is_space(c[i + 1]) &&
                            (i == s || !is_word(c[i - 1])));
            }
            if (boundary) {
                if (doubled) {
                    bold = !bold;
                    i += 2;
                } else {
                    italic = !italic;
                    i++;
                }
                continue;
            }
        }

        if (!code && (ch == '[' ||
                      (ch == '!' && i + 1 < e && c[i + 1] == '['))) {
            int lb = (ch == '[') ? i : i + 1;
            int close = lb + 1;
            while (close < e && c[close] != ']')
                close++;
            if (close < e && close + 1 < e && c[close + 1] == '(') {
                int url = close + 2;
                while (url < e && c[url] != ')')
                    url++;
                if (url < e) {
                    inline_put(b, c, lb + 1, close, base, mono_face,
                               mono_size,
                               extra_style | (bold ? 1 : 0) | (italic ? 2 : 0),
                               depth + 1);
                    i = url + 1;
                    continue;
                }
            }
        }

        {
            RLook lk = base;
            if (code) {
                lk.face = mono_face;
                lk.size = (float)mono_size;
            }
            lk.style |= extra_style | (bold ? 1 : 0) | (italic ? 2 : 0);
            block_put(b, ch, lk);
        }
        i++;
    }
}

static RetexParaFormat fmt_default(const MarkdownStyle *st)
{
    RetexParaFormat f;
    memset(&f, 0, sizeof(f));
    f.align = st->default_align;
    f.space_after = 6.0;
    return f;
}

static RetexParaFormat fmt_heading(const MarkdownStyle *st, int level)
{
    static const double before[6] = { 14.0, 11.0, 9.0, 7.0, 5.0, 4.0 };
    RetexParaFormat f;
    memset(&f, 0, sizeof(f));
    f.align = RETEX_ALIGN_LEFT;
    f.space_before = before[level - 1];
    f.space_after = 4.0;
    return f;
}

static RetexParaFormat fmt_list(const MarkdownStyle *st, int depth, int ordered)
{
    RetexParaFormat f;
    double hang = ordered ? 20.0 : 14.0;
    memset(&f, 0, sizeof(f));
    f.align = RETEX_ALIGN_LEFT;
    f.indent_left = depth * 18.0 + hang;
    f.first_line = -hang;
    f.space_after = 2.0;
    return f;
}

static RetexParaFormat fmt_quote(const MarkdownStyle *st)
{
    RetexParaFormat f;
    memset(&f, 0, sizeof(f));
    f.align = st->default_align;
    f.indent_left = 18.0;
    f.space_after = 6.0;
    return f;
}

static RetexParaFormat fmt_code(const MarkdownStyle *st)
{
    RetexParaFormat f;
    memset(&f, 0, sizeof(f));
    f.align = RETEX_ALIGN_LEFT;
    f.indent_left = 12.0;
    f.indent_right = 12.0;
    f.space_before = 6.0;
    f.space_after = 8.0;
    return f;
}

static RetexParaFormat fmt_rule(const MarkdownStyle *st)
{
    RetexParaFormat f;
    memset(&f, 0, sizeof(f));
    f.align = RETEX_ALIGN_LEFT;
    f.space_before = 6.0;
    f.space_after = 8.0;
    return f;
}

MarkdownDoc *markdown_parse(const char *utf8, const MarkdownStyle *style)
{
    MarkdownDoc *d;
    MarkdownStyle defstyle;
    MarkdownStyle *st = &defstyle;
    uint32_t *cps = NULL;
    int n = 0, i = 0;
    Block cur;
    int kind = K_NONE;
    int cur_hard = 0;
    int in_fence = 0;
    int fence_ch = 0, fence_len = 0;
    RLook base, code_look;
    int mono_face;
    double mono_size;

    if (!style) {
        memset(&defstyle, 0, sizeof(defstyle));
        defstyle.font_face = "Sans";
        defstyle.font_size = 13.0;
        defstyle.mono_face = "Mono";
        defstyle.default_align = RETEX_ALIGN_JUSTIFY;
    } else {
        defstyle = *style;
        if (!defstyle.font_face || !*defstyle.font_face)
            defstyle.font_face = "Sans";
        if (defstyle.font_size <= 0.0)
            defstyle.font_size = 13.0;
        if (!defstyle.mono_face || !*defstyle.mono_face)
            defstyle.mono_face = "Mono";
    }

    if (!utf8 || !*utf8)
        return NULL;

    n = utf32_count_utf8(utf8);
    if (n < 0) {
        /* Invalid UTF-8: treat the bytes as Latin-1 so help text still shows. */
        size_t len = strlen(utf8);
        size_t k;
        cps = (uint32_t *)malloc((len ? len : 1) * sizeof(uint32_t));
        if (!cps)
            return NULL;
        for (k = 0; k < len; k++)
            cps[k] = (unsigned char)utf8[k];
        n = (int)len;
    } else if (n > 0) {
        cps = (uint32_t *)malloc((size_t)n * sizeof(uint32_t));
        if (!cps)
            return NULL;
        n = utf32_from_utf8(utf8, cps, n);
        if (n < 0) {
            free(cps);
            return NULL;
        }
    }

    d = (MarkdownDoc *)calloc(1, sizeof(MarkdownDoc));
    if (!d) {
        free(cps);
        return NULL;
    }

    memset(&cur, 0, sizeof(cur));
    cur.fmt = fmt_default(st);

    base.face = conststr_lookup_c(st->font_face);
    base.style = 0;
    base.size = (float)st->font_size;
    mono_face = conststr_lookup_c(st->mono_face);
    mono_size = st->font_size * 0.95;
    code_look = base;
    code_look.face = mono_face;
    code_look.size = (float)mono_size;

    while (i <= n) {
        int j = i;
        int s, e, p;

        while (j < n && cps[j] != '\n')
            j++;
        s = i;
        e = j;

        if (in_fence) {
            int q, close = 1, k;
            p = skip_lead(cps, s, e, 3);
            q = p;
            while (q < e && cps[q] == (uint32_t)fence_ch)
                q++;
            if (q - p < fence_len)
                close = 0;
            for (k = q; k < e && close; k++)
                if (!is_space(cps[k]))
                    close = 0;
            if (close) {
                in_fence = 0;
                doc_close_block(d, &cur, base);
                kind = K_NONE;
            } else {
                int k;
                if (kind != K_CODE) {
                    doc_close_block(d, &cur, base);
                    cur.fmt = fmt_code(st);
                    kind = K_CODE;
                }
                if (cur.n > 0)
                    block_put(&cur, '\n', code_look);
                for (k = s; k < e; k++)
                    block_put(&cur, cps[k], code_look);
            }
            goto next_line;
        }

        if (line_blank(cps, s, e)) {
            doc_close_block(d, &cur, base);
            kind = K_NONE;
            goto next_line;
        }

        /* fenced code start */
        p = skip_lead(cps, s, e, 3);
        if (p < e && (cps[p] == '`' || cps[p] == '~')) {
            int q = p;
            while (q < e && cps[q] == cps[p])
                q++;
            if (q - p >= 3) {
                doc_close_block(d, &cur, base);
                kind = K_NONE;
                in_fence = 1;
                fence_ch = (int)cps[p];
                fence_len = q - p;
                goto next_line;
            }
        }

        /* ATX heading */
        p = skip_lead(cps, s, e, 3);
        {
            int level = 0;
            while (p < e && cps[p] == '#' && level < 6) {
                level++;
                p++;
            }
            if (level > 0 && (p == e || is_space(cps[p]))) {
                static const double scale[6] = { 1.80, 1.50, 1.30, 1.16,
                                                 1.08, 1.00 };
                int ce = e;
                RLook hlook;
                while (p < e && is_space(cps[p]))
                    p++;
                while (ce > p && is_space(cps[ce - 1]))
                    ce--;
                while (ce > p && cps[ce - 1] == '#')
                    ce--;
                while (ce > p && is_space(cps[ce - 1]))
                    ce--;
                doc_close_block(d, &cur, base);
                cur.fmt = fmt_heading(st, level);
                kind = K_HEADING;
                hlook = base;
                hlook.size = (float)(st->font_size * scale[level - 1]);
                inline_put(&cur, cps, p, ce, hlook, mono_face, mono_size,
                           1, 0);
                doc_close_block(d, &cur, base);
                kind = K_NONE;
                goto next_line;
            }
        }

        /* horizontal rule */
        p = skip_lead(cps, s, e, 3);
        if (p < e && (cps[p] == '-' || cps[p] == '*' || cps[p] == '_')) {
            int ch = (int)cps[p];
            int count = 0, ok = 1, k;
            for (k = p; k < e; k++) {
                if ((int)cps[k] == ch)
                    count++;
                else if (!is_space(cps[k]))
                    ok = 0;
            }
            if (ok && count >= 3) {
                int r;
                doc_close_block(d, &cur, base);
                cur.fmt = fmt_rule(st);
                kind = K_RULE;
                for (r = 0; r < 72; r++)
                    block_put(&cur, '-', base);
                doc_close_block(d, &cur, base);
                kind = K_NONE;
                goto next_line;
            }
        }

        /* block quote */
        p = skip_lead(cps, s, e, 3);
        if (p < e && cps[p] == '>') {
            p++;
            if (p < e && is_space(cps[p]))
                p++;
            if (line_blank(cps, p, e)) {
                doc_close_block(d, &cur, base);
                kind = K_NONE;
                goto next_line;
            }
            if (kind != K_QUOTE) {
                doc_close_block(d, &cur, base);
                cur.fmt = fmt_quote(st);
                kind = K_QUOTE;
            } else {
                block_put(&cur, ' ', base);
            }
            inline_put(&cur, cps, p, e, base, mono_face, mono_size, 0, 0);
            goto next_line;
        }

        /* unordered list item */
        {
            int lead = 0, depth;
            while (s + lead < e && cps[s + lead] == ' ')
                lead++;
            p = s + lead;
            if (p < e && (cps[p] == '-' || cps[p] == '*' || cps[p] == '+') &&
                (p + 1 == e || is_space(cps[p + 1]))) {
                depth = lead / 2;
                if (depth > 8)
                    depth = 8;
                doc_close_block(d, &cur, base);
                cur.fmt = fmt_list(st, depth, 0);
                kind = K_LIST;
                p++;
                while (p < e && is_space(cps[p]))
                    p++;
                block_put(&cur, 0x2022, base);
                block_put(&cur, ' ', base);
                block_put(&cur, ' ', base);
                inline_put(&cur, cps, p, e, base, mono_face, mono_size, 0, 0);
                goto next_line;
            }
        }

        /* ordered list item */
        {
            int lead = 0, depth;
            while (s + lead < e && cps[s + lead] == ' ')
                lead++;
            p = s + lead;
            if (p < e && cps[p] >= '0' && cps[p] <= '9') {
                int ds = p, k;
                while (p < e && cps[p] >= '0' && cps[p] <= '9' && p - ds < 9)
                    p++;
                if (p < e && (cps[p] == '.' || cps[p] == ')') &&
                    (p + 1 == e || is_space(cps[p + 1]))) {
                    depth = lead / 2;
                    if (depth > 8)
                        depth = 8;
                    doc_close_block(d, &cur, base);
                    cur.fmt = fmt_list(st, depth, 1);
                    kind = K_LIST;
                    for (k = ds; k <= p; k++)
                        block_put(&cur, cps[k], base);
                    p++;
                    while (p < e && is_space(cps[p]))
                        p++;
                    block_put(&cur, ' ', base);
                    inline_put(&cur, cps, p, e, base, mono_face, mono_size,
                               0, 0);
                    goto next_line;
                }
            }
        }

        /* ordinary paragraph line (or a lazy continuation) */
        {
            int hard = (e - s >= 2 && cps[e - 1] == ' ' && cps[e - 2] == ' ');
            int ts = s, te = e;
            while (ts < te && is_space(cps[ts]))
                ts++;
            while (te > ts && is_space(cps[te - 1]))
                te--;
            if (kind == K_PARA || kind == K_QUOTE || kind == K_LIST) {
                block_put(&cur, cur_hard ? '\n' : ' ', base);
            } else {
                doc_close_block(d, &cur, base);
                cur.fmt = fmt_default(st);
                kind = K_PARA;
            }
            inline_put(&cur, cps, ts, te, base, mono_face, mono_size, 0, 0);
            cur_hard = hard;
        }

next_line:
        if (j >= n)
            break;
        i = j + 1;
    }

    doc_close_block(d, &cur, base);
    free(cur.a);
    free(cps);

    if (d->natoms == 0) {
        markdown_free(d);
        return NULL;
    }
    return d;
}

void markdown_free(MarkdownDoc *doc)
{
    if (!doc)
        return;
    free(doc->atoms);
    free(doc->fmts);
    free(doc);
}

const RAtom *markdown_atoms(const MarkdownDoc *doc, int *n)
{
    if (n)
        *n = doc ? doc->natoms : 0;
    return doc ? doc->atoms : NULL;
}

const RetexParaFormat *markdown_formats(const MarkdownDoc *doc, int *n)
{
    if (n)
        *n = doc ? doc->nfmts : 0;
    return doc ? doc->fmts : NULL;
}
