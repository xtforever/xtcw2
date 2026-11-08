#ifndef RETEX_H
#define RETEX_H

#include "backend.h"
#include "renderer.h"
#include "rtable.h"

/**
 * @brief Opaque handle to a laid-out paragraph.
 */
typedef struct RetexParagraph RetexParagraph;

/**
 * @brief Alignment options for text layout.
 */
typedef enum {
    RETEX_ALIGN_LEFT,
    RETEX_ALIGN_CENTER,
    RETEX_ALIGN_RIGHT,
    RETEX_ALIGN_JUSTIFY
} RetexAlign;

/**
 * @brief Geometry of one laid-out visual line (plain and TeX layout).
 *
 * start_byte/end_byte are offsets into the source UTF-8 string; end_byte is
 * exclusive.  The newline that ended a logical line is not part of any line's
 * range.  y is the top of the line relative to the top of the paragraph.
 * vbox_index is the line's HBox position in the paragraph vbox child list.
 */
typedef struct {
    int start_byte;
    int end_byte;
    double y;
    double height;
    int vbox_index;
} RetexLine;

/**
 * @brief Performs layout on text and returns a persistent paragraph object.
 */
RetexParagraph* retex_layout(Backend *be, const char *text, double width_pt, const char *font_face, double font_size_pt, RetexAlign align);

/**
 * @brief Plain-text layout: literal UTF-8, hard newlines, no TeX syntax.
 *
 * Wraps at spaces (a single run wider than the line overflows).  Records
 * per-line byte ranges and geometry for hit testing and caret placement.
 */
RetexParagraph* retex_layout_plain(Backend *be, const char *text, double width_pt, const char *font_face, double font_size_pt, RetexAlign align);

/**
 * @brief Per-paragraph formatting for the styled layout.
 *
 * Paragraphs are separated by blank lines: a line containing only spaces,
 * tabs or CR is blank and ends the preceding paragraph.  A single '\n' is a
 * soft line break inside the same paragraph.  All lengths are in points.
 *
 * indent_left / indent_right add a margin on both sides of every line of the
 * paragraph; first_line adds an extra left indent to the paragraph's first
 * visual line.  space_before / space_after add vertical space around the
 * paragraph.  align is a RetexAlign value.
 */
typedef struct {
    int align;
    double indent_left;
    double indent_right;
    double first_line;
    double space_before;
    double space_after;
} RetexParaFormat;

/**
 * @brief Styled layout: build a paragraph directly from rich atoms.
 *
 * Each RAtom_CHAR is rendered with its own RLook (font face / style / size);
 * RAtom_OBJECT atoms are reserved for inline objects (currently a zero-width
 * placeholder).  Hard line breaks are RAtom_CHAR atoms with cp == '\n'.
 * Source offsets recorded on the nodes and in the per-line metadata are
 * UTF-8 byte offsets (objects count as U+FFFC), i.e. the same coordinate
 * space as retex_layout_plain(), so caret/selection code is interchangeable.
 */
RetexParagraph* retex_layout_styled(Backend *be, const RAtom *atoms, int natoms,
                                    double width_pt, const char *font_face,
                                    double font_size_pt, RetexAlign align);

/**
 * @brief Styled layout with per-paragraph formatting.
 *
 * fmts[i] applies to the i-th non-empty paragraph (blank-line separated).
 * Paragraphs beyond nfmts use default_align with no indents or spacing.
 * Pass fmts == NULL for a single default paragraph.  The paragraph index
 * matches retex_paragraphs().
 */
RetexParagraph* retex_layout_styled_paras(Backend *be, const RAtom *atoms,
                                          int natoms, double width_pt,
                                          const char *font_face,
                                          double font_size_pt,
                                          const RetexParaFormat *fmts, int nfmts,
                                          RetexAlign default_align);

/**
 * @brief Paragraph model used by the styled layout.
 *
 * Returns the number of paragraphs (maximal runs of non-blank lines), at
 * least 1 for an empty document.  If out != NULL, writes up to max atom
 * offsets of each paragraph's first atom (an empty document writes 0).
 * Matches the paragraph indexing of retex_layout_styled_paras().
 */
int retex_paragraphs(const RAtom *atoms, int natoms, int *out, int max);

/**
 * @brief Font of the character at a source byte offset (styled layout).
 *
 * Returns 1 and fills style / size / face_handle (any may be NULL) if a
 * character node with exactly that source offset exists, else 0.
 */
int retex_paragraph_get_font_at(RetexParagraph *para, int pos,
                                int *style, double *size, int *face_handle);

/**
 * @brief Number of visual lines (plain and TeX layout).
 */
int retex_paragraph_get_line_count(RetexParagraph *para);

/**
 * @brief Copy up to max RetexLine records into out.  Returns the number copied.
 */
int retex_paragraph_get_lines(RetexParagraph *para, RetexLine *out, int max);

/**
 * @brief Fetch one line's geometry.  Returns 1 on success, 0 if out of range.
 */
int retex_paragraph_get_line(RetexParagraph *para, int index, RetexLine *out);

/**
 * @brief Caret rectangle for a source byte offset (plain and TeX layout).
 *
 * x is relative to the paragraph's left edge, y is the line top, height is the
 * line box height.  byte_offset is clamped into the document.  Returns 1 on
 * success, 0 if the paragraph has no plain-line metadata.
 */
int retex_paragraph_get_caret_pos(RetexParagraph *para, int byte_offset,
                                  double *x, double *y, double *height);

/**
 * @brief Map paragraph coordinates (top-left origin) to a source byte offset.
 * Uses nearest-boundary rounding.  Returns the offset, or -1 if the paragraph
 * has no plain-line metadata.
 */
int retex_paragraph_get_offset_at(RetexParagraph *para, double x, double y);

/**
 * @brief Render only visual lines [first_line, last_line] (inclusive).
 *
 * (x, y) is the top-left of the first rendered line.  Used for viewport
 * scrolling of long documents.
 */
void retex_paragraph_render_range(RetexParagraph *para, Backend *be,
                                  int first_line, int last_line,
                                  double x, double y,
                                  const RendererColors *colors, int reverse_mode);

/**
 * @brief Returns the measured height of a laid-out paragraph.
 */
double retex_paragraph_get_height(RetexParagraph *para);

/**
 * @brief Returns the measured width of a laid-out paragraph (max line width).
 */
double retex_paragraph_get_width(RetexParagraph *para);

/**
 * @brief Returns the height of the first line (baseline offset from top).
 */
double retex_paragraph_get_first_line_height(RetexParagraph *para);

/**
 * @brief Renders a laid-out paragraph to a backend.
 * @param reverse_mode If non-zero, draw character backgrounds with foreground color (for text selection)
 */
void retex_paragraph_render(RetexParagraph *para, Backend *be, double x, double y, const RendererColors *colors, int reverse_mode);

/**
 * @brief Frees a paragraph and all its internal nodes.
 */
void retex_paragraph_free(RetexParagraph *para);

/**
 * @brief Find node at given coordinates (relative to paragraph top-left).
 * @return opaque handle to node or 0 if not found.
 */
int retex_paragraph_get_node_at(RetexParagraph *para, double x, double y);

/**
 * @brief Set selection range in paragraph.
 */
void retex_paragraph_set_selection(RetexParagraph *para, int start, int end);

/**
 * @brief Convenience function (internally uses layout API).
 */
double retex_measure_height(Backend *be, const char *text, double width_pt, const char *font_face, double font_size_pt);

#endif // RETEX_H
