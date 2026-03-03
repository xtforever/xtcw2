#include "../src/token.h"
#include "mls.h"
#include "m_tool.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_tokenize_basic() {
    m_init();
    conststr_init();
    
    const char *input = "Hello {\\bf World} $x$";
    int list = tokenize(input);
    
    // Check length
    // H, e, l, l, o, SPACE, {, \bf, SPACE (skipped?), W, o, r, l, d, }, SPACE, $, x, $
    // tokenize implementation skips spaces after control word "\bf"
    // So: H e l l o SP { \bf W o r l d } SP $ x $
    //     1 2 3 4 5 6  7 8   9 10 11 12 13 14 15 16 17 18
    
    // Let's verify specific tokens
    Token *t = (Token*)mls(list, 0);
    assert(t->type == TOK_CHAR && t->char_code == 'H');
    
    t = (Token*)mls(list, 5);
    assert(t->type == TOK_SPACE);
    
    t = (Token*)mls(list, 6);
    assert(t->type == TOK_GROUP_BEGIN);
    
    t = (Token*)mls(list, 7);
    assert(t->type == TOK_COMMAND);
    assert(strcmp(m_str(t->cmd_name), "bf") == 0);
    
    // The space after \bf should be eaten
    t = (Token*)mls(list, 8);
    assert(t->type == TOK_CHAR && t->char_code == 'W');
    
    printf("test_tokenize_basic passed\n");
    
    m_free(list);
    conststr_free();
    m_destruct();
}

int main() {
    test_tokenize_basic();
    return 0;
}