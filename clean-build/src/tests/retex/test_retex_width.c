#include "../src/retex.h"
#include "../src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>

void test_natural_width() {
    m_init();
    conststr_init();
    
    Backend *be = backend_cairo_create("dummy_width.png", 100, 100);
    assert(be != NULL);
    
    const char *text = "Hello World";
    
    RetexParagraph *para = retex_layout(be, text, 10000.0, "Sans", 12.0, RETEX_ALIGN_JUSTIFY);
    assert(para != NULL);
    
    double width = retex_paragraph_get_width(para);
    double height = retex_paragraph_get_height(para);
    
    printf("Text: '%s'\n", text);
    printf("Natural Width: %f pt\n", width);
    printf("Height: %f pt\n", height);
    
    // width should now be much less than 10000
    assert(width > 40.0);
    assert(width < 200.0);
    
    assert(height > 8.0);
    assert(height < 20.0);
    
    retex_paragraph_free(para);
    be->destroy(be);
    conststr_free();
    m_destruct();
}

int main() {
    test_natural_width();
    printf("test_retex_width passed\n");
    return 0;
}
