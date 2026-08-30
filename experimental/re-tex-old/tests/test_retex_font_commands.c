#include "../src/retex.h"
#include "../src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>

void test_font_commands() {
    m_init();
    conststr_init();
    trace_level = 1;

    Backend *be = backend_cairo_create("test_font.png", 500, 500);
    assert(be != NULL);
    
    const char *text_plain = "Hello World";
    const char *text_bold = "\\bf Hello World";
    const char *text_huge = "\\huge Hello World";
    const char *text_mixed = "Plain \\bf Bold \\it Italic \\normalsize Plain Again";

    RetexParagraph *p_plain = retex_layout(be, text_plain, 10000.0, "Sans", 12.0, RETEX_ALIGN_LEFT);
    double w_plain = retex_paragraph_get_width(p_plain);
    double h_plain = retex_paragraph_get_height(p_plain);
    printf("Plain - Width: %f, Height: %f\n", w_plain, h_plain);

    RetexParagraph *p_bold = retex_layout(be, text_bold, 10000.0, "Sans", 12.0, RETEX_ALIGN_LEFT);
    double w_bold = retex_paragraph_get_width(p_bold);
    printf("Bold - Width: %f\n", w_bold);

    RetexParagraph *p_huge = retex_layout(be, text_huge, 10000.0, "Sans", 12.0, RETEX_ALIGN_LEFT);
    double h_huge = retex_paragraph_get_height(p_huge);
    double w_huge = retex_paragraph_get_width(p_huge);
    printf("Huge - Width: %f, Height: %f\n", w_huge, h_huge);

    RetexParagraph *p_mixed = retex_layout(be, text_mixed, 10000.0, "Sans", 12.0, RETEX_ALIGN_LEFT);
    double w_mixed = retex_paragraph_get_width(p_mixed);
    printf("Mixed - Width: %f\n", w_mixed);

    // Bold should be wider than plain
    assert(w_bold > w_plain);
    // Huge should be much taller and wider than plain
    assert(h_huge > h_plain * 1.5);
    assert(w_huge > w_plain * 1.5);
    // Mixed should be wider than plain
    assert(w_mixed > w_plain);

    retex_paragraph_free(p_plain);
    retex_paragraph_free(p_bold);
    retex_paragraph_free(p_huge);
    retex_paragraph_free(p_mixed);
    
    be->destroy(be);
    conststr_free();
    m_destruct();
}

int main() {
    test_font_commands();
    printf("test_retex_font_commands passed\n");
    return 0;
}
