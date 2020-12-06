#ifndef SOCKET_LOG
#define SOCKET_LOG


#include <X11/Intrinsic.h>
#include <stdio.h>
#include <stdarg.h>

void slog_init(Widget top, int port);
void slog_init_service(Widget top, char *port);

void slog_mputs( int buf );
void slog_write( const char *s, ... );
void slog_write_va(const char *s, va_list ap );
void slog_close(int sfd);

extern int trace_slog;
extern void (*slog_cb) (int fd, char *buf, int size);

#endif
