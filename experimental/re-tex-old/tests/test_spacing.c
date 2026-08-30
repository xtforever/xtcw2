#include "retex.h"
#include "backend_cairo.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    m_init();
    conststr_init();
    trace_level = 1;
    
    const char *text = "Start \\hspace{20pt} End\\n\\nLine 1 \\vspace{40pt}\\n\\nLine 2\\n\\n\\vspace{-20pt}\\rule{100pt}{2pt}";
    const char *filename = "spacing.png";
    int width = 400;
    int height = 400;
    
    printf("Rendering spacing test: [%s]\n", text);
    
    Backend *be = backend_cairo_create(filename, width, height);
    if (!be) return 1;
    
    be->draw_rect(be, 0, 0, (double)width, (double)height, 0xFFFFFFFF);
    
    RetexParagraph *para = retex_layout(be, text, 380.0, "Serif", 24, 0);
    if (para) {
        retex_paragraph_render(para, be, 10, 50, 0xFF000000, 0);
        retex_paragraph_free(para);
    }
    
    be->destroy(be);
    conststr_free();
    m_destruct();
    printf("Done. Check %s\n", filename);
    return 0;
}
