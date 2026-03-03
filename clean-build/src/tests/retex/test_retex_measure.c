#include "../src/retex.h"
#include "../src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>

void test_measure() {
    m_init();
    conststr_init();
    
    // Create a backend for metric access (dummy surface)
    Backend *be = backend_cairo_create("dummy.png", 100, 100);
    assert(be != NULL);
    
    const char *text = 
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
        
    // 500pt width, 12pt font
    double height = retex_measure_height(be, text, 500.0, "Serif", 12.0);
    
    printf("Measured Height: %f pt\n", height);
    
    // Check if reasonable
    // Text is about 2 lines long at 500pt width (based on demo_lines experience)
    // 12pt font -> line advance ~14.4pt
    // 2 lines -> ~28.8pt
    assert(height > 20.0);
    assert(height < 100.0);
    
    be->destroy(be);
    conststr_free();
    m_destruct();
}

int main() {
    test_measure();
    return 0;
}
