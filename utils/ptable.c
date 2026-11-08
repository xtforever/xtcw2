#include "ptable.h"
#include "mls.h"
#include "utf8.h"
#include "utf32.h"

#include <stdlib.h>
#include <string.h>

/*
 * The piece table owns its buffers with malloc.  Only the explicit boundary
 * functions (ptable_to_utf8, ptable_flatten, ptable_save_utf8) touch MLS,
 * returning/filling caller-owned handles.  Keeping long-lived document state
 * out of the MLS handle space avoids aliasing with the many short-lived lists
 * the layout engine allocates.
 */

typedef struct {
    Piece *pieces;
    int n;
    int add_len;
} Snap;

struct PieceTable {
    uint32_t *orig;      /* immutable original buffer */
    int orig_len;
    int orig_cap;

    uint32_t *add;       /* append-only add buffer */
    int add_len;         /* logical length (bytes beyond are stale) */
    int add_cap;

    Piece *pieces;
    int npieces;
    int pieces_cap;

    int len;

    Snap *undo;
    int undo_n, undo_cap;
    Snap *redo;
    int redo_n, redo_cap;
};

static uint32_t *buf_of(const PieceTable *t, int buf)
{
    return buf ? t->add : t->orig;
}

static int pieces_total(const PieceTable *t)
{
    int i, total = 0;
    for (i = 0; i < t->npieces; i++)
        total += t->pieces[i].len;
    return total;
}

static int reserve_u32(uint32_t **buf, int *cap, int need)
{
    if (need <= *cap)
        return 0;
    {
        int nc = *cap ? *cap : 16;
        uint32_t *nb;
        while (nc < need)
            nc *= 2;
        nb = (uint32_t *)realloc(*buf, (size_t)nc * sizeof(uint32_t));
        if (!nb)
            return -1;
        *buf = nb;
        *cap = nc;
    }
    return 0;
}

static int reserve_pieces(PieceTable *t, int need)
{
    if (need <= t->pieces_cap)
        return 0;
    {
        int nc = t->pieces_cap ? t->pieces_cap : 8;
        Piece *nb;
        while (nc < need)
            nc *= 2;
        nb = (Piece *)realloc(t->pieces, (size_t)nc * sizeof(Piece));
        if (!nb)
            return -1;
        t->pieces = nb;
        t->pieces_cap = nc;
    }
    return 0;
}

/* ---- snapshot helpers -------------------------------------------------- */

static void snap_free(Snap *s)
{
    free(s->pieces);
    s->pieces = NULL;
    s->n = 0;
    s->add_len = 0;
}

static void snaps_drop(Snap *arr, int *n)
{
    int i;
    for (i = 0; i < *n; i++)
        snap_free(&arr[i]);
    *n = 0;
}

static void snaps_clear(Snap **arr, int *n, int *cap)
{
    snaps_drop(*arr, n);
    free(*arr);
    *arr = NULL;
    *cap = 0;
}

static int snap_make(const PieceTable *t, Snap *s)
{
    int n = t->npieces;
    s->n = n;
    s->add_len = t->add_len;
    s->pieces = NULL;
    if (n > 0) {
        s->pieces = (Piece *)malloc((size_t)n * sizeof(Piece));
        if (!s->pieces)
            return -1;
        memcpy(s->pieces, t->pieces, (size_t)n * sizeof(Piece));
    }
    return 0;
}

static int snaps_push(Snap **arr, int *n, int *cap, Snap s)
{
    if (*n >= *cap) {
        int nc = *cap ? *cap * 2 : 8;
        Snap *na = (Snap *)realloc(*arr, (size_t)nc * sizeof(Snap));
        if (!na)
            return -1;
        *arr = na;
        *cap = nc;
    }
    (*arr)[(*n)++] = s;
    return 0;
}

static Snap snap_pop(Snap *arr, int *n)
{
    Snap s = arr[--(*n)];
    arr[*n].pieces = NULL;
    arr[*n].n = 0;
    arr[*n].add_len = 0;
    return s;
}

static int snap_restore(PieceTable *t, const Snap *s)
{
    if (reserve_pieces(t, s->n) != 0)
        return -1;
    if (s->n > 0)
        memcpy(t->pieces, s->pieces, (size_t)s->n * sizeof(Piece));
    t->npieces = s->n;
    t->add_len = s->add_len;
    t->len = pieces_total(t);
    return 0;
}

/* ---- coalescing -------------------------------------------------------- */

static void coalesce(PieceTable *t)
{
    int i = 0;
    while (i < t->npieces) {
        Piece *p = &t->pieces[i];
        if (p->len <= 0) {
            memmove(&t->pieces[i], &t->pieces[i + 1],
                    (size_t)(t->npieces - i - 1) * sizeof(Piece));
            t->npieces--;
            continue;
        }
        if (i + 1 < t->npieces) {
            Piece *q = &t->pieces[i + 1];
            if (p->buf == q->buf && p->start + p->len == q->start) {
                p->len += q->len;
                memmove(&t->pieces[i + 1], &t->pieces[i + 2],
                        (size_t)(t->npieces - i - 2) * sizeof(Piece));
                t->npieces--;
                continue;
            }
        }
        i++;
    }
}

/* ---- lifecycle --------------------------------------------------------- */

PieceTable *ptable_create(void)
{
    return (PieceTable *)calloc(1, sizeof(PieceTable));
}

void ptable_free(PieceTable *t)
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

int ptable_load_utf8(PieceTable *t, const char *utf8)
{
    int n;

    if (!t || !utf8)
        return -1;
    n = utf32_count_utf8(utf8);
    if (n < 0)
        return -1;

    if (reserve_u32(&t->orig, &t->orig_cap, n) != 0)
        return -1;
    if (n > 0) {
        if (utf32_from_utf8(utf8, t->orig, n) != n)
            return -1;
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

int ptable_flatten(PieceTable *t, int u32mls)
{
    uint32_t *dst;
    int i, pos = 0;

    m_setlen(u32mls, t->len);
    dst = (uint32_t *)m_buf(u32mls);
    for (i = 0; i < t->npieces; i++) {
        Piece *p = &t->pieces[i];
        if (p->len > 0) {
            memcpy(dst + pos, buf_of(t, p->buf) + p->start,
                   (size_t)p->len * sizeof(uint32_t));
            pos += p->len;
        }
    }
    return pos;
}

int ptable_to_utf8(PieceTable *t)
{
    uint32_t *cps = NULL;
    int bytelen, out;

    if (t->len > 0) {
        int i, pos = 0;
        cps = (uint32_t *)malloc((size_t)t->len * sizeof(uint32_t));
        if (!cps)
            return 0;
        for (i = 0; i < t->npieces; i++) {
            Piece *p = &t->pieces[i];
            if (p->len > 0) {
                memcpy(cps + pos, buf_of(t, p->buf) + p->start,
                       (size_t)p->len * sizeof(uint32_t));
                pos += p->len;
            }
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

int ptable_save_utf8(PieceTable *t, int mls_out)
{
    int h = ptable_to_utf8(t);
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

int ptable_len(const PieceTable *t)
{
    return t->len;
}

int ptable_find_piece(const PieceTable *t, int pos, int *piece_idx, int *off)
{
    int i, start = 0;

    if (!t || pos < 0 || pos >= t->len)
        return 0;
    for (i = 0; i < t->npieces; i++) {
        Piece *p = &t->pieces[i];
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

uint32_t ptable_get(const PieceTable *t, int pos)
{
    int idx, off;
    Piece *p;

    if (!ptable_find_piece(t, pos, &idx, &off))
        return 0;
    p = &t->pieces[idx];
    return buf_of(t, p->buf)[p->start + off];
}

/* ---- editing ----------------------------------------------------------- */

int ptable_insert(PieceTable *t, int pos, const uint32_t *cps, int n)
{
    int add_start, idx, off;
    Piece np;

    if (!t || n <= 0)
        return t ? t->len : 0;
    if (!utf32_validate(cps, n))
        return -1;
    if (pos < 0)
        pos = 0;
    if (pos > t->len)
        pos = t->len;

    if (reserve_u32(&t->add, &t->add_cap, t->add_len + n) != 0)
        return -1;
    add_start = t->add_len;
    memcpy(t->add + add_start, cps, (size_t)n * sizeof(uint32_t));
    t->add_len += n;

    np.buf = 1;
    np.start = add_start;
    np.len = n;

    if (pos == 0 || t->npieces == 0) {
        if (reserve_pieces(t, t->npieces + 1) != 0)
            return -1;
        memmove(&t->pieces[1], &t->pieces[0],
                (size_t)t->npieces * sizeof(Piece));
        t->pieces[0] = np;
        t->npieces++;
    } else if (pos == t->len) {
        if (reserve_pieces(t, t->npieces + 1) != 0)
            return -1;
        t->pieces[t->npieces++] = np;
    } else {
        ptable_find_piece(t, pos, &idx, &off);
        if (off == 0) {
            if (reserve_pieces(t, t->npieces + 1) != 0)
                return -1;
            memmove(&t->pieces[idx + 1], &t->pieces[idx],
                    (size_t)(t->npieces - idx) * sizeof(Piece));
            t->pieces[idx] = np;
            t->npieces++;
        } else {
            Piece p = t->pieces[idx];
            Piece left = { p.buf, p.start, off };
            Piece right = { p.buf, p.start + off, p.len - off };
            if (reserve_pieces(t, t->npieces + 2) != 0)
                return -1;
            memmove(&t->pieces[idx + 3], &t->pieces[idx + 1],
                    (size_t)(t->npieces - idx - 1) * sizeof(Piece));
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

int ptable_delete(PieceTable *t, int pos, int n)
{
    int i, abs = 0, end, outn = 0;
    Piece *out;

    if (!t || n <= 0)
        return t ? t->len : 0;
    if (pos < 0)
        pos = 0;
    if (pos >= t->len)
        return t->len;
    if (pos + n > t->len)
        n = t->len - pos;
    end = pos + n;

    out = (Piece *)malloc((size_t)(t->npieces + 2) * sizeof(Piece));
    if (!out)
        return t->len;

    for (i = 0; i < t->npieces; i++) {
        Piece *p = &t->pieces[i];
        int ps = abs;
        int pe = abs + p->len;
        abs = pe;

        if (pe <= pos || ps >= end) {
            out[outn++] = *p;
        } else {
            if (ps < pos) {
                Piece left = { p->buf, p->start, pos - ps };
                out[outn++] = left;
            }
            if (pe > end) {
                Piece right = { p->buf, p->start + (end - ps), pe - end };
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

/* ---- undo / redo ------------------------------------------------------- */

void ptable_snapshot(PieceTable *t)
{
    Snap s;
    if (!t)
        return;
    if (snap_make(t, &s) != 0)
        return;
    snaps_drop(t->redo, &t->redo_n);
    if (snaps_push(&t->undo, &t->undo_n, &t->undo_cap, s) != 0)
        snap_free(&s);
}

int ptable_undo(PieceTable *t)
{
    Snap cur, s;

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

int ptable_redo(PieceTable *t)
{
    Snap cur, s;

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

int ptable_undo_depth(const PieceTable *t)
{
    return t ? t->undo_n : 0;
}

int ptable_redo_depth(const PieceTable *t)
{
    return t ? t->redo_n : 0;
}
