#include "scaled.h"
#include <stdio.h>
#include <stdlib.h>

Scaled scaled_add(Scaled a, Scaled b) {
    // Basic implementation for now; TeX usually checks for overflow here
    return a + b;
}

Scaled scaled_sub(Scaled a, Scaled b) {
    return a - b;
}

/**
 * @brief Multiply two scaled numbers.
 * result = (a * b) / 2^16
 */
Scaled scaled_mul(Scaled a, Scaled b) {
    int64_t res = (int64_t)a * (int64_t)b;
    return (Scaled)(res / UNITY);
}

/**
 * @brief Divide two scaled numbers.
 * result = (a * 2^16) / b
 */
Scaled scaled_div(Scaled a, Scaled b) {
    if (b == 0) {
        fprintf(stderr, "Fatal: Division by zero in scaled_div\n");
        exit(1);
    }
    int64_t res = ((int64_t)a * UNITY) / b;
    return (Scaled)res;
}

void scaled_print(Scaled s) {
    printf("%d.%05dpt", TO_INT(s), (int)(((abs(s) % UNITY) * 100000LL) / UNITY));
}