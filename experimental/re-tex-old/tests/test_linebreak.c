#include "../src/linebreak.h"
#include "../src/builder.h"
#include "../src/token.h"
#include "../src/glue.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>

static CharMetricsScaled mock_measure(void *ctx, int c, Scaled font_size, int style, const char *face) {
    (void)ctx; (void)c; (void)style; (void)face;
    CharMetricsScaled m;
    m.width = scaled_mul(font_size, FROM_INT(6)/10);
    m.height = scaled_mul(font_size, FROM_INT(7)/10);
    m.depth = scaled_mul(font_size, FROM_INT(2)/10);
    return m;
}

void test_linebreak_simple() {
    m_init();
    conststr_init();
    
    // "Hello World Hello World Hello World"
    int tokens = tokenize("Hello World Hello World Hello World");
    int nodes = build_hlist(tokens, mock_measure, NULL, FROM_INT(10), "Sans", 0);
    
    // Target width: approx width of "Hello World"
    // "Hello" ~ 5*6 = 30pt. Space ~ 3pt. "World" ~ 30pt. Total ~ 63pt.
    // Let's set width to 40pt to force breaks.
    Scaled width = FROM_INT(40);
    Glue zero = glue_zero();
    
    int lines = line_break(nodes, width, zero, zero, 0, 0);
    
    // Should have multiple lines
    printf("Lines created: %d\n", m_len(lines));
    assert(m_len(lines) > 1);
    
    // Verify lines are HBoxes
    int p; Node *n;
    m_foreach(lines, p, n) {
        assert(n->type == NODE_HBOX);
        printf("Line width: %d sp\n", n->width);
    }
    
    printf("test_linebreak_simple passed\n");
    
    // Cleanup
    node_list_free(lines);
    node_list_free(nodes);
    m_free(tokens);
    conststr_free();
    m_destruct();
}

int main() {
    test_linebreak_simple();
    return 0;
}
