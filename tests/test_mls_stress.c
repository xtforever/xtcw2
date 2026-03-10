#include <stdio.h>
#include <assert.h>
#include "mls.h"
#include "m_table.h"

void test_stress_nesting() {
    for (int i = 0; i < 100; i++) {
        int root = m_table_create();
        
        // Add some strings
        m_table_set_string_by_cstr(root, "name", "Stress Test");
        m_table_set_const_string_by_cstr(root, "type", "Internal");
        
        // Add a list of lists
        int list_of_lists = m_alloc(10, sizeof(int), MFREE_EACH);
        for (int j = 0; j < 10; j++) {
            int child_list = m_alloc(5, sizeof(int), 0);
            for (int k = 0; k < 5; k++) {
                int val = i + j + k;
                m_put(child_list, &val);
            }
            m_put(list_of_lists, &child_list);
        }
        m_table_set_list_by_cstr(root, "data", list_of_lists);
        
        // Freeing root should free everything except constant strings
        m_table_free(root);
    }
    printf("test_stress_nesting passed\n");
}

int main() {
    m_init();
    conststr_init();
    test_stress_nesting();
    conststr_free();
    m_destruct();
    printf("All stress tests passed\n");
    return 0;
}
