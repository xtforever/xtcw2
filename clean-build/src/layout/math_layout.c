#include "builder.h"
#include "math_node.h"
#include "mls.h"
#include "m_tool.h"
#include <string.h>

static float get_style_scale(MathStyle style) {
    switch (style) {
        case STYLE_SCRIPT: return 0.7f;
        case STYLE_SCRIPT_SCRIPT: return 0.5f;
        default: return 1.0f;
    }
}

static MathStyle next_style(MathStyle style) {
    switch (style) {
        case STYLE_DISPLAY:
        case STYLE_TEXT: return STYLE_SCRIPT;
        case STYLE_SCRIPT: return STYLE_SCRIPT_SCRIPT;
        default: return STYLE_SCRIPT_SCRIPT;
    }
}

static int math_field_to_hlist(MathField *f, MathStyle style, MeasureFunc measure_func, void *measure_ctx, Scaled font_size_base, const char *font_face_base) {
    int nodes = m_create(2, sizeof(Node));
    if (f->type == MATH_TYPE_CHAR) {
        float scale = get_style_scale(style);
        Scaled cur_size = (Scaled)((float)font_size_base * scale);
        int cur_style = 2; // Math is italic by default in our engine for now
        
        CharMetricsScaled m;
        if (measure_func) {
            m = measure_func(measure_ctx, f->data.c.char_code, cur_size, cur_style, font_face_base);
        } else {
            m.width = scaled_mul(cur_size, FROM_INT(6)/10);
            m.height = scaled_mul(cur_size, FROM_INT(7)/10);
            m.depth = scaled_mul(cur_size, FROM_INT(2)/10);
        }
        const char *face = font_face_base ? font_face_base : "Sans";
        int face_handle = conststr_lookup_c(face);
        node_create_char(nodes, f->data.c.char_code, m.width, m.height, m.depth, 
                                 (float)TO_DOUBLE(cur_size), cur_style, face_handle);
    } else if (f->type == MATH_TYPE_MLIST) {
        int sub = mlist_to_hlist(f->data.mlist, style, measure_func, measure_ctx, font_size_base, font_face_base);
        int p; Node *n;
        m_foreach(sub, p, n) {
            m_put(nodes, n);
        }
        m_free(sub);
    }
    return nodes;
}

typedef enum {
    SP_NONE,
    SP_THIN,
    SP_MEDIUM,
    SP_THICK
} MathSpacing;

// Spacing table rows/cols: Ord, Op, Bin, Rel, Open, Close, Punct, Inner
// This is a simplified version of TeX's spacing table
static const MathSpacing spacing_table[8][8] = {
    //         Ord      Op       Bin      Rel      Open     Close    Punct    Inner
    /* Ord  */ {SP_NONE,   SP_THIN, SP_MEDIUM, SP_THICK, SP_NONE, SP_NONE, SP_NONE, SP_THIN},
    /* Op   */ {SP_THIN,   SP_THIN, SP_NONE,   SP_THICK, SP_NONE, SP_NONE, SP_NONE, SP_THIN},
    /* Bin  */ {SP_MEDIUM, SP_MEDIUM, SP_NONE, SP_NONE,  SP_MEDIUM, SP_NONE, SP_NONE, SP_MEDIUM},
    /* Rel  */ {SP_THICK,  SP_THICK, SP_NONE,  SP_NONE,  SP_THICK, SP_NONE, SP_NONE, SP_THICK},
    /* Open */ {SP_NONE,   SP_NONE, SP_NONE,   SP_NONE,  SP_NONE, SP_NONE, SP_NONE, SP_NONE},
    /* Close*/ {SP_NONE,   SP_THIN, SP_MEDIUM, SP_THICK, SP_NONE, SP_NONE, SP_NONE, SP_THIN},
    /* Punct*/ {SP_THIN,   SP_THIN, SP_NONE,   SP_THICK, SP_THIN, SP_THIN, SP_THIN, SP_THIN},
    /* Inner*/ {SP_THIN,   SP_THIN, SP_MEDIUM, SP_THICK, SP_NONE, SP_NONE, SP_NONE, SP_THIN}
};

static Glue get_spacing_glue(MathSpacing sp, Scaled font_size) {
    Scaled unit = scaled_div(font_size, FROM_INT(18)); // 1 mu = 1/18 em
    switch (sp) {
        case SP_THIN:   return glue_create(scaled_mul(unit, FROM_INT(3)), 0, 0, 0, 0);
        case SP_MEDIUM: return glue_create(scaled_mul(unit, FROM_INT(4)), scaled_mul(unit, FROM_INT(2)), 0, scaled_mul(unit, FROM_INT(4)), 0);
        case SP_THICK:  return glue_create(scaled_mul(unit, FROM_INT(5)), scaled_mul(unit, FROM_INT(5)), 0, 0, 0);
        default: return glue_zero();
    }
}

int mlist_to_hlist(int mlist_handle, MathStyle style, MeasureFunc measure_func, void *measure_ctx, Scaled font_size_base, const char *font_face_base) {
    int hlist = m_create(10, sizeof(Node));
    if (mlist_handle <= 0) return hlist;
    
    // Math axis (approx 0.25 * font_size)
    Scaled axis = scaled_mul(font_size_base, FROM_DOUBLE(0.25));

    int p; Noad *n;
    int prev_type = -1;
    
    m_foreach(mlist_handle, p, n) {
        // Handle Scaling Delimiters
        if (n->type == NOAD_LEFT) {
            // Find NOAD_INNER and NOAD_RIGHT
            if (p + 2 < m_len(mlist_handle)) {
                Noad *ni = (Noad*)mls(mlist_handle, p+1);
                Noad *nr = (Noad*)mls(mlist_handle, p+2);
                if (ni->type == NOAD_INNER && nr->type == NOAD_RIGHT) {
                    // 1. Layout inner to find height/depth
                    int inner_hlist = mlist_to_hlist(ni->nucleus.data.mlist, style, measure_func, measure_ctx, font_size_base, font_face_base);
                    Scaled max_h = 0, max_d = 0;
                    int ihp; Node *node;
                    m_foreach(inner_hlist, ihp, node) {
                        if (node->height > max_h) max_h = node->height;
                        if (node->depth > max_d) max_d = node->depth;
                    }
                    
                    // 2. Select scale for delimiters
                    // Symmetric scale around math axis
                    double d_top = TO_DOUBLE(max_h) - TO_DOUBLE(axis);
                    double d_bot = TO_DOUBLE(max_d) + TO_DOUBLE(axis);
                    double max_dist = (d_top > d_bot) ? d_top : d_bot;
                    
                    double target_size = 2.0 * max_dist;
                    // typical delimiter height+depth is roughly font_size
                    float scale = (float)(target_size / TO_DOUBLE(font_size_base));
                    if (scale < 1.0f) scale = 1.0f;
                    
                    // 3. Layout and scale LEFT delimiter
                    int left_nodes = math_field_to_hlist(&n->nucleus, style, measure_func, measure_ctx, font_size_base, font_face_base);
                    int lnp; Node *lnode;
                    m_foreach(left_nodes, lnp, lnode) {
                        lnode->vertical_scale = scale;
                        // Center scaled glyph around axis
                        lnode->shift = axis - scaled_mul(lnode->height - lnode->depth, FROM_DOUBLE(0.5 * scale));
                        m_put(hlist, lnode);
                    }
                    m_free(left_nodes);
                    
                    // 4. Append INNER nodes
                    m_foreach(inner_hlist, ihp, node) {
                        m_put(hlist, node);
                    }
                    m_free(inner_hlist);
                    
                    // 5. Layout and scale RIGHT delimiter
                    int right_nodes = math_field_to_hlist(&nr->nucleus, style, measure_func, measure_ctx, font_size_base, font_face_base);
                    int rnp; Node *rnode;
                    m_foreach(right_nodes, rnp, rnode) {
                        rnode->vertical_scale = scale;
                        // Center scaled glyph around axis
                        rnode->shift = axis - scaled_mul(rnode->height - rnode->depth, FROM_DOUBLE(0.5 * scale));
                        m_put(hlist, rnode);
                    }
                    m_free(right_nodes);
                    
                    // Skip INNER and RIGHT in the main loop
                    p += 2;
                    continue;
                }
            }
        }

        // 0. Spacing between atoms
        if (prev_type != -1 && (int)n->type < 8 && prev_type < 8) {
            MathSpacing sp = spacing_table[prev_type][n->type];
            if (sp != SP_NONE) {
                Glue g = get_spacing_glue(sp, font_size_base);
                node_create_glue(hlist, g);
            }
        }
        prev_type = n->type;

        if (n->type == NOAD_FRACTION) {
            MathStyle sub_style = next_style(style);
            int num_hlist = math_field_to_hlist(&n->numerator, sub_style, measure_func, measure_ctx, font_size_base, font_face_base);
            int den_hlist = math_field_to_hlist(&n->denominator, sub_style, measure_func, measure_ctx, font_size_base, font_face_base);
            
            Node num_box = node_pack_hbox(num_hlist, 0);
            Node den_box = node_pack_hbox(den_hlist, 0);
            
            Scaled max_w = (num_box.width > den_box.width) ? num_box.width : den_box.width;
            max_w += FROM_INT(2);
            
            num_box.shift = (max_w - num_box.width) / 2;
            den_box.shift = (max_w - den_box.width) / 2;
            
            Scaled rule_thickness = scaled_div(font_size_base, FROM_INT(20));
            if (rule_thickness < FROM_INT(1)) rule_thickness = FROM_INT(1);
            
            // Add spacing kerns
            Scaled clearance = rule_thickness * 2;
            
            int vlist = m_create(5, sizeof(Node));
            m_put(vlist, &num_box);
            node_create_kern(vlist, clearance);
            node_create_rule(vlist, max_w, rule_thickness, 0);
            node_create_kern(vlist, clearance);
            m_put(vlist, &den_box);
            
            node_create_vbox(hlist, vlist);
            Node *vbox = (Node*)mls(hlist, m_len(hlist) - 1);
            vbox->width = max_w;
            // Center fraction around math axis
            vbox->height = num_box.height + num_box.depth + clearance + rule_thickness/2 + axis;
            vbox->depth = den_box.height + den_box.depth + clearance + rule_thickness/2 - axis;
            
            continue;
        }

        // 1. Process nucleus
        int nuc_nodes = math_field_to_hlist(&n->nucleus, style, measure_func, measure_ctx, font_size_base, font_face_base);
        
        int np; Node *nh;
        m_foreach(nuc_nodes, np, nh) {
            m_put(hlist, nh);
        }
        m_free(nuc_nodes);
        
        // 2. Process sub/sup (simple append for now to see it working)
        if (n->subscr.type != MATH_TYPE_EMPTY) {
            int sub_nodes = math_field_to_hlist(&n->subscr, next_style(style), measure_func, measure_ctx, font_size_base, font_face_base);
            
            // Apply vertical shift to each node in subscr (simplified)
            Scaled shift = -scaled_mul(font_size_base, FROM_DOUBLE(0.3 * get_style_scale(style)));
            int snp; Node *snode;
            m_foreach(sub_nodes, snp, snode) {
                snode->shift = shift;
                m_put(hlist, snode);
            }
            m_free(sub_nodes);
        }
        
        if (n->supscr.type != MATH_TYPE_EMPTY) {
            int sup_nodes = math_field_to_hlist(&n->supscr, next_style(style), measure_func, measure_ctx, font_size_base, font_face_base);
            
            Scaled shift = scaled_mul(font_size_base, FROM_DOUBLE(0.3 * get_style_scale(style)));
            int snp; Node *snode;
            m_foreach(sup_nodes, snp, snode) {
                snode->shift = shift;
                m_put(hlist, snode);
            }
            m_free(sup_nodes);
        }
    }
    
    return hlist;
}
