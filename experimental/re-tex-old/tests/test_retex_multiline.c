#include "../src/retex.h"
#include "../src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>

void test_multiline_height() {
    m_init();
    conststr_init();
    trace_level = 1;

    Backend *be = backend_cairo_create("test_multi.png", 500, 500);
    assert(be != NULL);
    
    // Two lines separated by a paragraph break
    const char *text = "Line one\n\nLine two";
    double font_size = 10.0;
    
    RetexParagraph *p = retex_layout(be, text, 1000.0, "Sans", font_size, RETEX_ALIGN_LEFT);
    assert(p != NULL);
    
    double h = retex_paragraph_get_height(p);
    printf("Multiline Height: %f pt\n", h);
    
    assert(h > 15.0);
    assert(h < 30.0);
    
    double fh = retex_paragraph_get_first_line_height(p);
    printf("First line height: %f pt\n", fh);
    assert(fh > 5.0 && fh < 12.0);

    retex_paragraph_free(p);
    be->destroy(be);
    conststr_free();
    m_destruct();
}

int main() {
    test_multiline_height();
    printf("test_retex_multiline passed\n");
    return 0;
}
