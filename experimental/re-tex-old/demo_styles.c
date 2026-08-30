#include "src/builder.h"
#include "src/token.h"
#include "src/backend_cairo.h"
#include "src/renderer.h"
#include "src/retex.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void render_text(Backend *be, const char *text, double x, double *y, double width, const char *face, double size, uint32_t color) {
    RetexParagraph *para = retex_layout(be, text, width, face, size, RETEX_ALIGN_LEFT);
    if (para) {
        RendererColors colors = {
            .text = color,
            .bg = 0x00000000, // Transparent
            .reverse_text = 0xFFFFFFFF, // White
            .reverse_bg = color,
            .sel_text = 0xFF000000,
            .sel_bg = 0xFFAAAAFF
        };
        retex_paragraph_render(para, be, x, *y, &colors, 0);
        double h = retex_paragraph_get_height(para);
        *y += h + 10;
        retex_paragraph_free(para);
    }
}

int main() {
    m_init();
    conststr_init();
    extern int trace_level;
    trace_level = 0;
    
    const char *filename = "demo_styles.png";
    int width = 600;
    int height = 500;
    
    Backend *be = backend_cairo_create(filename, width, height);
    if (!be) {
        fprintf(stderr, "Failed to create backend\n");
        return 1;
    }
    
    be->draw_rect(be, 0, 0, width, height, 0xFFFFFFFF);
    
    double y = 30;
    double x = 20;
    double text_width = width - 40;
    const char *face = "Sans";
    double size = 14;
    uint32_t color = 0xFF000000;
    
    render_text(be, "Text Styles Demo", x, &y, text_width, "Sans", 20, 0xFF1A1A1A);
    y += 10;
    
    render_text(be, "--- Font Styles ---", x, &y, text_width, face, size, 0xFF666666);
    render_text(be, "Normal text", x, &y, text_width, face, size, color);
    render_text(be, "\\bf{Bold text}", x, &y, text_width, face, size, color);
    render_text(be, "\\it{Italic text}", x, &y, text_width, face, size, color);
    render_text(be, "\\bf\\it{Bold Italic text}", x, &y, text_width, face, size, color);
    render_text(be, "\\rm{Roman (normal)}", x, &y, text_width, face, size, color);
    
    y += 15;
    render_text(be, "--- Reverse Mode ---", x, &y, text_width, face, size, 0xFF666666);
    render_text(be, "\\reverse{This text has reverse mode!}", x, &y, text_width, face, size, color);
    render_text(be, "Normal \\reverse{reversed} back to normal", x, &y, text_width, face, size, color);
    render_text(be, "\\reverse{Bold \\bf reversed}", x, &y, text_width, face, size, color);
    render_text(be, "\\normal{Normal mode restored}", x, &y, text_width, face, size, color);
    
    y += 15;
    render_text(be, "--- Font Sizes ---", x, &y, text_width, face, size, 0xFF666666);
    render_text(be, "\\tiny{Tiny text}", x, &y, text_width, face, size, color);
    render_text(be, "\\small{Small text}", x, &y, text_width, face, size, color);
    render_text(be, "Normal text", x, &y, text_width, face, size, color);
    render_text(be, "\\large{Large text}", x, &y, text_width, face, size, color);
    render_text(be, "\\Large{Larger text}", x, &y, text_width, face, size, color);
    render_text(be, "\\LARGE{ Largest text}", x, &y, text_width, face, size, color);
    render_text(be, "\\huge{Huge text}", x, &y, text_width, face, size, color);
    
    y += 15;
    render_text(be, "--- Combined Styles ---", x, &y, text_width, face, size, 0xFF666666);
    render_text(be, "\\reverse{\\bf Bold and reversed}", x, &y, text_width, face, size, color);
    render_text(be, "\\large\\reverse{Large and reversed}", x, &y, text_width, face, size, color);
    render_text(be, "\\it\\reverse{Italic and reversed}", x, &y, text_width, face, size, color);
    render_text(be, "\\bf\\it\\reverse{Bold Italic Reversed}", x, &y, text_width, face, size, color);
    
    be->destroy(be);
    conststr_free();
    m_destruct();
    
    printf("Created %s\n", filename);
    return 0;
}
