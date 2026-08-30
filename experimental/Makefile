# Experimental Makefile

# Compiler and flags
CC = gcc
CFLAGS = -g -I/usr/include/X11 -I../include -I. -I../util -I../wcl -I../LuaRunner
LDFLAGS = -L../wcl -L../lib -lX11 -lXt -lXaw -lXp -llua5.3 -lm -ldl

# Targets
TARGETS = commander_runner

# Object files
OBJS = commander_runner.o lua_bridge.o xt_bridge.o

all: $(TARGETS)

commander_runner: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGETS)

.PHONY: all clean
