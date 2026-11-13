#ifndef THEME_PARSE_H
#define THEME_PARSE_H

#include "theme.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Parse the hdf s-expression theme format:
 *
 *   (theme NAME (name value name value ...))
 *   (theme other  (...))
 *
 * NAME is a bare token (no whitespace, parens or ';'); theme_save() writes it
 * unquoted.  Value names are bare tokens too.
 *
 * Values: tokens starting with 0x/# are colours; a token that parses fully
 * as a signed decimal is an integer; a double-quoted token (spaces allowed,
 * escapes \" \\ \n \r \t) is a string; any other bare token is a string.
 * ';' starts a line comment, also directly after a value.  A later value
 * with the same name replaces an earlier one.  Returns the number of themes
 * parsed. */
int theme_parse_string(const char *buf);

/* Read |file| and parse it.  Returns number of themes, or 0 if the file
 * is missing/unreadable (a warning is emitted). */
int theme_load(const char *file);

#ifdef __cplusplus
}
#endif

#endif /* THEME_PARSE_H */
