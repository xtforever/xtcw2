#ifndef BACKEND_XPIXMAP_H
#define BACKEND_XPIXMAP_H

#include <X11/Xlib.h>
#include "backend.h"

/**
 * @brief Create a Cairo backend that renders to an X11 Pixmap.
 * 
 * @param dpy The X Display connection.
 * @param parent The parent window (for depth/screen info).
 * @param width Width of the pixmap to create.
 * @param height Height of the pixmap to create.
 * @return Backend* Pointer to the new backend, or NULL on failure.
 */
Backend* backend_xpixmap_create(Display *dpy, Window parent, int width, int height);

/**
 * @brief Get the underlying Pixmap from the backend.
 * Useful for copying the result to a window.
 * 
 * @param backend The backend instance (must be of type XPixmap).
 * @return Pixmap The underlying X Pixmap.
 */
Pixmap backend_xpixmap_get_pixmap(Backend *backend);

#endif // BACKEND_XPIXMAP_H
