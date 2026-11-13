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

**Fix:** added `shrinkCells()` in `plainc_widgets/Gridbox.c`. On a shortfall it
first shrinks weighted cells, proportional to weight and down to
`defaultDistance*2+1`; the old proportional-all behaviour remains as a second
pass when weighted cells cannot absorb the deficit (e.g. no weighted children,
or an unweighted-only grid that a parent refuses to grow). Applied to both the
column and row paths.

**File:** `plainc_widgets/Gridbox.c`

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
