#include "builder.h"
#include "token.h"
#include "backend_cairo.h"
#include "renderer.h"
#include "mls.h"
#include "m_tool.h"

/* Workaround for redefinition error in conststr.h vs m_tool.h */
#define s_cstr s_cstr_hidden
#define s_mstr s_mstr_hidden
#include "conststr.h"
#undef s_cstr
#undef s_mstr

#include <stdio.h>
#include <stdlib.h>

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
    
    printf("Building node list...\n");
    
    int tokens = tokenize("Hello World via Cairo");
    
    // Create Backend early for measuring
    Backend *be = backend_cairo_create("output.png", 300, 100);
    if (!be) {
        fprintf(stderr, "Failed to create backend\n");
        return 1;
    }
    
    be->set_font_size(be, 10.0);
    be->set_font_face(be, "Monospace", 0);
    
    int face_handle = conststr_lookup_c("Monospace");
    int node_list = build_hlist(tokens, measure_char, be, FROM_INT(10), "Monospace", face_handle);

    // Create a wrapper HBOX
    int box_handle_list = m_create(1, sizeof(Node));
    node_create_hbox(box_handle_list, node_list);
    Node *box = (Node*)mls(box_handle_list, 0);
    box->width = FROM_INT(200);
    box->height = FROM_INT(12); // approx

    // Render
    // Draw background/reference
    be->draw_rect(be, 0, 0, 300, 100, 0xFFFFFFFF); // White bg
    be->draw_rect(be, 10, 50, 280, 1, 0xFFCCCCCC); // Baseline
    
    renderer_render(be, box_handle_list, 20.0, 50.0);

    be->destroy(be);
    
    // Cleanup
    // (Skipping full tree free for demo brevity, rely on m_destruct)
    conststr_free();
    m_destruct();
    
    printf("Rendered to output.png\n");
    return 0;
}
