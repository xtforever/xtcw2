#include "renderer.h"
#include <cairo.h>
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>

static void renderer_render_node(Backend *backend, Node *n, double x, double y) {
    if (!n) return;
    
    switch (n->type) {
        case NODE_CHAR: {
            const char *face = m_str(n->font_face_handle);
            if (is_empty(face)) face = "Sans";
            backend->set_font_face(backend, face, n->font_style);
            backend->set_font_size(backend, (double)n->font_size);
            if (n->vertical_scale != 1.0f) {
                // This is a bit tricky since Backend interface doesn't have scale.
                // But we know Backend.ctx is cairo_t* for our current backends.
                cairo_t *cr = (cairo_t*)backend->ctx;
                cairo_save(cr);
                cairo_scale(cr, 1.0, (double)n->vertical_scale);
                // Adjust y because scaling is from 0,0
                backend->draw_char(backend, x, y / (double)n->vertical_scale, (char)n->data);
                cairo_restore(cr);
            } else {
                backend->draw_char(backend, x, y, (char)n->data);
            }
        }
        break;
            
        case NODE_HBOX: {
            double cur_x = x;
            int p; Node *child;
            int list_handle = n->list;
            // Iterate through children
            if (list_handle > 0) {
                m_foreach(list_handle, p, child) {
                    // For HBOX, child nodes are placed horizontally.
                    // The y coordinate is the baseline, adjusted by child's shift.
                    
                    renderer_render_node(backend, child, cur_x, y - TO_DOUBLE(child->shift));
                    
                    double w = TO_DOUBLE(child->width);
                    
                    // Apply glue adjustment if it's a glue node
                    if (child->type == NODE_GLUE) {
                        double adj = 0;
                        if (n->glue_set > 0 && (int)child->glue_spec.stretch_order == n->glue_order) {
                            adj = n->glue_set * TO_DOUBLE(child->glue_spec.stretch);
                        } else if (n->glue_set < 0 && (int)child->glue_spec.shrink_order == n->glue_order) {
                            adj = n->glue_set * TO_DOUBLE(child->glue_spec.shrink);
                            // Cap shrink to 100% of specified shrink (TeX standard-ish)
                            if (n->glue_set < -1.0) {
                                adj = -1.0 * TO_DOUBLE(child->glue_spec.shrink);
                            }
                        }
                        w += adj;
                    }
                    
                    cur_x += w;
                }
            }
            break;
        }
        
        case NODE_VBOX: {
             double cur_y = y - TO_DOUBLE(n->height);
             int p; Node *child;
             int list_handle = n->list;
             if (list_handle > 0) {
                 m_foreach(list_handle, p, child) {
                     if (child->type == NODE_KERN) {
                         cur_y += TO_DOUBLE(child->height);
                     } else {
                         cur_y += TO_DOUBLE(child->height);
                         renderer_render_node(backend, child, x + TO_DOUBLE(child->shift), cur_y);
                         cur_y += TO_DOUBLE(child->depth);
                     }
                 }
             }
             break;
        }

        case NODE_RULE:
            backend->draw_rect(backend, x, y - TO_DOUBLE(n->height), TO_DOUBLE(n->width), TO_DOUBLE(n->height + n->depth), 0xFF000000);
            break;

        case NODE_SVG: {
            const char *fname = m_str(n->data);
            if (!is_empty(fname)) {
                backend->draw_svg(backend, x, y - TO_DOUBLE(n->height), TO_DOUBLE(n->width), TO_DOUBLE(n->height), fname);
            }
            break;
        }

        case NODE_KERN:
        case NODE_GLUE:
            // Invisible, but takes space (handled by parent HBOX loop)
            break;

        default:
            break;
    }
}

void renderer_render(Backend *backend, int node_handle, double x, double y) {
    if (node_handle <= 0) return;
    Node *n = (Node*)mls(node_handle, 0);
    renderer_render_node(backend, n, x, y);
}
