# Makefile to generated widgets via wbuild
# register-widgets.sh copies widget headers from ../widgets/
# therefore i need to apply touch after the widget is copied.
# This makefile needs: WBPREFIX,OUTPUT,WBUILD,WFLAGS

prefix      ?= ../../build
exec_prefix ?= $(prefix)
bindir      ?= $(exec_prefix)/bin
libdir      ?= $(exec_prefix)/lib
incdir      ?= $(prefix)/include
srcdir      ?= $(prefix)/source
docdir      ?= $(prefix)/doc
libexecdir  ?= $(exec_prefix)/libexec
PKGDATADIR  ?= $(prefix)/etc/xtcw


# where to find wbuild and wb.sh
wb_install=$(PKGDATADIR)
WBUILD=$(bindir)/wbuild
WBWRAP=$(bindir)/wb.sh
WBPREFIX=xtcw
WFLAGS= -d $(docdir) -c $(srcdir) -i $(incdir)/$(WBPREFIX) --include-prefix=$(WBPREFIX) $(PKGDATADIR)/tex.w $(PKGDATADIR)/nroff.w

BINC=$(incdir)/$(WBPREFIX)
BSRC=$(srcdir)
BDOC=$(docdir)

ALL_WIDGETS:=${notdir $(wildcard *.widget)}
WIDGET_H:=${ALL_WIDGETS:%.widget=$(BINC)/%.h}

ALL: all-widgets $(BINC)/register_wb.h

$(BINC)/%.h: %.widget
	wb_search=./ $(WBWRAP) $^ $(WBUILD) $(WFLAGS)

$(BINC)/register_wb.h: $(WIDGET_H)
	./register-widgets.sh $(BINC) $(BSRC)

all-widgets: $(WIDGET_H)

clean:
	-$(RM) $(WIDGET_H)
