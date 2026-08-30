#include "src/builder.h"
#include "src/token.h"
#include "src/backend_cairo.h"
#include "src/renderer.h"
#include "mls.h"
#include "m_tool.h"
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
    double pt_per_px = 72.0 / 96.0;
    ms.width = FROM_DOUBLE(m.width * pt_per_px);
    ms.height = FROM_DOUBLE(m.height * pt_per_px);
    ms.depth = FROM_DOUBLE(m.depth * pt_per_px);
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
    
    int node_list = build_hlist(tokens, measure_char, be, FROM_INT(10), "Monospace", conststr_lookup_c("Monospace"));

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
    
    renderer_render(be, box_handle_list, 20.0, 50.0, -1, -1, NULL, 0);

    be->destroy(be);
    
    // Cleanup
    // (Skipping full tree free for demo brevity, rely on m_destruct)
    conststr_free();
    m_destruct();
    
    printf("Rendered to output.png\n");
    return 0;
}
