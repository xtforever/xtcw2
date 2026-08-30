#ifndef MATH_NODE_H
#define MATH_NODE_H

#include "node.h"
#include "builder.h"

typedef enum {
    STYLE_DISPLAY,
    STYLE_TEXT,
    STYLE_SCRIPT,
    STYLE_SCRIPT_SCRIPT
} MathStyle;

typedef enum {
    MATH_TYPE_EMPTY,
    MATH_TYPE_CHAR,
    MATH_TYPE_MLIST
} MathFieldType;

typedef struct {
    MathFieldType type;
    union {
        struct {
            int char_code;
            int font_family;
        } c;
        int mlist; // handle to mlist
    } data;
} MathField;

typedef enum {
    NOAD_ORD,
    NOAD_OP,
    NOAD_BIN,
    NOAD_REL,
    NOAD_OPEN,
    NOAD_CLOSE,
    NOAD_PUNCT,
    NOAD_INNER,
    NOAD_FRACTION,
    NOAD_LEFT,
    NOAD_RIGHT
} NoadType;

typedef struct {
    NoadType type;
    MathField nucleus;
    MathField subscr;
    MathField supscr;
    MathField numerator;   // for fraction
    MathField denominator; // for fraction
} Noad;

void noad_create(int list_handle, NoadType type);
void retex_mlist_free(int mlist_handle);

/**
 * @brief Converts a math mlist to a horizontal list of nodes.
 * 
 * @param mlist_handle Handle to the mlist.
 * @param style Current math style.
 * @param measure_func Function to measure character dimensions.
 * @param measure_ctx Context passed to measure_func.
 * @param font_size_base Base font size.
 * @param font_face_base Base font face name.
 * @param face_handle_base Base font face handle.
 * @return Handle to the hlist.
 */
int mlist_to_hlist(int mlist_handle, MathStyle style, MeasureFunc measure_func, void *measure_ctx, Scaled font_size_base, const char *font_face_base, int face_handle_base, int reverse_mode);

#endif // MATH_NODE_H
