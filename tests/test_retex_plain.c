#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "mls.h"
#include "conststr.h"
#include "backend_cairo.h"
#include "retex.h"

static Backend *g_be;

static RetexParagraph *lay(const char *s, double w)
{
    return retex_layout_plain(g_be, s, w, "Sans", 16.0, RETEX_ALIGN_LEFT);
}

static void test_hard_breaks_and_blank_lines(void)
{
    RetexParagraph *p = lay("line1\n\nline3", 500);
    RetexLine lines[8];
    int n;

    assert(p != NULL);
    n = retex_paragraph_get_line_count(p);
    assert(n == 3);
    assert(retex_paragraph_get_lines(p, lines, 8) == 3);
    assert(lines[0].height > 0.0);
    assert(lines[1].height > 0.0); /* blank line keeps a positive height */
    assert(lines[2].height > 0.0);
    /* lines start at successive byte offsets: "line1"=5, "\n"=1, blank, "line3" */
    assert(lines[0].start_byte == 0);
    assert(lines[1].start_byte == 6);  /* after "line1\n" */
    assert(lines[2].start_byte == 7);  /* after the second '\n' */
    /* vertical positions increase */
    assert(lines[1].y > lines[0].y);
    assert(lines[2].y > lines[1].y);
    retex_paragraph_free(p);
    printf("test_hard_breaks_and_blank_lines passed\n");
}

static void test_trailing_newline_and_empty(void)
{
    RetexParagraph *p;

    /* "a\n" = two lines: "a" and an empty line */
    p = lay("a\n", 500);
    assert(retex_paragraph_get_line_count(p) == 2);
    retex_paragraph_free(p);

    /* empty document = one empty line */
    p = lay("", 500);
    assert(retex_paragraph_get_line_count(p) == 1);
    {
        RetexLine l;
        assert(retex_paragraph_get_lines(p, &l, 1) == 1);
        assert(l.start_byte == 0 && l.end_byte == 0);
        assert(l.height > 0.0);
    }
    retex_paragraph_free(p);
    printf("test_trailing_newline_and_empty passed\n");
}

static void test_wrap(void)
{
    RetexParagraph *p = lay("aaaa bbbb cccc dddd", 30);
    int n;

    assert(p != NULL);
    n = retex_paragraph_get_line_count(p);
    assert(n > 1); /* wrapped into several visual lines */
    retex_paragraph_free(p);
    printf("test_wrap passed\n");
}

static void test_caret_and_hit(void)
{
    RetexParagraph *p = lay("abc", 500);
    RetexLine l;
    double x0, y0, h0, x1, y1, h1;
    int off;

    assert(retex_paragraph_get_lines(p, &l, 1) == 1);

    assert(retex_paragraph_get_caret_pos(p, 0, &x0, &y0, &h0) == 1);
    assert(x0 >= 0.0);
    assert(fabs(y0 - l.y) < 0.001);
    assert(h0 > 0.0);

    assert(retex_paragraph_get_caret_pos(p, 1, &x1, &y1, &h1) == 1);
    assert(x1 > x0);

    /* hit test just right of the first glyph boundary should map to 1 */
    off = retex_paragraph_get_offset_at(p, x1, l.y + l.height / 2.0);
    assert(off == 1);

    /* beyond the line end maps to the end offset */
    assert(retex_paragraph_get_offset_at(p, 10000.0, l.y + l.height / 2.0) == 3);

    /* caret at end */
    {
        double xe, ye, he;
        assert(retex_paragraph_get_caret_pos(p, 3, &xe, &ye, &he) == 1);
        assert(xe > x1);
    }

    retex_paragraph_free(p);
    printf("test_caret_and_hit passed\n");
}

static void test_unicode_lines(void)
{
    const char *s = "A\xC3\xA9\n\xE4\xB8\xAD\xF0\x9F\x98\x80";
    RetexParagraph *p = lay(s, 500);
    RetexLine lines[4];

    assert(p != NULL);
    assert(retex_paragraph_get_line_count(p) == 2);
    assert(retex_paragraph_get_lines(p, lines, 4) == 2);
    /* second line starts after "Aé\n" = 4 bytes */
    assert(lines[1].start_byte == 4);
    retex_paragraph_free(p);
    printf("test_unicode_lines passed\n");
}

static void test_trailing_space_caret(void)
{
    RetexParagraph *p;
    RetexLine lines[4];
    double x, y, h;

    /* "ab \ncd": the trailing space belongs to line 0, so a caret after it
     * must stay on line 0 (regression: it used to jump to the next line). */
    p = lay("ab \ncd", 500);
    assert(retex_paragraph_get_lines(p, lines, 4) == 2);
    assert(lines[0].start_byte == 0);
    assert(lines[0].end_byte == 3);
    assert(lines[1].start_byte == 4);
    assert(retex_paragraph_get_caret_pos(p, 3, &x, &y, &h) == 1);
    assert(fabs(y - lines[0].y) < 0.001);
    retex_paragraph_free(p);

    /* end of document "ab " */
    p = lay("ab ", 500);
    assert(retex_paragraph_get_lines(p, lines, 4) == 1);
    assert(lines[0].end_byte == 3);
    assert(retex_paragraph_get_caret_pos(p, 3, &x, &y, &h) == 1);
    assert(fabs(y - lines[0].y) < 0.001);
    retex_paragraph_free(p);

    printf("test_trailing_space_caret passed\n");
}

int main(void)
{
    m_init();
    conststr_init();
    g_be = backend_cairo_create("/tmp/test_retex_plain.png", 400, 300);
    assert(g_be != NULL);

    test_hard_breaks_and_blank_lines();
    test_trailing_newline_and_empty();
    test_wrap();
    test_caret_and_hit();
    test_unicode_lines();
    test_trailing_space_caret();

    g_be->destroy(g_be);
    conststr_free();
    m_destruct();
    printf("All retex plain tests passed\n");
    return 0;
}
