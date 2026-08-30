#include <stdio.h>
#include "mls.h"
#include "retex.h"
#include "node.h"

int main() {
    m_init();
    extern int trace_level;
    trace_level = 2;

    int child_list = m_create(1, sizeof(Node));
    node_create_char(child_list, 'A', FROM_INT(5), FROM_INT(7), 0, 10.0, 0, 0, 0);
    
    int box_list = m_create(1, sizeof(Node));
    node_create_hbox(box_list, child_list);

    node_list_free(box_list);
    m_destruct();
    return 0;
}
