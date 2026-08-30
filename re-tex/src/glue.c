#include "glue.h"
#include <stdio.h>

Glue glue_zero() {
    return (Glue){0, 0, 0, ORDER_NORMAL, ORDER_NORMAL};
}

Glue glue_create(Scaled w, Scaled stretch, GlueOrder stretch_o, Scaled shrink, GlueOrder shrink_o) {
    return (Glue){w, stretch, shrink, stretch_o, shrink_o};
}

static const char* order_to_str(GlueOrder o) {
    switch (o) {
        case ORDER_FIL: return "fil";
        case ORDER_FILL: return "fill";
        case ORDER_FILLL: return "filll";
        default: return "";
    }
}

void glue_print(Glue g) {
    scaled_print(g.width);
    if (g.stretch != 0) {
        printf(" plus ");
        scaled_print(g.stretch);
        printf("%s", order_to_str(g.stretch_order));
    }
    if (g.shrink != 0) {
        printf(" minus ");
        scaled_print(g.shrink);
        printf("%s", order_to_str(g.shrink_order));
    }
}
