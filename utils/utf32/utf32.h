#ifndef XTCW_UTF32_H
#define XTCW_UTF32_H

#include <stdint.h>

/*
 * libutf32 - UTF-32 (array of Unicode code points) conversions and helpers.
 *
 * The single code point UTF-8 codec lives in libutf8; this library operates
 * on arrays of uint32_t code points and uses libutf8 at the boundaries.
 */

/* Count the code points in a NUL-terminated UTF-8 string.
 * Returns the count, or -1 if the string is invalid UTF-8. */
int utf32_count_utf8(const char *utf8);

/* Decode a NUL-terminated UTF-8 string into out[0..out_cap).
 * out may be NULL to only validate/count.
 * Returns the number of code points decoded, or -1 if the input is invalid
 * or (when out != NULL) does not fit in out_cap. */
int utf32_from_utf8(const char *utf8, uint32_t *out, int out_cap);

/* Encode n code points to UTF-8.
 * out may be NULL to only measure.  When out != NULL a NUL terminator is
 * written and out_cap must include room for it.
 * Returns the number of bytes written excluding the terminator, or -1 on an
 * invalid code point or insufficient out_cap. */
int utf32_to_utf8(const uint32_t *cps, int n, char *out, int out_cap);

/* malloc() a NUL-terminated UTF-8 string from n code points.
 * Returns NULL on invalid input or allocation failure; caller frees. */
char *utf32_to_utf8_alloc(const uint32_t *cps, int n);

/* Return 1 if every code point is a valid Unicode scalar value. */
int utf32_validate(const uint32_t *cps, int n);

/* Compare two code point arrays for equality (length + contents). */
int utf32_equal(const uint32_t *a, int an, const uint32_t *b, int bn);

/* Find needle[0..nn) in hay[0..hn) starting at code point index from.
 * Returns the match index, or -1 if not found. */
int utf32_find(const uint32_t *hay, int hn, const uint32_t *needle, int nn,
               int from);

#endif /* XTCW_UTF32_H */
