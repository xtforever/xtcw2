#include "builder.h"
#include "token.h"
#include "math_node.h"
#include "linebreak.h"
#include "mls.h"
#include "m_tool.h"

/* Workaround for redefinition error in conststr.h vs m_tool.h */
#define s_cstr s_cstr_hidden
#define s_mstr s_mstr_hidden
#include "conststr.h"
#undef s_cstr
#undef s_mstr

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static NoadType classify_char(int c) {
    if (c == '+' || c == '-' || c == '*' || c == '/') return NOAD_BIN;
    if (c == '=' || c == '<' || c == '>' || c == ':') return NOAD_REL;
    if (c == '(' || c == '[' || c == '{') return NOAD_OPEN;
    if (c == ')' || c == ']' || c == '}') return NOAD_CLOSE;
    if (c == ',' || c == ';' || c == '!') return NOAD_PUNCT;
    return NOAD_ORD;
}

typedef struct {
    char face[64];
    Scaled base_size;
    float scale;
    int style;
    int is_math_mode;
} FontState;

static Scaled get_current_size(FontState *fs) {
    return (Scaled)((float)fs->base_size * fs->scale);
}

static int parse_mlist(int token_list, int *p_idx);

static void math_field_set_token(MathField *f, Token *t, int token_list, int *p_idx) {
    if (t->type == TOK_CHAR) {
        f->type = MATH_TYPE_CHAR;
        f->data.c.char_code = t->char_code;
    } else if (t->type == TOK_GROUP_BEGIN) {
        (*p_idx)++;
        int sub = parse_mlist(token_list, p_idx);
        f->type = MATH_TYPE_MLIST;
        f->data.mlist = sub;
    }
}

static int parse_mlist(int token_list, int *p_idx) {
    int mlist = m_create(5, sizeof(Noad));
    int len = m_len(token_list);
    
    while (*p_idx < len) {
        Token *t = (Token*)mls(token_list, *p_idx);
        
        if (t->type == TOK_MATH_SHIFT || t->type == TOK_GROUP_END) {
            // End of math or group
            return mlist;
        }
        
        if (t->type == TOK_CHAR || t->type == TOK_GROUP_BEGIN) {
            noad_create(mlist, t->type == TOK_CHAR ? classify_char(t->char_code) : NOAD_ORD);
            Noad *n = (Noad*)mls(mlist, m_len(mlist) - 1);
            math_field_set_token(&n->nucleus, t, token_list, p_idx);
        } else if (t->type == TOK_COMMAND) {
            const char *cmd = m_str(t->cmd_name);
            if (strcmp(cmd, "frac") == 0) {
                noad_create(mlist, NOAD_FRACTION);
                Noad *n = (Noad*)mls(mlist, m_len(mlist) - 1);
                
                // Expect two arguments
                (*p_idx)++;
                if (*p_idx < len) {
                    Token *num_t = (Token*)mls(token_list, *p_idx);
                    math_field_set_token(&n->numerator, num_t, token_list, p_idx);
                }
                (*p_idx)++;
                if (*p_idx < len) {
                    Token *den_t = (Token*)mls(token_list, *p_idx);
                    math_field_set_token(&n->denominator, den_t, token_list, p_idx);
                }
            }
        } else if (t->type == TOK_SUBSCRIPT || t->type == TOK_SUPERSCRIPT) {
            if (m_len(mlist) == 0) {
                // Attach to an empty Ord noad if nothing preceding
                noad_create(mlist, NOAD_ORD);
            }
            Noad *last_n = (Noad*)mls(mlist, m_len(mlist) - 1);
            MathField *target = (t->type == TOK_SUBSCRIPT) ? &last_n->subscr : &last_n->supscr;
            
            int next_t_idx = *p_idx + 1;
            if (next_t_idx < len) {
                Token *next_t = (Token*)mls(token_list, next_t_idx);
                math_field_set_token(target, next_t, token_list, &next_t_idx);
                *p_idx = next_t_idx;
            }
        } else if (t->type == TOK_LEFT) {
            noad_create(mlist, NOAD_LEFT);
            Noad *ln = (Noad*)mls(mlist, m_len(mlist) - 1);
            (*p_idx)++;
            if (*p_idx < len) {
                Token *delim_t = (Token*)mls(token_list, *p_idx);
                if (delim_t->type == TOK_CHAR) {
                    ln->nucleus.type = MATH_TYPE_CHAR;
                    ln->nucleus.data.c.char_code = delim_t->char_code;
                }
            }
            
            // Parse content until matching \right
            (*p_idx)++;
            int sub_mlist = parse_mlist(token_list, p_idx);
            
            // Check for \right
            if (*p_idx < len) {
                Token *rt = (Token*)mls(token_list, *p_idx);
                if (rt->type == TOK_RIGHT) {
                    noad_create(mlist, NOAD_INNER);
                    Noad *in = (Noad*)mls(mlist, m_len(mlist) - 1);
                    in->nucleus.type = MATH_TYPE_MLIST;
                    in->nucleus.data.mlist = sub_mlist;
                    
                    noad_create(mlist, NOAD_RIGHT);
                    Noad *rn = (Noad*)mls(mlist, m_len(mlist) - 1);
                    (*p_idx)++;
                    if (*p_idx < len) {
                        Token *delim_t = (Token*)mls(token_list, *p_idx);
                        if (delim_t->type == TOK_CHAR) {
                            rn->nucleus.type = MATH_TYPE_CHAR;
                            rn->nucleus.data.c.char_code = delim_t->char_code;
                        }
                    }
                }
            }
        } else if (t->type == TOK_RIGHT || t->type == TOK_GROUP_END) {
            // End of math, group, or left-block
            return mlist;
        }
        (*p_idx)++;
    }
    return mlist;
}

static int extract_group(int token_list, int *p_idx) {
    int sub = m_create(5, sizeof(Token));
    int depth = 1;
    int len = m_len(token_list);
    (*p_idx)++; // skip TOK_GROUP_BEGIN
    while (*p_idx < len && depth > 0) {
        Token *t = (Token*)mls(token_list, *p_idx);
        if (t->type == TOK_GROUP_BEGIN) depth++;
        else if (t->type == TOK_GROUP_END) depth--;
        if (depth > 0) {
            m_put(sub, t);
            (*p_idx)++;
        }
    }
    return sub;
}

static Glue calc_space_glue(Scaled font_size) {
    Scaled space_width = scaled_div(font_size, FROM_INT(3));
    return glue_create(space_width, space_width / 2, ORDER_NORMAL, space_width / 3, ORDER_NORMAL);
}

static Scaled parse_length(int token_list, int *p_idx, Scaled em_size) {
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
    
    // Collect non-space tokens that could form a length (including optional minus)
    int buf = s_printf(0, 0, "");
    while (end < len) {
        Token *t = (Token*)mls(token_list, end);
        if (t->type == TOK_CHAR) {
            if (t->char_code == '-' || isdigit(t->char_code) || t->char_code == '.' || isalpha(t->char_code)) {
                char tmp[2] = {(char)t->char_code, 0};
                s_app(buf, tmp, NULL);
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
        else result = FROM_DOUBLE(val); // default to pt
    }
    
    m_free(buf);
    *p_idx = end - 1; // build_hlist will increment
    return result;
}

int build_hlist(int token_list, MeasureFunc measure_func, void *measure_ctx, Scaled font_size_base, const char *font_face_base, int face_handle_base) {
    int nodes = m_create(10, sizeof(Node));
    int idx = 0;
    int len = m_len(token_list);
    
    int state_stack = m_create(5, sizeof(FontState));
    
    FontState fs;
    strncpy(fs.face, font_face_base ? font_face_base : "Sans", 63);
    fs.base_size = font_size_base;
    fs.scale = 1.0;
    fs.style = 0;
    fs.is_math_mode = 0;

    int face_handle = face_handle_base;
    Glue space_glue = calc_space_glue(get_current_size(&fs));
    
    while (idx < len) {
        Token *t = (Token*)mls(token_list, idx);
        if (t->type == TOK_CHAR) {
            Scaled cur_size = get_current_size(&fs);
            CharMetricsScaled m;
            if (measure_func) {
                m = measure_func(measure_ctx, t->char_code, cur_size, fs.style, fs.face);
            } else {
                m.width = scaled_mul(cur_size, FROM_INT(6)/10);
                m.height = scaled_mul(cur_size, FROM_INT(7)/10);
                m.depth = scaled_mul(cur_size, FROM_INT(2)/10);
            }
            node_create_char(nodes, t->char_code, m.width, m.height, m.depth, 
                                     (float)TO_DOUBLE(cur_size), fs.style, face_handle);
        } else if (t->type == TOK_SPACE) {
            node_create_glue(nodes, space_glue);
        } else if (t->type == TOK_MATH_SHIFT) {
            idx++;
            int mlist = parse_mlist(token_list, &idx);
            // Convert mlist to hlist
            int math_nodes = mlist_to_hlist(mlist, STYLE_TEXT, measure_func, measure_ctx, font_size_base, font_face_base, face_handle);
            
            // Append math_nodes to nodes
            int mn_p; Node *mn;
            m_foreach(math_nodes, mn_p, mn) {
                m_put(nodes, mn);
            }
            m_free(math_nodes);
            retex_mlist_free(mlist);
            // idx is now at MATH_SHIFT or end of list
        } else if (t->type == TOK_COMMAND) {
            const char *cmd = m_str(t->cmd_name);
            int changed = 0;
            if (strcmp(cmd, "bf") == 0) { fs.style |= 1; changed = 1; }
            else if (strcmp(cmd, "it") == 0) { fs.style |= 2; changed = 1; }
            else if (strcmp(cmd, "rm") == 0) { fs.style = 0; changed = 1; }
            else if (strcmp(cmd, "tiny") == 0) { fs.scale = 0.5; changed = 1; }
            else if (strcmp(cmd, "small") == 0) { fs.scale = 0.8; changed = 1; }
            else if (strcmp(cmd, "normalsize") == 0) { fs.scale = 1.0; changed = 1; }
            else if (strcmp(cmd, "large") == 0) { fs.scale = 1.2; changed = 1; }
            else if (strcmp(cmd, "Large") == 0) { fs.scale = 1.44; changed = 1; }
            else if (strcmp(cmd, "LARGE") == 0) { fs.scale = 1.72; changed = 1; }
            else if (strcmp(cmd, "huge") == 0) { fs.scale = 2.07; changed = 1; }
            else if (strcmp(cmd, "Huge") == 0) { fs.scale = 2.48; changed = 1; }
            else if (strcmp(cmd, "fontface") == 0) {
                idx++;
                if (idx < len) {
                    Token *arg_t = (Token*)mls(token_list, idx);
                    if (arg_t->type == TOK_GROUP_BEGIN) {
                        int start = idx + 1;
                        int depth = 1;
                        int end = start;
                        while (end < len && depth > 0) {
                            Token *tt = (Token*)mls(token_list, end);
                            if (tt->type == TOK_GROUP_BEGIN) depth++;
                            else if (tt->type == TOK_GROUP_END) depth--;
                            if (depth > 0) end++;
                        }
                        
                        int fname_buf = s_printf(0, 0, "");
                        for (int i = start; i < end; i++) {
                            Token *tt = (Token*)mls(token_list, i);
                            if (tt->type == TOK_CHAR) {
                                char tmp[2] = {(char)tt->char_code, 0};
                                s_app(fname_buf, tmp, NULL);
                            }
                        }
                        strncpy(fs.face, m_str(fname_buf), 63);
                        m_free(fname_buf);
                        idx = end;
                        changed = 1;
                    }
                }
            }
            else if (strcmp(cmd, "hspace") == 0) {
                idx++;
                Scaled len = parse_length(token_list, &idx, get_current_size(&fs));
                node_create_kern(nodes, len);
                Node *n = (Node*)mls(nodes, m_len(nodes) - 1);
                n->height = 0; n->depth = 0;
            }
            else if (strcmp(cmd, "vspace") == 0) {
                idx++;
                Scaled len = parse_length(token_list, &idx, get_current_size(&fs));
                // In hlist, vspace is a bit weird. It should probably be a box with height.
                node_create_rule(nodes, 0, len, 0);
            }
            else if (strcmp(cmd, "rule") == 0) {
                idx++; Scaled w = parse_length(token_list, &idx, get_current_size(&fs));
                idx++; Scaled h = parse_length(token_list, &idx, get_current_size(&fs));
                node_create_rule(nodes, w, h, 0);
            }
            else if (strcmp(cmd, "hfill") == 0) {
                Glue g = glue_create(0, FROM_INT(1), ORDER_FIL, 0, ORDER_NORMAL);
                node_create_glue(nodes, g);
            }
            else if (strcmp(cmd, "hangindent") == 0) {
                idx++;
                Scaled len = parse_length(token_list, &idx, get_current_size(&fs));
                node_create_property(nodes, PROP_HANG_INDENT, len);
            }
            else if (strcmp(cmd, "hangafter") == 0) {
                idx++;
                Scaled val = parse_length(token_list, &idx, FROM_INT(1)); // parse_length can parse integers too
                node_create_property(nodes, PROP_HANG_AFTER, TO_INT(val));
            }
            else if (strcmp(cmd, "hbox") == 0) {
                idx++;
                if (idx < len && ((Token*)mls(token_list, idx))->type == TOK_GROUP_BEGIN) {
                    int sub_tokens = extract_group(token_list, &idx);
                    int sub_hlist = build_hlist(sub_tokens, measure_func, measure_ctx, font_size_base, fs.face, face_handle);
                    Node hbox = node_pack_hbox(sub_hlist, 0); 
                    m_put(nodes, &hbox);
                    m_free(sub_tokens);
                }
            }
            else if (strcmp(cmd, "raisebox") == 0) {
                idx++;
                Scaled shift = parse_length(token_list, &idx, get_current_size(&fs));
                idx++;
                if (idx < len && ((Token*)mls(token_list, idx))->type == TOK_GROUP_BEGIN) {
                    int sub_tokens = extract_group(token_list, &idx);
                    int sub_hlist = build_hlist(sub_tokens, measure_func, measure_ctx, font_size_base, fs.face, face_handle);
                    Node hbox = node_pack_hbox(sub_hlist, 0);
                    hbox.shift = shift;
                    m_put(nodes, &hbox);
                    m_free(sub_tokens);
                }
            }
            else if (strcmp(cmd, "vcenter") == 0) {
                idx++;
                if (idx < len && ((Token*)mls(token_list, idx))->type == TOK_GROUP_BEGIN) {
                    int sub_tokens = extract_group(token_list, &idx);
                    int sub_hlist = build_hlist(sub_tokens, measure_func, measure_ctx, font_size_base, fs.face, face_handle);
                    Node hbox = node_pack_hbox(sub_hlist, 0);
                    // Center relative to the axis (approx 0.25 font size above baseline)
                    Scaled axis_height = get_current_size(&fs) / 4;
                    hbox.shift = axis_height + (hbox.depth - hbox.height) / 2;
                    m_put(nodes, &hbox);
                    m_free(sub_tokens);
                }
            }
            else if (strcmp(cmd, "parbox") == 0) {
                idx++;
                Scaled w = parse_length(token_list, &idx, get_current_size(&fs));
                idx++;
                if (idx < len && ((Token*)mls(token_list, idx))->type == TOK_GROUP_BEGIN) {
                    int sub_tokens = extract_group(token_list, &idx);
                    int sub_hlist = build_hlist(sub_tokens, measure_func, measure_ctx, get_current_size(&fs), fs.face, face_handle);
                    Glue zero = glue_zero();
                    int lines = line_break(sub_hlist, w, zero, zero, 0, 0);
                    int vbox_h = node_list_to_vbox(lines, TO_DOUBLE(get_current_size(&fs)));
                    Node *vbox = (Node*)mls(vbox_h, 0);
                    m_put(nodes, vbox);
                    m_free(vbox_h);
                    m_free(sub_tokens);
                    m_free(lines);
                    m_free(sub_hlist);
                }
            }
            else if (strcmp(cmd, "includesvg") == 0) {
                idx++;
                if (idx < len) {
                    Token *arg_t = (Token*)mls(token_list, idx);
                    if (arg_t->type == TOK_GROUP_BEGIN) {
                        int start = idx + 1;
                        int depth = 1;
                        int end = start;
                        while (end < len && depth > 0) {
                            Token *tt = (Token*)mls(token_list, end);
                            if (tt->type == TOK_GROUP_BEGIN) depth++;
                            else if (tt->type == TOK_GROUP_END) depth--;
                            if (depth > 0) end++;
                        }
                        
                        int fname_buf = s_printf(0, 0, "");
                        for (int i = start; i < end; i++) {
                            Token *tt = (Token*)mls(token_list, i);
                            if (tt->type == TOK_CHAR) {
                                char tmp[2] = {(char)tt->char_code, 0};
                                s_app(fname_buf, tmp, NULL);
                            }
                        }
                        const char *fname = m_str(fname_buf);
                        idx = end; // Points to TOK_GROUP_END

                        idx++; Scaled w = parse_length(token_list, &idx, get_current_size(&fs));
                        idx++; Scaled h = parse_length(token_list, &idx, get_current_size(&fs));
                        
                        node_create_svg(nodes, fname, w, h);
                        m_free(fname_buf);
                    }
                }
            }
            
            if (trace_level > 0) {
                printf("TOK_COMMAND: \\%s (style=%d, scale=%f)\n", cmd, fs.style, fs.scale);
            }

            if (changed) {
                face_handle = conststr_lookup_c(fs.face);
                space_glue = calc_space_glue(get_current_size(&fs));
            }
        } else if (t->type == TOK_GROUP_BEGIN) {
            m_put(state_stack, &fs);
        } else if (t->type == TOK_GROUP_END) {
            if (m_len(state_stack) > 0) {
                int last_idx = m_len(state_stack) - 1;
                fs = *(FontState*)mls(state_stack, last_idx);
                m_del(state_stack, last_idx);
                face_handle = conststr_lookup_c(fs.face);
                space_glue = calc_space_glue(get_current_size(&fs));
            }
        }
        idx++;
    }
    
    m_free(state_stack);
    return nodes;
}
