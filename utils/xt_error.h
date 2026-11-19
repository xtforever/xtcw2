#ifndef XT_ERROR_H
#define XT_ERROR_H

/*
 * errno-style error channel for Xt actions.
 *
 * A C action sets xt_errno/xt_errmsg instead of returning a value (Xt action
 * procs are void).  The Lua binding xtaction() clears the state before the
 * call and returns (code, message) afterwards.
 *
 * The UI thread owns this state.  Do not use it from worker threads.
 */

enum {
    XT_OK = 0,
    XT_ERR_ARG,          /* missing or invalid argument */
    XT_ERR_NO_WIDGET,    /* widget handle could not be resolved */
    XT_ERR_PATH,         /* empty path */
    XT_ERR_OPEN,         /* file could not be opened */
    XT_ERR_WRITE,        /* write/close failed */
    XT_ERR_FORMAT,       /* malformed input file */
    XT_ERR_STATE,        /* action not valid in the current state */
    XT_ERR_RANGE,        /* value out of range */
    XT_ERR_COLORS,       /* too many colours for the output format */
    XT_ERR_MEMORY        /* allocation failed */
};

extern int  xt_errno;
extern char xt_errmsg[256];

/* Reset to XT_OK / "". */
void xt_error_clear(void);

/* Set the code and format a message.  fmt may be NULL for an empty message. */
void xt_error_set(int code, const char *fmt, ...);

#endif /* XT_ERROR_H */
