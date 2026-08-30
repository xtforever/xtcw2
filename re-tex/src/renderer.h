#ifndef RENDERER_H
#define RENDERER_H

#include "backend.h"
#include "node.h"

typedef struct {
    uint32_t text;
    uint32_t bg;
    uint32_t reverse_text;
    uint32_t reverse_bg;
    uint32_t sel_text;
    uint32_t sel_bg;
} RendererColors;

/**
 * @brief Renders a node (and its children) using the provided backend.
 * 
 * @param backend The rendering backend
 * @param node_handle The root node to render
 * @param x Current X position (in points)
 * @param y Current Y position (in points)
 * @param sel_start Selection start offset (-1 for none)
 * @param sel_end Selection end offset (-1 for none)
 * @param colors Pointer to RendererColors, or NULL for defaults
 * @param reverse_mode Global reverse mode
 */
void renderer_render(Backend *backend, int node_handle, double x, double y, int sel_start, int sel_end, const RendererColors *colors, int reverse_mode);

#endif // RENDERER_H
