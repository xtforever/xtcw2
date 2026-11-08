/*
 * Phase 2: retex_layout_styled() builds a paragraph directly from rich atoms,
 * honoring each atom's font face / style / size, and records the same per-line
 * + caret metadata as the plain layout.
 */
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "mls.h"
#include "conststr.h"
#include "backend_cairo.h"
#include "retex.h"

static Backend *g_be;
static int g_sans, g_serif;

static RAtom mkchar(uint32_t cp, int face, int style, float size)
{
    RAtom a;
    a.kind = RATOM_CHAR;
    a.cp = cp;
    a.look.face = face;
    a.look.style = style;
    a.look.size = size;
    a.payload = 0;
    return a;
}

static void test_styled_fonts(void)
{
    /* A (plain) B (bold) C (serif) D (italic 28pt) E (plain) */
    RAtom atoms[5];
    RetexParagraph *p;
    RetexLine l;
    int style, face;
    double size;

    atoms[0] = mkchar('A', g_sans, 0, 16.0f);
    atoms[1] = mkchar('B', g_sans, 1, 16.0f);
    atoms[2] = mkchar('C', g_serif, 0, 16.0f);
    atoms[3] = mkchar('D', g_sans, 2, 28.0f);
    atoms[4] = mkchar('E', g_sans, 0, 16.0f);

    p = retex_layout_styled(g_be, atoms, 5, 500.0, "Sans", 16.0, RETEX_ALIGN_LEFT);
    assert(p != NULL);
    assert(retex_paragraph_get_line_count(p) == 1);
    assert(retex_paragraph_get_lines(p, &l, 1) == 1);
    assert(l.start_byte == 0);
    assert(l.end_byte == 5);
    assert(l.height > 0.0);

    assert(retex_paragraph_get_font_at(p, 0, &style, &size, &face));
    assert(style == 0 && fabs(size - 16.0) < 0.001 && face == g_sans);

    assert(retex_paragraph_get_font_at(p, 1, &style, &size, &face));
    assert(style == 1 && fabs(size - 16.0) < 0.001);       /* bold */

    assert(retex_paragraph_get_font_at(p, 2, &style, &size, &face));
    assert(style == 0 && face == g_serif);                  /* serif face */

    assert(retex_paragraph_get_font_at(p, 3, &style, &size, &face));
    assert(style == 2 && fabs(size - 28.0) < 0.001);        /* italic, large */

    assert(retex_paragraph_get_font_at(p, 4, &style, &size, &face));
    assert(style == 0 && fabs(size - 16.0) < 0.001 && face == g_sans);

    /* a space / out-of-range position has no character node */
    assert(!retex_paragraph_get_font_at(p, 99, NULL, NULL, NULL));

    retex_paragraph_free(p);
    printf("test_styled_fonts passed\n");
}

static void test_bold_wider_than_plain(void)
{
    RAtom plain[4], bold[4];
    RetexParagraph *pp, *bp;
    int i;

    for (i = 0; i < 4; i++) {
        plain[i] = mkchar('m', g_sans, 0, 16.0f);
        bold[i] = mkchar('m', g_sans, 1, 16.0f);
    }
    pp = retex_layout_styled(g_be, plain, 4, 500.0, "Sans", 16.0, RETEX_ALIGN_LEFT);
    bp = retex_layout_styled(g_be, bold, 4, 500.0, "Sans", 16.0, RETEX_ALIGN_LEFT);
    assert(pp && bp);
    assert(retex_paragraph_get_width(bp) >= retex_paragraph_get_width(pp));
    retex_paragraph_free(pp);
    retex_paragraph_free(bp);
    printf("test_bold_wider_than_plain passed\n");
}

static void test_newline_and_caret(void)
{
    /* "a b\nc" : line 0 = "a b" (bytes 0..3), line 1 = "c" (byte 4) */
    RAtom atoms[5];
    RetexParagraph *p;
    RetexLine lines[4];
    double x, y, h;
    int off;

    atoms[0] = mkchar('a', g_sans, 0, 16.0f);
    atoms[1] = mkchar(' ', g_sans, 0, 16.0f);
    atoms[2] = mkchar('b', g_sans, 0, 16.0f);
    atoms[3] = mkchar('\n', g_sans, 0, 16.0f);
    atoms[4] = mkchar('c', g_sans, 0, 16.0f);

    p = retex_layout_styled(g_be, atoms, 5, 500.0, "Sans", 16.0, RETEX_ALIGN_LEFT);
    assert(p != NULL);
    assert(retex_paragraph_get_line_count(p) == 2);
    assert(retex_paragraph_get_lines(p, lines, 4) == 2);
    assert(lines[0].start_byte == 0);
    assert(lines[0].end_byte == 3);
    assert(lines[1].start_byte == 4);
    assert(lines[1].end_byte == 5);
    assert(lines[1].y > lines[0].y);

    /* caret at the start of line 0 */
    assert(retex_paragraph_get_caret_pos(p, 0, &x, &y, &h) == 1);
    assert(fabs(y - lines[0].y) < 0.001);
    assert(h > 0.0);

    /* hit-test just after 'a' maps to byte 1 */
    {
        double x1, y1, h1;
        assert(retex_paragraph_get_caret_pos(p, 1, &x1, &y1, &h1) == 1);
        off = retex_paragraph_get_offset_at(p, x1, lines[0].y + lines[0].height / 2.0);
        assert(off == 1);
    }

    retex_paragraph_free(p);
    printf("test_newline_and_caret passed\n");
}

static void test_wrap(void)
{
    RAtom atoms[32];
    RetexParagraph *p;
    int i;

    for (i = 0; i < 32; i++)
        atoms[i] = mkchar((i % 8 == 7) ? ' ' : 'x', g_sans, 0, 16.0f);
    p = retex_layout_styled(g_be, atoms, 32, 40.0, "Sans", 16.0, RETEX_ALIGN_LEFT);
    assert(p != NULL);
    assert(retex_paragraph_get_line_count(p) > 1);
    retex_paragraph_free(p);
    printf("test_wrap passed\n");
}

static void test_trailing_space_caret(void)
{
    /* "ab \ncd": pressing space at the end of line 0 must keep the caret on
     * line 0, even though the trailing space is not part of a "word". */
    RAtom atoms[6];
    RetexParagraph *p;
    RetexLine lines[4];
    double x, y, h;

    atoms[0] = mkchar('a', g_sans, 0, 16.0f);
    atoms[1] = mkchar('b', g_sans, 0, 16.0f);
    atoms[2] = mkchar(' ', g_sans, 0, 16.0f);
    atoms[3] = mkchar('\n', g_sans, 0, 16.0f);
    atoms[4] = mkchar('c', g_sans, 0, 16.0f);
    atoms[5] = mkchar('d', g_sans, 0, 16.0f);

    p = retex_layout_styled(g_be, atoms, 6, 500.0, "Sans", 16.0, RETEX_ALIGN_LEFT);
    assert(p != NULL);
    assert(retex_paragraph_get_lines(p, lines, 4) == 2);
    /* line 0 range includes the trailing space up to the '\n' (byte 3) */
    assert(lines[0].start_byte == 0);
    assert(lines[0].end_byte == 3);
    assert(lines[1].start_byte == 4);

    /* caret immediately after the trailing space maps to line 0 */
    assert(retex_paragraph_get_caret_pos(p, 3, &x, &y, &h) == 1);
    assert(fabs(y - lines[0].y) < 0.001);
    retex_paragraph_free(p);

    /* same at the very end of the document (no newline): "ab " */
    p = retex_layout_styled(g_be, atoms, 3, 500.0, "Sans", 16.0, RETEX_ALIGN_LEFT);
    assert(p != NULL);
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
    g_sans = conststr_lookup_c("Sans");
    g_serif = conststr_lookup_c("Serif");
    g_be = backend_cairo_create("/tmp/test_retex_styled.png", 400, 300);
    assert(g_be != NULL);

    test_styled_fonts();
    test_bold_wider_than_plain();
    test_newline_and_caret();
    test_wrap();
    test_trailing_space_caret();

    g_be->destroy(g_be);
    conststr_free();
    m_destruct();
    printf("All retex styled tests passed\n");
    return 0;
}
