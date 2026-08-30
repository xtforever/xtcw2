#ifndef SCALED_H
#define SCALED_H

#include <stdint.h>

/**
 * @brief Scaled point (sp). 1pt = 65536sp (2^16).
 * TeX uses signed 32-bit integers for dimensions to prevent overflow 
 * during common operations while maintaining precision.
 */
typedef int32_t Scaled;

#define UNITY 65536
#define MAX_SCALED INT32_MAX
#define MIN_SCALED INT32_MIN

// Conversion macros
#define FROM_INT(x) ((Scaled)((x) * UNITY))
#define TO_INT(x) ((x) / UNITY)

#define TO_DOUBLE(s) ((double)(s) / (double)UNITY)
#define FROM_DOUBLE(d) ((Scaled)((d) * (double)UNITY))

typedef struct {
    Scaled width;
    Scaled height;
    Scaled depth;
} CharMetricsScaled;

// Basic Arithmetic with rounding/overflow protection inspired by glue.web
Scaled scaled_add(Scaled a, Scaled b);
Scaled scaled_sub(Scaled a, Scaled b);
Scaled scaled_mul(Scaled a, Scaled b); // Fixed-point multiplication
Scaled scaled_div(Scaled a, Scaled b); // Fixed-point division

// Utilities
void scaled_print(Scaled s);

#endif // SCALED_H
