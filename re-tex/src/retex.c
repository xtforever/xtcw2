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
    if (n->type == NODE_CHAR) {
        if (n->source_offset >= 0) {
            int s = n->source_offset;
            int e = n->source_offset + (n->byte_len > 0 ? n->byte_len : 1);
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

typedef struct {
    int vbox_list;
    Glue right_skip;
    Scaled target;
    double font_size;
    const char *face;
    RetexLine *meta;
    int n, cap;
    double top, first_h, max_w;
    int first;
} PlainCtx;

static void plain_finish_line(Backend *be, PlainCtx *ctx, int hlist,
                              int has_word, int start_byte, int end_byte) {
    Node hbox;
    double h, d, w;
    RetexLine *ln;

    if (!has_word) {
        CharMetricsScaled sm = internal_measure_char(be, ' ',
                                FROM_DOUBLE(ctx->font_size), 0, ctx->face);
        node_create_rule(hlist, 0, sm.height, sm.depth, 0);
    }
    node_create_glue(hlist, ctx->right_skip, 0);
    hbox = node_pack_hbox(hlist, ctx->target);

    h = TO_DOUBLE(hbox.height);
    d = TO_DOUBLE(hbox.depth);
    w = TO_DOUBLE(hbox.width);

    if (ctx->n == ctx->cap) {
        int nc = ctx->cap ? ctx->cap * 2 : 8;
        RetexLine *nm = (RetexLine *)realloc(ctx->meta, (size_t)nc * sizeof(RetexLine));
        if (!nm) return; /* OOM: drop line metadata, layout still stands */
        ctx->meta = nm;
        ctx->cap = nc;
    }

    ln = &ctx->meta[ctx->n];
    ln->start_byte = start_byte;
    ln->end_byte = end_byte;
    ln->y = ctx->top;
    ln->height = h + d;
    ln->vbox_index = m_len(ctx->vbox_list);
    if (ctx->first) {
        ctx->first_h = h;
        ctx->first = 0;
    }
    if (w > ctx->max_w) ctx->max_w = w;
    ctx->top += h + d;
    ctx->n++;
    m_put(ctx->vbox_list, &hbox);
}

RetexParagraph* retex_layout_plain(Backend *be, const char *text, double width_pt,
                                   const char *font_face, double font_size_pt,
                                   RetexAlign align) {
    const char *face;
    int face_handle, tokens, nt, text_len, t0;
    Scaled target, space_w;
    Glue zero, fil, left_skip, right_skip, space_glue;
    CharMetricsScaled sm;
    PlainCtx ctx;
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
    space_glue = glue_create(space_w, space_w, ORDER_NORMAL, 0, ORDER_NORMAL);

    memset(&ctx, 0, sizeof(ctx));
    ctx.vbox_list = m_create(8, sizeof(Node));
    ctx.right_skip = right_skip;
    ctx.target = target;
    ctx.font_size = font_size_pt;
    ctx.face = face;
    ctx.first = 1;

    t0 = 0;
    for (;;) {
        int t1 = t0;
        int hlist, i, has_word, line_start_byte, line_end_byte, line_end_sp;
        Scaled cur_w;

        while (t1 < nt && ((Token *)mls(tokens, t1))->type != TOK_HARD_BREAK)
            t1++;

        hlist = m_create(16, sizeof(Node));
        node_create_glue(hlist, left_skip, 0);
        cur_w = 0;
        has_word = 0;
        line_start_byte = (t0 < nt) ? ((Token *)mls(tokens, t0))->source_offset : text_len;
        line_end_byte = line_start_byte;
        line_end_sp = line_start_byte;

        i = t0;
        while (i < t1) {
            Token *tok = (Token *)mls(tokens, i);
            if (tok->type == TOK_SPACE) {
                Node *g;
                node_create_glue(hlist, space_glue, 0);
                g = (Node *)mls(hlist, m_len(hlist) - 1);
                g->source_offset = tok->source_offset;
                g->byte_len = tok->byte_len;
                line_end_sp = tok->source_offset + tok->byte_len;
                cur_w = scaled_add(cur_w, space_w);
                i++;
                continue;
            }

            /* Gather a word: a run of TOK_CHAR tokens. */
            {
                int word_start_byte = tok->source_offset;
                int word_end_byte = word_start_byte;
                int wnodes = m_create(8, sizeof(Node));
                Scaled ww = 0;

                while (i < t1 && ((Token *)mls(tokens, i))->type == TOK_CHAR) {
                    Token *c = (Token *)mls(tokens, i);
                    CharMetricsScaled m = internal_measure_char(be, c->char_code,
                                          FROM_DOUBLE(font_size_pt), 0, face);
                    Node *cn;
                    node_create_char(wnodes, c->char_code, m.width, m.height, m.depth,
                                     font_size_pt, 0, face_handle, 0);
                    cn = (Node *)mls(wnodes, m_len(wnodes) - 1);
                    cn->source_offset = c->source_offset;
                    cn->byte_len = c->byte_len;
                    ww = scaled_add(ww, m.width);
                    word_end_byte = c->source_offset + c->byte_len;
                    i++;
                }

                if (has_word && target > 0 && scaled_add(cur_w, ww) > target) {
                    /* Break before this word: drop the trailing space glue(s). */
                    while (m_len(hlist) > 1) {
                        Node *last = (Node *)mls(hlist, m_len(hlist) - 1);
                        if (last->type != NODE_GLUE) break;
                        m_del(hlist, m_len(hlist) - 1);
                    }
                    plain_finish_line(be, &ctx, hlist, has_word,
                                      line_start_byte, line_end_byte);
                    hlist = m_create(16, sizeof(Node));
                    node_create_glue(hlist, left_skip, 0);
                    cur_w = 0;
                    has_word = 0;
                    line_start_byte = word_start_byte;
                    line_end_byte = word_start_byte;
                    line_end_sp = word_start_byte;
                }

                {
                    Node *wn; int wp;
                    m_foreach(wnodes, wp, wn) {
                        m_put(hlist, wn);
                    }
                }
                cur_w = scaled_add(cur_w, ww);
                has_word = 1;
                line_end_byte = word_end_byte;
                line_end_sp = word_end_byte;
                m_free(wnodes);
            }
        }

        plain_finish_line(be, &ctx, hlist, has_word, line_start_byte, line_end_sp);

        if (t1 >= nt) break;
        t0 = t1 + 1;
    }

    token_list_free(tokens);

    h_vbox = m_create(1, sizeof(Node));
    memset(&vbox, 0, sizeof(vbox));
    vbox.type = NODE_VBOX;
    vbox.list = ctx.vbox_list;
    vbox.width = FROM_DOUBLE(ctx.max_w);
    vbox.height = FROM_DOUBLE(ctx.first_h);
    vbox.depth = FROM_DOUBLE(ctx.top - ctx.first_h);
    vbox.vertical_scale = 1.0f;
    m_put(h_vbox, &vbox);

    para = (RetexParagraph *)malloc(sizeof(RetexParagraph));
    if (!para) {
        node_free_tree(h_vbox);
        free(ctx.meta);
        return NULL;
    }
    para->vbox_handle = h_vbox;
    para->width = ctx.max_w;
    para->height = ctx.top;
    para->first_line_height = ctx.first_h;
    para->selection_start = -1;
    para->selection_end = -1;
    para->lines = ctx.meta;
    para->nlines = ctx.n;
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

RetexParagraph* retex_layout_styled(Backend *be, const RAtom *atoms, int natoms,
                                    double width_pt, const char *font_face,
                                    double font_size_pt, RetexAlign align) {
    const char *face;
    int face_handle;
    Scaled target;
    Glue zero, fil, left_skip, right_skip;
    PlainCtx ctx;
    Node vbox;
    RetexParagraph *para;
    int h_vbox, i, k;
    int *boff;

    if (natoms < 0) return NULL;
    if (!atoms && natoms > 0) return NULL;

    face = font_face ? font_face : "Sans";
    face_handle = conststr_lookup_c(face);
    be->set_font_face(be, face, 0);
    be->set_font_size(be, font_size_pt);

    target = FROM_DOUBLE(width_pt);

    zero = glue_zero();
    fil = glue_create(0, FROM_INT(1), ORDER_FIL, 0, ORDER_NORMAL);
    switch (align) {
        case RETEX_ALIGN_CENTER: left_skip = fil; right_skip = fil; break;
        case RETEX_ALIGN_RIGHT: left_skip = fil; right_skip = zero; break;
        case RETEX_ALIGN_JUSTIFY: left_skip = zero; right_skip = zero; break;
        default: left_skip = zero; right_skip = fil; break; /* LEFT */
    }

    /* Precompute the UTF-8 byte offset of every atom boundary. */
    boff = (int *)malloc((size_t)(natoms + 1) * sizeof(int));
    if (!boff) return NULL;
    boff[0] = 0;
    for (k = 0; k < natoms; k++) {
        uint32_t c = (atoms[k].kind == RATOM_CHAR) ? atoms[k].cp : 0xFFFC;
        boff[k + 1] = boff[k] + utf8_len_of(c);
    }

    memset(&ctx, 0, sizeof(ctx));
    ctx.vbox_list = m_create(8, sizeof(Node));
    ctx.right_skip = right_skip;
    ctx.target = target;
    ctx.font_size = font_size_pt;
    ctx.face = face;
    ctx.first = 1;

    i = 0;
    for (;;) {
        int j = i;
        int hlist, has_word, line_start, line_end, line_end_sp;
        Scaled cur_w;

        while (j < natoms &&
               !(atoms[j].kind == RATOM_CHAR && atoms[j].cp == '\n'))
            j++;

        hlist = m_create(16, sizeof(Node));
        node_create_glue(hlist, left_skip, 0);
        cur_w = 0;
        has_word = 0;
        line_start = boff[i];
        line_end = boff[i];
        line_end_sp = boff[i];

        k = i;
        while (k < j) {
            const RAtom *a = &atoms[k];
            if (a->kind == RATOM_CHAR && a->cp == '\r') { k++; continue; }
            if (a->kind == RATOM_CHAR && (a->cp == ' ' || a->cp == '\t')) {
                double sz = atom_size(a, font_size_pt);
                const char *fc = (a->look.face > 0) ? m_str(a->look.face) : face;
                /* Measure a space (matches the plain layout, which reuses one
                 * space width for both ' ' and '\t'). */
                CharMetricsScaled sm = internal_measure_char(be, ' ',
                                        FROM_DOUBLE(sz), a->look.style, fc);
                Glue sg = glue_create(sm.width, sm.width, ORDER_NORMAL, 0, ORDER_NORMAL);
                Node *g;
                node_create_glue(hlist, sg, 0);
                g = (Node *)mls(hlist, m_len(hlist) - 1);
                g->source_offset = boff[k];
                g->byte_len = boff[k + 1] - boff[k];
                line_end_sp = boff[k + 1];
                cur_w = scaled_add(cur_w, sm.width);
                k++;
                continue;
            }

            /* Gather a word: consecutive non-space, non-newline atoms. */
            {
                int word_start_bytes = boff[k];
                int word_end_bytes = boff[k];
                int wnodes = m_create(8, sizeof(Node));
                Scaled ww = 0;

                while (k < j) {
                    const RAtom *c = &atoms[k];
                    if (c->kind == RATOM_CHAR && c->cp == '\r') { k++; continue; }
                    if (c->kind == RATOM_CHAR &&
                        (c->cp == ' ' || c->cp == '\t' || c->cp == '\n'))
                        break;
                    if (c->kind == RATOM_CHAR) {
                        double sz = atom_size(c, font_size_pt);
                        const char *fc = (c->look.face > 0) ? m_str(c->look.face) : face;
                        int fh = (c->look.face > 0) ? c->look.face : face_handle;
                        CharMetricsScaled m = internal_measure_char(be, (int)c->cp,
                                              FROM_DOUBLE(sz), c->look.style, fc);
                        Node *cn;
                        node_create_char(wnodes, (int)c->cp, m.width, m.height, m.depth,
                                         (float)sz, c->look.style, fh, 0);
                        cn = (Node *)mls(wnodes, m_len(wnodes) - 1);
                        cn->source_offset = boff[k];
                        cn->byte_len = boff[k + 1] - boff[k];
                        ww = scaled_add(ww, m.width);
                    } else {
                        /* Inline object placeholder (Phase 4 fills this in). */
                        Node *on;
                        node_create_rule(wnodes, 0, 0, 0, 0);
                        on = (Node *)mls(wnodes, m_len(wnodes) - 1);
                        on->source_offset = boff[k];
                        on->byte_len = boff[k + 1] - boff[k];
                    }
                    k++;
                }
                word_end_bytes = boff[k];

                if (has_word && target > 0 && scaled_add(cur_w, ww) > target) {
                    /* Break before this word: drop the trailing space glue(s). */
                    while (m_len(hlist) > 1) {
                        Node *last = (Node *)mls(hlist, m_len(hlist) - 1);
                        if (last->type != NODE_GLUE) break;
                        m_del(hlist, m_len(hlist) - 1);
                    }
                    plain_finish_line(be, &ctx, hlist, has_word,
                                      line_start, line_end);
                    hlist = m_create(16, sizeof(Node));
                    node_create_glue(hlist, left_skip, 0);
                    cur_w = 0;
                    has_word = 0;
                    line_start = word_start_bytes;
                    line_end = word_start_bytes;
                    line_end_sp = word_start_bytes;
                }

                {
                    Node *wn; int wp;
                    m_foreach(wnodes, wp, wn) {
                        m_put(hlist, wn);
                    }
                }
                cur_w = scaled_add(cur_w, ww);
                has_word = 1;
                line_end = word_end_bytes;
                line_end_sp = word_end_bytes;
                m_free(wnodes);
            }
        }

        plain_finish_line(be, &ctx, hlist, has_word, line_start, line_end_sp);

        if (j >= natoms) break;
        i = j + 1;
    }

    free(boff);

    h_vbox = m_create(1, sizeof(Node));
    memset(&vbox, 0, sizeof(vbox));
    vbox.type = NODE_VBOX;
    vbox.list = ctx.vbox_list;
    vbox.width = FROM_DOUBLE(ctx.max_w);
    vbox.height = FROM_DOUBLE(ctx.first_h);
    vbox.depth = FROM_DOUBLE(ctx.top - ctx.first_h);
    vbox.vertical_scale = 1.0f;
    m_put(h_vbox, &vbox);

    para = (RetexParagraph *)malloc(sizeof(RetexParagraph));
    if (!para) {
        node_free_tree(h_vbox);
        free(ctx.meta);
        return NULL;
    }
    para->vbox_handle = h_vbox;
    para->width = ctx.max_w;
    para->height = ctx.top;
    para->first_line_height = ctx.first_h;
    para->selection_start = -1;
    para->selection_end = -1;
    para->lines = ctx.meta;
    para->nlines = ctx.n;
    return para;
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

int retex_paragraph_get_caret_pos(RetexParagraph *para, int byte_offset,
                                  double *x, double *y, double *height) {
    int idx, i, vlist, p;
    RetexLine *ln;
    Node *vbox, *hbox, *cn;
    double cur = 0, fx = 0;
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

    {
        double last_text_x = 0;
        m_foreach(hbox->list, p, cn) {
            Scaled eff = plain_eff_width(hbox, cn);
            if (cn->source_offset >= 0) {
                if (byte_offset <= cn->source_offset) { fx = cur; found = 1; break; }
                if (byte_offset < cn->source_offset + cn->byte_len) { fx = cur; found = 1; break; }
                last_text_x = cur + TO_DOUBLE(eff);
            }
            cur += TO_DOUBLE(eff);
        }
        if (!found) fx = last_text_x;
    }

    if (x) *x = fx;
    if (y) *y = ln->y;
    if (height) *height = ln->height;
    return 1;
}

int retex_paragraph_get_offset_at(RetexParagraph *para, double x, double y) {
    int idx, i, vlist, p;
    RetexLine *ln;
    Node *vbox, *hbox, *cn;
    double cur = 0;

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
        int seen_text = 0;
        m_foreach(hbox->list, p, cn) {
            Scaled eff = plain_eff_width(hbox, cn);
            double w = TO_DOUBLE(eff);
            if (x < cur + w) {
                if (cn->source_offset >= 0) {
                    if (x < cur + w / 2.0) return cn->source_offset;
                    return cn->source_offset + cn->byte_len;
                }
                return seen_text ? ln->end_byte : ln->start_byte;
            }
            if (cn->source_offset >= 0) seen_text = 1;
            cur += w;
        }
    }
    return ln->end_byte;
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
