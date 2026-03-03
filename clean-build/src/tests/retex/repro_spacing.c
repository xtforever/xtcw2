#include "../src/retex.h"
#include "../src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>

void repro_spacing() {
    m_init();
    conststr_init();
    trace_level = 1;

    Backend *be = backend_cairo_create("repro_spacing.png", 600, 400);
    assert(be != NULL);
    
    // Line 1 has a LARGE word.
    // Line 2 is normal.
    // Line 3 is normal.
    const char *text = "Line one with \\huge HUGE \\normalsize word.\n\nLine two is normal.\n\nLine three is also normal.";
    
    printf("Rendering mixed font text...\n");
    RetexParagraph *p = retex_layout(be, text, 500.0, "Sans", 12.0, RETEX_ALIGN_LEFT);
    assert(p != NULL);
    
    retex_paragraph_render(p, be, 50, 50);
    
    retex_paragraph_free(p);
    be->destroy(be);
    conststr_free();
    m_destruct();
}

int main() {
    repro_spacing();
    printf("repro_spacing completed, check repro_spacing.png\n");
    return 0;
}
