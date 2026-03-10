#include <stdio.h>
#include <assert.h>
#include "mls.h"

void test_alloc_free() {
    int h = m_alloc(10, sizeof(int), 0);
    assert(h > 0);
    m_free(h);
    printf("test_alloc_free passed\n");
}

void test_put_get() {
    int h = m_alloc(10, sizeof(int), 0);
    int val = 42;
    m_put(h, &val);
    assert(m_len(h) == 1);
    int *ret = (int*)mls(h, 0);
    assert(*ret == 42);
    m_free(h);
    printf("test_put_get passed\n");
}

int main() {
    m_init();
    test_alloc_free();
    test_put_get();
    m_destruct();
    printf("All foundation tests passed\n");
    return 0;
}
