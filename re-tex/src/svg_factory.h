#ifndef SVG_FACTORY_H
#define SVG_FACTORY_H

#include <cairo.h>

/**
 * @brief Process-wide SVG raster factory.
 *
 * Rasterises an SVG once per (filename, device-pixel size) into a transparent
 * cairo image surface and caches the result. Backends blit the cached surface
 * instead of re-rasterising on every expose/redraw/resize.
 *
 * The cache is global on purpose: widgets destroy and recreate their backend on
 * resize (e.g. Wlabel), so a per-backend cache would be thrown away constantly.
 *
 * The returned surface is owned by the factory; do not destroy it. It stays
 * valid until svg_factory_destroy() (or the next call with the same key, which
 * returns the same surface).
 *
 * @param filename Path to the SVG file.
 * @param pixel_w  Target width in device pixels (> 0).
 * @param pixel_h  Target height in device pixels (> 0).
 * @return A transparent ARGB32 surface with the SVG content cropped to its
 *         solid bounding box and scaled to fill pixel_w x pixel_h, or NULL on
 *         failure.
 */
cairo_surface_t *svg_factory_get_image(const char *filename, int pixel_w, int pixel_h);

/**
 * @brief Free all cached parsed images and raster surfaces.
 *        Safe to call when nothing has been cached.
 */
void svg_factory_destroy(void);

#endif /* SVG_FACTORY_H */
