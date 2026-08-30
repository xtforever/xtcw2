#ifndef BACKEND_H
#define BACKEND_H

#include <stdint.h>
#include "scaled.h"

typedef struct Backend Backend;

/**
 * @brief Full metrics of a character in points.
 */
typedef struct {
    double width;
    double height;
    double depth;
} CharMetrics;

struct Backend {
    void *ctx;
    
    /**
     * @brief Draw a filled rectangle.
     * 
     * @param self Backend instance
     * @param x X coordinate (in points)
     * @param y Y coordinate (in points)
     * @param w Width (in points)
     * @param h Height (in points)
     * @param color ARGB hex color (e.g., 0xFF0000FF for blue)
     */
    void (*draw_rect)(Backend *self, double x, double y, double w, double h, uint32_t color);
    
    /**
     * @brief Set the current drawing color.
     * 
     * @param self Backend instance
     * @param color ARGB hex color
     */
    void (*set_color)(Backend *self, uint32_t color);

    /**
     * @brief Draw a character.
     * 
     * @param self Backend instance
     * @param x X coordinate (baseline)
     * @param y Y coordinate (baseline)
     * @param c Character code
     */
    void (*draw_char)(Backend *self, double x, double y, int c);

    /**
     * @brief Draw an SVG image.
     * 
     * @param self Backend instance
     * @param x X coordinate (top-left)
     * @param y Y coordinate (top-left)
     * @param w Width in points
     * @param h Height in points
     * @param filename Path to SVG file
     */
    void (*draw_svg)(Backend *self, double x, double y, double w, double h, const char *filename);

    /**
     * @brief Set the font size in points.
     * 
     * @param self Backend instance
     * @param size Font size in points
     */
    void (*set_font_size)(Backend *self, double size);

    /**
     * @brief Set the font face and style.
     * 
     * @param self Backend instance
     * @param face Font family name (e.g. "Sans", "Serif", "Monospace")
     * @param style Font style bitmask (0: Normal, 1: Bold, 2: Italic)
     */
    void (*set_font_face)(Backend *self, const char *face, int style);

    /**
     * @brief Get the width of a character in points.
     * 
     * @param self Backend instance
     * @param c Character code
     * @return Width in points
     */
    double (*get_char_width)(Backend *self, int c);

    /**
     * @brief Get full metrics of a character in points.
     * 
     * @param self Backend instance
     * @param c Character code
     * @param metrics Output metrics structure
     */
    void (*get_char_metrics)(Backend *self, int c, CharMetrics *metrics);

    /**
     * @brief Ensure all pending drawing operations are completed.
     */
    void (*flush)(Backend *self);

    /**
     * @brief Clean up backend resources.
     */
    void (*destroy)(Backend *self);

    /**
     * @brief Set a clipping rectangle.
     * 
     * @param self Backend instance
     * @param x X coordinate (in points)
     * @param y Y coordinate (in points)
     * @param w Width (in points)
     * @param h Height (in points)
     * @param active If 0, disable clipping.
     */
    void (*set_clip)(Backend *self, double x, double y, double w, double h, int active);

    /**
     * @brief Draw a highlight background rectangle.
     * 
     * @param self Backend instance
     * @param x X coordinate (in points)
     * @param y Y coordinate (in points)
     * @param w Width (in points)
     * @param h Height (in points)
     * @param color ARGB hex color
     */
    void (*draw_highlight)(Backend *self, double x, double y, double w, double h, uint32_t color);

    /**
     * @brief Get the actual DPI used by this backend.
     * This matches the cairo_scale transform applied at creation,
     * so px2pt conversions using this DPI will match the coordinate system.
     */
    double (*get_dpi)(Backend *self);
};

#endif // BACKEND_H
