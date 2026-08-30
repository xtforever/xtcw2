#ifndef BACKEND_CAIRO_H
#define BACKEND_CAIRO_H

#include "backend.h"

/**
 * @brief Create a Cairo backend that outputs to a PNG file.
 * 
 * @param filename Output filename
 * @param width Image width in pixels/points
 * @param height Image height in pixels/points
 * @return Backend* Pointer to new backend, or NULL on failure
 */
Backend* backend_cairo_create(const char* filename, int width, int height);

#endif // BACKEND_CAIRO_H
