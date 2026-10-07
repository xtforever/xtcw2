#ifndef THEME_H
#define THEME_H

/*
 * Central theme / color registry.
 *
 * A global, MLS-backed store of name/value pairs (colours and integers,
 * e.g. borderwidth).  Widgets register themselves together with an
 * "apply theme" function; theme_apply() calls every registered function
 * so widgets can re-resolve their colours and redraw.
 *
 * Use get_color("name") to obtain a Pixel for the active theme.
 */

#include <X11/Intrinsic.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Called for every registered widget when the theme changes. */
typedef void (*theme_apply_fn)(Widget);

/* --- lifecycle ---------------------------------------------------------- */

/* Initialise the registry and the colour context (XCC).  Needs a realized
 * or at least created widget to derive Display/Visual/Colormap. */
void theme_init(Widget top);
void theme_destroy(void);

/* --- building themes (also used by the parser) -------------------------- */

void theme_begin(const char *name);
void theme_color(const char *name, uint32_t rgb);
void theme_int(const char *name, int value);
void theme_end(void);

/* --- selecting / querying ---------------------------------------------- */

/* Switch to the named theme and notify all registered widgets.
 * Unknown names are ignored (the active theme stays unchanged). */
void theme_select(const char *name);

/* Raw colour value (0 if the name is undefined or not a colour). */
uint32_t theme_get_rgb(const char *name);

int theme_color_defined(const char *name);
int theme_int_defined(const char *name);

/* Returns 1 and stores the value if |name| is defined as an integer. */
int theme_get_int(const char *name, int *out);

/* Name of the active theme, or NULL.  Pointer is valid until the next
 * theme_select()/theme_destroy(). */
const char *theme_active(void);

/* NULL-terminated array of theme names (borrowed pointers); the caller
 * frees only the array with free().  Valid until the next theme mutation. */
int theme_list(char ***out);

/* Resolve a colour name of the active theme to a Pixel (0 if undefined or
 * no colour context is available). */
Pixel get_color(const char *name);

/* --- widget registration ------------------------------------------------ */

void theme_register(Widget w, theme_apply_fn fn);
void theme_unregister(Widget w);
void theme_apply(void);

/* Persist the loaded themes in the hdf s-expression format. */
int theme_save(const char *file);

#ifdef __cplusplus
}
#endif

#endif /* THEME_H */
