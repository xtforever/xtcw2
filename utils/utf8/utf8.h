#ifndef XTCW_UTF8_H
#define XTCW_UTF8_H

#include <stdint.h>

/*
 * libutf8 - stateless UTF-8 <-> Unicode code point primitives.
 *
 * This library deliberately contains nothing about UTF-32 storage.  Whole
 * string conversions to/from UTF-32 arrays live in libutf32 (util/utf32).
 *
 * Conventions:
 *   - A "code point" is a uint32_t in the range 0..0x10FFFF excluding the
 *     surrogate range 0xD800..0xDFFF.
 *   - Text passed to these functions is NUL terminated UTF-8.  Embedded
 *     U+0000 cannot be represented in a NUL-terminated string; encoding a
 *     code point 0 is still supported (it yields a single 0x00 byte).
 */

/* Encode one code point to UTF-8 into out (room for 4 bytes).
 * Returns the number of bytes written (1..4), or 0 if cp is not a valid
 * Unicode scalar value (surrogate or > 0x10FFFF). */
int utf8_encode_cp(uint32_t cp, char out[4]);

/* Decode one UTF-8 code point.
 * On success: stores the code point in *cp, advances *sp past the sequence,
 *             and returns the number of bytes consumed (1..4).
 * On error:   leaves *sp unchanged and returns 0.  Rejects invalid lead
 *             bytes, truncated sequences, overlong forms, surrogates and
 *             code points > 0x10FFFF.  A NUL byte is treated as end of
 *             input (returns 0). */
int utf8_decode_cp(const char **sp, uint32_t *cp);

/* Number of bytes the code point occupies in UTF-8, or 0 if invalid. */
int utf8_cp_len(uint32_t cp);

/* Return 1 if the NUL-terminated string is valid UTF-8, 0 otherwise. */
int utf8_validate(const char *s);

/* Number of code points in the NUL-terminated string, or -1 if invalid. */
int utf8_strlen(const char *s);

/* Convert a byte offset in a NUL-terminated UTF-8 string to the index of the
 * code point that contains it.  Returns the code point index (0-based); an
 * offset at/after the end maps to the total code point count. */
int utf8_byte_to_cp_index(const char *s, int byte_offset);

/* Convert a code point index to its byte offset.  Returns the byte offset of
 * the requested boundary, or the string length if the index is past the end.
 * Returns -1 if cp_index < 0 or s is NULL. */
int utf8_cp_index_to_byte(const char *s, int cp_index);

#endif /* XTCW_UTF8_H */
