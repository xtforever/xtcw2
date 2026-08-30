#ifndef RETEX_H
#define RETEX_H

#include "backend.h"
#include "renderer.h"

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
 * @brief Performs layout on text and returns a persistent paragraph object.
 */
RetexParagraph* retex_layout(Backend *be, const char *text, double width_pt, const char *font_face, double font_size_pt, RetexAlign align);

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
