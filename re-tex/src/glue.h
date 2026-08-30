#ifndef GLUE_H
#define GLUE_H

#include "scaled.h"

typedef enum {
    ORDER_NORMAL = 0,
    ORDER_FIL    = 1,
    ORDER_FILL   = 2,
    ORDER_FILLL  = 3
} GlueOrder;

typedef struct {
    Scaled width;
    Scaled stretch;
    Scaled shrink;
    GlueOrder stretch_order;
    GlueOrder shrink_order;
} Glue;

// Basic operations
Glue glue_zero();
Glue glue_create(Scaled w, Scaled stretch, GlueOrder stretch_o, Scaled shrink, GlueOrder shrink_o);

// Formatting
void glue_print(Glue g);

#endif // GLUE_H
