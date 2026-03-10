CC ?= gcc
CFLAGS += -I. -I../utils -I../plainc_widgets

SRCS = MapAg.c syntax.c Table.c TableLoc.c TableVec.c WcActCB.c WcConvert.c WcCreate.c WcInvoke.c WcLateBind.c WcLoadRes.c WcName.c WcReg.c WcRegXt.c WcSetValue.c WcTemplate.c WcWarn.c Xp.c
OBJS = $(SRCS:.c=.o)

LIB = libwcl.a

all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

install: $(LIB)
	mkdir -p $(incdir)/xtcw $(libdir)
	cp *.h $(incdir)/xtcw/
	cp $(LIB) $(libdir)/

clean:
	rm -f *.o *.a
