#include "rtable.h"
#include "mls.h"
#include "utf8.h"
#include "utf32.h"

#include <stdlib.h>
#include <string.h>

/*
 * Same design as utils/ptable.c, but the buffers hold RAtom elements (a code
 * point + look, or an inline object) instead of bare uint32_t code points.
 * The piece algorithm itself is element-agnostic and identical.
 *
 * The piece table owns its buffers with malloc.  Only the explicit boundary
 * functions (rtable_to_utf8, rtable_flatten, rtable_save_utf8) touch MLS.
 */

typedef struct {
    int buf;    /* 0 = original buffer, 1 = add buffer */
    int start;  /* first atom index within that buffer */
    int len;    /* number of atoms */
} RPiece;

typedef struct {
    RPiece *pieces;
    int n;
    int add_len;
} RSpan;

struct RTable {
    RAtom *orig;         /* immutable original buffer */
    int orig_len;
    int orig_cap;

    RAtom *add;          /* append-only add buffer */
    int add_len;         /* logical length (beyond are stale) */
    int add_cap;

    RPiece *pieces;
    int npieces;
    int pieces_cap;

    int len;

    RSpan *undo;
    int undo_n, undo_cap;
    RSpan *redo;
    int redo_n, redo_cap;
};

static RAtom *buf_of(const RTable *t, int buf)
{
    return buf ? t->add : t->orig;
}

static int pieces_total(const RTable *t)
{
    int i, total = 0;
    for (i = 0; i < t->npieces; i++)
        total += t->pieces[i].len;
    return total;
}

static int reserve_atoms(RAtom **buf, int *cap, int need)
{
    if (need <= *cap)
        return 0;
    {
        int nc = *cap ? *cap : 16;
        RAtom *nb;
        while (nc < need)
            nc *= 2;
        nb = (RAtom *)realloc(*buf, (size_t)nc * sizeof(RAtom));
        if (!nb)
            return -1;
        *buf = nb;
        *cap = nc;
    }
    return 0;
}

static int reserve_pieces(RTable *t, int need)
{
    if (need <= t->pieces_cap)
        return 0;
    {
        int nc = t->pieces_cap ? t->pieces_cap : 8;
        RPiece *nb;
        while (nc < need)
            nc *= 2;
        nb = (RPiece *)realloc(t->pieces, (size_t)nc * sizeof(RPiece));
        if (!nb)
            return -1;
        t->pieces = nb;
        t->pieces_cap = nc;
    }
    return 0;
}

static int atom_valid(const RAtom *a)
{
    uint32_t c;
    if (!a)
        return 0;
    if (a->kind == RATOM_CHAR) {
        c = a->cp;
        if (c == 0 || c > 0x10FFFF)
            return 0;
        if (c >= 0xD800 && c <= 0xDFFF)
            return 0;
        return 1;
    }
    if (a->kind == RATOM_OBJECT)
        return 1;
    return 0;
}

/* ---- snapshot helpers -------------------------------------------------- */

static void snap_free(RSpan *s)
{
    free(s->pieces);
    s->pieces = NULL;
    s->n = 0;
    s->add_len = 0;
}

static void snaps_drop(RSpan *arr, int *n)
{
    int i;
    for (i = 0; i < *n; i++)
        snap_free(&arr[i]);
    *n = 0;
}

static void snaps_clear(RSpan **arr, int *n, int *cap)
{
    snaps_drop(*arr, n);
    free(*arr);
    *arr = NULL;
    *cap = 0;
}

static int snap_make(const RTable *t, RSpan *s)
{
    int n = t->npieces;
    s->n = n;
    s->add_len = t->add_len;
    s->pieces = NULL;
    if (n > 0) {
        s->pieces = (RPiece *)malloc((size_t)n * sizeof(RPiece));
        if (!s->pieces)
            return -1;
        memcpy(s->pieces, t->pieces, (size_t)n * sizeof(RPiece));
    }
    return 0;
}

static int snaps_push(RSpan **arr, int *n, int *cap, RSpan s)
{
    if (*n >= *cap) {
        int nc = *cap ? *cap * 2 : 8;
        RSpan *na = (RSpan *)realloc(*arr, (size_t)nc * sizeof(RSpan));
        if (!na)
            return -1;
        *arr = na;
        *cap = nc;
    }
    (*arr)[(*n)++] = s;
    return 0;
}

static RSpan snap_pop(RSpan *arr, int *n)
{
    RSpan s = arr[--(*n)];
    arr[*n].pieces = NULL;
    arr[*n].n = 0;
    arr[*n].add_len = 0;
    return s;
}

static int snap_restore(RTable *t, const RSpan *s)
{
    if (reserve_pieces(t, s->n) != 0)
        return -1;
    if (s->n > 0)
        memcpy(t->pieces, s->pieces, (size_t)s->n * sizeof(RPiece));
    t->npieces = s->n;
    t->add_len = s->add_len;
    t->len = pieces_total(t);
    return 0;
}

/* ---- coalescing -------------------------------------------------------- */

static void coalesce(RTable *t)
{
    int i = 0;
    while (i < t->npieces) {
        RPiece *p = &t->pieces[i];
        if (p->len <= 0) {
            memmove(&t->pieces[i], &t->pieces[i + 1],
                    (size_t)(t->npieces - i - 1) * sizeof(RPiece));
            t->npieces--;
            continue;
        }
        if (i + 1 < t->npieces) {
            RPiece *q = &t->pieces[i + 1];
            if (p->buf == q->buf && p->start + p->len == q->start) {
                p->len += q->len;
                memmove(&t->pieces[i + 1], &t->pieces[i + 2],
                        (size_t)(t->npieces - i - 2) * sizeof(RPiece));
                t->npieces--;
                continue;
            }
        }
        i++;
    }
}

/* ---- lifecycle --------------------------------------------------------- */

RTable *rtable_create(void)
{
    return (RTable *)calloc(1, sizeof(RTable));
}

void rtable_free(RTable *t)
{
    if (!t)
        return;
    snaps_clear(&t->undo, &t->undo_n, &t->undo_cap);
    snaps_clear(&t->redo, &t->redo_n, &t->redo_cap);
    free(t->orig);
    free(t->add);
    free(t->pieces);
    free(t);
}

/* ---- loading / saving -------------------------------------------------- */

int rtable_load_plain(RTable *t, const char *utf8, RLook look)
{
    int n, i;
    uint32_t *cps;

    if (!t || !utf8)
        return -1;
    n = utf32_count_utf8(utf8);
    if (n < 0)
        return -1;

    if (reserve_atoms(&t->orig, &t->orig_cap, n) != 0)
        return -1;
    if (n > 0) {
        cps = (uint32_t *)malloc((size_t)n * sizeof(uint32_t));
        if (!cps)
            return -1;
        if (utf32_from_utf8(utf8, cps, n) != n) {
            free(cps);
            return -1;
        }
        for (i = 0; i < n; i++) {
            t->orig[i].kind = RATOM_CHAR;
            t->orig[i].cp = cps[i];
            t->orig[i].look = look;
            t->orig[i].payload = 0;
        }
        free(cps);
    }
    t->orig_len = n;

    t->add_len = 0;

    t->npieces = 0;
    if (n > 0) {
        if (reserve_pieces(t, 1) != 0)
            return -1;
        t->pieces[0].buf = 0;
        t->pieces[0].start = 0;
        t->pieces[0].len = n;
        t->npieces = 1;
    }
    t->len = n;

    snaps_clear(&t->undo, &t->undo_n, &t->undo_cap);
    snaps_clear(&t->redo, &t->redo_n, &t->redo_cap);
    return n;
}

int rtable_flatten(RTable *t, int u32mls)
{
    uint32_t *dst;
    int i, j, pos = 0;

    m_setlen(u32mls, t->len);
    dst = (uint32_t *)m_buf(u32mls);
    for (i = 0; i < t->npieces; i++) {
        RPiece *p = &t->pieces[i];
        RAtom *src = buf_of(t, p->buf) + p->start;
        for (j = 0; j < p->len; j++)
            dst[pos++] = (src[j].kind == RATOM_CHAR) ? src[j].cp : 0xFFFC;
    }
    return pos;
}

int rtable_to_utf8(RTable *t)
{
    uint32_t *cps = NULL;
    int bytelen, out;

    if (t->len > 0) {
        int i, j, pos = 0;
        cps = (uint32_t *)malloc((size_t)t->len * sizeof(uint32_t));
        if (!cps)
            return 0;
        for (i = 0; i < t->npieces; i++) {
            RPiece *p = &t->pieces[i];
            RAtom *src = buf_of(t, p->buf) + p->start;
            for (j = 0; j < p->len; j++)
                cps[pos++] = (src[j].kind == RATOM_CHAR) ? src[j].cp : 0xFFFC;
        }
    }

    bytelen = utf32_to_utf8(cps, t->len, NULL, 0);
    if (bytelen < 0) {
        free(cps);
        return 0;
    }

    out = m_create(bytelen + 1, 1);
    m_setlen(out, bytelen + 1);
    utf32_to_utf8(cps, t->len, (char *)m_buf(out), bytelen + 1);
    free(cps);
    return out;
}

int rtable_save_utf8(RTable *t, int mls_out)
{
    int h = rtable_to_utf8(t);
    int bytelen, start;
    char zero = 0;

    if (!h)
        return -1;
    bytelen = m_len(h) - 1;
    start = m_len(mls_out);
    if (start > 0 && ((char *)m_buf(mls_out))[start - 1] == '\0')
        start--;
    if (bytelen > 0)
        m_write(mls_out, start, m_buf(h), bytelen);
    m_write(mls_out, start + bytelen, &zero, 1);
    m_free(h);
    return bytelen;
}

/* ---- queries ----------------------------------------------------------- */

int rtable_len(const RTable *t)
{
    return t->len;
}

static int find_piece(const RTable *t, int pos, int *piece_idx, int *off)
{
    int i, start = 0;

    if (!t || pos < 0 || pos >= t->len)
        return 0;
    for (i = 0; i < t->npieces; i++) {
        RPiece *p = &t->pieces[i];
        if (pos < start + p->len) {
            if (piece_idx)
                *piece_idx = i;
            if (off)
                *off = pos - start;
            return 1;
        }
        start += p->len;
    }
    return 0;
}

int rtable_get(const RTable *t, int pos, RAtom *out)
{
    int idx, off;
    RPiece *p;

    if (!find_piece(t, pos, &idx, &off))
        return 0;
    p = &t->pieces[idx];
    if (out)
        *out = buf_of(t, p->buf)[p->start + off];
    return 1;
}

RAtom *rtable_to_array(RTable *t, int *n)
{
    RAtom *arr;
    int i, j, pos = 0;

    if (n)
        *n = 0;
    if (!t || t->len <= 0)
        return NULL;
    arr = (RAtom *)malloc((size_t)t->len * sizeof(RAtom));
    if (!arr)
        return NULL;
    for (i = 0; i < t->npieces; i++) {
        RPiece *p = &t->pieces[i];
        RAtom *src = buf_of(t, p->buf) + p->start;
        for (j = 0; j < p->len; j++)
            arr[pos++] = src[j];
    }
    if (n)
        *n = pos;
    return arr;
}

/* ---- editing ----------------------------------------------------------- */

int rtable_insert(RTable *t, int pos, const RAtom *atoms, int n)
{
    int add_start, idx, off, i;
    RPiece np;

    if (!t || n <= 0)
        return t ? t->len : 0;
    for (i = 0; i < n; i++)
        if (!atom_valid(&atoms[i]))
            return -1;
    if (pos < 0)
        pos = 0;
    if (pos > t->len)
        pos = t->len;

    if (reserve_atoms(&t->add, &t->add_cap, t->add_len + n) != 0)
        return -1;
    add_start = t->add_len;
    memcpy(t->add + add_start, atoms, (size_t)n * sizeof(RAtom));
    t->add_len += n;

    np.buf = 1;
    np.start = add_start;
    np.len = n;

    if (pos == 0 || t->npieces == 0) {
        if (reserve_pieces(t, t->npieces + 1) != 0)
            return -1;
        memmove(&t->pieces[1], &t->pieces[0],
                (size_t)t->npieces * sizeof(RPiece));
        t->pieces[0] = np;
        t->npieces++;
    } else if (pos == t->len) {
        if (reserve_pieces(t, t->npieces + 1) != 0)
            return -1;
        t->pieces[t->npieces++] = np;
    } else {
        find_piece(t, pos, &idx, &off);
        if (off == 0) {
            if (reserve_pieces(t, t->npieces + 1) != 0)
                return -1;
            memmove(&t->pieces[idx + 1], &t->pieces[idx],
                    (size_t)(t->npieces - idx) * sizeof(RPiece));
            t->pieces[idx] = np;
            t->npieces++;
        } else {
            RPiece p = t->pieces[idx];
            RPiece left = { p.buf, p.start, off };
            RPiece right = { p.buf, p.start + off, p.len - off };
            if (reserve_pieces(t, t->npieces + 2) != 0)
                return -1;
            memmove(&t->pieces[idx + 3], &t->pieces[idx + 1],
                    (size_t)(t->npieces - idx - 1) * sizeof(RPiece));
            t->pieces[idx] = left;
            t->pieces[idx + 1] = np;
            t->pieces[idx + 2] = right;
            t->npieces += 2;
        }
    }

    t->len += n;
    coalesce(t);
    return t->len;
}

int rtable_insert_cps(RTable *t, int pos, const uint32_t *cps, int n, RLook look)
{
    RAtom *atoms;
    int i, r;

    if (!t || n <= 0)
        return t ? t->len : 0;
    atoms = (RAtom *)malloc((size_t)n * sizeof(RAtom));
    if (!atoms)
        return -1;
    for (i = 0; i < n; i++) {
        atoms[i].kind = RATOM_CHAR;
        atoms[i].cp = cps[i];
        atoms[i].look = look;
        atoms[i].payload = 0;
    }
    r = rtable_insert(t, pos, atoms, n);
    free(atoms);
    return r;
}

int rtable_delete(RTable *t, int pos, int n)
{
    int i, abs = 0, end, outn = 0;
    RPiece *out;

    if (!t || n <= 0)
        return t ? t->len : 0;
    if (pos < 0)
        pos = 0;
    if (pos >= t->len)
        return t->len;
    if (pos + n > t->len)
        n = t->len - pos;
    end = pos + n;

    out = (RPiece *)malloc((size_t)(t->npieces + 2) * sizeof(RPiece));
    if (!out)
        return t->len;

    for (i = 0; i < t->npieces; i++) {
        RPiece *p = &t->pieces[i];
        int ps = abs;
        int pe = abs + p->len;
        abs = pe;

        if (pe <= pos || ps >= end) {
            out[outn++] = *p;
        } else {
            if (ps < pos) {
                RPiece left = { p->buf, p->start, pos - ps };
                out[outn++] = left;
            }
            if (pe > end) {
                RPiece right = { p->buf, p->start + (end - ps), pe - end };
                out[outn++] = right;
            }
        }
    }

    free(t->pieces);
    t->pieces = out;
    t->pieces_cap = t->npieces + 2;
    t->npieces = outn;
    t->len -= n;
    coalesce(t);
    return t->len;
}

/* ---- look / formatting edits ------------------------------------------ */

typedef void (*RLookFn)(RAtom *a, void *ctx);

static int rtable_modify_looks(RTable *t, int pos, int n,
                               RLookFn fn, void *ctx)
{
    RAtom *tmp;
    int i, r;

    if (!t || n <= 0)
        return t ? t->len : 0;
    if (pos < 0)
        pos = 0;
    if (pos >= t->len)
        return t->len;
    if (pos + n > t->len)
        n = t->len - pos;
    if (n <= 0)
        return t->len;

    tmp = (RAtom *)malloc((size_t)n * sizeof(RAtom));
    if (!tmp)
        return t->len;
    for (i = 0; i < n; i++) {
        if (!rtable_get(t, pos + i, &tmp[i])) {
            free(tmp);
            return t->len;
        }
    }
    for (i = 0; i < n; i++)
        if (tmp[i].kind == RATOM_CHAR)
            fn(&tmp[i], ctx);

    rtable_delete(t, pos, n);
    r = rtable_insert(t, pos, tmp, n);
    free(tmp);
    return r;
}

struct style_ctx { int set_bits, clear_bits; };
static void style_fn(RAtom *a, void *ctx)
{
    struct style_ctx *c = (struct style_ctx *)ctx;
    a->look.style = (a->look.style & ~c->clear_bits) | c->set_bits;
}
static void face_fn(RAtom *a, void *ctx) { a->look.face = *(int *)ctx; }
static void size_fn(RAtom *a, void *ctx) { a->look.size = *(float *)ctx; }

int rtable_apply_style(RTable *t, int pos, int n, int set_bits, int clear_bits)
{
    struct style_ctx c;
    c.set_bits = set_bits;
    c.clear_bits = clear_bits;
    return rtable_modify_looks(t, pos, n, style_fn, &c);
}

int rtable_set_face(RTable *t, int pos, int n, int face)
{
    return rtable_modify_looks(t, pos, n, face_fn, &face);
}

int rtable_set_size(RTable *t, int pos, int n, float size)
{
    return rtable_modify_looks(t, pos, n, size_fn, &size);
}

/* ---- undo / redo ------------------------------------------------------- */

void rtable_snapshot(RTable *t)
{
    RSpan s;
    if (!t)
        return;
    if (snap_make(t, &s) != 0)
        return;
    snaps_drop(t->redo, &t->redo_n);
    if (snaps_push(&t->undo, &t->undo_n, &t->undo_cap, s) != 0)
        snap_free(&s);
}

int rtable_undo(RTable *t)
{
    RSpan cur, s;

    if (!t || t->undo_n == 0)
        return 0;
    if (snap_make(t, &cur) != 0)
        return 0;
    if (snaps_push(&t->redo, &t->redo_n, &t->redo_cap, cur) != 0) {
        snap_free(&cur);
        return 0;
    }
    s = snap_pop(t->undo, &t->undo_n);
    if (snap_restore(t, &s) != 0) {
        snap_free(&s);
        return 0;
    }
    snap_free(&s);
    return 1;
}

int rtable_redo(RTable *t)
{
    RSpan cur, s;

    if (!t || t->redo_n == 0)
        return 0;
    if (snap_make(t, &cur) != 0)
        return 0;
    if (snaps_push(&t->undo, &t->undo_n, &t->undo_cap, cur) != 0) {
        snap_free(&cur);
        return 0;
    }
    s = snap_pop(t->redo, &t->redo_n);
    if (snap_restore(t, &s) != 0) {
        snap_free(&s);
        return 0;
    }
    snap_free(&s);
    return 1;
}

int rtable_undo_depth(const RTable *t)
{
    return t ? t->undo_n : 0;
}

int rtable_redo_depth(const RTable *t)
{
    return t ? t->redo_n : 0;
}
