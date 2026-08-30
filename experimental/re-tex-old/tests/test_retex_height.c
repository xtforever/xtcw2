#include "../src/retex.h"
#include "../src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>

void test_wrapped_height() {
    m_init();
    conststr_init();
    
    Backend *be = backend_cairo_create("dummy_height.png", 100, 100);
    assert(be != NULL);
    
    const char *text = "This is a relatively long sentence that should definitely wrap into multiple lines when the target width is set to something small like one hundred points.";
    
    RetexParagraph *p_wide = retex_layout(be, text, 1000.0, "Sans", 12.0, RETEX_ALIGN_JUSTIFY);
    double h_wide = retex_paragraph_get_height(p_wide);
    double w_wide = retex_paragraph_get_width(p_wide);
    printf("Wide Width: %f, Height: %f\n", w_wide, h_wide);
    
    RetexParagraph *p_narrow = retex_layout(be, text, 100.0, "Sans", 12.0, RETEX_ALIGN_JUSTIFY);
    double h_narrow = retex_paragraph_get_height(p_narrow);
    double w_narrow = retex_paragraph_get_width(p_narrow);
    printf("Narrow Width: %f, Height: %f\n", w_narrow, h_narrow);
    
    assert(h_narrow > h_wide * 2.0);
    assert(w_narrow <= 100.0);
    assert(w_narrow > 80.0);
    
    retex_paragraph_free(p_wide);
    retex_paragraph_free(p_narrow);
    be->destroy(be);
    conststr_free();
    m_destruct();
}

int main() {
    test_wrapped_height();
    printf("test_retex_height passed\n");
    return 0;
}
