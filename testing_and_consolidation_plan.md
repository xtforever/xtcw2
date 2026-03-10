# Master Plan: Project Redesign, Consolidation, and Rigorous Testing

This plan outlines a phased approach to redesign, consolidate, and rigorously test the XTCW toolkit, focusing on its core sub-projects: `re-tex`, `Wlabel`, `luarunner`, `lui`, and the build system.

## Core Engineering Principles

### 1. Strict TDD Workflow (Red-Green-Refactor)
Every single task, no matter how small, must follow this cycle:
- **Red Phase:** Create a new test file/case and write a failing unit test that defines the expected behavior. Verify the failure.
- **Green Phase:** Write the **minimum** implementation code necessary to make the test pass.
- **Refactor Phase:** Clean up the implementation and test code while ensuring tests remain green.

### 2. Micro-Increments
Break every feature and fix into the smallest possible logical steps. 
- Time is secondary to correctness. 
- Favour a high volume of small, verified commits over large, complex updates.

### 3. Visual Traceability (The LAYOUT Mandate)
Since this is a complex visual application, we require a deterministic way to verify GUI element placement and sizing.
- **Mandatory Instrumentation:** ALL functions that create visual output (layout engines, widget draw methods, cell renderers) MUST include a `TRACE` call at level 2 with the prefix `LAYOUT`.
- **Format:** `TRACE(2, "LAYOUT handle %d, class %s, name %s, width %d, height %d, x %d, y %d, color %08x, content %s", ...)`
- **Verification:** Testing scripts will extract lines starting with `LAYOUT` from the debug output to compare actual rendered hierarchies against expected geometric baselines.

### 4. Core Component Policy (Waterfall with Feedback)
The project follows a "Modified Waterfall" model. Since the foundation supports everything else, the following rule applies:
- **Impact Analysis:** Any change to **Phase 1 (MLS)** or **Phase 2 (Extended MLS/Tables)** is considered a "Core Reset."
- **Regression Loop:** If a core component is modified, you MUST "Go Back to Start" and re-validate every single phase from Phase 3 onwards to ensure the architectural changes haven't introduced subtle regressions.

### 5. Continuous Documentation and Git
- **Git:** Every small increment (one TDD cycle) MUST be committed to Git.
- **Errors:** Every error and its solution MUST be documented in `errors.md` immediately.
- **Learning:** Update `learn.md` with architectural insights.

---

## Phase 1: Foundation - The Multiple List System (mls)
**Goal:** Master and document the core memory and data structure library.

*   **Step 1.1 [TDD]: Learning the Handle Model.** Write tests to verify handle allocation and retrieval logic.
*   **Step 1.2 [TDD]: Rigorous s_cstr Testing.** Verify immutability and global interning of `s_cstr` handles.
*   **Step 1.3 [TDD]: Memory Lifecycle Audit.** Verify `m_destruct()` vs `conststr_free()`.
*   **Step 1.4 [TDD]: Trace Macro Validation.** Ensure `TRACE(2, ...)` correctly outputs to stderr and that `LAYOUT` lines can be extracted via grep.
*   **Step 1.5: Regression Baseline.** Foundational pass state for all MLS core features.

## Phase 2: Extended MLS and Recursive Resource Management
**Goal:** Implement and validate advanced features (`free_hdl`, `m_table`).

*   **Step 2.1 [TDD]: Recursive Free Mechanism.** Verify `m_free` cleans children automatically using `free_hdl`.
*   **Step 2.2 [TDD]: Associative Tables (`m_table.c`).** Test key/value storage with mixed types and nested cleanup.
*   **Step 2.3 [TDD]: Ownership Rules.** Test dynamic string duplication vs. constant string handle usage.
*   **Step 2.4 [TDD]: Nested Structure Stress Test.** Verify zero leaks in deep (Table -> List -> String) hierarchies.
*   **Step 2.5 [Regression]:** Re-run Phase 1 tests. Core reset applies if Phase 2 modifies foundation.

## Phase 3: Build System and Makefile Consolidation
**Goal:** Establish a reliable, modular build pipeline for granular testing.

*   **Step 3.1: Makefile Refactoring.** Separate targets into `core`, `widgets`, `luarunner`, and `tests`.
*   **Step 3.2: Headless Testing.** Integrate `xvfb-run` and headless Cairo.
*   **Step 3.3: Trace Extraction Tooling.** Create a helper script to filter and diff `LAYOUT` trace output.
*   **Step 3.4: CI Automation.** Create `run_all_tests.sh`.
*   **Step 3.5 [Regression]:** Verify Phase 1 and 2 pass state.

## Phase 4: Core Rendering Engine (`re-tex`)
**Goal:** Verify the TeX-inspired layout engine's correctness and safety.

*   **Step 4.1 [TDD]: Headless Layout.** Tests asserting `width/height/depth` of boxes. **Requirement:** Every box creation must output a `LAYOUT` trace.
*   **Step 4.2 [TDD]: Line-breaking.** Verify wrapping and glue behavior via `LAYOUT` trace comparisons.
*   **Step 4.3 [TDD]: Math Layout.** Verify sub/superscript and delimiter geometry via `LAYOUT` trace.
*   **Step 4.4 [Visual]: Visual Regression.** Compare Cairo PNG renders against baseline images.
*   **Step 4.5 [Regression]:** Re-run Phase 1-3.

## Phase 5: The `Wlabel` Widget Redesign & Interactivity
**Goal:** Transform `Wlabel` into an interactive rich-text component.

*   **Step 5.1 [TDD]: Hit-Detection API.** Implement `retex_paragraph_get_node_at`.
*   **Step 5.2 [TDD]: Selection State.** Implement selection indices and highlight rendering (with `LAYOUT` traces for highlight boxes).
*   **Step 5.3 [TDD]: Clipboard.** Implement X11 Selection handlers for COPY/PASTE.
*   **Step 5.4 [TDD]: Drag and Drop.** Implement XDND protocol handlers.
*   **Step 5.5 [Stress]: Layout Resilience.** Verify rapid resizes; ensure `LAYOUT` traces remain stable under identical widths.
*   **Step 5.6 [Regression]:** Re-run Phase 1-4.

## Phase 6: Advanced Data Display - The Multi-column List Widget
**Goal:** Redesign `WlsMulti` for massive datasets and `re-tex` cell rendering.

*   **Step 6.1 [TDD]: Cell Caching.** Implement visible-only rendering. **Requirement:** Trace `LAYOUT` for visible cells only.
*   **Step 6.2 [TDD]: Re-tex Cell Integration.** Alignment and mixed fonts at cell level verified via `LAYOUT` trace.
*   **Step 6.3 [TDD]: Geometry.** Redesign column management; trace final calculated column widths.
*   **Step 6.4 [TDD]: Selection & Actions.** Verify selection logic and "Click-to-Action".
*   **Step 6.5 [Stress]: Volume Testing.** Recursive cleanup of 10,000+ row structures.
*   **Step 6.6 [Regression]:** Re-run Phase 1-5.

## Phase 7: Lua Scripting (`luarunner`)
**Goal:** Ensure a memory-safe and performant C-to-Lua bridge.

*   **Step 7.1 [TDD]: C-Bindings.** Verify data passing accuracy.
*   **Step 7.2 [TDD]: Main Loop.** Test non-blocking Lua execution.
*   **Step 7.3 [TDD]: Callbacks.** Verify widget events fire Lua functions.
*   **Step 7.4 [Audit]: Memory Sync.** Audit interaction between Lua GC and Xt destruction.
*   **Step 7.5 [Regression]:** Re-run Phase 1-6 within Lua context.

## Phase 8: LUI (Lua UI Layout System)
**Goal:** Verify accurate translation of declarative UI to widget hierarchies.

*   **Step 8.1 [TDD]: Parser.** Verify `.lui` file parsing into `WcCreate` trees.
*   **Step 8.2 [TDD]: Layout Regression.** Test space distribution in containers via root-to-leaf `LAYOUT` trace comparison.
*   **Step 8.3 [TDD]: Data Binding.** Verify LUI stores sync with Lua tables and trigger refreshes.
*   **Step 8.4 [E2E]: System Test.** Launch full application; compare captured `LAYOUT` log against expected UI tree.
*   **Step 8.5 [Regression]:** Re-run Phase 1-7.

## Phase 9: File Manager Components & Integration
**Goal:** Finalize widgets and Lua logic for the File Manager application.

*   **Step 9.1 [TDD]: Navigation.** Test `WPaned` and directory scanning.
*   **Step 9.2 [TDD]: Threaded Tasks.** Non-blocking file operations with IPC progress updates.
*   **Step 9.3 [TDD]: Progress.** `Gauge` widget updates verified via `LAYOUT` percentage traces.
*   **Step 9.4 [TDD]: File Interaction.** `IconSVG` and F-key mapping.
*   **Step 9.5 [TDD]: Feedback.** `Wmenu` and `Wtooltip`.
*   **Step 9.6 [Regression]:** Phase 1-8 execution.

## Phase 10: Consolidation & Ecosystem Audit
**Goal:** Standardize the entire toolkit and finalize documentation.

*   **Step 10.1: API Standardization.** Consistent property names across all widgets.
*   **Step 10.2: Universal Re-tex.** Port remaining widgets to `re-tex`.
*   **Step 10.3 [TDD]: Full Coverage.** Ensure every widget has a C test and a geometric baseline `LAYOUT` log.
*   **Step 10.4: Final Sweep.** Full regression sweep and finalized documentation.
