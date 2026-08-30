TARGET = wlabel_demo

prefix  ?= ../build
incdir  ?= $(prefix)/include
libdir  ?= $(prefix)/lib

CC       = gcc
CFLAGS  += -g -DMLS_DEBUG -O0 -Wall \
           -I$(incdir) -I$(incdir)/xtcw -I../utils \
           -I/usr/include/freetype2 \
           $(shell pkg-config --cflags cairo 2>/dev/null)

LDFLAGS += -L$(libdir) \
           -Wl,--start-group -lxtcw -lwcl -lplainc -lretex -lnanosvg -lutils -Wl,--end-group \
           $(shell pkg-config --libs cairo cairo-xlib 2>/dev/null) \
           -lXaw -lXmu -lXft -lfontconfig -lXrender -lXpm -lXext -lX11 -lXt -lm

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

.PHONY: clean run

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
