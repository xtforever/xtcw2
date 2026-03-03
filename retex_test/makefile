prefix = ../build
libdir = $(prefix)/lib
incdir = $(prefix)/include

CC = gcc
CFLAGS_BASE = -Wall -I$(incdir) -I$(incdir)/xtcw $(shell pkg-config --cflags cairo cairo-xlib xft fontconfig)
LDFLAGS = -L$(libdir) -Wl,-rpath,$(libdir)
LDLIBS = -lxtcw -lXaw -lXmu -lXt -lX11 -lXft -lfontconfig -lXrender -lm $(shell pkg-config --libs cairo cairo-xlib xft fontconfig)

TARGET = retex_test

CFLAGS = $(CFLAGS_BASE) -g -DMLS_DEBUG -O0

all: debug

debug: CFLAGS = $(CFLAGS_BASE) -g -DMLS_DEBUG -O0
debug: $(TARGET)

production: CFLAGS = $(CFLAGS_BASE) -O3
production: clean $(TARGET)

$(TARGET): main.o
	$(CC) $(LDFLAGS) -g -o $@ $^ $(LDLIBS)

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o

run: $(TARGET)
	XENVIRONMENT=main.ad ./$(TARGET)
