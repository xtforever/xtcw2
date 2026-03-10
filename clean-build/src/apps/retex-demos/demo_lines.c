#include <stdio.h>
#include <string.h>
#include "backend_cairo.h"
#include "renderer.h"
#include "node.h"
#include "builder.h"
#include "linebreak.h"
#include "token.h"
#include "mls.h"
#include "m_tool.h"

/* Workaround for redefinition error in conststr.h vs m_tool.h */
#define s_cstr s_cstr_hidden
#define s_mstr s_mstr_hidden
#include "conststr.h"
#undef s_cstr
#undef s_mstr

// Callback for builder to measure chars using backend
static CharMetricsScaled measure_char(void *ctx, int c, Scaled font_size, int style, const char *face) {
    Backend *be = (Backend*)ctx;
    be->set_font_face(be, face, style);
    be->set_font_size(be, TO_DOUBLE(font_size));
    CharMetrics m;
    be->get_char_metrics(be, c, &m);
    
    CharMetricsScaled ms;
    ms.width = FROM_DOUBLE(m.width);
    ms.height = FROM_DOUBLE(m.height);
    ms.depth = FROM_DOUBLE(m.depth);
    return ms;
}

int main() {
    m_init();
    conststr_init();

    const char *text = 
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
        "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
        "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in "
        "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla "
        "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
        "culpa qui officia deserunt mollit anim id est laborum.";
    
    printf("Tokenizing text...\n");
    int tokens = tokenize(text);
    
    // Setup Backend early for measuring
    // Using 1200x800 for enough space
    Backend *be = backend_cairo_create("demo_lines.png", 600, 400);
    if (!be) return 1;
    
    // Use a "beautiful" font
    be->set_font_face(be, "Serif", 0);
    be->set_font_size(be, 12.0); // 12pt font

    printf("Building node list...\n");
    int face_handle = conststr_lookup_c("Serif");
    int hlist = build_hlist(tokens, measure_char, be, FROM_INT(12), "Serif", face_handle);
    
    // Append \parfillskip (infinite stretch) to the end of the list
    // This ensures the last line is not fully justified if it's short.
    Glue parfill = glue_create(0, FROM_INT(1000), ORDER_FIL, 0, ORDER_NORMAL);
    node_create_glue(hlist, parfill);
    
    printf("Breaking lines...\n");
    // Target width: 500pt (leaving margins)
    Glue zero = glue_zero();
    int vbox_lines = line_break(hlist, FROM_INT(500), zero, zero, 0, 0);
    
    // Wrap the lines in a VBOX
    int vbox_handle = m_create(1, sizeof(Node));
    node_create_vbox(vbox_handle, vbox_lines);
    Node *vbox = (Node*)mls(vbox_handle, 0);
    vbox->width = FROM_INT(500);
    // Height calculation skipped for now (should sum children)
    
    // Render
    printf("Rendering...\n");
    
    // Background
    be->draw_rect(be, 0, 0, 600, 400, 0xFFFFFFFF);
    
    // Render the VBox at (50, 50)
    // Note: renderer VBOX logic is top-down
    renderer_render(be, vbox_handle, 50.0, 50.0);
    
    be->destroy(be);
    
    // Cleanup
    conststr_free();
    m_destruct();
    
    printf("Generated demo_lines.png\n");
    return 0;
}
