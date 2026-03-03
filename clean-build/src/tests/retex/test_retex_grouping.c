#include "../src/retex.h"
#include "../src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>

void test_grouping() {
    m_init();
    conststr_init();
    trace_level = 1;

    Backend *be = backend_cairo_create("test_group.png", 500, 500);
    assert(be != NULL);
    
    const char *text = "Plain {\\huge Huge} Plain";
    const char *text2 = "Plain {\\bf Bold} Plain";

    RetexParagraph *p1 = retex_layout(be, text, 10000.0, "Sans", 12.0, RETEX_ALIGN_LEFT);
    assert(p1 != NULL);
    
    double h = retex_paragraph_get_height(p1);
    printf("Grouping Height: %f\n", h);
    assert(h > 10.0);

    RetexParagraph *p2 = retex_layout(be, text2, 10000.0, "Sans", 12.0, RETEX_ALIGN_LEFT);
    assert(p2 != NULL);
    
    retex_paragraph_free(p1);
    retex_paragraph_free(p2);
    
    be->destroy(be);
    conststr_free();
    m_destruct();
}

int main() {
    test_grouping();
    printf("test_retex_grouping passed\n");
    return 0;
}
