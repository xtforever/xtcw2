#include "utf8.h"

/* Is cp a valid Unicode scalar value? */
static int utf8_valid_cp(uint32_t cp)
{
    if (cp > 0x10FFFFu) return 0;
    if (cp >= 0xD800u && cp <= 0xDFFFu) return 0;
    return 1;
}

int utf8_cp_len(uint32_t cp)
{
    if (!utf8_valid_cp(cp)) return 0;
    if (cp < 0x80u)    return 1;
    if (cp < 0x800u)   return 2;
    if (cp < 0x10000u) return 3;
    return 4;
}

int utf8_encode_cp(uint32_t cp, char out[4])
{
    if (!utf8_valid_cp(cp)) return 0;

    if (cp < 0x80u) {
        out[0] = (char)cp;
        return 1;
    }
    if (cp < 0x800u) {
        out[0] = (char)(0xC0u | (cp >> 6));
        out[1] = (char)(0x80u | (cp & 0x3Fu));
        return 2;
    }
    if (cp < 0x10000u) {
        out[0] = (char)(0xE0u | (cp >> 12));
        out[1] = (char)(0x80u | ((cp >> 6) & 0x3Fu));
        out[2] = (char)(0x80u | (cp & 0x3Fu));
        return 3;
    }
    out[0] = (char)(0xF0u | (cp >> 18));
    out[1] = (char)(0x80u | ((cp >> 12) & 0x3Fu));
    out[2] = (char)(0x80u | ((cp >> 6) & 0x3Fu));
    out[3] = (char)(0x80u | (cp & 0x3Fu));
    return 4;
}

int utf8_decode_cp(const char **sp, uint32_t *cp)
{
    const unsigned char *s;
    unsigned char c;
    uint32_t val;
    int n, i;

    if (!sp || !*sp || !cp) return 0;
    s = (const unsigned char *)*sp;
    c = s[0];
    if (c == 0x00u) return 0; /* NUL terminates the string */

    if (c < 0x80u) {
        *cp = c;
        *sp = (const char *)(s + 1);
        return 1;
    } else if ((c & 0xE0u) == 0xC0u) {
        n = 2; val = c & 0x1Fu;
    } else if ((c & 0xF0u) == 0xE0u) {
        n = 3; val = c & 0x0Fu;
    } else if ((c & 0xF8u) == 0xF0u) {
        n = 4; val = c & 0x07u;
    } else {
        return 0; /* invalid lead byte */
    }

    for (i = 1; i < n; i++) {
        if (s[i] == 0x00u) return 0;            /* truncated */
        if ((s[i] & 0xC0u) != 0x80u) return 0;  /* not a continuation */
        val = (val << 6) | (s[i] & 0x3Fu);
    }

    /* Reject overlong encodings. */
    if (n == 2 && val < 0x80u)    return 0;
    if (n == 3 && val < 0x800u)   return 0;
    if (n == 4 && val < 0x10000u) return 0;

    if (!utf8_valid_cp(val)) return 0;

    *cp = val;
    *sp = (const char *)(s + n);
    return n;
}

int utf8_validate(const char *s)
{
    const char *p = s;
    uint32_t cp;

    if (!s) return 0;
    while (*p) {
        if (utf8_decode_cp(&p, &cp) == 0) return 0;
    }
    return 1;
}

int utf8_strlen(const char *s)
{
    const char *p = s;
    uint32_t cp;
    int n = 0;

    if (!s) return -1;
    while (*p) {
        if (utf8_decode_cp(&p, &cp) == 0) return -1;
        n++;
    }
    return n;
}

int utf8_byte_to_cp_index(const char *s, int byte_offset)
{
    const char *p = s;
    int n = 0;

    if (!s || byte_offset < 0) return -1;
    while (*p) {
        const char *start = p;
        const char *q = p;
        uint32_t cp;
        int r = utf8_decode_cp(&q, &cp);
        int next = (r == 0) ? (int)(start - s) + 1 : (int)(q - s);
        if (byte_offset < next) return n;
        p = (r == 0) ? start + 1 : q;
        n++;
    }
    return n;
}

int utf8_cp_index_to_byte(const char *s, int cp_index)
{
    const char *p = s;
    int n = 0;

    if (!s || cp_index < 0) return -1;
    while (*p && n < cp_index) {
        uint32_t cp;
        const char *q = p;
        if (utf8_decode_cp(&q, &cp) == 0) {
            p++;
        } else {
            p = q;
        }
        n++;
    }
    return (int)(p - s);
}
