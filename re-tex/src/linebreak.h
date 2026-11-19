#ifndef LINEBREAK_H
#define LINEBREAK_H

#include "node.h"
#include "scaled.h"
#include "glue.h"

/**
 * @brief Breaks a horizontal list of nodes into a vertical list of HBoxes (lines).
 * 
 * @param node_list Handle to list of Node handles (the paragraph)
 * @param width Target width for the lines
 * @param left_skip Glue to insert at the left of each line
 * @param right_skip Glue to insert at the right of each line
 * @return Handle to list of Node handles (list of HBox nodes)
 */
int line_break(int node_list, Scaled width, Glue left_skip, Glue right_skip, Scaled hang_indent, int hang_after);

typedef struct {
    Scaled width;
    Glue left_skip;
    Glue right_skip;
    Scaled first_indent;
    Scaled rest_indent;
    Scaled right_indent;
    int tolerance;          /* TeX-like badness tolerance, e.g. 200 */
    Scaled emergencystretch;
    int hyphen_penalty;     /* penalty for an automatic hyphen, e.g. 50 */
} LineBreakParams;

/**
 * Knuth-Plass style paragraph breaking over boxes, glue and penalty nodes.
 * Returns a list of HBox line nodes (ownership of nested word boxes moves to
 * the returned lines).  The input node list may be freed afterwards.
 */
int line_break_kp(int node_list, const LineBreakParams *params);

#endif // LINEBREAK_H
