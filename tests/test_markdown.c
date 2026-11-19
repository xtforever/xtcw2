/*
 * Pure unit test for the Markdown subset parser used by Wmarkdown.
 * No X11 display required.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "markdown.h"
#include "retex.h"
#include "rtable.h"
#include "conststr.h"
#include "utf8.h"
#include "utf32.h"
#include "mls.h"

static const RAtom *atoms;
static int natoms;
static const RetexParaFormat *fmts;
static int nfmts;

static int contains_text(const char *needle)
{
    int n = 0;
    uint32_t *cps;
    int found = 0;
    int i, k;

    if (!atoms || !needle)
        return 0;
    n = utf32_count_utf8(needle);
    if (n <= 0)
        return 0;
    cps = (uint32_t *)malloc((size_t)n * sizeof(uint32_t));
    assert(cps != NULL);
    assert(utf32_from_utf8(needle, cps, n) == n);

    for (i = 0; i + n <= natoms; i++) {
        for (k = 0; k < n; k++)
            if (atoms[i + k].kind != RATOM_CHAR ||
                atoms[i + k].cp != cps[k])
                break;
        if (k == n) {
            found = 1;
            break;
        }
    }
    free(cps);
    return found;
}

static int text_index(const char *needle)
{
    int n = 0;
    uint32_t *cps;
    int found = -1;
    int i, k;

    if (!atoms || !needle)
        return -1;
    n = utf32_count_utf8(needle);
    if (n <= 0)
        return -1;
    cps = (uint32_t *)malloc((size_t)n * sizeof(uint32_t));
    assert(cps != NULL);
    assert(utf32_from_utf8(needle, cps, n) == n);
    for (i = 0; i + n <= natoms; i++) {
        for (k = 0; k < n; k++)
            if (atoms[i + k].kind != RATOM_CHAR ||
                atoms[i + k].cp != cps[k])
                break;
        if (k == n) {
            found = i;
            break;
        }
    }
    free(cps);
    return found;
}

static void parse(const char *src)
{
    MarkdownStyle st;
    MarkdownDoc *doc;

    memset(&st, 0, sizeof(st));
    st.font_face = "Sans";
    st.font_size = 14.0;
    st.mono_face = "Mono";
    st.default_align = RETEX_ALIGN_JUSTIFY;

    doc = markdown_parse(src, &st);
    atoms = markdown_atoms(doc, &natoms);
    fmts = markdown_formats(doc, &nfmts);
    /* Leak the doc deliberately: the borrowed views stay valid for the
     * duration of each check; the next parse starts from fresh input. */
    (void)doc;
}

int main(void)
{
    int i, starts[32];
    int para_count;

    m_init();
    conststr_init();

    /* empty input */
    assert(markdown_parse("", NULL) == NULL);

    parse(
        "# Heading one\n"
        "\n"
        "A **bold** and *italic* paragraph with `code` and a "
        "[link](https://example.com).\n"
        "\n"
        "- first item\n"
        "- second item\n"
        "\n"
        "1. ordered one\n"
        "2. ordered two\n"
        "\n"
        "> quoted text\n"
        "\n"
        "```\n"
        "int main(void) { return 0; }\n"
        "```\n");

    assert(atoms != NULL);
    assert(natoms > 0);
    assert(nfmts >= 8);

    /* Heading is bold and larger than the base font. */
    i = text_index("Heading one");
    assert(i >= 0);
    assert((atoms[i].look.style & 1) != 0);
    assert(atoms[i].look.size > 14.0f);
    assert(fmts[0].align == RETEX_ALIGN_LEFT);
    assert(fmts[0].space_before > 0.0);

    /* Ordinary paragraph: justified, bold/italic/code runs are preserved. */
    i = text_index("bold");
    assert(i >= 0);
    assert((atoms[i].look.style & 1) != 0);
    i = text_index("italic");
    assert(i >= 0);
    assert((atoms[i].look.style & 2) != 0);
    i = text_index("code");
    assert(i >= 0);
    assert(atoms[i].look.face == conststr_lookup_c("Mono"));
    assert(fmts[1].align == RETEX_ALIGN_JUSTIFY);

    /* Link label is shown, URL is not. */
    assert(contains_text("link"));
    assert(!contains_text("example.com"));

    /* List bullet and hanging indent. */
    assert(contains_text("first item"));
    i = text_index("first item");
    assert(i > 0);
    assert(atoms[i - 1].cp == ' ');
    assert(atoms[i - 2].cp == ' ');
    assert(atoms[i - 3].cp == 0x2022);
    assert(fmts[2].align == RETEX_ALIGN_LEFT);
    assert(fmts[2].indent_left > 0.0);
    assert(fmts[2].first_line < 0.0);

    /* Ordered list keeps the number and the hanging indent. */
    assert(contains_text("1. ordered one"));
    assert(fmts[4].indent_left > fmts[2].indent_left);

    /* Quote is indented. */
    assert(contains_text("quoted text"));
    assert(fmts[5].indent_left > 0.0);

    /* Fenced code keeps its text and uses the mono face. */
    i = text_index("int main");
    assert(i >= 0);
    assert(atoms[i].look.face == conststr_lookup_c("Mono"));

    /* Paragraph indexing agrees with retex. */
    para_count = retex_paragraphs(atoms, natoms, starts, 32);
    assert(para_count == nfmts);

    /* Inline markers must not leak into the visible text. */
    assert(!contains_text("**"));
    assert(!contains_text("```"));

    printf("test_markdown passed\n");
    return 0;
}
