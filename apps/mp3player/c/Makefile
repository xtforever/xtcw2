CC ?= gcc
CFLAGS ?= -O2 -fPIC -Wall -Wextra -D_GNU_SOURCE
LUA_CFLAGS := $(shell pkg-config --cflags lua5.3 2>/dev/null)
LUA_LIBS   := $(shell pkg-config --libs lua5.3 2>/dev/null || echo -llua5.3)

PULSE_CFLAGS := $(shell pkg-config --cflags libpulse-simple 2>/dev/null)
PULSE_LIBS   := $(shell pkg-config --libs libpulse-simple 2>/dev/null)
MPG123_PC     := $(shell pkg-config --exists libmpg123 2>/dev/null && echo yes)
ifeq ($(MPG123_PC),yes)
MPG123_CFLAGS := $(shell pkg-config --cflags libmpg123 2>/dev/null)
MPG123_LIBS   := $(shell pkg-config --libs libmpg123 2>/dev/null)
else
MPG123_CFLAGS := -I.
MPG123_LIBS   := -l:libmpg123.so.0
endif

all: mp3fft.so

mp3fft.so: mp3fft.c
	$(CC) $(CFLAGS) $(LUA_CFLAGS) -shared -o $@ $< $(LUA_LIBS) -lm

# In-process DSP engine (optional; app falls back to mpg123 if missing).
mp3engine.so: mp3engine.c mpg123_min.h
	$(CC) $(CFLAGS) -shared -fPIC $(LUA_CFLAGS) $(MPG123_CFLAGS) $(PULSE_CFLAGS) \
		-o $@ mp3engine.c $(LUA_LIBS) $(MPG123_LIBS) $(PULSE_LIBS) -lpthread -lm

# Standalone proof for libmpg123 + 5-band EQ + libpulse-simple.
# Requires libmpg123-dev and libpulse-dev.  Not part of the default build.
mp3engine_test: mp3engine_test.c mpg123_min.h
	$(CC) $(CFLAGS) -D_FILE_OFFSET_BITS=64 \
		$(MPG123_CFLAGS) $(PULSE_CFLAGS) \
		-o $@ $< $(MPG123_LIBS) $(PULSE_LIBS) -lm

clean:
	rm -f mp3fft.so mp3engine.so mp3engine_test

.PHONY: all clean
