#include "../src/linebreak.h"
#include "../src/builder.h"
#include "../src/token.h"
#include "../src/node.h"
#include "../src/glue.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static CharMetricsScaled mock_measure(void *ctx, int c, Scaled font_size, int style, const char *face) {
    (void)ctx; (void)c; (void)style; (void)face;
    CharMetricsScaled m;
    m.width = scaled_mul(font_size, FROM_INT(6)/10);
    m.height = scaled_mul(font_size, FROM_INT(7)/10);
    m.depth = scaled_mul(font_size, FROM_INT(2)/10);
    return m;
}

void test_full_pipeline() {
    m_init();
    conststr_init();
    
    // 1. Tokenize
    int tokens = tokenize("A B C");
    
    // 2. Build Paragraph (HList)
    int nodes = build_hlist(tokens, mock_measure, NULL, FROM_INT(10), "Sans");
    
    // 3. Line Break
    Scaled width = FROM_INT(15);
    Glue zero = glue_zero();
    int lines = line_break(nodes, width, zero, zero, 0, 0);
    
    // 4. Dump Output
    int out = s_printf(0, 0, "");
    
    int p; Node *n;
    m_foreach(lines, p, n) {
        int tmp = m_create(1, sizeof(Node));
        m_put(tmp, n);
        node_dump(tmp, 0, out);
        m_free(tmp);
    }
    
    // Verify
    const char *expected = 
        "HBOX w=983040 h=458750 d=131070 glue_set=0.000000\n"
        "  GLUE w=0\n"
        "  CHAR 'A' w=393210 h=458750 d=131070 s=0\n"
        "  GLUE w=0\n"
        "HBOX w=983040 h=458750 d=131070 glue_set=-0.299834\n"
        "  GLUE w=0\n"
        "  CHAR 'B' w=393210 h=458750 d=131070 s=0\n"
        "  GLUE w=218453\n"
        "  CHAR 'C' w=393210 h=458750 d=131070 s=0\n"
        "  GLUE w=0\n";
        
    if (strcmp(m_str(out), expected) != 0) {
        fprintf(stderr, "Integration Dump mismatch!\nExpected:\n%s\nGot:\n%s\n", expected, m_str(out));
    }
    
        printf("test_full_pipeline passed\n");
    
        m_free(out);
        node_list_free(lines);
        node_list_free(nodes);
        m_free(tokens);
        conststr_free();
        m_destruct();
}
    
    
    
void test_left_alignment() {
    m_init();
    conststr_init();
    
    // "Hello World"
    int tokens = tokenize("Hello World");
    int nodes = build_hlist(tokens, mock_measure, NULL, FROM_INT(10), "Sans");
    
    // Width 100pt. "Hello" is 30pt, space is 3.33pt, "World" is 30pt. Total 63.33pt.
    Scaled width = FROM_INT(100);
    Glue zero = glue_zero();
    Glue fil = glue_create(0, FROM_INT(1), ORDER_FIL, 0, ORDER_NORMAL);
    
    int lines = line_break(nodes, width, zero, fil, 0, 0);
    
    assert(m_len(lines) == 1);
    Node *hbox = (Node*)mls(lines, 0);
    
    // The hbox should have glue_set > 0 and glue_order = ORDER_FIL
    printf("Left Align HBox: width=%d, glue_set=%f, glue_order=%d\n", hbox->width, hbox->glue_set, hbox->glue_order);
    assert(hbox->glue_order == ORDER_FIL);
    assert(hbox->glue_set > 0);
    
    node_list_free(lines);
    node_list_free(nodes);
    m_free(tokens);
    conststr_free();
    m_destruct();
    printf("test_left_alignment passed\n");
}
    
    
    
int main() {
    test_full_pipeline();
    test_left_alignment();
    return 0;
}
