#ifndef RETEX_HYPHEN_H
#define RETEX_HYPHEN_H

/*
 * Optional hyphenation support.
 *
 * libhyphen is loaded at runtime (libhyphen.so.0).  When it is unavailable or
 * no dictionary matches the current language, hyphen_breaks() simply returns
 * no break points; the layout then behaves as before.
 */

int hyphen_init(void);
void hyphen_shutdown(void);

/* "de_DE", "de", "en_US", ...  NULL selects the process locale. */
void hyphen_set_language(const char *lang);

/* Returns a malloc'ed array of byte offsets at which the UTF-8 word may be
 * broken.  *out_count is set to the number of offsets (0 if none). */
int *hyphen_breaks(const char *word, int len, int *out_count);

#endif /* RETEX_HYPHEN_H */
