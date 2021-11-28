# x toolkit intrinsics core widget set (XTCW)
#
# ..x..x.t..../\        /
# ...xx.tttt./ 	\      /
# ...x.x..t..\ 	 \ /\ /
# ..x...x tt..\_  /  \
#
#
#
#
#

# export settings to sub-makefiles
export 

debug_enable ?= 1

prefix      ?= $(CURDIR)/build
exec_prefix ?= $(prefix)
bindir      ?= $(exec_prefix)/bin
libdir      ?= $(exec_prefix)/lib
incdir      ?= $(prefix)/include
srcdir      ?= $(prefix)/source
docdir      ?= $(prefix)/doc
libexecdir  ?= $(exec_prefix)/libexec
PKGDATADIR  ?= $(prefix)/etc/xtcw

MKDIR       ?= mkdir
RSYNC       ?= rsync -ruv 
CP          ?= cp
CC          ?= gcc
LN	    ?= gcc
CPPFLAGS    ?= -Wall -D_GNU_SOURCE
RMDIR	    ?= rm -rf

ifeq ($(debug_enable),1)
COMPTAG="\"DEBUG_$(shell date)\""
CFLAGS+= -g -DMLS_DEBUG -O0 
else
COMPTAG="\"PROD_$(shell date)\"" 
CFLAGS+= -O3 
endif
CFLAGS+= -DCOMPTAG=$(COMPTAG)


ALL: build_lib build_experimental

make_dirs:
	$(MKDIR) -p $(bindir)  $(docdir)  $(PKGDATADIR)  $(incdir) $(libdir) $(srcdir)

copy_files: make_dirs 
	$(RSYNC)  XPM $(incdir)/xtcw/
	$(RSYNC)  utils/ $(srcdir)/
	$(RSYNC)  wcl/*.c $(srcdir)/
	$(RSYNC)  wcl/*.h $(incdir)/
	$(RSYNC)  plainc_widgets/ $(srcdir)/
	$(RSYNC)  wbuild_widgets/ $(srcdir)/
	$(CP) makefile_library $(srcdir)/makefile

build_tools:
	$(MAKE) -C wbuild 
	$(MAKE) -C wbuild install



wbuild_widgets: build_tools
	$(MAKE) -C wbuild_widgets prefix=$(prefix)

build_lib: copy_files wbuild_widgets
	$(MAKE) -C $(srcdir) prefix=$(prefix)
	$(MAKE) -C $(srcdir) prefix=$(prefix) install

build_experimental:
	$(MAKE) -C experimental
	
clean:
	${RMDIR} build
	$(MAKE) -C wbuild clean
