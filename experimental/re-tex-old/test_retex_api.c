#include "src/retex.h"
#include "src/backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    m_init();
    conststr_init();
    
    Backend *be = backend_cairo_create("test_retex_api.png", 600, 500);
    if (!be) return 1;

    be->draw_rect(be, 0, 0, 600, 500, 0xFFFFFFFF); // White bg

    const char *text = "This is a test of the Retex API. It supports line breaking, "
                       "\\bf{bold text}, \\it{italic text}, and even \\bf{\\it{bold italic}}. "
                       "We can also test \\reverse{reverse mode} and \\normal{normal mode} again. "
                       "Selection should also work: \\bf{Selected Text}. "
                       "And math mode: $E = mc^2$ or $\\frac{a+b}{c}$.";

    RetexParagraph *para = retex_layout(be, text, 300.0, "Sans", 12.0, RETEX_ALIGN_JUSTIFY);
    if (para) {
        printf("Paragraph height: %f, width: %f\n", retex_paragraph_get_height(para), retex_paragraph_get_width(para));
        
        RendererColors colors1 = { .text = 0xFF000000, .bg = 0xFFFFFFFF, .reverse_text = 0xFFFFFFFF, .reverse_bg = 0xFF000000, .sel_text = 0xFF000000, .sel_bg = 0xFFAAAAFF };

        // Scenario 1: Normal rendering
        retex_paragraph_render(para, be, 20.0, 50.0, &colors1, 0);
        
        // Scenario 2: Custom colors for reverse mode
        RendererColors colors2 = colors1;
        colors2.reverse_bg = 0xFF000088;
        retex_paragraph_render(para, be, 20.0, 150.0, &colors2, 1);
        
        // Scenario 3: Custom selection colors
        RendererColors colors3 = colors1;
        colors3.sel_bg = 0xFFFF0000;
        colors3.sel_text = 0xFFFFFFFF;
        retex_paragraph_set_selection(para, 10, 30);
        retex_paragraph_render(para, be, 20.0, 250.0, &colors3, 0);
        
        // Scenario 4: Reverse mode + Selection with custom colors
        RendererColors colors4 = colors1;
        colors4.reverse_bg = 0xFF008800;
        retex_paragraph_render(para, be, 20.0, 350.0, &colors4, 1);

        // Scenario 5: Hit test
        double hx = 20.0 + 10.0; // "This"
        double hy = 50.0 + 5.0;
        int offset = retex_paragraph_get_node_at(para, 10.0, 5.0);
        printf("Hit test at (10, 5): offset %d (text: %s)\n", offset, text + offset);

        // Scenario 6: Custom background color for normal text
        RendererColors colors6 = colors1;
        colors6.bg = 0xFFFFFFCC; // Very light yellow
        retex_paragraph_render(para, be, 20.0, 430.0, &colors6, 0);

        retex_paragraph_free(para);
    }

    be->destroy(be);
    conststr_free();
    m_destruct();
    
    printf("Rendered to test_retex_api.png\n");
    return 0;
}
