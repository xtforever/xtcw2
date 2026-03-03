#include "../src/retex.h"
#include "../src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

void test_dpi_consistency() {
    m_init();
    conststr_init();
    trace_level = 1;

    Backend *be = backend_cairo_create("test_dpi.png", 500, 500);
    assert(be != NULL);
    
    const char *text = "Hello World";
    double font_size = 12.0;
    
    RetexParagraph *p = retex_layout(be, text, 1000.0, "Sans", font_size, RETEX_ALIGN_LEFT);
    assert(p != NULL);
    
    double h = retex_paragraph_get_height(p);
    double w = retex_paragraph_get_width(p);
    double fh = retex_paragraph_get_first_line_height(p);
    
    printf("Font size: %f\n", font_size);
    printf("Width: %f pt\n", w);
    printf("Height: %f pt\n", h);
    printf("First line height: %f pt\n", fh);
    
    assert(h > font_size);
    assert(h < font_size * 2.0);
    assert(fh > font_size * 0.5);
    assert(fh < font_size * 1.5);

    retex_paragraph_free(p);
    be->destroy(be);
    conststr_free();
    m_destruct();
}

int main() {
    test_dpi_consistency();
    printf("test_retex_measure_dpi passed\n");
    return 0;
}
