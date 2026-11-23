/* Minimal libmpg123 declarations for the mp3player engine.
 *
 * This lets the standalone engine build against the runtime library
 * (libmpg123.so.0) when libmpg123-dev is not installed.  If the real
 * <mpg123.h> is available, use it instead (see mp3engine.c).
 */
#ifndef MP3PLAYER_MPG123_MIN_H
#define MP3PLAYER_MPG123_MIN_H

#include <stddef.h>
#include <sys/types.h>

typedef struct mpg123_handle_struct mpg123_handle;

#define MPG123_OK          0
#define MPG123_ERR        -1
#define MPG123_DONE      -12
#define MPG123_NEW_FORMAT -11

#define MPG123_STEREO        2
#define MPG123_ENC_FLOAT_32  0x200

int mpg123_init(void);
void mpg123_exit(void);
mpg123_handle *mpg123_new(const char *decoder, int *error);
void mpg123_delete(mpg123_handle *mh);
int mpg123_open(mpg123_handle *mh, const char *path);
int mpg123_close(mpg123_handle *mh);
const char *mpg123_strerror(mpg123_handle *mh);
const char *mpg123_plain_strerror(int errcode);
int mpg123_format_none(mpg123_handle *mh);
int mpg123_format(mpg123_handle *mh, long rate, int channels, int encodings);
int mpg123_getformat(mpg123_handle *mh, long *rate, int *channels, int *encoding);
int mpg123_read(mpg123_handle *mh, unsigned char *out, size_t size, size_t *done);
int mpg123_scan(mpg123_handle *mh);
off_t mpg123_length(mpg123_handle *mh);
off_t mpg123_seek(mpg123_handle *mh, off_t sampleoff, int whence);

#endif
