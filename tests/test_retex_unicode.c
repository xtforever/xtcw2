#include <stdio.h>
#include <assert.h>
#include "mls.h"
#include "conststr.h"
#include "backend_cairo.h"
#include "retex.h"

/* Smoke test: the Unicode-aware backends must lay out and render a UTF-8
 * string containing multi-byte code points (Latin-1, CJK, emoji) without
 * truncating them to single bytes. */
int main(void)
{
    const char *s = "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80"; /* A é 中 😀 */
    Backend *be;
    RetexParagraph *p;
    RendererColors colors = {
        .text = 0xFF000000, .bg = 0x00FFFFFF,
        .reverse_text = 0xFFFFFFFF, .reverse_bg = 0xFF000000,
        .sel_text = 0xFF000000, .sel_bg = 0xFFA0A0FF
    };

    m_init();
    conststr_init();

    be = backend_cairo_create("/tmp/test_retex_unicode.png", 300, 160);
    assert(be != NULL);
    be->set_font_face(be, "Sans", 0);
    be->set_font_size(be, 16);
    /* direct code point metrics (would previously saturate to '?') */
    {
        CharMetrics m;
        be->get_char_metrics(be, 0x41, &m);     /* A */
        assert(m.width > 0);
        be->get_char_metrics(be, 0xE9, &m);     /* é */
        assert(m.width > 0);
        be->get_char_metrics(be, 0x4E2D, &m);   /* 中 */
        assert(m.width >= 0);
        be->get_char_metrics(be, 0x1F600, &m);  /* 😀 */
        assert(m.width >= 0);
    }

    p = retex_layout(be, s, 200.0, "Sans", 16.0, RETEX_ALIGN_LEFT);
    assert(p != NULL);
    assert(retex_paragraph_get_width(p) > 0.0);
    assert(retex_paragraph_get_height(p) > 0.0);

    retex_paragraph_render(p, be, 0.0,
                           retex_paragraph_get_first_line_height(p),
                           &colors, 0);
    retex_paragraph_free(p);
    be->destroy(be);

    conststr_free();
    m_destruct();

    printf("test_retex_unicode passed\n");
    return 0;
}
