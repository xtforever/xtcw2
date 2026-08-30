# Geometry Management Fix Plan — Wlabel + Gridbox + LUI

> **Status**: Build fixed (stale .c overwrite removed). Symbol collision `update_cache` between
> Wlabel.c and Wretex.c needs renaming. Core geometry bugs remain.

---

## 1. Target: A Working Wlabel-in-Gridbox-in-LUI Demo

```
LUI S-expression  →  Xt widget tree  →  retex layout  →  pixmap render  →  screen
     ▲                  ▲                   ▲                ▲               ▲
  parser.lua       backend_xt.lua       Wlabel.c         backend_      XCopyArea
                                        calculate_size    xpixmap.c
```

**Success**: A Lua script creates a window with a Gridbox containing a Wlabel. The label renders text via retex. Resizing the window re-layouts correctly. Changing the label text triggers auto-resize. Selection highlights visually.

---

## 2. Knowledge Foundation

### 2.1 What we already verified works

| Component | Status | Evidence |
|-----------|--------|----------|
| retex → pixmap → screen | ✅ Proof exists | `demo_wlabel_svg.lua` renders via commander_runner |
| retex selection (hit-test + visual) | ✅ Proved | `select_start/select_extend` + `renderer_render_backgrounds` |
| `$width`/`$height` expansion by wbuild | ✅ Verified | Fresh generation into build/source/ expands correctly |
| Gridbox weight distribution | ✅ Proved | `tests/test_gridbox.c` compiles, weights tested |
| LUI test harness (click/expect/verify) | ✅ Working | `lui_test/run_widget_tests.sh` 8/8 pass |
| Commander runner (Lua→Xt bridge) | ✅ Working | Creates shell, loads widgets, dispatches callbacks |

### 2.2 Key reference documents (in this repo)

| Document | Key content |
|----------|-------------|
| `learn.md` | Gridbox API, TRACE mechanism, wbuild syntax, known pitfalls |
| `errors.md` | 12 fixed bugs with root causes — prevention patterns |
| `ultimate_plan.md` | Widget verification checklist, testing gaps |
| `overview.md` | Build pipeline, selection system, wbuild `$` expansion rules |
| `howto.md` | LUI S-expression format, `gui.set`/`gui.get` |
| `AGENTS.md` | GitNexus impact analysis workflow (run before editing any symbol) |

### 2.3 Key reference code (in this repo)

| File | What to study |
|------|---------------|
| `plainc_widgets/Gridbox.c:496-618` | Correct geometry_manager pattern (query-only guard, parent negotiation) |
| `plainc_widgets/Gridbox.c:430-453` | Correct query_geometry (returns Yes when sizes match, Almost otherwise) |
| `plainc_widgets/Gridbox.c:1120-1170` | changeGeometry — asks parent, handles XtGeometryAlmost from Athena bug |
| `libxt-1.2.1/src/Shell.c:1905-1954` | allowShellResize gate — the single-point block |
| `libxt-1.2.1/src/Geometry.c` | XtMakeGeometryRequest, XtMakeResizeRequest implementation |
| `experimental/commander_runner.c:32` | allowShellResize=True in fallback resources |
| `experimental/commander_runner.c:190-272` | Shell creation + widget loading order |
| `lui/registry.lua` | 22 registered LUI widget tags |
| `lui/gui_xt.lua` | `gui.get_widget()`, `gui.get()`, `gui.set()` |
| `lui/test_harness.lua` | `click()`, `toggle()`, `action()`, `expect()`, `sequence()` |
| `wbuild_widgets/Wlabel.widget:252-266` | query_geometry (buggy — always returns Almost) |
| `wbuild_widgets/Wlabel.widget:268-279` | resize (destroys pixmap unconditionally) |
| `wbuild_widgets/Wlabel.widget:236-242` | autoHeight (uses XtVaSetValues instead of XtMakeResizeRequest) |
| `wbuild_widgets/Wlabel.widget:410-433` | calculate_size (creates real pixmap for measurement) |

---

## 3. Phase 0: Fix the Build (Completed)

- [x] **0.1** Remove stale `.c` file overwrite in makefile:66-76 — DONE
- [x] **0.2** Verify fresh wbuild generation produces correct code — VERIFIED
- [x] **0.3** `make clean && make all` succeeds through libxtcw — VERIFIED
- [ ] **0.4** Rename `update_cache` in Wretex.widget to avoid collision with Wlabel — BLOCKED tests
- [ ] **0.5** Fix `conststr_init`/`conststr_free` implicit declaration in test_retex_math.c

> **UTURN-0**: If wbuild generates broken code again after edits to `.widget` files,
> fall back to editing the generated `.c` directly in `build/source/` and pinning it.
> The stale-overwrite fix means build/source/ files are now the authority.

---

## Phase A: Fix Wlabel Geometry (The Leaf Widget)

### A.1 Fix `calculate_size` (measurement without pixmap)

**Current**: Creates `backend_xpixmap_create(dpy, root, 10, 10)` → full X11 pixmap → measures text → destroys. 5+ X11 round trips per call. Called from `initialize`, `query_geometry`, `set_values` (autoHeight).

**Goal**: Create a "measurement-only" backend that initializes fonts but uses an in-memory surface.

**Sub-points**:
- [ ] **A.1.1** Add `backend_measure_create(Display*, double dpi)` in `re-tex/src/`
  - Uses `cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1)` — no X11 round-trip
  - Initializes Xft fonts for the given DPI (needed for `get_char_width`)
  - Does NOT create a Pixmap
  - Stores DPI from `get_dpi()` cache
- [ ] **A.1.2** Cache DPI in Wlabel private state (`$wlabel_dpi`)
  - Read once in `initialize` or first `calculate_size`
  - Avoids `XGetDefault(dpy, "Xft", "dpi")` on every measurement
- [ ] **A.1.3** Modify `calculate_size` to use measurement backend
  - If `target_width == 0`, use a large virtual width (20000pt) for natural layout
  - Cap returned values at 4000×4000 (X11 backend limit)
- [ ] **A.1.4** Add `$prefered_width` / `$prefered_height` as cached values
  - Set in `initialize` after first measurement
  - `query_geometry` returns these cached values if label/font hasn't changed

> **UTURN-A1**: If measurement backend introduces font initialization bugs, fall back to
> Xft-only measurement using `XftTextExtentsUtf8` directly (skip retex for quick size queries).

### A.2 Fix `query_geometry` (correct Xt protocol)

**Current** (Wlabel.widget:252-266):
```c
reply->request_mode = CWWidth | CWHeight;
reply->width = w; reply->height = h;
return XtGeometryAlmost;  // ALWAYS — even if sizes match
```

**Goal**: Return the correct `XtGeometryResult` per Xt spec.

**Sub-points**:
- [ ] **A.2.1** Compare request sizes to preferred sizes
  - If `(request->request_mode & CWWidth) && request->width == w` AND
    `(request->request_mode & CWHeight) && request->height == h` → `XtGeometryYes`
  - Otherwise → propose preferred sizes and return `XtGeometryAlmost`
- [ ] **A.2.2** Handle the three request-mode cases
  - Only width requested → only check/propose width, accept existing height
  - Only height requested → only check/propose height, accept existing width
  - Both requested → check/propose both
- [ ] **A.2.3** Cap proposed dimensions at 4000×4000
  - Backend_xpixmap_create rejects >4000
  - Must not propose sizes that can't be rendered
- [ ] **A.2.4** Add TRACE(2, "LAYOUT ...") output
  - Log: requested size, computed preferred size, returned result
  - Format: `"LAYOUT Wlabel %s: query req=%dx%d pref=%dx%d → %s"`
  - This enables automated geometry verification

> **UTURN-A2**: If Gridbox (or other parents) misinterpret `XtGeometryYes`, 
> check their query_geometry handling. Some parents (Athena Box) modify widget 
> dimensions during query-only mode. Gridbox has a workaround for this (line 1142).

### A.3 Fix `resize` (don't destroy pixmap unnecessarily)

**Current** (Wlabel.widget:268-279):
```c
// Always destroys backend, always recreates
if ($backend_ptr) { be->destroy(be); $backend_ptr = NULL; $pixmap = 0; }
```

**Goal**: Only recreate pixmap when dimensions actually changed.

**Sub-points**:
- [ ] **A.3.1** Track current pixmap dimensions
  - Add `$pixmap_width` and `$pixmap_height` private vars
  - Set when backend is created in `update_cache`
- [ ] **A.3.2** Compare new widget size to pixmap size
  - If `$width == $pixmap_width && $height == $pixmap_height` → skip destroy
  - If dimensions differ OR pixmap is capped (4000 limit) → destroy + set dirty
- [ ] **A.3.3** When skipping destroy, still mark dirty if needed
  - If `$dirty` is already set → call `redraw_label` to re-render
  - If not dirty → nothing to do (layout didn't change)
- [ ] **A.3.4** Add TRACE(2, "LAYOUT Wlabel resize %s: %dx%d → %dx%d, recreate=%d")

> **UTURN-A3**: If tracking pixmap dimensions causes stale-render bugs (pixmap
> content doesn't match new font/label), always destroy on content change but
> keep on size-unchanged resize.

### A.4 Fix `autoHeight` (proper geometry request)

**Current** (Wlabel.widget:236-242):
```c
if (size_changed && $autoHeight) {
    calculate_size($, 0, &w, &h);
    if (w != $width || h != $height) {
        XtVaSetValues($, XtNwidth, w, XtNheight, h, NULL);
    }
}
```

**Problem**: `XtVaSetValues` triggers `set_values` again (re-entrant!) and doesn't go through the geometry negotiation chain. The parent doesn't get a say.

**Goal**: Use `XtMakeResizeRequest` to properly negotiate with parent.

**Sub-points**:
- [ ] **A.4.1** Replace `XtVaSetValues` with `XtMakeResizeRequest`
  ```c
  XtWidgetGeometry request = {0};
  request.request_mode = CWWidth | CWHeight;
  request.width = (Dimension)w;
  request.height = (Dimension)h;
  XtMakeResizeRequest($, &request, NULL);
  ```
- [ ] **A.4.2** Handle `XtGeometryNo` from parent
  - If parent refuses → widget keeps current size, text may be clipped
  - Set `$dirty = 1` for re-render at current size
  - Log TRACE(2, "LAYOUT Wlabel %s: autoHeight resize denied")
- [ ] **A.4.3** Add re-entrancy guard
  - `set_values` is called from `XtVaSetValues` currently → removes the re-entrancy risk
  - After switching to `XtMakeResizeRequest`, the parent's `geometry_manager` may call `XtConfigureWidget` which doesn't re-enter `set_values`
  - Still, add a guard: `if ($in_set_values) return do_expose;`

> **UTURN-A4**: If `XtMakeResizeRequest` causes infinite loops (child→parent→child),
> use a flag `$requesting_resize` to break cycles. Gridbox uses this pattern.

### A.5 Remaining Wlabel fixes

- [ ] **A.5.1** Rename `update_cache` in Wretex.widget to avoid collision
  - Wretex is a separate widget class with its own `update_cache` EXPORT
  - Wlabel also has `update_cache` → linker error when both in libxtcw.a
  - Rename Wretex's to `wretex_update_cache` or similar
- [ ] **A.5.2** Fix `conststr_init`/`conststr_free` implicit declarations in test_retex_math.c
  - Add `#include "conststr.h"` to the test file

---

## Phase B: Verify Wlabel + Gridbox Tandem

### B.1 Understanding the interaction protocol

```
Wlabel.set_values (label changed)
  → autoHeight check
  → XtMakeResizeRequest(wlabel, {width:new, height:new})
    → Gridbox.geometry_manager(child=wlabel, request)
      → Update wlabel's cached prefWidth/prefHeight
      → recomputeWidHgtMax() — recalculate all rows/columns
      → changeGeometry() → XtMakeGeometryRequest(gridbox, ...)
        → Shell.GeometryManager(gridbox, request)
          → if allowShellResize: forward to WM, wait for ConfigureNotify
          → if !allowShellResize: XtGeometryNo (if realized)
      → layout() with granted dimensions
      → layoutChild(wlabel) — compute wlabel's new cell size
      → XtConfigureWidget(wlabel, x, y, cell_w, cell_h, border)
      → return XtGeometryDone/Almost/No
```

### B.2 Gridbox prerequisites

- [ ] **B.2.1** Verify Gridbox `query_geometry` is correct
  - Already returns `XtGeometryAlmost` when preferred != current → ✓
  - Already returns `XtGeometryNo` when sizes match → ✓ (line 448-450)
  - Verify this works with Wlabel as child
- [ ] **B.2.2** Verify Gridbox `geometry_manager` handles Wlabel requests
  - Wlabel only requests width/height (never position/border) → Gridbox accepts ✓
  - Wlabel's request becomes its new `prefWidth`/`prefHeight` → Gridbox uses this
  - Gridbox's `changeGeometry` must correctly query Shell → depends on allowShellResize
- [ ] **B.2.3** Check Gridbox's `getPreferredSizes` calls Wlabel's `query_geometry`
  - Gridbox calls `XtQueryGeometry(wlabel, ...)` for each managed child
  - Wlabel's `query_geometry` calls `calculate_size` which creates temp pixmap → needs Phase A.1 fix
  - If Wlabel returns `XtGeometryAlmost` with preferred size → Gridbox uses it as `pref`

### B.3 Test scenarios

- [ ] **B.3.1** Static layout: Gridbox + Wlabel, fixed sizes
  - Gridbox at 400×300, Wlabel at gridx=0,gridy=0, fill="both"
  - Verify Wlabel fills cell, text renders centered
  - Verify via TRACE(2, "LAYOUT ...") output
- [ ] **B.3.2** Weight distribution: Two Wlabels with different weights
  - label1: weightx=1, label2: weightx=2
  - Verify label2 gets 2× the width of label1
  - Verify via `xtgeometry()` measurements
- [ ] **B.3.3** Window resize: Shell resize → Gridbox re-layout → Wlabel re-render
  - allowShellResize=True, user resizes window
  - Gridbox receives new size, calls layout(), Wlabel gets resize()
  - Verify Wlabel's pixmap is recreated at new size (Phase A.3)
  - Verify text re-renders correctly (no truncation, no garbage)
- [ ] **B.3.4** Label change with autoHeight: label text grows → Wlabel requests resize → Gridbox accommodates
  - Initial: label="Hello" → small widget
  - Change: label="Hello World This Is A Longer Text" → needs more width
  - Gridbox receives XtMakeResizeRequest → recomputes layout
  - Shell (allowShellResize=True) resizes window
  - Verify final widget sizes are correct

### B.4 Gridbox edge cases to test

- [ ] **B.4.1** Gridbox with `queryOnly` flag → must not modify widget dimensions
  - Call `XtQueryGeometry` on Gridbox → children must not be reconfigured
  - Verify Gridbox restores old child prefs (line 586-594)
- [ ] **B.4.2** Gridbox receives `XtGeometryAlmost` from parent → must accept compromise
  - Parent can't give full requested size → Gridbox re-layouts with compromise
  - Children get proportionally reduced space
  - Wlabel must render at reduced size (text wraps or clips)
- [ ] **B.4.3** Gridbox with unmanaged children → skips them
  - `XtUnmanageChild(wlabel)` → Gridbox ignores it in layout
  - `XtManageChild(wlabel)` → Gridbox includes it, calls changeGeometry

> **UTURN-B**: If Gridbox is too complex (non-deterministic layout, 1000+ lines),
> switch to HBox/VBox after fixing their geometry_manager. The key missing piece
> in HBox/VBox is parent negotiation — adding a `changeGeometry` call is ~20 lines.
> But Gridbox is ALREADY PROVEN working by `tests/test_gridbox.c` — prefer it.

---

## Phase C: Fix HBox/VBox Geometry (Backup Containers)

### C.1 Current broken behavior

**Current** (HBox.widget:93-111, VBox.widget:94-112):
```c
geometry_manager: {
    XtConfigureWidget(child, x, y, wd, ht, bw);
    return XtGeometryDone;  // Always — never negotiates with parent
}
```

**Problems**:
1. Ignores `XtCWQueryOnly` — reconfigures children even on query
2. Never calls parent to grow the container
3. Always returns `XtGeometryDone` — even if child doesn't fit

### C.2 Fix plan (only if Gridbox path fails)

- [ ] **C.2.1** Add query-only guard: `if (request->request_mode & XtCWQueryOnly) return XtGeometryYes;`
- [ ] **C.2.2** Compute if container needs to grow to fit all children
- [ ] **C.2.3** Call parent's `geometry_manager` via `XtMakeGeometryRequest`
- [ ] **C.2.4** Only `XtConfigureWidget` children after parent grants space
- [ ] **C.2.5** Return appropriate result: Done/Yes/Almost/No

---

## Phase D: Tiny LUI Wrapper for Geometry Testing

### D.1 Minimal test script

```lua
-- test_wlabel_gridbox.lua
local lui = require("lui")
local gui = require("gui_xt")

local ui = [[
(window :id "win" :title "Geometry Test" :width 400 :height 200
  (grid :id "grid" :width 400 :height 200
    (label :id "lbl1" :label "Hello World"
           :gridx 0 :gridy 0 :gridWidth 1 :gridHeight 1
           :weightx 1 :weighty 1 :fill "both"
           :font-size 18 :auto-height true
           :left-gap 10 :right-gap 10 :top-gap 5 :bottom-gap 5)
  ))
]]

lui.run(ui)

-- Step 1: Verify widget creation
local w = gui.get_widget("lbl1")
assert(w ~= nil, "Wlabel not created")

-- Step 2: Get initial geometry
local geom = gui.get_geometry("lbl1")
print(string.format("INITIAL: width=%d height=%d x=%d y=%d",
    geom.width, geom.height, geom.x, geom.y))
assert(geom.width > 0, "Wlabel width is zero")
assert(geom.height > 0, "Wlabel height is zero")

-- Step 3: Get label text property
local label = gui.get("lbl1", "label")
assert(label == "Hello World", "Label mismatch: " .. tostring(label))

-- Step 4: Change label (should trigger autoHeight resize)
gui.set("lbl1", "label", "A much longer label text that needs more space")
-- Wait for geometry to settle
gui.wait(200)

-- Step 5: Verify new geometry (should be wider)
local geom2 = gui.get_geometry("lbl1")
print(string.format("AFTER: width=%d height=%d", geom2.width, geom2.height))
assert(geom2.width >= geom.width, "Label didn't grow after text change")

gui.exit(0)
```

### D.2 Required C bindings (add to experimental/xt_bridge.c)

- [ ] **D.2.1** `xtgeometry(widget_id)` → returns `{width, height, x, y}`
  - Calls `XtVaGetValues(w, XtNwidth, &w, XtNheight, &h, NULL)`
  - Returns Lua table with 4 integer fields
  - On Lua side: `gui.get_geometry(id)` wrapper
- [ ] **D.2.2** `xtwait(ms)` → pauses test for geometry to settle
  - Uses `xtapptimeout(ms, callback)` to schedule continuation
  - Wraps in `gui.wait(ms)` for test scripts
- [ ] **D.2.3** `xtscreenshot(widget_id, filename)` → captures widget to PNG
  - Creates Cairo image surface at widget size
  - Renders widget content (or window region) to surface
  - Writes PNG file
  - On Lua side: `gui.screenshot(id, filename)` for visual debugging
  - **Defer if complex** — use TRACE output + `xdpyinfo` geometry check instead

### D.3 Test runner

- [ ] **D.3.1** Add test to `lui_test/`
  - Copy the test script to `lui_test/test_wlabel_gridbox.lua`
  - Add entry to `run_widget_tests.sh` that runs it
  - Use `xvfb-run` for headless execution (already set up)
  - Capture stdout for geometry assertions
  - Capture stderr for TRACE(2, "LAYOUT ...") output
- [ ] **D.3.2** Geometry verification pattern
  ```bash
  # Run test, capture geometry output
  xvfb-run --auto-servernum ./commander_runner -Tracelevel 2 \
      -Luafile test_wlabel_gridbox.lua > geom.log 2>&1
  
  # Verify geometry
  grep "INITIAL: width=[1-9]" geom.log    # width must be positive
  grep "AFTER: width=" geom.log | while read line; do
      initial=$(echo $line | sed 's/.*width=\([0-9]*\).*/\1/')
      [ "$initial" -gt 0 ] || exit 1
  done
  ```

### D.4 LUI registry additions

The `grid` and `label` tags are already registered in `lui/registry.lua`:
- `label` → `"XtcwWlabel"` (line 3)
- `grid` → `"XtcwGridbox"` (line 9)

Need to verify constraint resources are passed through:
- [ ] **D.4.1** Check `backend_xt.lua` passes `gridx`, `gridy`, `weightx`, `weighty`, `fill`, `gravity`, `margin` as Xt args
- [ ] **D.4.2** If not, add property mapping in registry or backend
- [ ] **D.4.3** Test: `(grid {} (label {:gridx 1 :gridy 2 :weightx 3 ...}))` must create a Gridbox with Wlabel child at the correct constraint position

---

## Phase E: U-Turn Checkpoints (When to Abandon an Approach)

### E.1 Geometry negotiation dead-ends

| Symptom | Likely Cause | U-Turn Action |
|---------|--------------|---------------|
| Wlabel resize causes infinite loop | `set_values` → `XtMakeResizeRequest` → `geometry_manager` → `XtConfigureWidget` → `resize` → `set_values` | Add `$in_set_values` guard; break cycle at `set_values` |
| Gridbox layout produces 0-width children | Unsigned underflow in margin/border calc | Use `int` intermediates (already fixed per errors.md); check `layoutChild` |
| Shell blocks all resize with `allowShellResize=False` | Default shell setting | Force `allowShellResize=True` in all test shells; document as requirement |
| `XtMakeGeometryRequest` on Shell returns `XtGeometryNo` | WM rejects resize request | Accept `XtGeometryAlmost` from parent and re-layout with compromise size |
| retex measurement takes >100ms per call | `XCreatePixmap` + font init in `calculate_size` | Use Phase A.1 measurement backend; cache fonts per DPI |

### E.2 Widget architecture dead-ends

| Symptom | Likely Cause | U-Turn Action |
|---------|--------------|---------------|
| Wlabel.$width not expanding | wbuild regression | Edit generated `.c` directly in `build/source/`; pin the file |
| Gridbox constraint resources invisible | Wrong superclass (composite vs constraint) | Link `plainc_widgets/Gridbox.o` directly, not the wbuild stub |
| LUI can't set constraint resources | Missing property mapping in backend_xt.lua | Add explicit mapping for Gridbox constraint resources |
| Pixmap >4000px crashes X11 | Backend cap + no size negotiation cap | Cap in both `calculate_size` and `query_geometry`; add scroll area if needed |
| Two widgets define same EXPORT function | `@EXPORTS` in multiple widgets | Rename to include widget name prefix (`wlabel_update_cache` vs `wretex_update_cache`) |

### E.3 When to stop and re-evaluate

**Hard stop criteria**:
1. Any single fix takes >2 hours of debugging wbuild internals → edit generated `.c` instead
2. Gridbox needs >3 new features to support Wlabel properly → switch to HBox/VBox
3. retex rendering shows visual corruption for >10% of test cases → retex needs its own debugging phase first
4. Shell/WM interaction is inconsistent across window managers → force `allowShellResize=True` and document WM dependency

---

## Phase F: Verification Checklist

### F.1 Per-widget verification (after each Phase)

For Wlabel:
- [ ] `gui.get_widget("lbl1")` returns non-nil
- [ ] `gui.get_geometry("lbl1")` returns width>0, height>0
- [ ] `gui.get("lbl1", "label")` returns correct string
- [ ] `gui.set("lbl1", "label", "new text")` → geometry updates if autoHeight
- [ ] `query_geometry` returns `XtGeometryYes` when sizes match
- [ ] `query_geometry` returns `XtGeometryAlmost` with preferred sizes when different
- [ ] `resize` doesn't destroy pixmap when dimensions unchanged
- [ ] Expose event copies pixmap to window without garbage
- [ ] Selection highlight renders on mouse drag (visual check)
- [ ] No X11 protocol errors on creation, resize, destruction

For Gridbox:
- [ ] `gui.get_widget("grid")` returns non-nil
- [ ] Children are positioned/sized according to gridx/gridy/weight
- [ ] `geometry_manager` re-layouts on child resize request
- [ ] `changeGeometry` negotiates with parent correctly
- [ ] `query_geometry` returns correct preferred size based on children
- [ ] Unmanaged children are excluded from layout
- [ ] Weight distribution: child with weight=2 gets 2× space of weight=1

### F.2 Integration verification

- [ ] Wlabel inside Gridbox renders correctly (static layout)
- [ ] Resizing window → Gridbox re-layouts → Wlabel re-renders
- [ ] Changing Wlabel text → autoHeight requests resize → Gridbox accommodates
- [ ] Two Wlabels in Gridbox with different weights → correct proportion
- [ ] No infinite resize loops (verified by TRACE output count)
- [ ] X11 error handler reports zero protocol errors
- [ ] No MLS "Not initialized" errors (MLS_DEBUG consistency)

---

## Phase G: Implementation Order

```
G.1 ── Fix build symbol collision (Wretex update_cache rename) ── 30min
G.2 ── Fix Wlabel calculate_size (measurement backend)         ── 60min
G.3 ── Fix Wlabel query_geometry (correct Xt protocol)        ── 30min
G.4 ── Fix Wlabel resize (conditional pixmap recreate)        ── 30min
G.5 ── Fix Wlabel autoHeight (XtMakeResizeRequest)            ── 30min
G.6 ── Add xtgeometry() C binding + LUI wrapper               ── 45min
G.7 ── Create test_wlabel_gridbox.lua                         ── 30min
G.8 ── Run test, fix bugs, iterate                            ── 60min
G.9 ── Fix HBox/VBox geometry_manager (if Gridbox fails)      ── 45min
G.10── Final verification (checklist F.1 + F.2)               ── 30min
─────────────────────────────────────────────────────────────────────────
Estimated total: 5-7 hours
```

**Critical path**: G.2 → G.3 → G.5 → G.7 → G.8
**Parallel possible**: G.1 + G.6 can be done in parallel with G.2-G.5

---

## References

### In-project files
```
learn.md                        — Gridbox API, TRACE, wbuild, pitfalls
errors.md                       — Fixed bug catalogue, prevention patterns
ultimate_plan.md                — Widget verification checklist
overview.md                     — Build pipeline, selection, $ expansion rules
howto.md                        — LUI S-expression format
AGENTS.md                       — GitNexus tools (run impact analysis before edits)

plainc_widgets/Gridbox.c        — Reference geometry_manager (496-618)
libxt-1.2.1/src/Shell.c         — allowShellResize gate (1905-1954)
libxt-1.2.1/src/Geometry.c      — XtMakeGeometryRequest implementation
experimental/commander_runner.c — Shell creation, allowShellResize=True (32)
lui/registry.lua                — 22 registered tags
lui/gui_xt.lua                  — get_widget, get, set
lui/test_harness.lua            — click, toggle, action, expect, sequence

wbuild_widgets/Wlabel.widget    — Canonical source for Wlabel
wbuild_widgets/HBox.widget      — Broken geometry_manager (93-111)
wbuild_widgets/VBox.widget      — Broken geometry_manager (94-112)
wbuild/init.w                   — Xt class definitions for wbuild codegen
```

### Xt specification
```
O'Reilly X Toolkit Intrinsics Vol 4  — Definitive geometry protocol
X11/Xlib.h                            — XtGeometryResult enum
libxt-1.2.1/src/Intrinsic.h:199-204  — Geometry result constants
```
