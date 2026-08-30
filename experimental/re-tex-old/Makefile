CC = gcc
CFLAGS = -Wall -Wextra -I./src -I../utils -I../LuaRunner/nanosvg/src `pkg-config --cflags cairo`
LIBS = `pkg-config --libs cairo` -L../LuaRunner/nanosvg/src -lnanosvg -lm

SRCS = src/scaled.c src/glue.c src/node.c src/token.c src/builder.c src/linebreak.c src/retex.c src/renderer.c src/math_node.c src/math_layout.c
MEMC_SRCS = ../utils/mls.c ../utils/m_tool.c

all: test_scaled test_glue test_node_layout test_tokenizer test_builder test_linebreak test_integration test_retex_measure test_retex_width test_retex_height test_retex_api demo_png demo_styles demo_x11 demo_lines demo_x11_lines test_large_font

test_retex_api: $(SRCS) src/backend_cairo.c test_retex_api.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_retex_api $(SRCS) src/backend_cairo.c test_retex_api.c $(MEMC_SRCS) $(LIBS)
	./test_retex_api

debug: CFLAGS += -DMLS_DEBUG -g
debug: clean all

production: CFLAGS += -O3
production: clean all

test_scaled: src/scaled.c tests/test_scaled.c
	$(CC) $(CFLAGS) -o test_scaled src/scaled.c tests/test_scaled.c
	./test_scaled

test_glue: src/scaled.c src/glue.c tests/test_glue.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_glue src/scaled.c src/glue.c tests/test_glue.c $(MEMC_SRCS)
	./test_glue

test_node_layout: src/scaled.c src/glue.c src/node.c tests/test_node_layout.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_node_layout src/scaled.c src/glue.c src/node.c tests/test_node_layout.c $(MEMC_SRCS) $(LIBS)
	./test_node_layout

test_tokenizer: src/token.c tests/test_tokenizer.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_tokenizer src/token.c tests/test_tokenizer.c $(MEMC_SRCS)
	./test_tokenizer

test_builder: $(SRCS) tests/test_builder.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_builder $(SRCS) tests/test_builder.c $(MEMC_SRCS) $(LIBS)
	./test_builder

test_linebreak: $(SRCS) tests/test_linebreak.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_linebreak $(SRCS) tests/test_linebreak.c $(MEMC_SRCS) $(LIBS)
	./test_linebreak

test_integration: $(SRCS) tests/test_integration.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_integration $(SRCS) tests/test_integration.c $(MEMC_SRCS) $(LIBS)
	./test_integration

test_retex_measure: $(SRCS) src/backend_cairo.c tests/test_retex_measure.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_retex_measure $(SRCS) src/backend_cairo.c tests/test_retex_measure.c $(MEMC_SRCS) $(LIBS)

demo_png: $(SRCS) src/backend_cairo.c demo_png.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o demo_png $(SRCS) src/backend_cairo.c demo_png.c $(MEMC_SRCS) -L../LuaRunner/nanosvg/src -lnanosvg $(LIBS)

demo_styles: $(SRCS) src/backend_cairo.c demo_styles.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o demo_styles $(SRCS) src/backend_cairo.c demo_styles.c $(MEMC_SRCS) -L../LuaRunner/nanosvg/src -lnanosvg $(LIBS)
	./demo_styles

demo_x11: $(SRCS) src/backend_xpixmap.c demo_x11.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) `pkg-config --cflags x11` -o demo_x11 $(SRCS) src/backend_xpixmap.c demo_x11.c $(MEMC_SRCS) $(LIBS) `pkg-config --libs x11`

demo_lines: $(SRCS) src/backend_cairo.c demo_lines.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o demo_lines $(SRCS) src/backend_cairo.c demo_lines.c $(MEMC_SRCS) -L../LuaRunner/nanosvg/src -lnanosvg $(LIBS)

demo_x11_lines: $(SRCS) src/backend_xpixmap.c demo_x11_lines.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) `pkg-config --cflags x11` -o demo_x11_lines $(SRCS) src/backend_xpixmap.c demo_x11_lines.c $(MEMC_SRCS) $(LIBS) `pkg-config --libs x11`

clean:
	rm -f test_scaled test_glue test_node_layout test_tokenizer test_builder test_linebreak test_integration test_retex_measure demo_png demo_styles demo_x11 demo_lines demo_x11_lines output.png test_retex_width test_retex_height test_spacing demo_styles.png test_large_font test_large_simple.png test_large_math.png test_large_styles.png test_large_paragraph.png

test_retex_width: $(SRCS) src/backend_cairo.c tests/test_retex_width.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_retex_width $(SRCS) src/backend_cairo.c tests/test_retex_width.c $(MEMC_SRCS) $(LIBS)
	./test_retex_width

test_retex_height: $(SRCS) src/backend_cairo.c tests/test_retex_height.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_retex_height $(SRCS) src/backend_cairo.c tests/test_retex_height.c $(MEMC_SRCS) $(LIBS)
	./test_retex_height

test_spacing: $(SRCS) src/backend_cairo.c tests/test_spacing.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_spacing $(SRCS) src/backend_cairo.c tests/test_spacing.c $(MEMC_SRCS) $(LIBS)
	./test_spacing

test_large_font: $(SRCS) src/backend_cairo.c tests/test_large_font.c $(MEMC_SRCS)
	$(CC) $(CFLAGS) -o test_large_font $(SRCS) src/backend_cairo.c tests/test_large_font.c $(MEMC_SRCS) $(LIBS)
	./test_large_font
