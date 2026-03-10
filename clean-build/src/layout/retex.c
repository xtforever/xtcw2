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

    // Replace literal \n and \t with actual characters
    int buf = m_create(strlen(text)+1, 1);
    const char *p_in = text;
    while (*p_in) {
        if (*p_in == '\\' && *(p_in+1) == 'n') {
            char tmp = '\n'; m_put(buf, &tmp);
            p_in += 2;
        } else if (*p_in == '\\' && *(p_in+1) == 't') {
            char tmp = '\t'; m_put(buf, &tmp);
            p_in += 2;
        } else {
            m_put(buf, p_in);
            p_in++;
        }
    }
    char tmp = 0; m_put(buf, &tmp);

    int tokens = tokenize(m_str(buf));
    m_free(buf);
    
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
            node_create_kern(all_lines, len);
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
                node_create_glue(hlist, parfill);
                
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

    m_free(all_lines);
    token_list_free(tokens);

    return para;
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


void retex_paragraph_render(RetexParagraph *para, Backend *be, double x, double y) {
    renderer_render(be, para->vbox_handle, x, y);
    if (be->flush) be->flush(be);
}

void retex_paragraph_free(RetexParagraph *para) {
    if (para) {
        node_free_tree(para->vbox_handle);
        para->vbox_handle = 0;
        free(para);
    }
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
