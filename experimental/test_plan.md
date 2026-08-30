# Widget Test Plan

> **Version**: 1.0  
> **Created**: 2026-04-13  
> **Status**: Living Document

---

## Guiding Principle

**No widget is "done" until it renders correctly AND its actions/callbacks work.**  
Past experience shows that widgets compiled and linked without errors, but produced broken graphical output, truncated text, misaligned layouts, or silently dropped callbacks. Every test must verify **visual output** and **functional behavior**, not just "no crash."

---

## Lessons Learned (from errors.md and session history)

### Pattern 1: Widget Compiles ≠ Widget Works
Multiple widgets (Wlabel, WlsMulti, Gridbox, Woptc) compiled cleanly but had critical runtime bugs:
- **Wlabel**: Text height truncated due to `int` cast of subpixel values; selection didn't render visually
- **WlsMulti**: Property propagation broken — `set_values` didn't forward properties to child widget
- **Gridbox**: Unsigned underflow caused children to render at 65534px wide
- **Woptc**: No `query_geometry` → 0-width widgets in Gridbox

### Pattern 2: Layout/Rendering Bugs Are Silent
None of the above bugs caused crashes. They only showed up when you **looked at the rendered output**. Unit tests that only check widget creation or callback firing will miss rendering regressions.

### Pattern 3: Action Procs Fire But Callbacks Don't
The Command widget's `notify` action is guarded by `command.set`. Calling `xtaction(widget, "notify")` without `xtaction(widget, "set")` silently does nothing. Tests must verify the full action chain.

### Pattern 4: TRACE(50) Instrumentation Must Be in Source of Truth
Edits to generated `.c` files in `build/source/` are overwritten by `wbuild`. Always edit `.widget` files in `wbuild_widgets/` for permanent TRACE(50).

### Pattern 5: Some plainc_widgets Cannot Be Linked
SelectW, SliderW, Flip have missing symbols (`sig_send`, `WheelWidgetClassInstance`, `XtNnormalFg`). They are standalone example widgets not designed for the main build.

---

## Test Categories

### Category 1: Widget Existence & Creation (E-level)

**Goal**: Verify each widget class can be instantiated via LUI without errors or warnings.

```lua
local gui = require('gui_xt')
local ui = '(window :id "tw" :title "Test" :width 200 :height 150\n  (grid :id "gr"\n    (WIDGET :id "w1")))\n'
lui.run(ui)
xtapptimeout(500, function()
    local w = gui.get_widget("w1")
    assert(w ~= nil, "Widget not created")
    xtapptimeout(200, function() xtappexitflag() end)
end)
```

**Pass criteria**:
- No "Unknown widget tag" warnings
- No X11 protocol errors
- `gui.get_widget(id)` returns non-nil
- Widget class matches expected class

**All xtcw widgets to test** (43 classes from register_wb.c):
Board, Canvas, Common, Dartboard, Frame, Gauge, Gridbox, HBox, HSlider, IconSVG, MessageBox, Radio2, ScrolledCanvas, SelectReq, VBox, VSlider, Wbutton, Wcheckbox, Wcombo, WeditMV, Wedit, WfileSelector, Wheel, Wlabel, Wlist4, WlistMulti, Wlist, WlsMulti, Wls, WmenuPopup, Wmenu, Woptc, Woption, WPaned, Wpassword, WpixBtn, Wradio, WSeparator, WspinBox, Wsplitter, Wtext, WviewVar

**Xaw widgets** (key subset): Command, Toggle, List, AsciiText, Paned, Box

---

### Category 2: Property Get/Set Round-Trip (P-level)

**Goal**: Verify that setting a resource and reading it back returns the expected value.

```lua
gui.set("w1", "label", "Test")
local val = gui.get("w1", "label")
assert(val == "Test", "Expected 'Test', got: " .. tostring(val))
```

**Widgets and properties to test**:

| Widget | Properties | Expected |
|--------|-----------|----------|
| Wlabel | label, alignment | "Hello" → set → get → "Hello" |
| Wbutton | label | Button text round-trip |
| Command | label | Command text round-trip |
| Toggle | state | Toggle state: false → true |
| Wedit | label (also "value" falls back to label) | "StartText" → set "Changed" → get "Changed" |
| Wpassword | label | "Secret" → set "NewSecret" → get "NewSecret" |
| WspinBox | value, min, max | 5 → set 42 → get 42 |
| Wcombo | label | Combo text round-trip |
| Wcombo | entry_mode | Toggle entry mode |
| Wcheckbox | state | Check/uncheck state |
| Wlist4 | lines | List content |
| Gridbox | gridx, gridy, weightx, weighty | Layout properties |
| WPaned | frac | Pane position |

---

### Category 3: Callback & Action Verification (A-level)

**Goal**: Verify that widget actions produce observable effects (callbacks fire, state changes).

**Critical pattern for Command widget**:
```lua
xtaction(widget, "set")      -- MUST set command.set = True first
xtaction(widget, "notify")   -- Then callbacks fire
xtaction(widget, "unset")    -- Clean up
```

**Widgets and actions**:

| Widget | Action(s) | Verification |
|--------|----------|-------------|
| Command | set→notify→unset | Callback fires, counter increments |
| Toggle | notify | State toggles |
| WpixBtn | highlight→notify→reset | TRACE(50) output; callback fires |
| Wlist4 | highlight, reset, select_line | TRACE(50) output |
| Wcombo | set_cursor, insert_char, notify | TRACE(50) output |
| Wlabel | select_start→select_extend→select_end | TRACE(50) output |
| Wedit | insert-char, delete-next | Text changes |
| WspinBox | spin-up, spin-down | Value changes |

---

### Category 4: Layout & Rendering Verification (L-level)

**Goal**: Verify that widgets render with correct geometry and visual output.

**This is the category that catches the bugs from errors.md.**

#### 4a. Geometry Verification

```lua
-- Verify widget has non-zero dimensions after realize
local w = gui.get_widget("w1")
local width = gui.get("w1", "width")
local height = gui.get("w1", "height")
assert(tonumber(width) > 0, "Widget has zero width")
assert(tonumber(height) > 0, "Widget has zero height")
```

#### 4b. Gridbox Layout Verification

Test that Gridbox distributes space correctly:
- Weight-based distribution (weightx=1 vs weightx=2)
- Cell spanning (gridWidth=2)
- Fill modes (none, width, height, both)
- **Check**: Children must have width > 0 and height > 0 (catches unsigned underflow)

#### 4c. Text Rendering Verification

- Wlabel text must be visible (width > 0, height > 0)
- Wlabel with `autoHeight` must adjust height to fit text
- Wlabel alignment (0=left, 1=center, 2=right) must produce different x offsets
- **Check**: `ceil()` is used for pixel calculations (catches truncation)

#### 4d. Selection Rendering Verification (Wlabel)

- Select text via `select_start` → `select_extend` → `select_end`
- Verify selection state is set (`dirty = 1` forces redraw)
- **Check**: Selection highlight is visible (catches the "selection not updating" bug from errors.md)

---

### Category 5: TRACE(50) Instrumentation (T-level)

**Goal**: Verify TRACE(50) output for instrumented widget action procs.

Currently instrumented widgets (all in wbuild source of truth):

| Widget | Source | Actions | TRACE Format |
|--------|--------|---------|---------------|
| WpixBtn | `wbuild_widgets/WpixBtn.c` | highlight, reset, notify, next_pixmap | `[50]highlight: WpixBtn %s highlight` |
| Wlist4 | `wbuild_widgets/Wlist4.widget` | highlight, reset, motion_start, motion_end, select_line, toggle_line_state_act | `[50]Wlist4 %s highlight` |
| Wcombo | `wbuild_widgets/Wcombo.widget` | SetKeyboardFocus, set_cursor, insert_char, remove_char, focus_in, focus_out, notify | `[50]Wcombo %s notify` |
| Wlabel | `wbuild_widgets/Wlabel.widget` | info, select_start, select_extend, select_end | `[50]Wlabel %s info` |

**Test procedure**:
1. Run with `-Tracelevel 50`
2. Trigger actions via `xtaction()`
3. Capture stderr and grep for `[50]` lines
4. Assert expected patterns appear

---

## Test Infrastructure

### Current Test Runner
- `lui_test/run_widget_tests.sh` — runs under `xvfb-run`, captures combined stdout+stderr
- `lui/test_harness.lua` — Lua module with `click()`, `toggle()`, `action()`, `expect()`, `verify()`
- C bindings: `xtaction()`, `xtapptimeout()`, `gui.get_widget()`, `gui.get()`, `gui.set()`

### Required Enhancements

#### 1. Screenshot/Pixmap Capture Test

Capture widget rendered output for automated comparison:

```c
// In xt_bridge.c — new binding: xtscreenshot(widget_id, filename)
Pixmap pix = XtGetPixmap(widget);  // or use XGetImage
XWriteBitmapFile(display, filename, pix, width, height);
```

This would allow:
- Baseline screenshots for regression testing
- Pixel-level comparison of rendered output
- Automated detection of rendering bugs (truncated text, misaligned layouts)

#### 2. Geometry Query Bindings

Add to `xt_bridge.c`:
```c
// xtgeometry(widget) → { width, height, x, y }
// xtquerygeometry(widget) → preferred { width, height }
```

This enables assertions like:
```lua
assert(geo.width > 0, "Widget has zero width")
assert(geo.height >= 20, "Widget too short for text")
```

#### 3. X11 Error Handler Check

Add an X11 error handler that counts protocol errors:
```c
static int x11_error_count = 0;
int x11_error_handler(Display *d, XErrorEvent *e) {
    x11_error_count++;
    return 0;
}
// Reset at test start, assert == 0 at test end
```

#### 4. Warning Detection

The current test runner should also check stderr for "Warning:" lines from the LUI parser (unknown widget tags, missing resources) and fail the test if unexpected warnings appear.

---

## Test Execution Matrix

### Phase A: Smoke Test (All Widgets)
Run E-level tests for every registered widget class. This catches:
- Missing registry entries
- Widget classes that compile but fail at XtCreateWidget
- Resource type mismatches

### Phase B: Property Tests
Run P-level tests for key widgets. This catches:
- Broken property propagation (WlsMulti bug pattern)
- Resource name mismatches (WlsMulti callback→notify bug pattern)

### Phase C: Action & Callback Tests
Run A-level tests for interactive widgets. This catches:
- Guard conditions (Command.set bug pattern)
- Callback dispatch latency (100ms polling timer)

### Phase D: Layout & Rendering Tests
Run L-level tests for layout-sensitive widgets. This catches:
- Unsigned underflow (Gridbox bug pattern)
- Text truncation (Wlabel int cast bug pattern)
- Selection rendering (Wlabel dirty flag bug pattern)
- query_geometry bugs (Woptc bug pattern)

### Phase E: TRACE(50) Regression
Run T-level tests for instrumented widgets. This catches:
- Regressed TRACE(50) calls (if `.widget` file edits are lost)
- Action proc failures (if action signatures change)

---

## Priority Order for New Tests

1. **E-level: All 43 xtcw widgets** — Create existence test for each widget class
2. **P-level: Top 10 widgets** — Wlabel, Wbutton, Wedit, Wcombo, Wlist4, Gridbox, WpixBtn, WspinBox, Wcheckbox, Wpassword
3. **A-level: Interactive widgets** — Command, Toggle, WpixBtn, Wlist4, Wcombo
4. **L-level: Layout widgets** — Gridbox (weightx/y, fill, spanning), Wlabel (text, alignment, selection), HBox/VBox
5. **T-level: All instrumented widgets** — WpixBtn, Wlist4, Wcombo, Wlabel

---

## File Structure

```
lui_test/
├── run_widget_tests.sh          # Master test runner
├── run_all_tests.sh             # Run all test categories (new)
├── test_harness.lua             # Test utilities module
├── test_creation.lua            # E-level: widget existence (new)
├── test_properties.lua          # P-level: property round-trip (new)
├── test_actions.lua             # A-level: callbacks & actions (enhanced)
├── test_layout.lua              # L-level: geometry & rendering (new)
├── test_trace.lua               # T-level: TRACE(50) regression (new)
├── test_button.lua              # Current: button callback test
├── test_toggle.lua              # Current: toggle test
├── test_focus.lua               # Current: focus test
├── test_list.lua                # Current: list test
└── test_results/                # Output directory
```

---

## C Bindings for Testing

Two new C bindings were added for test infrastructure:

### `xtgeometry(widget)` → table
Returns a Lua table with `x`, `y`, `width`, `height` of the widget. Usage: `gui.geometry("id")` or `xtgeometry(gui.get_widget("id"))`.

### `xterror_count(reset)` → integer
Returns the number of X11 protocol errors since the last reset. Call `xterror_count(true)` to reset the counter and install the error handler. Call `xterror_count(false)` to just read the count. Usage: `gui.xerrors()` or `xterror_count(false)`.

---

## Verified Test Results (Session 2026-04-13)

**Test runner**: `lui_test/run_widget_tests.sh` — 8/8 passing (automated)

**Standalone verified** (run individually due to xvfb-run cleanup issues):

| # | Test | Category | Result |
|---|------|----------|--------|
| 1 | Widget creation (label, command) | E-level | ✅ PASS |
| 2 | Label property round-trip | P-level | ✅ PASS |
| 3 | Button callback (set→notify→unset) | A-level | ✅ PASS |
| 4 | WpixBtn action TRACE(50) | T-level | ✅ PASS |
| 5 | TRACE(50) level activation | T-level | ✅ PASS |
| 6 | SpinBox property round-trip (value, min, max) | P-level | ✅ PASS |
| 7 | Edit/Password label round-trip + value fallback | P-level | ✅ PASS |
| 8 | Toggle callback + state | A-level | ✅ PASS |
| 9 | All-widgets creation smoke test (20/20) | E-level | ✅ PASS |
| 10 | Widget geometry (4/4 positive dimensions) | L-level | ✅ PASS |
| 11 | X11 error detection (0 errors) | L-level | ✅ PASS |
| 12 | Wcombo/Wlabel TRACE(50) actions | T-level | ✅ PASS |

### Widget-by-widget verified status:

| Widget | E | P | A | T | L | Notes |
|--------|---|---|---|---|---|-------|
| Command | ✅ | ✅ label | ✅ set→notify | — | — | |
| Label | ✅ | ✅ label | — | — | — | |
| Wlabel | ✅ | ✅ label | — | ✅ select_start | — | |
| Wbutton | ✅ | — | — | — | — | |
| Toggle | ✅ | ✅ state | ✅ notify | — | — | |
| WpixBtn | ✅ | — | ✅ highlight/notify/reset | ✅ TRACE | — | |
| Wlist4 | ✅ | — | — | ✅ highlight/reset | — | |
| Wcombo | ✅ | ✅ label | — | ✅ focus_in/out | — | CallOnEnter warning |
| WspinBox | ✅ | ✅ value/min/max | — | — | — | |
| Wedit | ✅ | ✅ label, value→label fallback | — | — | — | |
| Wpassword | ✅ | ✅ label | — | — | — | Text in `label` resource |
| Gridbox | ✅ | — | — | — | — | |
| WSeparator | ✅ | — | — | — | — | |
| Splitter/Wsplitter | ✅ | ✅ fraction | — | — | — | |
| IconSVG | ✅ | — | — | — | — | Needs valid SVG path |
| ScrolledCanvas | ✅ | — | — | — | — | |
| WlsMulti (list-view) | ✅ | ✅ model/tableStrs | — | — | — | **Bug**: crashes with `retexCells=true` |
| Check/Wradio | ✅ | — | — | — | — | |

### Known issues found during testing:

1. **WlsMulti `retexCells=true` crash**: `vas_printf` MLS error. Omit this property.
2. **Wcombo `CallOnEnter` warning**: Harmless — action not registered but widget works.
3. **Edit widgets use `label` for text**: `gui.get(id, "value")` falls back to `label` automatically.
4. **SpinBox S-expression property**: `:spinValue` maps to `value` resource internally.

---

## Success Criteria

A widget is considered **verified** when it passes:

1. **E-test**: Creates without error, no "Unknown widget tag" warning
2. **P-test**: At least one property round-trips correctly
3. **No X11 protocol errors** on creation or destruction
4. **No memory leaks** detectable by AddressSanitizer beyond known baseline

A widget is considered **fully verified** when it also passes:

5. **A-test**: Actions produce expected TRACE(50) output and/or callback fires
6. **L-test**: Renders with width > 0 and height > 0, text visible, layout correct
7. **T-test**: TRACE(50) instrumentation present in source of truth