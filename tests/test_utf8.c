#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "utf8.h"

static void test_encode_decode_roundtrip(void)
{
    uint32_t cases[] = {
        0x0041, 0x007F, 0x00E9, 0x07FF, 0x4E2D, 0xFFFF, 0x1F600, 0x10FFFF
    };
    int n = (int)(sizeof(cases) / sizeof(cases[0]));
    int i;

    for (i = 0; i < n; i++) {
        char buf[4];
        int k = utf8_encode_cp(cases[i], buf);
        const char *p = buf;
        uint32_t cp = 0xDEADBEEF;
        int r;
        assert(k == utf8_cp_len(cases[i]));
        r = utf8_decode_cp(&p, &cp);
        assert(r == k);
        assert(cp == cases[i]);
        assert(p == buf + k);
    }
    printf("test_encode_decode_roundtrip passed\n");
}

static void test_reject_encode(void)
{
    char buf[4];
    assert(utf8_encode_cp(0xD800, buf) == 0);   /* low surrogate */
    assert(utf8_encode_cp(0xDFFF, buf) == 0);   /* high surrogate */
    assert(utf8_encode_cp(0x110000, buf) == 0); /* out of range */
    printf("test_reject_encode passed\n");
}

static void test_reject_decode(void)
{
    /* invalid lead byte */
    const char *s1 = "\x80";
    /* lone continuation */
    const char *s2 = "\xC0\x80";          /* overlong NUL */
    const char *s3 = "\xE0\x80\x80";      /* overlong 3-byte */
    const char *s4 = "\xF0\x80\x80\x80";  /* overlong 4-byte */
    const char *s5 = "\xED\xA0\x80";      /* surrogate U+D800 */
    const char *s6 = "\xF4\x90\x80\x80";  /* U+110000 */
    const char *s7 = "\xE2\x82";          /* truncated */
    const char *s8 = "\xFF";              /* invalid lead */
    const char *bad[] = { s1, s2, s3, s4, s5, s6, s7, s8 };
    int i;
    uint32_t cp;

    for (i = 0; i < (int)(sizeof(bad) / sizeof(bad[0])); i++) {
        const char *p = bad[i];
        int r = utf8_decode_cp(&p, &cp);
        assert(r == 0);
        assert(p == bad[i]); /* pointer unchanged on error */
    }
    printf("test_reject_decode passed\n");
}

static void test_string_helpers(void)
{
    const char *good = "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80"; /* A é 中 😀 */
    const char *bad = "A\xC3\x28";                                 /* bad cont. */

    assert(utf8_validate(good) == 1);
    assert(utf8_validate(bad) == 0);
    assert(utf8_strlen(good) == 4);
    assert(utf8_strlen(bad) == -1);
    assert(utf8_strlen("") == 0);
    assert(utf8_validate("") == 1);
    printf("test_string_helpers passed\n");
}

static void test_nul(void)
{
    char buf[4];
    int k = utf8_encode_cp(0, buf);
    assert(k == 1);
    assert(buf[0] == 0);
    printf("test_nul passed\n");
}

static void test_byte_cp_mapping(void)
{
    /* A (1 byte), é (2), 中 (3), 😀 (4): cp indices 0..4, byte offsets 0,1,3,6,10 */
    const char *s = "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80";

    assert(utf8_cp_index_to_byte(s, 0) == 0);
    assert(utf8_cp_index_to_byte(s, 1) == 1);
    assert(utf8_cp_index_to_byte(s, 2) == 3);
    assert(utf8_cp_index_to_byte(s, 3) == 6);
    assert(utf8_cp_index_to_byte(s, 4) == 10);
    assert(utf8_cp_index_to_byte(s, 5) == 10); /* past end */
    assert(utf8_cp_index_to_byte(s, -1) == -1);

    assert(utf8_byte_to_cp_index(s, 0) == 0);
    assert(utf8_byte_to_cp_index(s, 1) == 1);
    assert(utf8_byte_to_cp_index(s, 3) == 2);
    assert(utf8_byte_to_cp_index(s, 6) == 3);
    assert(utf8_byte_to_cp_index(s, 10) == 4);
    assert(utf8_byte_to_cp_index(s, 2) == 1); /* inside é -> that code point */
    assert(utf8_byte_to_cp_index(s, 4) == 2); /* inside 中 -> that code point */

    /* round-trip at every code point boundary */
    {
        int i;
        for (i = 0; i <= 4; i++) {
            int b = utf8_cp_index_to_byte(s, i);
            assert(utf8_byte_to_cp_index(s, b) == (i == 4 ? 4 : i));
        }
    }
    printf("test_byte_cp_mapping passed\n");
}

int main(void)
{
    test_encode_decode_roundtrip();
    test_reject_encode();
    test_reject_decode();
    test_string_helpers();
    test_nul();
    test_byte_cp_mapping();
    printf("All utf8 tests passed\n");
    return 0;
}
