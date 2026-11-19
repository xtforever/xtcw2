#include "linebreak.h"
#include "builder.h"
#include "shaping.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <math.h>

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

/* ------------------------------------------------------------------ */
/* Knuth-Plass style paragraph breaking                                */
/* ------------------------------------------------------------------ */

typedef struct {
    int type;          /* 0 = box, 1 = glue, 2 = penalty */
    int node_index;
    Scaled width;
    Scaled stretch;
    Scaled shrink;
    int stretch_order;
    int shrink_order;
    int penalty;
    Scaled hyphen_width;
    int hyphen_list;
} KpItem;

static double kp_badness(Scaled natural, Scaled target, Scaled stretch,
                         Scaled shrink, Scaled emergencystretch,
                         int *feasible)
{
    Scaled diff = scaled_sub(target, natural);
    double ratio, badness;

    *feasible = 1;
    if (diff == 0)
        return 0.0;

    if (diff > 0) {
        Scaled st = stretch;
        if (st <= 0)
            st = emergencystretch;
        if (st <= 0) {
            *feasible = 0;
            return 100000.0;
        }
        ratio = (double)diff / (double)st;
        badness = 100.0 * ratio * ratio * ratio;
        if (badness > 100000.0)
            badness = 100000.0;
    } else {
        if (shrink <= 0) {
            *feasible = 0;
            return 100000.0;
        }
        ratio = (double)(-diff) / (double)shrink;
        if (ratio <= 1.0) {
            badness = 100.0 * ratio * ratio * ratio;
        } else {
            *feasible = 0;
            badness = 10000.0 + 10000.0 * (ratio - 1.0);
        }
    }
    return badness;
}

static void kp_move_node(Node *dst, Node *src)
{
    *dst = *src;
    if (src->type == NODE_HBOX || src->type == NODE_VBOX)
        src->list = 0;
    else if (src->type == NODE_SVG)
        src->data = 0;
    else if (src->type == NODE_CHAR)
        src->shaped = NULL;
    else if (src->type == NODE_PENALTY)
        src->list = 0;
}

static void scale_word_box(Node *n, double s);

static void kp_move_hyphen_list(int line_nodes, Node *penalty, double scale)
{
    int hl, p;
    Node *hn;

    if (!penalty || penalty->type != NODE_PENALTY || penalty->list <= 0)
        return;
    hl = penalty->list;
    m_foreach(hl, p, hn) {
        Node copy;
        kp_move_node(&copy, hn);
        if (scale != 1.0)
            scale_word_box(&copy, scale);
        m_put(line_nodes, &copy);
    }
    node_list_free(hl);
    penalty->list = 0;
}

static Node *first_char_node(Node *n)
{
    int p;
    Node *cn;
    if (!n) return NULL;
    if (n->type == NODE_CHAR)
        return n->shaped_hidden ? NULL : n;
    if (n->type == NODE_HBOX || n->type == NODE_VBOX) {
        m_foreach(n->list, p, cn) {
            Node *r = first_char_node(cn);
            if (r) return r;
        }
    }
    return NULL;
}

static Node *last_char_node(Node *n)
{
    int p;
    Node *cn, *last = NULL;
    if (!n) return NULL;
    if (n->type == NODE_CHAR)
        return n->shaped_hidden ? NULL : n;
    if (n->type == NODE_HBOX || n->type == NODE_VBOX) {
        m_foreach(n->list, p, cn) {
            Node *r = last_char_node(cn);
            if (r) last = r;
        }
    }
    return last;
}

static double protrusion_left_factor(uint32_t cp)
{
    switch (cp) {
        case '(': case '[': case '{':
        case 0x00AB: case 0x201C: case 0x2018:
            return 0.5;
        default:
            return 0.0;
    }
}

static double protrusion_right_factor(uint32_t cp)
{
    switch (cp) {
        case ',': case '.': case ';': case ':': case '!': case '?':
        case ')': case ']': case '}':
        case 0x00BB: case 0x201D: case 0x2019:
        case '-': case 0x2013: case 0x2014:
            return 0.5;
        default:
            return 0.0;
    }
}

static double line_protrusion_left(int node_list, KpItem *items,
                                   int start, int end)
{
    int i;
    for (i = start; i < end; i++) {
        if (items[i].type == 0) {
            Node *n = (Node *)mls(node_list, items[i].node_index);
            Node *c = first_char_node(n);
            if (c)
                return protrusion_left_factor((uint32_t)c->data) *
                       TO_DOUBLE(c->width);
            break;
        }
    }
    return 0.0;
}

static double line_protrusion_right(int node_list, KpItem *items,
                                    int start, int end)
{
    int i;
    for (i = end - 1; i >= start; i--) {
        if (items[i].type == 0) {
            Node *n = (Node *)mls(node_list, items[i].node_index);
            Node *c = last_char_node(n);
            if (c)
                return protrusion_right_factor((uint32_t)c->data) *
                       TO_DOUBLE(c->width);
            break;
        }
    }
    return 0.0;
}

static void scale_word_box(Node *n, double s)
{
    int p;
    Node *cn;
    if (!n || s == 1.0) return;
    if (n->type == NODE_CHAR) {
        if (n->shaped) {
            ShapedCluster *c = (ShapedCluster *)n->shaped;
            c->x_scale *= s;
            c->width *= s;
            n->width = FROM_DOUBLE(c->width);
        } else {
            n->width = FROM_DOUBLE(TO_DOUBLE(n->width) * s);
        }
        return;
    }
    if (n->type == NODE_HBOX || n->type == NODE_VBOX) {
        double w = 0.0;
        m_foreach(n->list, p, cn) {
            scale_word_box(cn, s);
            w += TO_DOUBLE(cn->width);
        }
        n->width = FROM_DOUBLE(w);
    }
}

int line_break_kp(int node_list, const LineBreakParams *params)
{
    int n, i, j, nb, *bp = NULL, *prev = NULL;
    double *best = NULL, *line_scale = NULL;
    KpItem *items = NULL;
    Scaled *pw = NULL, *pst = NULL, *psh = NULL, *pg = NULL, *pgl = NULL;
    int lines = m_create(5, sizeof(Node));
    int last_forced = 0;
    Glue parfill;

    if (!params || node_list <= 0)
        return lines;

    n = m_len(node_list);
    if (n <= 0)
        return lines;

    items = (KpItem *)calloc((size_t)n, sizeof(KpItem));
    pw = (Scaled *)calloc((size_t)n + 1, sizeof(Scaled));
    pst = (Scaled *)calloc((size_t)n + 1, sizeof(Scaled));
    psh = (Scaled *)calloc((size_t)n + 1, sizeof(Scaled));
    pg = (Scaled *)calloc((size_t)n + 1, sizeof(Scaled));
    pgl = (Scaled *)calloc((size_t)n + 1, sizeof(Scaled));
    if (!items || !pw || !pst || !psh || !pg || !pgl)
        goto done;

    for (i = 0; i < n; i++) {
        Node *node = (Node *)mls(node_list, i);
        KpItem *it = &items[i];
        it->node_index = i;
        if (node->type == NODE_GLUE) {
            it->type = 1;
            it->width = node->glue_spec.width;
            it->stretch = node->glue_spec.stretch;
            it->shrink = node->glue_spec.shrink;
            it->stretch_order = node->glue_spec.stretch_order;
            it->shrink_order = node->glue_spec.shrink_order;
        } else if (node->type == NODE_PENALTY) {
            it->type = 2;
            it->penalty = node->data;
            it->hyphen_width = node->width;
            it->hyphen_list = node->list;
        } else {
            it->type = 0;
            it->width = node->width;
        }
        pw[i + 1] = scaled_add(pw[i], it->width);
        pst[i + 1] = scaled_add(pst[i], it->stretch);
        psh[i + 1] = scaled_add(psh[i], it->shrink);
        pg[i + 1] = scaled_add(pg[i], (it->type == 0) ? it->width : 0);
        pgl[i + 1] = scaled_add(pgl[i], (it->type == 1) ? it->width : 0);
    }

    bp = (int *)calloc((size_t)n + 2, sizeof(int));
    if (!bp)
        goto done;
    nb = 0;
    bp[nb++] = -1;
    for (i = 0; i < n; i++) {
        if (items[i].type == 1 || items[i].type == 2)
            bp[nb++] = i;
    }
    bp[nb++] = n;

    best = (double *)malloc((size_t)nb * sizeof(double));
    line_scale = (double *)malloc((size_t)nb * sizeof(double));
    prev = (int *)malloc((size_t)nb * sizeof(int));
    if (!best || !line_scale || !prev)
        goto done;
    for (i = 0; i < nb; i++) {
        best[i] = 1e300;
        line_scale[i] = 1.0;
        prev[i] = -1;
    }
    best[0] = 0.0;

    for (j = 1; j < nb; j++) {
        int bj = bp[j];
        int is_forced = 0;
        int is_penalty = (bj < n && items[bj].type == 2);
        int pen = 0;

        if (is_penalty) {
            pen = items[bj].penalty;
            if (pen >= 10000)
                is_forced = 1;
        }

        for (i = last_forced; i < j; i++) {
            int start, end, first_line, ragged;
            Scaled natural, stretch, shrink, indent, target, emerg;
            double badness, dem, scale = 1.0;
            int feasible;

            if (best[i] >= 1e299)
                continue;
            start = bp[i] + 1;
            end = bj;
            if (start >= end)
                continue;

            first_line = (i == 0);
            indent = first_line ? params->first_indent : params->rest_indent;
            if (indent < 0) indent = 0;
            target = scaled_sub(params->width, indent);
            target = scaled_sub(target, params->right_indent);
            target = scaled_sub(target, params->left_skip.width);
            target = scaled_sub(target, params->right_skip.width);

            ragged = (params->right_skip.stretch_order == ORDER_FIL ||
                      params->left_skip.stretch_order == ORDER_FIL);
            {
                Scaled glyph_w = scaled_sub(pg[end], pg[start]);
                Scaled space_w = scaled_sub(pgl[end], pgl[start]);
                double p_left = line_protrusion_left(node_list, items,
                                                     start, end);
                double p_right = is_penalty ? 0.0 :
                    line_protrusion_right(node_list, items, start, end);

                if (!ragged && glyph_w > 0) {
                    double desired = (TO_DOUBLE(target) -
                                      TO_DOUBLE(space_w)) /
                                     TO_DOUBLE(glyph_w);
                    if (desired < 0.98) desired = 0.98;
                    if (desired > 1.02) desired = 1.02;
                    scale = desired;
                }
                target = scaled_add(target, FROM_DOUBLE(p_left + p_right));
                natural = scaled_add(FROM_DOUBLE(TO_DOUBLE(glyph_w) * scale),
                                     space_w);
                if (is_penalty && items[bj].hyphen_width > 0)
                    natural = scaled_add(natural, items[bj].hyphen_width);
            }
            stretch = scaled_sub(pst[end], pst[start]);
            shrink = scaled_sub(psh[end], psh[start]);
            if (ragged)
                stretch = FROM_INT(1000000);

            emerg = params->emergencystretch;
            badness = kp_badness(natural, target, stretch, shrink, emerg,
                                 &feasible);
            if (!feasible && !is_forced) {
                /* Allow it as a last resort, but prefer feasible breaks and
                 * among infeasible ones the least overfull/loose line. */
                dem = 1e12 + badness * 1e6;
            } else {
                dem = (10.0 + badness) * (10.0 + badness);
                if (!is_forced)
                    dem += (double)pen * (double)pen;
                if (!feasible)
                    dem += 1e9;
            }
            dem += 5000.0 * (scale - 1.0) * (scale - 1.0);
            dem += best[i];
            if (dem < best[j]) {
                best[j] = dem;
                prev[j] = i;
                line_scale[j] = scale;
            }
        }

        if (is_forced)
            last_forced = j;
    }

    /* Reconstruct breakpoints from the end. */
    {
        int *chosen = (int *)malloc((size_t)nb * sizeof(int));
        int nc = 0;
        if (!chosen)
            goto done;
        for (j = nb - 1; j > 0 && prev[j] >= 0; j = prev[j])
            chosen[nc++] = j;
        chosen[nc++] = 0;

        for (i = nc - 1; i > 0; i--) {
            int bi = chosen[i];
            int bj = chosen[i - 1];
            int start = bp[bi] + 1;
            int end = bp[bj];
            int t, line_nodes;
            Node *last_pen = NULL;
            Glue right_skip = params->right_skip;
            double scale = line_scale[bj];
            double p_left = line_protrusion_left(node_list, items, start, end);
            double p_right = (bp[bj] < n && items[bp[bj]].type == 2) ? 0.0 :
                line_protrusion_right(node_list, items, start, end);
            Scaled hbox_target = scaled_add(params->width,
                                            FROM_DOUBLE(p_right));

            line_nodes = m_create(10, sizeof(Node));
            node_create_glue(line_nodes, params->left_skip, 0);
            {
                Scaled indent = (bi == 0) ? params->first_indent
                                          : params->rest_indent;
                if (indent > 0)
                    node_create_kern(line_nodes, indent, 0);
            }
            if (p_left > 0)
                node_create_kern(line_nodes, FROM_DOUBLE(-p_left), 0);
            for (t = start; t < end; t++) {
                Node *src = (Node *)mls(node_list, items[t].node_index);
                Node copy;
                if (src->type == NODE_PENALTY)
                    continue;
                kp_move_node(&copy, src);
                scale_word_box(&copy, scale);
                m_put(line_nodes, &copy);
            }
            if (bp[bj] < n && items[bp[bj]].type == 2) {
                last_pen = (Node *)mls(node_list, items[bp[bj]].node_index);
                kp_move_hyphen_list(line_nodes, last_pen, scale);
            }

            if (bp[bj] == n && params->left_skip.stretch_order != ORDER_FIL) {
                parfill = glue_create(0, FROM_INT(1000), ORDER_FIL, 0,
                                      ORDER_NORMAL);
                node_create_glue(line_nodes, parfill, 0);
            }
            node_create_glue(line_nodes, right_skip, 0);
            {
                Node hbox = node_pack_hbox(line_nodes, hbox_target);
                m_put(lines, &hbox);
            }
        }
        free(chosen);
    }

done:
    free(items);
    free(pw);
    free(pst);
    free(psh);
    free(pg);
    free(pgl);
    free(bp);
    free(best);
    free(line_scale);
    free(prev);
    return lines;
}
