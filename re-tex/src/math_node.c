#include "math_node.h"
#include "mls.h"
#include "m_tool.h"
#include <stdlib.h>

void noad_create(int mlist, NoadType type) {
    Noad n = {0};
    n.type = type;
    n.nucleus.type = MATH_TYPE_EMPTY;
    n.subscr.type = MATH_TYPE_EMPTY;
    n.supscr.type = MATH_TYPE_EMPTY;
    n.numerator.type = MATH_TYPE_EMPTY;
    n.denominator.type = MATH_TYPE_EMPTY;
    
    m_put(mlist, &n);
}

static void math_field_free(MathField *f) {
    if (f->type == MATH_TYPE_MLIST) {
        retex_mlist_free(f->data.mlist);
    }
}

void retex_mlist_free(int mlist_handle) {
    if (mlist_handle <= 0) return;
    int p; Noad *n;
    m_foreach(mlist_handle, p, n) {
        math_field_free(&n->nucleus);
        math_field_free(&n->subscr);
        math_field_free(&n->supscr);
        math_field_free(&n->numerator);
        math_field_free(&n->denominator);
    }
    m_free(mlist_handle);
}
