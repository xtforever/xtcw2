# wbuild/Xt Widget Coding Conventions

> The single source of truth for writing `.widget` files. Follow these rules and your widget will compile, render, and interact correctly with Xt geometry management.

Modern reference widgets worth reading alongside this document:

| Widget | Shows |
|--------|-------|
| `Wbravo.widget` | rich text view, Xt action interface, theme colours, external-type-free header, undo/state snapshots |
| `WListView.widget` | Composite with managed slider children, backing pixmap cache, fixed header + scrolling body, icons, `theme_register` without `Wheel` |
| `WfileChooser.widget` | `Gridbox` composition, list model/icon cells, navigation and output resources |
| `Wviewport.widget` | native scroll container (clip + reparented content + sliders) |
| `Wheel.widget` | theme registry/`colors_changed` base pattern |

For UI layout, typography, spacing, colour roles and dialog anatomy see
**[`style-guide.md`](../style-guide.md)**.

## 1. Class Declaration

```wbuild
@class WidgetName (Superclass)
```

Every widget inherits from a superclass. The class hierarchy is resolved by `wb.sh` at build time. Common superclasses:

| Superclass | Use when |
|------------|----------|
| `Core` | Base widget with no children |
| `Wheel` | Widget using Xft fonts/colors (most display widgets) |
| `Composite` | Widget that manages children |
| `Constraint` | Widget that manages children with per-child layout data (Gridbox) |
| `Shell` / `TopLevelShell` | Top-level windows |

## 2. Section Order

Sections can appear in any order; this is the recommended convention:

| Tag | Purpose |
|-----|---------|
| `@EXPORTS` | Public API: functions visible to other widgets |
| `@CLASSVARS` | Class-level shared variables (must have `=` initializer) |
| `@PUBLIC` | Xt resources (settable via `XtVaSetValues`) |
| `@PRIVATE` | Internal instance variables |
| `@CONSTRAINTS` | Per-child layout data (Constraint subclasses only) |
| `@METHODS` | Overrideable Xt virtual methods |
| `@ACTIONS` | Event action procedures |
| `@TRANSLATIONS` | Default event-to-action mappings |
| `@UTILITIES` | Static helper functions |
| `@IMPORTS` | `#include` directives |

## 3. `@TRANSLATIONS` — CRITICAL

**MUST use `@trans` prefix, NOT `@`:**

```wbuild
@TRANSLATIONS
@trans <Btn1Down>:    select_start()
@trans <Btn1Motion>:  select_extend()
@trans <Key>Return:   info()
```

**WRONG** (will be silently ignored by wbuild):
```wbuild
@ <Btn1Down>: select_start()
```

## 4. The `$` Macro System

| Syntax | Expands to | Use |
|--------|-----------|-----|
| `$` | `self` (widget pointer) | Reference to current instance |
| `$field` | `((WidgetClass)self)->part.field` | Access own instance variable |
| `$old$field` | `((WidgetClass)old)->part.field` | Access previous value in `set_values` |
| `$child$field` | Child constraint/resource | Access child's resource or constraint |
| `#method(...)` | Superclass method call | Call inherited implementation |

## 5. Resource Types (`@PUBLIC`)

```wbuild
@var <ResourceType> CType varName = <DefaultType> defaultValue
```

String resources MUST be owned by the widget (X11 passes borrowed pointers):
```wbuild
@PRIVATE
@var String text_mem

@proc initialize {
    if (!$text || !*$text) $text = $name;
    $text_mem = XtNewString($text);
    $text = $text_mem;
}

@proc destroy {
    XtFree($text_mem);
}

@proc set_values {
    if ($text != $old$text) {
        XtFree($old$text_mem);
        if ($text == 0) $text = "";
        $text = $text_mem = XtNewString($text);
    }
}
```

## 6. Geometry Methods — When You Need Them

### `query_geometry` — STRONGLY RECOMMENDED for all display widgets

The Xt spec (CH06) says: if `query_geometry` is NULL, `XtQueryGeometry` returns `XtGeometryYes` with the widget's current geometry as preferred. This is technically valid but **often produces wrong results** in practice.

Per the O'Reilly X Toolkit Intrinsics Programming Manual (Ch 6.6): "If your widget specifies null in the class structure for the query_geometry method, the parent will be told that your widget's current geometry is its preferred geometry. This is often wrong information." And: "Even if your widget has no particular preference for size, it is a good idea to specify the widget's default size in query_geometry."

**When it matters most:**
1. **Content-dependent sizing** — preferred size changes with content (text, list items)
2. **Differential flexibility** — "I can be any width, but my height must be at least X"
3. **Child of a layout manager** — Gridbox/VBox/HBox queries children for preferred sizes

**Contract** (from Xt spec + O'Reilly book):
```c
XtGeometryResult query_geometry(self, request, reply):
    examine: request->request_mode   // what parent plans to change
    compute: preferred size based on content + request constraints
    store:   reply->request_mode = CWWidth | CWHeight
             reply->width  = preferred_width
             reply->height = preferred_height
    return:
        XtGeometryYes    — parent's proposal is fine as-is
        XtGeometryAlmost — child has a different preference (MOST COMMON)
        XtGeometryNo     — child prefers its current geometry unchanged
```

**Reference implementation** (Wretex.widget):
```wbuild
@proc query_geometry
{
    int tw = (request->request_mode & CWWidth) ? request->width : 0;
    int w, h;
    calculate_size($, tw, &w, &h);

    reply->request_mode = CWWidth | CWHeight;
    reply->width = w;
    reply->height = h;

    $prefered_width = w;
    $prefered_height = h;

    return XtGeometryAlmost;
}
```

### `resize` — needed for display widgets

Per Xt spec: "If a class need not recalculate anything when resized, it can specify NULL. This is unusual and should occur only for widgets with very trivial display semantics."

**Pattern:**
```wbuild
@proc resize
{
    if ($backend_ptr) {
        Backend *be = (Backend*)$backend_ptr;
        be->destroy(be);
        $backend_ptr = NULL;
        $pixmap = 0;
    }
    $dirty = 1;
    redraw_widget($);
}
```

### `expose` — needed for display widgets

```wbuild
@proc expose
{
    redraw_widget($);   // delegates to function that checks dirty, updates cache, copies pixmap
}
```

### `set_values_almost` — MUST NOT be NULL

Per the O'Reilly book (Ch 11): **"You should never specify a null set_values_almost method because Xt will print a warning message when set_values_almost would have been called, and continue as if it had been called and had returned XtGeometryYes."**

- If you don't need custom behavior: inherit from Core with `XtInheritSetValuesAlmost` (the default in wbuild). This inherited version **always approves the parent's compromise.**
- If you need custom behavior (set dirty flag, recalculate layout): implement as shown below.
- **Never set it to NULL.**

```wbuild
@proc set_values_almost
{
    request->width = reply->width;
    request->height = reply->height;
    request->request_mode = CWWidth | CWHeight;
    $dirty = 1;
}
```

**To terminate negotiation without accepting:** set `request->request_mode = 0`.

**When geometry_manager returns XtGeometryNo** (no compromise offered): `set_values_almost` should usually just set `request->request_mode = 0` to terminate. The inherited method handles this correctly.

### `geometry_manager` — needed for Composite subclasses

```wbuild
@proc geometry_manager
{
    /* Note: wbuild auto-inserts: Widget $ = XtParent(child); */
    Dimension wd = request->request_mode & CWWidth  ? request->width  : $child$width;
    Dimension ht = request->request_mode & CWHeight ? request->height : $child$height;
    XtConfigureWidget(child, x, y, wd, ht, 0);
    return XtGeometryDone;
}
```

## 7. Constraint Widgets — CRITICAL

**Constraint widgets MUST inherit from `Constraint`, not `Composite`:**

```wbuild
@class Gridbox (Constraint)   # CORRECT
```

**WRONG** (wbuild stubs will generate `compositeClassRec` superclass, children stack at 0,0):
```wbuild
@class Gridbox (Composite)    # WRONG — constraint resources silently dropped
```

Constraint resources go in `@CONSTRAINTS`:
```wbuild
@CONSTRAINTS
@var Position gridx = 0
@var Position gridy = 0
@var int weightx = 0
@var int weighty = 0
```

Access child constraints via `$child$field` — wbuild resolves through the constraint record.

## 8. Shell & TopLevel — allowShellResize

Top-level windows MUST set `allowShellResize: True`:

```wbuild
# In @PUBLIC of a Shell subclass:
@var Boolean allow_shell_resize <allowShellResize> = True
```

Or set at creation time:
```c
XtVaSetValues(shell, XtNallowShellResize, True, NULL);
```

## 9. `set_values` — Resource Change Handling

```wbuild
@proc set_values
{
    int do_expose = 0;
    int size_changed = 0;

    if ($text != $old$text) {
        XtFree($old$text_mem);
        if ($text == 0) $text = "";
        $text = $text_mem = XtNewString($text);
        size_changed = 1;
        do_expose = 1;
    }

    if (size_changed && $autoHeight) {
        int w, h;
        calculate_size($, 0, &w, &h);
        if (w != $width || h != $height) {
            XtVaSetValues($, XtNwidth, (Dimension)w, XtNheight, (Dimension)h, NULL);
        }
    }

    $dirty |= do_expose;
    return do_expose;
}
```

## 10. Gridbox Child Constraints

Children of Gridbox/VBox/HBox must specify layout hints:

| Resource | Type | Default | Purpose |
|----------|------|---------|---------|
| `weightx` | int | 0 | Horizontal stretch weight (extra space ratio) |
| `weighty` | int | 0 | Vertical stretch weight |
| `fill` | FillType | FillBoth | How child fills cell: 0=None, 1=X, 2=Y, 3=Both |
| `gridx` | Position | 0 | Column position |
| `gridy` | Position | 0 | Row position |
| `gridWidth` | Dimension | 1 | Column span |
| `gridHeight` | Dimension | 1 | Row span |

Example (LUI):
```lua
(Wlabel :label "Title" :weightx 1 :weighty 1 :fill 3)
```

Without `weightx`/`weighty` > 0, children get minimum size and extra space is unused.

## 11. Numeric Types — Prevent Underflow

**ALWAYS use signed `int` for intermediate geometry math, never unsigned `Dimension`:**

```c
// WRONG — unsigned underflow produces 65534:
Dimension child_w = cell_size - margin - border;

// CORRECT — signed math with min clamp:
int child_w = (int)cell_size - (int)margin - (int)border;
if (child_w < 1) child_w = 1;
```

**ALWAYS use `ceil()` for subpixel-to-pixel conversions, never `int` cast:**
```c
// WRONG — truncates 23.9 to 23:
int pixels = (int)subpixel_value;

// CORRECT — rounds up to avoid clipping:
int pixels = (int)ceil(subpixel_value);
```

## 12. Expose Pattern — Pixmap Backed Widgets

```wbuild
@proc expose
{
    redraw_widget($);
}
```

Where `redraw_widget` checks realized state, rebuilds cache if dirty, copies pixmap:
```c
void redraw_widget(Widget self) {
    if (!XtIsRealized($)) return;
    if ($dirty || !$backend_ptr) {
        update_cache($);
        $dirty = 0;
    }
    if ($pixmap && XtWindow($)) {
        int w = $width > 4000 ? 4000 : $width;    // X11 server limit
        int h = $height > 4000 ? 4000 : $height;
        XCopyArea(XtDisplay($), $pixmap, XtWindow($), $gc[0], 0, 0, w, h, 0, 0);
    }
}
```

## 13. Property Propagation in Composites

When a composite widget wraps a child, `set_values` MUST forward relevant properties:

```wbuild
@proc set_values
{
    if ($fontSize != $old$fontSize) {
        XtVaSetValues($child_widget, XtNfontSize, $fontSize, NULL);
    }
    if ($retexCells != $old$retexCells) {
        XtVaSetValues($child_widget, XtNretexCells, $retexCells, NULL);
    }
}
```

## 14. Link Order — Stubs vs Real

When a widget has BOTH a wbuild stub (generated `.c`) AND a hand-written implementation (in `plainc_widgets/`), the **real implementation must take precedence in link order**.

- wbuild stubs for constraint widgets may inherit from `compositeClassRec` instead of `constraintClassRec`
- The stub has a smaller class record, missing constraint resources
- If linked before the real implementation, the broken stub wins

## 15. Quick Reference: Geometry Protocol

| Step | Direction | Method | Returns |
|------|-----------|--------|---------|
| Parent queries child's preferred size | Parent → Child | `query_geometry()` | `XtGeometryAlmost` (usually) |
| Parent computes layout | Parent internal | — | — |
| Parent offers compromise | Parent → Child | Returns `XtGeometryAlmost` | compromise in `reply` |
| Child accepts compromise | Child | `set_values_almost()` | modifies `request` to match |
| Parent applies size to child | Parent → Child | `resize()` | void |
| Window needs repaint | System → Widget | `expose()` | void |

## 16. Checklist for New Widgets

- [ ] `@class` inherits from correct superclass (Constraint for layout managers)
- [ ] `@TRANSLATIONS` uses `@trans` prefix
- [ ] String resources use `_mem` pattern with `XtNewString`/`XtFree`
- [ ] `set_values` compares `$field != $old$field`, returns do_expose
- [ ] `resize` destroys size-dependent backends, sets `$dirty = 1`
- [ ] `expose` delegates to redraw function
- [ ] `query_geometry` implemented if content-dependent sizing or child of layout manager
- [ ] `set_values_almost` implemented if `query_geometry` exists
- [ ] `geometry_manager` implemented if subclass of Composite
- [ ] Intermediate math uses `int`, not `Dimension`
- [ ] Subpixel conversions use `ceil()`, not `int` cast
- [ ] Shell allows resize: `allowShellResize = True`
- [ ] Composite propagates properties to children in `set_values`
- [ ] `initialize` registers with the theme registry, `destroy` unregisters it (§18)
- [ ] Subclass `colors_changed` override declared as `@proc void colors_changed` (no parameter list) and calls `#colors_changed($)` (§19)
- [ ] `@CLASSVARS` fields use an `=` initializer (§20)
- [ ] Lifecycle methods trace entry with `XTFUNC()` (§21)
- [ ] Public resources and helper functions documented with `@` comments (§21)
- [ ] Adding a method to a base class → `make clean && make` (§18)
- [ ] Generated header is self-contained; external types stored as `void *` or their header is pulled in via `@EXPORTS` (§22)
- [ ] Composite children created in `initialize` and given non-zero geometry before realize (§23)
- [ ] Backing pixmap cache tracks its size and is recreated on resize (§24)
- [ ] Scrolled content is clipped to the body; fixed header/footer never overdrawn (§25)
- [ ] List/row data ownership documented (borrowed vs widget-owned) (§26)
- [ ] Mutable/output String resources owned via `*_mem` (§27)
- [ ] Application-facing commands exposed as `@ACTIONS` (§28)
- [ ] Undo/redo side tables snapshotted with the model (§29)
- [ ] Non-`Wheel` widgets register/ unregister with the theme registry (§30)
- [ ] Fixed chrome uses `weight* 0`, scrolling body `weight* 1` (§31)
- [ ] Registered in `register-widgets.sh`
- [ ] Added to LUI registry in `lui/registry.lua`, with resource/callback aliases (§32)
- [ ] Resize and fractional-scroll covered by a test (§33)
- [ ] No inherited resource is redeclared; `@def` uses ` = ` (§34)

## 17. Known Bug Patterns (see `know_bugs.md`)

| Bug | Cause | Fix |
|-----|-------|-----|
| TRANSLATIONS not working | `@` instead of `@trans` | Use `@trans` prefix |
| Gridbox children at (0,0) | `@class` inherits from Composite not Constraint | Use `@class Widget (Constraint)` |
| Text truncated | `int` cast of subpixel values | Use `ceil()` |
| Widget gets 65534px | `Dimension` math underflows | Use signed `int` + min clamp |
| Selection not rendering | `$dirty` not set in select actions | Set `$dirty = 1` |
| Properties lost in composites | `set_values` doesn't forward to child | Add explicit `XtVaSetValues` |
| query_geometry missing | Widget has no preferred width | Implement `query_geometry` |
| Link picks wrong class record | Broken wbuild stub linked first | Remove stub or reorder link line |
| `XtFree` crash on constraints | Freeing static class data | Never free `XtGetConstraintResourceList` result |
| Method override build failure | An override repeats the inherited parameter list, e.g. `@proc void colors_changed($)` | Omit the parameter list on overrides: `@proc void colors_changed` (§19) |
| Widget colours/caches don't follow the theme | Widget caches a GC/pixmap or has its own colour resources; the base hook doesn't cover them | Override `colors_changed`, call `#colors_changed($)`, re-resolve and redraw (§19) |
| `unknown type name '<T>'` including `<Widget>P.h` | External type in `@PRIVATE`/`@PUBLIC`; `@IMPORTS` are not emitted into the header | Store as `void *` or add `@EXPORTS` + `@incl` of the defining header (§22) |
| `Error: Widget X has zero width and/or height` | Managed child realized before geometry was assigned | Configure children in `relayout()` before realize; never map 0×0 (§23) |
| Scrollbar grows but list/content stays old size | Backing pixmap cache condition is always true | Track `cache_w`/`cache_h`, recreate pixmap + drawable (§24) |
| `BadDrawable` on `XPutImage` after resize | Cached `XImage2` icon holds the destroyed drawable | Set `im->d = $pixmap` before `xim2_put_image()` (§24) |
| Rows paint over a fixed header when dragged | Fractional slider `top_y`; row `y` above the body band | Clip background/text/grid/icons to the body band (§25) |
| Fixed footer/header clipped on first layout | Gridbox shrank every row proportionally when smaller than preferred | Weighted-first shrink in `Gridbox.layout()` (§31) |
| `@def FOO 1` parse error | `@def` requires an `=` | `@def FOO = 1` (§34) |
| Callback list double free at destroy | Subclass redeclared an inherited `callback` resource | Remove the duplicate `@var`; use the inherited field (§34) |
| `heap-use-after-free` in `strcmp`/`fnmatch` during a later relayout | String resource still points at a Lua string that was GC'd | Copy borrowed strings in `initialize`/`set_values` (§27) |
| Crash in `#destroy($)` / `core_class.destroy` is NULL | The parent class record has no destroy method (e.g. `Wbutton`) | Use an `XtNdestroyCallback` to free owned data (§27) |

## 18. Theme / Color Registry — Registration & Calling Pattern

The central registry (`utils/theme.h`) is a global, MLS-backed store of name/value
pairs (colours, integers such as `borderwidth`, and strings such as
`font_ui`). Widgets register an "apply theme" function; `theme_select()` copies
the selected theme into the active map and calls `theme_apply()`, which invokes
the apply function of every registered widget.

### Registration pattern

Register the **exported dispatcher**, not the virtual method — that is what lets
subclasses override the hook:

```wbuild
@proc initialize
{
    ...
    theme_register($, wheel_colors_changed);
}

@proc destroy
{
    ...
    theme_unregister($);
}
```

Any widget that does not inherit `Wheel` can still take part: define
`own_apply_fn(Widget)` and call `theme_register($, own_apply_fn)` in `initialize`,
and `theme_unregister($)` in `destroy`.

### Calling pattern — safe dispatch from `@EXPORTS`

The exported functions in `@EXPORTS` are the public C API. They guard on the class
and method pointer, then dispatch to the overrideable virtual method; otherwise
they warn. From `Wheel.widget`:

```wbuild
@EXPORTS
@type wheel_colors = enum { ... }
@def COLOR_FG_MAX = 3

@ Called by the theme registry to (re)apply all theme properties.
@ Widgets must override |colors_changed| to handle their own extra colours.
@proc void wheel_colors_changed($)
{
  if( XtIsSubclass( $, wheelWidgetClass) && $colors_changed )
    {
      $colors_changed($);
      return;
    }

  XtWarning( "Widget is not subclass of wheel" );
}
```

`wheel_exec_command` and `wheel_sig_recv` follow the same shape. The registered
callback is a plain `theme_apply_fn` (`void (*)(Widget)`), so pass the exported
dispatcher rather than the virtual method directly.

### Widget-facing query API

| Function | Returns | Use |
|----------|---------|-----|
| `theme_color_defined(name)` | int | Is `name` a defined colour in the active theme? |
| `get_color(name)` | `Pixel` | Resolve a theme colour to a Pixel (0 if undefined / no colour context) |
| `theme_int_defined(name)` | int | Is `name` a defined integer? |
| `theme_get_int(name, &out)` | int | Fetch an integer (1 on success) |
| `theme_metric(name, def)` | int | Integer theme value, or `def` when undefined |
| `theme_str_defined(name)` | int | Is `name` a defined string? |
| `theme_get_str(name)` | `const char *` | Borrowed string value (NULL if undefined) |
| `theme_font_spec()` | `const char *` | Active `font_ui` spec (borrowed) |
| `theme_font_parts(spec, face, size, &px)` | int | Split `Face-NN`; 1 when a size was parsed |

`theme_select()` / `theme_apply()` are driven by the application, not by widgets;
widgets only register and query.

### Cautions

- Never change `borderWidth` / `borderColor` on a geometry-managed child with
  `XtVaSetValues` — it loops the parent's geometry negotiation. Set the core field
  and update the X window directly with `XSetWindowBorder*` (see `Wheel.widget`'s
  `colors_changed`).
- Adding a method to a base class grows its ClassPart. The generated
  `build/source/makefile` has no header dependencies, so stale subclass objects
  alias the new slot — run `make clean && make` after changing a base class's
  method set.

## 19. Overriding `colors_changed` (Theme Hook) in Subclasses

`colors_changed` is the per-widget theme hook. `Wheel` supplies a default
implementation that reassigns its six colour resources and the border from the
active theme. A widget that caches its own GC/pixmap, or has its own colour
resources, must override the hook.

**Rules:**

1. Declare the override in `@METHODS` **without a parameter list**:
   ```wbuild
   @proc void colors_changed        # override — no ($)
   ```
   wbuild fixes an inherited method's signature. Repeating it
   (`@proc void colors_changed($)`) makes `wbuild/generatec.c` call
   `warn_if_params` and exit non-zero, failing the generation of every subclass'
   header. The *first / owning* definition (e.g. in `Wheel.widget`) may keep `($)`.
   The body still uses `$` and `#colors_changed($)` because wbuild injects `self`
   from the inherited signature.
2. Call `#colors_changed($)` so `Wheel`'s base colours are reapplied. The order
   relative to your own dirty/cache flags is widget-specific: EqFader calls it
   first, Wlabel invalidates its cache first.
3. Re-derive your own colours, rebuild any cached GC/pixmap, and redraw if
   realized.

`EqFader.widget` (extra colours + a cached background GC):

```wbuild
@ override the Wheel theme hook: our backing GC caches the background pixel,
@ and our tick/label colours are EqFader resources (not Wheel ones), so derive
@ them from the theme foreground to stay readable on dark themes.
@proc void colors_changed
{
    #colors_changed($);

    if (theme_color_defined("tick_color"))
        $tickColor = get_color("tick_color");
    else if (theme_color_defined("fg_norm"))
        $tickColor = get_color("fg_norm");
    /* ... label/thumb colours ... */

    col_set($);

    if ($gc_bg) XtReleaseGC($, $gc_bg);
    /* rebuild gc_bg from $background_pixel */

    if (XtIsRealized($)) redraw($);
}
```

`Wlabel.widget` (only invalidates its pixmap cache):

```wbuild
@ override the Wheel theme hook: our pixmap cache must be rebuilt
@proc void colors_changed
{
    $dirty = 1;
    #colors_changed($);
}
```

## 20. Class-Level Variables (`@CLASSVARS`)

`@CLASSVARS` are shared by every instance, so they **must** have an `=`
initializer (see §2). They typically override Core/Expose class fields. From
`EqFader.widget`:

```wbuild
@CLASSVARS
@var compress_exposure = XtExposeCompressMultiple
@var compress_motion = True
@var visible_interest = False
```

Use a single value per `@var`: wbuild has no `OR` operator, and a bare `var` line
that directly follows a `@` doc comment is swallowed by that comment (see
`know_bugs.md`).

## 21. Tracing & Documentation Style (EqFader)

- Add `XTFUNC();` at the top of the lifecycle methods (`initialize`, `realize`,
  `resize`, `expose`, `destroy`) for entry tracing. It comes from `utils/xutil.h`
  (import with `@incl "xutil.h"`) and expands to
  `fprintf(stderr, "%s: %s\n", XtName(self), __func__)`.
- Document `@PUBLIC` resources, `@METHODS` overrides, `@ACTIONS` and
  `@UTILITIES` helpers with a preceding `@` comment line. The reference widgets
  `Frame.widget` and `Board.widget` show the intended density: every resource
  gets a short explanation, and non-obvious methods/actions explain their
  contract and side effects. wbuild turns these comments into
  `build/doc/<Widget>.tex` and `build/doc/<Widget>.3`.
- **Only the first line of a comment paragraph starts with `@`.** Continuation
  lines are plain prose. Repeating `@` on every line makes wbuild emit one
  `\Section` per line instead of a single paragraph (see `know_bugs.md`).
- Keep the `@class` doc comment to a single line — two or more consecutive
  `@`-prefixed lines start a second doc block and are a parse error (see
  `know_bugs.md`).
- Documentation is a side effect of the normal widget build: `make widgets`
  writes `build/doc/<Widget>.{tex,3}` and `build/doc/hdf/<Widget>.hdf` (§35).
  `tests/test_widget_docs.sh` regenerates a small set into a temporary
  directory and asserts that resource and method documentation appears in the
  text backends and in HDF.

## 22. Header Self-Containment & External Types

`@IMPORTS` is emitted only into `<Widget>.c`, never into `<Widget>.h` or
`<Widget>P.h`. Every type used by `@PUBLIC`, `@PRIVATE` or `@type` must therefore
be visible to a consumer that includes the generated header.

- **Prefer opaque `void *`** for external structs and cast at the point of use:
  `XftFont`, `XftDraw`, `XftColor`, `RetexParaFormat *`, icon/state caches.
  `Wbravo` stores `para_fmt`, `font`, `draw` and the colour table this way;
  `WListView` stores `font`, `draw`, `xcol` and its icon list the same way.
- **If a public resource genuinely needs a typedef**, pull its header into the
  public widget header with `@EXPORTS` + `@incl`:

  ```wbuild
  @EXPORTS
  @ ArrayInt is used by public resources, so the generated header must see it.
  @incl "converters.h"
  ```

  `WListView` needs this for `ArrayInt`; without it, any consumer of
  `<xtcw/WListViewP.h>` fails with `unknown type name 'ArrayInt'`.
- **`@type` structs are emitted into the private header.** Keep their fields to
  primitive/X11 types or `void *`; never embed an external struct by value. Use
  `void *svg` for an `XImage2` cache and cast internally.
- Safe in the private header without extra includes: `Pixmap`, `Pixel`,
  `Position`, `Dimension`, `Window`, `Widget`. Not safe unless included:
  `XftColor`, `XftFont`, `XftDraw`, `RetexParagraph`, `RAtom`, `ArrayInt`,
  `RetexParaFormat`.

Symptom to recognise: the generated `.c` compiles (its `@IMPORTS` come first),
but a test or another widget that includes `<Widget>P.h` fails with
`unknown type name`.

## 23. Composite Widgets With Managed Children

Use `@class W (Composite)` (or a layout manager such as `Gridbox`) when the
widget creates its own managed children. `Core`/`Wheel` cannot manage children.

### Creation and realize order

- Create children in `initialize`.
- In `realize`, call the superclass realize, then `relayout()`, and let Xt
  realize the managed children afterwards. Do **not** explicitly
  `XtRealizeWidget(child)` before its geometry is set.
- Xt aborts if a managed child is realized with 0×0 geometry:
  `Error: Widget hsb has zero width and/or height`. Even a hidden scrollbar must
  be configured with at least 1×1 and then unmapped with
  `XtSetMappedWhenManaged(child, False)`.

```wbuild
@proc realize
{
    #realize($, mask, attributes);
    rebuild_cache($);
    relayout($);          /* configures the managed children */
}
```

For a **Core** subclass whose superclass has no realize proc, create the window
with the mask/attributes that Xt computed — never mask 0 (this was the Wbravo
blank-window bug):

```wbuild
XtCreateWindow($, (unsigned int) InputOutput, (Visual *) CopyFromParent,
               *mask, attributes);
```

### Geometry

- Implement `geometry_manager`, `query_geometry` and `set_values_almost` (§6).
- Position internal children in a single `relayout()` called from `realize`,
  `resize`, `set_values` and scroll/slider callbacks. Use `XtConfigureWidget`.
- Set internal children's theme colours from the parent's theme apply function;
  they do not need their own `theme_register`.

### Sizing

`query_geometry` is a *preference*, not a guarantee. A widget that reports a
large preferred height can still be shrunk by its parent — all drawing must
work at any size (§24/§25). Give fixed chrome (`weighty 0`) and the scrolling
body (`weighty 1`) so a layout manager shrinks the right part (§31).

## 24. Backing Pixmap Caches Must Track Their Size

A common pattern (`Wbravo`, `WListView`, `Wlabel`) is: render into a pixmap,
then `XCopyArea` it to the window. The cache must be recreated whenever the
widget size changes.

**Wrong** — the condition is always true (`w`/`h` are just the clamped size):

```c
if (pixmap && width <= w && height <= h) { /* keep */ }
```

**Correct** — remember the cache dimensions:

```wbuild
@PRIVATE
@var Pixmap pixmap
@var int cache_w
@var int cache_h

@proc void rebuild_cache($)
{
    int w = (int)$width, h = (int)$height;
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    if (w > 4000) w = 4000;        /* X11 server limit used in this tree */
    if (h > 4000) h = 4000;

    if ($pixmap && $cache_w == w && $cache_h == h) return;
    if ($draw) { XftDrawDestroy((XftDraw*)$draw); $draw = NULL; }
    if ($pixmap) { XFreePixmap($dpy, $pixmap); $pixmap = 0; }
    $pixmap = XCreatePixmap($dpy, XtWindow($), w, h, depth);
    $cache_w = w; $cache_h = h;
    $draw = XftDrawCreate($dpy, $pixmap, vis, None);
}
```

Reset `cache_w`/`cache_h` in `destroy`. If the cache is stale, a slider (a real
child window) grows while the content stays at the old size.

### Cached drawables inside the cache

If a helper caches an `XImage2`/`XImage`/pixmap bound to the backing drawable,
re-point it before use. `WListView`'s SVG icon cache stores `XImage2.d`; after
the backing pixmap is recreated the next repaint must set `im->d = $pixmap`
before `xim2_put_image()`, otherwise X fails with
`BadDrawable ... X_PutImage`.

## 25. Fixed Header/Footer + Scrolling Body: Clip the Body

When a widget has fixed chrome and a scrolling content area, every content draw
must be clipped to `[body_top, body_bottom)`.

- Slider scrolling is **fractional**: `top_y` is not row-aligned, so the first
  row starts above the body top.
- Compute the visible band per row:

```c
int y0  = list_y + i * row_h - top_y;
int top = y0 < body_top ? body_top : y0;
int bot = y0 + row_h > body_bottom ? body_bottom : y0 + row_h;
if (top >= bot) continue;
XFillRectangle(dpy, pixmap, gc, 0, top, width, bot - top);
```

- Set the `XftDraw` clip to the same band for text; draw a row separator only if
  it falls inside the band.
- `XCopyArea` and `xim2_put_image` are **not** bounded by the `XftDraw` clip.
  Skip an icon whose rect is not fully inside the band (or clip it on its own
  GC; XPM's `XSetClipMask` and a clip region do not compose, so skipping is the
  simple correct choice).
- Use the theme header colour for the header fill, not the body background GC;
  otherwise the `*_header_bg` role is resolved but never used.

## 26. List-Style Data Models, Icons & Ownership

Existing list widgets use MLS string lists (`WlistMulti.tableStrs`,
`WListView.rows`) with one tab-separated string per row, plus an optional
`getEntry` callback (`wlist_entry_t` in `Wlist-common.h`) for large or virtual
lists:

```c
wlist_entry_t ent = { .pos = row, .max = 0, .state = 0, .data = "" };
XtCallCallbackList(self, self->getEntry, &ent);   /* app sets ent.max / ent.data */
```

- Document ownership: the widget **borrows** the caller's string list. Install
  the new list, then free the previous one; free the current list in `destroy`
  only if the widget allocated it.
- Column metadata can be parallel resources (`columnWidths`, `columnAligns`,
  `columnKinds` as `ArrayInt`; `columnTitles` as a tab-separated `String`) —
  see `WListView`.
- Icon cells: define a compact in-cell convention, e.g. `"@icons/file.svg|label"`
  (icon + text) or `"@icons/file.svg"` (icon only); plain text is drawn as-is.
  Cache loaded icons by filename. SVG uses `xim2_load_svg`/`xim2_render`; XPM
  uses `load_pixmap_from_file`.
- Recompute derived sizes (row height, weighted column widths, header height)
  from the font and resources whenever they change.

## 27. String Resources the Widget Mutates

§5 covers ordinary string resources. Two extra cases from `Wbravo`/`WfileChooser`:

- **The widget assigns a generated value to a String resource** (a loaded font
  face, the current directory, a selected path). Keep an owned copy and point
  the resource at it:

  ```wbuild
  @PRIVATE
  @var char *fontFace_mem

  @proc void set_face($, const char *face)
  {
      free($fontFace_mem);
      $fontFace_mem = strdup(face);
      $fontFace = $fontFace_mem;
  }
  ```

  Free `*_mem` in `destroy`. In `set_values`, when the resource may point at
  either the owned copy or an external string, compare/duplicate before freeing
  (see `WfileChooser.directory` / `selectedPath`).
- **Output resources** that must be valid after a callback (e.g.
  `selectedPath`, `WfileSelector.selectedPath`) should be documented as "valid
  until the next selection" if they borrow memory from a child.
- **Xt borrows every String resource.** This includes strings that LUI passes
  as Lua strings (`:fontFace "Sans"`, `:pattern "*.txt"`, ...). Lua may collect
  them after the chunk returns. Any String used after `initialize` (relayout,
  refresh, a later callback) must be copied in `initialize` and whenever
  `set_values` reports a new pointer. `Wbravo`/`WListView` own `fontFace`,
  `WfileChooser` owns `pattern`/`folderIcon`/`fileIcon`, and `WfileSelector` owns
  `directory`/`pattern`/icons. Symptom of getting this wrong: ASan
  `heap-use-after-free` in `strcmp`/`fnmatch` from a later relayout.
- **Freeing copies without overriding `destroy`**: `#destroy($)` expands to
  `<Parent>ClassRec.core_class.destroy(self)`, which can be `NULL` (for example
  `Wbutton`). If the parent's slot is not safe, register
  `XtAddCallback($, XtNdestroyCallback, free_cb, $)` in `initialize` instead of
  overriding `destroy`.

## 28. Expose a Command Interface as Xt Actions

Put application-facing commands in `@ACTIONS`. They are ordinary Xt action procs
and can be driven from other applications or the Lua bridge:

```c
XtCallActionProc(w, "set_text", NULL, params, nparams);
```

- Actions are `void`. Return data through resources (`text`, `selectedPath`,
  `selected`) or callbacks (`notify`, `activate`, `accept`).
- Parse `num_params`/`params[]` defensively; clamp numeric values; accept names
  or numbers where useful (see `Wbravo` and `WfileChooser`).
- Document the interface in one comment block above `@ACTIONS` (groups, argument
  meaning, clamping) so callers do not have to read the implementation.
- Good examples: `Wbravo` (document, selection, style, paragraph and snapshot
  commands) and `WListView` (`refresh`, `SetFocus`, navigation actions).

## 29. Undo/Redo and Full-State Snapshots

If the widget keeps side tables parallel to a document model (selection, caret,
per-paragraph formats, search highlights), snapshot them **together** with the
model snapshot and restore them in the same undo/redo step. `Wbravo` pairs
`rtable_snapshot()` with `caret_cp`/`anchor_cp` and the paragraph-format array.

For serialized snapshots (`save_state` / `load_state`):

- Include the complete widget state (document + formatting + side tables +
  resources + undo/redo), not just the text.
- Store interned handles (font faces, object payloads) as **strings** so the
  snapshot is portable across processes; intern them on load.
- Piece tables: the add buffer is append-only and undo does **not** truncate it.
  Redo snapshots can reference atoms past the current `add_len`; save the
  physical maximum range and restore `add_len` separately (`rtable_serialize`
  in `utils/rtable.c`).
- Reconcile side-table counts with the model's undo depth after load so a
  corrupt snapshot cannot leave the two out of sync.

## 30. Theme Integration for Non-`Wheel` Widgets

§18–§19 cover `Wheel` subclasses. A `Composite`/`Constraint` can join too:

```wbuild
@proc initialize { ... theme_register($, my_apply_theme); }
@proc destroy    { ... theme_unregister($); }

@proc void my_apply_theme($)
{
    $bg = theme_color_defined("list_bg") ? get_color("list_bg")
                                         : get_color("bg_norm");
    /* ... resolve every role you draw with ... */

    if ($colors_ready) free_my_xftcolors($);
    for (i = 0; i < NCOL; i++) xft_color_alloc($, $px[i], &$xcol[i]);
    $colors_ready = 1;
    if (XtIsRealized($)) redraw($);
}
```

- Keep a `colors_ready` flag so a zeroed `XftColor` is not freed on the first
  call.
- Use `theme_color_defined`/`get_color` with sensible fallbacks; document the
  fallback chain and add new role names to `themes.hdf`.
- Use every resolved role when drawing (header background, alternating rows,
  selection, grid line). A role that is resolved but never used is a bug.
- Internal children (sliders) get their `foreground`/`background` from the same
  apply function.

## 31. Gridbox: Fixed vs Weighted Cells

- Give fixed chrome (`header`, `footer`, toolbar) `weighty`/`weightx` **0** and
  the expandable body **1**. Growth is distributed to weighted cells.
- When the box is smaller than its preferred size, `Gridbox.layout()` now
  shrinks weighted cells first (`shrinkCells()` in `wbuild_widgets/gridbox_core.c`);
  fixed cells keep their preferred size. Only if weighted cells cannot absorb
  the shortfall does it shrink everything.
- A widget with a large `query_geometry` preferred size is still shrinkable by
  its parent. Do not rely on the preferred size being honoured; render correctly
  at any size (§24/§25).
- `defaultDistance` is both the outer margin and the inter-cell gap; the
  minimum cell size is `defaultDistance * 2 + 1`.

## 32. LUI Integration for a New Widget

- `make widgets` auto-registers the class; add a friendly alias in
  `lui/registry.lua`:

  ```lua
  M.register('listview', { class = 'WListView' })
  M.register('file-chooser', { class = 'WfileChooser' })
  ```

- Resource aliases and Lua-friendly names go in `lui/backend_xt.lua`
  (`model` → `rows` for `WListView` and `tableStrs` for older lists,
  `columns` → `columnWidths`, `on-row-activated` → `notify`,
  `on-activate` → `activate`, `on-header` → `headerClicked`).
- LUI tables cannot be passed directly to an `int`/MLS resource. Use the
  `model` mapping (it builds the MLS string list) or
  `gui.set_string_list(id, "rows", {...})` from `lui/gui_xt.lua`.
- `LUA(fn)` callbacks do **not** receive the widget's `call_data`; read results
  from resources inside the callback (e.g. `gui.get(id, "selectedPath")`).
- LUI table literals use `key = value` pairs: `{ 1 = "row1" 2 = "row2" }`.
  For arrays, `gui.set_string_list` after `lui.run` is usually clearer.

## 33. Testing & Debugging Widget Rendering

- **Resize**: assert the backing cache dimensions and layout metrics after
  `XtConfigureWidget`/`XtResizeWidget`, not merely that the widget exists.
- **Fractional scroll**: set `top_y` to a non-multiple of the row height (or
  drive the slider), then read a pixel from the backing pixmap with
  `XGetImage`/`XGetPixel` and assert the fixed header colour is intact. This
  catches "rows paint over the header" deterministically.
- **Valgrind**: run the widget test under
  `valgrind --error-exitcode=9 --num-callers=20`. Real double frees (e.g.
  duplicate callback resources) show up; the debug-only MLS post-mortem messages
  at exit are usually false positives (see `know_bugs.md`).
- **Headless**: use `xvfb-run -a` as `tests/run_all_tests.sh` does. Mapping and
  geometry negotiation can differ on a live display (e.g. `Wviewport`).
- **Screenshots**: capture the initial frame (before any resize) and after a
  scrollbar drag to catch first-layout and clipping regressions.
- Keep widget tests self-checking (exit non-zero on failure) and add them to
  `tests/Makefile` and `tests/run_all_tests.sh`.

## 34. Resource Declaration Pitfalls

- **Never redeclare an inherited resource.** `Wcheckbox` redeclared
  `@var <Callback> XtCallbackList callback = NULL` although `Wheel` already
  provides `callback`; wbuild emitted a second resource at the same offset and
  Xt freed the callback list twice at destroy. Check the superclass chain
  (`Wlabel` → `Wheel` → `Core`) before declaring a resource.
- `@def` requires ` = `: `@def COLT_TEXT = 0`; `@def COLT_TEXT 0` is a parse
  error.
- `@type`/`@def` are emitted into the private header; keep them self-contained
  (§22).
- `@var` arrays (`@var Pixel col_px[8]`) are fine if the element type is visible
  in the header; otherwise use `void *` and allocate at runtime.
- Overrides omit the parameter list (§19); only the first/owning definition keeps
  `($)`.

## 35. HDF Documentation Output (help browser)

`make widgets` writes structured documentation to `build/doc/hdf/<Widget>.hdf`
in addition to the `.tex`/`.3` backends. This is the **Human Data Forms**
language (`~/git/hdf`): nested lists, `seq` arrays, raw strings and `rem`
comments. It is unrelated to the flat theme format in `themes.hdf`.

The root is an object with `kind widget`; every node is an object with a
`kind` field (HDF has no tagged-object syntax):

```lisp
(
  kind widget
  schema_version 1
  anchor widget/EqFader
  class EqFader
  superclass Wheel
  superclass_chain (seq (class Wheel anchor widget/Wheel) ...)
  description [[...]]
  resources (seq
    (kind resource
      anchor resource/EqFader/XtNlabel
      name XtNlabel
      type [[String]]
      default ""
      description [[frequency label shown below the fader]]
      owner EqFader
      owner_link widget/EqFader
      inherited false
      constraint false))
  methods (seq (kind method anchor method/EqFader/realize ...))
)
```

Anchor conventions:

| Node | Anchor |
|------|--------|
| Widget | `widget/<Class>` |
| Resource | `resource/<Owner>/<XtNname>` |
| Method | `method/<Class>/<name>` |
| Action | `action/<Class>/<name>` |
| Utility | `utility/<Class>/<name>` |

- `description_links` records inline `|symbol|` spans as byte offsets
  (`start`, exclusive `end`, `label`, `target`, `kind`). The help browser
  rewrites them to Markdown links; unresolved symbols stay plain text.
- Inherited resources are emitted with `owner`/`owner_link` and
  `inherited true`; the browser can show the full superclass chain.
- The HDF backend is additive: `.tex` and `.3` stay unchanged. `wbuild` only
  rewrites an HDF file when the `.widget` source is newer, so delete the
  directory (or `touch` the widget) when testing emitter changes.
- HDF raw strings that end with `]` need at least one `=` in the delimiter
  (`[=[...]=]`); otherwise `]]]` is ambiguous. `hdf_raw()` handles this.

`tools/hdf.lua` is the small core-HDF reader used by `tools/widget_help.lua`.
`Wmarkdown` records `[label](target)` spans and exposes them through the
`linkActivated` callback plus the `lastLink` resource (the generic LUA callback
bridge does not forward Xt `call_data`). `Tab`/`Shift+Tab` cycle links,
`Return` activates the focused link, and the focused link is underlined.
