#include "../src/retex.h"
#include "../src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>

void test_grouping_multiple() {
    m_init();
    conststr_init();
    trace_level = 1;

    Backend *be = backend_cairo_create("test_group_multi.png", 500, 500);
    assert(be != NULL);
    
    // Multiple groups in a row
    const char *text = "Plain {\\bf test} of the {\\huge Retex} engine.";

    RetexParagraph *p = retex_layout(be, text, 10000.0, "Sans", 12.0, RETEX_ALIGN_LEFT);
    assert(p != NULL);
    
    double h = retex_paragraph_get_height(p);
    printf("Grouping Height: %f\n", h);
    
    assert(h < 35.0); 

    retex_paragraph_free(p);
    be->destroy(be);
    conststr_free();
    m_destruct();
}

int main() {
    test_grouping_multiple();
    printf("test_retex_grouping_multiple passed\n");
    return 0;
}
