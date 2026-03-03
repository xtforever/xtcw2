#include "node.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>

void node_create_char(int list_handle, int char_code, Scaled w, Scaled h, Scaled d, float fsize, int fstyle, int fhandle) {
    Node n = {0};
    n.type = NODE_CHAR;
    n.data = char_code;
    n.width = w;
    n.height = h;
    n.depth = d;
    n.font_size = fsize;
    n.font_style = fstyle;
    n.font_face_handle = fhandle;
    n.vertical_scale = 1.0f;
    m_put(list_handle, &n);
}

void node_create_hbox(int list_handle, int child_list) {
    Node n = {0};
    n.type = NODE_HBOX;
    n.list = child_list;
    n.vertical_scale = 1.0f;
    m_put(list_handle, &n);
}

void node_create_vbox(int list_handle, int child_list) {
    Node n = {0};
    n.type = NODE_VBOX;
    n.list = child_list;
    n.vertical_scale = 1.0f;
    m_put(list_handle, &n);
}

void node_create_rule(int list_handle, Scaled w, Scaled h, Scaled d) {
    Node n = {0};
    n.type = NODE_RULE;
    n.width = w;
    n.height = h;
    n.depth = d;
    n.vertical_scale = 1.0f;
    m_put(list_handle, &n);
}

void node_create_property(int list_handle, PropType type, int value) {
    Node n = {0};
    n.type = NODE_PROPERTY;
    n.data = type;
    n.width = value; // Store value in width (Scaled)
    m_put(list_handle, &n);
}

void node_create_svg(int list_handle, const char *filename, Scaled w, Scaled h) {
    Node n = {0};
    n.type = NODE_SVG;
    n.width = w;
    n.height = h;
    n.depth = 0;
    n.data = s_strdup_c(filename);
    n.vertical_scale = 1.0f;
    m_put(list_handle, &n);
}

int node_list_to_vbox(int all_lines, double font_size_pt) {
    // Calculate Height and Width
    double height = 0;
    double width = 0;
    double first_line_height = -1.0; // Marker
    double prev_depth = 0;
    int p; Node *line;

    int vbox_list = m_create(10, sizeof(Node));

    m_foreach(all_lines, p, line) {
        if (line->type == NODE_KERN && line->width == 0) {
            // Explicit vertical spacing
            double kh = TO_DOUBLE(line->height);
            if (first_line_height < 0) {
                height += kh;
            } else {
                // This kern is below the first baseline
                // We'll add it to the total height later via glue calculation
            }
            m_put(vbox_list, line);
            continue;
        }

        double h = TO_DOUBLE(line->height);
        double d = TO_DOUBLE(line->depth);
        double w = TO_DOUBLE(line->width);
        
        if (first_line_height < 0) {
            first_line_height = height + h;
            height += h;
        } else {
            // TeX-like inter-line spacing
            double baseline_skip = font_size_pt * 1.2;
            double glue_val = baseline_skip - prev_depth - h;
            if (glue_val < font_size_pt * 0.1) glue_val = font_size_pt * 0.1;
            
            node_create_kern(vbox_list, FROM_DOUBLE(glue_val));
            Node *kn = (Node*)mls(vbox_list, m_len(vbox_list) - 1);
            kn->width = 0;
            height += glue_val + h;
        }

        m_put(vbox_list, line);
        if (w > width) width = w;
        prev_depth = d;
    }
    height += prev_depth;
    if (first_line_height < 0) first_line_height = 0;

    int h_vbox = m_create(1, sizeof(Node));
    Node vbox = {0};
    vbox.type = NODE_VBOX;
    vbox.list = vbox_list;
    vbox.width = FROM_DOUBLE(width);
    vbox.height = FROM_DOUBLE(first_line_height);
    vbox.depth = FROM_DOUBLE(height - first_line_height);
    vbox.vertical_scale = 1.0f;
    m_put(h_vbox, &vbox);
    return h_vbox;
}

void node_create_glue(int list_handle, Glue spec) {
    Node n = {0};
    n.type = NODE_GLUE;
    n.glue_spec = spec;
    n.width = spec.width;
    n.vertical_scale = 1.0f;
    m_put(list_handle, &n);
}

void node_create_kern(int list_handle, Scaled amount) {
    Node n = {0};
    n.type = NODE_KERN;
    n.width = amount;
    n.height = amount;
    n.depth = 0;
    n.vertical_scale = 1.0f;
    m_put(list_handle, &n);
}

Node node_pack_hbox(int child_list, Scaled target_width) {
    Node n = {0};
    n.type = NODE_HBOX;
    n.list = child_list;
    n.vertical_scale = 1.0f;
    
    Scaled total_width = 0;
    Scaled total_stretch[4] = {0}; // Normal, fil, fill, filll
    Scaled total_shrink[4] = {0};
    
    Scaled max_h = 0;
    Scaled max_d = 0;
    
    int p; Node *child;
    m_foreach(child_list, p, child) {
        // Sum dimensions
        if (child->type == NODE_CHAR || child->type == NODE_HBOX || child->type == NODE_VBOX ||
            child->type == NODE_RULE || child->type == NODE_KERN || child->type == NODE_SVG) {
            total_width = scaled_add(total_width, child->width);
            Scaled effective_h = scaled_add(child->height, child->shift);
            Scaled effective_d = scaled_sub(child->depth, child->shift);
            if (effective_h > max_h) max_h = effective_h;
            if (effective_d > max_d) max_d = effective_d;
        } else if (child->type == NODE_GLUE) {
            total_width = scaled_add(total_width, child->width);
            total_stretch[child->glue_spec.stretch_order] += child->glue_spec.stretch;
            total_shrink[child->glue_spec.shrink_order] += child->glue_spec.shrink;
        }
    }
    
    // If target_width is "infinite" (we use 10000pt as convention for natural width)
    // or if it is 0, we take the natural width.
    if (target_width >= FROM_INT(10000) || target_width == 0) {
        n.width = total_width;
    } else {
        n.width = target_width; // The box takes the target width (justified)
    }

    n.height = max_h;
    n.depth = max_d;
    
    // Calculate Glue Set
    Scaled diff = n.width - total_width;
    
    if (diff == 0) {
        n.glue_set = 0;
    } else if (diff > 0) {
        // Stretching
        // Find highest order with non-zero stretch
        int order;
        for (order = 3; order >= 0; order--) {
             if (total_stretch[order] != 0) break;
        }
        
        if (order >= 0) {
             n.glue_set = (double)diff / (double)total_stretch[order];
             n.glue_order = order;
        } else {
             n.glue_set = 0; // Underfull box with no stretchability
             n.glue_order = 0;
        }
    } else {
        // Shrinking
         int order;
        for (order = 3; order >= 0; order--) {
             if (total_shrink[order] != 0) break;
        }
        
        if (order >= 0) {
             n.glue_set = (double)diff / (double)total_shrink[order];
             n.glue_order = order;
        } else {
             n.glue_set = 0; // Overfull box with no shrinkability
             n.glue_order = 0;
        }
    }
    
    return n;
}

void node_free_tree(int node_handle) {
    if (node_handle <= 0) return;
    
    Node *n = (Node*)mls(node_handle, 0);
    if (!n) {
        m_free(node_handle);
        return;
    }

    if (n->type == NODE_HBOX || n->type == NODE_VBOX) {
        node_list_free(n->list);
    } else if (n->type == NODE_SVG) {
        m_free(n->data);
    }
    
    m_free(node_handle);
}

void node_list_free(int list_handle) {
    if (list_handle <= 0) return;
    int p; Node *n;
    m_foreach(list_handle, p, n) {
        if (n->type == NODE_HBOX || n->type == NODE_VBOX) {
            node_list_free(n->list);
        } else if (n->type == NODE_SVG) {
            m_free(n->data);
        }
    }
    m_free(list_handle);
}

static void indent_buf(int buf, int n) {
    for (int i = 0; i < n; i++) s_app(buf, "  ", NULL);
}

void node_dump(int node_handle, int indent, int output_buf) {
    if (node_handle <= 0) return;
    Node *n = (Node*)mls(node_handle, 0);
    if (!n) return;
    indent_buf(output_buf, indent);

    switch (n->type) {
        case NODE_CHAR:
            s_printf(output_buf, -1, "CHAR '%c' w=%d h=%d d=%d s=%d\n", n->data, n->width, n->height, n->depth, n->shift);
            break;
        case NODE_HBOX:
            s_printf(output_buf, -1, "HBOX w=%d h=%d d=%d glue_set=%f\n", n->width, n->height, n->depth, n->glue_set);
            int p; Node *child;
            m_foreach(n->list, p, child) {
                // To reuse node_dump, we still need a way to pass a handle.
                // This is a bit tricky now. Let's create a temporary handle.
                int tmp = m_create(1, sizeof(Node));
                m_put(tmp, child);
                node_dump(tmp, indent + 1, output_buf);
                m_free(tmp);
            }
            break;
        case NODE_VBOX:
            s_printf(output_buf, -1, "VBOX w=%d h=%d d=%d\n", n->width, n->height, n->depth);
            int vp; Node *vchild;
            m_foreach(n->list, vp, vchild) {
                int tmp = m_create(1, sizeof(Node));
                m_put(tmp, vchild);
                node_dump(tmp, indent + 1, output_buf);
                m_free(tmp);
            }
            break;
        case NODE_RULE:
            s_printf(output_buf, -1, "RULE w=%d h=%d d=%d\n", n->width, n->height, n->depth);
            break;
        case NODE_SVG:
            s_printf(output_buf, -1, "SVG '%s' w=%d h=%d\n", m_str(n->data), n->width, n->height);
            break;
        case NODE_GLUE:
            s_printf(output_buf, -1, "GLUE w=%d\n", n->width);
            break;
        case NODE_KERN:
            s_printf(output_buf, -1, "KERN w=%d h=%d d=%d\n", n->width, n->height, n->depth);
            break;
        case NODE_PENALTY:
            s_printf(output_buf, -1, "PENALTY %d\n", n->data);
            break;
        case NODE_PROPERTY:
            s_printf(output_buf, -1, "PROPERTY %d val=%d\n", n->data, n->width);
            break;
        default:
            s_printf(output_buf, -1, "UNKNOWN NODE type=%d\n", n->type);
    }
}
