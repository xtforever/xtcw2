#include "../src/glue.h"
#include "mls.h"
#include <stdio.h>
#include <assert.h>

void test_glue_creation() {
    Glue g = glue_create(FROM_INT(10), FROM_INT(5), ORDER_FIL, 0, ORDER_NORMAL);
    assert(g.width == FROM_INT(10));
    assert(g.stretch == FROM_INT(5));
    assert(g.stretch_order == ORDER_FIL);
    assert(g.shrink == 0);
    printf("test_glue_creation passed\n");
}

void test_glue_list_memc() {
    m_init(); // Initialize memc library
    
    int glue_list = m_create(5, sizeof(Glue));
    
    Glue g1 = glue_create(FROM_INT(10), 0, 0, 0, 0);
    Glue g2 = glue_create(0, FROM_INT(1), ORDER_FILL, 0, 0);
    
    m_put(glue_list, &g1);
    m_put(glue_list, &g2);
    
    assert(m_len(glue_list) == 2);
    
    Glue *p1 = (Glue*)mls(glue_list, 0);
    Glue *p2 = (Glue*)mls(glue_list, 1);
    
    assert(p1->width == FROM_INT(10));
    assert(p2->stretch_order == ORDER_FILL);
    
    m_free(glue_list);
    m_destruct(); // Finalize memc library
    printf("test_glue_list_memc passed\n");
}

int main() {
    test_glue_creation();
    test_glue_list_memc();
    printf("All Glue tests passed!\n");
    return 0;
}