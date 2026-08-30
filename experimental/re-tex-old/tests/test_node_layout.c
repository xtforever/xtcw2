#include "../src/node.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void test_headless_layout_dump() {
    m_init();
    
    int child_list = m_create(2, sizeof(Node));
    node_create_char(child_list, 'A', FROM_INT(5), FROM_INT(7), 0, 10.0, 0, 0, 0);
    Glue g_spec = glue_create(FROM_INT(10), 0, 0, 0, 0);
    node_create_glue(child_list, g_spec, 0);
    
    // Create HBox containing them
    int box_list = m_create(1, sizeof(Node));
    node_create_hbox(box_list, child_list);
    Node *bn = (Node*)mls(box_list, 0);
    bn->width = FROM_INT(15); // Simple sum for this test
    
    // Generate dump
    int out = s_printf(0, 0, "");
    node_dump(box_list, 0, out);
    
    const char *expected = 
        "HBOX w=983040 h=0 d=0 glue_set=0.000000\n"
        "  CHAR 'A' w=327680 h=458752 d=0 s=0\n"
        "  GLUE w=655360\n";
    
    if (strcmp(m_str(out), expected) != 0) {
        fprintf(stderr, "Dump mismatch!\nExpected:\n%s\nGot:\n%s\n", expected, m_str(out));
        assert(0);
    }
    
    printf("test_headless_layout_dump passed\n");
    
    node_list_free(box_list);
    m_free(out);
    m_destruct();
}

int main() {
    test_headless_layout_dump();
    return 0;
}
