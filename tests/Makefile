CC = gcc
# Use prefix-based paths from the root makefile
CFLAGS += -Wall -I../utils -I$(incdir) -I$(incdir)/xtcw
LDFLAGS += -L$(libdir) -lxtcw -lwcl -lplainc -lretex -lnanosvg -lutils $(shell pkg-config --libs cairo cairo-xlib) -lXaw -lXmu -lXft -lfontconfig -lXrender -lXpm -lXext -lX11 -lXt -lm

TESTS = test_mls_foundation test_s_cstr test_trace test_mls_recursive test_m_table test_mls_ownership test_mls_stress test_retex_layout test_retex_linebreak test_retex_math test_wlabel test_retex_hit test_wlabel_selection test_wlistmulti test_wlabel_multi_para test_wlabel_interactive test_selection_wlabel_v2

all: $(TESTS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

run: all
	./run_all_tests.sh

clean:
	rm -f $(TESTS) *.o
