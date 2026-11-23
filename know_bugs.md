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

## Headless C test: a lone Core widget under `sessionShell` leaves the shell 0x0

**Symptom:** a test that creates a single Core/Wheel-derived leaf widget
directly under `sessionShellWidgetClass` and calls `XtRealizeWidget` aborts
with

```
Error: Shell widget <name> has zero width and/or height
```

(followed by the post-mortem analyser output); assertions are never reached.

**Cause:** a Shell derives its size from its managed children's
`query_geometry`. A Composite child (e.g. `VBox`) drives that negotiation, but a
single non-Composite child leaves the shell at 0x0, and Xt raises the error on
realize.

**Fix:** wrap the leaf widget in a sized Composite such as `VBox` (or set
explicit `XtNwidth`/`XtNheight` on the shell) before `XtRealizeWidget`.
See `tests/test_wtoggleswitch.c`.

**File:** `tests/test_wtoggleswitch.c`

## Window shaping: XmuReshapeWidget pitfalls; use XShapeCombineMask instead

**Symptom A (crash):** a widget that calls `XmuReshapeWidget` from its `resize`
method aborts on the first resize with

```
X Error of failed request:  BadDrawable (invalid Pixmap or Window parameter)
  Major opcode of failed request:  53 (X_CreatePixmap)
```

**Cause A:** `resize` is invoked during geometry negotiation *before* the
widget's window exists; `resize` then calls `XmuReshapeWidget` on
`XtWindow($) == 0`.

**Symptom B (no rounding):** `XmuReshapeWidget($, XmuShapeRoundedRectangle, d, d)`
produces only a **1 px-inset rectangle**, not a rounded rectangle/pill. Measured
with `xwininfo -shape`: shape extents `54x34+1+1` for a 56×36 window, identical
for `d` = the width/2 and `d` = the smaller side; pixel analysis showed magenta
only on the 1 px border, never in the corners.

**Cause B:** despite the header naming the parameters `corner_width`/`corner_height`,
`XmuShapeRoundedRectangle` did not round the corners in this setup (it may
require a mapped/visible window). Passing the corner *diameter* (not radius) is
also easy to get wrong.

**Fix:** do not rely on `XmuReshapeWidget`. Build a 1-bit mask with Xlib
(`XFillRectangle` cross-bars + four `XFillArc` quarter-discs, or two filled
arcs + a middle rectangle for a stadium) and install it with
`XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, mask, ShapeSet)`. It works
before the window is mapped and gives an exact, predictable shape. Guard with
`if (!win) return;` and only call it once the widget is realized (the `resize`
body must be guarded by `XtIsRealized`).

**Files:** `wbuild_widgets/WToggleSwitch.widget` (uses `XShapeCombineMask`);
`wbuild_widgets/Wlabel.widget`, `Gauge.widget`, `Wradio.widget` still use
`XmuReshapeWidget` (their defaults are 0 or a small percentage, so the weakness
is not obvious).

## Wedit / WeditMV: `cornerRoundPercent` is a dead resource

**Symptom:** setting `cornerRoundPercent` on an `edit` (Wedit) or `WeditMV`
widget has no visible effect; the window corners are never rounded.

**Cause:** `Wedit.widget` and `WeditMV.widget` declare
`@var int cornerRoundPercent = 20` but never call `reshape_widget` /
`XmuReshapeWidget` anywhere, unlike `Wlabel`, `Gauge` and `Wradio`. The resource
is silently ignored, so window shaping was effectively never exercised by any
demo before `WToggleSwitch` was added.

**Fix:** wire `reshape_widget($)` into `realize` and `resize` (guarded by
`XtIsRealized`) as `Wlabel`/`Gauge` do. Not fixed here (out of scope).

**File:** `wbuild_widgets/Wedit.widget`, `wbuild_widgets/WeditMV.widget`

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

## commander_runner: global `*width`/`*height` fallback resources break Xaw Viewport

**Symptom:** an Xaw `Viewport` created from LUI renders as a blank window; its
internal clip window stays `1x1` and the scrollbars are degenerate (`1x400`,
`600x1`) — no usable scrolling, even though the same widget tree built by hand
in C works.

**Cause:** `commander_runner.c`'s `fallback_resources[]` set
`"*width: 600"` / `"*height: 400"`. The `*` applies to **every** widget, not
just the top-level shell. The Xaw Viewport creates its internal `clip` and the
vertical/horizontal `Scrollbar` children itself, without explicit width/height,
so they all inherit 600x400. In `ComputeLayout()` (`Xaw/Viewport.c`) the bar
thickness is subtracted from the clip size; 600/400 minus a 600x400 bar clamps
the clip to the `if (clip_width < 1) clip_width = 1;` floor, i.e. `1x1`.

**Fix:** scope the default size to the shell:
`APP_NAME ".width: 600"` / `APP_NAME ".height: 400"` (APP_NAME is `luarunner`).
Confirmed headless: right vertical bar `14xN`, bottom horizontal bar `Nx14`,
clip > 1x1.

**File:** `commander/commander_runner.c`

## libXt calls `change_managed` only when the parent is already realized

**Symptom:** a Composite/Constraint child created (and managed) *before*
`XtRealizeWidget` is never adopted by a custom `change_managed` — e.g. a scroll
viewport reports "no content", its content is never reparented, scrollbars stay
hidden, and `geometry_manager` layout for that child never runs.

**Cause:** in libXt `XtManageChildren` calls the parent's `change_managed` only
if `parent_realized` is true (`Manage.c`: `if (... && parent_realized)`). LUI
creates and manages every widget before realizing the shell, so the initial
managed set never triggers `change_managed`. (This is why a widget must not
depend on `change_managed` for its initial layout.)

**Fix:** scan the current managed children in `realize` (after `#realize`), and
keep `change_managed` for later dynamic manage/unmanage. `Wviewport` factors this
into `sync_content()` called from both `realize` and `change_managed`.

**File:** `wbuild_widgets/Wviewport.widget`

## Composite `insert_child`/`delete_child` overrides MUST call the inherited method

**Symptom:** a Constraint/Composite that overrides `insert_child` gets **no
children at all** in `$children`/`$num_children`; `page_at()`/loops find nothing.

**Cause:** the default `XtInheritInsertChild`/`XtInheritDeleteChild` are what add
(or remove) the child from `composite.children`. An override that does its own
bookkeeping and returns replaces that behaviour.

**Fix:** call `#insert_child(child)` / `#delete_child(child)` in the override
(see `WPaned.widget`). Do it before any early `return`, so internal children are
still registered.

**File:** `wbuild_widgets/WtabWidget.widget`

## Do not call `XtRemoveEventHandler` from a widget's `destroy`

**Symptom:** on teardown, `XtRemoveEventHandler(widget, ...)` in a `destroy`
method segfaults inside `XtBuildEventMask` (NULL/garbage `core.event_table`).

**Cause:** `XtBuildEventMask` walks the widget's event table; during
`Phase2Destroy` the widget is already being torn down. Event handlers live in the
widget's own `core.event_table` and are released with the widget, so removing
them in `destroy` is both unnecessary and unsafe.

**Fix:** let Xt release handlers with the widget. Only remove a handler at
runtime when it is attached to a **live** widget that you are swapping out
(`set_content()` does this for the previously-scrolled content). Xaw's Viewport
registers handlers and never removes them in `destroy` either.

**File:** `wbuild_widgets/Wviewport.widget`

## ConstraintClassPart fields are `initialize`/`destroy`, not `constraint_*`

**Symptom:** `error: 'ConstraintClassPart' has no member named
'constraint_initialize'` when manually registering constraint procs.

**Cause:** `wbuild`'s generated comments label the slots
`constraint_initialize`/`constraint_destroy`, but the actual libXt struct members
are `resources`, `num_resources`, `constraint_size`, `initialize`, `destroy`,
`set_values`, `extension`. A constraint `initialize` is an `XtInitProc`
(`void (*)(Widget, Widget, ArgList, Cardinal*)`), `destroy` is an `XtWidgetProc`
(`void (*)(Widget)`).

**Fix:** assign `cwc->constraint_class.initialize = ...` /
`.destroy = ...` in `class_initialize` (mirror `WPaned.widget`).

**File:** `wbuild_widgets/WtabWidget.widget`

## Constraint-owned String titles: one owner, idempotent free on both teardown paths

**Symptom:** leaking the owned title when the shell is destroyed, or a double
free / glibc abort when a page is destroyed individually.

**Cause:** for an individually destroyed child libXt runs **both** the parent's
`delete_child` **and** the constraint `destroy` (delete_child first); for
whole-shell teardown only the constraint `destroy` runs (descendants are freed by
`Phase2Destroy` without calling `delete_child`). Also the constraint record is
`XtMalloc`'d, so `tabTitle_mem` starts as garbage unless zeroed.

**Fix:** a `constraint_class.initialize` that sets the owned pointer to NULL, and
one idempotent free-and-NULL helper called from both `delete_child` and the
constraint `destroy`. Never walk `$children` in the parent's own `destroy`
(children are already freed).

**File:** `wbuild_widgets/WtabWidget.widget`

## `XtWidth`/`XtHeight`/`XtX`/`XtY` are not in libXt (Xaw defines them privately)

**Symptom:** `implicit declaration of function 'XtWidth'` (etc.) when compiling a
generated widget.

**Cause:** these are Xaw convenience macros (`experimental/libxaw/src/Private.h`),
not part of the installed libXt headers.

**Fix:** use the core fields directly: `child->core.width`, `child->core.height`,
`child->core.x`, `child->core.y`.

**File:** `wbuild_widgets/Wviewport.widget`

## String-typed theme values (fonts)

**Symptom / feature:** the theme store only held colours and integers; font
names needed a third value kind.

**Fix:** `struct theme_value` gained `char *sval; int is_str;`. `theme_save`
**always quotes** strings; the parser treats any double-quoted token as a string
(spaces allowed, no escapes) and any *other* bare token as a string too. The
int predicate is now `!is_color && !is_str`. Invariant: string values contain no
`"`, `\`, or newline. `theme_get_str()` returns a borrowed pointer owned by the
active map (invalidated by `theme_select`/`theme_destroy`).

**Files:** `utils/theme.[ch]`, `utils/theme_parse.[ch]`

## Xft widgets that do not inherit `Wheel` need their own `XftDraw` and font

**Symptom:** compile error (`$draw` undeclared) or a missing tab-label font.

**Cause:** `Constraint`/`Composite` -> `Core` has no `draw` field and no font
resource; `Wheel` provides both, but a Constraint-based widget does not inherit
them.

**Fix:** declare `@var XftDraw *draw` (create in `realize` with `XftDrawCreate`,
destroy with `XftDrawDestroy`), register the Xft string converter with
`converters_xft_init()` in `class_initialize`, and keep the font as an Xt-owned
converter resource (`@var <XftFont> XftFont* f = <String> "Sans-11"`) plus an
optional self-opened theme font (`xft_fontopen`) that you close yourself. Never
`XftFontClose` the Xt-owned converter font (Xt frees it via the converter cache).

**File:** `wbuild_widgets/WtabWidget.widget`

## Stale binary: the launcher never rebuilt after the first build

**Symptom:** after editing a `.widget` (or any library source), running a demo via
`./t1.sh` still uses the **old** widget code — e.g. a crash that is already fixed
in the sources keeps happening ("Segmentation fault" on resize/exit), because the
linked `commander_runner` still embeds the previous `libxtcw.a`.

**Cause:** `start.sh` only ran `make all` when a library or the runner binary was
*missing*; if they existed it skipped the build, and the wbuild-generated sources
and static archives have **no automatic dependency tracking**. So editing a
`.widget` never rebuilt `libxtcw.a`, and `make` was never invoked again.

**Fix:** `start.sh` now always runs an incremental `make all` (which
regenerates changed widgets, rebuilds the libraries and relinks the runner).
Set `START_SKIP_BUILD=1` to skip it. After changing a **base class's method set**
(ClassPart layout), still do `make clean && make` — incremental relinking cannot
fix stale subclass objects (see the earlier entry).

**Files:** `start.sh`

## MLS_DEBUG analyzer reports "List base address ... is not allocated" during Wbravo destroy

**Symptom:** Under `MLS_DEBUG` builds, destroying a `Wbravo` editor widget prints
the "POST MORTEM ANALYSER" block and e.g.
`ERROR in funtion: '_m_free'. ... List base address for handle N is not allocated`,
called from `node_free_tree` or `ptable_free`. The tests still pass and the text
buffer/tests are correct.

**Cause:** The MLS debug analyzer (`_mlsdb_check_handle` in `utils/mls.c`) inspects
a handle while running the debug-only free path and sees a base pointer that is
already NULL. Repeating plain layout+free or piece-table+layout churn in a tight
loop 200-300x does **not** reproduce it, so it is tied to the widget's interleaved
buffer/layout lifecycle rather than a simple double free in the library. It is
emitted by the debug analyzer (which only prints), not by `_get_list` (which
would `exit(1)`), so release builds do not run the analyzer.

**Status:** Open / debug-only. The functional widget tests (`tests/test_wbravo.c`)
pass. Investigate the MLS handle/uaf bookkeeping when a long-lived node handle is
touched during heavy `ptable_to_utf8` + `retex_layout_plain` churn.

**Files:** `wbuild_widgets/Wbravo.widget`, `utils/mls.c`, `re-tex/src/node.c`

## opencode run: `--file` is an array and swallows the positional prompt

**Symptom:** `opencode run --file shot.png "Inspect this image"` fails with
`Error: File not found: Inspect this image ...` and the model never receives the
message.

**Cause:** the `-f/--file` option is declared as an array (`[array]`) and
greedily consumes all following non-option arguments as file paths. A positional
prompt placed after `--file` is treated as a second filename.

**Fix:** put the message before `--file`, or terminate option parsing with `--`:

```bash
opencode run --model deepseek/deepseek-flash --agent plan --file "$png" -- "$prompt"
```

**File:** `tools/widget_shot.sh`

## MLS_DEBUG: use-after-free post-mortem hid the creation site of the handle

**Symptom:** When a freed list handle was used, the debug build printed

```
ERROR: ... List 4 not allocated
POST MORTEM ANALYSER STARTED
ERROR in funtion: '_mls'. Called by 'main:24' ...
Checking Handle 4, uaf protection: 0
List base address for handle 4 is not allocated
```

but not the source line that **created** handle 4, so the offending allocation
could not be located. This is the situation that made the `Webravo` editor bug
hard to debug: a list handle was freed and later used, and the post-mortem only
said "not allocated".

**Cause:** Two problems in `utils/mls.c`:

1. `_mlsdb_check_handle()` returned as soon as `ML[h]` was `NULL` (which is the
   normal state after `m_free`). The `lst_owner` record — which holds the
   creation site — was consulted only *after* that check.
2. `_m_free()` overwrote the creation site (`o->ln`, `o->fun`, `o->fn`) with the
   free site (negative line number), so the creation line was lost even when the
   record was read.

**Fix:** `lst_owner` now keeps allocation and free information separately
(`freed`, `free_ln`, `free_fn`, `free_fun`). `_mlsdb_check_handle()` reports

```
Array was created by <fun>() in <file>:<line>
Array was previously freed by <fun>() in <file>:<line>
```

before validating the base pointer. Trace level 1 logs both directions with
source location: `NEW LIST <h> allocated by ...` and
`FREE LIST <h> freed by ...`.

**How to trace a known handle H:** set the global before doing any MLS work,
restart, and grep the log for `List H` / `NEW LIST H`:

```c
extern int trace_level;
trace_level = 1;   /* before m_init(): show every NEW LIST / FREE LIST */
```

If the program aborts, the post-mortem also names the failing call site, the
creation site and (when applicable) the free site.

**Regression test:** `tests/test_mls_trace.c`

```bash
make tests
./tests/test_mls_trace          # self-checking, exit 0 on success
./tests/test_mls_trace --demo   # raw output; exits 1 on the intentional UAF
```

**Files:** `utils/mls.c`, `tests/test_mls_trace.c`

## Wbravo: `realize` created the window with an empty event mask (no keyboard, blank screen)

**Symptom:** a `Wbravo` editor showed a fully black/blank body; typing did
nothing and clicking did not move the caret. The header `Wlabel` rendered
normally, and `tests/test_wbravo` still passed because it drives the actions
directly with `XtCallActionProc` (it never exercises real key/button/expose
events).

**Cause:** `Wbravo.realize` called

```c
XtCreateWindow($, InputOutput, CopyFromParent,
               (XtValueMask) 0, (XSetWindowAttributes *) NULL);
```

instead of forwarding the `mask`/`attributes` that libXt computes in
`ComputeWindowAttributes()` and passes to the realize proc. With mask `0` the
window selected **no** `KeyPress`/`ButtonPress`/`Expose`/`FocusIn`/`FocusOut`
events — so the translation table never fired (no keyboard, click-to-focus
dead) and the widget was never repainted after mapping (blank/black). With no
`CWBackPixel` the contents were also undefined. Every working interactive
widget (`EqFader`, `Gauge`, `Wedit`, `WeditMV`, and Core's own `CoreRealize`)
passes `*mask, attributes`.

**Fix:**

```wbuild
XtCreateWindow($, (unsigned int) InputOutput, (Visual *) CopyFromParent,
               *mask, attributes);
```

Do **not** use `#realize($, mask, attributes)` here: `Wheel`/`Core` define no
realize proc, so the inherited default is empty and no window would be created.
Add a regression assertion in `tests/test_wbravo.c` that the realized window's
`your_event_mask` contains `KeyPressMask | ButtonPressMask | ExposureMask`.

**Files:** `wbuild_widgets/Wbravo.widget`, `tests/test_wbravo.c`

## Wretex: `realize` created the window with an empty event mask (black body)

**Symptom:** every `Wretex` widget rendered as a black rectangle; the text and
inline SVGs were laid out but never painted. `demo_wretex` showed five black
panes under a normal `Wlabel` header. The layout/colour state was correct
(`text`, `fg_norm`, `bg_norm` and `xft_col[]` all had the right values).

**Cause:** `Wretex.realize` called

```c
XtCreateWindow($, InputOutput, CopyFromParent,
               (XtValueMask) 0, (XSetWindowAttributes *) NULL);
```

so the window selected no `Expose` events (and no background pixel). The
initial `redraw_wretex()` ran before mapping, and no expose followed to copy
the pixmap again. This is the same realize bug that was fixed in `Wbravo`.

**Fix:** forward the libXt mask/attributes:

```wbuild
XtCreateWindow($, (unsigned int) InputOutput, (Visual *) CopyFromParent,
               *mask, attributes);
```

`demo_wretex` now paints its text and inline SVG icons; `t1_menu` uses the
fixed widget for rich demo descriptions.

**Files:** `wbuild_widgets/Wretex.widget`

## EqFader: font resources were not converted when created first

**Symptom:** creating an `EqFader` as the first widget in a process left
`xftFont`/`scaleFont` pointing at the resource *strings* instead of
`XftFont*`; drawing then read garbage metrics. The demo worked only because a
`Wlabel`/`Wbutton` was created first and registered the Xft converters.

**Cause:** `EqFader` had no `class_initialize`. Xt does not automatically run
`Wheel.class_initialize` for subclasses, so `converters_xft_init()` (and the
other converters) were never registered before the font resources were
converted.

**Fix:** add `EqFader.class_initialize` with `converters_init(); m_init();
conststr_init();` (same pattern as `Wlabel`). `tests/test_eqfader.c` creates an
`EqFader` as the first widget and checks that the separate `scaleFont` is a
valid, smaller `XftFont`.

**Files:** `wbuild_widgets/EqFader.widget`, `tests/test_eqfader.c`

## WListView: empty area below a short list took the last row's colour

**Symptom:** in a short `WListView` (e.g. the four-row trace selector) the
empty area below the rows sometimes turned light grey after clicking rows, and
another click made it disappear.

**Cause:** `render()` clears the whole backing pixmap with the shared `$gc`, but
`draw_header()` and `draw_rows()` change that GC's foreground and leave it at
the **last drawn row's** background. The next `render()` therefore filled the
entire pixmap with `C_SEL_BG`/`C_ALT_BG` instead of `C_BG`; the rows were
repainted over it, but the empty area below them kept the stale colour. Which
row was last depended on the selection, hence the flicker between clicks.

**Fix:** reset `$gc` to `$col_px[C_BG]` immediately before the full-pixmap
`XFillRectangle()` in `render()`.

Regression coverage in `tests/test_wlistview.c`: switch to a three-row list,
select the last row, force a refresh and assert the pixel below the rows is
`C_BG`.

**Files:** `wbuild_widgets/WListView.widget`, `tests/test_wlistview.c`

## WListView: dragging the scrollbar thumb stopped about halfway

**Symptom:** dragging the vertical scrollbar thumb to the bottom stopped at
roughly 50 % of the list; the mouse wheel and keyboard scrolled all the way to
the end. The same applied to the horizontal bar.

**Cause:** `sync_scrollbars()` maps the content offset to the slider with the
*total* extent:

```c
pos = top_y * SCROLL_RES / total_h;   /* total_h, not maxy */
```

but `vscroll_cb()` inverted it with `maxy`:

```c
top_y = pos * maxy / SCROLL_RES;
```

At `pos = RES - frac` this yields `maxy * maxy / total_h`, i.e. about half when
half the list is visible. The wheel path (`scroll_by`) clamps directly to
`maxy`, so it was unaffected.

**Fix:** invert with the same extent used by `sync_scrollbars()`:

```c
top_y = pos * total_h / SCROLL_RES;   /* hscroll: total_w */
```

Regression coverage in `tests/test_wlistview.c` calls the slider's `sl_motion`
action at the bottom and asserts `top_y` reaches `maxy`.

**Files:** `wbuild_widgets/WListView.widget`, `tests/test_wlistview.c`

## VSlider/HSlider: theme colours ignored, scrollbar colour changed between runs

**Symptom:** the list scrollbar kept its initialize-time colour (yellow, green
or red depending on the run) instead of the theme's colour. `WListView`
called `XtVaSetValues(vsb, "foreground", ..., "background", ...)`, but the
drawn bar did not change.

**Cause:** `VSlider.set_values()`/`HSlider.set_values()` only reacted to
`pos`/`frac`; the `foreground`, `background` and `lineWidth` resources were
ignored, so the `gc_fg`/`gc_background` GCs created in `initialize()` were
never rebuilt. The colour seen at runtime depended on the resource values at
widget creation, which is why it changed between runs.

**Fix:** `set_values()` now detects colour/line-width changes, releases and
recreates both GCs, invalidates the cached slider rectangle (`$sly = -1`) and
repaints. `tests/test_wlistview.c` verifies that changing the slider colours
replaces the GCs and that the new GC foreground matches.

**Files:** `wbuild_widgets/VSlider.widget`, `wbuild_widgets/HSlider.widget`,
`tests/test_wlistview.c`

## Wbravo: own `$xcol[]` cache must follow theme changes

**Symptom:** after the realize fix above, a `Wbravo` rendered with its
initialize-time colours even after the active theme changed; the rest of the
app (Wlabel, Gridbox, …) re-coloured but the editor body kept the old
foreground/background.

**Cause:** `Wbravo` caches its four colours in a private `$xcol[4]`
(`COL_BG/FG/SELBG/SELFG`) plus `$blit_gc` in `alloc_colors()`. It inherited
`Wheel.colors_changed`, which only updates Wheel's own `$pixel[]`/`$xft_col[]`
and never touches `$xcol[]` (this is a concrete case of the general
"widget-owned colours/caches are not covered by Wheel.colors_changed" bug).

**Fix:** override the hook (no parameter list — it is an override) and
re-resolve the cache:

```wbuild
@proc void colors_changed
{
    #colors_changed($);
    free_colors($);
    alloc_colors($);
    $dirty = 1;
    if (XtIsRealized($)) redraw($);
}
```

**Files:** `wbuild_widgets/Wbravo.widget`

## wbuild/Xt: superclass `initialize`/`destroy` are auto-chained by libXt

**Not a bug — a rule to remember.** A subclass `@proc initialize` (or
`@proc destroy`) does **not** need to call `#initialize($)` / `#destroy($)`:
libXt's `CallInitialize`/destroy walk calls each class in the hierarchy from
base to derived. That is why a `Wheel` subclass gets `Wheel.initialize`
(`theme_register`, `$background_pixel`, `col_set`) and `Wheel.destroy`
(`theme_unregister`) automatically even though its generated C contains only
its own body. (Verified: `theme_register`/`theme_unregister` fire for a plain
`Wbravo`, with the `Wheel.widget` frames in the backtrace.)

Only `initialize`/`destroy` are chained; `set_values` and other virtual methods
are **not** — an override replaces the superclass implementation, so call
`#set_values($)` explicitly if you rely on the base behaviour.

**Files:** `wbuild_widgets/Wbravo.widget` (informational)

## Wbravo: caret drawn too high — double `px2pt` on the vertical offset

**Symptom:** pressing Down (or Up) in a `Wbravo` moved the caret only "a little
bit" — less than a full line — and the gap grew with every line; clicking a
line placed the caret above it. The document caret index was correct (typing
landed on the right line), only the drawn caret was misplaced.

**Cause:** `draw_caret_in_cache()` drew the caret at

```c
px2pt($, (double)$topGap) + px2pt($, cy - y0)
```

but `cy` (`retex_paragraph_get_caret_pos` → `*y = ln->y`) and `y0`
(`retex_paragraph_get_line` → `l0.y`) are already in **points** — `RetexLine.y`
is accumulated with `TO_DOUBLE()` (1 pt = 65536 sp, `re-tex/src/scaled.h`).
The extra `px2pt` multiplies by `72/dpi` (~0.75 at 96 dpi), so the caret was
drawn at ~75 % of each line's offset and drifted further behind line by line.
The x coordinate on the line above is the tell: it uses `cx` (points) directly.

**Fix:** do not re-convert — `px2pt($, (double)$topGap) + (cy - y0)`. The
renderer path (`retex_paragraph_render_range` → `y + (ln->y - y0) + ...`) uses
the same point-space convention.

**Files:** `wbuild_widgets/Wbravo.widget` (`draw_caret_in_cache`)

## WtabWidget: content frame is white/unpainted on left/right/bottom

**Symptom:** the top tab strip is drawn in the theme frame colour, but the
border around the content area (left/right/bottom) shows the Xt default
background (white) instead of the frame colour, so the content looks unenclosed
("broken layout"). Confirmed visually with `tools/widget_shot.sh`.

**Cause:** `draw_strip()` only fills the top tab strip `(0,0,width,tabHeight)`.
The left/right/bottom `borderThickness` band around the `Wviewport` is the
widget's own exposed background, but `WtabWidget` is a `Constraint` subclass
and never sets its Core `background_pixel` (it stays at the Xt default, white).
`wtabwidget_colors_changed()` calls `XSetWindowBackground()` only when realized,
which is too late for the initial theme apply (that runs before realize).

**Fix:** in `resolve_colors()`, set `$background_pixel = $frameColor`. It runs
in `initialize`, `set_values` and `colors_changed`, so the frame is correct at
realize and on theme switch.

**File:** `wbuild_widgets/WtabWidget.widget`

## demo_wtabwidget: auto-exits after 6 seconds (unlike every other demo)

**Symptom:** `./t1.sh wtabwidget` closes the window after ~6 s.

**Cause:** `demos/demo_wtabwidget.lua` contained
`xtapptimeout(6000, function() xtappexitflag() end)`. It is the only demo with
such a timeout; all other demos stay open (the smoke test kills them).

**Fix:** removed the timeout so the demo behaves like the rest.

**File:** `demos/demo_wtabwidget.lua`

## widget_shot.sh: multi-line fragment breaks the output filename

**Symptom:** passing a multi-line LUI fragment produced a screenshot filename
containing the whole fragment with embedded newlines.

**Cause:** the widget class name was derived with `sed` over **all** lines, so
every nested child line was rewritten and the result was a multi-line string.

**Fix:** extract the class from `head -n 1` only and sanitize it to a
filename-safe token.

**File:** `tools/widget_shot.sh`

## Mixing MLS_DEBUG and non-debug objects corrupts the MLS owner bookkeeping

**Symptom:** after adding a new `utils/` module and building it by hand, a test
that frees a list created by that module aborts with
`ERROR: mls.c ... Function: lst Index(n) out of Bounds` and
`Handle H ... out of range`, even though the module's own logic is fine.

**Cause:** under `MLS_DEBUG`, `m_create`/`m_free` are macros wrapping
`_m_create`/`_m_free`, which register a per-handle owner record in the `DEB`
list. If a single object file is compiled WITHOUT `-DMLS_DEBUG` (e.g. by running
`make -C utils` directly — it does not inherit the root makefile's exported
`CFLAGS`), its `m_create` calls skip the owner record, so a later debug
`m_free` indexes `mls(DEB, handle-1)` past the end of the `DEB` list.

**Fix:** always build through the root makefile (it exports `CFLAGS` with
`-DMLS_DEBUG` when `debug_enable=1`). After a manual build of a module:
`rm utils/<mod>.o utils/libutils.a && make core` (then relink the affected
binaries). For a freshly-created test binary, `rm tests/<test> && make tests`.

**Files:** build system (`makefile`, `utils/makefile`); surfaced by
`utils/rtable.c` / `tests/test_rtable.c`.

## Generated `XxxP.h` needs the field-type headers included first

**Symptom:** a white-box test that includes the generated private header
directly fails with `unknown type name 'XtSetValuesFunc'`, `unknown type name
'XftColor'`, or `unknown type name 'RLook'` (and dozens of cascading errors).

**Cause:** wbuild's `XxxP.h` only `#include`s the superclass `P` header; it does
not include the headers needed by its own `@PRIVATE` field types. `WheelP.h`
in turn assumes `<X11/IntrinsicP.h>` and `<X11/Xft/Xft.h>` were included first.
A field of a custom struct type (e.g. Wbravo's `@var RLook look`) needs that
struct's header (`rtable.h`) before `WbravoP.h`.

**Fix:** before `#include "XxxP.h"` include, in order:
`<X11/Intrinsic.h>`, `<X11/IntrinsicP.h>`, `<X11/Xft/Xft.h>`, then any header
defining custom `@PRIVATE` field types, then `<Xxx.h>`, then `<XxxP.h>`.
See `tests/test_rich_wbravo.c`.

**File:** `tests/test_rich_wbravo.c` (worked around).

## WtabWidget tab labels clipped on HiDPI displays (fixed tab-strip height)

**Symptom:** on a HiDPI display the tab titles are cut off at the top — only the
lower half of the glyphs is visible ("the tab buttons are only half the height
they should be"). The same build looks fine on a 96 dpi Xvfb display.

**Cause:** the strip height was the fixed default `tabHeight = 26` and the label
baseline was `tabHeight - lineGap - 2`. A theme font `font_small = "Sans-11"`
under a high Xft dpi renders far larger (measured **ascent 32 + descent 9 = 41 px**
on a 2× GNOME display vs ~11 px on Xvfb), so the glyph tops were drawn above
`y = 0` and clipped.

**Fix:** derive the layout from the real `XftFont` metrics — vertically centre
the label in `tabHeight - (lineGap + underline)` — and grow `tabHeight`
(`ensure_tab_height`) to `ascent + descent + lineGap + underline + padding` when
the font needs more room. Called from `initialize` and
`wtabwidget_colors_changed`.

**File:** `wbuild_widgets/WtabWidget.widget`

## retex: line `end_byte` excluded trailing spaces -> caret jumps to next line

**Symptom:** in a `Wbravo` editor, pressing **space at the end of a short
line** made the caret jump down to the next line; typing any character then
moved it back to the correct line.

**Cause:** `RetexLine.end_byte` was set to the end of the last **word**, so
trailing spaces were not part of any line's source range. A caret just after
the trailing space had a source offset greater than that line's `end_byte`,
so `caret_line()` / `retex_paragraph_get_caret_pos()` attributed it to the
following line. Typing a character extended the word (and thus the line's
`end_byte`), which is why the caret then snapped back.

**Fix:** track a second line end that includes trailing spaces and use it when
the line ends at a hard break or the end of the paragraph; keep the last-word
end for wrapped lines (their trailing spaces are dropped at the wrap point).
Applied to both `retex_layout_plain()` and `retex_layout_styled()`.

**Files:** `re-tex/src/retex.c`. Regression tests:
`tests/test_retex_plain.c` and `tests/test_retex_styled.c`
(`test_trailing_space_caret`).

## Popup shells must be sized before `XtRealizeWidget`

**Symptom:** opening a popup aborts with `Error: Shell widget popup has zero
width and/or height`.

**Cause:** `XtRealizeWidget(shell)` on a freshly created popup shell whose
width/height are still 0 makes Xt raise the fatal "zero width/height" error.

**Fix:** set the shell's size (`XtVaSetValues($shell, XtNwidth, ..., XtNheight,
...)`) **before** calling `XtRealizeWidget($shell)`, then reparent/configure the
content.

**File:** `wbuild_widgets/WmenuButton.widget`

## `XtRegisterDrawable` also runs the target widget's translations

**Symptom:** an outside click on a click-catcher closes a popover and it
immediately reopens.

**Cause:** `XtRegisterDrawable(catcher, widget)` dispatches the catcher's events
to `widget` *as if they were on the widget's own window*, so the widget's
`<Btn1Down>: toggle()` translation fires as well as the catcher's event handler
(close, then reopen).

**Fix:** register the catcher drawable with a widget that has no relevant
translations — use the popup shell (whose only job is to host the content) and
put the ButtonPress event handler there, passing the menu button as
`client_data`.

**File:** `wbuild_widgets/WmenuButton.widget`

## Do not `XtPopdown` a popup shell from the parent's `destroy`

**Symptom:** segfault in `XtWindowOfObject` / `XtPopdown` during teardown of a
widget that owns a popup shell.

**Cause:** `XtDestroyWidget(parent)` destroys popup children itself, and by the
time the parent's `destroy` method runs the shell's window is already going
away; `XtPopdown` then dereferences it.

**Fix:** in `destroy`, only clean up resources Xt does **not** own (here the raw
`InputOnly` click-catcher window). Let Xt destroy the popup shell. Do not draw
into the widget's own window from `destroy` either.

**File:** `wbuild_widgets/WmenuButton.widget`

## Popovers: avoid exclusive grabs (leaked grab = "invisible window eats clicks")

**Symptom:** after a popup/menu bug, the whole app stops responding to the mouse —
clicks go to a widget that is no longer visible.

**Cause:** `XtGrabExclusive` / `XtPopupSpringLoaded` add a grab-list entry that
redirects *all* events to the popup widget. If the popdown is ever skipped, the
grab leaks and every click is swallowed by the (possibly hidden) popup.

**Fix (WmenuButton):** pop up with `XtPopup(shell, XtGrabNone)` (no grab at all)
and catch outside clicks with a full-app `InputOnly` override-redirect window
that is mapped under the popup and **destroyed on every close path and in
`destroy`**. Register it with `XtRegisterDrawable`; close on its ButtonPress.
This gives reliable "click outside / click the button closes" without any pointer
grab that could leak.

**File:** `wbuild_widgets/WmenuButton.widget`

## wbuild: an `@var` with an external struct pointer breaks consumers of `<Widget>P.h`

**Symptom:** Adding e.g. `@var RetexParaFormat *para_fmt` to a `.widget` compiles
the generated `<Widget>.c` (the `@IMPORTS` includes are emitted before the private
header), but any other translation unit that includes `<Widget>P.h` fails with
`error: unknown type name 'RetexParaFormat'`.  `<Widget>P.h` does not include the
widget's `@IMPORTS` headers.

**Cause:** `wbuild` emits `@IMPORTS` only into the generated `.c`, never into the
generated public/private headers, so external types used in `@PRIVATE` fields are
not visible to header consumers.

**Fix:** Store the field as `void *` (the convention already used for `ptable`,
`para_ptr`, `backend_ptr`) and cast to the real type where it is used, or keep the
private field opaque.  Do not expose an external struct type directly unless its
header is included by the public widget header.

**File:** `wbuild_widgets/Wbravo.widget`

## Stale test binary after rebuilding only one library ("mls Not initialized")

**Symptom:** Running a test that was built before a library change aborts with
`ERROR: mls.c ... Not initialized` before any test output, even though the test
calls `m_init()`.

**Cause:** The test links the static `libretex.a`/`libutils.a` at link time.  If
one archive was rebuilt with different flags (e.g. a manual
`make -C re-tex/src` picked up the makefile's fallback `CFLAGS` instead of the
root makefile's exported `-DMLS_DEBUG`/include paths) while an already-built test
binary kept the old objects, the mixed `MLS_DEBUG` translation units disagree
about the mls bookkeeping and the handle check fails.  The `tests/Makefile` has
no header-dependency tracking, so the binary is not relinked.

**Fix:** Rebuild through the root makefile, which exports one consistent
`CFLAGS`, and force relinking after a library change:
`rm -f tests/<test> && make tests`.  Avoid invoking `make -C re-tex/src`
directly with hand-written flags during a debug build.

**Files:** `tests/Makefile`, `re-tex/src/makefile`, root `makefile`

## rtable: serialized undo/redo snapshots reference add-buffer atoms past `add_len`

**Symptom:** saving an editor snapshot after an undo and loading it back failed
(`rtable_deserialize` returned an error / the restored document was empty).

**Cause:** the add buffer is append-only. `rtable_undo()` restores the logical
`add_len` from the snapshot but does not truncate the buffer, so a *redo*
span can legitimately have `span.add_len > table.add_len` (it references atoms
that are still physically present but were "removed" by the undo). A
serializer that writes only `table.add_len` atoms loses those bytes and the
span fails validation on load.

**Fix:** `rtable_serialize()` computes `max_add` over the current state and all
undo/redo spans, writes both `add_len` (current logical) and `max_add`
(physical), and saves `max_add` atoms. `rtable_deserialize()` restores
`add_len = current` but `add_cap = max_add`, matching the piece-table
invariant that stale add-buffer atoms remain available to redo.

**Files:** `utils/rtable.c` (`rtable_serialize`, `rtable_deserialize`)

## Wcheckbox: duplicate `callback` resource caused a double free in Xt destroy

**Symptom:** a `WfileChooser` containing a `Wcheckbox` crashed at
`XtDestroyWidget` with `double free or corruption (fasttop)`. Valgrind showed
the callback list allocated by `XtAddCallback(wcheckbox, "callback", ...)`
being freed twice inside libXt.

**Cause:** `Wcheckbox` redeclared `@var <Callback> XtCallbackList callback = NULL`
even though its superclass chain (`Wlabel` -> `Wheel`) already provides the
`callback` resource. wbuild emitted a second subclass resource with the same
name/offset (`wheel.callback`), so Xt managed the same callback-list pointer
twice and freed it twice during `ObjectDestroy`.

**Fix:** remove the duplicate `callback` declaration from `Wcheckbox.widget`;
`toggle` already calls `XtCallCallbackList($, $callback, ...)` on Wheel's
inherited field. valgrind is clean afterwards.

**File:** `wbuild_widgets/Wcheckbox.widget`

## Composite with managed slider children: zero geometry aborts realize

**Symptom:** creating a `WListView` aborted during realize with
`Error: Widget vsb has zero width and/or height` (or `hsb`), and the process
exited 1.

**Cause:** the widget created `VSlider`/`HSlider` children in `initialize`,
then `realize` called `#realize` and explicitly realized the children before
`relayout()` had assigned their geometry. Xt refuses to realize a managed
child with 0x0 size.

**Fix:** in `realize`, call `relayout()` after the superclass realize and let
Xt realize the managed children afterwards; never configure a hidden slider
with 0 width/height (use at least 1x1 while it is unmapped).

**File:** `wbuild_widgets/WListView.widget`

## wbuild: `@var ArrayInt` needs `converters.h` in the generated header

**Symptom:** consumers that include `<Widget>P.h` fail with
`unknown type name 'ArrayInt'`.

**Cause:** `ArrayInt` is a typedef from `converters.h`, but `wbuild` emits
`@IMPORTS` only into the generated `.c`, not into the public/private headers.

**Fix:** add `@EXPORTS` with `@incl "converters.h"` so the public header is
self-contained (done for `WListView`).

**File:** `wbuild_widgets/WListView.widget`

## WListView: resizing grew the scrollbar but not the list body

**Symptom:** resizing the `WListView` demo enlarged the vertical scrollbar (and
the widget), but the rows/header stayed at the old size; the extra area was
blank.

**Cause:** `rebuild_cache()` decided whether to recreate the backing pixmap
with `if ($pixmap && (int)$width <= w && (int)$height <= h)`, where `w`/`h`
were just the clamped `width`/`height`. That condition is true for every
normal resize, so the pixmap was only ever created once at the initial size;
`render()` then drew into the stale small pixmap and `blit()` copied
`width x height` out of it. The slider children are separate X windows sized
by `relayout()` via `XtConfigureWidget`, so they grew correctly.

**Fix:** track the actual cache size in private `cache_w`/`cache_h` fields and
recreate the pixmap + `XftDraw` whenever they differ from the clamped window
size. `relayout()` already re-derives the list area each resize.

**File:** `wbuild_widgets/WListView.widget` (`rebuild_cache`)

## WListView: cached SVG icons used a stale drawable after resize

**Symptom:** after fixing the cache resize, resizing the demo aborted with
`X Error of failed request: BadDrawable ... X_PutImage`.

**Cause:** the per-filename SVG icon cache stores an `XImage2` whose `d`
(drawable) field was set when the icon was first rasterised. Recreating the
backing pixmap on resize destroyed that drawable, but `xim2_put_image()` uses
`im->d`, so the next repaint put the image into the destroyed pixmap.

**Fix:** set `im->d = $pixmap` immediately before `xim2_put_image()` in
`draw_icon()` (XPM icons already copy into `$pixmap` directly).

**File:** `wbuild_widgets/WListView.widget` (`draw_icon`)

## WListView: rows painted over the header while scrolling with the slider

**Symptom:** dragging the vertical scrollbar made list rows overwrite the
column header ("Name / Size / Type"); wheel/keyboard scrolling looked fine.

**Cause:** the slider produces a fractional `top_y`, so the first visible row
starts at `y = list_y - (top_y mod row_h)`, i.e. *above* the header boundary.
`draw_rows()` painted the row background over the full row height at `y`, and
the row's icon/grid line used the same unclamped coordinates, so they drew
over the header (which is rendered first). Wheel/keyboard scrolling moves by
whole rows, which is why only the scrollbar reproduced it.

**Fix:** clip every row to the list body `[list_y, list_y + list_h)`:
background fill, text clip rectangle and grid line are clamped to the visible
band; icons are skipped when their rect is not fully inside the band (they use
`XCopyArea`/`xim2_put_image`, which are not bounded by the `XftDraw` clip).
Also use `list_header_bg` for the header fill instead of the body background
GC, so the header colour from the theme is actually applied.

**File:** `wbuild_widgets/WListView.widget` (`draw_rows`, `draw_icon`,
`draw_header`)

## Gridbox shrank fixed rows/columns when the box was smaller than preferred

**Symptom:** in the `WListView` demo, the first frame showed only about half of
the bottom status label. Resizing the window made the label fully visible.

**Cause:** `layout()` treated growth and shrink asymmetrically. When the
gridbox was larger than its preferred size, extra space went only to weighted
cells; when it was *smaller*, the `excess < 0` branch shrank **all** rows/
columns proportionally, ignoring `weighty`/`weightx`. A tall preferred height
from the weighted list therefore squeezed the fixed (`weighty 0`) label too.

**Fix:** added `shrinkCells()` in `wbuild_widgets/gridbox_core.c`. On a shortfall it
first shrinks weighted cells, proportional to weight and down to
`defaultDistance*2+1`; the old proportional-all behaviour remains as a second
pass when weighted cells cannot absorb the deficit (e.g. no weighted children,
or an unweighted-only grid that a parent refuses to grow). Applied to both the
column and row paths.

**File:** `wbuild_widgets/gridbox_core.c`

## Borrowed String resources: use-after-free after Lua GC

**Symptom:** in `demo_filechooser`, double-clicking `commander_runner.c` aborted
with an AddressSanitizer `heap-use-after-free` in `strcmp`, reached from
`cairo_select_font_face` → `xpixmap_set_font_face` → `retex_layout_styled_paras`
→ `Wbravo.relayout`. The freed 29-byte block had been allocated by Lua/lpeg.

**Cause:** Xt String resources are **borrowed** pointers. LUI passes Lua strings
(e.g. `:fontFace "Sans"`), and `Wbravo` kept `$fontFace` pointing at that Lua
string. After the Lua chunk returned, GC could collect it; the next relayout
(triggered by `set_text` after choosing a file) used the dangling pointer.
`WfileSelector`/`WfileChooser` (`pattern`, `folderIcon`, `fileIcon`,
`directory`) and `WListView.fontFace` had the same exposure.

**Fix:** copy borrowed String resources that are used after creation into owned
buffers and free them on destroy / when set_values sees a new value:
`Wbravo`/`WListView` own `fontFace` (`fontFace_mem`); `WfileChooser` owns
`pattern`/`folderIcon`/`fileIcon`; `WfileSelector` owns
`directory`/`pattern`/`folderIcon`/`fileIcon`.

`WfileSelector` cannot override `destroy`: `#destroy($)` expands to
`wbuttonClassRec.core_class.destroy(self)`, and that slot is `NULL` for
`Wbutton`. It registers an `XtNdestroyCallback` instead to free the copies.

**Files:** `wbuild_widgets/Wbravo.widget`, `WListView.widget`,
`WfileChooser.widget`, `WfileSelector.widget`

## Lowering `Wheel.xftFont` default destabilises the legacy WlistMulti renderer

**Symptom:** after changing `Wheel`'s `xftFont` default from `"Sans-22"` to
`"Sans-13"` (whole-toolkit font unification), `test_wlistmulti` aborted with an
MLS handle corruption in `s_split()` while `render_retex_row()` was drawing
retex cells (`Index ... out of Bounds`).

**Cause:** not fully root-caused; the legacy `WlistMulti` retex-cell path is
sensitive to the default font / derived line height (the widget opens its own
font in `initialize` but still shares Wheel's resource/font cache). Kept as a
known fragility.

**Fix / workaround:** keep `Wheel.xftFont = "Sans-22"` for the legacy widgets.
The modern text widgets resolve `font_ui` (`"Sans-13"`) themselves via the
theme helpers (`theme_font_spec`/`theme_font_parts`), so the file chooser,
labels, buttons, inputs and `WListView` are consistent without changing the
legacy default. Migrate `WlistMulti` before lowering the Wheel default.

**Files:** `wbuild_widgets/Wheel.widget`, `WlistMulti.widget`

## WlistMulti retex-cell renderer intermittently corrupts the MLS free list

**Symptom:** `tests/test_wlistmulti` occasionally aborts under `MLS_DEBUG` with
`Index(...) out of Bounds` from `lst`, while `render_retex_row()` is drawing
cells; `_m_put` then reports handle 7 created by `render_retex_row`. The same
binary passes other runs, so it is a heap-layout-dependent corruption.

**Cause:** not root-caused. The legacy `WlistMulti` retex-cell path creates/frees
per-row `columns` MLS lists (`m_create`/`s_split`/`m_free_strings`/`m_free`) and
the MLS debug free list occasionally ends up with a bad handle. Unrelated to the
modern `WListView`/`WfileChooser` path.

**Status:** open / legacy. Keep `WlistMulti` at its existing font size (22) and
do not rely on `test_wlistmulti` as a hard gate until the renderer is reworked.
The LAYOUT baseline is also environment-dependent (see `todo.md`).

**Files:** `wbuild_widgets/WlistMulti.widget`

## WfileChooser: empty/filter/error states never reached the status line

**Symptom:** an empty directory and a filter with no matches still showed a
blank status label; `ok_cb` without a selection was a silent no-op.

**Cause:** `update_status()` compared the MLS *handle* in `rows` with `0`
instead of `m_len(rows) == 0`. `rows` is a non-zero handle even when the list is
empty, so every empty state took the "content present" branch.

**Fix:** count `m_len($rows)` and render "This folder is empty.", "No files
match the filter." or the open error from that count. `ok_cb` now writes
"Select a file to open." and keeps the dialog open.

**File:** `wbuild_widgets/WfileChooser.widget`

## WfileChooser: `selectDirs` navigated into the selected directory

**Symptom:** with `selectDirs=True`, selecting a folder and pressing Open
entered the folder instead of accepting it; only the "current directory"
fallback worked when nothing was selected.

**Cause:** `ok_cb` called `activate_row()`, whose directory branch always
navigates. There was no separate accept path for the primary action.

**Fix:** split `accept_path()`/`accept_current()` from `activate_row()`. Open
accepts a selected directory when `selectDirs` is set; double-click/Return still
navigates.

**File:** `wbuild_widgets/WfileChooser.widget`

## WfileChooser: uninitialised `stat` fields for unreadable entries

**Symptom:** a broken symlink or an entry whose `stat()` failed produced garbage
Size/Modified values.

**Cause:** `struct stat st` was used for `st_size`/`st_mtime` even when
`stat()` returned non-zero.

**Fix:** track `have_stat` and use zero/"-" for size and date when it fails.

**File:** `wbuild_widgets/WfileChooser.widget`

## WfileChooser: special filenames broke activation

**Symptom:** a file named `@notes.txt` opened the wrong path (the leading `@`
was treated as WListView's icon marker); names containing `|`, tab or newline
could corrupt the row or the name lookup.

**Cause:** `row_name()` re-parsed the display cell and stripped the icon prefix,
and the row was a tab-separated string with no escaping.

**Fix:** keep an owned `names` list parallel to `rows` and use it for
activation. The display cell sanitises tab/newline/CR and prefixes a space when
a leading `@` is not an icon marker.

**File:** `wbuild_widgets/WfileChooser.widget`

## WfileChooser: filter text pointed into Wedit's internal buffer

**Symptom:** a use-after-free was possible after typing a long filter and then
refreshing the directory.

**Cause:** `filter_cb()` assigned Wedit's borrowed `label` pointer to
`$pattern`; Wedit owns and may reallocate that buffer.

**Fix:** copy the filter text into the owned `pattern_mem` buffer and point
`$pattern` at the copy.

**File:** `wbuild_widgets/WfileChooser.widget`

## WfileSelector: `selectedPath` borrowed chooser-owned memory

**Symptom:** the selector's `selectedPath` could dangle after the chooser made
its next selection or was destroyed.

**Cause:** `accept_cb()` assigned the chooser's `selected_mem` pointer directly
to the selector's resource.

**Fix:** the selector keeps its own `selected_mem` copy (freed by the destroy
callback) and passes that copy to its callback.

**File:** `wbuild_widgets/WfileSelector.widget`

## WfileSelector: redeclaring the inherited `callback` resource

**Symptom:** `XtDestroyWidget()` aborted with `double free or corruption
(fasttop)` after adding the selector cancel callback.

**Cause:** `WfileSelector` redeclared `@var <Callback> XtCallbackList callback`
although `Wheel` already provides it. Xt then managed the same callback-list
pointer twice (the same failure mode as the earlier Wcheckbox bug).

**Fix:** remove the duplicate declaration; the inherited `callback` resource is
used as before. Only the new `cancel` callback is declared.

**File:** `wbuild_widgets/WfileSelector.widget`

## WfileSelector: dialog was modeless and keyboard-incomplete

**Symptom:** the file chooser was a plain top-level window with no parent
transient, no Escape/WM-close handling, no initial focus and no focus restore.

**Cause:** `XtVaCreatePopupShell(topLevelShellWidgetClass)` + `XtGrabNone`, and
no keyboard handlers.

**Fix:** use `transientShellWidgetClass` + `XtNtransientFor`, `XtGrabExclusive`,
`WM_DELETE_WINDOW` handling, `focus_default` on open, focus restore on close,
and Escape handlers on every focusable child of the chooser (Xt does not walk
from the focused child to the composite's translation table).

**Files:** `wbuild_widgets/WfileSelector.widget`,
`wbuild_widgets/WfileChooser.widget`

## WfileChooser: column model arrays leaked

**Symptom:** `_m_destruct()` reported the `columnWidths`/`columnAligns`/
`columnKinds` arrays from `do_layout()` as still allocated.

**Cause:** the temporary MLS arrays were passed to `WListView` but their handles
were local and never freed. WListView reads them again during relayout, so they
must live as long as the chooser.

**Fix:** store the handles in private fields and free them in `destroy()`.

**File:** `wbuild_widgets/WfileChooser.widget`

## Wlabel `useSmallFont`: retex crash was a stale-object ABI mismatch, not a retex bug

**Symptom:** after adding the `useSmallFont` resource to `Wlabel` and using it
on the `WfileChooser` status label, `test_wfilechooser` crashed during shutdown
inside `node_free_tree()`/`renderer_render_text()` with MLS errors such as
`Index(...) out of Bounds` and `Handle out of range`. The crash looked like a
re-tex bug.

**Investigation:** a standalone re-tex stress test (`retex_layout_plain()` +
`retex_paragraph_render()` + `retex_paragraph_free()` for font sizes 8–24,
empty strings, wrapping, newlines and UTF-8, repeated 200 times) passes. A
minimal `Wlabel` with the default theme, font size 11 and repeated label changes
also passes. The crash only reproduced in the full widget library after an
incremental build.

**Cause:** adding a field to `WlabelPart` changes the size/layout of every
widget that embeds it (`Wbutton`, `Wcheckbox`, `Wframe`, …). The widget build had
no header dependency tracking: `make libxtcw` recompiled only the changed
`Wlabel.c` (and the file-chooser widgets), while `Wbutton.o`, `Wcheckbox.o`, etc.
kept the old struct layout. The resulting ABI mismatch corrupted the heap; the
first visible failure happened in the re-tex renderer. The re-tex engine itself
was not at fault.

**Fix:** generate dependency files for the widget build and include them:

```make
# build/source/makefile
DEPS = $(OBJS:.o=.d)
-include $(DEPS)

# compile with
CFLAGS += -MMD -MP
```

`make libxtcw` now passes `-MMD -MP` and the generated makefile includes the
`.d` files, so changing a widget header rebuilds every dependent widget object.
A clean rebuild (`rm -f build/source/*.o && make libxtcw`) confirms the crash is
gone with `useSmallFont` enabled.

**Files:** `makefile`, `wbuild_widgets/Wlabel.widget`,
`wbuild_widgets/WfileChooser.widget`

## WListView SVG icon cache leaked when the app exited via `os.exit()`

**Symptom:** ASan reported an indirect leak of 1008 bytes in 3 objects
allocated by `nsvg__addShape()` via `nsvgParseFromFile()` →
`xim2_load_svg()` → `WListView.icon_index()`.

**Cause:** the widget cleanup itself was present and correct:
`WListView.destroy()` calls `free_icons()`, which calls `xim2_delete()` and
`nsvgDelete()` for every cached SVG. The leak happened because LUI scripts
commonly call `os.exit(0)` from their `quit_cb`; that bypasses
`XtDestroyWidget()`, so the live widget tree (and its icon cache) was still
reachable when the process exited.

**Fix:**
- `commander_runner.c` now registers `atexit(top_cleanup_atexit)`, which
  destroys `TopLevel` if it is still alive. The normal shutdown path and the
  LUI bootstrap-failure path also destroy the shell explicitly.
- `canvas-draw-cb.c`: `xim2_load_svg()` now calls `xim2_delete()` before parsing
  a new SVG, so reloading into an already-rendered `XImage2` also releases the
  old `XImage`, mask, rasterizer and pixel buffer (previously only `nsvg` was
  freed).

**Verification:** an ASan LUI script that loads `folder.svg`/`file.svg` and
exits via `os.exit(0)` no longer reports any `nsvg`/`xim2` leaks. The remaining
leaks in that run are fontconfig caches, not the icon cache.

**Files:** `commander/commander_runner.c`, `wbuild_widgets/canvas-draw-cb.c`

## Theme registry: `theme_save()` crashes when no themes are loaded

**Symptom:** `theme_save(path)` before any theme is defined (or after
`theme_destroy()`) aborts with `ERROR: mls.c ... _get_list / Not initialized`,
because `theme_save()` calls `m_len(themes)` while `themes == 0`.

**Fix:** `theme_save()` now returns 1 after writing an empty file when
`themes == 0`, checks `ferror()`/`fclose()`, and returns 0 on write failure.

**File:** `utils/theme.c` (`theme_save`)

## Theme parser: `;` only starts a comment after whitespace

**Symptom:** `(theme c (bg_norm 0xff0000;red borderwidth 2;two))` parses
`bg_norm` and `borderwidth` as *strings* (`"0xff0000;red"`, `"2;two"`) instead
of a colour and an integer.

**Cause:** `read_token()` does not treat `;` as a delimiter; `skip_ws()` only
recognises comments at token boundaries.

**Fix:** `read_token()` now stops at `;` as well, so a comment may follow a
value without a space (quoted values already go through `read_quoted()`).

**File:** `utils/theme_parse.c`

## Theme parser: duplicate keys in one theme keep the first value

**Symptom:** `(theme dup (bg_norm 0x111111 bg_norm 0x222222))` resolves
`bg_norm` to `0x111111`; the later assignment is silently ignored, and
`theme_save()` writes both entries.

**Cause:** `append_value()` always appends and `active_lookup()` returns the
first match.

**Fix:** `append_value()` now replaces an existing same-named value in the
current build (last-wins); `active_lookup()` uses the shared `map_lookup()`.

**File:** `utils/theme.c` (`append_value`, `map_lookup`)

## Theme persistence: strings with `"`, `\` or newline corrupt the file

**Symptom:** `theme_str("s", "a\"b")` round-trips as `"a"`; a newline splits
the record. `theme_save()` emits the value verbatim between quotes, and the
parser has no escape sequences.

**Cause:** the documented invariant (`theme.h`) was not enforced.

**Fix:** `theme_save()` escapes `"`, `\`, `\n`, `\r` and `\t`; `read_quoted()`
decodes those escapes. `theme.h`/`theme_parse.h` now document the supported
escapes instead of forbidding them.

**Files:** `utils/theme.[ch]`, `utils/theme_parse.[ch]`

## Theme metrics fall back to the previous value, not the default

**Symptom:** after switching from a theme with `bar_size 20` to a partial theme
that omits `bar_size`, `WListView` keeps 20 instead of the widget default.

**Cause:** `list_apply_theme()` passes the current resource as the fallback:
`theme_metric("bar_size", (int)$barSize)` (same for `cellPaddingX/Y`,
`iconSize`, and `fontSize`).

**Fix:** `list_apply_theme()` now passes the documented constant defaults
(14/6/2/16) to `theme_metric()`, so a partial theme resets to the default
instead of keeping the previous theme's value.

**File:** `wbuild_widgets/WListView.widget` (`list_apply_theme`)

## Theme data drift: partial themes and dead keys

- `themes.hdf`: `test` omits `switch_off/on` and `switch_knob_off/on`;
  `default`/`highcontrast` omit `bordercolor`.
- `cookbook/themes/cookbook.hdf` lacks all style-guide roles (`button_*`,
  `input_*`, `list_*`, `font_ui`, metrics), so widgets under the cookbook themes
  fall back to resource defaults / sticky values.
- Defined but never consumed in `themes.hdf`: `group_gap`, `button_border`,
  `font_tiny`, `font_normal`, `font_large`, `font_heading1`, `font_heading2`,
  `list_row_height`, `list_header_height`.

**Fix:** added the missing `switch_*` roles to `test`, `bordercolor` to
`default`/`highcontrast`, and the full style-guide role/metric set to both
cookbook themes. The remaining unused keys (`group_gap`, `button_border`,
`font_tiny/normal/large/heading*`, `list_row_height`, `list_header_height`) are
reserved roles (most are documented in the style guide) and are not yet consumed
by widget code; wiring them (especially the list row/header heights) is a
follow-up that changes layout baselines.

**Files:** `themes.hdf`, `cookbook/themes/cookbook.hdf`

## Theme registry: observers registered before `theme_init()` were dropped

**Symptom:** a widget created before `theme_init()` (or when `XCCCreate()`
failed) never received theme updates; `theme_register()` returned early on
`!display_ready`.

**Fix:** `theme_register()`/`theme_unregister()` now always maintain the
observer list; only `theme_apply()` and `get_color()` require a ready display.
Widgets registered before `theme_init()` are applied on the next
`theme_select()`/`theme_apply()`.

**File:** `utils/theme.c`

## Theme registry: `theme_apply()` skipped observers when callbacks mutated the list

**Symptom:** an apply callback that registered or unregistered a widget shifted
the live MLS indices, so later widgets could be skipped; a callback that called
`theme_select()` recursed into `theme_apply()`.

**Fix:** `theme_apply()` snapshots the observer list before dispatch and ignores
re-entrant calls (`applying` guard).

**File:** `utils/theme.c`

## Theme registry: calling `theme_init()` twice leaked the old colour context

**Symptom:** a second `theme_init()` overwrote `xcc` without freeing the first
`XCC`.

**Fix:** `theme_init()` frees an existing `xcc` before creating the new one.

**File:** `utils/theme.c`

## t1_menu: left demo list and right trace list frames were not aligned

**Symptom:** in the launcher the outer frames of the left demo list and the
right trace-level list did not end on the same line; the right frame sat a few
pixels lower.

**Cause:** the trace list was nested in a `Gridbox` (`trace_box`) together with
a separate `Trace: 0` label. The nested box added its own border/margins and
the trace list started below the label, so the two list frames could not align.
The label was redundant: the list header already says `Trace level` and the
selected row is highlighted.

**Fix:** put `trace_lv` directly into the main grid at the same `gridy` as
`lv` (both `weightx 1`, `weighty 1`, `fill 3`) and remove the separate
`trace_lbl` label and its `gui.set()` update.

**File:** `tools/t1_menu.lua`

## t1_menu / demo_listview: selecting a row did not update the description

**Symptom:** clicking or keyboard-selecting a row in the left `WListView`
moved the highlight, but the description below (and the `demo_listview` status
label) stayed on the first row.

**Cause:** LUI callbacks written as `LUA(on_select)` are invoked through the
`LUA_callback` C bridge, which passes only the callback's *client data*
(`on_select` / optional argument), not the widget callback's `call_data`.
`WListView.notify` supplies the selected row as `call_data`, so
`on_select(i)` received `nil`; `select_demo(nil)` returned without changing
the description. The initial row worked because `select_demo(0)` was called
explicitly.

**Fix:** read the current selection from the widget instead of relying on the
callback argument:

```lua
function on_select(_)
    select_demo(gui.get('lv', 'selected'))
end
```

The same fix was applied to `on_activate`/`on_trace` in `t1_menu.lua` and to
`on_select`/`on_activate` in `demos/demo_listview.lua`.
`tests/test_t1_menu.sh` now selects row 4 (`theme`) via the
`T1_MENU_LATE_SELECT` hook and asserts that the description changed.

**Files:** `tools/t1_menu.lua`, `demos/demo_listview.lua`,
`tests/test_t1_menu.sh`

## WspinBox: numeric value was not displayed (widget showed its name)

**Symptom:** the form/contacts demos showed `agespin` / `ph_area` instead of
the numeric value in the spinbox row; the demo looked like it was missing the
age spin widget.

**Cause:** `WspinBox` inherits `Wlabel` but never copied its `value` resource
into the label. `Wlabel.initialize` therefore fell back to the widget name,
and `spin_up()`/`spin_down()` only called `redraw_label()` on that stale text.

**Fix:** `sync_label()` formats `value` into `XtNlabel` and is called from
`initialize()`, `spin_up()` and `spin_down()`. The form demo now composes
visible `-`/`+` `Wbutton`s that call the spin actions via `xtaction()` and
resets both `value` and `label` on Cancel. `tests/test_wspinbox.c` checks the
initial label and the label after up/down.

**Files:** `wbuild_widgets/WspinBox.widget`, `demos/demo_form.lua`,
`tests/test_wspinbox.c`

## WtabWidget: unselected tab titles were invisible

**Symptom:** only the selected tab title was visible; the other tabs appeared
empty. The tab strip was painted with `gridbox_border`, while unselected text
used `fg_norm` — in most themes those are the same dark colour.

**Fix:** add a `tabBg` resource resolved from `list_header_bg` (fallback
`gridbox_bg`); resolve `tabFg` from `list_header_fg` (fallback `fg_norm`) and
`tabFgSel` from `accent` (fallback `fg_sel`). The strip now follows the
style-guide header roles.

**File:** `wbuild_widgets/WtabWidget.widget`

## WtabWidget/Wviewport: tab pages did not fill the content area

**Symptom:** a tab page with a small natural width (e.g. the cookbook recipe
list) was rendered as a narrow island on the left; the list's flex column
collapsed and headers/columns were clipped.

**Fix:** `WtabWidget.relayout()` sizes the current page to the viewport using
`XtQueryGeometry` (preferred height, at least the viewport height) and the clip
width (minus the vertical bar). `Wviewport.content_pref()` honours the page's
stretched core size. Pages now fill the tab area and only scroll when their
preferred height is larger.

**Files:** `wbuild_widgets/WtabWidget.widget`, `wbuild_widgets/Wviewport.widget`

## Wviewport: scrollbars used the unthemed VSlider default colour

**Symptom:** scrollbars inside tab pages were red (the X default foreground)
instead of the theme's scrollbar colour.

**Fix:** `Wviewport` registers with the theme and applies `scrollbar_thumb`
(fallback `list_header_bg`) to the VSlider/HSlider foreground and
`gridbox_bg`/`bg_norm` to their background and the clip. The existing
VSlider GC rebuild makes live theme changes work.

**File:** `wbuild_widgets/Wviewport.widget`

## KaroEd: explicit `:height` was ignored; editor used the page background

**Symptom:** a `KaroEd` with `:height 80` collapsed to a thin strip in the
cookbook editor, and its text area blended into the page surface.

**Fix:** `initialize()` only derives width/height from the grid dimensions when
they are 0, so an explicit `:height`/`:width` is respected. A `colors_changed`
override resolves `input_bg`/`input_fg` for the editor content (falling back to
the Wheel colours) and reallocates the Xft colours.

**File:** `wbuild_widgets/KaroEd.widget`

## Cookbook: Xaw Toggle and legacy Wcombo replaced; style-guide redesign

The cookbook now uses a `WtabWidget` for its four screens, `WListView` for the
recipe/ingredient/shopping lists, `WToggleSwitch` for dietary flags, a flat
`Wbutton` category cycler instead of the legacy `Wcombo`, and top action bars
(Cancel/Save, Back/Edit) with a primary accent action. The layout follows
`style-guide.md` (header/toolbar, tabbed content, status bar, themed list and
scrollbar roles).

**Files:** `cookbook/app/main.lua`, `cookbook/app/screens/*.lua`,
`cookbook/app/ui/components.lua`, `cookbook/README.md`

## run_widget.sh: normal Quit exited 1 under LeakSanitizer

**Symptom:** the new single-widget probe harness rendered correctly, but
pressing Quit (or using `--autoclose`) exited with status 1 and printed a
LeakSanitizer report.

**Cause:** the debug `commander_runner` is ASan-instrumented. Process-global
fontconfig, Xt and task-manager allocations are reported as leaks on normal
shutdown, independently of the probe widget.

**Fix:** `run_widget.sh` exports `ASAN_OPTIONS=detect_leaks=0` by default,
matching the cookbook smoke test. Set `RUN_WIDGET_LEAKS=1` to keep leak
detection enabled when investigating a widget-specific leak.

**File:** `run_widget.sh`

## Wviewport: dragging the scrollbar thumb stopped before the bottom

**Symptom:** in the cookbook Detail tab (and any `Wviewport` with a tall
content), dragging the vertical scrollbar thumb down did not reach the bottom.
Mouse-wheel scrolling inside the viewport did reach the end and updated the
thumb.

**Cause:** `relayout()` synced the slider position with the total content
extent:

```c
pos = scroll_y * 1000000 / ch;   /* ch = content height */
```

but `vscroll_cb()` inverted it with the scrollable extent:

```c
scroll_y = pos * maxy / 1000000; /* maxy = ch - clip_h */
```

At a full drag (`pos = RES - frac`) this produced `maxy^2 / ch`, so the thumb
stopped early. This is the same mapping bug previously fixed in `WListView`.

**Fix:** invert with the same total extent used by `relayout()`:

```c
scroll_y = pos * ch / 1000000;   /* hscroll: scroll_x = pos * cw / 1000000 */
```

`relayout()` still clamps the result to `maxy`/`maxx`.

Regression coverage in `tests/test_wviewport.c` calls the vertical slider's
`sl_motion` action at the bottom and asserts `scroll_y` reaches `maxy`.

**Files:** `wbuild_widgets/Wviewport.widget`, `tests/test_wviewport.c`

## wbuild: `WMenu` / `Wmenu` generated header guard collision

**Symptom:** adding a new `WMenu` widget next to the existing legacy `Wmenu`
made `register_wb.c` fail with `wMenuWidgetClass undeclared`, even though the
generated `WMenu.h` declared it.

**Cause:** wbuild derives the include guard from the uppercased class name.
`WMenu` and `Wmenu` both produced `XTCW_WMENU_H`, so whichever header was
included first suppressed the other.

**Fix:** name the new popup-menu container `WPopupMenu` (no case-only
collision) and expose it through the `menu-list` / `popup-menu` LUI aliases.
`WMenuBar` remains as the menu-bar container.

**Files:** `wbuild_widgets/WPopupMenu.widget`, `wbuild_widgets/WMenuBar.widget`,
`lui/registry.lua`

## WmenuButton: menu labels used the default GC font

**Symptom:** the cookbook File/View/Help labels were much smaller than the
rest of the UI, because `WmenuButton` drew the text with `XDrawString` and the
X11 default GC font. Theme changes also had no effect on the label.

**Cause:** `WmenuButton` is a `Composite`, not a `Wheel`/`Wlabel` subclass, so
it never resolved `font_ui` or allocated an Xft font/colour.

**Fix:** `WmenuButton` now opens an Xft font from `theme_font_spec()` and
allocates its colour from `button_fg` (fallback `fg_norm`). The font/colour are
rebuilt by the existing theme callback, and `draw_icon()` uses
`XftDrawStringUtf8` with a centred baseline. `XDrawString` remains as a
fallback when no theme colour is available.

No new theme entries are required: menu bars use `list_header_bg`/`bg_hi`,
popup menus use `list_bg`, and triggers use `bg_norm`/`bg_hi` plus
`button_fg`/`fg_norm`. Dedicated `menubar_*`/`menu_*` roles can be added later
if independent menu styling is needed.

**Files:** `wbuild_widgets/WmenuButton.widget`, `tests/test_wmenubar.c`

## cookbook smoke test left commander_runner running

**Symptom:** after `cookbook/scripts/smoke-test.sh` finished, a
`commander_runner` process and its window could remain open.

**Cause:** the cleanup trap sent `SIGKILL` to the `timeout` wrapper. The
wrapper never got a chance to forward a signal to its child, so
`commander_runner` was reparented and kept running.

**Fix:** launch the smoke-test app under `setsid` and terminate the whole
process group with `SIGTERM`, wait briefly, then `SIGKILL` if needed. This
also covers the optional `setarch` wrapper.

**File:** `cookbook/scripts/smoke-test.sh`

## Wlabel subclasses: overriding `set_values` can double-free the label

**Symptom:** `WComboBox` initially crashed with an ASan double-free in
`Wlabel.set_values()` when `items`/`selected` were changed from Lua.

**Cause:** `Wlabel.set_values()` frees `$old$label_mem` and replaces
`$label_mem`. A subclass that calls `update_label()` (which itself calls
`XtVaSetValues(label)`) and then calls the inherited `set_values` with the
original `old` state can free the same label storage twice. wbuild's
`#set_values(...)` helper also emits an undeclared `new` parameter instead of
`request`, so it is not usable here.

**Fix:** `WComboBox` does not override `set_values`. It exposes explicit
`refresh` and `select_index` actions; `update_label()` changes the label via
`XtVaSetValues`, and `Wlabel.set_values` handles the copy exactly once.
`items`/`selected` remain data resources; Lua can call `refresh` after changing
them programmatically.

**Files:** `wbuild_widgets/WComboBox.widget`

## WComboBox popup overlapped its trigger

**Symptom:** the combo popup opened at the combo's parent origin and covered the
trigger instead of appearing below it.

**Cause:** `XtTranslateCoords()` returned the parent/container origin for the
`Wlabel`-based `WComboBox` (not the widget's actual root position), so the popup
shell received the wrong `x/y`.

**Fix:** use `XTranslateCoordinates()` from `XtWindow(self)` to the root window
with `y = height + 2`, then position the override shell. Also added a full-app
InputOnly click catcher so clicking outside closes the popup, and the catcher
is freed on close/destroy.

**Files:** `wbuild_widgets/WComboBox.widget`, `tests/test_wcombobox.c`

## WStatusBar looked clipped / too tight

**Symptom:** the cookbook status line looked cramped and not fully visible; the
32px bar was exactly the label height plus padding, and its background blended
into the window.

**Fix:** `WStatusBar` now derives its automatic height from the message
label's preferred font height: `label_height + 2*barPadding + 4`, with a 32px
minimum. The cookbook leaves `barHeight` unset and uses `barPadding 6`, so the
bar grows with the active theme font instead of clipping descenders. The bar
prefers `status_bg`/`list_header_bg`/`bg_hi` for a visible surface and draws a
top separator line. The cookbook's extra `UTF-8` label was removed because a
`Wlabel` child paints its own background box inside the bar.

**Files:** `wbuild_widgets/WStatusBar.widget`, `cookbook/app/main.lua`

## Cookbook status bar clipped when launched via `t1.sh`

**Symptom:** `./t1.sh cookbook` showed only half of the status bar, while
`cookbook/scripts/run.sh` looked fine.

**Cause:** the cookbook window was 1000x760. On a 1024x768 screen with window
manager decorations, the bottom of the window (the status bar) is clipped.
`t1.sh` also omitted `-theme default`, so the cookbook theme file relied on the
host fallback warning path.

**Fix:** reduce the cookbook window to 1000x680 and pass `-theme default` from
`t1.sh` like `cookbook/scripts/run.sh` does. The status bar itself remains
40px high with 6px padding.

**Files:** `cookbook/app/main.lua`, `t1.sh`

## WComboBox popup item callback crashed

**Symptom:** selecting an item from a `WComboBox` popup aborted with an ASan
SEGV in `item_selected_cb` while reading `$items`/`m_len()`.

**Cause:** the Wbutton callback was registered with the item index as
`client_data`, but the callback signature treats `client_data` as the
`WComboBox` widget. `self` therefore became `0`, `1`, `2`, ... and the first
field access dereferenced an invalid pointer.

**Fix:** register the callback with the combo as `client_data` and store the
item index in the Wbutton's `user_data` resource. `item_selected_cb()` now
reads the index from the button (`w`) and uses the combo (`self`) for state,
label updates and the user callback.

Regression coverage in `tests/test_wcombobox.c` invokes the third popup item
and asserts the selected value/label.

**File:** `wbuild_widgets/WComboBox.widget`

## WTreeView clipped after expanding

**Symptom:** expanding a tree node showed only the first rows; newly visible
children had no space and the tree looked truncated.

**Cause:** `WTreeView.rebuild()` changed `visibleCount` but did not request a
new widget height. The surrounding `WFormLayout` kept the old row height, so
additional rows were drawn outside the visible area.

**Fix:** `rebuild()` now calls `XtMakeGeometryRequest(CWHeight)` when
`visibleCount` changes, and the tree rows use indented markers/text. The
cookbook also has a dedicated full-height **Tree** tab instead of squeezing the
tree into the Gallery form.

**Files:** `wbuild_widgets/WTreeView.widget`, `cookbook/app/screens/tree_view.lua`

## GitNexus runner fails under the default Node 14

**Symptom:** `node .gitnexus/run.cjs impact ...` aborted with a `node-gyp`
build failure in `tree-sitter-javascript` (`ValueError: invalid mode: 'rU'`)
and `npx gitnexus@latest` exit code 1.

**Cause:** the shell's default nvm Node is v14.5.0, so `run.cjs` selected the
`npx` rung and tried to install GitNexus with an interpreter the package does
not support (`engines: node ^22.18.0 || >=24.11.0`).

**Fix:** added `~/.local/bin/gitnexus`, a wrapper that pins the CLI to Node
22.19.0 and the existing npx-cached GitNexus 1.6.12 install. `run.cjs` now
finds `gitnexus` on `PATH` first and never falls through to `npx`. A global
`npm i -g gitnexus` also works but duplicates ~1.9G on this full disk.

**Files:** `~/.local/bin/gitnexus` (machine-local), `.gitnexus/run.cjs`
(generated).

## GitNexus refuses a copied index as "foreign"

**Symptom:** after fixing the Node runner, `gitnexus analyze --index-only`
failed with `Storage path is in state "foreign" but requires one of: missing,
empty, owned`.

**Cause:** `.gitnexus/` was copied from `/home/jens/git/xtcw2` into this
worktree/copy, so `meta.json` still recorded the old `repoPath` and
`storagePath`. GitNexus 1.6.12 validates that metadata and rejects the index.

**Fix:** removed the copied `.gitnexus/` directory (regenerable, gitignored)
and re-ran `gitnexus analyze --index-only` from
`/home/jens/git/xtcw2-x1`, which rebuilt and registered the index in place.

**Files:** `.gitnexus/` (generated).

## WTreeView expander brackets overlap between rows

**Symptom:** the `[+]`/`[-]` expander glyphs looked taller than the row pitch
and visually touched/overlapped the row above or below, even with
`:itemHeight 26`.

**Cause:** bracket glyphs in the theme font extend well above `ascent` and
below `descent`. Centering normal text in `itemHeight` therefore did not
reserve enough vertical space for the bracket glyphs.

**Fix:** `draw_tree()` no longer renders the marker as text. It draws a
vector box sized from `itemHeight` (`7..13` px), vertically centered in the
row, with a horizontal minus line and an optional vertical plus line for
collapsed nodes. The box can never exceed the row, independent of the theme
font. Text x-position follows the box width.

**Files:** `wbuild_widgets/WTreeView.widget`

## WListView sorting broke selection → model mapping

**Symptom:** after `table-view.sort`, opening the selected row showed the wrong
recipe. Sorting by name left row 0 highlighted but `selected` was still a
display index, while the cookbook used it as a model index.

**Cause:** `WListView.sort()` sorted the `$rows` array in place and had no
parallel identity column, so the row index no longer mapped to the original
data row.

**Fix:** added `rowIds` (an MLS list parallel to `rows`) and a public
`selectedId` resource. Sorting now permutes `rows` and `rowIds` together via a
`row_sort_pair` array, and `update_selected_id()` keeps `selectedId` in sync
on selection, row changes and sorting. The cookbook sets `rowIds` from
`model.list_ids()` and opens/deletes by `selectedId`.

**Files:** `wbuild_widgets/WListView.widget`, `cookbook/app/main.lua`

## WDialog result was invisible to Lua callbacks

**Symptom:** the cookbook confirm dialog reported "Delete cancelled" even after
clicking OK.

**Cause:** `LUA(...)` callbacks are invoked as `funcname()`; the host does not
forward `XtCallCallbackList`'s `call_data`. `WDialog` kept its result in a
private field, so Lua could not distinguish OK from Cancel.

**Fix:** exposed `result` as a public `WDialog` resource (1 = OK, 0 = Cancel).
Cookbook callbacks use the callback argument when present and fall back to
`gui.get(id, "result")`. `cb_dialog_result` was updated the same way.

**Files:** `wbuild_widgets/WDialog.widget`, `cookbook/app/main.lua`

## Wretex autoHeight endless loop when replacing text

**Symptom:** updating a `Wretex` text a second time with `autoHeight TRUE`
inside a weighted `Gridbox` cell caused an endless geometry loop; the grid grew
to tens of thousands of pixels and the app hung.

**Cause:** `Wretex.set_values_almost()` copied `reply->width/height` but then
forced `request->request_mode = CWWidth | CWHeight`. Xt's `XtSetValues()` loops
while `request_mode != 0`; when the parent denied the request it set
`reply->request_mode = 0`, but the forced mask kept the loop alive forever.
The width/height copied from that non-compromise reply were uninitialised
garbage, which is why the grid kept growing.

**Fix:** `set_values_almost()` now follows the Xt contract and does
`*request = *reply;`, so `request_mode` becomes 0 when the parent has no
compromise and the loop terminates. `set_values()` also computes the auto
height from the widget's current width instead of a hardcoded 400pt width. The
cookbook no longer needs the `:autoHeight FALSE` workaround.

**Files:** `wbuild_widgets/Wretex.widget`, `cookbook/app/screens/recipe_detail.lua`

## DrawCanvas `accept_focus` must live in `@METHODS`

**Symptom:** a `DrawCanvas` with `:takeFocus TRUE` never received keyboard
events; the generated `accept_focus` was a static `void` helper and the class
record still used `XtInheritAcceptFocus`.

**Cause:** `@proc accept_focus { return True; }` was placed in the widget's
`@UTILITIES` section. wbuild emits utility procs as plain `void` functions and
does not install them in the Core class record. Core methods such as
`accept_focus`, `resize`, `expose` and `set_values` must be in `@METHODS`.

**Fix:** moved `accept_focus` into `@METHODS`. The widget now registers a real
`Boolean accept_focus(Widget, Time*)`, and `takeFocus` schedules
`XtSetKeyboardFocus` after realize (a zero-timeout avoids setting focus before
the shell is mapped).

**Files:** `wbuild_widgets/DrawCanvas.widget`

## MLS debug post-mortem false positives after widget teardown

**Symptom:** destroying a realized `Wlabel` (or any widget whose `destroy`
calls `theme_unregister()`) prints a post-mortem block at exit:

```
ERROR in function: '_mls'. Handle: 6. Called by 'theme_unregister()' ...
Array Index out of bounds i=0 should be lower than m_len(6)=0.
```

A related false positive appears after a realized `WToolTip` presenter is used
(`_m_next`, `renderer_render_text()`), even when the test does not destroy the
shell. Exit status stays 0, so tests pass; the noise is misleading.

**Cause:** `_mlsdb_caller()` records the arguments of the *last* debug-wrapped
MLS call in the global `debi`. `exit_error()` is registered with `atexit()`
and has no "an error actually happened" flag: it always re-validates that
recorded call when the process exits. By then the widget/list teardown has
emptied or freed the relevant array, so a call that was valid when it ran is
reported as out-of-bounds/UAF. `_m_destruct()` cleared `debi.me`, which is why
normal `commander_runner` runs were quiet while tests that return without
`m_destruct()` printed the block.

**Fix:** every debug wrapper (`_m_create`, `_m_alloc`, `_m_free`, `_m_buf`,
`_mls`, `_m_next`, `_m_put`, `_m_clear`) now calls `_mlsdb_done()` after a
successful operation, clearing `debi.me`. The record survives only while a
call is in flight, so a real `ERR`/abort inside MLS still produces the
post-mortem (verified with an out-of-bounds `mls()` call), but stale calls at
teardown no longer do.

**Files:** `utils/mls.c` (`_mlsdb_done`, the `_m_*` wrappers),
`tests/test_wtooltip.c`, `tests/test_wdialog.c`

## Wedit destroy crashed when the draw context was never created

**Symptom:** destroying a dialog that contains a `WLineEdit`/`Wedit` field
(e.g. the new `gui.prompt` helper) aborted with a SIGSEGV inside
`XftDrawDestroy`.

**Cause:** `Wedit.destroy` called `XftDrawDestroy($draw)` unconditionally.
`$draw` is only created on the first expose; a dialog destroyed before the
field was exposed (or after the popup was popped down) passed NULL/garbage.

**Fix:** guard the call with `if ($draw) { XftDrawDestroy($draw); $draw = NULL; }`.

**Files:** `wbuild_widgets/Wedit.widget`

## WComboBox choice teardown trips the MLS post-mortem check

**Symptom:** after a `WDialog` with `inputKind 4` (choice) is destroyed, the
debug post-mortem reports `_mls` in `WComboBox.update_label()` on the dialog's
`inputItems` handle. Exit status stays 0.

**Cause:** same post-mortem behaviour as the entry above: `update_label()` was
the last recorded `_mls` call, and the dialog freed `inputItems` afterwards.
The functional path was already correct (the dialog destroys the field before
freeing the list); only the exit-time re-validation was wrong.

**Status:** resolved by the `_mlsdb_done()` fix above. Separately, WComboBox
still borrows the caller's `items` MLS; giving it ownership would be a good
hardening step but is not required for correctness today.

**Files:** `wbuild_widgets/WComboBox.widget`, `wbuild_widgets/WDialog.widget`,
`tests/test_wdialog.c`

## Direct Xt tests must register the ArrayInt converter

**Symptom:** `test_wlistmulti` intermittently aborted during realize with
`ERROR: mls.c ... lst Index(1432522877) out of Bounds`, called from
`render_retex_row()` → `m_len($columnWidths)`.

**Cause:** the test passed `"columnWidths", "100,100,100"` to
`XtVaCreateManagedWidget()` without calling `converters_init()`. The
`ArrayInt` converter is registered by `WcAddConverters()`, which the LUI/xtcw
creation path (`WcWidgetCreation`) calls — but direct Xt tests bypass it. Xt
then left the int resource holding the `String` pointer instead of an MLS
handle, and `m_len()` received a pointer as the handle. Whether the field was
zero or garbage depended on allocation history, hence the flakiness.

**Mitigation:** call `converters_init()` after `XtcwRegister()` in the test.
That is required for direct Xt tests, but it did not fully eliminate the
flakiness: with `-xrm` font settings the widget still sometimes renders with a
non-handle in `columnWidths` before `render_retex_row()` runs. The remaining
issue is in the legacy `WlistMulti` resource/realize path, not in MLS; the
`lst` abort is the symptom.

**Status:** pre-existing flake, independent of the MLS post-mortem fix.
Standalone runs pass 5/5 without the `-xrm` font pins; the layout suite can
still fail. A proper fix should validate `columnWidths` before `m_len()` (or
initialize it defensively in `WlistMulti.initialize`).

**Files:** `tests/test_wlistmulti.c`, `wbuild_widgets/WlistMulti.widget`

## WTreeView: rows below the viewport and keyboard actions were unreachable

**Symptom:** a `WTreeView` taller than its allocated space silently clipped the
remaining rows; the mouse wheel, `Home`/`End`, `PageUp`/`PageDown` and the
existing arrow translations had no effect because the widget never acquired
keyboard focus.

**Cause:** `draw_tree()` always started at row 0 and `rebuild()` only grew the
widget; there was no scroll offset. The class also inherited `Core` and had no
`focus_self()` call on click, so Xt never delivered key events to it.

**Fix:** added `firstVisible`/`visibleRows` resources and an `autoHeight`
switch, draw only the rows that fit (`autoHeight FALSE` keeps the allocated
height), added wheel/page/home/end actions plus `ensure_visible()`, and set
keyboard focus on click via `XtSetKeyboardFocus()` with `FocusIn`/`FocusOut`
actions drawing an accent focus ring.

**Files:** `wbuild_widgets/WTreeView.widget`, `tests/test_wtreeview.c`,
`tests/widgets/wtreeview.lui`

## WTreeView test: a Core child of a shell is stretched to the shell size

**Symptom:** a `WTreeView` created with `XtNheight 120` directly under the
session shell reported height 300 (the shell height) after realize, so the
scroll-window assertions saw the wrong `visibleRows`.

**Cause:** the shell lays out its managed child to fill the shell; the tree's
requested height is not preserved for a direct shell child.

**Fix:** put the tree inside a `Gridbox` with `weightx/weighty 0` and
`fill "none"`, then the requested height survives and the scroll window is
deterministic. This is a test-harness gotcha, not a widget defect.

**Files:** `tests/test_wtreeview.c`

## WDialog: the `open` resource never actually opened the popup

**Symptom:** `XtVaSetValues(dlg, "open", True)` made `wdialog_is_open()` return
true, but the dialog shell was never realized or popped up; tests that only
read the `open` resource passed while no window appeared.

**Cause:** `dialog_open()` began with `if ($open) return;`. When the resource
change was handled by `set_values()`, the new `$open` value was already true,
so the function returned before realizing the shell.

**Fix:** treat the dialog as already open only when the shell is realized
(`if ($open && XtIsRealized($shell)) return;`). Also guard the centering
translation with `XtWindow(top) != None`, because `gui.alert` can run before
the application shell is realized.

**Files:** `wbuild_widgets/WDialog.widget`, `tests/test_wdialog.c`,
`tests/test_dialogs.sh`

## Direct Xt tests: `StringMArray` resources need an MLS handle

**Symptom:** passing `"items", "Small,Medium,Large"` to a widget with a
`StringMArray` resource in a direct Xt test leaves the string pointer in the
int resource; `m_len()` then reports an out-of-bounds MLS index.

**Cause:** the `XtRString -> StringMArray` converter is installed by
`converters_init()`, which the LUI creation path calls via `WcAddConverters`.
In a direct `XtVaCreateManagedWidget()` call the conversion can run before the
widget class/converter setup, and Xt stores the unconverted value.

**Fix:** build an MLS string list with `m_create()`/`m_put()` and pass that
handle, as `tests/test_wcombobox.c` and `tests/test_wradiogroup.c` do. This is
a test-harness gotcha; LUI applications are unaffected.

**Files:** `tests/test_wradiogroup.c`, `tests/test_wcombobox.c`

## WListView: a fixed row-height metric ignored the font size

**Symptom:** `WListView` rows used the theme's `table_row_height` (or
`list_row_height`) as the final height. Increasing `fontSize`, changing the
UI font, or running at a larger DPI could clip the text or leave no visible
gap, because the row height stayed fixed.

**Cause:** `open_font()` preferred the theme row-height metric over the
font-derived line box (`font->height + 2 * cellPaddingY`).

**Fix:** derive the row and header height from the font line box plus
`list_cell_pad_y`; treat `table_row_height`/`list_row_height` and the header
metrics as minimums only. An explicit `rowHeight`/`headerHeight` resource still
wins. The default `list_cell_pad_y` is now 4, giving a visible gap around a
13px UI font.

**Files:** `wbuild_widgets/WListView.widget`, `tests/test_wlistview.c`,
`themes.hdf`, `cookbook/themes/cookbook.hdf`, `style-guide.md`

## DrawCanvas ignored requested sizes below 200x200

**Symptom:** a small `DrawCanvas` used as a colour swatch (requested 60x28)
was laid out at 200x200 or larger, pushing the surrounding toolbox out of the
window.

**Cause:** `query_geometry()` always started with a 200x200 preferred size and
only increased it when the request was larger, so a smaller explicit
`:width`/`:height` was discarded.

**Fix:** honour the requested width/height when present, otherwise fall back to
the widget's current size, and only use 200x200 as a last resort.

**Files:** `wbuild_widgets/DrawCanvas.widget`, `demos/demo_iconeditor.lua`

## IconEditor treated opaque black as transparent

**Symptom:** drawing with the black palette colour produced no visible pixels
and the XPM export contained only `None`.

**Cause:** the pixel buffer used `0` as the transparent sentinel; opaque black
(`0x000000`) was indistinguishable from transparent.

**Fix:** store opaque pixels as `0xFFRRGGBB` and keep `0` for transparency.
XPM save/load mask to 24 bits and preserve `None` separately from `#000000`.

**Files:** `wbuild_widgets/iconeditor-draw.c`

## Wcl late-binder crash when a plug-in invokes a Lua callback list

**Symptom:** an IconEditor `changedCallback` set to `LUA(on_canvas_changed)`
and invoked repeatedly from the plug-in crashed in `XtCallCallbackList` with
the program counter equal to the late-binder closure address.

**Cause:** the Wcl late binder for the new callback list was not stable across
repeated invocations from the C plug-in in this setup.

**Fix:** do not call Lua from the plug-in. DrawCanvas exposes a monotonically
increasing `changeCount`; the Lua UI polls it with `xtapptimeout` and updates
its dirty/status state. Mouse edits stay entirely in C.

**Files:** `wbuild_widgets/DrawCanvas.widget`, `wbuild_widgets/iconeditor-draw.c`,
`demos/demo_iconeditor.lua`

## Wbutton labels were clipped at the bottom

**Symptom:** every button label (toolbar, toolbox, menu items) looked cut off
along the bottom; the glyph bottoms sat on the line-box edge, and fixed-height
buttons left no vertical breathing room.

**Cause:** `libXt` calls the inherited `Wlabel` initializer before the
`Wbutton` initializer, so the label was set up.  The clipping came from the
natural height being exactly the re-tex line box (no vertical padding), from
subtracting `topGap`/`bottomGap` during centering, and from fixed 28px button
rows in the demo.  Setting the gaps in `Wbutton.initialize` was too late,
because the inherited initializer had already measured the natural height; an
explicit `#initialize` call fixed the height but ran the inherited initializer
twice.

**Fix:** make `topGap`/`bottomGap = 3` the `Wlabel` resource defaults, so every
label-derived widget (including `Wbutton`) gets the padding before the
inherited initializer measures the natural height.  Centre the paragraph in
the full widget height and keep the pixmap cache in sync with the widget
size.  `Wbutton` no longer needs a `class_initialize` resource override; the
explicit `#initialize` call stays removed.  The IconEditor demo uses
natural-height buttons and a slightly taller window instead of fixed 28px
rows.

**Files:** `wbuild_widgets/Wlabel.widget`, `wbuild_widgets/Wbutton.widget`,
`demos/demo_iconeditor.lua`, `tests/baselines/test_wlabel.layout`,
`tests/baselines/test_wlabel_selection.layout`

## IconEditor repainted the whole canvas for every pixel

**Symptom:** setting a pixel in the IconEditor (pencil drag, fill, undo)
redrew the complete grid and preview.  On non-double-buffered or remote X
connections the full-canvas repaint was visible as flicker, and every edit
caused two paints: an Xt expose from the `changeCount` update plus the timer
repaint.

**Cause:** `DrawCanvas.set_values()` returned `True` for *every* resource
change, so bumping the non-visual `changeCount` scheduled a full expose.
The IconEditor plug-in also had no way to describe a partial update and
always called `mark_dirty`, which repaints the whole widget.

**Fix:** DrawCanvas now tracks a damage rectangle.  `mark_dirty_area(x,y,w,h)`
accumulates a partial region and `paint1()` exposes it to the plug-in through
`draw_canvas_t.damage_x/y/w/h` plus `damage_full` (set by expose, resize,
`mark_dirty` and `redraw`).  `set_values()` only repaints for visual
resources and returns `False` otherwise.  The IconEditor accumulates the
touched icon cells in `set_pixel()`/`flood_fill()`, converts that box to
widget coordinates for the grid and preview, and redraws only those cells
(clipped, including grid lines and the outline).  Shape tools still request a
full repaint while their rubber band is visible.

**Files:** `wbuild_widgets/DrawCanvas.widget`, `wbuild_widgets/drawcanvas-draw.h`,
`wbuild_widgets/iconeditor-draw.c`

## File chooser "Hidden" label overlapped its check box

**Symptom:** in the WfileSelector/WfileChooser dialog the check box was drawn
inside the "H" of the "Hidden" label.

**Cause:** `Wcheckbox.initialize` reserves space for the 14px box by setting
`leftGap = 22`.  The chooser's uniform control-padding loop then overwrote
`leftGap` with the button padding (6), so the label started inside the box.

**Fix:** the padding loop still applies `rightGap`/`topGap`/`bottomGap` to the
checkbox but leaves its `leftGap` alone.

**Files:** `wbuild_widgets/WfileChooser.widget`

## WToolBar squeezed buttons and removed their bottom label gap

**Symptom:** toolbar buttons (New/Open/Save/Undo, Up/Home/…) had space above
the label but almost none below it.

**Cause:** `toolbar_layout()` forced every child to
`toolbarHeight - 2 * toolbarPadding`, regardless of the child's preferred
height.  With the 3px label padding the natural button height is ~37px, so a
42px toolbar gave the buttons only 34px and the label lost its bottom gap.
`query_geometry()` likewise reported the configured height, so the parent
could not grow the toolbar.

**Fix:** treat `toolbarHeight` as a minimum.  Both `query_geometry()` and
`toolbar_layout()` now use `max(toolbarHeight, tallest_child_pref + 2 *
toolbarPadding)`.  `tests/test_wtoolbar.c` checks the advertised height and
that children keep their preferred height.

**Files:** `wbuild_widgets/WToolBar.widget`, `tests/test_wtoolbar.c`

## Window-manager close killed the client without cleanup

**Symptom:** closing a `t1.sh` demo with the window-manager close button
terminated the process without the normal shutdown path.  `m_destruct()` was
never called and `WM_PROTOCOLS` was not set on the application shell, so a
window manager fell back to killing the client.

**Cause:** `commander_runner` never installed `WM_DELETE_WINDOW` or handled
the corresponding `ClientMessage`.  Only the file-selector popup did.

**Fix:** after realizing the top-level shell, `commander_runner` now sets
`WM_DELETE_WINDOW` and installs a `ClientMessage` handler.  The handler calls
an optional global Lua `on_close()`; returning `false` vetoes the close
(e.g. to show an "unsaved changes" dialog), while a missing callback, `nil`
or `true` allows it and sets the Xt exit flag so the normal teardown (widget
destroy, `lua_close`, `theme_destroy`, `m_destruct`) runs.  The IconEditor
demo implements `on_close()` by reusing its quit flow, and the tutorial
documents the hook.

**Files:** `commander/commander_runner.c`, `demos/demo_iconeditor.lua`,
`commander/COMMANDER_RUNNER_TUTORIAL.md`

## Shutdown leaks in the Lua/XT bridge

**Symptom:** closing the IconEditor (or any LUI app) produced LeakSanitizer
reports from `xtgetvalue_lua`, `register_widget`/`xtcreate_lua`,
`xtsetvalue_lua`, `add_fontinfo` and `task_register_func`.

**Cause:** several bridge paths allocated without a matching free:
- `WcGetResourceType()` returns an `XtNewString` copy that
  `xtgetvalue_lua()` never freed (one per `xtgetvalue`).
- `register_widget()` `strdup()`s every Lua-created widget name into a
  process-lifetime registry that was never released.
- `xtsetvalue_lua()` freed the typed-arg list but not the converted `ArgList`
  or its string/struct copies.  `xtcreate_lua()` used the success count as the
  offset of the conversion-memory half instead of the original argument count.
- `GetAllResourcesForChild()` leaked the `XtGetConstraintResourceList()`
  copy (and the loop advanced the pointer before any free could happen).
- The normal shutdown path never called `font_cache_flush()` or
  `task_manager_cleanup()`.

**Fix:** free the resource-type copy, add `xt_bridge_cleanup()` for the name
registry, free converted args with the correct input-count offset in both
create and set paths, free the constraint resource list from its saved base
pointer, and call `xt_bridge_cleanup()`, `font_cache_flush()` and
`task_manager_cleanup()` in the normal teardown.

The remaining ASan report is fontconfig/Xft global state created by `FcInit`
(`FcConfig`, `FcFontSet`/`FcValue` caches).  It is not owned by the
application; calling `FcFini()` at shutdown asserts because Xft still holds
cached patterns, and closing the display with `XtDestroyApplicationContext()`
crashes in Xft's display-close hook.  Leave those third-party caches alone (or
suppress them in LSAN) rather than forcing an unsafe teardown.

**Files:** `commander/xt_bridge.c`, `commander/xt_bridge.h`,
`commander/commander_runner.c`

## Constant-string cache was never released at shutdown

**Symptom:** `m_destruct()` printed
`List N still allocated. Source: conststr_init() in m_tool.c` and a list from
`vas_printf()` after closing a LUI app.

**Cause:** widgets call `conststr_init()` lazily, but `commander_runner` never
called `conststr_free()` in its teardown path.  The cache also kept
`CS_ZERO` alive across a free/init cycle because `conststr_free()` did not
reset it.

**Fix:** call `conststr_free()` before `m_destruct()` (normal and bootstrap
failure paths), and reset `CS_ZERO = -1` in both implementations of
`conststr_free()` (`utils/m_tool.c` and `utils/conststr.c`).

**Files:** `commander/commander_runner.c`, `utils/m_tool.c`,
`utils/conststr.c`

## AdmPanel port: optional bracket capture broke `$name` expansion

**Symptom:** `vars.expand('auswahl.sh $kk', {kk='jens'})` returned the
unexpanded string.  `$arr[0]` was also not parsed.

**Cause:** the Lua pattern
`^([%a_][%w_]*)(%[([%d]+)%])?()` did not match when the optional bracket
group was absent.  Lua's capture/position handling inside an optional group
made the whole match fail.

**Fix:** match the identifier first, then check for a following `[` and parse
the index with a separate pattern.  Arrays remain 0-based for legacy
compatibility (`$arr[0]` reads Lua index 1).

**Files:** `apps/admpanel/vars.lua`, `tests/test_launcher_vars.lua`

## AdmPanel port: confirmation prompts with the `?` before the group

**Symptom:** `Delete customer? (y/N)` and `(Y/n?)` were not detected as
confirmation prompts; only `(Y/n)?` with a trailing question mark worked.

**Cause:** the prompt pattern required `%?` immediately after the option
group.

**Fix:** allow an optional trailing `?`, allow `?` inside the group, and
require a question mark somewhere in the line.  This covers
`Continue (Y/n)?`, `(Y/n?)`, `Delete customer? (y/N)` and
`( y / n ) ?`, while still rejecting `Continue (Y/n)` without any question
mark.

**Files:** `apps/admpanel/prompt.lua`, `tests/test_launcher_prompt.lua`

## AdmPanel port: command preview caused an infinite Gridbox layout loop

**Symptom:** the launcher window spun in `GridboxGeometryManager` and produced
hundreds of megabytes of trace output when the script path/command preview
was long.

**Cause:** the command preview was a `Wlabel` with `autoHeight TRUE` inside a
weighted Gridbox.  Long command text re-triggered height negotiation
indefinitely.

**Fix:** use a single-line `WLineEdit` (`showClear FALSE`) for the command
preview.  Long commands scroll horizontally instead of wrapping and no longer
feed the layout loop.

**Files:** `apps/admpanel/ui.lua`, `tests/test_launcher_described.sh`

## AdmPanel port: poll timer reaped the child before the pty was drained

**Symptom:** the last line a script wrote just before exiting sometimes
disappeared from the embedded log (`line2` in `tests/test_launcher_script.sh`).

**Cause:** the 200 ms `waitpid(WNOHANG)` poll timer could reap the exited
child and free/close the pty master before `read_cb` had drained the last
bytes.  The exit event then arrived before the final output line.

**Fix:** while the pty master is still open, `poll_cb` only re-arms; it lets
`read_cb` drain output and observe EOF.  `waitpid` runs after EOF, when no
unread output can remain.  `script_process_cleanup()` also terminates and
frees any jobs still alive at shutdown.

**Files:** `commander/script_process.c`, `commander/commander_runner.c`

## AdmPanel port: CRLF output produced empty log rows

**Symptom:** every normal pty line appeared with an extra blank row in the
`WListView` log.

**Cause:** the line splitter treated `\r` and `\n` as two independent
separators, so `\r\n` produced one real line and one empty line.

**Fix:** consume a following `\n` when the separator is `\r`.

**Files:** `commander/script_process.c`

## WComboBox popup explodes its item geometry with many items

**Symptom:** clicking the `Kuerzel` drop-down in the AdmPanel launcher (a
SQL-backed `WComboBox` with 779 rows in the test database) opened a popup
whose item buttons were each `725x8404` pixels and whose `VBox` y positions
wrapped around 65536. Layout took seconds, memory climbed, and a longer run
took the desktop session down with the OOM killer. The previous interactive
session died exactly this way.

**Cause:** `WComboBox.open_popup()` creates one `Wbutton` per item inside a
`WPopupMenu` (a `VBox`). The popup shell is capped to
`linesHeight * 30` (240 px), but the menu widget keeps its full preferred
height (all items). `VBox.do_layout()` therefore divides that huge height by
the item count and gives every child a huge height; the `Dimension y`
accumulator then wraps. There is no scrolling viewport, so the popup tries
to materialize every item at once. `Wlabel.query_geometry()` also accepts any
larger proposed height, so the bad size sticks.

**Reproduction (safe):**
```bash
./apps/admpanel/run_guarded.sh apps/admpanel/examples/freigabe.lua
# click the Kuerzel combo; or drive it headlessly:
ADMPANEL_DROPDOWN_ITEMS=1000 ADMPANEL_DROPDOWN_MEM_MB=128 \
  ./tests/test_launcher_dropdown_guard.sh
```
The guard caps memory and keeps `run.log`/`meta.txt`. Before the fix this
reproduced the runaway (timeout/nonzero exit); after the fix the same command
completes with a small bounded popup.

**Fix:** `WComboBox.open_popup()` now creates a single scrollable
`WListView` (`rows = items`, `visibleRows = min(linesHeight, item_count)`,
`showHeader`/`gridLines`/`alternateRows` off, horizontal scrollbar off). The
popup height comes from `WListView.query_geometry()` (visible rows times row
height plus the scrollbar), so the number of items no longer affects the
widget geometry or creates one X window per item.

`WListView` gained a `clicked` callback for a single mouse click on a row.
`WComboBox` uses `clicked` and `activate` to commit, but defers the commit
through a zero-delay timeout; the list is therefore never destroyed inside
its own event handler. `notify` updates the combo label live while the user
navigates, and Escape still dismisses without committing.

**Verification:** `tests/test_wcombobox.c` now covers single-click commit,
popup placement, and a 200-item popup whose height stays bounded.
`tests/test_launcher_dropdown_guard.sh` with
`ADMPANEL_DROPDOWN_ITEMS=1000 ADMPANEL_DROPDOWN_MEM_MB=128` used to time out
at the old geometry explosion; it now completes with a ~25 MB peak.

**Status:** resolved.

**Files:** `wbuild_widgets/WComboBox.widget`,
`wbuild_widgets/WListView.widget`, `tests/test_wcombobox.c`,
`tools/run_guarded.sh`, `apps/admpanel/run_guarded.sh`,
`tests/test_launcher_dropdown_guard.sh`

## WComboBoxEdit: opening the popup before realize crashed XTranslateCoords

**Symptom:** calling `open_popup` from a Lua `--after` hook (before the
harness window was realized) aborted with
`X Error of failed request: BadWindow ... X_TranslateCoords`.

**Cause:** `comboedit_open()` called `XTranslateCoordinates(XtWindow($), ...)`
without checking that the composite had a window yet.

**Fix:** return early from `comboedit_open()` unless `XtIsRealized($)` is true.
The arrow click and Down key only happen after realize, so this only affects
programmatic calls.

**Files:** `wbuild_widgets/WComboBoxEdit.widget`

## WComboBoxEdit: "▾" in a Wbutton rendered as tofu

**Symptom:** the dropdown arrow button showed a hollow box instead of a
triangle.

**Cause:** the theme UI font used by `Wlabel`/`Wbutton` does not contain
U+25BE in this environment.

**Fix:** do not use a child `Wbutton` for the arrow. The right strip is part
of the composite's own window; `expose` fills it with the button background,
draws the separator and a small filled triangle, and the `<Btn1Down>` action
`arrow_click` opens/closes the popup. This also removes one child widget per
combo edit.

**Files:** `wbuild_widgets/WComboBoxEdit.widget`

## WComboBoxEdit: typing kept the old selected index

**Symptom:** after choosing a lookup item and then editing the text, the
widget still reported the previous `selected` index. AdmPanel therefore kept
using the old SQL key instead of the typed value.

**Cause:** `edit_changed_cb()` refiltered the popup but never cleared
`selected`.

**Fix:** every user edit sets `selected = -1`; clicking a row or pressing
Return sets it again from the chosen item. The AdmPanel SQL lookup now uses
the typed text (matching a key or display string when possible) while
`selected` is -1.

**Files:** `wbuild_widgets/WComboBoxEdit.widget`, `apps/admpanel/ui.lua`,
`apps/admpanel/sql.lua`, `tests/test_wcomboedit.c`,
`tests/test_launcher_sql_ui.lua`

## WComboBoxEdit lookup returned the display field instead of the key

**Symptom:** the `kunde-auswahl.lua` demo showed the display string
(`jens (db1)`) in the edit field and it was easy to accidentally use that as
the value instead of the configured `sql_key_field` (`jens`).

**Fix:** `WComboBoxEdit` now has an optional `keys` list parallel to `items`
and a `selectedKey` value. AdmPanel fills `keys` from the SQL map and reads
`selectedKey` for the lookup value, so a selected item returns the key while
the edit/list keep showing the display field. Free text still falls back to
the typed value; a typed display string is resolved to its key first.

**Files:** `wbuild_widgets/WComboBoxEdit.widget`, `apps/admpanel/ui.lua`,
`apps/admpanel/sql.lua`, `tests/test_wcomboedit.c`,
`tests/test_launcher_sql_ui.lua`

## Wedit/WLineEdit/WComboBoxEdit: non-ASCII typing corrupted UTF-8

**Symptom:** typing `ä`, `ö`, `ü` or other non-ASCII characters into an edit
field produced invalid UTF-8. Cursor movement, glyph layout and filtering
then behaved incorrectly, and the `WComboBoxEdit` prefix filter could miss
matching items.

**Cause:** `_XawLookupString()`/`XLookupString()` return Latin-1 for many
keysyms (for example `a-diaeresis` arrives as the single byte `0xE4`).
`Wedit.insert_char` inserted those bytes directly into a UTF-8 buffer.

**Fix:** normalize the lookup result before inserting it: keep it when it is
already valid UTF-8, otherwise encode the keysym's Unicode code point as
UTF-8. If the lookup returns no bytes but a Unicode keysym (CJK text injected
via XTEST), encode that keysym directly. This is the same strategy already
used by `KaroEd`. Paste continues to use `XA_UTF8_STRING`, so no extra
conversion is needed there.

**Files:** `wbuild_widgets/Wedit.widget`, `tests/test_wcomboedit.c`

## WLineEdit placeholder was a dangling XtRString pointer

**Symptom:** clearing the new `WComboBoxEdit` (or any empty `WLineEdit`)
aborted with an AddressSanitizer heap-use-after-free in `draw_extras()` at
`strlen(placeholder)`.

**Cause:** `placeholder` was a raw `XtRString` resource. Xt stores the pointer
without copying the string. When the app set the placeholder from Lua, the
Lua string was freed after the call and `draw_extras()` later read freed
memory. The field never showed the placeholder while it held a selected
value, so enabling the clear button exposed the bug.

**Fix:** `Wedit` now owns a 256-byte placeholder copy (`placeholder_buf`).
`initialize` and `set_values` copy the resource value into it and point
`placeholder` at the buffer. The resource was moved from `WLineEdit` to
`Wedit` so all edit widgets use the same ownership rule.

**Files:** `wbuild_widgets/Wedit.widget`, `wbuild_widgets/WLineEdit.widget`,
`apps/admpanel/ui.lua`

## WListView selectedId stayed -1 for a list created with selected/rowIds

**Symptom:** in `WComboBoxEdit`, typing `aa` and pressing Down highlighted
`aamit` in the popup, but pressing Enter did not select it or put it in the
edit field.

**Cause:** `WComboBoxEdit.comboedit_commit()` reads the list's `selectedId`.
A freshly created `WListView` was given `selected = 0` and `rowIds`, but Xt
applies those resources before `initialize()` and does not call `set_values`
for them. `selectedId` therefore kept its default `-1`. Enter fell back to an
exact text match for `aa`, found none, and left the field unchanged.

A second issue was that `comboedit_refilter()` freed and recreated the MLS
lists; if the allocator reused the same addresses, `WListView`'s pointer
comparison could miss the change and keep stale row ids.

**Fix:** `WListView` now recomputes `selectedId` in `get_values_hook()` (and
once in `initialize()`), so reads are always current. `comboedit_refilter()`
clears the list's `rows`, `rowIds` and `selected` before freeing/refilling the
lists, so a refill is always observed even when the addresses are reused.

**Files:** `wbuild_widgets/WListView.widget`,
`wbuild_widgets/WComboBoxEdit.widget`, `tests/test_wcomboedit.c`

## Wedit selection state survived programmatic text changes

**Symptom:** clearing or replacing the text programmatically while a
selection existed aborted with a fatal MLS out-of-bounds error from
`px_char()` in `Wedit.widget`.

**Cause:** `copy_str()` rebuilt the glyph list but left `has_selection`,
`sel_start`/`sel_end` and `crsr_pos` pointing at the old text. `draw_selection`
then asked for a glyph index that no longer existed.

**Fix:** `copy_str()` now clears the selection, resets the cursor and the
horizontal view. `delete_selection()` also clamps `left_char_offset` after the
text shrinks.

## Wedit Backspace/Delete did not rebuild the glyph list

**Symptom:** deleting a single character left the rendered text stale.

**Cause:** `remove_char_at_cursor()` removed bytes from `str_m` but did not
call `str_to_glyphlist()`.

**Fix:** it now rebuilds the glyph list and clamps `left_char_offset`.
`pos_from_x()` also guards the empty-glyph-list case.

**Files:** `wbuild_widgets/Wedit.widget`, `tests/test_wcomboedit.c`

## Inline completion saw the old cursor position

**Symptom:** after typing the second character the edit text was not completed
and no suffix was selected.

**Cause:** `insert_char_at_cursor()` fired the `changed` callback before
`insert_char()` advanced the cursor. `WComboBoxEdit` only completes when the
cursor is at the end of the text, so the callback saw `cursor == len - 1` and
skipped completion.

**Fix:** `insert_char_at_cursor()` no longer notifies; `insert_char()` and
`RequestSelection()` notify after advancing the cursor. Inline completion now
sees the final cursor position.

**Files:** `wbuild_widgets/Wedit.widget`,
`wbuild_widgets/WComboBoxEdit.widget`, `tests/test_wcomboedit.c`

## Wedit get_values_hook crashed on an empty text buffer

**Symptom:** clearing the lookup field and typing `bn` aborted with
`ERROR: mls.c ... Index(-1) out of Bounds` from `get_values_hook()` in
`Wedit.widget`.

**Cause:** `get_values_hook()` assumed `$str_m` always contains at least the
sentinel character. In this state the buffer was empty (`m_len($str_m) == 0`),
so `CHAR($label_m, l-1)` indexed `-1`.

**Fix:** `get_values_hook()` now returns an empty string when the buffer is
empty. `ensure_str_sentinel()` restores the sentinel before inserting or
deleting text, and `draw_cursor()` guards an empty glyph list.

**Files:** `wbuild_widgets/Wedit.widget`,
`wbuild_widgets/WComboBoxEdit.widget`, `tests/test_wcomboedit.c`

## WComboBoxEdit popup stayed small and misplaced after filtering

**Symptom:** after trying several prefixes, typing `s` and pressing Down showed
a one-row list that overlapped the input field.

**Cause:** `comboedit_refilter()` updated `rows`/`rowIds`/`selected` but not
`visibleRows`. The popup therefore kept the height from the first filter
(which may have had only one match). The shell position was only set when the
popup opened, so it could also be stale after layout changes.

**Fix:** `comboedit_refilter()` now sets
`visibleRows = min(linesHeight, filtered_count)`. `comboedit_resize_popup()`
re-anchors the shell below the current field geometry, and `resize`/
`change_managed` re-anchor an open popup.

**Files:** `wbuild_widgets/WComboBoxEdit.widget`, `tests/test_wcomboedit.c`

## Wedit select-all before realize caused BadDrawable

**Symptom:** starting a launcher with an `input` field aborted with
`X Error of failed request: BadDrawable` in `X_PolyFillRectangle`.

**Cause:** `post_build()` selected the initial input text immediately after
`lui.run()`. At that moment the edit had no pixmap yet, so `select_all()` ->
`paint()` used `$pix.data == 0`.

**Fix:** `paint()` and `draw_cursor()` now return early unless the widget is
realized and has a pixmap. The selection state is still stored and is drawn
on the first expose.

**Files:** `wbuild_widgets/Wedit.widget`, `apps/admpanel/ui.lua`

## WLineEdit clear button disappeared after a repaint

**Symptom:** starting `apps/admpanel/run.sh examples/freigabe.lua` showed the
selected SQL lookup value with the small clear `x` at the right edge. Clicking
the first character (or any action that repainted the edit) removed the `x`,
even though the field still had text.

**Cause:** `WLineEdit` drew its clear button (and the empty-field placeholder)
only from its `expose` override, directly on the X window. Every text action
calls `Wedit.paint()`, which copies the text pixmap over the window and never
called the decoration code again. The button therefore survived only until the
next paint.

**Fix:** `Wedit` now has an overridable `draw_extras()` method and calls it at
the end of `paint()`, after the pixmap copy and cursor. `WLineEdit` implements
the hook instead of wrapping `expose`, so the clear button and placeholder are
repainted on every update.

**Files:** `wbuild_widgets/Wedit.widget`,
`wbuild_widgets/WLineEdit.widget`, `tests/test_wcomboedit.c`

## Wmarkdown probe showed a blank window until commander_runner was relinked

**Symptom:** `tools/widget_shot.sh '(Wmarkdown ...)'` and `run_widget.sh`
produced an empty themed background. `xwininfo -tree` showed the probe host
without a child window, even though the widget test and `libxtcw` were current.

**Cause:** `commander_runner` keeps a statically linked `register_wb.o` from
`libxtcw`. `make widgets` generated and registered the new `Wmarkdown` class,
and `make tests` rebuilt `libxtcw`, but the runner binary was not relinked, so
`xtcreate("Wmarkdown", ...)` silently created nothing.

**Fix:** relink the runner after adding a widget: `make -C commander
commander_runner` (or `make commander`). The probe then creates the window.
When a new widget renders blank only in LUI/`widget_shot` but passes its C
test, check `stat build/lib/libxtcw.a commander/commander_runner` first.

**Files:** `makefile`, `commander/makefile`, `wbuild_widgets/Wmarkdown.widget`

## retex stretched the last line of a justified paragraph

**Symptom:** `Wmarkdown` with `alignment = 3` (justify) stretched the final
line of every paragraph across the full width, leaving huge inter-word gaps.

**Cause:** `plain_finish_line()` always appended `ctx->right_skip`; for
`RETEX_ALIGN_JUSTIFY` that is a zero-width glue, so `node_pack_hbox()` spread
the leftover width over the normal inter-word glues on every line, including
the paragraph's last line.

**Fix:** the styled layout detects the last line of a paragraph
(`styled_last_line()`) and `plain_finish_line()` appends a `fil` right-skip for
that line. Only the last line is left-aligned; preceding lines stay justified.

**Files:** `re-tex/src/retex.c`, `tests/test_retex_styled.c`

## Knuth-Plass last line was stretched because of an index mix-up

**Symptom:** justified paragraphs still stretched their final line, and
left-aligned paragraphs could leave a single short word on a line.

**Cause:** `line_break_kp()` stores breakpoints as indices into `bp[]`, but
the line builder compared the **breakpoint index** with the item count
(`bj == n`).  The parfill glue was therefore never added.  The same mix-up
meant the discretionary hyphen list of the last breakpoint was never moved
into the line.

**Fix:** compare `bp[bj]` with `n` and look up the penalty item via
`bp[bj]`.  Ragged/centred lines also get effectively infinite stretch so
the breaker prefers full lines instead of a one-word line.

**Files:** `re-tex/src/linebreak.c`

## Caret and hit-testing broke after paragraphs gained nested word boxes

**Symptom:** after Knuth-Plass, clicking in Wbravo/Wmarkdown jumped to the
wrong byte, and caret positions landed at the end of a word.

**Cause:** `retex_paragraph_get_caret_pos()` and
`retex_paragraph_get_offset_at()` only inspected the direct children of a
line HBox.  Knuth-Plass wraps each word in its own HBox, so the character
nodes were one level deeper.  Space glue source ranges were also ignored.

**Fix:** both functions now recurse into nested HBoxes and treat glue source
ranges as text positions.  `collect_line_range()` also includes glue so a
line ending in a space keeps its real end byte.

**Files:** `re-tex/src/retex.c`

## Plain-layout blank lines disappeared after the Knuth-Plass switch

**Symptom:** `retex_layout_plain()` collapsed a blank line between two hard
breaks, and `test_retex_plain` saw fewer lines than before.

**Cause:** the new paragraph path only emitted a line when it had at least
one word.  Blank segments produced an empty item list and were dropped.

**Fix:** blank segments now emit an empty line with the font's height and a
hidden source anchor, so line count and caret mapping stay correct.

**Files:** `re-tex/src/retex.c`, `tests/test_retex_plain.c`

## Tests silently used stale toolkit libraries

**Symptom:** after changing `libretex`/`libxtcw`, a test binary kept running
the old layout code until its `.c` file was touched.

**Cause:** the generic test rule only depended on `%.c`; the toolkit archives
were not prerequisites, so `make tests` did not relink.

**Fix:** `tests/Makefile` now makes every test target depend on
`$(libdir)/libretex.a` and `$(libdir)/libxtcw.a`.

**Files:** `tests/Makefile`

## Shaped text crashed after ShapedCluster grew new fields

**Symptom:** Wmarkdown/Wbravo segfaulted during the first expose after
`ShapedCluster` gained `x_scale` and `cp`.  GDB showed
`shaping_draw_cluster()` receiving `font = 0x3ff0000000000000` (the bit
pattern of `1.0`).

**Cause:** `re-tex/src/makefile` had no header dependencies.  Only the files
whose `.c` changed were rebuilt; `renderer.o`/`backend_*.o` kept the old
`ShapedCluster` layout and read `x_scale` where the new struct stores `font`.
Classic C ABI mismatch.

**Fix:** the re-tex makefile now uses `-MMD -MP`, includes the generated
`.d` files, and makes every object depend on the library headers.  Header
changes trigger a full re-tex rebuild.

**Files:** `re-tex/src/makefile`

## libhyphen overran the small word buffer for long URLs

**Symptom:** laying out a long URL in a narrow column caused
`free(): invalid next size` / heap corruption.  Valgrind pointed at
`hyphen_breaks()` reading one byte past a `word_size + 5` allocation.

**Cause:** `hnj_hyphen_hyphenate2()` is allowed to grow `hyphenated_word`
and may not NUL-terminate exactly at `word_size + 5` for unusual words.
The caller loop relied on that NUL terminator.

**Fix:** allocate `word_size + 32` bytes for both arrays, force a terminator
at the end, and bound the output scan to `word_size + 31`.

**Files:** `re-tex/src/hyphen.c`

## Per-run features exposed uninitialised RLook fields

**Symptom:** after adding `RLook.features`, Wbravo layout crashed with
`mls.c ... Index(...) out of Bounds` while shaping text.  The invalid handle
was read by `m_str()` in `append_shaped_run()`.

**Cause:** several places built an `RLook` field-by-field and never set the
new `features` field.  `get_look()` (Wbravo), the test helper `mkchar()`, and
the Markdown parser's base look all left it as stack garbage.

**Fix:** initialise `features = 0` in `get_look()`, `mkchar()`, and the
Markdown base look.  `RLook` static initializers zero it automatically.
Wbravo state snapshots now also store the feature string (format version 2).

**Files:** `wbuild_widgets/Wbravo.widget`, `wbuild_widgets/markdown.c`,
`tests/test_retex_styled.c`, `utils/rtable.c`

## retex_layout_ex had no prototype and shifted call arguments

**Symptom:** after exposing `fontFeatures`, Wlabel/Wretex segfaulted during
initial layout.  GDB showed `retex_layout_ex(..., font_size_pt=0.6,
align=13, features=0x3)` — every argument after the face was shifted.

**Cause:** the new `retex_layout_ex()` declaration was added to the source
header in a patch that did not match the actual one-line `retex_layout()`
declaration.  Wlabel.c/Wretex.c therefore saw an implicit declaration and
passed the arguments with the old six-argument convention.

**Fix:** declare `retex_layout_ex()` in `re-tex/src/retex.h`, rebuild/install
the header, and rebuild the widgets.  The same header-dependency rule in
`re-tex/src/makefile` prevents a stale `retex.o`/widget mismatch.

**Files:** `re-tex/src/retex.h`, `wbuild_widgets/Wlabel.widget`,
`wbuild_widgets/Wretex.widget`

## Wmarkdown kept the old layout after programmatic text replacement

**Symptom:** the paged `demo_nibelungen.lua` updated its page header but the
viewer still showed the previous page.  The text resource changed, yet the
paragraph was not re-laid out.

**Cause:** two independent issues:
1. `rebuild_doc()` set `$dirty` but not `$layout_dirty`, so the cached
   `RetexParagraph` was reused with the new document atoms.
2. `set_values()` compared only the `text` pointer.  A caller such as LUI's
   `gui.set` can reuse the same address after the old `text_mem` was freed,
   which made the change invisible.

**Fix:** `rebuild_doc()` sets `$layout_dirty = 1`; `set_values()` compares
the string contents (`strcmp`) as well as the pointer and copies the new
string before freeing the old one.  Scrolling now only re-renders the cached
layout, which also makes large documents responsive.

**Files:** `wbuild_widgets/Wmarkdown.widget`,
`demos/demo_nibelungen.lua`

## Resizing Wmarkdown re-laid out the whole page on every geometry query

**Symptom:** dragging the window of the paged Nibelungenlied demo made the
current page appear to be redrawn continuously.  With 1200-line pages each
redraw took seconds, so resizing felt like a loop.

**Cause:** `query_geometry()` called `calculate_size()`, which lays out the
entire document, for every geometry proposal.  Immediately afterwards the
`resize` method laid the same page out again.  A drag generates many geometry
proposals, so the page was laid out twice per event.

**Fix:** when `autoHeight` is off, accept a proposed width+height directly
(`XtGeometryYes`) and let `resize()` perform the single real layout.  Only
initial/preferred-size queries (and `autoHeight` widgets) run
`calculate_size()`.

**Files:** `wbuild_widgets/Wmarkdown.widget`

## Animated window resize queued one full layout per ConfigureNotify

**Symptom:** while dragging the window, the Nibelungenlied page was laid out
again and again for every intermediate size, so the viewer appeared to redraw
the page continuously.

**Cause:** `resize()` freed the backend and called `redraw_markdown()`
synchronously for every ConfigureNotify.  A 1200-line page takes seconds to
lay out, so the event queue backed up with resize events.

**Fix:** resize is now debounced with `XtAppAddTimeOut(..., 180ms, ...)`.
Every new resize restarts the timer; only the final size is laid out.
`redraw_markdown()` skips work while `resize_pending` is set.  The timer is
scheduled even before the widget is realized (the callback checks realization
itself), because the first ConfigureNotify can arrive during realize; guarding
the timer with `XtIsRealized()` left `resize_pending` stuck forever.

**Files:** `wbuild_widgets/Wmarkdown.widget`, `tests/test_wmarkdown.c`

## Shaping font cache never hit and produced inconsistent advances

**Symptom:** the Nibelungenlied page was letter-spaced/ghosted and took
seconds to lay out.  Shaping "DAS" at 15pt returned advances 17.91/16.22/10.22
instead of 10.73/9.73/6.14.  The same word rendered normally in a standalone
shaper.

**Cause:** `shaping_font_get()` stored fontconfig's **matched** family in
`ShapingFont.family` (e.g. "EB Garamond 12"), but `font_lookup()` compared it
with the **requested** family ("EB Garamond").  The cache therefore never hit:
every word created a new FT_Face/HarfBuzz font.  Depending on the fontconfig
match, different words got different advances, and layout was extremely slow.

**Fix:** cache by the requested family string.  The matched face is still used
to load the file, but it is no longer the cache key.  Page text now shapes
consistently and layout is much faster.

**Files:** `re-tex/src/shaping.c`

## Nibelungenlied demo was slow because of the ASan/-O0 build

**Symptom:** the demo stayed blank for more than five seconds even after the
font-cache fix.

**Cause:** the default build uses `debug_enable=1` (`-O0`, `-DMLS_DEBUG`,
`-fsanitize=address`).  ASan plus unoptimised shaping made startup and the
50-line page layout several times slower.

**Fix / usage:** build the demo release-style:

```bash
make debug_enable=0 libxtcw commander
./run_lui.sh demos/demo_nibelungen.lua
```

The demo also embeds the first page text at widget creation and pre-lays it out
before the window is realised, so the first page is ready when the window maps.
The debug/ASan build still works but remains noticeably slower.

**Files:** `demos/demo_nibelungen.lua`, `wbuild_widgets/Wmarkdown.widget`

## Wmarkdown now coalesces expose events instead of caching geometry

**Symptom:** animated window resizes produced a burst of ConfigureNotify and
Expose events; the old resize debounce still tied rendering to geometry
requests and could keep delaying the final frame.

**Design:** geometry is no longer cached or reflowed in `query_geometry()`.
Every proposed size within `minWidth/minHeight .. maxWidth/maxHeight` is
accepted directly.  `resize()` only marks the layout dirty and records
activity.  The actual work happens from `expose()`.

**Expose timer:** `expose()` and `resize()` set `last_expose_ms` and
`render_pending`, then ensure one `XtAppAddTimeOut` timer is running.
Additional expose events do not restart the timer; the callback checks:

- at least `exposeInterval` ms since the last expose, and
- at least `exposeInterval` ms since the last render.

If either is too recent it reschedules for the remaining time.  When both are
satisfied and `render_pending` is set, it clears the flag and redraws once at
the current size.  `exposeInterval` is a public resource (default 200 ms).

This keeps resizing fluid: dozens of resize/expose events collapse into one
final render.

**Files:** `wbuild_widgets/Wmarkdown.widget`

## Shaped text gained phantom spaces after drawing at non-72 DPI

**Symptom:** `./run_lui.sh demos/demo_nibelungen.lua` on a 120-dpi display
showed words split apart at hyphenation fragments (`Nibelungens t rophe`,
`Durst en`, `eingefl ocht en`); the same page looked correct in a 96-dpi Xvfb.
A Wlabel header could also look letter-spaced.  The gaps appeared after the
first render and grew worse after a window resize.

**Cause:** `shaping_font_get()` created the HarfBuzz font with
`hb_ft_font_create_referenced(ft)` and shared that `FT_Face` with the cairo
font face (`cairo_ft_font_face_create_for_ft_face(ft, 0)`).  cairo resets the
FT char size for the target DPI while drawing, so after a 120-dpi draw the FT
face was 13.33 px/em while HarfBuzz still expected the 8 pt size.  hb-ft reads
glyph advances through the FT face, so the next layout pass returned advances
`120/72` times too wide for some glyphs.  Because the cache key matched, the
same corrupted font object was reused.

**Fix:** build the HarfBuzz font from an `hb_face_t` with the OpenType font
functions instead of hb-ft:

```c
hb_face_t *hb_face = hb_ft_face_create_referenced(ft);
hb = hb_font_create(hb_face);
hb_face_destroy(hb_face);
hb_ot_font_set_funcs(hb);
hb_font_set_scale(hb, size * 64, size * 64);
```

The OpenType funcs read `hmtx`/`GPOS` directly and no longer depend on the
FT face's current pixel size, so cairo can freely change it for rendering.

**Files:** `re-tex/src/shaping.c`

## Wmarkdown horizontal rules were a fixed run of dashes

**Symptom:** a Markdown `---` line rendered as 72 literal `-` characters, so
the "rule" was a dashed line of arbitrary length that did not follow the
widget width or the text margins.

**Fix:** the Markdown parser now emits a single `RATOM_OBJECT` atom with the
new object type `ROBJ_RULE` instead of a run of dashes.  The styled retex
layout recognises a paragraph consisting of that atom and emits one
`NODE_RULE` spanning the paragraph measure (width minus the paragraph
indents), with the usual `space_before`/`space_after` from `fmt_rule()`.
`ROBJ_RULE` is declared in `utils/rtable.h`.

**Files:** `utils/rtable.h`, `wbuild_widgets/markdown.c`, `re-tex/src/retex.c`

## Pixel magic numbers were not DPI-aware

**Symptom:** scrollbar thickness, tab heights and text gaps were fixed pixel
values (e.g. `barSize = 14`, `tabHeight = 26`, Wmarkdown `leftGap = 8`).  On a
120/144-dpi display they became physically thinner while the text kept its
point size, so scrollbars and padding looked cramped.

**Fix:** the physical widget/theme metrics now use the existing `Distance`
converter (`utils/converters.c`) with string defaults such as `"10.5pt"` or
`"3mm"`.  `Distance` is an `int`, so programmatic integer values keep working;
the string default is converted at widget creation through `converters_init()`.

Changed resources:

- `Wviewport.barSize`, `WListView.barSize`: `"10.5pt"` (14 px at 96 dpi)
- `WtabWidget.tabHeight`/`lineGap`: `"19.5pt"`/`"3pt"`
- `Wmarkdown` gaps: `"4.5pt"`/`"6pt"`
- `Wlabel` gaps: `"2.25pt"`/`"0pt"`
- `Wbravo`/`Wretex` gaps: `"3pt"`

Theme metrics now support units too: `theme_metric_distance()` accepts
`"10.5pt"`/`"3mm"`/... and `themes.hdf` uses those for `bar_size`,
`list_row_height`, `list_header_height`, `table_row_height`,
`table_header_height`, `list_cell_pad_x/y` and `icon_size`.  Plain integer
theme values still mean pixels.

The `XtRDistance` converter itself was hardened:

- decimal values (`10.5pt`) are accepted (previously `strtol` stopped at `.`)
- `in` is supported in addition to `pt`/`mm`/`cm`/`px`
- results are rounded instead of truncated (19.5 pt at 120 dpi is 33 px, not 32)
- the destination-size check no longer writes `sizeof(int)` into a shorter
  field; use `Distance` (int), never `Dimension`, for converted resources

The duplicated `get_dpi()/pt2px()/px2pt()` helpers in `Wlabel`, `Wretex`,
`Wmarkdown` and `Wbravo` were replaced by `converters_dpi()`,
`converters_pt2px()` and `converters_px2pt()`.

Remaining follow-up: theme metrics that are not in the list above (e.g.
`control_height`, `control_padding`, `tree_indent`) are still plain pixel
integers and should be migrated the same way.

**Files:** `utils/converters.c`, `utils/converters.h`, `utils/theme.c`,
`utils/theme.h`, `themes.hdf`, `cookbook/themes/cookbook.hdf`, the widget files
above, `tests/test_distance.c`

## `Distance` is part of the generated public headers

Private headers such as `WlabelP.h` use the `Distance` typedef for resource
fields.  Previously every consumer (subclasses and tests) had to include
`converters.h` before the private header, which was easy to forget and broke
external code that only included `WlabelP.h`.

`wbuild` now emits a guarded typedef in every generated public header:

```c
#ifndef XTCW_DISTANCE_DEFINED
#define XTCW_DISTANCE_DEFINED
typedef int Distance;
#endif
```

`utils/converters.h` uses the same guard.  Wheel-based widgets get the typedef
through `Wheel.h`; non-Wheel widgets that also declare `Distance` resources
(`Wviewport`, `WListView`, `WtabWidget`, `VBox`, `HBox`, `Gauge`,
`ScrolledCanvas`, `IconSVG`, ...) get it from their own public header.  The
per-widget `@incl "converters.h"` workarounds and the test include patches were
removed again.

The widget makefile now also depends on the `wbuild` binary, so changing the
generator regenerates the headers instead of leaving stale ones behind.

**Files:** `wbuild/generateh.c`, `utils/converters.h`, `wbuild_widgets/makefile`

### Distance resources need converters_init() in every declaring class

Adding `Distance` defaults such as `"2mm"` to `VBox`, `HBox`, `Gauge`,
`ScrolledCanvas`, `Woptc` and `IconSVG` exposed a second gap: those widgets had
no `class_initialize` calling `converters_init()`.  Xt therefore warned
`No type converter registered for 'String' to 'Distance' conversion` and left
the value at 0 (e.g. VBox `vgap`).  All six widgets now call
`converters_init()` from `class_initialize`, matching `Wlabel`, `Wviewport`,
`WListView`, `WtabWidget` and the other converted widgets.

The `test_wlabel*` and `test_wlistmulti` layout baselines were regenerated: the
hb-ot shaper reports advances with slightly different 1/64-pt rounding than
hb-ft (and a matching line height), so the exact glyph widths changed by a few
1/65536-pt units.  Structure and line breaks are unchanged.

**Files:** `wbuild_widgets/{VBox,HBox,Gauge,ScrolledCanvas,Woptc,IconSVG}.widget`,
`tests/baselines/*.layout`

## Justified paragraphs could start with a one-word first line

**Symptom:** `demos/demo_markdown.lua` rendered the final paragraph as
`Select ... text` on the first line (a huge gap) followed by the rest on the
second line.  The same happened for any justified paragraph whose text is only
slightly longer than one line.

**Cause:** `line_break_kp()` applied normal badness to the final line.  When
the last line was short, the DP preferred a tiny first line plus a nearly full
last line, because both paths cost roughly the same `1e10` infeasible-line
penalty.  TeX's `\parfillskip` (infinite stretch for the last line) was missing
from the DP.

**Fix:** in the candidate loop, `end == n` now uses infinite stretch, so a
short final line is free and the earlier lines are optimised normally.  Added
a regression case to `test_retex_linebreak` (11 words, 100pt measure: the first
line must be about 100pt).

`test_wlistmulti` also passed a string for the `ArrayInt` `columnWidths`
resource.  Xt does not convert explicit string arguments for non-string
resources, so the field held a truncated string pointer and `m_len()` crashed
once the layout changed.  The test now builds a valid MLS handle.

**Files:** `re-tex/src/linebreak.c`, `tests/test_retex_linebreak.c`,
`tests/baselines/test_retex_linebreak.layout`, `tests/test_wlistmulti.c`

## Unmatched backtick runs disappeared from inline text

**Symptom:** the demo line `- Fenced code blocks with ``` or ~~~` rendered as
`Fenced code blocks with    or ~~~`; the three backticks were missing.

**Cause:** `inline_put()` toggled an `int code` flag on every backtick and
dropped the delimiter characters.  A run of three backticks toggled the flag
three times, so all three were consumed and the rest of the line was styled as
code.

**Fix:** a backtick run of length N now opens a code span only if a matching
run of length N exists later on the same line.  The span content is emitted
with the mono look; unmatched runs are emitted literally.  Added a regression
check to `test_markdown` (literal ``` plus a matched `monospan`).

**Files:** `wbuild_widgets/markdown.c`, `tests/test_markdown.c`

## Ctrl+C left the clipboard empty

**Symptom:** after selecting text in Wmarkdown and pressing Ctrl+C, nothing
reached the clipboard in some desktop environments.

**Cause:** `copy_selection()` called `XtOwnSelection()` with `CurrentTime`
and `ConvertSelection()` only answered `UTF8_STRING`.  Clipboard managers ask
for `TARGETS` first; getting no reply, they treated the selection as empty.
`CurrentTime` can also be rejected when a newer timestamp has already been
processed.

**Fix:** own the selection with `XtLastTimestampProcessed()` (falling back to
`CurrentTime`) and answer `TARGETS`, `UTF8_STRING`, `STRING`, `TEXT` and
`TIMESTAMP`.  `test_wmarkdown` now reads the CLIPBOARD selection back and
checks both the UTF-8 text and the advertised TARGETS list.

**Files:** `wbuild_widgets/Wmarkdown.widget`, `tests/test_wmarkdown.c`

## Selection highlight one line above the mouse pointer

**Symptom:** clicking/dragging in Wmarkdown highlighted the line above the
pointer, e.g. selecting the paragraph under the "Wmarkdown" heading highlighted
the heading itself.

**Cause:** `retex_paragraph_render_range()` normalises every line by
`lines[first_line].y`, but `scroll_offset_pt()` returned 0 when
`scroll_line == 0`.  The hit-test therefore compared the mouse y against the
unnormalised line metadata, which includes the first paragraph's
`space_before`.  At the top of the document that shifted the hit by exactly one
line.

**Fix:** `scroll_offset_pt()` now returns `lines[scroll_line].y` for line 0 as
well.  The same fix was applied to Wbravo's cursor/selection hit-test.  Added a
regression check to `test_wmarkdown` that clicks the second visual line and
asserts the selection lands in that line's byte range.

**Files:** `wbuild_widgets/Wmarkdown.widget`, `wbuild_widgets/Wbravo.widget`,
`tests/test_wmarkdown.c`

## WLineEdit did not take keyboard focus on mouse click

**Symptom:** in the AdmPanel `freigabe` form, clicking the `Domain` input
while the `sandbox` option combo had keyboard focus left the focus on the
combo. Typing/arrow keys still acted on the combo instead of the input.

**Cause:** `WComboBox.notify` explicitly calls `focus_self()` on `Btn1Down`,
but `WLineEdit.lineedit_click` only called `set_cursor()`. `Wedit.set_cursor`
moved the text cursor but never called `XtSetKeyboardFocus`, so clicking a
line edit never changed Xt's keyboard focus. The clear-button shortcut in
`lineedit_click` also returned before any focus change.

**Fix:** `Wedit.set_cursor` now calls `wedit_focus_self()`, and
`WLineEdit.lineedit_click` takes focus before the clear-button early return.
Added `tests/test_wlineedit_focus.c`, which focuses a combo and then clicks a
line edit and asserts `XtGetKeyboardFocusWidget()` moved.

**Files:** `wbuild_widgets/Wedit.widget`, `wbuild_widgets/WLineEdit.widget`,
`tests/test_wlineedit_focus.c`, `tests/Makefile`

## AdmPanel tokenizer drops empty quoted arguments

**Symptom:** a launcher command template such as `cmd '' value` starts the
child as `cmd value`; the explicit empty argument is lost. This matters for
scripts that distinguish a missing argument from an empty one.

**Cause:** `vars.tokenize` accumulates characters in `cur` and only appends an
argv entry in `push()` when `#cur > 0`. A quoted empty string (`''` or `""`)
therefore produces no entry.

**Status:** open. Fix by tracking whether the current token was started (for
example a `started` boolean) and pushing it even when its text is empty. Add a
regression case to `tests/test_launcher_vars.lua`.

**Files:** `apps/admpanel/vars.lua`, `tests/test_launcher_vars.lua`

## LUI `(window …)` wraps the first child in a Gridbox

**Symptom:** a fixed-size `WAudioChassis` placed inside
`(window :width 1160 :height 770 (WAudioChassis …))` was resized to
`1152×762` by the implicit Gridbox, so the chassis background and its
pixel-positioned children no longer matched the design.

**Cause:** `lui/backend_xt.lua` converts the first `topLevelShellWidgetClass`
node under the root shell into a `Gridbox`. The actual root shell is the
commander shell; a `(window …)` node therefore adds a layout container
between the shell and the chassis.

**Fix:** use the chassis itself as the top-level LUI node:
`(WAudioChassis :width 1160 :height 770 :chassisWidth 1160 …)`. The chassis
is then the only managed child of the real root shell. It sets the shell
title and WM min/max hints itself.

**Files:** `apps/mp3player/main.lua`, `wbuild_widgets/WAudioChassis.widget`

## WAudioChassis: Board children need Core geometry, not `location`

**Symptom:** arbitrary children such as `Wlabel` do not have Board's
`location`/`abs_x` resources; only Board subclasses do.

**Cause:** `location` belongs to `BoardPart` on the Board widget itself. A
plain child is positioned through its Core `x/y/width/height` resources.

**Fix:** use `:x`, `:y`, `:width`, `:height` for arbitrary children of
`WAudioChassis`; `location` remains available for nested Board widgets.
`tests/test_mp3_chassis.c` verifies 50 `Wlabel` children at exact pixel
positions.

**Files:** `wbuild_widgets/WAudioChassis.widget`, `tests/test_mp3_chassis.c`

## C Lua module: `lua_settop(L, -2)` after building a result table pops it

**Symptom:** `mp3fft.process_pcm_data` returned a number instead of the result
table; Lua reported "attempt to index a number value".

**Cause:** after creating the result/levels/rms/peak tables, the helper called
`lua_settop(L, -2)`. With one value left on the stack, index `-2` resolves to
0 and pops the result.

**Fix:** leave the result table on the stack and return 1 directly.

**Files:** `apps/mp3player/c/mp3fft.c`, `tests/test_mp3_fft.lua`

## ParentRelative child stayed white over a Cairo-drawn chassis

**Symptom:** a `WXftLabel` with `backgroundPixmap = ParentRelative` rendered
a white rectangle over the `WAudioChassis` metal background instead of
showing the texture through.

**Cause:** `ParentRelative` uses the *parent's background pixmap*. The
chassis drew its metal into a private pixmap and only blitted it in
`expose`, but never installed it as the chassis's `backgroundPixmap`, so the
child fell back to the default background.

**Fix:** after rendering the chassis background, set
`XtVaSetValues(chassis, XtNbackgroundPixmap, bg_pixmap, NULL)` and
`XSetWindowBackgroundPixmap(...)`. `destroy_bg` detaches the pixmap again
before freeing it. `WXftLabel` now shows the chassis texture through a
`ParentRelative` window.

**Files:** `wbuild_widgets/WAudioChassis.widget`,
`wbuild_widgets/WXftLabel.widget`,
`tests/test_mp3_xftlabel.c`

## WSeekBar time labels ghosted on every position update

**Symptom:** the `WSeekBar` in the running mp3player showed doubled/overlapping
time digits (e.g. `00:0` + a ghost `0`) after the 100 ms UI tick updated the
position.

**Cause:** `redraw()` drew the track/fill and text into the backing pixmap but
never cleared the whole pixmap first. Each update painted the new text over the
old one, leaving the previous digits behind.

**Fix:** fill the backing pixmap with `panelColor` before drawing the track.
The standalone `WSeekBar` screenshot was clean because it only redrew once.

**Files:** `wbuild_widgets/WSeekBar.widget`, `apps/mp3player/main.lua`

## WVfdDisplay: `state` resource collides with Wheel's integer state

**Symptom:** `@var String state` in the new VFD widget compiled with
pointer/int warnings and would have corrupted `Wheel`'s internal
`state` field (selection state).

**Cause:** `Wheel` already has an integer member named `state`; a subclass
resource with the same name shadows/collides with it.

**Fix:** rename the VFD resource to `playState` (`XtNplayState`). The app
sets `gui.set("vfd", "playState", ...)`.

**Files:** `wbuild_widgets/WVfdDisplay.widget`, `apps/mp3player/main.lua`

## WVfdDisplay redrew on every UI tick / endless expose

**Symptom:** with the mouse inside a transport button, the VFD appeared to
expose continuously. The 100 ms `status_update` called `gui.set("vfd", ...)`
for album/title/time on every tick.

**Cause:** `WVfdDisplay.set_values` compared string **pointers**. The Lua
bridge passes a fresh pointer for identical text, so the comparison always
looked changed and `set_values` returned True, making Xt redisplay the VFD.
The button's Enter/Leave highlight made the effect more visible.

**Fix:** compare string contents with `strcmp` and only redraw when the text
really changed. The app also caches the last VFD values in `last_vfd` and
skips redundant `gui.set` calls. A mouse-over button test now produces a
single VFD expose instead of a continuous stream.

**Files:** `wbuild_widgets/WVfdDisplay.widget`, `apps/mp3player/main.lua`

## Spectrum exposed continuously while paused

**Symptom:** after pausing, the app logged `spec:expose` continuously.

**Cause:** `WSpectrum.set_values` compared the `levels` string by pointer.
The app sent the same spectrum string every 100 ms while paused; each new Lua
string pointer looked different, so `set_values` returned True and Xt kept
redisplaying the widget.

**Fix:** compare `levels`/`label` contents with `strcmp`, copy the new pointer
safely, and cache the last spectrum string in `main.lua`. The widget also
redraws its backing pixmap directly from `set_values` and returns False, so
Xt does not clear/expose the window on every update.

**Files:** `wbuild_widgets/WSpectrum.widget`, `apps/mp3player/main.lua`

## VFD flashed on each time update

**Symptom:** during playback the VFD looked like it was cleared and then
redrawn once per second.

**Cause:** `WVfdDisplay.set_values` returned True, so Xt cleared the widget
window (default background) before calling `expose`, causing a visible flash.
The widget also used pointer comparison for strings.

**Fix:** compare string contents, copy new strings safely, and redraw the
backing pixmap directly from `set_values` while returning False. The rendered
pixmap is also installed as the window background, so a real expose clears to
the previous frame instead of the chassis/default colour.

**Files:** `wbuild_widgets/WVfdDisplay.widget`, `apps/mp3player/main.lua`

## VU meter / SeekBar flickered on every level update

**Symptom:** the VU meters visibly flickered during playback; the same
pattern was possible on the SeekBar.

**Cause:** `WVuMeter.set_values` and `WSeekBar.set_values` returned True when
their integer resources changed (level/peak at 10 Hz, position at 10 Hz).
Xt then cleared the widget window before calling `expose`, producing a flash.
`WVuMeter` also compared its label string by pointer.

**Fix:** both widgets now redraw their backing pixmap directly from
`set_values` and return False, so Xt does not clear/expose on each update.
They also install the rendered pixmap as `backgroundPixmap`, compare labels
with `strcmp`, and detach the pixmap before freeing it on resize/destroy.
`main.lua` already caches VU/seek values, so unchanged values do not trigger
a redraw at all.

**Files:** `wbuild_widgets/WVuMeter.widget`,
`wbuild_widgets/WSeekBar.widget`, `apps/mp3player/main.lua`

## WModulePanel showed black triangles in its rounded corners

**Symptom:** the rounded EQ module panel had black triangular artefacts in
the four corners. The window itself was still rectangular.

**Cause:** `WModulePanel` drew a rounded rectangle into its background
pixmap, but did not shape the X window. Pixels outside the rounded path
remained unpainted/black.

**Fix:** build a 1-bit rounded-rectangle mask and install it with
`XShapeCombineMask(dpy, win, ShapeBounding, ...)` in `realize`, `resize` and
when `cornerRadius` changes. The window corners are now transparent; no
black triangles.

**Files:** `wbuild_widgets/WModulePanel.widget`

## Transparent WVuMeter flickered after switching to ParentRelative

**Symptom:** after the VU meters became transparent, they flickered during
playback because every redraw cleared the window to the parent background
before painting the new level.

**Cause:** the transparent redraw path used `XClearWindow` and then drew
directly on the window. At 10 updates/second the clear/paint sequence was
visible as a flash.

**Fix:** keep a backing pixmap, seed it with the parent panel's pixels via
`XCopyArea(parent, pixmap, ...)`, draw the meter into it, then blit the
pixmap to the VU window. The panel texture stays visible, but there is no
clear-before-paint flash. `set_values` still returns False, so Xt does not
trigger additional exposes.

**Files:** `wbuild_widgets/WVuMeter.widget`

## Play stayed in LOADING when the mpg123 output module failed

**Symptom:** pressing play left the VFD at `LOADING`, no time advanced, VU
and spectrum stayed empty. Direct `mpg123 -o pulse` failed with
`out123 error 3: failure loading driver module`; `alsa` failed to open the
device. The app never fell back.

**Cause:** `player.lua` started `mpg123 -R -o <output>` once and ignored
output-module failures. Non-`@` error lines were discarded, and an early
process exit left the state at `loading`.

**Fix:** `player.lua` now builds an output list from
`player.output` + `player.fallback_outputs` + `dummy`. It captures error
lines, watches for an early exit, and uses a 1.5 s startup timeout. On
failure it stops the old process, starts the next output, re-applies volume
and EQ, and reloads the current track. Default order: `pulse`, `alsa`,
`dummy`.

**Files:** `apps/mp3player/player.lua`, `apps/mp3player/config.lua`

## Transparent VU left ghost pixels after several exposes

**Symptom:** after multiple expose events, coloured ghost pixels remained
around the VU bar.

**Cause:** the transparent VU seeded its backing pixmap with
`XCopyArea(parent_window, ...)`. On X11 the parent window copy can include the
previous content of the VU child window, so old coloured segments were copied
back into the new frame.

**Fix:** read the parent's `XtNbackgroundPixmap` and copy from that pixmap
instead of the parent window. The panel background is then used as the clean
base; the VU child's previous frame cannot leak into it. Fallback remains the
parent window if no background pixmap is set.

**Files:** `wbuild_widgets/WVuMeter.widget`

## mp3engine `--null` decoded far faster than real time

**Symptom:** the DSP engine with `sink="null"` jumped to the end of a
94-second track in well under a second, so pause/seek tests were meaningless.

**Cause:** `pa_simple_write` normally paces playback; with no Pulse sink the
worker loop decoded as fast as possible.

**Fix:** when no Pulse stream exists, the engine sleeps for
`frames / sample_rate` seconds after each block. Null output now behaves like
real time, while `pa_simple_write` still provides the pacing for real sinks.

**Files:** `apps/mp3player/c/mp3engine.c`

## VFD album text overflowed into the time area ("Sa…")

**Symptom:** long album text ran across the VFD separator; its tail appeared
next to `ELAPSED` as `Sa…`. In some runs the VFD then stopped drawing the
remaining text/time because Cairo had entered an error state.

**Cause:** `WVfdDisplay` drew album/title without a width limit. The new
ellipsis helper truncated the already-shortened UTF-8 string at a byte
boundary, producing invalid UTF-8 for `cairo_show_text`.

**Fix:** `vfd_text_fit` now truncates on UTF-8 character boundaries and the
app no longer pre-shortens VFD album/title strings. The elapsed/remaining
labels were also moved down slightly to keep clear of the 7-segment digits.

**Files:** `wbuild_widgets/WVfdDisplay.widget`, `apps/mp3player/main.lua`

## LUI `:fraction` never reached Wsplitter

**Symptom:** a `(splitter ... :fraction 0.3)` always opened with the panes at
50/50. Dragging still worked, but the configured initial split was ignored.

**Cause:** `backend_xt.process_property()` translated both `fraction` and
`frac` to the resource name `frac`. That name exists on the sliders, while
`Wsplitter` exposes `fraction` (0..1000). Xt silently dropped the unknown
argument, so the splitter kept its default 500. The 0..1 ratio used by LUI
callers also needed scaling to the widget's per-mille range.

**Fix:** keep the resource names separate. `fraction` is now passed as
`fraction` and a value in (0, 1] is multiplied by 1000 (rounded); `frac`
remains the unscaled slider resource. `tools/t1_menu.lua` uses the splitter
for its demo list / trace-level panes and `tests/test_t1_menu.sh` asserts the
configured 720 per-mille value.

**Files:** `lui/backend_xt.lua`, `tools/t1_menu.lua`,
`tests/test_t1_menu.sh`

## wbuild: each `@` line in a doc comment becomes its own section

**Symptom:** a multi-line doc comment above a `@proc`/`@var` renders as several
tiny `\Section` blocks in the generated `.tex` (and as separate paragraphs in
the `.3` man page), e.g. the three-line `colors_changed` comment in
`EqFader.widget` was split into three sections.

**Cause:** wbuild starts a new documentation block at every line that begins
with `@`. Only the *first* line of a paragraph may carry the `@`; continuation
lines are plain prose.

**Fix:** prefix only the first line with `@`, and write the remaining lines
without a leading `@`:

```
@ Override the Wheel theme hook: our backing GC caches the background pixel,
and our tick/label colours are EqFader resources (not Wheel ones), so derive
them from the theme foreground to stay readable on dark themes.
@proc void colors_changed
```

This applies to resource (`@var`), method (`@proc`), action and utility
comments. Do **not** repeat the `@` on continuation lines.

**Files:** `wbuild_widgets/EqFader.widget` (fixed); latent in several other
widgets (e.g. `Wbravo.widget`, `WToggleSwitch.widget`, `Wmarkdown.widget`).

## wbuild docs are generated as a side effect of the header build

`make widgets` (and a direct `wb.sh <Widget>.widget ... -d build/doc ...` run)
writes both `<Widget>.tex` and `<Widget>.3` into `build/doc`. wbuild only
rewrites them when the `.widget` source is newer than the output, so `touch`
the widget (or change it) before checking a documentation change.
`tests/test_widget_docs.sh` regenerates a small set into a temporary directory
to verify the resource/method documentation end to end.

## WListView: borrowed `columnTitles` caused heap-use-after-free

**Symptom:** opening a LUI screen with a `WListView` could abort with an ASan
`heap-use-after-free` in `s_split()` from `rebuild_columns()` during realize.
The widget reference browser (`tools/widget_help.lua`) triggered it as soon as
it set `:columnTitles` from Lua.

**Cause:** `columnTitles` is a `String` resource. Xt borrows the pointer, and
`WListView` kept using it in `rebuild_columns()`/`relayout()` after the Lua
string had been garbage-collected. `fontFace` already had an owned `_mem`
copy; `columnTitles` did not.

**Fix:** keep an owned `char *columnTitles_mem` in `@PRIVATE`, copy
`columnTitles` in `initialize`, re-copy it in `set_values` when the resource
changes, and free it in `destroy`. This is the same borrowed-String pattern as
`fontFace` (see CONVENTIONS §27).

**Files:** `wbuild_widgets/WListView.widget`, `tools/widget_help.lua`
(regression trigger)

## wbuild HDF: raw string ending in `]` is ambiguous with `]]`

**Symptom:** `hdfpy`/the Lua HDF reader stopped at an extra `]` for
`WspinBox.hdf` (`description [[set the value from params[0]]]`). The reader
closed the raw string one byte early.

**Cause:** the emitter used `[[...]]` when the content did not contain `]]`,
but a content string that ends with `]` produces `...]]]` together with the
closing delimiter. The first `]]` is then the content's `]` plus the first
closing `]`, leaving one `]` behind.

**Fix:** when the content ends with `]`, force at least one `=` in the raw
string delimiter (`[=[...]=]`), so the close marker starts at the real closing
bracket. `hdf_raw()` in `wbuild/generatedoc_hdf.c` does this.

**Files:** `wbuild/generatedoc_hdf.c`, `tools/hdf.lua`,
`build/doc/hdf/WspinBox.hdf` (regenerated)

## HDF has no tagged objects — use `kind` fields

**Symptom:** a schema like `(resource name XtNfoo ...)` is not an HDF object;
the Lua reader treated `resource` as a key and failed on the odd element count.

**Cause:** in core HDF a bare list is an object made of alternating key/value
pairs. Only `seq` is a structural tag. There is no `(tag ...)` object form.

**Fix:** emit explicit `kind` fields and keep every object list even:

```lisp
(resources (seq
  (kind resource
    name XtNfoo
    ...)))
```

The same applies to methods, actions, translations, exports, classvars,
utilities and the root (`kind widget`, `kind index`).

**Files:** `wbuild/generatedoc_hdf.c`, `tools/hdf.lua`,
`tools/widget_help.lua`

## wbuild runs once per widget — a single `index.hdf` is incomplete

**Symptom:** `build/doc/hdf/index.hdf` contained only the last widget
(`WXftLabel`).

**Cause:** `wb.sh` invokes `wbuild` once per `.widget`. `classes` therefore
contains only the class being generated, so generating an index from the
`classes` list in `main()` sees just that one class and overwrites the index
on every run.

**Fix:** do not generate the index inside a per-widget `wbuild` run. The
widget reference browser scans `build/doc/hdf/*.hdf` instead. If a single
index is needed later, generate it after all widgets have been processed
(e.g. from `register-widgets.sh` or a dedicated post-processing step).

**Files:** `wbuild/wbuild.c`, `tools/widget_help.lua`

## Wmarkdown links: Lua callback bridge does not forward Xt call_data

**Symptom:** a `LUA(on_link)` callback on Wmarkdown's `linkActivated` could
not see the clicked anchor; the generic bridge calls `on_link()` with no
arguments.

**Cause:** `commander/commander_runner.c`'s `LUA_callback()` uses the
registered client_data string and ignores `call_data`. Wmarkdown passes the
target as `call_data` (correct for C callbacks), but Lua never receives it.

**Fix:** Wmarkdown also stores the target in the `lastLink` String resource
before invoking the callback. The Lua browser reads
`gui.get("viewer", "lastLink")`. C callbacks still get the target as
`call_data`.

**Files:** `wbuild_widgets/Wmarkdown.widget`, `tools/widget_help.lua`

## Help browser: U+2192 (→) is not rendered by the UI font

**Symptom:** the EqFader translations line showed
`<Btn1Down>  fader_press` — the right arrow was missing.

**Cause:** the help browser used the Unicode arrow U+2192 in the Markdown it
feeds to Wmarkdown. The active UI font/renderer path does not provide a glyph
for it, so it rendered as nothing.

**Fix:** use the ASCII sequence `->` in generated help text
(`tools/widget_help.lua`). The self-test now asserts that the translation line
contains ` -> ` and no U+2192.

**Files:** `tools/widget_help.lua`

## Wsplitter: grab band did not match the drawn divider

**Symptom:** the splitter line was hard to grab; the pointer could be on the
visible divider without starting a drag (or start dragging just next to it).

**Cause:** `start_drag` computed the divider position as
`width * fraction / 1000`, while `do_layout` and `draw_divider` use
`(width - dividerWidth) * fraction / 1000`. The hit band was therefore offset
from the visible line by up to `dividerWidth`.

**Fix:** a single `divider_position()` helper now feeds draw, layout/hit
testing and the resize-cursor check. The widget also shows the same
`XC_sb_h_double_arrow` / `XC_sb_v_double_arrow` resize cursor as WListView's
resizable column headers while the pointer is over the divider.

**Files:** `wbuild_widgets/Wsplitter.widget`

## `make all` rebuilt everything on every run (stale-copy timestamps)

**Symptom:** starting `t1.sh` (which calls `start.sh` → `make all`) printed
hundreds of `wb.sh`/`cc` commands and rebuilt the whole toolkit even when
nothing had changed.

**Cause:** several install steps copied files with plain `cp`, refreshing the
destination mtime on every run. The worst offenders were:
- `wbuild`/`wbuild_widgets` install refreshed `build/bin/wbuild`, so every
  widget header looked older than the generator.
- `cp build/include/xtcw/*.h build/source/` refreshed headers that the
  generated `.d` files reference by their relative `build/source` names, so
  every widget object was rebuilt.
- `wbuild_widgets/Gridbox.widget` is `@nocode`; its header is installed by
  `plainc_widgets`, so make kept invoking wbuild for a target wbuild never
  writes.

**Fix:** use `cp -p` for install/copy steps so mtimes are preserved, exclude
`@nocode` widgets from `WIDGET_H`, and align unchanged generated headers with
the generator mtime once. Repeated `make all` now takes ~0.3 s with no
compiles, links or wbuild invocations.

**Files:** `makefile`, `wbuild/makefile`, `wbuild_widgets/makefile`,
`plainc_widgets/makefile`, `utils/makefile`, `utils/utf8/makefile`,
`utils/utf32/makefile`, `wcl/makefile`, `re-tex/src/makefile`,
`LuaRunner/nanosvg/src/makefile`

## wbuild: `@CONSTRAINTS` documentation generation segfault

**Symptom:** a `.widget` using `@CONSTRAINTS` made `wbuild` exit with
`SIGSEGV` (139) when documentation generation was enabled (`-d ... tex.w
nroff.w`), which is the normal `make widgets` path. C/header generation alone
worked.

**Cause:** `wbuild/tex.w` defined the plural tag with a typo:
`@t contraints` instead of `@t constraints`. `t_constraints` therefore stayed
NULL for the TeX backend, and `constraint_resources()` in `generatedoc.c`
called `fputs(topen(t_constraints), f)` unconditionally. The crash was at
`generatedoc.c:427`; `nroff.w` already had the correct tag name.

**Fix:** corrected the tag name in `wbuild/tex.w` and added defensive NULL
guards around the `t_constraints`, `t_constraint` and `t_section` open/close
strings in `generatedoc.c`, so a backend that omits a tag degrades to plain
text instead of crashing.

**Files:** `wbuild/tex.w`, `wbuild/generatedoc.c`

Note: `@CONSTRAINTS` C generation also derives resource classes/types from the
field name and C type (e.g. `XtCGridx`/`XtRInt` instead of
`XtCPosition`/`XtRGravity`). For a faithful port of an existing widget, keep
using `@private-constraints` plus an explicit `XtResource` table.

## Gridbox string `fill` arguments were never converted

**Symptom:** `XtVaCreateManagedWidget(..., XtNfill, "both", ...)` stored a
truncated string pointer in the `FillType` constraint field instead of
`FillBoth`; string fill modes appeared to be broken.

**Cause:** Xt does not convert raw `Arg` values from strings automatically.
Only `XtVaTypedArg` asks Xt to run the registered type converter. The
`_CvtStringToFillType` converter was registered correctly but was never called
for the raw `XtNfill, "..."` arguments. The converter also overwrote
`toVal->size` before checking it, making the size guard dead code.

**Fix:** use `XtVaTypedArg` for string fill values in the active tests
(`test_gridbox`, `test_selection_wlabel_v2`, `test_wtreeview`) and in the ported
Gridbox test. Corrected the converter's `toVal->size` handling in
`wbuild_widgets/gridbox_core.c` and the ported `gridbox_core.c`.

**Files:** `wbuild_widgets/gridbox_core.c`,
`experimental/port-gridbox/gridbox_core.c`, `tests/test_gridbox.c`,
`tests/test_selection_wlabel_v2.c`, `tests/test_wtreeview.c`

## Gridbox wbuild port: geometry_manager got the parent instead of the child

**Symptom:** after replacing the plain-C Gridbox with the wbuild wrapper,
`test_wdialog` segfaulted in `GridboxGeometryManager` at
`gc->gridbox.prefWidth` with `gc == NULL`.

**Cause:** in wbuild, `$` inside `geometry_manager` is `XtParent(child)`. The
wrapper passed `$` to `gridbox_core_geometry_manager()`, but the core function
expects the child requesting geometry (`w->core.constraints`). It therefore
read the parent's constraint record, which was NULL.

**Fix:** the wrapper now calls
`gridbox_core_geometry_manager(child, request, reply)`.

**Files:** `wbuild_widgets/Gridbox.widget`,
`experimental/port-gridbox/Gridbox.widget`
