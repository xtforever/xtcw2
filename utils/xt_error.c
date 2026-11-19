#include "xt_error.h"

#include <stdarg.h>
#include <stdio.h>

int  xt_errno = XT_OK;
char xt_errmsg[256];

void xt_error_clear(void)
{
    xt_errno = XT_OK;
    xt_errmsg[0] = '\0';
}

void xt_error_set(int code, const char *fmt, ...)
{
    va_list ap;

    xt_errno = code;
    if (!fmt) {
        xt_errmsg[0] = '\0';
        return;
    }
    va_start(ap, fmt);
    vsnprintf(xt_errmsg, sizeof xt_errmsg, fmt, ap);
    va_end(ap);
}
