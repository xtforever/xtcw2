#include "../src/scaled.h"
#include <stdio.h>
#include <assert.h>

void test_basic_conversion() {
    Scaled s = FROM_INT(10);
    assert(s == 655360);
    assert(TO_INT(s) == 10);
    printf("test_basic_conversion passed\n");
}

void test_arithmetic() {
    Scaled a = FROM_INT(1);
    Scaled b = FROM_INT(2);
    
    assert(scaled_add(a, b) == FROM_INT(3));
    assert(scaled_sub(b, a) == FROM_INT(1));
    
    // 0.5pt * 2 = 1.0pt
    Scaled half = UNITY / 2;
    assert(scaled_mul(half, FROM_INT(2)) == FROM_INT(1));
    
    // 1pt / 2 = 0.5pt
    assert(scaled_div(FROM_INT(1), FROM_INT(2)) == half);
    
    printf("test_arithmetic passed\n");
}

int main() {
    test_basic_conversion();
    test_arithmetic();
    printf("All Scaled tests passed!\n");
    return 0;
}