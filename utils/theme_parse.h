#ifndef THEME_PARSE_H
#define THEME_PARSE_H

#include "theme.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Parse the hdf s-expression theme format:
 *
 *   (theme NAME (colourname 0xRRGGBB intname 123 ...))
 *   (theme other  (...))
 *
 * Value tokens starting with 0x/# are colours, plain decimals are integers.
 * ';' starts a line comment.  Returns the number of themes parsed. */
int theme_parse_string(const char *buf);

/* Read |file| and parse it.  Returns number of themes, or 0 if the file
 * is missing/unreadable (a warning is emitted). */
int theme_load(const char *file);

#ifdef __cplusplus
}
#endif

#endif /* THEME_PARSE_H */
