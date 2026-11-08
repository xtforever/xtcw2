#include "utf32.h"
#include "utf8.h"

#include <stdlib.h>
#include <string.h>

int utf32_count_utf8(const char *utf8)
{
    const char *p = utf8;
    uint32_t cp;
    int n = 0;

    if (!utf8) return -1;
    while (*p) {
        if (utf8_decode_cp(&p, &cp) == 0) return -1;
        n++;
    }
    return n;
}

int utf32_from_utf8(const char *utf8, uint32_t *out, int out_cap)
{
    const char *p = utf8;
    uint32_t cp;
    int n = 0;

    if (!utf8) return -1;
    while (*p) {
        if (utf8_decode_cp(&p, &cp) == 0) return -1;
        if (out) {
            if (n >= out_cap) return -1;
            out[n] = cp;
        }
        n++;
    }
    return n;
}

int utf32_to_utf8(const uint32_t *cps, int n, char *out, int out_cap)
{
    int i, pos = 0;

    if (n < 0) return -1;
    for (i = 0; i < n; i++) {
        char tmp[4];
        int k = utf8_encode_cp(cps[i], tmp);
        if (k == 0) return -1;
        if (out) {
            if (pos + k + 1 > out_cap) return -1;
            memcpy(out + pos, tmp, (size_t)k);
        }
        pos += k;
    }
    if (out) {
        if (out_cap < 1) return -1;
        out[pos] = '\0';
    }
    return pos;
}

char *utf32_to_utf8_alloc(const uint32_t *cps, int n)
{
    int len = utf32_to_utf8(cps, n, NULL, 0);
    char *buf;

    if (len < 0) return NULL;
    buf = (char *)malloc((size_t)len + 1);
    if (!buf) return NULL;
    if (utf32_to_utf8(cps, n, buf, len + 1) < 0) {
        free(buf);
        return NULL;
    }
    return buf;
}

int utf32_validate(const uint32_t *cps, int n)
{
    int i;
    if (n < 0 || (!cps && n > 0)) return 0;
    for (i = 0; i < n; i++) {
        if (utf8_cp_len(cps[i]) == 0) return 0;
    }
    return 1;
}

int utf32_equal(const uint32_t *a, int an, const uint32_t *b, int bn)
{
    if (an != bn) return 0;
    if (an == 0) return 1;
    if (!a || !b) return 0;
    return memcmp(a, b, (size_t)an * sizeof(uint32_t)) == 0;
}

int utf32_find(const uint32_t *hay, int hn, const uint32_t *needle, int nn,
               int from)
{
    int i;

    if (nn == 0) return from >= 0 && from <= hn ? from : -1;
    if (from < 0) from = 0;
    if (!hay || !needle) return -1;
    for (i = from; i + nn <= hn; i++) {
        if (memcmp(hay + i, needle, (size_t)nn * sizeof(uint32_t)) == 0)
            return i;
    }
    return -1;
}
