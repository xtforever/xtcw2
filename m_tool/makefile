CFLAGS += -D_GNU_SOURCE -fms-extensions -Wall -fsanitize=address -fno-omit-frame-pointer
CFLAGS+= -ggdb -DMLS_DEBUG -O0
CC = gcc
m_tool_test: m_tool.o mls.o conststr.o
clean:
	-${RM} *.o m_tool_test
