#include "token.h"
#include "mls.h"
#include "m_tool.h"
#include "utf8.h"
#include <stdio.h>

/* ASCII-only predicates: code points above 127 must never be passed to
 * <ctype.h>, which is undefined for values outside unsigned-char range. */
static int is_ascii_alpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int is_ascii_space(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

int tokenize(const char *input) {
    int list = m_create(10, sizeof(Token));
    const char *p = input;
    const char *start_p = input;

    while (*p) {
        Token tok = {0};
        tok.source_offset = (int)(p - start_p);
        tok.byte_len = 1;

        if (*p == '\\') {
            const char *cmd_start = p;
            p++; // Skip '\'
            if (!*p) break; // End of string after '\'

            // Check for \n and \t escapes
            if (*p == 'n') {
                tok.type = TOK_CHAR;
                tok.char_code = '\n';
                p++;
                tok.byte_len = (int)(p - cmd_start);
            } else if (*p == 't') {
                tok.type = TOK_CHAR;
                tok.char_code = '\t';
                p++;
                tok.byte_len = (int)(p - cmd_start);
            } else if (!is_ascii_alpha((unsigned char)*p)) {
                tok.type = TOK_COMMAND;
                tok.cmd_name = s_printf(0, 0, "%c", *p);
                p++;
            } else {
                // Control word (sequence of letters)
                tok.type = TOK_COMMAND;
                int start = s_printf(0, 0, "");
                while (*p && is_ascii_alpha((unsigned char)*p)) {
                    char tmp[2] = {*p, 0};
                    s_app(start, tmp, NULL);
                    p++;
                }
                tok.cmd_name = start;
                const char *cmd_str = m_str(start);
                if (strcmp(cmd_str, "left") == 0) {
                    tok.type = TOK_LEFT;
                } else if (strcmp(cmd_str, "right") == 0) {
                    tok.type = TOK_RIGHT;
                } else if (strcmp(cmd_str, "par") == 0) {
                    tok.type = TOK_PAR;
                    m_free(tok.cmd_name);
                    tok.cmd_name = 0;
                }

                // TeX rule: skip spaces after control word
                while (*p && is_ascii_space((unsigned char)*p)) p++;
            }
        } else if (*p == '{') {
            tok.type = TOK_GROUP_BEGIN;
            p++;
        } else if (*p == '}') {
            tok.type = TOK_GROUP_END;
            p++;
        } else if (*p == '$') {
            tok.type = TOK_MATH_SHIFT;
            p++;
        } else if (*p == '_') {
            tok.type = TOK_SUBSCRIPT;
            p++;
        } else if (*p == '^') {
            tok.type = TOK_SUPERSCRIPT;
            p++;
        } else if (*p == '%') {
            tok.type = TOK_COMMENT;
            while (*p && *p != '\n') p++; // Skip until newline
            if (*p == '\n') p++;
            continue;
        } else if (is_ascii_space((unsigned char)*p)) {
            if (*p == '\n') {
                tok.type = TOK_PAR;
            } else {
                tok.type = TOK_SPACE;
            }
            p++;
        } else {
            // A literal character: decode a full UTF-8 code point.
            uint32_t cp;
            const char *q = p;
            int n = utf8_decode_cp(&q, &cp);
            tok.type = TOK_CHAR;
            if (n == 0) {
                tok.char_code = 0xFFFD; // replacement char for invalid UTF-8
                tok.byte_len = 1;
                p++;
            } else {
                tok.char_code = (int)cp;
                tok.byte_len = n;
                p = q;
            }
        }

        m_put(list, &tok);
    }

    return list;
}

int tokenize_plain(const char *input) {
    int list = m_create(10, sizeof(Token));
    const char *p = input;
    const char *start_p = input;

    while (*p) {
        Token tok = {0};
        tok.source_offset = (int)(p - start_p);
        tok.byte_len = 1;

        if (*p == '\n') {
            tok.type = TOK_HARD_BREAK;
            tok.char_code = '\n';
            p++;
        } else if (*p == '\r') {
            p++; // CR is dropped; a following LF produces the break
            continue;
        } else if (*p == ' ' || *p == '\t') {
            tok.type = TOK_SPACE;
            tok.char_code = (unsigned char)*p;
            p++;
        } else {
            uint32_t cp;
            const char *q = p;
            int n = utf8_decode_cp(&q, &cp);
            tok.type = TOK_CHAR;
            if (n == 0) {
                tok.char_code = 0xFFFD; // replacement char for invalid UTF-8
                tok.byte_len = 1;
                p++;
            } else {
                tok.char_code = (int)cp;
                tok.byte_len = n;
                p = q;
            }
        }

        m_put(list, &tok);
    }

    return list;
}

void token_list_free(int token_list_handle) {
    if (token_list_handle <= 0) return;
    int p; Token *t;
    m_foreach(token_list_handle, p, t) {
        if (t->type == TOK_COMMAND && t->cmd_name > 0) {
            m_free(t->cmd_name);
            t->cmd_name = 0;
        }
    }
    m_free(token_list_handle);
}

void token_dump(int token_list_handle) {
    int p; Token *t;
    m_foreach(token_list_handle, p, t) {
        switch (t->type) {
            case TOK_CHAR: printf("CHAR U+%04X\n", t->char_code); break;
            case TOK_COMMAND: printf("CMD \\%s\n", m_str(t->cmd_name)); break;
            case TOK_SPACE: printf("SPACE\n"); break;
            case TOK_PAR: printf("PAR\n"); break;
            case TOK_GROUP_BEGIN: printf("{\n"); break;
            case TOK_GROUP_END: printf("}\n"); break;
            case TOK_MATH_SHIFT: printf("$\n"); break;
            case TOK_SUBSCRIPT: printf("_\n"); break;
            case TOK_SUPERSCRIPT: printf("^\n"); break;
            case TOK_LEFT: printf("LEFT\n"); break;
            case TOK_RIGHT: printf("RIGHT\n"); break;
            case TOK_COMMENT: printf("COMMENT\n"); break;
            case TOK_HARD_BREAK: printf("HARD_BREAK\n"); break;
        }
    }
}
