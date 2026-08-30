#include "renderer.h"
#include <cairo.h>
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>

static const RendererColors default_colors = {
    .text = 0xFF000000,
    .bg = 0x00000000,
    .reverse_text = 0xFFFFFFFF,
    .reverse_bg = 0xFF000000,
    .sel_text = 0xFF000000,
    .sel_bg = 0xFFAAAAFF
};

static void renderer_render_backgrounds(Backend *backend, Node *n, double x, double y, int sel_start, int sel_end, const RendererColors *colors, double width_override, int reverse_mode) {
    if (!n) return;

    double use_w = (width_override > 0) ? width_override : TO_DOUBLE(n->width);
    double total_h = TO_DOUBLE(n->height + n->depth);
    
    switch (n->type) {
        case NODE_CHAR:
        case NODE_GLUE: {
            int use_reverse = reverse_mode || n->reverse_mode;
            if (sel_start != -1 && n->source_offset >= sel_start && n->source_offset <= sel_end) {
                backend->draw_highlight(backend, x, y - TO_DOUBLE(n->height), use_w, total_h, colors->sel_bg);
            } else if (use_reverse) {
                backend->draw_highlight(backend, x, y - TO_DOUBLE(n->height), use_w, total_h, colors->reverse_bg);
            } else if ((colors->bg >> 24) != 0) {
                backend->draw_highlight(backend, x, y - TO_DOUBLE(n->height), use_w, total_h, colors->bg);
            }
            break;
        }
            
        case NODE_HBOX: {
            double cur_x = x;
            int p; Node *cn;
            
            // Try to consolidate background rects for the whole HBOX if possible
            uint32_t last_color = 0;
            double start_x = x;
            int in_rect = 0;

            m_foreach(n->list, p, cn) {
                Scaled effective_width = cn->width;
                if (cn->type == NODE_GLUE) {
                    if (n->glue_set != 0) {
                        if (n->glue_set > 0 && cn->glue_spec.stretch_order == n->glue_order) {
                            effective_width += (Scaled)(n->glue_set * (double)cn->glue_spec.stretch);
                        } else if (n->glue_set < 0 && cn->glue_spec.shrink_order == n->glue_order) {
                            effective_width += (Scaled)(n->glue_set * (double)cn->glue_spec.shrink);
                        }
                    }
                }
                
                uint32_t current_color = 0;
                int use_reverse = reverse_mode || cn->reverse_mode;
                if (sel_start != -1 && cn->source_offset >= sel_start && cn->source_offset <= sel_end) {
                    current_color = colors->sel_bg;
                } else if (use_reverse) {
                    current_color = colors->reverse_bg;
                } else if ((colors->bg >> 24) != 0) {
                    current_color = colors->bg;
                }

                if (current_color != last_color) {
                    if (in_rect && last_color != 0) {
                        backend->draw_highlight(backend, start_x, y - TO_DOUBLE(n->height), cur_x - start_x, TO_DOUBLE(n->height + n->depth), last_color);
                    }
                    start_x = cur_x;
                    last_color = current_color;
                    in_rect = 1;
                }
                
                // If it's a nested box, we might need to recurse if it's not a simple character/glue
                if (cn->type == NODE_HBOX || cn->type == NODE_VBOX) {
                    // Flush current rect
                    if (in_rect && last_color != 0) {
                        backend->draw_highlight(backend, start_x, y - TO_DOUBLE(n->height), cur_x - start_x, TO_DOUBLE(n->height + n->depth), last_color);
                    }
                    in_rect = 0;
                    last_color = 0;
                    renderer_render_backgrounds(backend, cn, cur_x, y + TO_DOUBLE(cn->shift), sel_start, sel_end, colors, TO_DOUBLE(effective_width), reverse_mode);
                }

                cur_x += TO_DOUBLE(effective_width);
            }
            
            if (in_rect && last_color != 0) {
                backend->draw_highlight(backend, start_x, y - TO_DOUBLE(n->height), cur_x - start_x, TO_DOUBLE(n->height + n->depth), last_color);
            }
            break;
        }
            
        case NODE_VBOX: {
            double cur_y = y - TO_DOUBLE(n->height);
            int p; Node *cn;
            m_foreach(n->list, p, cn) {
                renderer_render_backgrounds(backend, cn, x + TO_DOUBLE(cn->shift), cur_y + TO_DOUBLE(cn->height), sel_start, sel_end, colors, -1.0, reverse_mode);
                cur_y += TO_DOUBLE(cn->height + cn->depth);
            }
            break;
        }
        default: break;
    }
}

static void renderer_render_text(Backend *backend, Node *n, double x, double y, int sel_start, int sel_end, const RendererColors *colors, double width_override, int reverse_mode) {
    if (!n) return;

    double use_w = (width_override > 0) ? width_override : TO_DOUBLE(n->width);
    
    switch (n->type) {
        case NODE_CHAR: {
            int use_reverse = reverse_mode || n->reverse_mode;
            if (sel_start != -1 && n->source_offset >= sel_start && n->source_offset <= sel_end) {
                backend->set_color(backend, colors->sel_text);
            } else if (use_reverse) {
                backend->set_color(backend, colors->reverse_text);
            } else {
                backend->set_color(backend, colors->text);
            }

            if (n->font_face_handle > 0) {
                const char *face = m_str(n->font_face_handle);
                backend->set_font_face(backend, face, n->font_style);
            }
            backend->set_font_size(backend, n->font_size);
            backend->draw_char(backend, x, y, n->data);
            break;
        }
        
        case NODE_RULE:
            backend->set_color(backend, colors->text);
            backend->draw_rect(backend, x, y - TO_DOUBLE(n->height), use_w, TO_DOUBLE(n->height + n->depth), colors->text);
            break;
            
        case NODE_HBOX: {
            double cur_x = x;
            int p; Node *cn;
            m_foreach(n->list, p, cn) {
                Scaled effective_width = cn->width;
                if (cn->type == NODE_GLUE) {
                    if (n->glue_set != 0) {
                        if (n->glue_set > 0 && cn->glue_spec.stretch_order == n->glue_order) {
                            effective_width += (Scaled)(n->glue_set * (double)cn->glue_spec.stretch);
                        } else if (n->glue_set < 0 && cn->glue_spec.shrink_order == n->glue_order) {
                            effective_width += (Scaled)(n->glue_set * (double)cn->glue_spec.shrink);
                        }
                    }
                }
                renderer_render_text(backend, cn, cur_x, y + TO_DOUBLE(cn->shift), sel_start, sel_end, colors, TO_DOUBLE(effective_width), reverse_mode);
                cur_x += TO_DOUBLE(effective_width);
            }
            break;
        }
            
        case NODE_VBOX: {
            double cur_y = y - TO_DOUBLE(n->height);
            int p; Node *cn;
            m_foreach(n->list, p, cn) {
                renderer_render_text(backend, cn, x + TO_DOUBLE(cn->shift), cur_y + TO_DOUBLE(cn->height), sel_start, sel_end, colors, -1.0, reverse_mode);
                cur_y += TO_DOUBLE(cn->height + cn->depth);
            }
            break;
        }
            
        case NODE_SVG:
            if (n->data > 0) {
                const char *filename = m_str(n->data);
                backend->draw_svg(backend, x, y - TO_DOUBLE(n->height), TO_DOUBLE(n->width), TO_DOUBLE(n->height), filename);
            }
            break;
            
        default: break;
    }
}

void renderer_render(Backend *backend, int node_handle, double x, double y, int sel_start, int sel_end, const RendererColors *colors, int reverse_mode) {
    if (node_handle <= 0) return;
    Node *n = (Node*)mls(node_handle, 0);
    
    if (sel_start != -1 && sel_end != -1) {
        if (sel_start > sel_end) {
            int tmp = sel_start;
            sel_start = sel_end;
            sel_end = tmp;
        }
    }

    if (!colors) colors = &default_colors;

    // Pass 1: Backgrounds
    renderer_render_backgrounds(backend, n, x, y, sel_start, sel_end, colors, -1.0, reverse_mode);
    
    // Pass 2: Text and foregrounds
    renderer_render_text(backend, n, x, y, sel_start, sel_end, colors, -1.0, reverse_mode);
}
