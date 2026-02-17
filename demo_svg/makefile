TARGET = demo_svg
CC = gcc

PREFIX = ../build
INCDIR = $(PREFIX)/include
LIBDIR = $(PREFIX)/lib

CFLAGS = -g -Wall -I$(INCDIR) -I$(INCDIR)/xtcw $(shell pkg-config --cflags cairo xft fontconfig)
LIBS = $(LIBDIR)/libxtcw.a $(shell pkg-config --libs cairo xft fontconfig) -lXt -lXaw -lX11 -lXpm -lXext -lXmu -lXrender -lm -lutil -ldl

SRCS = demo_svg.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
