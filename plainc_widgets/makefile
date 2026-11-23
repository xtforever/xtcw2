# Legacy plain-C widget sources are no longer part of the production build.
# Gridbox moved to wbuild_widgets/Gridbox.widget + gridbox_core.c.
# Only the legacy headers are installed because utils/xu.h uses IconLabel.h.
CC ?= gcc
CFLAGS += -I. -I../utils -I../wcl -I$(incdir)

all:

install:
	mkdir -p $(incdir)/xtcw
	cp -p *.h $(incdir)/xtcw/

clean:
	rm -f *.o *.a
