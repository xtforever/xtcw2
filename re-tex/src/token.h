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
    TOK_COMMENT,
    TOK_HARD_BREAK
} TokenType;

typedef struct {
    TokenType type;
    int char_code; // For TOK_CHAR: a full Unicode code point
    int cmd_name;  // Handle to string for TOK_COMMAND (using memc s_cstr)
    int source_offset; // Byte offset in original string
    int byte_len;      // Length in bytes of this token's source text (1..4)
} Token;

// Tokenizes a TeX-flavored string buffer into a memc list of Tokens.
// UTF-8 is decoded to full code points; ASCII TeX syntax is retained.
int tokenize(const char *input);

// Tokenizes a plain text string buffer into a memc list of Tokens.
// No TeX interpretation: every character is literal, spaces become
// TOK_SPACE, and '\n' becomes TOK_HARD_BREAK.
int tokenize_plain(const char *input);

// Frees a list of tokens and its contents
void token_list_free(int token_list_handle);

// Debug helper
void token_dump(int token_list_handle);

#endif // TOKEN_H
