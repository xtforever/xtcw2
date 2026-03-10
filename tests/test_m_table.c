#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "mls.h"
#include "m_table.h"

void test_table_basic() {
    int tbl = m_table_create();
    m_table_set_int_val_by_cstr(tbl, "age", 25);
    m_table_set_string_by_cstr(tbl, "name", "John Doe");
    
    assert(m_table_get_cstr(tbl, "age") == 25);
    int name_h = m_table_get_cstr(tbl, "name");
    assert(strcmp(CHARP(name_h), "John Doe") == 0);
    
    m_table_free(tbl);
    printf("test_table_basic passed\n");
}

void test_table_nested() {
    int root = m_table_create();
    int child = m_table_create();
    m_table_set_int_val_by_cstr(child, "score", 100);
    m_table_set_table_by_cstr(root, "stats", child);
    
    int retrieved_child = m_table_get_cstr(root, "stats");
    assert(retrieved_child == child);
    assert(m_table_get_cstr(retrieved_child, "score") == 100);
    
    // m_table_free should recursively free 'child'
    m_table_free(root);
    assert(m_is_freed(root));
    assert(m_is_freed(child));
    
    printf("test_table_nested passed\n");
}

int main() {
    m_init();
    conststr_init();
    test_table_basic();
    test_table_nested();
    conststr_free();
    m_destruct();
    printf("All m_table tests passed\n");
    return 0;
}
