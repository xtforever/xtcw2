#include "token.h"
#include "mls.h"
#include "m_tool.h"
#include <ctype.h>
#include <stdio.h>

static int is_alpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int tokenize(const char *input) {
    int list = m_create(10, sizeof(Token));
    const char *p = input;
    const char *start_p = input;
    
    while (*p) {
        Token tok = {0};
        tok.source_offset = p - start_p;
        
        if (*p == '\\') {
            const char *cmd_start = p;
            p++; // Skip '\'
            if (!*p) break; // End of string after '\'
            
            // Check for \n and \t escapes
            if (*p == 'n') {
                tok.type = TOK_CHAR;
                tok.char_code = '\n';
                p++;
            } else if (*p == 't') {
                tok.type = TOK_CHAR;
                tok.char_code = '\t';
                p++;
            } else if (!is_alpha(*p)) {
                tok.type = TOK_COMMAND;
                tok.cmd_name = s_printf(0, 0, "%c", *p);
                p++;
            } else {
                // Control word (sequence of letters)
                tok.type = TOK_COMMAND;
                int start = s_printf(0, 0, "");
                while (*p && is_alpha(*p)) {
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
                while (*p && isspace(*p)) p++;
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
        } else if (isspace(*p)) {
            if (*p == '\n') {
                tok.type = TOK_PAR;
            } else {
                tok.type = TOK_SPACE;
            }
            p++;
        } else {
            tok.type = TOK_CHAR;
            tok.char_code = (unsigned char)*p;
            p++;
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
            case TOK_CHAR: printf("CHAR '%c'\n", t->char_code); break;
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
        }
    }
}
