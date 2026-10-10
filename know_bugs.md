# Known bugs and gotchas

Discovered while implementing `EqFader` (audio-EQ fader widget).

## wbuild: `@class` doc comment must be a single line

**Symptom:** `syntax error at ... (token = TEXT)` at the first extra line after
`@class`.

**Cause:** wbuild accepts exactly one `@ ...` documentation line after `@class`.
Two or more consecutive `@`-prefixed lines start a second doc block and fail.

**Fix:** keep the class doc to one line, e.g.

```
@class EqFader (Wheel)
@ Vertical audio-EQ fader (Cairo track, Xft dB scale), value in centibel.
```

**File:** `wbuild_widgets/EqFader.widget`

## wbuild: `OR` is not an operator, and `var` lines can be swallowed by comments

**Symptom:** generated class record contains invalid C such as
`XtExposeCompressMultiple OR XtExposeGraphicsExpose` (compile error:
`expected '}' before 'OR'`), or a class variable silently keeps its default.

**Cause:** wbuild has no `OR` token — the text is emitted verbatim into the class
record. Additionally, a line written as `var name = ...` (without `@var`) that
directly follows a documentation comment block is parsed as a *continuation* of
that comment and ignored.

Existing widgets `HSlider.widget` / `VSlider.widget` contain:

```
@ The Core variable |compress_exposure| is OR'ed with
|XtExposeGraphicsExpose|, in order to get graphics expose events delivered
to the |expose| method.
var compress_exposure = XtExposeCompressMultiple OR XtExposeGraphicsExpose
```

Here the `var ...` line is swallowed by the preceding doc comment, so
`compress_exposure` stays at the Core default (`FALSE`) instead of
`XtExposeCompressMultiple`. The intent of those widgets is therefore not
honoured. (Verified: `build/source/HSlider.c` emits `compress_exposure FALSE`.)

**Fix (new code):** use a single `@var` with a single value:

```
@var compress_exposure = XtExposeCompressMultiple
```

**Files:** `wbuild_widgets/EqFader.widget` (fixed); latent in
`wbuild_widgets/HSlider.widget`, `wbuild_widgets/VSlider.widget` (not yet fixed).

## wbuild: private vars need one `@var` per line and no initializer

**Symptom:** `syntax error at ','` / `syntax error at '='`.

**Cause:** `privatedecl : VAR type_and_name opt_semi` — the grammar accepts one
declarator per `@var` and no `= initializer`. Pointer and array declarators are
fine (`cairo_surface_t *surface`, `XftColor col[3]`).

**Fix:** declare every private field on its own `@var` line without `=`, and
initialise values in `initialize`. Public resources DO require defaults
(`EQUALS` is mandatory there).

**File:** `wbuild_widgets/EqFader.widget`

## LUI parser: no negative number literals

**Symptom:** `lui.run(ui)` builds nothing (blank/default window); `parser.parse`
prints `Syntax Error ... Stopped at character: '-'`.

**Cause:** `lui/parser.lua` defines the number rule as
`number = digit^1 * (P(".") * digit^1)^-1` with no optional leading `-`, and a
symbol may not start with `-` either. So `:value -200` is unparseable. A
negative literal anywhere inside a list makes the whole top-level list fail,
which then makes the entire program match nothing.

**Impact:** every LUI property with a negative literal, e.g.
`(widget :value -1 ...)`, is broken.

**Fix / workaround:** avoid negative literals in S-expression source; set such
properties via `gui.set(id, "value", -200)` after `lui.run(...)`. A real fix
would add an optional sign to the `number` grammar rule.

**File:** `lui/parser.lua` (latent); worked around in `demos/demo_eqfader.lua`.

## LUI `lui.run` swallows parse errors

**Symptom:** a malformed UI string is silently ignored; the app shows an empty
default shell, and the demo smoke test can pass on a blank window.

**Cause:** `lui/lui.lua` `M.run` returns `nil, "Parse error"` instead of raising,
and callers typically ignore the return value. `M.load` wraps the chunk in
`pcall`, so a returned `nil` does not become a logged error.

**Fix:** check the return of `lui.run` and `error(...)` on failure (done in
`demos/demo_eqfader.lua`). Additionally `commander/run_demo_tests.sh` now treats
`Syntax Error` as a failure, so a blank/default window cannot pass the smoke
test anymore.

## wbuild: method overrides must not repeat the parameter list

**Symptom:** `make` fails while generating a subclass, with
`<file>.widget:<n>: Parameter list of \`<method>' ignored` and
`*** [makefile:35: .../<Sub>.h] Fehler 1` (exit marker).

**Cause:** `wbuild/generatec.c` (`method_func_decl`) calls `warn_if_params` for a
method whose declaration is found in a **superclass** (i.e. an override). An
inherited method's signature is fixed, so repeating `($)` / `(...)` increments
`nerrors`, and wbuild exits non-zero when `nerrors > 0`. The *first* definition
of a method (its owning class) may keep the parameter list.

**Fix:** omit the parameter list on overrides, e.g.

```
@proc void colors_changed      # override of Wheel's method — no ($)
{
    $dirty = 1;
    #colors_changed($);
}
```

**File:** `wbuild_widgets/Wlabel.widget` (fixed); compare `Frame.widget`'s
`total_frame_width`/`compute_inside` overrides.

## Adding a method to a base class requires a full clean rebuild

**Symptom:** after adding a method (e.g. `colors_changed`) to `Wheel.widget`,
subclasses (Gauge/Wlabel/...) dispatch to garbage or warn, or the app misbehaves.

**Cause:** the new method grows `WheelClassPart`, but `build/source/makefile` is
a bare `wildcard *.c → .o` archive with **no header dependencies**. `make widgets`
only regenerates `Wheel.h`/`Wheel.c` for the changed `.widget`; stale
`Gauge.o`/`Wlabel.o` keep the old class-part layout and alias the new slot.

**Fix:** `make clean && make` after changing a base-class method set (or add real
header dependency tracking to `build/source/makefile`). The new method slot is
appended after the existing ones (`sig_recv`), so it is append-only for the
stale `experimental/karo_widget` `WheelP.h`.

**File:** build system (`makefile` / generated `build/source/makefile`).

## XtVaSetValues(borderWidth) on a geometry-managed child loops forever

**Symptom:** applying a theme that changes `borderwidth` on a child of a Gridbox
hangs the app in an endless stream of `GridboxGeometryManager`.

**Cause:** `XtVaSetValues(w, XtNborderWidth, n, NULL)` changes a geometry
resource; the parent's `geometry_manager` re-lays-out the child, which
re-triggers negotiation, without converging.

**Fix:** set the core field (`$border_width`) and update the X window directly
with `XSetWindowBorderWidth()` / `XSetWindowBorder()` instead of
`XtVaSetValues`. Before realize the core field is enough (Xt uses it when the
window is created). Note: runtime border changes then do not re-flow the parent;
acceptable for a test/theme toggle.

**File:** `wbuild_widgets/Wheel.widget` (`colors_changed`).


## Theming: widget-owned colours/caches are not covered by Wheel.colors_changed

**Symptom:** under a dark theme (e.g. `dracula`) an EqFader's dB labels stayed
dark/unreadable, and its background stayed the old colour: widget-specific
colours and cached GCs/pixmaps did not follow the theme.

**Cause:** `Wheel.colors_changed` only reassigns Wheel's six colour resources
(`bg_norm`, `bg_sel`, `bg_hi`, `fg_norm`, `fg_sel`, `fg_hi`) plus the border. A
widget that caches its own `GC`/Pixmap (EqFader's `gc_bg`) or uses extra colour
resources (EqFader's `tickColor`/`labelColor`) never sees the change.

**Fix:** override `colors_changed` in the widget: call `#colors_changed($)` for
the Wheel base behaviour, then re-derive the widget's own colours (e.g. from
`get_color("fg_norm")`, or optional theme names such as `tick_color`,
`label_color`, `thumb_color`), rebuild any cached GC/pixmap, and redraw.
See `wbuild_widgets/EqFader.widget`.

## Widgets that do not inherit Wheel register directly

Widgets not derived from `Wheel` can still take part in theming by calling
`theme_register($, own_apply_fn)` in their `initialize` (and
`theme_unregister($)` in `destroy`), where `own_apply_fn(Widget)` re-resolves
their colours via `get_color()`/`theme_get_int()` and redraws.

## No production build: `commander/makefile` hardcodes MLS_DEBUG + ASan

**Symptom:** `make debug_enable=0` builds the toolkit libraries as release
(`-O3`, no `-DMLS_DEBUG`) but `commander_runner` is still compiled with
`-DMLS_DEBUG -O0 -fsanitize=address` and links `libasan`. There is no way to
produce a real, sanitizer-free production binary of the host application.

**Cause:** `commander/makefile` line 13 set the compiler command itself:
`CC=gcc -I. -I../utils -ggdb -O0 -DMLS_DEBUG -Wall -fsanitize=address
-fno-omit-frame-pointer`. Because the flags live in `$(CC)`, the root
`debug_enable` switch (and the exported `CFLAGS`) had no effect on the host.
Worse, mixing `-DMLS_DEBUG` objects with non-debug toolkit libraries crashes at
runtime (see `overview.md` §9.1).

**Fix:** move the mode flags out of `$(CC)` into `$(CFLAGS)`, gated on
`debug_enable`:

```
debug_enable ?= 1
CC = gcc
CFLAGS += -Wall -D_GNU_SOURCE -fms-extensions
...
ifeq ($(debug_enable),1)
CFLAGS += -g -O0 -DMLS_DEBUG -fsanitize=address -fno-omit-frame-pointer
else
CFLAGS += -O3
endif
```

Always build from the repository root (`make commander [debug_enable=0]`) so the
libraries and the host agree on `MLS_DEBUG`.

**Files:** `commander/makefile` (fixed); `makefile` (root)

## `make clean` leaves ASan `LuaRunner/*.o` and deletes a tracked source

**Symptom:** after a debug build, switching to `make debug_enable=0` and
relinking can fail with undefined `__asan_*` symbols or silently link
instrumented objects — the build is not actually clean. Additionally
`make clean` removes `LuaRunner/luaxt_wrap.c`, which is tracked by git, leaving
the worktree dirty.

**Cause:** `commander/makefile`'s `clean` only ran `rm *.o ${T}` in `commander/`
and removed `../LuaRunner/luaxt_wrap.c`; it never removed
`LuaRunner/luaxt.o` / `LuaRunner/luaxt_wrap.o` (which hold `__asan` symbols
after a debug build), and the root `clean` did not touch `LuaRunner` either.

**Fix:** `commander/makefile` clean now removes
`../LuaRunner/luaxt.o ../LuaRunner/luaxt_wrap.o ../LuaRunner/luaxt_wrap.c`; a
new root `distclean` target (`clean` + the same removals) is the supported way
to switch build modes. `LuaRunner/luaxt*.o` and `luaxt_wrap.c` are now in
`.gitignore` (swig regenerates `luaxt_wrap.c` on the next build).

**Files:** `commander/makefile`, `makefile`, `.gitignore` (fixed)

## WlsMulti / WlistMulti: BadAlloc on X_CreatePixmap in a nested Gridbox

**Symptom:** an application that puts a `list-view` (class `WlsMulti`, which
renders through `WlistMulti`) inside a nested `Gridbox` aborts at startup with

```
X Error of failed request:  BadAlloc (insufficient resources for operation)
  Major opcode of failed request:  53 (X_CreatePixmap)
```

The error is the last thing logged after a run of
`[2]get_line_intern: WlistMulti:list ...` / `[2]print_pixmap: ...` lines.
Reported while building the `cookbook/` demo (three `list-view` widgets in
nested screen grids).

**Cause:** `WlistMulti` creates a pixmap per visible line/resize; in a nested
`Gridbox` the size negotiation can hand it an oversized (or repeatedly
changing) width, exceeding the X server's pixmap limits. Xt's default error
handler turns the resulting `BadAlloc` into a fatal error. The debug build
(`-fsanitize=address`, `-O0`) sometimes survives longer because the allocator
and timing differ, so a 3-second smoke test can pass on debug while the release
build dies — the build mode is not the root cause.

**Workaround / fix:** avoid `list-view`/`WlsMulti` in nested layout managers
until the sizing is fixed; the `cookbook/` reference app uses `Wlabel` for its
lists. A proper fix needs a size audit of `WlistMulti`/`WlsMulti` (clamp the
pixmap dimensions, avoid re-creating pixmaps during `resize`) and a regression
test with a `list-view` inside a nested `Gridbox`.

**Files:** `wbuild_widgets/WlsMulti.widget`, `wbuild_widgets/WlistMulti.widget`
(latent; worked around in `cookbook/`)

## Shell sizing reads uninitialised width/height (breaks every -O3 build)

**Symptom:** every LUI demo started with a full-screen-width window of (near)
zero height — only the titlebar was visible. Debug builds
(`-O0 -fsanitize=address`) were fine; `make debug_enable=0` (`-O3`, no ASan)
broke *all* demos, because they all use `Gridbox`.

**Cause:** `commander/commander_runner.c` sized the shell from its first child
with a positive size, using uninitialised locals:

```c
WidgetList children; Cardinal num_children;
XtVaGetValues(TopLevel, XtNchildren, &children, XtNnumChildren, &num_children, NULL);
for (...) {
    Dimension width, height;                 /* uninitialised */
    XtVaGetValues(children[i], XtNwidth, &width, XtNheight, &height, NULL);
    if (width > 0 && height > 0)
        XtVaSetValues(TopLevel, XtNwidth, width, XtNheight, height, NULL);
}
```

The shell's internal `shellext` child does **not** expose `XtNwidth`/`XtNheight`
through `XtVaGetValues`, so `width`/`height` keep their stale stack contents.
With `-O0`/ASan those happen to be `0` (child skipped, the real size is found);
with `-O3` they are large garbage (e.g. `58576x1`), which is then applied to the
shell. `Gridbox` is laid out far below its preferred size, its `Position`
(16-bit) accumulators overflow to negative coordinates, and the first
`X_CreatePixmap` fails with `BadAlloc`, aborting the app.

Found with Valgrind (`Conditional jump ... depends on uninitialised value`,
origin `main`'s stack) plus a sentinel-value experiment that showed the
sentinel survived for the `shellext` child.

**Fix:** initialise the locals and skip NULL entries:

```c
WidgetList children = NULL; Cardinal num_children = 0;
XtVaGetValues(TopLevel, XtNchildren, &children, XtNnumChildren, &num_children, NULL);
for (...) {
    Dimension width = 0, height = 0;
    Widget child = children ? children[i] : NULL;
    if (!child) continue;
    XtVaGetValues(child, XtNwidth, &width, XtNheight, &height, NULL);
    if (width > 0 && height > 0) { ... }
}
```

**Files:** `commander/commander_runner.c` (fixed)

**Regression test:** `cookbook/scripts/smoke-test.sh` now asserts the toplevel
window has a positive width *and* height (not merely that a window exists).

## Inline SVG icons float above the text baseline (viewBox padding)

**Symptom:** in a Wlabel/Wretex line that mixes text with
`\includesvg{file}{w}{h}`, the inline icons sat a little too high — not on the
text baseline (see `demos/demo_wlabel_svg.lua`).

**Cause:** the SVG node is created with `height = h, depth = 0`, and the
renderer draws it with its box bottom on the baseline
(`re-tex/src/renderer.c`, `NODE_SVG`). Both backends' `draw_svg`
(`backend_cairo.c`, `backend_xpixmap.c`) scaled the **full nanoSVG viewBox**
(`image->width` × `image->height`) onto the `w` × `h` box. Most icon assets
carry padding in their viewBox (e.g. `LuaRunner/alert.svg` uses
`viewBox="-10 -10 380 380"`), so the visible content stopped short of the box
bottom and the icon appeared to float above the baseline (and slightly small).

A first attempt cropped to the bounds of *all* visible shapes. That over-crops
when the asset contains soft shadows/glows: `alert.svg`'s blurred circle has
opacity 0.59 and extends ~10 units below the white circle, so the visible icon
was pushed up (most obvious on the large `\includesvg{...}{24pt}{24pt}` icon in
`demos/demo_wlabel_svg.lua`).

**Fix:** in both `draw_svg` implementations, compute the bounding box of the
**solid** shapes only (`NSVGshape.bounds` for shapes with `opacity >= 0.9`;
nanosvg ignores SVG filters, so blurred shadows are not applied but would still
pollute the bounds), and map that box onto the `w` × `h` box
(`translate(x,y); scale(w/(maxx-minx), h/(maxy-miny)); translate(-minx,-miny)`).
Fall back to all visible shapes, then to the viewBox. The icon's visible bottom
then lands on the baseline and its horizontal padding is removed too.

**Files:** `re-tex/src/backend_cairo.c`, `re-tex/src/backend_xpixmap.c` (fixed)
