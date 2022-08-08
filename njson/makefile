
DEBUGFLAGS=-g -Wall -DMLS_DEBUG
PRODUCTIONFLAGS=-O3

ifeq ($(production),1)
TAG="\"PROD_$(shell date)\""
CFLAGS+=$(PRODUCTIONFLAGS)
else
TAG="\"DEBUG_$(shell date)\""
CFLAGS+=$(DEBUGFLAGS)
endif


CFLAGS+=-D_GNU_SOURCE -Wall -I../utils -DNO_XPM
CFLAGS+=-DCOMP_TAG=$(TAG)
VPATH=../utils
YFLAGS  = -d
YACC    = bison
LEX     = flex


ALL: njson_read_test


%.tab.c %.tab.h: %.y
	$(YACC) $(YFLAGS) $<

%.lex.c %.lex.h: %.l
	$(LEX) -o$*.lex.c --header-file=$*.lex.h $<



njson_lex.lex.o: njson_parse.tab.c

njson_read.c: njson_parse.tab.c njson_lex.lex.c

njson_read_test: mls.o njson_read.o njson_lex.lex.o njson_parse.tab.o

clean:
	${RM} njson_read_test *.o *~ *.lex.* *.tab.* 

extra: clean ALL
