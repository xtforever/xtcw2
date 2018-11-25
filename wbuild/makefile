# x toolkit intrinsics core widget set (XTCW)
#
# ..x..x.t..../\        /
# ...xx.tttt./ 	\      /
# ...x.x..t..\ 	 \ /\ /
# ..x...x tt..\_  /  \
#
#
#
# makefile for wbuild
# dependecies:
#    bison,flex
#
#
# example
#
# make PKGDATADIR=/tmp bindir=/tmp extra
# make PKGDATADIR=/tmp bindir=/tmp install
#
# install will copy binaries to $bindir and resources to $PKGDATADIR
# if you change PKGDATADIR you must rebuild from scratch!
#
# jens harms (au1064@gmail.com), 10.11.2018, project "xtcw"
#
#
#
bindir ?= /tmp
PKGDATADIR ?= $(CURDIR)
MKDIR ?= mkdir
CP ?= cp
RM ?= rm


YFLAGS  = -d
YACC    = bison
LEX     = flex

OBJS=declarefunc.o  generatedoc.o parse.tab.c scan.o util.o generatec.o generateh.o printbody.o  symbol.o
CPPFLAGS+=-DPKGDATADIR=\"$(PKGDATADIR)\" -DYYDEBUG -I. -D_GNU_SOURCE

%.tab.c %.tab.h: %.y
	$(YACC) $(YFLAGS) $<

%.lex.c %.lex.h: %.l
	$(LEX) -o$*.lex.c --header-file=$*.lex.h $<


ALL: wbuild
extra: clean ALL

scan.o: parse.tab.h

wbuild: $(OBJS)

install: wbuild
	-${MKDIR} -p ${PKGDATADIR}
	-${MKDIR} -p ${bindir}
	-${CP} *.w *.tex ${PKGDATADIR}
	-${CP} wbuild wb.sh ${bindir}

clean:
	${RM} wbuild $(OBJS) parse.c parse.tab.c parse.tab.h scan.c *~
