# X Toolkit Intrinsics (Xt) Geometry Management

> See also: `wbuild_widgets/CONVENTIONS.md` (authoritative coding rules), `libxt-1.2.1/specs/CH06.xml` (Xt spec), and *X Toolkit Intrinsics Programming Manual* (O'Reilly, 2nd Ed., R4) Chapters 6 and 11.

## Overview

Xt provides a geometry negotiation protocol between parent and child widgets. A widget does not directly control its size — its parent does. The protocol lets the child express preferences while the parent has final authority.

---

## Is `query_geometry` Required?

**Technically no, but practically yes for all display widgets.**

Per the Xt spec (CH06 §6.5): if `query_geometry` is NULL, `XtQueryGeometry` returns `XtGeometryYes` and fills in the widget's current geometry as the "preferred" reply.

However, the O'Reilly X Toolkit Intrinsics Programming Manual (Ch 6.6) strongly warns against omitting it: "If your widget specifies null in the class structure for the query_geometry method, the parent will be told that your widget's current geometry is its preferred geometry. This is often wrong information... Even if your widget has no particular preference for size, it is a good idea to specify the widget's default size in query_geometry. Then, at least, the parent has a ballpark figure for typical sizes for your widget."

**This explains why the "simple widgets that were working" still need it**: they work at their initial size, but when the application is resized down to 1x1 and then back up, the parent has lost all knowledge of the widget's preferred size — it just gets back "1x1 is my preference."

---

## Initial Geometry Negotiation (from O'Reilly Ch 11.1.1)

When `XtRealizeWidget` is called on the top-level widget, geometry negotiation ripples through the hierarchy before any windows are created:

```
1. XtRealizeWidget(toplevel)
2. change_managed called on every composite widget, BOTTOM-UP (post-order)
   → Each parent determines initial size for each child via XtQueryGeometry or child's current width/height
   → Each parent calls XtResizeWidget/XtMoveWidget for children
3. Shell widget reached: sets its size to child's size
4. If user specified geometry (command line/resources) different from shell child's size:
   → Shell resizes child to user-specified size
   → resize methods called TOP-DOWN
   → Each parent can query children via XtQueryGeometry again
5. Only now: realize methods called, windows created
```

**Key insight**: If a widget's `query_geometry` returns its current (resize-crushed) 1x1 size as preferred, the parent will never restore it to a reasonable size when the window grows again.

---

## Geometry Negotiation Protocol

### Step 1: Parent Queries Child's Preferred Size

Parent calls `XtQueryGeometry()`, which invokes the child's `query_geometry` method:

```c
XtGeometryResult query_geometry(Widget self,
    XtWidgetGeometry *request,   // What parent plans to give (fields valid per request_mode)
    XtWidgetGeometry *reply);    // What child prefers (output)
```

**Request inspection:**
- `request->request_mode` indicates which fields the parent cares about
- `CWWidth` = parent specified a target width
- If `request->width == 0`, this is an initial query (no width constraint)

**Child must:**
- Set `reply->request_mode` to indicate which fields it cares about (typically `CWWidth | CWHeight`)
- Set `reply->width`, `reply->height` to preferred values
- Return one of three values (see below)

### Return Values — Precise Semantics (from Xt spec)

| Return | Meaning |
|--------|---------|
| **`XtGeometryYes`** | Child accepts the proposed change without modification. Parent need not modify its layout plans. |
| **`XtGeometryAlmost`** | Child's preference DIFFERS from parent's intention in at least one field both care about, OR child expressed interest in a field parent didn't ask about. **Most common return** for content-aware widgets. |
| **`XtGeometryNo`** | Child's preferred geometry is IDENTICAL to its current geometry AND parent and child expressed interest in overlapping fields. Child suggests the current value is its preferred value. |

**If `query_geometry` is NULL:** `XtQueryGeometry` returns `XtGeometryYes` and uses current geometry as preferred. The `preferred_return` structure is fully populated with current values regardless.

### Step 2: Parent Computes Actual Size

Parent (e.g., Gridbox) decides size based on:
- Child's preferred size (from `query_geometry`)
- Available space in container
- Child's `weightx`/`weighty` (distribution of extra space)
- Child's `fill` mode (None=0, X=1, Y=2, Both=3)

### Step 3: Parent Proposes Compromise (if needed)

If parent can't give child what it asked for, `geometry_manager` returns `XtGeometryAlmost` with compromise in reply:

```c
reply->width = compromise_width;
reply->height = compromise_height;
reply->request_mode = CWWidth | CWHeight;
return XtGeometryAlmost;
```

### Step 4: Child Accepts/Adjusts Compromise

Xt calls child's `set_values_almost`:

```c
void set_values_almost(Widget old, Widget new,
    XtWidgetGeometry *request,   // What child originally requested
    XtWidgetGeometry *reply);    // Parent's compromise (input/output)
```

**The child MUST modify `request` to accept the compromise:**

```wbuild
@proc set_values_almost
{
    request->width = reply->width;
    request->height = reply->height;
    request->request_mode = CWWidth | CWHeight;
    $dirty = 1;    /* force re-layout with new size */
}
```

### Step 5: Parent Applies Size

Parent calls `XtResizeWidget()` on each child, which invokes child's `resize()` method to notify the child of new dimensions.

---

## Reference Implementations

### Wretex.widget — Complete Geometry Implementation

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

@proc set_values_almost
{
    request->width = reply->width;
    request->height = reply->height;
    request->request_mode = CWWidth | CWHeight;
    $dirty = 1;
}

@proc resize
{
    if ($backend_ptr) {
        Backend *be = (Backend*)$backend_ptr;
        be->destroy(be);
        $backend_ptr = NULL;
        $pixmap = 0;
    }
    $dirty = 1;
    redraw_wretex($);
}

@proc expose
{
    redraw_wretex($);
}
```

### VBox.widget — Composite with geometry_manager

```wbuild
@proc geometry_manager
{
    /* wbuild auto-inserts: Widget $ = XtParent(child); */
    Dimension wd = request->request_mode & CWWidth  ? request->width  : $child$width;
    Dimension ht = request->request_mode & CWHeight ? request->height : $child$height;
    XtConfigureWidget(child, x, y, wd, ht, 0);
    return XtGeometryDone;
}
```

---

## Resize Event Chain

```
User resizes window
    │
    ▼
Shell receives X ConfigureNotify event
    │
    ▼
Xt calls Shell's resize proc
    │
    ▼
Shell calls Composite's resize (or layout manager)
    │
    ▼
Gridbox.resize()  ─── recalculates column/row sizes
    │
    ▼
For each managed child: XtResizeWidget(child, ...) → child->resize()
    │
    ▼
Wlabel.resize()  ─── destroys old backend, sets dirty=1
    │
    ▼
Next expose triggers redraw_label() → update_cache()
    │
    ▼
retex_layout() called with NEW width → text re-wrapped
```

---

## Conventions Verification Checklist

For each widget, verify against this table. See `wbuild_widgets/CONVENTIONS.md` for full rules.

| Check | Applies to | Method |
|-------|-----------|--------|
| `query_geometry` implemented | Content-dependent or layout-manager children | `@proc query_geometry` |
| `query_geometry` checks `request_mode & CWWidth` | Widgets with qg | `int tw = (request->request_mode & CWWidth) ? request->width : 0` |
| `resize` implemented | All non-trivial display widgets | `@proc resize` |
| `resize` sets `$dirty = 1` | Display widgets | `$dirty = 1;` |
| `expose` implemented | All display widgets | `@proc expose` |
| `set_values_almost` implemented | Widgets with qg that may get compromises | `@proc set_values_almost` |
| `geometry_manager` implemented | Composite subclasses | `@proc geometry_manager` |
| Constraint superclass correct | Constraint widgets | `@class Widget (Constraint)` |
| `allowShellResize = True` | TopLevel/Shell windows | In `@PUBLIC` or at creation |
| Intermediate math uses `int` | Geometry calculations | `int w = (int)cell - (int)margin` |
| Subpixel uses `ceil()` | Text/font calculations | `(int)ceil(subpixel_value)` |
| Weight/fill set on children | Gridbox/VBox/HBox children | `:weightx 1 :weighty 1 :fill 3` |

---

## Common Geometry Issues

### 1. Widget doesn't re-layout on resize
**Symptom:** Text stays wrapped at old width after container grows.
**Cause:** `resize()` didn't destroy cached layout data.
**Fix:** Set `$dirty = 1` in resize and destroy size-dependent caches (backend, pixmap).

### 2. `set_values_almost` not implemented
**Symptom:** Widget gets wrong size from parent, text clipped or extra space wasted.
**Cause:** Inherited `set_values_almost` doesn't handle widget-specific dirty flag.
**Fix:** Implement `set_values_almost` that accepts compromise and sets `$dirty = 1`.

### 3. `query_geometry` returns wrong preferred size
**Symptom:** Container gives widget too much or too little space.
**Cause:** Using cached width instead of `request->width`.
**Fix:** Check `request->request_mode & CWWidth`; use `request->width` if provided.

### 4. Unsigned underflow (e.g., 65534px)
**Symptom:** Widget renders massively oversized, off-screen.
**Cause:** `Dimension` (unsigned) math: `cell_size - margin` wraps to 65534.
**Fix:** Use signed `int` intermediates, clamp to minimum of 1.

### 5. Constraint resources silently dropped
**Symptom:** All Gridbox children stack at (0,0).
**Cause:** `@class Widget (Composite)` instead of `@class Widget (Constraint)`.
**Fix:** Use correct superclass. Never link a wbuild stub that declares `compositeClassRec`.

### 6. Text truncated by int cast
**Symptom:** Last line of text clipped.
**Cause:** `int pixels = (int)subpixel_value` truncates 23.9 to 23.
**Fix:** Use `(int)ceil(subpixel_value)`.

---

## Gridbox-Specific Behavior

Gridbox queries each child's preferred size via `query_geometry()`, then:
1. Computes column/row sizes based on max preferred sizes + weights
2. Distributes extra space based on weights (`weightx`/`weighty`)
3. Calls each child's `resize()` with calculated cell size
4. Child's `set_values_almost` is called only if Gridbox offers different size than child requested

### Child Constraint Resources

| Resource | Type | Default | Purpose |
|----------|------|---------|---------|
| `gridx` / `gridy` | Position | 0 | Cell position |
| `gridWidth` / `gridHeight` | Dimension | 1 | Cell span |
| `weightx` / `weighty` | int | 0 | Extra space distribution ratio |
| `fill` | FillType | FillBoth | 0=None, 1=X, 2=Y, 3=Both |
| `gravity` | int | CenterGravity | Position when smaller than cell |
| `margin` | int | 4 | Cell padding |

Without `weightx` > 0, children get minimum preferred size and extra space is unused — widgets appear tiny or at wrong positions.
