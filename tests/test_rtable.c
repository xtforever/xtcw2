#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "mls.h"
#include "rtable.h"
#include "utf32.h"

static void assert_text(RTable *t, const char *expect)
{
    int h = rtable_to_utf8(t);
    assert(h != 0);
    assert(strcmp((char *)m_buf(h), expect) == 0);
    m_free(h);
}

static int insert_str(RTable *t, int pos, const char *s, RLook look)
{
    int n = utf32_count_utf8(s);
    uint32_t *cps;
    int r;

    assert(n >= 0);
    if (n == 0)
        return rtable_len(t);
    cps = (uint32_t *)malloc((size_t)n * sizeof(uint32_t));
    assert(cps);
    assert(utf32_from_utf8(s, cps, n) == n);
    r = rtable_insert_cps(t, pos, cps, n, look);
    free(cps);
    return r;
}

static const RLook PLAIN = { 0, 0, 0.0f };
static const RLook BOLD  = { 0, 1, 16.0f };

static void test_load_query(void)
{
    RTable *t = rtable_create();
    const char *uni = "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80"; /* A é 中 😀 */
    RAtom a;

    assert(rtable_load_plain(t, "hello", PLAIN) == 5);
    assert(rtable_len(t) == 5);
    assert(rtable_get(t, 0, &a) && a.kind == RATOM_CHAR && a.cp == 'h');
    assert(rtable_get(t, 4, &a) && a.cp == 'o');
    assert(!rtable_get(t, 5, &a));   /* out of range */
    assert(!rtable_get(t, -1, &a));
    assert_text(t, "hello");

    assert(rtable_load_plain(t, uni, PLAIN) == 4);
    assert(rtable_get(t, 0, &a) && a.cp == 0x41);
    assert(rtable_get(t, 1, &a) && a.cp == 0xE9);
    assert(rtable_get(t, 2, &a) && a.cp == 0x4E2D);
    assert(rtable_get(t, 3, &a) && a.cp == 0x1F600);
    assert_text(t, uni);

    /* invalid UTF-8 load fails and leaves the document intact */
    assert(rtable_load_plain(t, "bad\xC3\x28", PLAIN) == -1);
    assert_text(t, uni);

    assert(rtable_load_plain(t, "", PLAIN) == 0);
    assert(rtable_len(t) == 0);
    assert_text(t, "");

    rtable_free(t);
    printf("test_load_query passed\n");
}

static void test_insert_look_preserved(void)
{
    RTable *t = rtable_create();
    RAtom a;

    rtable_load_plain(t, "abcdef", PLAIN);
    insert_str(t, 3, "XY", BOLD);          /* abcXYdef, XY bold */
    assert_text(t, "abcXYdef");

    /* the inserted atoms keep the bold look after a later split */
    insert_str(t, 4, "_", PLAIN);          /* abcX_Ydef */
    assert(rtable_get(t, 3, &a) && a.cp == 'X' && a.look.style == 1);
    assert(rtable_get(t, 4, &a) && a.cp == '_' && a.look.style == 0);
    assert(rtable_get(t, 5, &a) && a.cp == 'Y' && a.look.style == 1);
    assert_text(t, "abcX_Ydef");

    /* insert at an existing piece boundary */
    insert_str(t, 0, "12", PLAIN);
    assert_text(t, "12abcX_Ydef");

    /* invalid atom rejected, no change */
    {
        RAtom bad = { RATOM_CHAR, 0xD800, PLAIN, 0 };
        int before = rtable_len(t);
        assert(rtable_insert(t, 0, &bad, 1) == -1);
        assert(rtable_len(t) == before);
    }

    rtable_free(t);
    printf("test_insert_look_preserved passed\n");
}

static void test_object_atom(void)
{
    RTable *t = rtable_create();
    RAtom obj = { RATOM_OBJECT, ROBJ_SVG, { 0, 0, 0.0f }, 42 };
    RAtom a;

    rtable_load_plain(t, "ab", PLAIN);
    assert(rtable_insert(t, 1, &obj, 1) == 3);   /* a <svg> b */
    assert(rtable_get(t, 1, &a) && a.kind == RATOM_OBJECT && a.cp == ROBJ_SVG
           && a.payload == 42);
    /* objects flatten to U+FFFC (EF BF BC) in UTF-8 */
    assert_text(t, "a\xEF\xBF\xBC" "b");

    /* undo restores both text and the object */
    rtable_free(t);
    printf("test_object_atom passed\n");
}

static void test_delete(void)
{
    RTable *t = rtable_create();

    rtable_load_plain(t, "abcdef", PLAIN);
    insert_str(t, 3, "XY", PLAIN);          /* abcXYdef */
    assert_text(t, "abcXYdef");
    rtable_delete(t, 3, 2);                 /* abcdef */
    assert_text(t, "abcdef");

    rtable_load_plain(t, "abcdef", PLAIN);
    insert_str(t, 2, "X", PLAIN);           /* abXcdef */
    insert_str(t, 4, "Y", PLAIN);           /* abXcYdef */
    assert_text(t, "abXcYdef");
    rtable_delete(t, 2, 3);                 /* abdef */
    assert_text(t, "abdef");

    rtable_delete(t, 0, rtable_len(t));
    assert(rtable_len(t) == 0);
    assert_text(t, "");

    rtable_free(t);
    printf("test_delete passed\n");
}

static void test_undo_redo(void)
{
    RTable *t = rtable_create();

    rtable_load_plain(t, "abc", PLAIN);

    rtable_snapshot(t);
    insert_str(t, 3, "d", BOLD);            /* abcd, d bold */
    rtable_snapshot(t);
    insert_str(t, 0, "X", PLAIN);           /* Xabcd */
    rtable_snapshot(t);
    rtable_delete(t, 0, 1);                 /* abcd */

    assert_text(t, "abcd");
    assert(rtable_undo_depth(t) == 3);

    assert(rtable_undo(t) == 1);
    assert_text(t, "Xabcd");
    assert(rtable_undo(t) == 1);
    assert_text(t, "abcd");
    { RAtom a; assert(rtable_get(t, 3, &a) && a.cp == 'd' && a.look.style == 1); }
    assert(rtable_undo(t) == 1);
    assert_text(t, "abc");
    assert(rtable_undo(t) == 0);

    assert(rtable_redo(t) == 1);
    assert_text(t, "abcd");
    assert(rtable_redo(t) == 1);
    assert_text(t, "Xabcd");
    assert(rtable_redo(t) == 1);
    assert_text(t, "abcd");
    assert(rtable_redo_depth(t) == 0);

    rtable_free(t);
    printf("test_undo_redo passed\n");
}

static void test_look_edits(void)
{
    RTable *t = rtable_create();
    RAtom a;

    rtable_load_plain(t, "abcd", PLAIN);

    rtable_snapshot(t);
    rtable_apply_style(t, 1, 2, 1, 0);            /* b,c bold */
    assert(rtable_get(t, 1, &a) && a.look.style == 1);
    assert(rtable_get(t, 2, &a) && a.look.style == 1);
    assert(rtable_get(t, 0, &a) && a.look.style == 0);
    assert_text(t, "abcd");

    /* face/size changes preserve the style bits */
    rtable_snapshot(t);
    rtable_set_size(t, 1, 2, 24.0f);
    assert(rtable_get(t, 1, &a) && a.look.style == 1 && a.look.size == 24.0f);
    assert(rtable_get(t, 2, &a) && a.look.size == 24.0f);

    rtable_snapshot(t);
    rtable_set_face(t, 0, 4, 7);
    assert(rtable_get(t, 0, &a) && a.look.face == 7);
    assert(rtable_get(t, 3, &a) && a.look.face == 7);

    rtable_snapshot(t);
    rtable_set_features(t, 0, 4, conststr_lookup_c("liga=0,clig=0"));
    assert(rtable_get(t, 0, &a) &&
           a.look.features == conststr_lookup_c("liga=0,clig=0"));
    assert(rtable_get(t, 3, &a) &&
           a.look.features == conststr_lookup_c("liga=0,clig=0"));

    /* undo unwinds each look edit */
    assert(rtable_undo(t) == 1);
    assert(rtable_get(t, 0, &a) && a.look.features == 0);
    assert(rtable_undo(t) == 1);
    assert(rtable_get(t, 0, &a) && a.look.face == 0);
    assert(rtable_undo(t) == 1);
    assert(rtable_get(t, 1, &a) && a.look.size == 0.0f && a.look.style == 1);
    assert(rtable_undo(t) == 1);
    assert(rtable_get(t, 1, &a) && a.look.style == 0);

    rtable_free(t);
    printf("test_look_edits passed\n");
}

int main(void)
{
    m_init();
    conststr_init();
    test_load_query();
    test_insert_look_preserved();
    test_object_atom();
    test_delete();
    test_undo_redo();
    test_look_edits();
    m_destruct();
    printf("All rtable tests passed\n");
    return 0;
}
