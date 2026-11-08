#ifndef XTCW_RTABLE_H
#define XTCW_RTABLE_H

#include <stdint.h>

/*
 * rtable - a rich piece-table document model over "atoms".
 *
 * Extends the Xerox PARC Bravo piece table (see utils/ptable.h) with per-atom
 * formatting ("looks") and inline objects, the way Bravo's pieces carried a
 * look.  The document is an ordered list of pieces, each a {buffer, start, len}
 * span into either the immutable original buffer or an append-only add buffer;
 * editing never moves atoms, inserts append to the add buffer and split a
 * piece, deletes shrink/split a piece.  Snapshots of the piece list give cheap
 * multi-level undo that covers text, formatting and objects in one step.
 *
 * An Atom is either:
 *   - RATOM_CHAR : one Unicode code point plus its RLook (font face / style /
 *                  size), or
 *   - RATOM_OBJECT: an inline object (SVG image, math span, ...) that occupies
 *                  one document position.  `cp` holds the object type and
 *                  `payload` an interned-string handle (e.g. filename).
 *
 * Positions and piece start/len are measured in atoms (code points for text),
 * not bytes.  Use libutf8/libutf32 at the boundaries.  Callers must call
 * m_init() before using these functions; face/payload handles come from
 * conststr (call conststr_init() first).
 */

typedef struct {
    int face;    /* conststr handle for the font face (0 = default face) */
    int style;   /* bitmask: 1=bold, 2=italic, 4=smallcaps (Node.font_style) */
    float size;  /* font size in points (0 = default size) */
} RLook;

enum {
    RATOM_CHAR = 0,   /* one code point + look */
    RATOM_OBJECT = 1  /* inline object */
};

enum {
    ROBJ_SVG = 1,     /* payload = conststr handle of the .svg filename */
    ROBJ_MATH = 2     /* payload = conststr handle of the math source       */
};

typedef struct {
    int kind;        /* RATOM_CHAR or RATOM_OBJECT */
    uint32_t cp;     /* RATOM_CHAR: code point; RATOM_OBJECT: object type */
    RLook look;      /* RATOM_CHAR only */
    int payload;     /* RATOM_OBJECT only: conststr handle */
} RAtom;

typedef struct RTable RTable;

RTable *rtable_create(void);
void rtable_free(RTable *t);

/* Replace the document with the given NUL-terminated UTF-8 text; every
 * character becomes a RATOM_CHAR with `look`.  Clears undo/redo history.
 * Returns the number of atoms, or -1 if the UTF-8 is invalid. */
int rtable_load_plain(RTable *t, const char *utf8, RLook look);

/* Insert n atoms at pos (clamped to [0,len]); n <= 0 is a no-op.
 * Returns the new document length, or -1 on invalid atom / OOM. */
int rtable_insert(RTable *t, int pos, const RAtom *atoms, int n);

/* Convenience: insert n code points (all sharing `look`) as char atoms. */
int rtable_insert_cps(RTable *t, int pos, const uint32_t *cps, int n, RLook look);

/* Delete n atoms starting at pos.  Range is clamped.  Returns the new length. */
int rtable_delete(RTable *t, int pos, int n);

/* Number of atoms in the document. */
int rtable_len(const RTable *t);

/* Copy the atom at pos into *out.  Returns 1 on success, 0 if out of range. */
int rtable_get(const RTable *t, int pos, RAtom *out);

/* Allocate a flat copy of all atoms (malloc; caller frees with free()).
 * Sets *n to the count and returns NULL when the document is empty or on OOM. */
RAtom *rtable_to_array(RTable *t, int *n);

/* Formatting edits over the atom range [pos, pos+n).  Only RATOM_CHAR atoms
 * are affected; their existing look is preserved except for the field being
 * changed.  Implemented as delete+reinsert (the caller must take a snapshot
 * first for undo).  Return the new document length (unchanged). */
int rtable_apply_style(RTable *t, int pos, int n, int set_bits, int clear_bits);
int rtable_set_face(RTable *t, int pos, int n, int face);
int rtable_set_size(RTable *t, int pos, int n, float size);

/* Flatten the document code points (objects become U+FFFC) into u32mls, an MLS
 * list of uint32_t.  Returns the number of code points written. */
int rtable_flatten(RTable *t, int u32mls);

/* Return a new MLS byte handle holding a NUL-terminated UTF-8 copy of the
 * document (objects become U+FFFC).  Returns 0 on failure. */
int rtable_to_utf8(RTable *t);

/* Append the document as UTF-8 to the MLS byte handle mls_out (see ptable). */
int rtable_save_utf8(RTable *t, int mls_out);

/* Push a snapshot of the current state for a subsequent undo.  Clears redo.
 * Call once before each user-visible edit. */
void rtable_snapshot(RTable *t);

/* Undo / redo one step.  Return 1 if the document changed, 0 otherwise. */
int rtable_undo(RTable *t);
int rtable_redo(RTable *t);

int rtable_undo_depth(const RTable *t);
int rtable_redo_depth(const RTable *t);

#endif /* XTCW_RTABLE_H */
