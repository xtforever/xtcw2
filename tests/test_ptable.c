#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "mls.h"
#include "ptable.h"
#include "utf32.h"

static void assert_text(PieceTable *t, const char *expect)
{
    int h = ptable_to_utf8(t);
    assert(h != 0);
    assert(strcmp((char *)m_buf(h), expect) == 0);
    m_free(h);
}

static int insert_str(PieceTable *t, int pos, const char *s)
{
    int n = utf32_count_utf8(s);
    uint32_t *cps;
    int r;

    assert(n >= 0);
    if (n == 0)
        return ptable_len(t);
    cps = (uint32_t *)malloc((size_t)n * sizeof(uint32_t));
    assert(cps);
    assert(utf32_from_utf8(s, cps, n) == n);
    r = ptable_insert(t, pos, cps, n);
    free(cps);
    return r;
}

/* ---- part A: load / query / save -------------------------------------- */

static void test_load_query(void)
{
    PieceTable *t = ptable_create();
    int u32 = m_create(16, sizeof(uint32_t));
    const char *uni = "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80"; /* A é 中 😀 */
    int n;

    assert(ptable_load_utf8(t, "hello") == 5);
    assert(ptable_len(t) == 5);
    assert(ptable_get(t, 0) == 'h');
    assert(ptable_get(t, 4) == 'o');
    assert(ptable_get(t, 5) == 0);   /* out of range */
    assert(ptable_get(t, -1) == 0);

    n = ptable_flatten(t, u32);
    assert(n == 5);
    assert(((uint32_t *)m_buf(u32))[1] == 'e');
    assert_text(t, "hello");

    /* multi-byte code points */
    assert(ptable_load_utf8(t, uni) == 4);
    assert(ptable_get(t, 0) == 0x41);
    assert(ptable_get(t, 1) == 0xE9);
    assert(ptable_get(t, 2) == 0x4E2D);
    assert(ptable_get(t, 3) == 0x1F600);
    assert_text(t, uni);

    /* invalid UTF-8 load fails and leaves the document intact */
    assert(ptable_load_utf8(t, "bad\xC3\x28") == -1);
    assert_text(t, uni);

    /* empty document */
    assert(ptable_load_utf8(t, "") == 0);
    assert(ptable_len(t) == 0);
    assert_text(t, "");
    assert(ptable_get(t, 0) == 0);

    m_free(u32);
    ptable_free(t);
    printf("test_load_query passed\n");
}

static void test_save_append(void)
{
    PieceTable *t = ptable_create();
    int out = m_create(8, 1);

    ptable_load_utf8(t, "abc");
    assert(ptable_save_utf8(t, out) == 3);
    ptable_load_utf8(t, "de");
    assert(ptable_save_utf8(t, out) == 2);
    assert(strcmp((char *)m_buf(out), "abcde") == 0);

    m_free(out);
    ptable_free(t);
    printf("test_save_append passed\n");
}

/* ---- part B: insert / delete ------------------------------------------ */

static void test_insert(void)
{
    PieceTable *t = ptable_create();

    ptable_load_utf8(t, "abcdef");
    insert_str(t, 3, "XY");            /* abcXYdef */
    assert_text(t, "abcXYdef");
    insert_str(t, 0, "12");            /* 12abcXYdef */
    assert_text(t, "12abcXYdef");
    insert_str(t, ptable_len(t), "Z"); /* append */
    assert_text(t, "12abcXYdefZ");

    /* insert at an existing piece boundary (after the middle insert) */
    insert_str(t, 2, "_");             /* 12_abcXYdefZ */
    assert_text(t, "12_abcXYdefZ");

    /* multi-byte insert */
    insert_str(t, 0, "\xE4\xB8\xAD");  /* 中 */
    assert_text(t, "\xE4\xB8\xAD" "12_abcXYdefZ");

    /* invalid code point rejected, no change */
    {
        uint32_t bad[] = { 0xD800 };
        int before = ptable_len(t);
        assert(ptable_insert(t, 0, bad, 1) == -1);
        assert(ptable_len(t) == before);
    }

    ptable_free(t);
    printf("test_insert passed\n");
}

static void test_delete(void)
{
    PieceTable *t = ptable_create();

    ptable_load_utf8(t, "abcdef");
    insert_str(t, 3, "XY");            /* abcXYdef */
    assert_text(t, "abcXYdef");

    ptable_delete(t, 3, 2);            /* abcdef */
    assert_text(t, "abcdef");

    /* delete spanning piece boundaries: build 1|2|3 pieces then remove across */
    ptable_load_utf8(t, "abcdef");
    insert_str(t, 2, "X");             /* abXcdef */
    insert_str(t, 4, "Y");             /* abXcYdef */
    assert_text(t, "abXcYdef");
    ptable_delete(t, 2, 3);            /* remove "XcY" -> abdef */
    assert_text(t, "abdef");

    /* delete everything */
    ptable_delete(t, 0, ptable_len(t));
    assert(ptable_len(t) == 0);
    assert_text(t, "");

    /* out-of-range / empty deletes are harmless */
    ptable_load_utf8(t, "abc");
    ptable_delete(t, 10, 5);
    ptable_delete(t, -3, 0);
    assert_text(t, "abc");

    /* multi-byte delete */
    ptable_load_utf8(t, "\xE4\xB8\xAD" "ab"); /* 中ab */
    ptable_delete(t, 0, 1);
    assert_text(t, "ab");

    ptable_free(t);
    printf("test_delete passed\n");
}

static void test_undo_after_edit(void)
{
    PieceTable *t = ptable_create();

    ptable_load_utf8(t, "abcdef");
    ptable_snapshot(t);
    insert_str(t, 2, "ZZ");            /* abZZcdef */
    assert_text(t, "abZZcdef");

    /* undo, then a new insert at the smaller add_len must be correct */
    assert(ptable_undo(t) == 1);
    assert_text(t, "abcdef");
    insert_str(t, 3, "QQ");            /* abcQQdef */
    assert_text(t, "abcQQdef");

    ptable_free(t);
    printf("test_undo_after_edit passed\n");
}

/* ---- part C: undo / redo ---------------------------------------------- */

static void test_undo_redo(void)
{
    PieceTable *t = ptable_create();

    ptable_load_utf8(t, "abc");

    ptable_snapshot(t);
    insert_str(t, 3, "d");             /* abcd */
    ptable_snapshot(t);
    insert_str(t, 0, "X");             /* Xabcd */
    ptable_snapshot(t);
    ptable_delete(t, 0, 1);            /* abcd */

    assert_text(t, "abcd");
    assert(ptable_undo_depth(t) == 3);

    assert(ptable_undo(t) == 1);
    assert_text(t, "Xabcd");
    assert(ptable_undo(t) == 1);
    assert_text(t, "abcd");
    assert(ptable_undo(t) == 1);
    assert_text(t, "abc");
    assert(ptable_undo_depth(t) == 0);
    assert(ptable_undo(t) == 0);       /* nothing left */

    assert(ptable_redo_depth(t) == 3);
    assert(ptable_redo(t) == 1);
    assert_text(t, "abcd");
    assert(ptable_redo(t) == 1);
    assert_text(t, "Xabcd");
    assert(ptable_redo(t) == 1);
    assert_text(t, "abcd");
    assert(ptable_redo_depth(t) == 0);

    ptable_free(t);
    printf("test_undo_redo passed\n");
}

static void test_redo_cleared_on_edit(void)
{
    PieceTable *t = ptable_create();

    ptable_load_utf8(t, "abc");
    ptable_snapshot(t);
    insert_str(t, 3, "d");             /* abcd */
    assert(ptable_undo(t) == 1);       /* abc */
    assert(ptable_redo_depth(t) == 1);

    /* a new edit must clear redo */
    ptable_snapshot(t);
    insert_str(t, 0, "Z");             /* Zabc */
    assert(ptable_redo_depth(t) == 0);
    assert_text(t, "Zabc");

    ptable_free(t);
    printf("test_redo_cleared_on_edit passed\n");
}

int main(void)
{
    m_init();
    test_load_query();
    test_save_append();
    test_insert();
    test_delete();
    test_undo_after_edit();
    test_undo_redo();
    test_redo_cleared_on_edit();
    m_destruct();
    printf("All ptable tests passed\n");
    return 0;
}
