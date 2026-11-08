/*
 * Phase 0: the rich (TeX) layout path must expose the same per-line geometry
 * and caret/hit-test metadata that the plain layout path already provides.
 *
 * The retrace pass in retex_layout() walks the paragraph vbox and records a
 * RetexLine per visual line (start/end source byte, y, height, vbox_index).
 */
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "mls.h"
#include "conststr.h"
#include "backend_cairo.h"
#include "retex.h"

static Backend *g_be;

/* "aa {\bf bb}" <newline> "cc" -- 2 paragraphs => 2 lines. */
static void test_rich_line_metadata(void)
{
    const char *src = "aa {\\bf bb}\ncc";
    RetexParagraph *p = retex_layout(g_be, src, 500.0, "Sans", 16.0, RETEX_ALIGN_LEFT);
    RetexLine lines[8];
    int n;

    assert(p != NULL);
    n = retex_paragraph_get_line_count(p);
    assert(n == 2);
    assert(retex_paragraph_get_lines(p, lines, 8) == 2);

    /* first line: 'a','a','b','b' at bytes 0,1,8,9 (groups/commands ignored). */
    assert(lines[0].start_byte == 0);
    assert(lines[0].end_byte > lines[0].start_byte);
    assert(lines[0].end_byte <= 11);      /* before the '\n' at byte 11 */

    /* second line starts at the 'c' after '\n' (byte 12). */
    assert(lines[1].start_byte == 12);
    assert(lines[1].end_byte >= lines[1].start_byte + 2);

    assert(lines[0].height > 0.0 && lines[1].height > 0.0);
    assert(fabs(lines[0].y) < 0.001);     /* first line top == paragraph top */
    assert(lines[1].y > lines[0].y);      /* vertical positions increase */

    retex_paragraph_free(p);
    printf("test_rich_line_metadata passed\n");
}

static void test_rich_caret_and_hit(void)
{
    RetexParagraph *p = retex_layout(g_be, "abc", 500.0, "Sans", 16.0, RETEX_ALIGN_LEFT);
    RetexLine l;
    double x0, y0, h0, x1, y1, h1;
    int off;

    assert(p != NULL);
    assert(retex_paragraph_get_line_count(p) == 1);
    assert(retex_paragraph_get_lines(p, &l, 1) == 1);
    assert(l.start_byte == 0 && l.end_byte == 3);

    assert(retex_paragraph_get_caret_pos(p, 0, &x0, &y0, &h0) == 1);
    assert(x0 >= 0.0);
    assert(fabs(y0 - l.y) < 0.001);
    assert(h0 > 0.0);

    assert(retex_paragraph_get_caret_pos(p, 1, &x1, &y1, &h1) == 1);
    assert(x1 > x0);

    off = retex_paragraph_get_offset_at(p, x1, l.y + l.height / 2.0);
    assert(off == 1);
    assert(retex_paragraph_get_offset_at(p, 10000.0, l.y + l.height / 2.0) == 3);

    retex_paragraph_free(p);
    printf("test_rich_caret_and_hit passed\n");
}

static void test_rich_wrap(void)
{
    RetexParagraph *p =
        retex_layout(g_be, "aaaa bbbb cccc dddd", 30.0, "Sans", 16.0, RETEX_ALIGN_LEFT);
    assert(p != NULL);
    assert(retex_paragraph_get_line_count(p) > 1); /* wrapped into several lines */
    retex_paragraph_free(p);
    printf("test_rich_wrap passed\n");
}

int main(void)
{
    m_init();
    conststr_init();
    g_be = backend_cairo_create("/tmp/test_retex_rich_lines.png", 400, 300);
    assert(g_be != NULL);

    test_rich_line_metadata();
    test_rich_caret_and_hit();
    test_rich_wrap();

    g_be->destroy(g_be);
    conststr_free();
    m_destruct();
    printf("All retex rich-line tests passed\n");
    return 0;
}
