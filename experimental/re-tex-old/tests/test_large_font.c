#include "../src/retex.h"
#include "../src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <stdlib.h>

void test_large_font_render(const char *filename, const char *text, double width_pt, double font_size_pt) {
    printf("Testing large font (%f pt) for: %s -> %s\n", font_size_pt, text, filename);
    
    // Create a backend with enough space
    // 28pt font is roughly 37px high. We need some padding.
    Backend *be = backend_cairo_create(filename, (int)width_pt + 40, 200);
    if (!be) {
        fprintf(stderr, "Failed to create backend\n");
        return;
    }

    // Draw background
    be->draw_rect(be, 0, 0, width_pt + 40, 200, 0xFFFFFFFF);

    RetexParagraph *para = retex_layout(be, text, width_pt, "Serif", font_size_pt, RETEX_ALIGN_LEFT);
    if (para) {
        double height = retex_paragraph_get_height(para);
        double first_h = retex_paragraph_get_first_line_height(para);
        
        printf("  Layout Height: %f pt\n", height);
        printf("  First Line Height: %f pt\n", first_h);
        
        RendererColors colors = {
            .text = 0xFF000000,    // Black
            .bg = 0xFFFFFFFF,      // White
            .sel_text = 0xFFFFFFFF, // White
            .sel_bg = 0xFF0000FF   // Blue
        };

        // Render at x=20, y=first_h + 20 (to avoid clipping top)
        retex_paragraph_render(para, be, 20.0, first_h + 20.0, &colors, 0);
        retex_paragraph_free(para);
    } else {
        fprintf(stderr, "  Failed to layout text\n");
    }

    be->destroy(be);
}

int main() {
    m_init();
    conststr_init();
    
    // Test 1: Simple text
    test_large_font_render("test_large_simple.png", "Hello Big World!", 400.0, 28.0);
    
    // Test 2: Math
    test_large_font_render("test_large_math.png", "Math: $E=mc^2$ and $\\frac{a}{b}$", 400.0, 28.0);
    
    // Test 3: Mixed styles
    test_large_font_render("test_large_styles.png", "\\bf{Bold} and \\it{Italic} at 28pt", 400.0, 28.0);
    
    // Test 4: Long paragraph to check line breaking at large size
    const char *long_text = "This is a longer paragraph rendered at twenty-eight points to see how the Knuth-Plass algorithm handles large glyphs and if any overflow or overlap occurs.";
    test_large_font_render("test_large_paragraph.png", long_text, 400.0, 28.0);

    conststr_free();
    m_destruct();
    
    printf("Large font tests completed.\n");
    return 0;
}
