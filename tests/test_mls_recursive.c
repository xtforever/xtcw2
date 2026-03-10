#include <stdio.h>
#include <assert.h>
#include "mls.h"

void test_recursive_free() {
    // h_parent will use free_hdl=2 (MFREE_EACH)
    int h_parent = m_alloc(10, sizeof(int), MFREE_EACH);
    int h_child1 = m_alloc(10, sizeof(int), 0);
    int h_child2 = m_alloc(10, sizeof(int), 0);
    
    m_put(h_parent, &h_child1);
    m_put(h_parent, &h_child2);
    
    // Freeing parent should free children
    m_free(h_parent);
    
    // Verify children are freed
    assert(m_is_freed(h_child1));
    assert(m_is_freed(h_child2));
    assert(m_is_freed(h_parent));
    
    printf("test_recursive_free passed\n");
}

int main() {
    m_init();
    test_recursive_free();
    m_destruct();
    printf("All recursive free tests passed\n");
    return 0;
}
