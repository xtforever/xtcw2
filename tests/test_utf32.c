#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "utf32.h"

static void test_count(void)
{
    const char *s = "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80"; /* A é 中 😀 */
    assert(utf32_count_utf8("") == 0);
    assert(utf32_count_utf8("abc") == 3);
    assert(utf32_count_utf8(s) == 4);
    assert(utf32_count_utf8("A\xC3\x28") == -1); /* invalid */
    printf("test_count passed\n");
}

static void test_from_utf8(void)
{
    const char *s = "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80";
    uint32_t cps[8];
    int n = utf32_from_utf8(s, cps, 8);

    assert(n == 4);
    assert(cps[0] == 0x0041);
    assert(cps[1] == 0x00E9);
    assert(cps[2] == 0x4E2D);
    assert(cps[3] == 0x1F600);

    /* too small output buffer */
    assert(utf32_from_utf8(s, cps, 3) == -1);
    /* count-only mode */
    assert(utf32_from_utf8(s, NULL, 0) == 4);
    printf("test_from_utf8 passed\n");
}

static void test_to_utf8(void)
{
    uint32_t cps[] = { 0x0041, 0x00E9, 0x4E2D, 0x1F600 };
    const char *expect = "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80";
    char buf[32];
    int n;

    /* measure only */
    n = utf32_to_utf8(cps, 4, NULL, 0);
    assert(n == (int)strlen(expect));

    /* encode */
    n = utf32_to_utf8(cps, 4, buf, sizeof(buf));
    assert(n == (int)strlen(expect));
    assert(strcmp(buf, expect) == 0);

    /* alloc */
    {
        char *a = utf32_to_utf8_alloc(cps, 4);
        assert(a != NULL);
        assert(strcmp(a, expect) == 0);
        free(a);
    }

    /* invalid code point */
    {
        uint32_t bad[] = { 0xD800 };
        assert(utf32_to_utf8(bad, 1, buf, sizeof(buf)) == -1);
        assert(utf32_to_utf8_alloc(bad, 1) == NULL);
    }

    /* insufficient buffer */
    {
        char tiny[4];
        assert(utf32_to_utf8(cps, 4, tiny, sizeof(tiny)) == -1);
    }
    printf("test_to_utf8 passed\n");
}

static void test_validate_equal_find(void)
{
    uint32_t a[] = { 0x41, 0x42, 0x43 };
    uint32_t b[] = { 0x41, 0x42, 0x43 };
    uint32_t c[] = { 0x41, 0x42 };
    uint32_t needle[] = { 0x42, 0x43 };

    assert(utf32_validate(a, 3) == 1);
    { uint32_t bad[] = { 0x110000 }; assert(utf32_validate(bad, 1) == 0); }

    assert(utf32_equal(a, 3, b, 3) == 1);
    assert(utf32_equal(a, 3, c, 2) == 0);

    assert(utf32_find(a, 3, needle, 2, 0) == 1);
    assert(utf32_find(a, 3, needle, 2, 2) == -1);
    printf("test_validate_equal_find passed\n");
}

int main(void)
{
    test_count();
    test_from_utf8();
    test_to_utf8();
    test_validate_equal_find();
    printf("All utf32 tests passed\n");
    return 0;
}
