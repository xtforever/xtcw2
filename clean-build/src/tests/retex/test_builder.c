#include "../src/builder.h"
#include "../src/token.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>

// Mock measure function
static CharMetricsScaled mock_measure(void *ctx, int c, Scaled font_size, int style, const char *face) {
    (void)ctx; (void)c; (void)style; (void)face;
    CharMetricsScaled m;
    m.width = scaled_mul(font_size, FROM_INT(6)/10);
    m.height = scaled_mul(font_size, FROM_INT(7)/10);
    m.depth = scaled_mul(font_size, FROM_INT(2)/10);
    return m;
}

void test_builder_simple() {
    m_init();
    conststr_init();
    
    // Tokenize
    int tokens = tokenize("Hello World");
    
    // Build with 10pt
    int nodes = build_hlist(tokens, mock_measure, NULL, FROM_INT(10), "Sans");
    
    // Check nodes
    // H, e, l, l, o, Glue, W, o, r, l, d -> 11 nodes
    assert(m_len(nodes) == 11);
    
    int p; Node *n;
    int count = 0;
    m_foreach(nodes, p, n) {
        if (count == 5) {
            assert(n->type == NODE_GLUE);
        } else {
            assert(n->type == NODE_CHAR);
        }
        count++;
    }
    
    printf("test_builder_simple passed\n");
    
    // Clean up nodes
    node_list_free(nodes);
    m_free(tokens);
    conststr_free();
    m_destruct();
}

int main() {
    test_builder_simple();
    return 0;
}
