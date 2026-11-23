CC ?= gcc
CFLAGS += -I. -I../utils

SRCS = MapAg.c syntax.c Table.c TableLoc.c TableVec.c WcActCB.c WcConvert.c WcCreate.c WcInvoke.c WcLateBind.c WcLoadRes.c WcName.c WcReg.c WcRegXt.c WcSetValue.c WcTemplate.c WcWarn.c Xp.c
OBJS = $(SRCS:.c=.o)

LIB = libwcl.a

all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

install: $(LIB)
	mkdir -p $(incdir)/xtcw $(libdir)
	cp -p *.h $(incdir)/xtcw/
	cp -p $(LIB) $(libdir)/

clean:
	rm -f *.o *.a
