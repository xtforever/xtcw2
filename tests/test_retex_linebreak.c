#include <stdio.h>
#include "mls.h"
#include "retex.h"
#include "node.h"
#include "linebreak.h"
#include "glue.h"

int main() {
    m_init();
    extern int trace_level;
    trace_level = 2;

    int p_list = m_create(10, sizeof(Node));
    
    // Word 1 (50pt)
    node_create_char(p_list, '1', FROM_INT(50), FROM_INT(10), 0, 10.0, 0, 0, 0);
    // Glue (10pt)
    Glue g = glue_create(FROM_INT(10), 0, 0, 0, 0);
    node_create_glue(p_list, g, 0);
    // Word 2 (50pt)
    node_create_char(p_list, '2', FROM_INT(50), FROM_INT(10), 0, 10.0, 0, 0, 0);
    node_create_glue(p_list, g, 0);
    // Word 3 (50pt)
    node_create_char(p_list, '3', FROM_INT(50), FROM_INT(10), 0, 10.0, 0, 0, 0);

    // Target 80pt.
    // Line 1: Word 1 (50pt) + Glue(10pt) = 60pt. Word 2 (50pt) does NOT fit.
    // Line 2: Word 2 (50pt) + Glue(10pt) = 60pt. Word 3 (50pt) does NOT fit.
    // Line 3: Word 3 (50pt)
    
    Glue left = glue_create(0, 0, 0, 0, 0);
    Glue right = glue_create(0, 0, 0, 0, 0);
    
    int lines = line_break(p_list, FROM_INT(80), left, right, 0, 0);
    printf("Line count: %d\n", m_len(lines));

    m_destruct();
    return 0;
}
