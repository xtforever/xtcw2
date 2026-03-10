#include <stdio.h>
#include <assert.h>
#include "mls.h"
#include "retex.h"
#include "backend_cairo.h"
#include "m_tool.h"

int main() {
    m_init();
    conststr_init();
    
    Backend *be = backend_cairo_create("hit_test.png", 200, 100);
    if (!be) return 1;

    // A single character 'A'
    RetexParagraph *para = retex_layout(be, "A", 200.0, "serif", 12.0, RETEX_ALIGN_LEFT);
    if (para) {
        double first_h = retex_paragraph_get_first_line_height(para);
        // retex_paragraph_get_node_at(para, x, y) expects y relative to paragraph top.
        // The first line baseline is at y = first_h.
        // Char 'A' extends from baseline up to first_h - height.
        // So y = first_h - 2 should be inside.
        
        int node_data = retex_paragraph_get_node_at(para, 5.0, first_h - 2.0);
        printf("Hit at (5, %f): %d (expected 0 because it's the first CHAR index)\n", first_h - 2.0, node_data);
        assert(node_data == 0);
        
        retex_paragraph_free(para);
    }

    be->destroy(be);
    conststr_free();
    m_destruct();
    printf("test_retex_hit passed\n");
    return 0;
}
