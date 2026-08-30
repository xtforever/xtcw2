# XTCW2 Ultimate GUI Toolkit Implementation Plan

> **Version**: 1.2  
> **Created**: 2026-03-14  
> **Revised**: 2026-04-13  
> **Status**: Living Document

---

## CRITICAL FINDINGS (v1.2 Update - 2026-04-13)

### No Fully Working Demo Yet

Despite phases 0–5 being marked "✅ COMPLETE", **there is no fully working graphical demo**. Every attempt to run a complete application revealed that widgets compiled but produced broken graphical output or silently dropped callbacks. The "complete" status means only that widgets compile and link — not that they render correctly or function as expected.

### Key Bug Patterns Found (from errors.md)

1. **Compiles ≠ Works**: Wlabel text was truncated (int cast of subpixel values), Gridbox children got 65534px wide (unsigned underflow), Woptc had 0-width (missing query_geometry). None crashed.
2. **Silent failures**: Command widget's `notify` action silently does nothing unless `set()` is called first. These require functional testing, not just visual inspection.
3. **Property propagation broken**: WlsMulti's `set_values` didn't forward properties to its child. Wlabel selection didn't visually update because `dirty` flag wasn't set.
4. **Layout rendering bugs are invisible to unit tests**: Only showing the window reveals truncation, misalignment, zero-width widgets, etc.

### Phase Progress Update (Corrected)

| Phase | Status | Notes |
|-------|--------|-------|
| Phase 0: Widget Audit | ✅ COMPLETE | 42 widgets in wbuild_widgets/ |
| Phase 1: Foundation | ⚠️ BUILD ONLY | Builds, but no visual verification |
| Phase 2: Core Widgets | ⚠️ COMPILE ONLY | Widgets compile; rendering bugs found in Wlabel, Gridbox, WlsMulti |
| Phase 3: Advanced Widgets | ⚠️ COMPILE ONLY | Wmenu, WmenuPopup, Gauge compile; no demo verified |
| Phase 4: Layout & Containers | ⚠️ COMPILE ONLY | Gridbox had unsigned underflow (fixed); no regression suite |
| Phase 5: Input Controls | ⚠️ COMPILE ONLY | Wcheckbox, WspinBox, Wpassword compile; no demo verified |
| Phase 8: LUI + Test Harness | 🟡 IN PROGRESS | 22 LUI tags, 8/8 tests pass; no visual/rendering tests |
| Phase 9: Testing & QA | 🔴 INSUFFICIENT | 8 assertions pass, but only covers creation+callbacks, NOT rendering |

### Verified Widget Status ("confirmed running" = created, rendered correctly, actions work)

| Widget | Creation | Property Get/Set | Callback | TRACE(50) | Visual | Status |
|--------|----------|-----------------|----------|-----------|--------|--------|
| command | ✅ Found | ✅ label OK | ✅ set→notify→unset | ❌ Xaw upstream | ❓ Not verified | ⚠️ Partial |
| Wlabel | ✅ Found | ✅ label get/set | ❌ N/A | ✅ info, select_* | ❌ Truncation fixed, not regression-tested | ⚠️ Partial |
| WpixBtn | ✅ Found | ❓ Not tested | ❓ Not tested | ✅ highlight, notify, reset | ❌ Not verified | ⚠️ Partial |
| Gridbox | ✅ Found | ✅ gridx/y etc | ❌ N/A | ❌ No TRACE | ❌ Underflow fixed, not regression-tested | ⚠️ Partial |

**No widget is fully verified.** Every widget needs visual rendering verification.

### Measures to Catch Broken Widgets Early

1. **E-level tests**: Every widget must have a creation test that verifies `gui.get_widget(id)` returns non-nil AND no "Unknown widget tag" warnings appear. (See `test_plan.md` Category 1)
2. **L-level tests**: Key layout widgets (Gridbox, VBox, HBox, Wlabel) must verify `width > 0 AND height > 0` to catch unsigned underflow and truncation bugs. (See `test_plan.md` Category 4)
3. **A-level tests with full action chains**: Never test `notify` alone on Command — always test `set→notify→unset`. Test callback dispatch with 300ms delay. (See `test_plan.md` Category 3)
4. **TRACE(50) regression tests**: Every instrumented widget must have a test that triggers its actions and verifies `[50]` lines appear in stderr. (See `test_plan.md` Category 5)
5. **Incremental demo requirement**: No widget phase is "COMPLETE" until a visual demo runs showing all widgets with correct layout, text, and interaction. A screenshot or Xdpyinfo geometry check is required.
6. **X11 error check**: Tests must count X11 protocol errors (via `XSetErrorHandler`). Zero errors on creation and destruction.
7. **Warning detection**: stderr must be scanned for "Warning:" lines from the LUI parser. Unknown widget tags indicate missing registry entries.

### Test Infrastructure Status

| Component | Status | Location |
|-----------|--------|----------|
| Test runner | ✅ Working | `lui_test/run_widget_tests.sh` |
| Test harness (Lua) | ✅ Working | `lui/test_harness.lua` |
| C bindings (xtaction, xtapptimeout) | ✅ Working | `experimental/xt_bridge.c` |
| Gui module (get_widget, get, set) | ✅ Working | `lui/gui_xt.lua` |
| LUI registry (22 tags) | ✅ Working | `lui/registry.lua` |
| TRACE(50) instrumentation (4 widgets) | ✅ Working | WpixBtn, Wlist4, Wcombo, Wlabel |
| Visual/screenshot tests | ❌ MISSING | — |
| X11 error detection | ❌ MISSING | — |
| Geometry verification | ❌ MISSING | — |
| Property round-trip tests | ⚠️ Minimal | Only label get/set tested |

| Source | Count | Status |
|--------|-------|--------|
| wbuild_widgets/ | 42 | ✅ CANONICAL |
| LuaRunner/ | ~15 | Lua integration |
| adminpanel/ | ~12 | Application widgets |
| AdmPnl/ | ~15 | Admin panel |
| Other directories | ~50+ | Legacy/experimental |

### Build Status
- ✅ `make clean && make all` succeeds
- ✅ All 42 canonical widgets compile
- ✅ Tests build and run

---

## Table of Contents

1. [Vision & Philosophy](#vision--philosophy)
2. [Architecture Overview](#architecture-overview)
3. [Reference Documentation](#reference-documentation)
4. [Phases Overview (Revised)](#phases-overview)
5. [Phase 0: Widget Audit](#phase-0-widget-audit)
6. [Phase 1: Foundation & Core Systems](#phase-1-foundation--core-systems)
7. [Phase 2: Core Widget Library](#phase-2-core-widget-library)
8. [Phase 3: Advanced Widgets (Consolidation)](#phase-3-advanced-widgets)
9. [Phase 4: Layout & Containers](#phase-4-layout--containers)
10. [Phase 5: Input Controls](#phase-5-input-controls)
11. [Phase 6: Data Views](#phase-6-data-views)
12. [Phase 7: Application Shell & Dialogs](#phase-7-application-shell--dialogs)
13. [Phase 8: Lua Integration (LUI)](#phase-8-lua-integration-lui)
14. [Phase 9: Testing & Quality Assurance](#phase-9-testing--quality-assurance)
15. [Phase 10: Documentation & Examples](#phase-10-documentation--examples)
16. [Phase 11: Performance & Optimization](#phase-11-performance--optimization)
17. [Phase 12: Ecosystem & Distribution](#phase-12-ecosystem--distribution)
18. [Verification Checklist](#verification-checklist)

---

## Phases Overview (Revised)

| Phase | Focus | Estimated Duration | Status |
|-------|-------|-------------------|--------|
| 0 | Widget Audit | 1 week | ✅ COMPLETE |
| 1 | Foundation | 2 weeks | ⚠️ BUILD ONLY |
| 2 | Core Widgets | 3 weeks | ⚠️ COMPILE ONLY — rendering bugs found |
| 3 | Advanced Widgets (Consolidate) | 2 weeks | ⚠️ COMPILE ONLY — no demo verified |
| 4 | Layout & Containers | 2 weeks | ⚠️ COMPILE ONLY — Gridbox underflow fixed |
| 5 | Input Controls | 2 weeks | ⚠️ COMPILE ONLY — no demo verified |
| 6 | Data Views | 3 weeks | 🔴 PENDING |
| 7 | Dialogs & Shell | 2 weeks | 🔴 PENDING |
| 8 | Lua Integration | 3 weeks | 🟡 IN PROGRESS — 22 tags, 8/8 tests, no visual tests |
| 9 | Testing & QA | Ongoing | 🔴 INSUFFICIENT — no rendering/layout/X11 tests |
| 10 | Documentation | 2 weeks | 🔴 PENDING |
| 11 | Performance | 2 weeks | 🔴 PENDING |
| 12 | Ecosystem | 2 weeks | 🔴 PENDING |

---

## Phase 0: Widget Audit

### Goals
Catalog all existing widgets, identify canonical sources, and create unified registry.

### Status: ✅ COMPLETE (v1.1 - 2026-03-14)

### Results

#### Widget Discovery (Task 0.1)
- **Task 0.1.1**: ✅ Cataloged 150+ .widget files across 30+ directories
- **Task 0.1.2**: ✅ Identified canonical source: `wbuild_widgets/` (42 widgets)
- **Task 0.1.3**: ✅ Documented widget dependencies in WIDGETS.md

**Verification**: Complete spreadsheet in `WIDGETS.md`

#### Canonical Source Selection (Task 0.2)
- **Task 0.2.1**: ✅ Determined official widget source: `wbuild_widgets/`
- **Task 0.2.2**: ✅ Migrated Wmenu, WmenuPopup, Gauge to canonical source
- **Task 0.2.3**: 🔲 Clean up legacy directories (deferred)

**Verification**: 42 widgets now in canonical source

#### Widget Registry (Task 0.3)
- **Task 0.3.1**: ✅ Created `WIDGETS.md` with complete inventory
- **Task 0.3.2**: ✅ Categorized: Core (42), Extended, Legacy, Experimental
- **Task 0.3.3**: ✅ Documented status: Working, Partial, Broken

**Verification**: All widgets documented with status

---

## Phase 1: Foundation & Core Systems

### Goals
- Stabilize build system
- Complete MLS documentation
- Establish testing infrastructure
- Create verification demos

### Tasks

#### 1.1 Build System Consolidation
- [ ] **Task 1.1.1**: Verify root makefile builds all components
- [ ] **Task 1.1.2**: Document build flags in `projekt_build_plan.md`
- [ ] **Task 1.1.3**: Create CI/CD skeleton (GitHub Actions)
- [ ] **Task 1.1.4**: Add `distclean` target to all makefiles

**Verification**: `make distclean && make` succeeds without errors

#### 1.2 MLS Memory System
- [ ] **Task 1.2.1**: Document all MLS functions (see `utils/mls.h`)
- [ ] **Task 1.2.2**: Create MLS API reference in `learn.md`
- [ ] **Task 1.2.3**: Add memory debugging tools

**Verification**: All MLS functions have documentation comments

#### 1.3 Testing Infrastructure
- [ ] **Task 1.3.1**: Create `tests/run_all_tests.sh`
- [ ] **Task 1.3.2**: Add headless testing with `xvfb-run`
- [ ] **Task 1.3.3**: Implement LAYOUT trace extraction tool

**Verification**: Tests run in CI without display

#### 1.4 Widget Verification Demo
- [ ] **Task 1.4.1**: Create `demo/` with label, button, list
- [ ] **Task 1.4.2**: Add selection/paste output demo
- [ ] **Task 1.4.3**: Document demo structure

**Verification**: Demo runs and shows all widget types

---

## Phase 2: Core Widget Library

### Goals
Complete the foundational widget set required for any application.

### Widget Inventory

| Widget | Source | Status | Priority |
|--------|--------|--------|----------|
| Wlabel | `wbuild_widgets/` | ✅ Complete | P0 |
| Wbutton | `wbuild_widgets/` | ✅ Complete | P0 |
| Wlist | `wbuild_widgets/` | ✅ Complete | P0 |
| Wlist4 | `wbuild_widgets/` | ✅ Complete | P0 |
| WlistMulti | `wbuild_widgets/` | ✅ Complete | P0 |
| Wls | `wbuild_widgets/` | ✅ Complete | P0 |
| WlsMulti | `wbuild_widgets/` | ✅ Complete | P0 |
| Wedit | `wbuild_widgets/` | ✅ Complete | P0 |
| WeditMV | `wbuild_widgets/` | ✅ Complete | P0 |
| HSlider | `wbuild_widgets/` | ✅ Complete | P0 |
| VSlider | `wbuild_widgets/` | ✅ Complete | P0 |
| WPaned | `wbuild_widgets/` | ✅ Complete | P0 |
| Wcombo | `wbuild_widgets/` | ✅ Complete | P1 |
| Woption | `wbuild_widgets/` | ✅ Complete | P1 |
| IconSVG | `wbuild_widgets/` | ✅ Complete | P1 |
| Canvas | `wbuild_widgets/` | ✅ Complete | P1 |
| Frame | `wbuild_widgets/` | ✅ Complete | P1 |
| VBox/HBox | `wbuild_widgets/` | ✅ Complete | P0 |
| Wseparator | `wbuild_widgets/` | ✅ Complete | P1 |

### Tasks

#### 2.1 Widget Documentation
- [ ] **Task 2.1.1**: Document each widget in `learn.md`
- [ ] **Task 2.1.2**: Create widget reference markdown per widget
- [ ] **Task 2.1.3**: Add widget screenshots to docs

**Verification**: All widgets have API documentation

#### 2.2 Widget Tests
- [ ] **Task 2.2.1**: Create C test for each widget
- [ ] **Task 2.2.2**: Add visual regression baselines
- [ ] **Task 2.2.3**: Document test patterns

**Verification**: Each widget has a test file

---

## Phase 3: Advanced Widgets (Consolidation)

### Goals
Consolidate existing advanced widgets and integrate into main build.

### Widget Status (v1.1 Analysis)

| Widget | Status | Location | Notes |
|--------|--------|----------|-------|
| WscrollArea | ⚠️ EXISTS | adminpanel/WlistScroll | Needs registration |
| Wsplitter | ✅ COMPLETE | wbuild_widgets/ | Already in build |
| WtabWidget | ❌ MISSING | - | Needs implementation |
| Wmenu | ✅ COMPLETE | wbuild_widgets/ | Migrated from clean-build |
| WmenuPopup | ✅ COMPLETE | wbuild_widgets/ | Migrated from clean-build |
| Wmenubar | ❌ MISSING | - | Needs implementation |
| Wtooltip | ❌ MISSING | - | Needs implementation |
| WstatusBar | ❌ MISSING | - | Needs implementation |
| Wtoolbar | ❌ MISSING | - | Needs implementation |
| Wimage | ❌ MISSING | - | Needs implementation |
| Wgauge | ✅ COMPLETE | wbuild_widgets/ | Migrated from gauge_widget_test |

### Tasks

#### 3.1 WscrollArea (WlistScroll)
- [ ] **Task 3.1.1**: Copy WlistScroll to wbuild_widgets
- [ ] **Task 3.1.2**: Add to build system
- [ ] **Task 3.1.3**: Test scrolling behavior

**Verification**: Child widget scrolls when content exceeds bounds

#### 3.2 Wmenu/WmenuPopup Integration
- [ ] **Task 3.2.1**: Copy Wmenu.widget to wbuild_widgets
- [ ] **Task 3.2.2**: Copy WmenuPopup.widget to wbuild_widgets  
- [ ] **Task 3.2.3**: Add to register_wb.h and rebuild

**Verification**: Menu opens on click, items selectable

#### 3.3 Wgauge Integration
- [ ] **Task 3.3.1**: Copy Gauge.widget to wbuild_widgets
- [ ] **Task 3.3.2**: Add progress bar functionality
- [ ] **Task 3.3.3**: Test percentage display

**Verification**: Gauge displays 0-100% progress

#### 3.4 WtabWidget (NEW - Needs Implementation)
- [ ] **Task 3.4.1**: Design tab bar + content area
- [ ] **Task 3.4.2**: Implement tab switching
- [ ] **Task 3.4.3**: Add to build system

**Verification**: Click tab switches visible content

---

## Phase 4: Layout & Containers

### Goals
Provide flexible layout mechanisms beyond basic boxes.

### Status: ✅ COMPLETE (v1.2 - 2026-03-18)

### Widget Status

| Widget | Source | Status | Notes |
|--------|--------|--------|-------|
| VBox | `wbuild_widgets/` | ✅ Complete | Vertical box layout |
| HBox | `wbuild_widgets/` | ✅ Complete | Horizontal box layout |
| Gridbox | `plainc_widgets/` | ✅ Complete | Weight-based grid layout |
| WPaned | `wbuild_widgets/` | ✅ Complete | Paned window with grips |
| Wsplitter | `wbuild_widgets/` | ✅ Complete | Simple two-pane splitter |

### Tasks

#### 4.1 Wgridbox
- [x] **Task 4.1.1**: Enhanced Gridbox with weight-based layout
- [x] **Task 4.1.2**: Added row/column spanning (gridWidth, gridHeight)
- [x] **Task 4.1.3**: Documented grid layout syntax in `learn.md`

**Verification**: ✅ Complex layouts with weights work correctly (see `tests/test_gridbox.c`)

#### 4.2 Layout Tests
- [x] **Task 4.2.1**: Created layout verification demo (`tests/test_gridbox.c`)
- [x] **Task 4.2.2**: Tested nested layouts
- [x] **Task 4.2.3**: Verified resize behavior

**Verification**: ✅ All layout demos render correctly, weights distribute extra space proportionally

### Gridbox Features Implemented
- **Weight-based distribution**: `weightx`, `weighty` for proportional sizing
- **Cell spanning**: `gridWidth`, `gridHeight` for multi-cell widgets
- **Fill modes**: "none", "width", "height", "both"
- **Gravity alignment**: Position within cell when larger than preferred
- **Margin control**: `margin` resource for cell padding

### Reference
See `learn.md` section "Gridbox Layout Widget" for full API documentation.

---

## Phase 5: Input Controls

### Goals
Provide rich input mechanisms for user data entry.

### Status: ✅ COMPLETE (v1.1 - 2026-03-15)

### Widget Requirements

| Widget | Description | Priority | Status |
|--------|-------------|----------|--------|
| Wcheckbox | Toggle with checkmark | P1 | ✅ COMPLETE |
| WspinBox | Numeric +/- input | P1 | ✅ COMPLETE |
| Wpassword | Hidden input | P1 | ✅ COMPLETE |
| WdatePicker | Calendar selection | P2 | ❌ MISSING |
| WsearchBox | Search input with icon | P2 | ❌ MISSING |

### Tasks

#### 5.1 Wcheckbox
- [x] **Task 5.1.1**: Design checkbox widget
- [x] **Task 5.1.2**: Implement toggle state
- [ ] **Task 5.1.3**: Add tri-state support

**Verification**: ✅ Click toggles state, callback fires

#### 5.2 WspinBox
- [x] **Task 5.2.1**: Create numeric input with arrows
- [x] **Task 5.2.2**: Implement min/max/step
- [x] **Task 5.2.3**: Add keyboard navigation

**Verification**: ✅ Arrows change value, Enter confirms

#### 5.3 Wpassword
- [x] **Task 5.3.1**: Create password input widget
- [x] **Task 5.3.2**: Implement hidden text display
- [x] **Task 5.3.3**: Add submit callback

**Verification**: ✅ Text hidden, callback on Enter

#### 5.4 WdatePicker
- [ ] **Task 5.4.1**: Design calendar popup
- [ ] **Task 5.4.2**: Implement month navigation
- [ ] **Task 5.4.3**: Add date selection

**Verification**: Can select date from calendar

---

## Phase 6: Data Views

### Goals
Display hierarchical and tabular data effectively.

### Widget Requirements

| Widget | Description | Priority |
|--------|-------------|----------|
| WtreeView | Hierarchical tree | P1 |
| WtableView | Multi-column grid | P1 |
| WgridView | Spreadsheet-like | P2 |

### Tasks

#### 6.1 WtreeView
- [ ] **Task 6.1.1**: Design tree data structure
- [ ] **Task 6.1.2**: Implement expand/collapse
- [ ] **Task 6.1.3**: Add icons per node type

**Verification**: Tree displays, nodes expand/collapse

#### 6.2 WtableView
- [ ] **Task 6.2.1**: Design column model
- [ ] **Task 6.2.2**: Implement cell rendering
- [ ] **Task 6.2.3**: Add sorting capability

**Verification**: Table displays data, columns sortable

---

## Phase 7: Application Shell & Dialogs

### Goals
Provide standard application chrome and common dialogs.

### Tasks

#### 7.1 Standard Dialogs
- [ ] **Task 7.1.1**: Implement WmessageBox (alert/confirm)
- [ ] **Task 7.1.2**: Create WcolorPicker
- [ ] **Task 7.1.3**: Build WfontPicker

**Verification**: Dialogs open, return values correctly

#### 7.2 Application Shell
- [ ] **Task 7.2.1**: Enhance Wtoolbar
- [ ] **Task 7.2.2**: Implement WstatusBar
- [ ] **Task 7.2.3**: Create app template

**Verification**: Demo app uses all shell components

---

## Phase 8: Lua Integration (LUI)

### Goals
Complete the LUI framework for declarative UI building.

### LUI Current Status (v1.2 Update - 2026-04-13)

| Component | Status | Location |
|-----------|--------|----------|
| Parser | ✅ Working | `lui/parser.lua` |
| Macros | ✅ Working | `lui/macros.lua` |
| Xt Backend | ✅ Working | `lui/backend_xt.lua` |
| GUI Glue | ✅ Working | `lui/gui_xt.lua` (get_widget, get, set) |
| Registry | ✅ Working | `lui/registry.lua` (22 tags) |
| Test Harness | ✅ Working | `lui/test_harness.lua` |
| Commander Runner | ✅ Working | `experimental/commander_runner.c` |
| C Bindings (xtaction, xtapptimeout) | ✅ Working | `experimental/xt_bridge.c` |
| Demo (Commander) | ❌ NO WORKING DEMO | No complete application runs correctly |
| re-tex in LUI | ⚠️ Partial | Needs explicit demo |
| LUI Tests | ⚠️ 8/8 pass, no visual | `lui_test/run_widget_tests.sh` |
| Deployment Docs | ⚠️ Missing | Needs documentation |

### Widget Registry Coverage

22 LUI tags currently registered in `lui/registry.lua`:
window, label, Wlabel, button, Wbutton, command, toggle, edit, grid, vertical, horizontal, image, WpixBtn, separator, check, scrolled, list-view, splitter, vslider, list4, wlist4, Wlist4

43 xtcw widget classes registered via `XtcwRegister()` + `XpRegisterAll()` — all can be created but only 4 have verified TRACE(50) actions.

### Critical Gap: No Visual Verification

The 8/8 passing tests only verify:
- Widget creation via `gui.get_widget()` (existence check)
- Property get/set (string round-trip)
- Callback dispatch (with 300ms delay for polling timer)
- TRACE(50) output (stderr grep)

What they do NOT verify:
- Widgets render with correct dimensions (width > 0, height > 0)
- Text is not truncated (Wlabel int-cast bug pattern)
- Layout distributes space correctly (Gridbox underflow pattern)
- Selection visually highlights (Wlabel dirty flag pattern)
- X11 protocol errors are zero

### Reference: `test_plan.md` (new)

## Documentation

All project documentation has been consolidated into `docs/`:

| File | Content |
|------|---------|
| `docs/index.md` | Master index with cross-references |
| `docs/architecture.md` | Project structure, build pipeline, wbuild, C host, TRACE |
| `docs/widgets.md` | Widget class reference, LUI registry, known bugs, TRACE(50) status |
| `docs/lui.md` | LUI framework, S-expressions, property mapping, C bindings, tests |
| `docs/mls.md` | MLS library: lists, strings, debugging |
| `docs/testing.md` | Test plan: E/P/A/L/T categories, bug patterns, infrastructure needs |

Legacy files (still in project root, superseded by `docs/`):
`overview.md`, `WIDGETS.md`, `lui-summary.md`, `how-to-use-mls.md`, `howto.md`

---

#### 8.1 LUI Parser
- [ ] **Task 8.1.1**: Document LPeg parser
- [ ] **Task 8.1.2**: Add macro system tests
- [ ] **Task 8.1.3**: Optimize parser performance

**Verification**: Complex LUI scripts parse correctly

#### 8.2 LUI Backend
- [ ] **Task 8.2.1**: Map all widgets to LUI tags
- [ ] **Task 8.2.2**: Add property transformations
- [ ] **Task 8.2.3**: Document backend API

**Verification**: All widgets accessible from LUI

#### 8.3 LUI Examples
- [ ] **Task 8.3.1**: Create form example
- [ ] **Task 8.3.2**: Build table example
- [ ] **Task 8.3.3**: Document best practices

**Verification**: Examples run without errors

---

## Phase 9: Testing & Quality Assurance

### Test Infrastructure Current Status (v1.2 Update - 2026-04-13)

| Category | Coverage | Details |
|----------|---------|----------|
| Widget Creation | ✅ 22 LUI tags | All resolve to widget classes, `gui.get_widget()` returns non-nil |
| Property Get/Set | ⚠️ Minimal | Only `label` tested for string round-trip |
| Callbacks | ✅ Command tested | set→notify→unset chain verified with 300ms delay |
| TRACE(50) Actions | ✅ 4 widgets | WpixBtn, Wlist4, Wcombo, Wlabel have TRACE(50) in source |
| Visual Rendering | ❌ MISSING | No screenshot or geometry verification |
| Layout Correctness | ❌ MISSING | No test for width > 0, height > 0, weight distribution |
| X11 Error Check | ❌ MISSING | No X11 error handler counting |
| Memory Leaks | ⚠️ ASan only | LeakSanitizer reports leaks on exit; no systematic leak testing |
| Regression Baselines | ❌ MISSING | No visual or geometry baselines to diff against |

### Testing Patterns Used:
- **Integration Tests**: `lui_test/run_widget_tests.sh` (8/8 pass, creation+callbacks only)
- **TRACE(50) Capture**: Widgets instrumented in `.widget` source files, verified by stderr grep
- **Lua Test Harness**: `lui/test_harness.lua` provides `click()`, `toggle()`, `action()`, `expect()`

### Testing Gaps (Critical):
1. **No visual/rendering tests** — The most common bug type (truncated text, misaligned layout) is completely untested
2. **No geometry assertions** — Widgets can have 0 width/height and tests pass
3. **No X11 error detection** — Protocol errors pass silently
4. **No property coverage** — Only `label` is tested; no numeric, color, or list properties
5. **No action coverage** — Only Command callback and WpixBtn TRACE tested; other 43 widgets untested
6. **No regression baselines** — When bugs are fixed (Gridbox underflow, Wlabel truncation), nothing prevents regression

### Reference: `test_plan.md` (new)

### Goals
Establish rigorous testing methodology.

### Tasks

#### 9.1 Widget Existence Tests (E-level) — HIGHEST PRIORITY
- [ ] **Task 9.1.1**: Create test that instantiates every xtcw widget class (43 classes)
- [ ] **Task 9.1.2**: Assert no "Unknown widget tag" warnings for registered tags
- [ ] **Task 9.1.3**: Assert `gui.get_widget()` returns non-nil for each widget
- [ ] **Task 9.1.4**: Assert zero X11 protocol errors during creation/destruction

**Verification**: All 43 classes create without X11 errors or warnings

#### 9.2 Property Round-Trip Tests (P-level)
- [ ] **Task 9.2.1**: Test `label` get/set on Wlabel, Wbutton, WpixBtn
- [ ] **Task 9.2.2**: Test `spinValue` get/set on WspinBox
- [ ] **Task 9.2.3**: Test `alignment` get/set on Wlabel (0, 1, 2)
- [ ] **Task 9.2.4**: Test Gridbox properties (gridx, gridy, weightx, weighty, fill)

**Verification**: At least one property round-trips for each widget

#### 9.3 Action & Callback Tests (A-level)
- [ ] **Task 9.3.1**: Test Command widget with full set→notify→unset chain
- [ ] **Task 9.3.2**: Test Toggle widget state change via notify
- [ ] **Task 9.3.3**: Test WpixBtn highlight→notify→reset with TRACE(50)
- [ ] **Task 9.3.4**: Test Wlist4 select_line with TRACE(50)
- [ ] **Task 9.3.5**: Test callback dispatch latency (300ms wait for polling timer)

**Verification**: Actions produce TRACE(50) output, callbacks fire, state changes observed

#### 9.4 Layout & Rendering Tests (L-level) — CRITICAL
- [ ] **Task 9.4.1**: Add `xtgeometry(widget)` C binding to return width, height, x, y
- [ ] **Task 9.4.2**: Test Gridbox weight distribution: child with weight=2 gets 2x space
- [ ] **Task 9.4.3**: Test Wlabel text dimensions > 0 (catches int-cast truncation)
- [ ] **Task 9.4.4**: Test Wlabel alignment produces different x-offsets
- [ ] **Task 9.4.5**: Test no widget has width=0 or height=0 after realize (catches underflow)

**Verification**: All widgets render with positive dimensions, layout distributes space correctly

#### 9.5 Visual Regression Framework
- [ ] **Task 9.5.1**: Add X11 error handler that counts protocol errors
- [ ] **Task 9.5.2**: Add `xtscreenshot(widget, filename)` C binding for screenshot capture
- [ ] **Task 9.5.3**: Create baseline screenshots for key widget layouts
- [ ] **Task 9.5.4**: Create pixel-diff comparison for regression detection

**Verification**: Rendered output matches baseline; X11 errors count stays at zero

#### 9.6 TRACE(50) Regression Tests (T-level)
- [x] **Task 9.6.1**: WpixBtn TRACE(50) — highlight, notify, reset
- [x] **Task 9.6.2**: Wlist4 TRACE(50) — highlight, reset, motion_start, motion_end, select_line
- [x] **Task 9.6.3**: Wcombo TRACE(50) — all 7 action procs
- [x] **Task 9.6.4**: Wlabel TRACE(50) — info, select_start, select_extend, select_end
- [ ] **Task 9.6.5**: Add TRACE(50) to Xaw Command, Toggle, Repeater, Scrollbar

**Verification**: Each action proc produces `[50]` line in stderr output

---

## Phase 10: Documentation & Examples

### Goals
Complete all documentation for users and developers.

### Tasks

#### 10.1 API Documentation
- [ ] **Task 10.1.1**: Generate widget reference
- [ ] **Task 10.1.2**: Document MLS API
- [ ] **Task 10.1.3**: Document re-tex API

**Verification**: All functions documented

#### 10.2 Tutorials
- [ ] **Task 10.2.1**: "Your First App" tutorial
- [ ] **Task 10.2.2**: Widget creation guide
- [ ] **Task 10.2.3**: LUI tutorial

**Verification**: New users can build apps

#### 10.3 Examples
- [ ] **Task 10.3.1**: Complete file manager demo
- [ ] **Task 10.3.2**: Form builder demo
- [ ] **Task 10.3.3**: Text editor demo

**Verification**: Demos showcase all features

---

## Phase 11: Performance & Optimization

### Goals
Ensure toolkit meets performance targets.

### Tasks

#### 11.1 Profiling
- [ ] **Task 11.1.1**: Profile widget rendering
- [ ] **Task 11.1.2**: Identify bottlenecks
- [ ] **Task 11.1.3**: Optimize hot paths

**Verification**: Render times under thresholds

#### 11.2 Caching
- [ ] **Task 11.2.1**: Optimize re-tex layout cache
- [ ] **Task 11.2.2**: Implement pixmap pooling
- [ ] **Task 11.2.3**: Add lazy rendering

**Verification**: Scroll performance smooth

---

## Phase 12: Ecosystem & Distribution

### Goals
Package toolkit for easy adoption.

### Tasks

#### 12.1 Packaging
- [ ] **Task 12.1.1**: Create distribution packages
- [ ] **Task 12.1.2**: Document dependencies
- [ ] **Task 12.1.3**: Create installer

**Verification**: Package installs cleanly

#### 12.2 Community
- [ ] **Task 12.2.1**: Set up issue tracker
- [ ] **Task 12.2.2**: Create contribution guide
- [ ] **Task 12.2.3**: Document roadmap publicly

**Verification**: External contributions accepted

---

## Verification Checklist

### Build Verification
```bash
# Clean build
make distclean && make

# All 43 xtcw widgets compile
make libxtcw

# Test suite passes
bash lui_test/run_widget_tests.sh
```

### Widget Verification (Per Widget — REVISED)
- [ ] Creates without error OR "Unknown widget tag" warning
- [ ] `gui.get_widget()` returns non-nil Widget
- [ ] At least one property round-trips correctly (get/set)
- [ ] Renders with width > 0 AND height > 0
- [ ] No X11 protocol errors on creation or destruction
- [ ] Has TRACE(50) instrumentation (or is Xaw upstream)
- [ ] Actions produce expected TRACE(50) output
- [ ] Callbacks fire correctly (for interactive widgets)
- [ ] No memory leaks beyond known baseline
- [ ] Has test coverage in `lui_test/`
- [ ] Has documentation in `learn.md`

**NO WIDGET IS CONSIDERED VERIFIED UNTIL IT PASSES ALL APPLICABLE CHECKS ABOVE.**

### Integration Verification
- [x] LUI scripts parse correctly
- [ ] Widgets interact properly (full action chains)
- [ ] Callbacks fire correctly (with 300ms dispatch delay)
- [ ] TRACE(50) output captured and verified
- [ ] No X11 protocol errors during test run

### Performance Verification
- [ ] Startup time < 1s
- [ ] Render time < 16ms (60fps)
- [ ] Memory usage < 100MB baseline

---

## Appendix: Common Error Reference

See `errors.md` for solutions to:
- MLS_DEBUG flag mismatches
- Gridbox geometry calculations
- Wlabel selection rendering
- re-tex coordinate systems
- Xt resource conversion

---

## Appendix: Widget Quick Reference

### Creating a Widget (C)
```c
Widget w = XtVaCreateManagedWidget(
    "name",
    widgetClass,
    parent,
    XtNlabel, "Text",
    NULL
);
```

### Creating a Widget (LUI)
```lua
(window {:title "My App"}
  (vertical {}
    (button {:label "Click" :on-click "handle_click"})
    (label {:text "Hello"})))
```

### Widget Resources
```c
// Get value
XtVaGetValues(widget, XtNlabel, &value, NULL);

// Set value
XtVaSetValues(widget, XtNlabel, "new value", NULL);
```

### Callbacks
```c
// Register callback
XtAddCallback(widget, XtNcallback, my_callback, data);

// In widget
XtCallCallback(widget, XtNcallback, call_data);
```

---

**End of Plan**
