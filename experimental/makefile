VPATH=../utils
prefix      ?= ../build
exec_prefix ?= $(prefix)
bindir      ?= $(exec_prefix)/bin
libdir      ?= $(exec_prefix)/lib
incdir      ?= $(prefix)/include
srcdir      ?= $(prefix)/source
docdir      ?= $(prefix)/doc
libexecdir  ?= $(exec_prefix)/libexec
PKGDATADIR  ?= $(prefix)/etc/xtcw



CC=gcc -I. -I../utils -ggdb -O0 -DMLS_DEBUG -Wall -fsanitize=address -fno-omit-frame-pointer
#CC=gcc -I. -I../utils -ggdb -O0 -Wall -fsanitize=address -fno-omit-frame-pointer 
CFLAGS += -D_GNU_SOURCE -fms-extensions 
CFLAGS += -I$(incdir) -I$(incdir)/xtcw -I/usr/include/freetype2 -I/usr/local/include/freetype2 -I. -I../utils -Ixtcw


#enable profiler
#CFLAGS+=-pg

#CC=gcc -Wall -I. -I../utils  -O3 -march=znver2 
#CC=gcc -Wall -I. -I../utils  -O3 -pg -march=znver2
#CC=gcc -Wall -I. -I../utils  -O3  -fomit-frame-pointer
LOADLIBES+=-lm

T=var5-test hash_test geo1
D=mls.o ctx.o m_tool.o
ALL: ${T}

ADDT: clean ${T} svar2-test svar-util-test
${T}: ${D}
svar2-test: mls.o svar2.o 
svar4-test: mls.o svar2.o 

svar-util-test: svar-util.o svar3.o mls.o

var5-test: mls.o var5.o m_tool.o
clean:
	-${RM} *.o ${T}
