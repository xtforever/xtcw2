#ifndef BUILDER_H
#define BUILDER_H

#include "node.h"
#include "scaled.h"

typedef CharMetricsScaled (*MeasureFunc)(void *ctx, int char_code, Scaled font_size, int style, const char *face);

/**
 * @brief Builds a list of nodes from a list of tokens.
 * 
 * @param token_list Handle to a memc list of Tokens.
 * @param measure_func Function to measure character dimensions.
 * @param measure_ctx Context passed to measure_func.
 * @param font_size_base Base font size in points.
 * @param font_face_base Base font face name.
 * @param face_handle_base Base font face handle.
 * @return Handle to a memc list of Nodes.
 */
int build_hlist(int token_list, MeasureFunc measure_func, void *measure_ctx, Scaled font_size_base, const char *font_face_base, int face_handle_base);

#endif // BUILDER_H
