#ifndef XTCW_MARKDOWN_H
#define XTCW_MARKDOWN_H

/*
 * Small Markdown -> retex rich-text converter.
 *
 * This is intentionally a viewer subset, not a CommonMark implementation.
 * Supported block syntax:
 *   # .. ######   ATX headings
 *   - * +         unordered list items (nested by indentation)
 *   1. 1)         ordered list items
 *   >             block quotes
 *   ``` / ~~~     fenced code blocks
 *   --- *** ___   horizontal rules
 *   blank line    paragraph separator
 *
 * Supported inline syntax:
 *   **bold**  __bold__  *italic*  _italic_  `code`
 *   [label](url) and ![alt](url) (only the label/alt is shown)
 *   \* escapes the next marker
 *
 * The result is a flat atom array plus one RetexParaFormat per non-empty
 * paragraph, ready for retex_layout_styled_paras().
 */

#include "rtable.h"
#include "retex.h"

typedef struct MarkdownDoc MarkdownDoc;

typedef struct {
    const char *font_face;   /* base face, e.g. "Sans" */
    double font_size;        /* base size in points */
    const char *mono_face;   /* code face, e.g. "Mono" */
    const char *features;        /* body HarfBuzz features (NULL = none) */
    const char *heading_features;/* heading features (NULL = body) */
    const char *code_features;   /* code features (NULL = liga=0,clig=0) */
    int default_align;       /* RetexAlign used for ordinary paragraphs */
} MarkdownStyle;

/* Parse utf8.  Returns NULL on allocation failure or empty/invalid input
 * that cannot be decoded (invalid UTF-8 is treated as Latin-1 bytes). */
MarkdownDoc *markdown_parse(const char *utf8, const MarkdownStyle *style);

void markdown_free(MarkdownDoc *doc);

/* Borrowed views into the parsed document.  *n is set to the count. */
const RAtom *markdown_atoms(const MarkdownDoc *doc, int *n);
const RetexParaFormat *markdown_formats(const MarkdownDoc *doc, int *n);

#endif /* XTCW_MARKDOWN_H */
