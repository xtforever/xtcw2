#include "linebreak.h"
#include "builder.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>

int line_break(int node_list, Scaled width, Glue left_skip, Glue right_skip, Scaled hang_indent, int hang_after) {
    int lines = m_create(5, sizeof(Node)); // List of HBox Nodes
    int line_count = 0;
    int p; Node *n;
    int line_nodes = 0;

    auto void start_new_line(void) {
        line_nodes = m_create(10, sizeof(Node));
        line_count++;

        Scaled cur_indent = 0;
        int apply_hang = 0;
        if (hang_after >= 0) {
            if (line_count > hang_after) apply_hang = 1;
        } else {
            if (line_count <= -hang_after) apply_hang = 1;
        }
        
        if (apply_hang && hang_indent > 0) cur_indent = hang_indent;

        node_create_glue(line_nodes, left_skip, 0);
        if (cur_indent > 0) {
            node_create_kern(line_nodes, cur_indent, 0);
        }
    };

    auto Scaled get_current_target_width(void) {
        int apply_hang = 0;
        int next_line = line_count + 1;
        if (hang_after >= 0) {
            if (next_line > hang_after) apply_hang = 1;
        } else {
            if (next_line <= -hang_after) apply_hang = 1;
        }
        if (apply_hang) {
            if (hang_indent > 0) return scaled_sub(width, hang_indent);
            return scaled_add(width, hang_indent); // hang_indent is negative
        }
        return width;
    };

    start_new_line();
    Scaled line_width = left_skip.width + ((hang_after < 0 && hang_indent > 0) ? hang_indent : 0);
    
    // Word buffer
    int word_nodes = m_create(10, sizeof(Node));
    Scaled word_width = 0;
    
    m_foreach(node_list, p, n) {
        if (n->type == NODE_GLUE) {
            Scaled target = get_current_target_width();
            if (m_len(line_nodes) > 1 && (line_width + word_width + right_skip.width > target)) {
                // Overflow!
                if (m_len(line_nodes) > 1) { 
                    int last_idx = m_len(line_nodes) - 1;
                    Node *last_n = (Node*)mls(line_nodes, last_idx);
                    if (last_n->type == NODE_GLUE) {
                        m_del(line_nodes, last_idx);
                    }
                }
                
                node_create_glue(line_nodes, right_skip, 0);
                Node hbox = node_pack_hbox(line_nodes, width);
                m_put(lines, &hbox);
                
                start_new_line();
                int wp; Node *wn;
                m_foreach(word_nodes, wp, wn) {
                    m_put(line_nodes, wn);
                }
                line_width = left_skip.width + word_width;
                if (hang_indent > 0) {
                    int apply_hang = (hang_after >= 0) ? (line_count > hang_after) : (line_count <= -hang_after);
                    if (apply_hang) line_width = scaled_add(line_width, hang_indent);
                }

                m_clear(word_nodes);
                word_width = 0;
                
                m_put(line_nodes, n);
                line_width = scaled_add(line_width, n->width);
            } else {
                int wp; Node *wn;
                m_foreach(word_nodes, wp, wn) {
                    m_put(line_nodes, wn);
                }
                line_width = scaled_add(line_width, word_width);
                m_clear(word_nodes);
                word_width = 0;
                
                m_put(line_nodes, n);
                line_width = scaled_add(line_width, n->width);
            }
        } else {
            m_put(word_nodes, n);
            word_width = scaled_add(word_width, n->width);
        }
    }
    
    int wp; Node *wn;
    m_foreach(word_nodes, wp, wn) {
        m_put(line_nodes, wn);
    }
    node_create_glue(line_nodes, right_skip, 0);
    Node hbox = node_pack_hbox(line_nodes, width);
    m_put(lines, &hbox);
    
    m_free(word_nodes);
    return lines;
}
