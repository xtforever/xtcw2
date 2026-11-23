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
WFLAGS= -d $(docdir) -c $(srcdir) -i $(incdir)/$(WBPREFIX) --include-prefix=$(WBPREFIX) --hdf-dir=$(docdir)/hdf $(PKGDATADIR)/tex.w $(PKGDATADIR)/nroff.w

BINC=$(incdir)/$(WBPREFIX)
BSRC=$(srcdir)
BDOC=$(docdir)
LUI_REGISTRY ?= ../lui/registry_generated.lua

ALL_WIDGETS := $(notdir $(wildcard *.widget))
WIDGET_H := $(ALL_WIDGETS:%.widget=$(BINC)/%.h)

ALL: all-widgets $(BINC)/register_wb.h

# Regenerate when the widget spec or the wbuild generator itself changes.
$(BINC)/%.h: %.widget $(WBUILD)
	wb_search=./ $(WBWRAP) $< $(WBUILD) $(WFLAGS)

$(BINC)/register_wb.h: $(WIDGET_H)
	./register-widgets.sh $(BINC) $(BSRC) $(LUI_REGISTRY)

# wbuild skips writing outputs that are newer than their .widget source, but
# make still sees those outputs as older than the generator binary and would
# invoke wbuild for every widget on every run.  Align the skipped outputs with
# the generator mtime once; a real generator change then rebuilds dependents
# once instead of on every `make all`.
all-widgets: $(WIDGET_H)
	@for h in $(WIDGET_H); do \
		if [ "$$h" -ot "$(WBUILD)" ]; then touch -r "$(WBUILD)" "$$h"; fi; \
	done

install: all-widgets $(BINC)/register_wb.h
	mkdir -p $(BINC)
	cp -p *.h $(BINC)/

.PHONY: all-widgets install clean

clean:
	-$(RM) $(WIDGET_H)
