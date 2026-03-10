#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "mls.h"
#include "conststr.h"

void test_interning() {
    int h1 = s_cstr("Hello");
    int h2 = s_cstr("Hello");
    int h3 = s_cstr("World");
    
    assert(h1 == h2);
    assert(h1 != h3);
    assert(strcmp(CHARP(h1), "Hello") == 0);
    printf("test_interning passed\n");
}

void test_empty() {
    int h1 = s_cstr("");
    int h2 = s_cstr("");
    assert(h1 == h2);
    assert(strcmp(CHARP(h1), "") == 0);
    printf("test_empty passed\n");
}

int main() {
    m_init();
    conststr_init();
    test_interning();
    test_empty();
    conststr_free();
    m_destruct();
    printf("All s_cstr tests passed\n");
    return 0;
}
