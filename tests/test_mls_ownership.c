#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "mls.h"
#include "conststr.h"

void test_ownership() {
    const char *raw = "Dynamic String";
    
    // Constant string handle
    int c1 = s_cstr(raw);
    int c2 = s_cstr(raw);
    assert(c1 == c2); // Interned
    
    // Dynamic string (must be freed)
    int d1 = s_printf(0, 0, "%s", raw);
    int d2 = s_printf(0, 0, "%s", raw);
    assert(d1 != d2); // Different handles
    assert(d1 != c1); // Dynamic != Constant handle
    
    assert(strcmp(CHARP(d1), raw) == 0);
    
    m_free(d1);
    m_free(d2);
    // c1/c2 are freed by conststr_free()
    
    printf("test_ownership passed\n");
}

int main() {
    m_init();
    conststr_init();
    test_ownership();
    conststr_free();
    m_destruct();
    printf("All ownership tests passed\n");
    return 0;
}
