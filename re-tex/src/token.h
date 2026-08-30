#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOK_CHAR,
    TOK_COMMAND,
    TOK_SPACE,
    TOK_PAR,
    TOK_GROUP_BEGIN,
    TOK_GROUP_END,
    TOK_MATH_SHIFT,
    TOK_SUBSCRIPT,
    TOK_SUPERSCRIPT,
    TOK_LEFT,
    TOK_RIGHT,
    TOK_COMMENT
} TokenType;

typedef struct {
    TokenType type;
    int char_code; // For TOK_CHAR
    int cmd_name;  // Handle to string for TOK_COMMAND (using memc s_cstr)
    int source_offset; // Byte offset in original string
} Token;

// Tokenizes a string buffer into a memc list of Tokens
int tokenize(const char *input);

// Frees a list of tokens and its contents
void token_list_free(int token_list_handle);

// Debug helper
void token_dump(int token_list_handle);

#endif // TOKEN_H
