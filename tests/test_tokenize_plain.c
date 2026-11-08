#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "mls.h"
#include "token.h"

static Token *tk(int list, int i)
{
    return (Token *)mls(list, i);
}

static void test_plain_ascii(void)
{
    int l = tokenize_plain("ab\ncd");

    assert(m_len(l) == 5);
    assert(tk(l, 0)->type == TOK_CHAR && tk(l, 0)->char_code == 'a');
    assert(tk(l, 0)->byte_len == 1 && tk(l, 0)->source_offset == 0);
    assert(tk(l, 1)->type == TOK_CHAR && tk(l, 1)->char_code == 'b');
    assert(tk(l, 1)->source_offset == 1);
    assert(tk(l, 2)->type == TOK_HARD_BREAK && tk(l, 2)->source_offset == 2);
    assert(tk(l, 3)->type == TOK_CHAR && tk(l, 3)->char_code == 'c');
    assert(tk(l, 3)->source_offset == 3);
    assert(tk(l, 4)->type == TOK_CHAR && tk(l, 4)->char_code == 'd');
    assert(tk(l, 4)->source_offset == 4);
    token_list_free(l);
    printf("test_plain_ascii passed\n");
}

static void test_plain_utf8(void)
{
    const char *s = "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80"; /* A é 中 😀 */
    int l = tokenize_plain(s);

    assert(m_len(l) == 4);
    assert(tk(l, 0)->char_code == 0x41 && tk(l, 0)->byte_len == 1);
    assert(tk(l, 0)->source_offset == 0);
    assert(tk(l, 1)->char_code == 0xE9 && tk(l, 1)->byte_len == 2);
    assert(tk(l, 1)->source_offset == 1);
    assert(tk(l, 2)->char_code == 0x4E2D && tk(l, 2)->byte_len == 3);
    assert(tk(l, 2)->source_offset == 3);
    assert(tk(l, 3)->char_code == 0x1F600 && tk(l, 3)->byte_len == 4);
    assert(tk(l, 3)->source_offset == 6);
    token_list_free(l);
    printf("test_plain_utf8 passed\n");
}

static void test_plain_is_literal(void)
{
    /* Backslash, percent, braces, caret, underscore, dollar must all be
     * literal characters in plain mode. */
    const char *s = "\\%{}^_$";
    int l = tokenize_plain(s);
    int i;

    assert(m_len(l) == 7);
    for (i = 0; i < 7; i++) {
        assert(tk(l, i)->type == TOK_CHAR);
        assert(tk(l, i)->char_code == (unsigned char)s[i]);
    }
    token_list_free(l);

    /* spaces become TOK_SPACE, tabs too */
    l = tokenize_plain("a b\tc");
    assert(m_len(l) == 5);
    assert(tk(l, 0)->type == TOK_CHAR);
    assert(tk(l, 1)->type == TOK_SPACE);
    assert(tk(l, 2)->type == TOK_CHAR);
    assert(tk(l, 3)->type == TOK_SPACE);
    assert(tk(l, 4)->type == TOK_CHAR);
    token_list_free(l);

    /* CRLF: CR dropped, LF produces the break */
    l = tokenize_plain("a\r\nb");
    assert(m_len(l) == 3);
    assert(tk(l, 0)->type == TOK_CHAR);
    assert(tk(l, 1)->type == TOK_HARD_BREAK);
    assert(tk(l, 2)->type == TOK_CHAR);
    token_list_free(l);
    printf("test_plain_is_literal passed\n");
}

static void test_tex_utf8(void)
{
    int l;

    /* UTF-8 decodes to one token per code point */
    l = tokenize("\xC3\xA9");
    assert(m_len(l) == 1);
    assert(tk(l, 0)->type == TOK_CHAR);
    assert(tk(l, 0)->char_code == 0xE9);
    assert(tk(l, 0)->byte_len == 2);
    token_list_free(l);

    /* ASCII TeX syntax is still interpreted */
    l = tokenize("a\nb"); /* literal newline -> TOK_PAR */
    assert(m_len(l) == 3);
    assert(tk(l, 0)->type == TOK_CHAR);
    assert(tk(l, 1)->type == TOK_PAR);
    assert(tk(l, 2)->type == TOK_CHAR);
    token_list_free(l);

    l = tokenize("{x}");
    assert(tk(l, 0)->type == TOK_GROUP_BEGIN);
    assert(tk(l, 1)->type == TOK_CHAR);
    assert(tk(l, 2)->type == TOK_GROUP_END);
    token_list_free(l);

    l = tokenize("\\par");
    assert(m_len(l) == 1 && tk(l, 0)->type == TOK_PAR);
    token_list_free(l);

    l = tokenize("\\&");
    assert(m_len(l) == 1 && tk(l, 0)->type == TOK_COMMAND);
    assert(strcmp(m_str(tk(l, 0)->cmd_name), "&") == 0);
    token_list_free(l);

    /* % comment swallows to end of line (and the newline) */
    l = tokenize("a%comment\nb");
    assert(m_len(l) == 2);
    assert(tk(l, 0)->type == TOK_CHAR && tk(l, 0)->char_code == 'a');
    assert(tk(l, 1)->type == TOK_CHAR && tk(l, 1)->char_code == 'b');
    token_list_free(l);
    printf("test_tex_utf8 passed\n");
}

int main(void)
{
    m_init();
    test_plain_ascii();
    test_plain_utf8();
    test_plain_is_literal();
    test_tex_utf8();
    m_destruct();
    printf("All tokenize tests passed\n");
    return 0;
}
