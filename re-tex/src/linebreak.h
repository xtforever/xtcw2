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

#endif // LINEBREAK_H
