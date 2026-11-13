CC = gcc
# Use prefix-based paths from the root makefile
CFLAGS += -Wall -I../utils -I../utils/utf8 -I../utils/utf32 -I$(incdir) -I$(incdir)/xtcw
LDFLAGS += -L$(libdir) -Wl,--start-group -lxtcw -lwcl -lplainc -lretex -lnanosvg -lutils -lutf32 -lutf8 -Wl,--end-group $(shell pkg-config --libs cairo cairo-xlib) -lXaw -lXmu -lXft -lfontconfig -lXrender -lXpm -lXext -lX11 -lXt -lm

TESTS = test_utf8 test_utf32 test_ptable test_rtable test_tokenize_plain test_retex_unicode test_retex_plain test_retex_rich_lines test_retex_styled test_wbravo test_rich_wbravo test_wbravo_state test_wretex test_eqfader test_wspinbox test_wlistview test_wfilechooser test_mls_foundation test_s_cstr test_trace test_mls_recursive test_m_table test_mls_ownership test_mls_stress test_mls_trace test_retex_layout test_retex_linebreak test_retex_math test_wlabel test_retex_hit test_wlabel_selection test_wlistmulti test_wlabel_multi_para test_wlabel_interactive test_selection_wlabel_v2 test_gridbox test_vbox test_wlabel_colors test_wlabel_comprehensive test_theme_parse test_wtoggleswitch test_wviewport test_wtabwidget test_wmenubutton test_wappwindow

GEOMETRY_TESTS = test_wlabel_geometry test_wlabel_multipara test_wedit_geometry test_hslider_geometry test_vslider_geometry test_canvas_geometry test_wlist4_geometry test_wpaned_geometry test_gridbox_layout test_vbox_layout test_hbox_layout

all: $(TESTS) $(GEOMETRY_TESTS)

all: $(TESTS)

# test_wbravo embeds the generated widget source (not yet part of libxtcw).
test_wbravo: test_wbravo.c ../build/source/Wbravo.c
	$(CC) $(CFLAGS) -I../build/include/xtcw test_wbravo.c ../build/source/Wbravo.c -o $@ $(LDFLAGS)

# test_rich_wbravo also embeds Wbravo.c and inspects the document via WbravoP.h.
test_rich_wbravo: test_rich_wbravo.c ../build/source/Wbravo.c
	$(CC) $(CFLAGS) -I../build/include/xtcw test_rich_wbravo.c ../build/source/Wbravo.c -o $@ $(LDFLAGS)

# test_wbravo_state embeds Wbravo.c and checks full-state save/load snapshots.
test_wbravo_state: test_wbravo_state.c ../build/source/Wbravo.c
	$(CC) $(CFLAGS) -I../build/include/xtcw test_wbravo_state.c ../build/source/Wbravo.c -o $@ $(LDFLAGS)

# test_mls_trace verifies the MLS_DEBUG trace/post-mortem code paths. The
# debug entry points always exist in libutils, so build the test itself with
# MLS_DEBUG even if the surrounding build is not.
test_mls_trace: test_mls_trace.c
	$(CC) $(CFLAGS) -DMLS_DEBUG $< -o $@ $(LDFLAGS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

run: all
	./run_all_tests.sh

clean:
	rm -f $(TESTS) $(GEOMETRY_TESTS) *.o
