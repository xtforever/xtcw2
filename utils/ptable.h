#ifndef XTCW_PTABLE_H
#define XTCW_PTABLE_H

#include <stdint.h>

/*
 * ptable - a piece table document model over UTF-32 code points.
 *
 * Based on the data structure used by the Xerox PARC Bravo editor (invented
 * by J. Strother Moore): the document is an ordered list of pieces, each a
 * {buffer, start, length} span into either the immutable original buffer or
 * an append-only add buffer.  Editing never moves text; inserts append to the
 * add buffer and split a piece, deletes shrink/split a piece.  Snapshots of
 * the piece list give cheap multi-level undo.
 *
 * Representation:
 *   - Buffers and pieces are stored with the MLS handle library (mls.h).
 *   - Positions and piece start/len are measured in code points (elements of
 *     a uint32_t array), not bytes.  Use libutf8/libutf32 at the boundaries.
 *   - The document is NUL-terminated UTF-8 externally; an embedded U+0000 is
 *     not representable in a C string and is out of scope.
 *
 * Callers must call m_init() before using these functions.
 */

typedef struct {
    int buf;    /* 0 = original buffer, 1 = add buffer */
    int start;  /* first code point index within that buffer */
    int len;    /* number of code points */
} Piece;

typedef struct PieceTable PieceTable;

PieceTable *ptable_create(void);
void ptable_free(PieceTable *t);

/* Replace the document with the given NUL-terminated UTF-8 text.
 * Clears all undo/redo history and frees the previous contents.
 * Returns the number of code points, or -1 if the UTF-8 is invalid. */
int ptable_load_utf8(PieceTable *t, const char *utf8);

/* Append the document as UTF-8 to the MLS byte handle mls_out and keep it
 * NUL-terminated.  If mls_out already ends in a NUL, that terminator is
 * overwritten so repeated calls concatenate.  Returns the number of bytes
 * appended excluding the terminator, or -1. */
int ptable_save_utf8(PieceTable *t, int mls_out);

/* Return a new MLS byte handle holding a NUL-terminated UTF-8 copy of the
 * document.  Returns 0 on failure.  The caller owns the handle. */
int ptable_to_utf8(PieceTable *t);

/* Number of code points in the document. */
int ptable_len(const PieceTable *t);

/* Code point at pos, or 0 if pos is out of range. */
uint32_t ptable_get(const PieceTable *t, int pos);

/* Flatten the document into u32mls (an MLS list of uint32_t).  The list is
 * resized as needed.  Returns the number of code points written. */
int ptable_flatten(PieceTable *t, int u32mls);

/* Locate pos in the piece list.
 * On success stores the piece index and the offset within that piece and
 * returns 1.  Returns 0 if pos is out of range. */
int ptable_find_piece(const PieceTable *t, int pos, int *piece_idx, int *off);

/* Insert n code points at pos.  pos is clamped to [0,len]; n <= 0 is a no-op.
 * Returns the new document length, or -1 on invalid code point / OOM. */
int ptable_insert(PieceTable *t, int pos, const uint32_t *cps, int n);

/* Delete n code points starting at pos.  Range is clamped to the document.
 * Returns the new document length. */
int ptable_delete(PieceTable *t, int pos, int n);

/* Push a snapshot of the current state for a subsequent undo.  Clears the
 * redo stack.  Call once before each user-visible edit. */
void ptable_snapshot(PieceTable *t);

/* Undo / redo one step.  Return 1 if the document changed, 0 otherwise. */
int ptable_undo(PieceTable *t);
int ptable_redo(PieceTable *t);

int ptable_undo_depth(const PieceTable *t);
int ptable_redo_depth(const PieceTable *t);

#endif /* XTCW_PTABLE_H */
