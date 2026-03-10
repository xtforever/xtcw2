#include <stdio.h>
#include "mls.h"
#include "retex.h"
#include "backend_cairo.h"

int main() {
    m_init();
    conststr_init();
    extern int trace_level;
    trace_level = 2;

    Backend *be = backend_cairo_create("math_test.png", 200, 100);
    if (!be) return 1;

    // A simple fraction
    RetexParagraph *para = retex_layout(be, "$\\frac{a}{b}$", 200.0, "serif", 12.0, RETEX_ALIGN_LEFT);
    if (para) {
        retex_paragraph_free(para);
    }

    be->destroy(be);
    conststr_free();
    m_destruct();
    return 0;
}
