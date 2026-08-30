#include "retex.h"
#include "builder.h"
#include "linebreak.h"
#include "token.h"
#include "node.h"
#include "renderer.h"
#include "scaled.h"
#include "glue.h"
#include "mls.h"
#include "m_tool.h"

/* Workaround for redefinition error in conststr.h vs m_tool.h */
#define s_cstr s_cstr_hidden
#define s_mstr s_mstr_hidden
#include "conststr.h"
#undef s_cstr
#undef s_mstr

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct RetexParagraph {
    int vbox_handle;
    double height;
    double width;
    double first_line_height;
    int selection_start;
    int selection_end;
};

static CharMetricsScaled internal_measure_char(void *ctx, int c, Scaled font_size, int style, const char *face) {
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

static Scaled retex_layout_parse_length(int token_list, int *p_idx, Scaled em_size) {
    int len = m_len(token_list);
    if (*p_idx >= len) return 0;

    Token *t = (Token*)mls(token_list, *p_idx);
    int in_group = 0;
    if (t->type == TOK_GROUP_BEGIN) {
        in_group = 1;
        (*p_idx)++;
    }

    int start = *p_idx;
    int end = start;
    
    int buf = m_create(16, 1);
    while (end < len) {
        Token *t = (Token*)mls(token_list, end);
        if (t->type == TOK_CHAR) {
            if (t->char_code == '-' || isdigit(t->char_code) || t->char_code == '.' || isalpha(t->char_code)) {
                char tmp = (char)t->char_code;
                m_put(buf, &tmp);
                end++;
            } else break;
        } else if (t->type == TOK_GROUP_END && in_group) {
            end++;
            break;
        } else if (t->type == TOK_SPACE) {
            if (m_len(buf) == 0) { end++; start++; continue; }
            break;
        } else break;
    }
    
    char tmp = 0; m_put(buf, &tmp);
    const char *s = m_str(buf);
    char *endptr;
    double val = strtod(s, &endptr);
    Scaled result = 0;
    
    if (endptr != s) {
        if (strncmp(endptr, "pt", 2) == 0) result = FROM_DOUBLE(val);
        else if (strncmp(endptr, "mm", 2) == 0) result = FROM_DOUBLE(val * 72.27 / 25.4);
        else if (strncmp(endptr, "cm", 2) == 0) result = FROM_DOUBLE(val * 72.27 / 2.54);
        else if (strncmp(endptr, "in", 2) == 0) result = FROM_DOUBLE(val * 72.27);
        else if (strncmp(endptr, "em", 2) == 0) result = (Scaled)(val * (double)em_size);
        else result = FROM_DOUBLE(val); 
    }
    
    m_free(buf);
    *p_idx = end - 1; 
    return result;
}

RetexParagraph* retex_layout(Backend *be, const char *text, double width_pt, const char *font_face, double font_size_pt, RetexAlign align) {
    if (!text) return NULL;
    RetexParagraph *para = malloc(sizeof(RetexParagraph));
    if (!para) return NULL;

    const char *face = font_face ? font_face : "Sans";
    int face_handle = conststr_lookup_c(face);
    
    be->set_font_face(be, face, 0);
    be->set_font_size(be, font_size_pt);

    int tokens = tokenize(text);
    
    int all_lines = m_create(10, sizeof(Node));

    Glue parfill = glue_create(0, FROM_INT(1000), ORDER_FIL, 0, ORDER_NORMAL);
    Glue zero = glue_zero();
    Glue fil = glue_create(0, FROM_INT(1), ORDER_FIL, 0, ORDER_NORMAL);
    Scaled target_width = FROM_DOUBLE(width_pt);

    Glue left_skip, right_skip;
    switch (align) {
        case RETEX_ALIGN_CENTER:
            left_skip = fil; right_skip = fil; break;
        case RETEX_ALIGN_RIGHT:
            left_skip = fil; right_skip = zero; break;
        case RETEX_ALIGN_LEFT:
            left_skip = zero; right_skip = fil; break;
        default: // JUSTIFY
            left_skip = zero; right_skip = zero; break;
    }

    int tp = 0;
    while (tp < m_len(tokens)) {
        Token *t = (Token*)mls(tokens, tp);
        
        if (t->type == TOK_COMMAND && (strcmp(m_str(t->cmd_name), "vspace") == 0 || strcmp(m_str(t->cmd_name), "vskip") == 0)) {
            tp++;
            Scaled len = retex_layout_parse_length(tokens, &tp, FROM_DOUBLE(font_size_pt));
            node_create_kern(all_lines, len, 0);
            Node *kn = (Node*)mls(all_lines, m_len(all_lines) - 1);
            kn->width = 0; // vertical kern has no width
            tp++;
        } else if (t->type == TOK_PAR || t->type == TOK_SPACE) {
            tp++;
        } else {
            // Start of paragraph
            int current_para_tokens = m_create(10, sizeof(Token));
            while (tp < m_len(tokens)) {
                Token *t2 = (Token*)mls(tokens, tp);
                if (t2->type == TOK_PAR) break;
                if (t2->type == TOK_COMMAND && (strcmp(m_str(t2->cmd_name), "vspace") == 0 || strcmp(m_str(t2->cmd_name), "vskip") == 0)) break;
                m_put(current_para_tokens, t2);
                tp++;
            }
            
            if (m_len(current_para_tokens) > 0) {
                int hlist = build_hlist(current_para_tokens, internal_measure_char, be, FROM_DOUBLE(font_size_pt), face, face_handle);
                if (align != RETEX_ALIGN_CENTER && align != RETEX_ALIGN_RIGHT) {
                    node_create_glue(hlist, parfill, 0);
                    Node *gn = (Node*)mls(hlist, m_len(hlist) - 1);
                    if (tp < m_len(tokens)) {
                        Token *t_end = (Token*)mls(tokens, tp);
                        gn->source_offset = t_end->source_offset;
                    } else {
                        Token *t_last = (Token*)mls(tokens, m_len(tokens) - 1);
                        gn->source_offset = t_last->source_offset;
                    }
                }
                
                Scaled hang_indent = 0;
                int hang_after = 1;
                
                // Scan for properties and remove them from hlist
                int hp;
                for (hp = 0; hp < m_len(hlist); hp++) {
                    Node *n = (Node*)mls(hlist, hp);
                    if (n->type == NODE_PROPERTY) {
                        if (n->data == PROP_HANG_INDENT) hang_indent = n->width;
                        else if (n->data == PROP_HANG_AFTER) hang_after = (int)n->width;
                        m_del(hlist, hp); hp--;
                    }
                }

                int lines = line_break(hlist, target_width, left_skip, right_skip, hang_indent, hang_after);
                int lp; Node *lnode;
                m_foreach(lines, lp, lnode) {
                    m_put(all_lines, lnode);
                }
                m_free(lines);
                m_free(hlist);
            }
            m_free(current_para_tokens);
        }
    }

    para->vbox_handle = node_list_to_vbox(all_lines, font_size_pt);
    Node *vbox = (Node*)mls(para->vbox_handle, 0);
    para->width = TO_DOUBLE(vbox->width);
    para->height = TO_DOUBLE(vbox->height + vbox->depth);
    para->first_line_height = TO_DOUBLE(vbox->height);
    para->selection_start = -1;
    para->selection_end = -1;

    m_free(all_lines);
    token_list_free(tokens);

    return para;
}

void retex_paragraph_set_selection(RetexParagraph *para, int start, int end) {
    if (para) {
        para->selection_start = start;
        para->selection_end = end;
    }
}

double retex_paragraph_get_height(RetexParagraph *para) {
    return para->height;
}

double retex_paragraph_get_width(RetexParagraph *para) {
    return para->width;
}

double retex_paragraph_get_first_line_height(RetexParagraph *para) {
    return para->first_line_height;
}


void retex_paragraph_render(RetexParagraph *para, Backend *be, double x, double y, const RendererColors *colors, int reverse_mode) {
    renderer_render(be, para->vbox_handle, x, y, para->selection_start, para->selection_end, colors, reverse_mode);
    if (be->flush) be->flush(be);
}

void retex_paragraph_free(RetexParagraph *para) {
    if (para) {
        node_free_tree(para->vbox_handle);
        para->vbox_handle = 0;
        free(para);
    }
}

static int find_node_at(Node *n, Scaled x, Scaled y, int *current_idx) {
    if (!n) return -1;

    if (n->type == NODE_CHAR || n->type == NODE_GLUE || n->type == NODE_RULE) {
        if (x >= 0 && x <= n->width && y >= -n->height && y <= n->depth) {
            if (n->source_offset >= 0) return n->source_offset;
        }
        return -1;
    }

    if (n->type == NODE_HBOX) {
        Scaled cur_x = 0;
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

            if (x >= cur_x && x <= cur_x + effective_width) {
                int found = find_node_at(cn, x - cur_x, y - cn->shift, current_idx);
                if (found != -1) return found;
            }
            cur_x = scaled_add(cur_x, effective_width);
        }
    }
 else if (n->type == NODE_VBOX) {
        Scaled cur_y = - n->height; // Top of VBOX
        int p; Node *cn;
        m_foreach(n->list, p, cn) {
            Scaled baseline = scaled_add(cur_y, cn->height);
            Scaled next_top = scaled_add(cur_y, scaled_add(cn->height, cn->depth));
            
            if (y >= cur_y && y <= next_top) {
                int found = find_node_at(cn, x - cn->shift, y - baseline, current_idx);
                if (found != -1) return found;
            }
            cur_y = next_top;
        }
    }

    return -1;
}

int retex_paragraph_get_node_at(RetexParagraph *para, double x, double y) {
    if (!para) return -1;
    Node *vbox = (Node*)mls(para->vbox_handle, 0);
    int idx = 0;
    return find_node_at(vbox, FROM_DOUBLE(x), FROM_DOUBLE(y - para->first_line_height), &idx);
}

double retex_measure_height(Backend *be, const char *text, double width_pt, const char *font_face, double font_size_pt) {
    const char *face = font_face ? font_face : "Sans";
    RetexParagraph *para = retex_layout(be, text, width_pt, face, font_size_pt, RETEX_ALIGN_JUSTIFY);
    double h = 0;
    if (para) {
        h = para->height;
        retex_paragraph_free(para);
    }
    return h;
}
