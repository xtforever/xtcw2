CC ?= gcc
CFLAGS += -fms-extensions -I. -I$(incdir) -I$(incdir)/xtcw

# List of core objects from the original makefile_library
OBJS = mls.o m_tool.o conststr.o m_table.o
OBJS += focus-group.o converters-xft.o xutil.o
OBJS += ini_parse.tab.o ini_lex.lex.o ini_read2.o
OBJS += njson_parse.tab.o njson_lex.lex.o njson_read.o
OBJS += converters.o util.o xlayout.o wcreg2.o xprint2.o
OBJS += micro_vars.o form_parse.o parser_util.o communication.o socket_log.o
OBJS += XCC.o long.o icon.o choosecol.o strarray.o StrToPmap.o
OBJS += Pen.o str2color.o
OBJS += font.o ctx.o table256-crc16.o timer.o
OBJS += cp2.o slop4.o slopnet.o command-parser.o mrb.o
OBJS += svar2.o var5.o opts.o

LIB = libutils.a

all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

install: $(LIB)
	mkdir -p $(incdir)/xtcw $(libdir)
	cp *.h $(incdir)/xtcw/
	cp $(LIB) $(libdir)/

%.tab.c %.tab.h: %.y
	bison -d $<

%.lex.c %.lex.h: %.l
	flex -o$*.lex.c --header-file=$*.lex.h $<

# Dependencies for generated files
ini_lex.lex.o: ini_parse.tab.h
ini_read2.o: ini_lex.lex.o
njson_lex.lex.o: njson_parse.tab.h
njson_read.o: njson_lex.lex.o

clean:
	rm -f *.o *.a *.tab.c *.tab.h *.lex.c *.lex.h
