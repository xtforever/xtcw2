#include "../src/retex.h"
#include "../src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>

void test_grouping_nested() {
    m_init();
    conststr_init();
    trace_level = 1;

    Backend *be = backend_cairo_create("test_group_nested.png", 500, 500);
    assert(be != NULL);
    
    // Nested groups
    const char *text = "Plain {\\bf Bold {\\huge BoldHuge} BoldAgain} PlainAgain";

    RetexParagraph *p = retex_layout(be, text, 10000.0, "Sans", 12.0, RETEX_ALIGN_LEFT);
    assert(p != NULL);
    
    retex_paragraph_free(p);
    be->destroy(be);
    conststr_free();
    m_destruct();
}

int main() {
    test_grouping_nested();
    printf("test_retex_grouping_nested passed\n");
    return 0;
}
