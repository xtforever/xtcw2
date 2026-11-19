#include "retex.h"
#include "builder.h"
#include "linebreak.h"
#include "token.h"
#include "node.h"
#include "renderer.h"
#include "scaled.h"
#include "glue.h"
#include "shaping.h"
#include "hyphen.h"
#include "utf8.h"
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
#include <math.h>

struct RetexParagraph {
    int vbox_handle;
    double height;
    double width;
    double first_line_height;
    int selection_start;
    int selection_end;
    /* plain-layout line metadata (NULL/0 for TeX layout) */
    RetexLine *lines;
    int nlines;
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

/* Shape one UTF-8 run and append its clusters to an hlist.  Returns 1 on
 * success, 0 when shaping is unavailable (caller falls back to per-char
 * metrics). */
static int append_shaped_run(int wnodes, const char *utf8, int len,
                             int base_offset, const char *face,
                             int face_handle, int style, double size,
                             int reverse_mode, Scaled *out_width)
{
    ShapingFont *sf;
    double width = 0.0;

    if (!utf8 || len <= 0 || !face)
        return 0;
    sf = shaping_font_get(face, style, size);
    if (!sf)
        return 0;
    if (!shaping_append_run(wnodes, sf, utf8, len, base_offset,
                            (float)size, style, face_handle, reverse_mode,
                            &width))
        return 0;
    *out_width = scaled_add(*out_width, FROM_DOUBLE(width));
    return 1;
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
            if (t->char_code == '-' || (t->char_code >= '0' && t->char_code <= '9') ||
                t->char_code == '.' || (t->char_code >= 'a' && t->char_code <= 'z') ||
                (t->char_code >= 'A' && t->char_code <= 'Z')) {
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

/* Recursively collect the [min, max) source range covered by character nodes
 * in a line subtree.  Non-character nodes (glue/kern/rule) carry no source and
 * are skipped; nested boxes (e.g. \hbox) are descended into. */
static void collect_line_range(Node *n, int *start, int *end, int *found) {
    int p; Node *cn;
    if (!n) return;
    if (n->type == NODE_CHAR || n->type == NODE_GLUE) {
        if (n->source_offset >= 0) {
            int s = n->source_offset;
            int e = n->source_offset + n->byte_len;
            if (!*found) { *start = s; *end = e; *found = 1; }
            else { if (s < *start) *start = s; if (e > *end) *end = e; }
        }
        return;
    }
    if (n->type == NODE_HBOX || n->type == NODE_VBOX) {
        m_foreach(n->list, p, cn) collect_line_range(cn, start, end, found);
    }
}

/* Populate para->lines/nlines from the paragraph vbox so that the caret,
 * hit-test and scrolling helpers work for TeX (rich) layout too.  y is the top
 * of the line relative to the paragraph top, accumulated exactly like
 * renderer_render_node()'s NODE_VBOX case (height + depth of every child in
 * order, including inter-line and \vspace kerns).  vbox_index is the line
 * HBox's position in the vbox child list. */
static void retrace_line_metadata(RetexParagraph *para) {
    Node *vbox;
    int p; Node *cn;
    double cur = 0.0;
    RetexLine *meta = NULL;
    int n = 0, cap = 0;

    if (!para || para->vbox_handle <= 0) return;
    vbox = (Node *)mls(para->vbox_handle, 0);
    if (!vbox) return;

    m_foreach(vbox->list, p, cn) {
        if (cn->type == NODE_HBOX) {
            int start = 0, end = 0, found = 0;
            collect_line_range(cn, &start, &end, &found);
            if (n == cap) {
                int nc = cap ? cap * 2 : 8;
                RetexLine *nm = (RetexLine *)realloc(meta, (size_t)nc * sizeof(RetexLine));
                if (!nm) { free(meta); return; }
                meta = nm; cap = nc;
            }
            meta[n].start_byte = found ? start : -1;
            meta[n].end_byte = found ? end : -1;
            meta[n].y = cur;
            meta[n].height = TO_DOUBLE(cn->height + cn->depth);
            meta[n].vbox_index = p;
            n++;
        }
        cur += TO_DOUBLE(cn->height + cn->depth);
    }

    free(para->lines);
    para->lines = meta;
    para->nlines = n;
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

                int lines;
                if (hang_indent != 0) {
                    lines = line_break(hlist, target_width, left_skip, right_skip,
                                       hang_indent, hang_after);
                } else {
                    LineBreakParams lbp;
                    memset(&lbp, 0, sizeof(lbp));
                    lbp.width = target_width;
                    lbp.left_skip = left_skip;
                    lbp.right_skip = right_skip;
                    lbp.first_indent = 0;
                    lbp.rest_indent = 0;
                    lbp.right_indent = 0;
                    lbp.tolerance = 200;
                    lbp.emergencystretch = FROM_DOUBLE(font_size_pt);
                    lbp.hyphen_penalty = 50;
                    lines = line_break_kp(hlist, &lbp);
                }
                int lp; Node *lnode;
                m_foreach(lines, lp, lnode) {
                    m_put(all_lines, lnode);
                }
                m_free(lines);
                node_list_free(hlist);
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
    para->lines = NULL;
    para->nlines = 0;
    retrace_line_metadata(para);

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
        free(para->lines);
        para->lines = NULL;
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

/* ------------------------------------------------------------------ */
/* Plain-text layout                                                   */
/* ------------------------------------------------------------------ */

/* Effective width of a child, accounting for glue stretch/shrink exactly as
 * the renderer does. */
static Scaled plain_eff_width(Node *parent, Node *cn) {
    Scaled w = cn->width;
    if (cn->type == NODE_GLUE && parent && parent->glue_set != 0) {
        if (parent->glue_set > 0 &&
            cn->glue_spec.stretch_order == parent->glue_order) {
            w = scaled_add(w, (Scaled)(parent->glue_set * (double)cn->glue_spec.stretch));
        } else if (parent->glue_set < 0 &&
                   cn->glue_spec.shrink_order == parent->glue_order) {
            w = scaled_add(w, (Scaled)(parent->glue_set * (double)cn->glue_spec.shrink));
        }
    }
    return w;
}

static void append_plain_hyphenated_word(int para_nodes, Backend *be,
                                         const char *wbuf, int wlen,
                                         int base_offset, const char *face,
                                         int face_handle, double size,
                                         int reverse_mode, Scaled max_width);

RetexParagraph* retex_layout_plain(Backend *be, const char *text, double width_pt,
                                   const char *font_face, double font_size_pt,
                                   RetexAlign align) {
    const char *face;
    int face_handle, tokens, nt, t0, text_len;
    Scaled target, space_w;
    Glue zero, fil, left_skip, right_skip, space_glue;
    CharMetricsScaled sm;
    int all_lines;
    Node vbox;
    RetexParagraph *para;
    int h_vbox;

    if (!text) return NULL;

    face = font_face ? font_face : "Sans";
    face_handle = conststr_lookup_c(face);
    be->set_font_face(be, face, 0);
    be->set_font_size(be, font_size_pt);

    target = FROM_DOUBLE(width_pt);
    tokens = tokenize_plain(text);
    nt = m_len(tokens);
    text_len = (int)strlen(text);

    zero = glue_zero();
    fil = glue_create(0, FROM_INT(1), ORDER_FIL, 0, ORDER_NORMAL);
    switch (align) {
        case RETEX_ALIGN_CENTER: left_skip = fil; right_skip = fil; break;
        case RETEX_ALIGN_RIGHT: left_skip = fil; right_skip = zero; break;
        case RETEX_ALIGN_JUSTIFY: left_skip = zero; right_skip = zero; break;
        default: left_skip = zero; right_skip = fil; break; /* LEFT */
    }

    sm = internal_measure_char(be, ' ', FROM_DOUBLE(font_size_pt), 0, face);
    space_w = sm.width;
    /* TeX-like interword glue: a little stretch, a little shrink. */
    space_glue = glue_create(space_w, scaled_div(space_w, FROM_INT(2)),
                             ORDER_NORMAL, scaled_div(space_w, FROM_INT(3)),
                             ORDER_NORMAL);

    all_lines = m_create(8, sizeof(Node));

    t0 = 0;
    for (;;) {
        int t1 = t0;
        int para_nodes, i;

        while (t1 < nt && ((Token *)mls(tokens, t1))->type != TOK_HARD_BREAK)
            t1++;

        para_nodes = m_create(16, sizeof(Node));
        int has_word = 0;

        i = t0;
        while (i < t1) {
            Token *tok = (Token *)mls(tokens, i);
            if (tok->type == TOK_SPACE) {
                Node *g;
                node_create_glue(para_nodes, space_glue, 0);
                g = (Node *)mls(para_nodes, m_len(para_nodes) - 1);
                g->source_offset = tok->source_offset;
                g->byte_len = tok->byte_len;
                i++;
                continue;
            }

            /* Gather a word: a run of TOK_CHAR tokens. */
            {
                int word_start_byte = tok->source_offset;
                int wlen = 0;
                char *wbuf = (char *)malloc((size_t)(t1 - i) * 4 + 1);

                while (i < t1 && ((Token *)mls(tokens, i))->type == TOK_CHAR) {
                    Token *c = (Token *)mls(tokens, i);
                    if (wbuf) {
                        char tmp[4];
                        int k = utf8_encode_cp((uint32_t)c->char_code, tmp);
                        if (k > 0) {
                            memcpy(wbuf + wlen, tmp, (size_t)k);
                            wlen += k;
                        }
                    }
                    i++;
                }
                if (wbuf) {
                    wbuf[wlen] = 0;
                    append_plain_hyphenated_word(para_nodes, be, wbuf, wlen,
                                                 word_start_byte, face,
                                                 face_handle, font_size_pt, 0,
                                                 target);
                    has_word = 1;
                }
                free(wbuf);
            }
        }

        if (!has_word) {
            /* Blank line: keep a visible empty line with the font's height
             * and a source anchor so line metadata/caret mapping survives. */
            CharMetricsScaled em = internal_measure_char(be, ' ',
                                    FROM_DOUBLE(font_size_pt), 0, face);
            int line_nodes = m_create(4, sizeof(Node));
            Node hbox;
            Node *cn;
            int blank_start = (t0 < nt)
                ? ((Token *)mls(tokens, t0))->source_offset : text_len;
            node_create_glue(line_nodes, left_skip, 0);
            node_create_char(line_nodes, ' ', 0, em.height, em.depth,
                             font_size_pt, 0, face_handle, 0);
            cn = (Node *)mls(line_nodes, m_len(line_nodes) - 1);
            cn->source_offset = blank_start;
            cn->byte_len = (blank_start < text_len) ? 1 : 0;
            cn->shaped_hidden = 1;
            node_create_glue(line_nodes, right_skip, 0);
            hbox = node_pack_hbox(line_nodes, target);
            m_put(all_lines, &hbox);
        } else if (m_len(para_nodes) > 0) {
            LineBreakParams lbp;
            int lines, lp;
            Node *lnode;
            memset(&lbp, 0, sizeof(lbp));
            lbp.width = target;
            lbp.left_skip = left_skip;
            lbp.right_skip = right_skip;
            lbp.first_indent = 0;
            lbp.rest_indent = 0;
            lbp.right_indent = 0;
            lbp.tolerance = 200;
            lbp.emergencystretch = space_w;
            lbp.hyphen_penalty = 50;
            lines = line_break_kp(para_nodes, &lbp);
            m_foreach(lines, lp, lnode) {
                m_put(all_lines, lnode);
            }
            m_free(lines);
        }
        node_list_free(para_nodes);

        if (t1 >= nt) break;
        t0 = t1 + 1;
    }

    token_list_free(tokens);

    h_vbox = node_list_to_vbox(all_lines, font_size_pt);
    memset(&vbox, 0, sizeof(vbox));
    {
        Node *vb = (Node *)mls(h_vbox, 0);
        if (vb) vbox = *vb;
    }

    para = (RetexParagraph *)malloc(sizeof(RetexParagraph));
    if (!para) {
        node_free_tree(h_vbox);
        m_free(all_lines);
        return NULL;
    }
    para->vbox_handle = h_vbox;
    para->width = TO_DOUBLE(vbox.width);
    para->height = TO_DOUBLE(vbox.height + vbox.depth);
    para->first_line_height = TO_DOUBLE(vbox.height);
    para->selection_start = -1;
    para->selection_end = -1;
    para->lines = NULL;
    para->nlines = 0;
    retrace_line_metadata(para);
    m_free(all_lines);
    return para;
}

/* ------------------------------------------------------------------ */
/* Styled layout (rich atoms)                                          */
/* ------------------------------------------------------------------ */

/* UTF-8 byte length of a code point (objects are encoded as U+FFFC). */
static int utf8_len_of(uint32_t c) {
    if (c < 0x80) return 1;
    if (c < 0x800) return 2;
    if (c < 0x10000) return 3;
    return 4;
}

static double atom_size(const RAtom *a, double default_size) {
    return (a->look.size > 0.0f) ? (double)a->look.size : default_size;
}

/* Fallback for a shaped run that HarfBuzz could not handle: one char node per
 * UTF-8 code point. */
static void append_fallback_run(int wnodes, const char *utf8, int len,
                                int base_offset, Backend *be,
                                const char *face, int face_handle, int style,
                                double size, int reverse_mode,
                                Scaled *out_width)
{
    const char *p = utf8;
    const char *end = utf8 + len;
    int consumed = 0;

    while (p < end) {
        uint32_t cp = 0;
        int n = utf8_decode_cp(&p, &cp);
        CharMetricsScaled m;
        Node *cn;
        if (n <= 0)
            break;
        m = internal_measure_char(be, (int)cp, FROM_DOUBLE(size), style, face);
        node_create_char(wnodes, (int)cp, m.width, m.height, m.depth,
                         (float)size, style, face_handle, reverse_mode);
        cn = (Node *)mls(wnodes, m_len(wnodes) - 1);
        cn->source_offset = base_offset + consumed;
        cn->byte_len = n;
        *out_width = scaled_add(*out_width, m.width);
        consumed += n;
    }
}

/* A discretionary hyphen: the hyphen glyph list is only appended to the line
 * when Knuth-Plass actually breaks at this penalty. */
static void append_hyphen_penalty(int para_nodes, Backend *be,
                                  const char *face, int face_handle, int style,
                                  double size, int reverse_mode)
{
    int hnodes = m_create(2, sizeof(Node));
    Scaled hw = 0;
    Node hbox;
    int list;

    if (!append_shaped_run(hnodes, "-", 1, 0, face, face_handle, style,
                           size, reverse_mode, &hw)) {
        CharMetricsScaled m = internal_measure_char(be, '-', FROM_DOUBLE(size),
                                                    style, face);
        node_create_char(hnodes, '-', m.width, m.height, m.depth,
                         (float)size, style, face_handle, reverse_mode);
        hw = m.width;
    }
    hbox = node_pack_hbox(hnodes, 0);
    list = m_create(1, sizeof(Node));
    m_put(list, &hbox);
    node_create_penalty(para_nodes, 200, hbox.width, list, reverse_mode);
}

/* Put a word box into the paragraph, splitting it at cluster boundaries when
 * it is wider than the available measure.  This is the emergency break for
 * long URLs/identifiers and for words without a hyphenation dictionary. */
static void append_word_box_split(int para_nodes, Node *wbox, Scaled max_width)
{
    int list, chunk, p;
    Node *cn;
    Scaled chunk_w = 0;

    if (!wbox || wbox->type != NODE_HBOX || max_width <= 0 ||
        wbox->width <= max_width) {
        if (wbox) m_put(para_nodes, wbox);
        return;
    }

    list = wbox->list;
    chunk = m_create(8, sizeof(Node));
    m_foreach(list, p, cn) {
        Scaled cw = cn->width;
        if (chunk_w > 0 && scaled_add(chunk_w, cw) > max_width) {
            Node hbox = node_pack_hbox(chunk, 0);
            m_put(para_nodes, &hbox);
            node_create_penalty(para_nodes, 1000, 0, 0, 0);
            chunk = m_create(8, sizeof(Node));
            chunk_w = 0;
        }
        {
            Node copy = *cn;
            if (cn->type == NODE_HBOX || cn->type == NODE_VBOX)
                cn->list = 0;
            else if (cn->type == NODE_CHAR)
                cn->shaped = NULL;
            else if (cn->type == NODE_SVG)
                cn->data = 0;
            m_put(chunk, &copy);
            chunk_w = scaled_add(chunk_w, cw);
        }
    }
    if (m_len(chunk) > 0) {
        Node hbox = node_pack_hbox(chunk, 0);
        m_put(para_nodes, &hbox);
    } else {
        m_free(chunk);
    }
    node_list_free(list);
}

/* Plain word with optional automatic hyphenation. */
static void append_plain_hyphenated_word(int para_nodes, Backend *be,
                                         const char *wbuf, int wlen,
                                         int base_offset, const char *face,
                                         int face_handle, double size,
                                         int reverse_mode, Scaled max_width)
{
    int count = 0, i, start = 0;
    int *breaks = hyphen_breaks(wbuf, wlen, &count);

    for (i = 0; i < count; i++) {
        int b = breaks[i];
        int wnodes;
        Scaled ww = 0;
        Node wbox;
        if (b <= start || b >= wlen)
            continue;
        wnodes = m_create(8, sizeof(Node));
        if (!append_shaped_run(wnodes, wbuf + start, b - start,
                               base_offset + start, face, face_handle, 0,
                               size, reverse_mode, &ww)) {
            append_fallback_run(wnodes, wbuf + start, b - start,
                                base_offset + start, be, face, face_handle,
                                0, size, reverse_mode, &ww);
        }
        wbox = node_pack_hbox(wnodes, 0);
        append_word_box_split(para_nodes, &wbox, max_width);
        append_hyphen_penalty(para_nodes, be, face, face_handle, 0, size,
                              reverse_mode);
        start = b;
    }

    {
        int wnodes = m_create(8, sizeof(Node));
        Scaled ww = 0;
        Node wbox;
        if (!append_shaped_run(wnodes, wbuf + start, wlen - start,
                               base_offset + start, face, face_handle, 0,
                               size, reverse_mode, &ww)) {
            append_fallback_run(wnodes, wbuf + start, wlen - start,
                                base_offset + start, be, face, face_handle,
                                0, size, reverse_mode, &ww);
        }
        wbox = node_pack_hbox(wnodes, 0);
        append_word_box_split(para_nodes, &wbox, max_width);
    }
    free(breaks);
}

/* Build a word from styled atoms, shaping each same-font run separately.
 * Falls back to per-atom metrics when shaping is unavailable. */
static void append_styled_word(int wnodes, const RAtom *atoms, int start, int end,
                               const int *boff, double font_size_pt,
                               const char *default_face,
                               int default_face_handle, Backend *be,
                               Scaled *out_width)
{
    int p = start;

    while (p < end) {
        const RAtom *a = &atoms[p];
        if (a->kind == RATOM_CHAR && a->cp == '\r') {
            p++;
            continue;
        }
        if (a->kind == RATOM_CHAR) {
            int q = p + 1;
            int fh = (a->look.face > 0) ? a->look.face : default_face_handle;
            const char *fc = (a->look.face > 0) ? m_str(a->look.face)
                                                : default_face;
            int style = a->look.style;
            double sz = atom_size(a, font_size_pt);
            char *wbuf;
            int wlen = 0, t;

            while (q < end) {
                const RAtom *b = &atoms[q];
                int bfh = (b->look.face > 0) ? b->look.face
                                             : default_face_handle;
                double bsz = atom_size(b, font_size_pt);
                if (b->kind != RATOM_CHAR || bfh != fh ||
                    b->look.style != style || fabs(bsz - sz) > 0.01)
                    break;
                q++;
            }

            wbuf = (char *)malloc((size_t)(q - p) * 4 + 1);
            if (wbuf) {
                for (t = p; t < q; t++) {
                    char tmp[4];
                    int k = utf8_encode_cp(atoms[t].cp, tmp);
                    if (k > 0) {
                        memcpy(wbuf + wlen, tmp, (size_t)k);
                        wlen += k;
                    }
                }
                wbuf[wlen] = 0;
            }

            if (!wbuf || !append_shaped_run(wnodes, wbuf, wlen, boff[p],
                                            fc, fh, style, sz, 0,
                                            out_width)) {
                for (t = p; t < q; t++) {
                    CharMetricsScaled m = internal_measure_char(be,
                                          (int)atoms[t].cp, FROM_DOUBLE(sz),
                                          style, fc);
                    Node *cn;
                    node_create_char(wnodes, (int)atoms[t].cp, m.width,
                                     m.height, m.depth, (float)sz, style, fh,
                                     0);
                    cn = (Node *)mls(wnodes, m_len(wnodes) - 1);
                    cn->source_offset = boff[t];
                    cn->byte_len = boff[t + 1] - boff[t];
                    *out_width = scaled_add(*out_width, m.width);
                }
            }
            free(wbuf);
            p = q;
        } else {
            Node *on;
            node_create_rule(wnodes, 0, 0, 0, 0);
            on = (Node *)mls(wnodes, m_len(wnodes) - 1);
            on->source_offset = boff[p];
            on->byte_len = boff[p + 1] - boff[p];
            p++;
        }
    }
}

static void append_styled_word_box(int para_nodes, int wnodes,
                                  Scaled max_width)
{
    Node wbox = node_pack_hbox(wnodes, 0);
    append_word_box_split(para_nodes, &wbox, max_width);
}

/* Styled word with optional automatic hyphenation.  Hyphenation points are
 * byte offsets in the word's UTF-8 representation; they are mapped back to
 * atom boundaries via boff. */
static void append_styled_hyphenated_word(int para_nodes, const RAtom *atoms,
                                          int start, int end, const int *boff,
                                          double font_size_pt,
                                          const char *default_face,
                                          int default_face_handle, Backend *be,
                                          Scaled max_width)
{
    int wlen, i, pos = 0, count = 0, start_atom = start;
    char *wbuf;
    int *breaks;
    int has_object = 0, has_cr = 0;

    for (i = start; i < end; i++) {
        if (atoms[i].kind != RATOM_CHAR) { has_object = 1; break; }
        if (atoms[i].cp == '\r') has_cr = 1;
    }
    if (has_object || has_cr || end <= start) {
        int wnodes = m_create(8, sizeof(Node));
        Scaled ww = 0;
        append_styled_word(wnodes, atoms, start, end, boff, font_size_pt,
                           default_face, default_face_handle, be, &ww);
        append_styled_word_box(para_nodes, wnodes, max_width);
        return;
    }

    wlen = boff[end] - boff[start];
    wbuf = (char *)malloc((size_t)wlen + 1);
    if (!wbuf) {
        int wnodes = m_create(8, sizeof(Node));
        Scaled ww = 0;
        append_styled_word(wnodes, atoms, start, end, boff, font_size_pt,
                           default_face, default_face_handle, be, &ww);
        append_styled_word_box(para_nodes, wnodes, max_width);
        return;
    }
    for (i = start; i < end; i++) {
        char tmp[4];
        int n = utf8_encode_cp(atoms[i].cp, tmp);
        if (n > 0) {
            memcpy(wbuf + pos, tmp, (size_t)n);
            pos += n;
        }
    }
    wbuf[pos] = 0;
    wlen = pos;

    breaks = hyphen_breaks(wbuf, wlen, &count);
    for (i = 0; i < count; i++) {
        int b = breaks[i];
        int target_byte, atom_mid, wnodes;
        Scaled ww = 0;
        const RAtom *a;
        int fh;
        const char *fc;
        double sz;

        if (b <= 0 || b >= wlen)
            continue;
        target_byte = boff[start] + b;
        atom_mid = start_atom;
        while (atom_mid < end && boff[atom_mid] < target_byte)
            atom_mid++;
        if (atom_mid <= start_atom || atom_mid >= end)
            continue;

        wnodes = m_create(8, sizeof(Node));
        append_styled_word(wnodes, atoms, start_atom, atom_mid, boff,
                           font_size_pt, default_face, default_face_handle,
                           be, &ww);
        append_styled_word_box(para_nodes, wnodes, max_width);

        a = &atoms[atom_mid - 1];
        fh = (a->look.face > 0) ? a->look.face : default_face_handle;
        fc = (a->look.face > 0) ? m_str(a->look.face) : default_face;
        sz = atom_size(a, font_size_pt);
        append_hyphen_penalty(para_nodes, be, fc, fh, a->look.style, sz, 0);
        start_atom = atom_mid;
    }

    {
        int wnodes = m_create(8, sizeof(Node));
        Scaled ww = 0;
        Node wbox;
        append_styled_word(wnodes, atoms, start_atom, end, boff, font_size_pt,
                           default_face, default_face_handle, be, &ww);
        wbox = node_pack_hbox(wnodes, 0);
        append_word_box_split(para_nodes, &wbox, max_width);
    }
    free(breaks);
    free(wbuf);
}

/* A line containing only spaces, tabs or CR is blank and separates paragraphs. */
static int para_segment_blank(const RAtom *atoms, int i, int j) {
    int k;
    for (k = i; k < j; k++) {
        const RAtom *a = &atoms[k];
        if (a->kind != RATOM_CHAR) return 0;
        if (a->cp != ' ' && a->cp != '\t' && a->cp != '\r') return 0;
    }
    return 1;
}

static void para_align_glues(RetexAlign align, Glue zero, Glue fil,
                             Glue *left_skip, Glue *right_skip) {
    switch (align) {
        case RETEX_ALIGN_CENTER: *left_skip = fil; *right_skip = fil; break;
        case RETEX_ALIGN_RIGHT: *left_skip = fil; *right_skip = zero; break;
        case RETEX_ALIGN_JUSTIFY: *left_skip = zero; *right_skip = zero; break;
        default: *left_skip = zero; *right_skip = fil; break; /* LEFT */
    }
}

static RetexParaFormat para_fmt_at(const RetexParaFormat *fmts, int nfmts,
                                   int idx, RetexAlign default_align) {
    RetexParaFormat f;
    if (fmts && idx >= 0 && idx < nfmts)
        return fmts[idx];
    f.align = (int)default_align;
    f.indent_left = 0.0;
    f.indent_right = 0.0;
    f.first_line = 0.0;
    f.space_before = 0.0;
    f.space_after = 0.0;
    return f;
}

int retex_paragraphs(const RAtom *atoms, int natoms, int *out, int max) {
    int n = 0, i = 0, in_para = 0;

    if (natoms < 0) natoms = 0;
    if (natoms == 0) {
        if (out && max > 0) out[0] = 0;
        return 1;
    }

    for (;;) {
        int j = i;
        int blank;
        while (j < natoms &&
               !(atoms[j].kind == RATOM_CHAR && atoms[j].cp == '\n'))
            j++;
        blank = para_segment_blank(atoms, i, j);
        if (!blank && !in_para) {
            if (out && n < max) out[n] = i;
            n++;
            in_para = 1;
        }
        if (blank)
            in_para = 0;
        if (j >= natoms)
            break;
        i = j + 1;
    }

    if (n == 0) {
        if (out && max > 0) out[0] = 0;
        return 1;
    }
    return n;
}

static RetexParagraph* styled_layout_impl(Backend *be, const RAtom *atoms,
                                          int natoms, double width_pt,
                                          const char *font_face,
                                          double font_size_pt,
                                          const RetexParaFormat *fmts, int nfmts,
                                          RetexAlign default_align,
                                          int compact_blank) {
    const char *face;
    int face_handle;
    Scaled target;
    Glue zero, fil;
    Node vbox;
    RetexParagraph *para;
    int h_vbox, i, k;
    int *boff;
    int cur_para = -1, in_para = 0, para_first = 0;

    if (natoms < 0) return NULL;
    if (!atoms && natoms > 0) return NULL;

    face = font_face ? font_face : "Sans";
    face_handle = conststr_lookup_c(face);
    be->set_font_face(be, face, 0);
    be->set_font_size(be, font_size_pt);

    target = FROM_DOUBLE(width_pt);

    zero = glue_zero();
    fil = glue_create(0, FROM_INT(1), ORDER_FIL, 0, ORDER_NORMAL);

    /* Precompute the UTF-8 byte offset of every atom boundary. */
    boff = (int *)malloc((size_t)(natoms + 1) * sizeof(int));
    if (!boff) return NULL;
    boff[0] = 0;
    for (k = 0; k < natoms; k++) {
        uint32_t c = (atoms[k].kind == RATOM_CHAR) ? atoms[k].cp : 0xFFFC;
        boff[k + 1] = boff[k] + utf8_len_of(c);
    }

    {
        int all_lines = m_create(8, sizeof(Node));
        double pending_space = 0.0;
        double space_w;
        CharMetricsScaled swm = internal_measure_char(be, ' ',
                                FROM_DOUBLE(font_size_pt), 0, face);
        space_w = TO_DOUBLE(swm.width);

        i = 0;
        for (;;) {
            int j = i;
            int blank;
            RetexParaFormat pf;
            double l0, l1, right_margin;

            while (j < natoms &&
                   !(atoms[j].kind == RATOM_CHAR && atoms[j].cp == '\n'))
                j++;

            blank = para_segment_blank(atoms, i, j);
            if (!blank) {
                if (!in_para) {
                    in_para = 1;
                    cur_para++;
                    para_first = 1;
                    pf = para_fmt_at(fmts, nfmts, cur_para, default_align);
                    if (pf.space_before > pending_space)
                        pending_space = pf.space_before;
                } else {
                    para_first = 0;
                    pf = para_fmt_at(fmts, nfmts, cur_para, default_align);
                }
            } else {
                if (in_para) {
                    RetexParaFormat prev = para_fmt_at(fmts, nfmts, cur_para,
                                                       default_align);
                    if (prev.space_after > pending_space)
                        pending_space = prev.space_after;
                    in_para = 0;
                }
                para_first = 0;
                pf = para_fmt_at(NULL, 0, 0, default_align);
            }

            if (blank) {
                if (compact_blank) {
                    if (j >= natoms) break;
                    i = j + 1;
                    continue;
                }
                /* Classic layout: a blank line is a visible empty line. */
                {
                    Glue lskip, rskip;
                    int line_nodes = m_create(4, sizeof(Node));
                    Node hbox;
                    para_align_glues((RetexAlign)pf.align, zero, fil,
                                     &lskip, &rskip);
                    node_create_glue(line_nodes, lskip, 0);
                    node_create_glue(line_nodes, rskip, 0);
                    hbox = node_pack_hbox(line_nodes, target);
                    m_put(all_lines, &hbox);
                }
                if (j >= natoms) break;
                i = j + 1;
                continue;
            }

            l0 = pf.indent_left;
            l1 = pf.indent_left + pf.first_line;
            right_margin = pf.indent_right;
            if (l0 < 0.0) l0 = 0.0;
            if (l1 < 0.0) l1 = 0.0;
            if (right_margin < 0.0) right_margin = 0.0;

            if (pending_space > 0.0) {
                node_create_kern(all_lines, FROM_DOUBLE(pending_space), 0);
                {
                    Node *kn = (Node *)mls(all_lines, m_len(all_lines) - 1);
                    kn->width = 0;
                }
                pending_space = 0.0;
            }

            {
                Glue lskip, rskip;
                int para_nodes = m_create(16, sizeof(Node));
                LineBreakParams lbp;
                int lines, lp;
                Node *lnode;

                para_align_glues((RetexAlign)pf.align, zero, fil,
                                 &lskip, &rskip);

                k = i;
                while (k < j) {
                    const RAtom *a = &atoms[k];
                    if (a->kind == RATOM_CHAR && a->cp == '\r') { k++; continue; }
                    if (a->kind == RATOM_CHAR &&
                        (a->cp == ' ' || a->cp == '\t')) {
                        double sz = atom_size(a, font_size_pt);
                        const char *fc = (a->look.face > 0) ? m_str(a->look.face)
                                                            : face;
                        CharMetricsScaled sm = internal_measure_char(be, ' ',
                                                FROM_DOUBLE(sz),
                                                a->look.style, fc);
                        Glue sg = glue_create(sm.width,
                                              scaled_div(sm.width, FROM_INT(2)),
                                              ORDER_NORMAL,
                                              scaled_div(sm.width, FROM_INT(3)),
                                              ORDER_NORMAL);
                        Node *g;
                        node_create_glue(para_nodes, sg, 0);
                        g = (Node *)mls(para_nodes, m_len(para_nodes) - 1);
                        g->source_offset = boff[k];
                        g->byte_len = boff[k + 1] - boff[k];
                        k++;
                        continue;
                    }

                    {
                        int word_start_k = k;

                        while (k < j) {
                            const RAtom *c = &atoms[k];
                            if (c->kind == RATOM_CHAR && c->cp == '\r') {
                                k++;
                                continue;
                            }
                            if (c->kind == RATOM_CHAR &&
                                (c->cp == ' ' || c->cp == '\t' ||
                                 c->cp == '\n'))
                                break;
                            k++;
                        }
                        append_styled_hyphenated_word(para_nodes, atoms,
                                                      word_start_k, k, boff,
                                                      font_size_pt, face,
                                                      face_handle, be, target);
                    }
                }

                memset(&lbp, 0, sizeof(lbp));
                lbp.width = target;
                lbp.left_skip = lskip;
                lbp.right_skip = rskip;
                lbp.first_indent = FROM_DOUBLE(para_first ? l1 : l0);
                lbp.rest_indent = FROM_DOUBLE(l0);
                lbp.right_indent = FROM_DOUBLE(right_margin);
                lbp.tolerance = 200;
                lbp.emergencystretch = FROM_DOUBLE(space_w);
                lbp.hyphen_penalty = 50;
                lines = line_break_kp(para_nodes, &lbp);
                m_foreach(lines, lp, lnode) {
                    m_put(all_lines, lnode);
                }
                m_free(lines);
                node_list_free(para_nodes);
            }

            para_first = 0;

            {
                int last_seg = 1;
                if (j < natoms) {
                    int nj = j + 1;
                    while (nj < natoms &&
                           !(atoms[nj].kind == RATOM_CHAR &&
                             atoms[nj].cp == '\n'))
                        nj++;
                    last_seg = para_segment_blank(atoms, j + 1, nj);
                }
                if (last_seg && pf.space_after > pending_space)
                    pending_space = pf.space_after;
            }

            if (j >= natoms) break;
            i = j + 1;
        }

        if (pending_space > 0.0) {
            node_create_kern(all_lines, FROM_DOUBLE(pending_space), 0);
            {
                Node *kn = (Node *)mls(all_lines, m_len(all_lines) - 1);
                kn->width = 0;
            }
        }

        free(boff);
        h_vbox = node_list_to_vbox(all_lines, font_size_pt);
        memset(&vbox, 0, sizeof(vbox));
        {
            Node *vb = (Node *)mls(h_vbox, 0);
            if (vb) vbox = *vb;
        }
        m_free(all_lines);
    }

    para = (RetexParagraph *)malloc(sizeof(RetexParagraph));
    if (!para) {
        node_free_tree(h_vbox);
        return NULL;
    }
    para->vbox_handle = h_vbox;
    para->width = TO_DOUBLE(vbox.width);
    para->height = TO_DOUBLE(vbox.height + vbox.depth);
    para->first_line_height = TO_DOUBLE(vbox.height);
    para->selection_start = -1;
    para->selection_end = -1;
    para->lines = NULL;
    para->nlines = 0;
    retrace_line_metadata(para);
    return para;
}

RetexParagraph* retex_layout_styled(Backend *be, const RAtom *atoms, int natoms,
                                    double width_pt, const char *font_face,
                                    double font_size_pt, RetexAlign align) {
    RetexParaFormat f;
    f.align = (int)align;
    f.indent_left = 0.0;
    f.indent_right = 0.0;
    f.first_line = 0.0;
    f.space_before = 0.0;
    f.space_after = 0.0;
    return styled_layout_impl(be, atoms, natoms, width_pt, font_face,
                              font_size_pt, &f, 1, align, 0);
}

RetexParagraph* retex_layout_styled_paras(Backend *be, const RAtom *atoms,
                                          int natoms, double width_pt,
                                          const char *font_face,
                                          double font_size_pt,
                                          const RetexParaFormat *fmts, int nfmts,
                                          RetexAlign default_align) {
    return styled_layout_impl(be, atoms, natoms, width_pt, font_face,
                              font_size_pt, fmts, nfmts, default_align, 0);
}

RetexParagraph* retex_layout_styled_paras_compact(Backend *be, const RAtom *atoms,
                                                  int natoms, double width_pt,
                                                  const char *font_face,
                                                  double font_size_pt,
                                                  const RetexParaFormat *fmts,
                                                  int nfmts,
                                                  RetexAlign default_align) {
    return styled_layout_impl(be, atoms, natoms, width_pt, font_face,
                              font_size_pt, fmts, nfmts, default_align, 1);
}


static int find_char_font(Node *n, int pos, int *style, double *size, int *face) {
    int p; Node *cn;
    if (!n) return 0;
    if (n->type == NODE_CHAR) {
        if (n->source_offset == pos) {
            if (style) *style = n->font_style;
            if (size) *size = n->font_size;
            if (face) *face = n->font_face_handle;
            return 1;
        }
        return 0;
    }
    if (n->type == NODE_HBOX || n->type == NODE_VBOX) {
        m_foreach(n->list, p, cn)
            if (find_char_font(cn, pos, style, size, face)) return 1;
    }
    return 0;
}

int retex_paragraph_get_font_at(RetexParagraph *para, int pos,
                                int *style, double *size, int *face_handle) {
    Node *vbox;
    if (!para) return 0;
    vbox = (Node *)mls(para->vbox_handle, 0);
    return find_char_font(vbox, pos, style, size, face_handle);
}

int retex_paragraph_get_line_count(RetexParagraph *para) {
    return para ? para->nlines : 0;
}

int retex_paragraph_get_lines(RetexParagraph *para, RetexLine *out, int max) {
    int i, n;
    if (!para || !out || max <= 0) return 0;
    n = para->nlines < max ? para->nlines : max;
    for (i = 0; i < n; i++) out[i] = para->lines[i];
    return n;
}

int retex_paragraph_get_line(RetexParagraph *para, int index, RetexLine *out) {
    if (!para || !out || index < 0 || index >= para->nlines) return 0;
    *out = para->lines[index];
    return 1;
}

static double node_hit_advance(Node *parent, Node *n)
{
    if (!n) return 0.0;
    if (n->type == NODE_GLUE)
        return TO_DOUBLE(plain_eff_width(parent, n));
    return TO_DOUBLE(n->width);
}

static int caret_x_rec(Node *n, Node *parent, int byte_offset, double *cur,
                       double *fx, double *last_text_x, int *found)
{
    int p;
    Node *cn;

    if (!n) return 0;
    if (n->type == NODE_CHAR) {
        double w = TO_DOUBLE(n->width);
        if (n->shaped_hidden) return 0;
        if (n->source_offset >= 0) {
            if (byte_offset <= n->source_offset) {
                *fx = *cur;
                *found = 1;
                return 1;
            }
            if (byte_offset < n->source_offset + n->byte_len) {
                *fx = *cur;
                *found = 1;
                return 1;
            }
            *last_text_x = *cur + w;
        }
        *cur += w;
        return 0;
    }
    if (n->type == NODE_HBOX || n->type == NODE_VBOX) {
        m_foreach(n->list, p, cn) {
            if (caret_x_rec(cn, n, byte_offset, cur, fx, last_text_x, found))
                return 1;
        }
        return 0;
    }
    if (n->type == NODE_PENALTY)
        return 0;
    if (n->type == NODE_GLUE && n->source_offset >= 0) {
        double w = node_hit_advance(parent, n);
        if (byte_offset <= n->source_offset) {
            *fx = *cur;
            *found = 1;
            return 1;
        }
        if (byte_offset < n->source_offset + n->byte_len) {
            *fx = *cur;
            *found = 1;
            return 1;
        }
        *last_text_x = *cur + w;
    }
    *cur += node_hit_advance(parent, n);
    return 0;
}

static int offset_in_rec(Node *n, Node *parent, double x, double *cur,
                         int *seen_text, int line_start, int line_end)
{
    int p;
    Node *cn;

    if (!n) return -1;
    if (n->type == NODE_CHAR) {
        double w = TO_DOUBLE(n->width);
        if (n->shaped_hidden)
            return -1;
        if (x < *cur + w) {
            if (n->source_offset >= 0) {
                if (x <= *cur + w / 2.0)
                    return n->source_offset;
                return n->source_offset + n->byte_len;
            }
            return *seen_text ? line_end : line_start;
        }
        if (n->source_offset >= 0)
            *seen_text = 1;
        *cur += w;
        return -1;
    }
    if (n->type == NODE_HBOX || n->type == NODE_VBOX) {
        m_foreach(n->list, p, cn) {
            int r = offset_in_rec(cn, n, x, cur, seen_text, line_start, line_end);
            if (r != -1)
                return r;
        }
        return -1;
    }
    if (n->type == NODE_PENALTY)
        return -1;
    {
        double w = node_hit_advance(parent, n);
        if (x < *cur + w) {
            if (n->source_offset >= 0) {
                if (x <= *cur + w / 2.0)
                    return n->source_offset;
                return n->source_offset + n->byte_len;
            }
            return *seen_text ? line_end : line_start;
        }
        if (n->source_offset >= 0)
            *seen_text = 1;
        *cur += w;
    }
    return -1;
}

int retex_paragraph_get_caret_pos(RetexParagraph *para, int byte_offset,
                                  double *x, double *y, double *height) {
    int idx, i, vlist;
    RetexLine *ln;
    Node *vbox, *hbox;
    double cur = 0, fx = 0, last_text_x = 0;
    int found = 0;

    if (!para || para->nlines <= 0) return 0;

    idx = para->nlines - 1;
    for (i = 0; i < para->nlines; i++) {
        if (byte_offset <= para->lines[i].end_byte) {
            idx = i;
            break;
        }
    }
    ln = &para->lines[idx];

    vbox = (Node *)mls(para->vbox_handle, 0);
    vlist = vbox->list;
    hbox = (Node *)mls(vlist, ln->vbox_index);

    caret_x_rec(hbox, NULL, byte_offset, &cur, &fx, &last_text_x, &found);
    if (!found) fx = last_text_x;

    if (x) *x = fx;
    if (y) *y = ln->y;
    if (height) *height = ln->height;
    return 1;
}

int retex_paragraph_get_offset_at(RetexParagraph *para, double x, double y) {
    int idx, i, vlist;
    RetexLine *ln;
    Node *vbox, *hbox;
    double cur = 0;
    int seen_text = 0;

    if (!para || para->nlines <= 0) return -1;

    idx = 0;
    for (i = 0; i < para->nlines; i++) {
        RetexLine *l = &para->lines[i];
        if (y >= l->y && y < l->y + l->height) { idx = i; break; }
        if (y < l->y) { idx = i; break; }
        idx = i;
    }
    ln = &para->lines[idx];

    vbox = (Node *)mls(para->vbox_handle, 0);
    vlist = vbox->list;
    hbox = (Node *)mls(vlist, ln->vbox_index);

    {
        int r = offset_in_rec(hbox, NULL, x, &cur, &seen_text,
                              ln->start_byte, ln->end_byte);
        if (r != -1)
            return r;
    }
    return ln->end_byte;
}

void retex_set_features(const char *features)
{
    shaping_set_features(features);
}

void retex_set_language(const char *lang)
{
    shaping_set_language(lang);
    hyphen_set_language(lang);
}

void retex_paragraph_render_range(RetexParagraph *para, Backend *be,
                                  int first_line, int last_line,
                                  double x, double y,
                                  const RendererColors *colors, int reverse_mode) {
    int i, vlist;
    double y0;
    Node *vbox;

    if (!para || para->nlines <= 0) return;
    if (first_line < 0) first_line = 0;
    if (last_line >= para->nlines) last_line = para->nlines - 1;
    if (last_line < first_line) return;

    vbox = (Node *)mls(para->vbox_handle, 0);
    vlist = vbox->list;
    y0 = para->lines[first_line].y;

    for (i = first_line; i <= last_line; i++) {
        RetexLine *ln = &para->lines[i];
        Node *hbox = (Node *)mls(vlist, ln->vbox_index);
        double baseline = y + (ln->y - y0) + TO_DOUBLE(hbox->height);
        renderer_render_node(be, hbox, x, baseline,
                             para->selection_start, para->selection_end,
                             colors, reverse_mode);
    }
}
