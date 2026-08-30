# wbuild/Xt Widget Coding Conventions

> The single source of truth for writing `.widget` files. Follow these rules and your widget will compile, render, and interact correctly with Xt geometry management.

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
- [ ] Registered in `register-widgets.sh`
- [ ] Added to LUI registry in `lui/registry.lua`

## 17. Known Bug Patterns (from errors.md)

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
