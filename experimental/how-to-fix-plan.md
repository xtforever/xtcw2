# xtcw2 Completion Plan

> Derived from `how-to-fix`, `errors.md`, `ultimate_plan.md`, codebase audit, and X Toolkit Intrinsics spec (libxt-1.2.1/specs/CH06.xml).
> Root cause: LLMs misinterpret Xt's implicit geometry protocol and wbuild conventions, causing widgets to compile but render broken.

## Xt Geometry Protocol Reference

> Sources: X Toolkit Intrinsics spec (CH06), *X Toolkit Intrinsics Programming Manual* (O'Reilly, 2nd Ed., R4) Chapters 6 & 11.

### `query_geometry` — When You Need It

**Technically optional, practically essential for all display widgets.** If the `query_geometry` field is NULL, `XtQueryGeometry` returns `XtGeometryYes` with the widget's current geometry as "preferred." This means if the widget was ever resized to 1x1, the parent is told "1x1 is my preference" — permanently losing the widget's natural size. Per the O'Reilly book: "This is often wrong information. Even if your widget has no particular preference for size, it is a good idea to specify the widget's default size in query_geometry."

**You MUST have `query_geometry` when:**
1. **Content-dependent sizing** — preferred size depends on dynamic content (Wlabel text wrapping, Wlist row count)
2. **Child of a layout manager** — Gridbox/VBox/HBox must know preferences to compute column/row sizes
3. **Widget can be resized** — any widget placed in a resizable container needs to report preferences after size changes

**The only widgets that can safely omit `query_geometry`:**
1. Base/abstract classes (Wheel) that are never instantiated directly
2. Pure containers with no intrinsic preferred size
3. Widgets ONLY used as fixed-size popups/dialogs never resized by parent

### `query_geometry` — Contract

Per the spec (CH06 §6.5), the procedure must:

```c
XtGeometryResult query_geometry(Widget w, XtWidgetGeometry *request, XtWidgetGeometry *preferred_return)
```

1. **Examine** bits set in `request->request_mode` (what parent plans to change)
2. **Compute** preferred geometry based on widget content and constraints
3. **Store** result in `preferred_return`, setting bits in `preferred_return->request_mode` for fields it cares about
4. **Return:**
   - **`XtGeometryYes`** — proposed change acceptable without modification. Parent need not modify plans.
   - **`XtGeometryAlmost`** — child's preference differs from parent's intention OR child expressed interest in a field parent didn't ask about. Most common return value for content-aware widgets.
   - **`XtGeometryNo`** — preferred geometry is identical to current geometry AND both parent/child expressed interest in overlapping fields.

### `resize` — When You Need It

Per spec (CH06 §6.6): "If a class need not recalculate anything when a widget is resized, it can specify NULL. This is an unusual case and should occur only for widgets with very trivial display semantics."

**You need `resize` when** the widget must re-layout internal data when its dimensions change. If NULL, the widget will be reconfigured (window resized) but won't know to redraw its content.

### `set_values_almost` — When You Need It

Called when a geometry request returns `XtGeometryAlmost`. The child modifies `request` to accept the compromise. If NULL, the inherited version from Core/RectObj is used, which may not handle widget-specific layout needs.

### Key Protocols

**Initial geometry negotiation** (O'Reilly Ch 11.1.1): When `XtRealizeWidget` is called:
1. `change_managed` called BOTTOM-UP on every composite — each parent queries children via `XtQueryGeometry` or uses current width/height
2. Shell sets its size to child's size
3. If user specified geometry (command line/resources) differs → `resize` cascades TOP-DOWN, with parents re-querying children
4. Only then: `realize` methods called, windows created

**Run-time geometry negotiation:**

| Protocol Step | Who | Action |
|---------------|-----|--------|
| 1. Child reports preferred size | Child | `query_geometry()` returns preferred dimensions |
| 2. Parent computes layout | Parent (Manager) | Uses preferred sizes + weights to allocate space |
| 3. Parent proposes compromise | Parent | Returns `XtGeometryAlmost` with compromise in `reply` |
| 4. Child accepts compromise | Child | `set_values_almost()` sets `request = reply` |
| 5. Parent applies size | Parent | Calls child's `resize()`, then `expose()` |

---

## Phase 1: Formalize wbuild/Xt Conventions (2 days)

**Goal:** Create a single reference document that eliminates ambiguity for code generation.

### Task 1.1: Write `wbuild_widgets/CONVENTIONS.md`

A spec covering all rules that LLMs (and humans) must follow when writing `.widget` files:

- `@TRANSLATIONS` must use `@trans` prefix, not `@` — see errors.md:50
- Constraint widgets MUST inherit from `constraintWidgetClass`, not `compositeWidgetClass` — see testing.md:34
- TopLevel windows MUST set `allowShellResize: True` on the Shell — see how-to-fix:12
- Gridbox/VBox/HBox children MUST specify `weightx`, `weighty`, `fill` (default FillBoth) — see how-to-fix:19
- Interactive/display widgets SHOULD implement `expose` and `resize`
- Widgets with content-dependent sizing SHOULD implement `query_geometry`
- Widgets that are children of layout managers SHOULD implement `query_geometry`
- `query_geometry` MUST check `request->request_mode & CWWidth`, use `request->width` for constrained layout computation
- `query_geometry` return: `XtGeometryYes` if parent's proposal is fine, `XtGeometryAlmost` if child has different preference, `XtGeometryNo` if child's preference equals its current geometry
- `set_values_almost` SHOULD be implemented for widgets with `query_geometry` that may get compromise sizes
- `set_values_almost` MUST: `request->width = reply->width; request->height = reply->height; request->request_mode = CWWidth | CWHeight`
- Coordinate types: use `ceil()` for subpixel-to-pixel conversions (NOT `int` cast — see errors.md:18)
- Intermediate geometry math MUST use signed `int`, not unsigned `Dimension` — see errors.md:18
- `resize()` MUST set `$dirty = 1` and call redraw
- `expose()` MUST call redraw (which checks dirty flag)
- Link order matters: real hand-written implementations (e.g., `plainc_widgets/Gridbox.o`) must take precedence over wbuild stubs — see testing.md:36

### Task 1.2: Update `docs/xt-geometry.md`

Add the conventions checklist as a concrete verification table.

---

## Phase 2: Audit & Fix Geometry in All Widgets (3-4 days)

**Goal:** Every widget compiles, creates, realizes, and renders with correct geometry at the C level. No LUI dependency — widgets are verified via standalone C test programs that link directly against `libxtcw.a` and run under `xvfb-run`.

**Principle:** Widgets must work before LUI can work. LUI is useless if widgets are broken. All testing in Phase 2 is pure C.

### Test infrastructure for Phase 2

Each widget gets a C test program in `tests/` that follows this pattern:

```c
// tests/test_widget_geometry.c
// Build: make test_widget_geometry  (add to tests/makefile)
// Run:   xvfb-run -a ./test_widget_geometry

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <xtcw/Wlabel.h>   // generated header
#include <xtcw/Gridbox.h>

static int x11_errors = 0;
static int x11_error_handler(Display *d, XErrorEvent *e) {
    x11_errors++; return 0;
}

int main(int argc, char **argv) {
    XtAppContext app;
    XSetErrorHandler(x11_error_handler);

    Widget shell = XtVaAppInitialize(&app, "GeoTest", NULL, 0,
        &argc, argv, NULL,
        XtNallowShellResize, True, NULL);

    Widget grid = XtVaCreateManagedWidget("grid", gridboxWidgetClass, shell,
        XtNweightx, 1, XtNweighty, 1, NULL);

    Widget label = XtVaCreateManagedWidget("label", wlabelWidgetClass, grid,
        XtNlabel, "Hello World",
        XtNgridx, 0, XtNgridy, 0,
        XtNweightx, 1, XtNweighty, 1,
        XtNfill, 3, /* FillBoth */
        NULL);

    XtRealizeWidget(shell);

    // Verify geometry
    Dimension w = 0, h = 0;
    XtVaGetValues(label, XtNwidth, &w, XtNheight, &h, NULL);
    assert(w > 0 && "widget has zero width");
    assert(h > 0 && "widget has zero height");

    // Verify no X11 protocol errors
    assert(x11_errors == 0 && "X11 protocol errors detected");

    // Verify Gridbox children at distinct positions
    Position x1, y1, x2, y2;
    XtVaGetValues(label1, XtNx, &x1, XtNy, &y1, NULL);
    XtVaGetValues(label2, XtNx, &x2, XtNy, &y2, NULL);
    assert(x1 != x2 || y1 != y2 && "children stacked at same position");

    printf("PASS: %s\n", argv[0]);
    return 0;
}
```

**Running during Phase 2:**
```bash
make widgets              # regenerate .c from .widget
make tests                # build all test binaries
xvfb-run -a ./test_wlabel_geometry
xvfb-run -a ./test_gridbox_layout
```

**Pass criteria per widget (C-level, no LUI):**
- Widget creates via `XtVaCreateManagedWidget` without error
- Widget realizes via `XtRealizeWidget` without X11 protocol errors
- `XtVaGetValues(..., XtNwidth/XtNheight, ...)` returns `> 0`
- Gridbox children: `XtNx`/`XtNy` coordinates are distinct (not all 0,0)
- `XSetErrorHandler` counter stays at 0
- No stderr warnings

---

## Phase 3: C-Level Geometry Verification Suite (2 days)

**Goal:** Every widget fix from Phase 2 gets a permanent C test in `tests/`. Tests link directly against `libxtcw.a` and core libraries — **no libwcl.a dependency**. Widgets are created via `XtVaCreateManagedWidget("name", widgetClassPtr, parent, ...)` using the generated class symbols (e.g., `wlabelWidgetClass`). No WCL registration step needed.

### Task 3.1: Create C test templates

Add build rules to `tests/makefile` for geometry tests:

```makefile
# tests/makefile additions
TEST_SRCS += test_wlabel_geometry.c test_gridbox_layout.c test_vbox_layout.c \
             test_wbutton_geometry.c test_wedit_geometry.c test_wpixbtn_geometry.c \
             test_wcombo_geometry.c test_wspinbox_geometry.c test_wcheckbox_geometry.c \
             test_wpassword_geometry.c test_woption_geometry.c test_wradio_geometry.c \
             test_hslider_geometry.c test_vslider_geometry.c test_wpaned_geometry.c \
             test_wsplitter_geometry.c test_wseparator_geometry.c test_wretex_geometry.c \
             test_gauge_geometry.c test_icon_geometry.c test_wlist_geometry.c \
             test_wlistmulti_geometry.c test_wlist4_geometry.c test_wls_geometry.c \
             test_wlsmulti_geometry.c test_wtext_geometry.c test_wviewvar_geometry.c \
             test_canvas_geometry.c test_frame_geometry.c test_board_geometry.c \
             test_dartboard_geometry.c test_weditmv_geometry.c test_woptc_geometry.c \
             test_wmenu_geometry.c test_wmenupopup_geometry.c test_wfileselector_geometry.c \
             test_messagebox_geometry.c test_radio2_geometry.c test_scrolledcanvas_geometry.c

# Each test links against core libraries (NO libwcl dependency — pure C/Xt)
test_%_geometry: test_%_geometry.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -lxtcw -lretex -lutils -lplainc -lnanosvg -lXft -lX11 -lXt -lm
```

### Task 3.2: Standard test pattern for every widget

```c
// tests/test_widget_geometry.c — template for all widget geometry tests
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <xtcw/WIDGET.h>  // widget-specific header

static int x11_errors = 0;
static int x11_error_handler(Display *d, XErrorEvent *e) { (void)d; (void)e; x11_errors++; return 0; }
static int warnings = 0;
static void warning_handler(String msg) { (void)msg; warnings++; }

int main(int argc, char **argv) {
    XtAppContext app;
    XtSetWarningHandler(warning_handler);
    XSetErrorHandler(x11_error_handler);

    Widget shell = XtVaAppInitialize(&app, "Test", NULL, 0,
        &argc, argv, NULL,
        XtNallowShellResize, True,
        XtNmappedWhenManaged, False,
        NULL);

    // Test 1: Create widget directly (not in Gridbox — tests standalone realize)
    Widget w = XtVaCreateManagedWidget("test", WIDGETWidgetClass, shell, NULL);
    XtRealizeWidget(shell);

    Dimension width = 0, height = 0;
    XtVaGetValues(w, XtNwidth, &width, XtNheight, &height, NULL);
    assert(width > 0);
    assert(height > 0);
    assert(x11_errors == 0);

    XtDestroyWidget(shell);
    printf("PASS: %s (direct create)\n", argv[0]);
    return 0;
}
```

### Task 3.3: Gridbox layout test (separate binary — tests multiple widgets interacting)

```c
// tests/test_gridbox_layout.c — verifies Gridbox lays out children correctly
int main(int argc, char **argv) {
    // ... setup shell + gridbox ...
    Widget child[4];
    for (int i = 0; i < 4; i++) {
        char name[32]; snprintf(name, 32, "child%d", i);
        child[i] = XtVaCreateManagedWidget(name, wlabelWidgetClass, grid,
            XtNlabel, name,
            XtNgridx, i % 2,     // columns 0,1,0,1
            XtNgridy, i / 2,     // rows    0,0,1,1
            XtNweightx, 1, XtNweighty, 1,
            XtNfill, 3, /* FillBoth */
            NULL);
    }
    XtRealizeWidget(shell);

    // Verify all children have positive dimensions
    for (int i = 0; i < 4; i++) {
        Dimension w = 0, h = 0;
        XtVaGetValues(child[i], XtNwidth, &w, XtNheight, &h, NULL);
        assert(w > 0 && h > 0);
    }

    // Verify children at distinct positions (not all at 0,0)
    Position x[4], y[4];
    for (int i = 0; i < 4; i++) {
        XtVaGetValues(child[i], XtNx, &x[i], XtNy, &y[i], NULL);
    }
    // Top-left and top-right must differ
    assert(x[0] != x[1] && "children stacked at same x position");
    // Top-left and bottom-left must differ
    assert(y[0] != y[2] && "children stacked at same y position");

    // Verify weightx=2 child is wider than weightx=1 child
    // ... (add weighted child test) ...

    assert(x11_errors == 0);
    printf("PASS: Gridbox layout\n");
    return 0;
}
```

### Task 3.4: Run all geometry tests

```bash
# tests/run_geometry_tests.sh — added to tests/run_all_tests.sh
for test in test_*_geometry; do
    echo -n "$test: "
    xvfb-run -a ./$test && echo "PASS" || echo "FAIL"
done
```

---

## Phase 4: C-Level Integration Demos (2 days)

**Goal:** Prove the widget set works together in real C programs. LUI integration is deferred until widgets are verified at the C level.

### Task 4.1: Create C demo programs

New files in `tests/demos/` (or `demos/`), compiled and linked against `libxtcw.a`:

- `demos/demo_form.c` — Form with labels, edits, spinbox, toggle, buttons in Gridbox
- `demos/demo_calc.c` — Calculator with Gridbox column spanning
- `demos/demo_contacts.c` — Contact manager with multi-type widgets
- `demos/demo_login.c` — Login dialog with password fields, toggle, feedback

Each demo follows the same pattern: create shell → Gridbox → children with constraints → realize → enter Xt event loop.

### Task 4.2: Widget gallery demo

`demos/demo_gallery.c` — single window showing one of each widget class in a scrollable Gridbox layout. Organized by category (Display, Input, List, Slider, Layout, Special).

### Task 4.3: Automated demo smoketest

Run each demo under `xvfb-run` with a 2-second timeout (demand exit via `XtAppAddTimeOut`):

```bash
# demos/run_demos.sh
for demo in demo_form demo_calc demo_contacts demo_login; do
    echo -n "$demo: "
    timeout 3 xvfb-run -a ./$demo && echo "OK" || echo "FAIL"
done
```

Verifies: no crash, no X11 errors, no geometry warnings on startup.

---

## Phase 5: LUI Integration (after widgets work at C level) (2 days)

**Goal:** Wire the verified widget set into the LUI framework. Now that widgets are proven working at C level, fix LUI-specific issues.

### Task 5.1: Verify LUI registry coverage

- Ensure all 42 widget tags are registered in `lui/registry.lua`
- Fix any "Unknown widget tag" warnings

### Task 5.2: Port C tests to LUI tests

- Convert the C geometry tests to Lua equivalents in `lui_test/`
- Verify `gui.get()`, `gui.set()`, `gui.geometry()`, `gui.xerrors()` work for all widgets

### Task 5.3: Port C demos to LUI demos

- Convert `demos/demo_form.c` → `demos/demo_form.lua`
- Convert other C demos to LUI equivalents
- Run existing `t1.sh` demos, fix any remaining issues

---

## Phase 6: Cleanup & Final Verification (1 day)

## Phase 6: Cleanup & Final Verification (1 day)

### Task 6.1: Fix test baseline drift

- Update `tests/baselines/test_retex_math.layout` to match current font metrics
- Re-run `tests/verify_layout.sh` — all baselines must pass

### Task 6.2: Commit LuaRunner cleanup

The `git status` shows ongoing LuaRunner transition (removing stale wbuild stubs, using canonical widgets from `build/lib/`). Complete and commit this.

### Task 6.3: Documentation refresh

- Update `docs/widgets.md` with final verified status for each widget
- Note: `wbuild_widgets/CONVENTIONS.md` and `docs/xt-geometry.md` already written in Phase 1

### Task 6.4: Index refresh

- Run `npx gitnexus analyze` to refresh the GitNexus index

---

## Success Criteria

- `make clean && make` succeeds without errors
- All C tests pass: `bash tests/run_all_tests.sh`
- All C geometry tests pass: `bash tests/run_geometry_tests.sh`
- Layout verification passes: `bash tests/verify_layout.sh`
- All P0 + P1 widgets have `query_geometry`, `resize`, `expose` implemented
- Wlabel, Wretex, VBox, HBox + P0 display widgets have `set_values_almost`
- All C demos run without crash: `bash demos/run_demos.sh`
- Zero X11 protocol errors across all test runs
- Widget gallery demo shows all 42 widgets with `width > 0 AND height > 0`
- **(Phase 5 gates):** LUI tests pass, `t1.sh` demos work, LUI demos match C demo behavior

## Estimated Total Effort

| Phase | Duration |
|-------|----------|
| 1: Conventions | 2 days (done) |
| 2: Geometry fixes + C tests | 3-4 days |
| 3: C-level geometry suite | 2 days |
| 4: C-level integration demos | 2 days |
| 5: LUI integration | 2 days |
| 6: Cleanup | 1 day |
| **Total** | **12-13 days** |
